/*
 * Copyright [2020] [Technical University of Munich]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//////////////////////////////////////////////////////////////////////////////////////
/// \file permutation.cpp
/// \brief C++ Source for vector vector permutation helpers for RISC-V ISS
/// \date 09/10/2020
//////////////////////////////////////////////////////////////////////////////////////

#include <cstring>
#include <cstdint>
#include <cstddef>
#include <cstdlib>

#include "misc/permutation.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"
#include "arithmetic/softfloat-extension.hpp"

// Private function declarations

auto iterate_vector_slide_one(SVector const &vs2, std::uint64_t scalar, SVector &vd, SVRegister const &vm, bool mask,
                              std::size_t start_index, bool slide_down) -> void;

auto convert_float_from_freg(std::uint64_t raw_value, std::size_t sew, std::size_t flen) -> std::uint64_t;

auto convert_float_from_vec(std::uint64_t raw_value, std::size_t sew, std::size_t flen) -> std::uint64_t;

// Private function definitions

auto iterate_vector_slide_one(SVector const &vs2, std::uint64_t scalar, SVector &vd, SVRegister const &vm, bool mask,
                              std::size_t start_index, bool slide_down) -> void
{
    // Slide1up: vd[i+1] = vs2[i]
    // Slide1down: vd[i] = vs2[i+1]
    auto const vs2_offset = slide_down;
    auto const vd_offset = !vs2_offset;

    if (start_index == 0 && (!mask || vm.get_bit(0)) && !slide_down)
    {
        // Slideup: If 0-element is active: copy scalar value to vd[0]
        vd[0] = scalar;
    }
    else if ((!mask || vm.get_bit(vd.length_ - 1)) && slide_down)
    {
        // Slidedown: If last element is active: copy scalar value to vd[vl-1]
        vd[vd.length_ - 1] = scalar;
    }

    for (std::size_t i_element = start_index; i_element < (vd.length_ - 1); ++i_element)
    {
        auto mask_bit = vm.get_bit(i_element + vd_offset);
        if (!mask || mask_bit)
        {
            vd[i_element + vd_offset] = vs2[i_element + vs2_offset];
        }
    }
}

inline auto convert_float_from_freg(std::uint64_t raw_value, std::size_t sew, std::size_t flen) -> std::uint64_t
{
    std::uint64_t converted_value = 0;
    if (flen > sew)
    {
        converted_value = (sew == 16) ? check_and_unbox_f16(f64(raw_value)).v : check_and_unbox_f32(f64(raw_value)).v;
    }
    else
    {
        switch (sew)
        {
        case 16:
            converted_value = f16(raw_value).v;
            break;
        case 32:
            converted_value = f32(raw_value).v;
            break;
        case 64:
            converted_value = f64(raw_value).v;
            break;
        default:
            // Illegal
            exit(EXIT_FAILURE);
            break;
        }
    }
    return converted_value;
}

inline auto convert_float_from_vec(std::uint64_t raw_value, std::size_t sew, std::size_t flen) -> std::uint64_t
{
    std::uint64_t converted_value = 0;

    switch (sew)
    {
    case 16:
        converted_value = (flen == 32) ? f16_to_f32(f16(raw_value)).v : box_f16(f16(raw_value)).v;
        break;
    case 32:
        converted_value = (flen == 32) ? raw_value : box_f32(f32(raw_value)).v;
        break;
    case 64:
        converted_value = raw_value;
        break;
    default:
        // Illegal
        exit(EXIT_FAILURE);
        break;
    }

    return converted_value;
}

// Public function definitions

auto VPERM::perm_op_slide_vx(std::uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                             PermInstrInfo const &perm_instr_info, std::uint16_t reg_vd, std::uint16_t reg_vs2,
                             std::uint8_t *scalar_reg_mem, std::uint8_t scalar_reg_len_bytes) -> VILL::vpu_return_t
{
    // TODO: Currently only supports slide1x
    if (v_instr_info.start_element > v_instr_info.vector_length || v_instr_info.vector_length == 0)
    {
        return (VILL::VPU_RETURN::NO_EXCEPT);
    }

    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(reg_vs2))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    std::uint64_t scalar = (scalar_reg_len_bytes > xlen_32_bytes)
                               ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                               : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));

    auto const sew_bytes = v_instr_info.sew >> 3;

    if (perm_instr_info.float_instr)
    {
        if (scalar_reg_len_bytes > sew_bytes)
        {
            scalar = (sew_bytes == 2) ? check_and_unbox_f16(f64(scalar)).v : check_and_unbox_f32(f64(scalar)).v;
        }
        else
        {
            switch (v_instr_info.sew)
            {
            case 16:
                scalar = f16(scalar).v;
                break;
            case 32:
                scalar = f32(scalar).v;
                break;
            case 64:
                scalar = f64(scalar).v;
                break;
            default:
                break;
            }
        }
    }
    else
    {
        scalar = mask_and_sign_extend_scalar(scalar, v_instr_info.sew, true);
    }

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = V.get_vec(reg_vd);

    iterate_vector_slide_one(vs2, scalar, vd, V.get_mask_reg(), v_instr_info.masked, v_instr_info.start_element,
                             perm_instr_info.slide_down);

    return VILL::vpu_return_t::NO_EXCEPT;
}

auto VPERM::perm_op_move_float(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info, uint16_t reg_v,
                               uint8_t *scalar_reg_mem, uint8_t flen, bool vec_is_dest) -> VILL::vpu_return_t
{

    RVVRegField V(v_instr_info.vector_register_length, 1, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(reg_v))
    {
        return VILL::VPU_RETURN::VFMV_VEC_ILL;
    }

    V.init();

    RVVector &vec = V.get_vec(reg_v);

    if (vec_is_dest)
    {
        // Take raw value from F registers, convert, move into vector
        std::uint64_t raw_value = (flen > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                              : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));

        auto converted_value = convert_float_from_freg(raw_value, v_instr_info.sew, flen);
        vec[0] = converted_value;
        return VILL::vpu_return_t::NO_EXCEPT;
    }
    else
    {
        std::uint64_t raw_value = vec[0].to_u64();
        auto converted_value = convert_float_from_vec(raw_value, v_instr_info.sew, flen);
        if (flen == 32)
        {
            std::memcpy(scalar_reg_mem, &converted_value, 4);
        }
        else
        {
            std::memcpy(scalar_reg_mem, &converted_value, 8);
        }
        return VILL::vpu_return_t::NO_EXCEPT;
    }
}

VILL::vpu_return_t VPERM::mv_xs(std::uint8_t *vec_reg_mem, std::uint16_t sew_bytes, std::uint16_t vec_len,
                                std::uint16_t vec_reg_len_bytes, std::uint16_t src_vec_reg,
                                std::uint8_t *scalar_reg_mem, std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(1, 1), vec_reg_mem);

    V.init();

    RVVector &vs2 = V.get_vec(src_vec_reg);

    for (std::size_t dst_byte_i = 0; dst_byte_i < scalar_reg_len_bytes; ++dst_byte_i)
    {
        if (dst_byte_i < sew_bytes)
        {
            scalar_reg_mem[dst_byte_i] = vs2[0][dst_byte_i];
        }
        else
        {
            scalar_reg_mem[dst_byte_i] = vs2[0][0] & 0x80 ? 0xFF : 0x00;
        }
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::mv_sx(std::uint8_t *vec_reg_mem, std::uint16_t sew_bytes, std::uint16_t vec_len,
                                std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start,
                                std::uint8_t scalar_reg_len_bytes)
{
    if (vec_elem_start >= vec_len)
    {
        return (VILL::VPU_RETURN::NO_EXCEPT);
    }

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(1, 1), vec_reg_mem);

    V.init();

    RVVector &vd = V.get_vec(dst_vec_reg);

    for (std::size_t dst_byte_i = 0; dst_byte_i < sew_bytes; ++dst_byte_i)
    {
        if (dst_byte_i < scalar_reg_len_bytes)
        {
            vd[0][dst_byte_i] = scalar_reg_mem[dst_byte_i];
        }
        else
        {
            vd[0][dst_byte_i] = scalar_reg_mem[scalar_reg_len_bytes - 1] & 0x80 ? 0xFF : 0x00;
        }
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::fmv_fs(std::uint8_t *vec_reg_mem, std::uint16_t sew_bytes, std::uint16_t vec_len,
                                 std::uint16_t vec_reg_len_bytes, std::uint16_t src_vec_reg,
                                 std::uint8_t *scalar_fp_reg_mem, std::uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::fmv_sf(std::uint8_t *vec_reg_mem, std::uint16_t sew_bytes, std::uint16_t vec_len,
                                 std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                 std::uint8_t *scalar_fp_reg_mem, std::uint16_t vec_elem_start,
                                 std::uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slideup_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                     std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                     std::uint8_t scalar_reg_len_bytes)
{
    if (vec_elem_start > vec_len)
    {
        return (VILL::VPU_RETURN::NO_EXCEPT);
    }

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                        : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_slideup(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slideup_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t u_imm,
                                     std::uint16_t vec_elem_start, bool mask_f)
{
    if (vec_elem_start > vec_len)
    {
        return (VILL::VPU_RETURN::NO_EXCEPT);
    }

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        std::uint64_t imm = static_cast<std::uint8_t>(u_imm & 0x1F);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_slideup(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slidedown_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                       std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                       std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                        : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        std::size_t vlmax = emul_num * vec_reg_len_bytes / sew_bytes / emul_denom;

        vd.m_slidedown(vs2, imm, V.get_mask_reg(), !mask_f, vlmax, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slidedown_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t u_imm,
                                       std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        std::uint64_t imm = static_cast<std::uint8_t>(u_imm & 0x1F);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        std::size_t vlmax = emul_num * vec_reg_len_bytes / sew_bytes / emul_denom;

        vd.m_slidedown(vs2, imm, V.get_mask_reg(), !mask_f, vlmax, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slide1up(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                   std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                   std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                   std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                   std::uint8_t scalar_reg_len_bytes)
{
    if (vec_elem_start > vec_len)
    {
        return (VILL::VPU_RETURN::NO_EXCEPT);
    }

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        if (V.get_mask_reg().get_bit(vec_elem_start) || mask_f)
        {
            V.get_mask_reg().reset_bit(vec_elem_start);
            vd.m_slideup(vs2, 1, V.get_mask_reg(), !mask_f, vec_elem_start);
            V.get_mask_reg().set_bit(vec_elem_start);
            vd[vec_elem_start] = imm;
        }
        else
        {
            vd.m_slideup(vs2, 1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::fslide1up(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                    std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                    std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                    std::uint8_t *scalar_fp_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                    std::uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slide1down(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                     std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                     std::uint8_t scalar_reg_len_bytes)
{
    if (vec_elem_start > vec_len)
    {
        return (VILL::VPU_RETURN::NO_EXCEPT);
    }

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_slidedown(vs2, 1, V.get_mask_reg(), !mask_f, vec_elem_start);
        if (V.get_mask_reg().get_bit(vd.length_ - 1) || mask_f)
        {
            vd[vd.length_ - 1] = imm;
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::fslide1down(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                      std::uint8_t *scalar_fp_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                      std::uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::vrgather_vv(std::uint8_t *vec_reg_mem, std::uint64_t lmul_num, std::uint64_t lmul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f,
                                      bool ei16)
{
    static constexpr auto eew_ei16 = 16;
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(lmul_num, lmul_denom), vec_reg_mem);
    RVVRegField V_ei16(vec_reg_len_bytes * 8, vec_len, eew_ei16, SVMul(16 * lmul_num, sew_bytes * 8 * lmul_denom),
                       vec_reg_mem);

    if ((!ei16 && !V.vec_reg_is_aligned(src_vec_reg_rhs)) || (ei16 && !V_ei16.vec_reg_is_aligned(src_vec_reg_rhs)))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    if (ei16)
    {
        V_ei16.init();
    }

    RVVector &vs1 = ei16 ? V_ei16.get_vec(src_vec_reg_rhs) : V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    std::size_t vlmax = ((lmul_num * vec_reg_len_bytes) / sew_bytes) / lmul_denom;
    vd.m_vrgather(vs2, vs1, V.get_mask_reg(), !mask_f, vlmax, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::vrgather_vi(std::uint8_t *vec_reg_mem, std::uint64_t lmul_num, std::uint64_t lmul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t imm,
                                      std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(lmul_num, lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    std::uint64_t rhs = imm & 0x1F;
    std::size_t vlmax = ((lmul_num * vec_reg_len_bytes) / sew_bytes) / lmul_denom;
    vd.m_vrgather(vs2, rhs, V.get_mask_reg(), !mask_f, vlmax, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VPERM::vrgather_vx(std::uint8_t *vec_reg_mem, std::uint64_t lmul_num, std::uint64_t lmul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                      std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                      std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(lmul_num, lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    std::size_t vlmax = ((lmul_num * vec_reg_len_bytes) / sew_bytes) / lmul_denom;
    vd.m_vrgather(vs2, imm, V.get_mask_reg(), !mask_f, vlmax, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::vcompress_vm(std::uint8_t *vec_reg_mem, std::uint64_t lmul_num, std::uint64_t lmul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(lmul_num, lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    SVRegister &vs1 = V.get_vecreg(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    std::size_t vlmax = ((lmul_num * vec_reg_len_bytes) / sew_bytes) / lmul_denom;
    vd.m_vcompress(vs2, vs1, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::vmvr_v(std::uint8_t *vec_reg_mem, std::uint64_t lmul_num, std::uint64_t lmul_denom,
                                 std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                 std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t simm,
                                 std::uint16_t vec_elem_start)
{
    auto n_registers = simm + 1;
    if (!(n_registers == 1 || n_registers == 2 || n_registers == 4 || n_registers == 8))
    {
        return VILL::VPU_RETURN::VMVR_SIMM_ILL;
    }

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(n_registers, 1), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    SVRegister &vs2 = V.get_vecreg(src_vec_reg_lhs);
    SVRegister &vd = V.get_vecreg(dst_vec_reg);

    // TODO: check overrun!
    std::memcpy(vd.mem_, vs2.mem_, vec_reg_len_bytes * n_registers);

    return VILL::VPU_RETURN::NO_EXCEPT;
}