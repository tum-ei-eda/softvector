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

#include "misc/permutation.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

VILL::vpu_return_t VPERM::mv_xs(uint8_t *vec_reg_mem, uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                uint16_t src_vec_reg, uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(1, 1), vec_reg_mem);

    V.init();

    RVVector &vs2 = V.get_vec(src_vec_reg);

    for (size_t dst_byte_i = 0; dst_byte_i < scalar_reg_len_bytes; ++dst_byte_i)
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

VILL::vpu_return_t VPERM::mv_sx(uint8_t *vec_reg_mem, uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                uint16_t dst_vec_reg, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                uint8_t scalar_reg_len_bytes)
{
    if (vec_elem_start > vec_len)
    {
        return (VILL::VPU_RETURN::NO_EXCEPT);
    }

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(1, 1), vec_reg_mem);

    V.init();

    RVVector &vd = V.get_vec(dst_vec_reg);

    for (size_t dst_byte_i = 0; dst_byte_i < sew_bytes; ++dst_byte_i)
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

VILL::vpu_return_t VPERM::fmv_fs(uint8_t *vec_reg_mem, uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                 uint16_t src_vec_reg, uint8_t *scalar_fp_reg_mem, uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::fmv_sf(uint8_t *vec_reg_mem, uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                 uint16_t dst_vec_reg, uint8_t *scalar_fp_reg_mem, uint16_t vec_elem_start,
                                 uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slideup_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                     bool mask_f, uint8_t scalar_reg_len_bytes)
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

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_slideup(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slideup_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint16_t src_vec_reg_lhs, uint8_t u_imm, uint16_t vec_elem_start, bool mask_f)
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

        uint64_t imm = static_cast<uint8_t>(u_imm & 0x1F);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_slideup(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slidedown_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
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

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        size_t vlmax = emul_num * vec_reg_len_bytes / sew_bytes / emul_denom;

        vd.m_slidedown(vs2, imm, V.get_mask_reg(), !mask_f, vlmax, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slidedown_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t u_imm, uint16_t vec_elem_start, bool mask_f)
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

        uint64_t imm = static_cast<uint8_t>(u_imm & 0x1F);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        size_t vlmax = emul_num * vec_reg_len_bytes / sew_bytes / emul_denom;

        vd.m_slidedown(vs2, imm, V.get_mask_reg(), !mask_f, vlmax, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slide1up(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                   uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                   uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                   bool mask_f, uint8_t scalar_reg_len_bytes)
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

        if (V.get_mask_reg().get_bit(vec_elem_start))
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

VILL::vpu_return_t VPERM::fslide1up(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                    uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                    uint16_t src_vec_reg_lhs, uint8_t *scalar_fp_reg_mem, uint16_t vec_elem_start,
                                    bool mask_f, uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::slide1down(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                     bool mask_f, uint8_t scalar_reg_len_bytes)
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
        vd[vd.length_ - 1] = imm;
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VPERM::fslide1down(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_fp_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_fp_reg_len_bytes)
{
    // TODO: not implemented, yet.
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
