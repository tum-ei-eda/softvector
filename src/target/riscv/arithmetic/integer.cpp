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
/// \file integer.cpp
/// \brief C++ Source for vector integer arithmetic helpers for RISC-V ISS
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#include <cstdint>
#include <cstddef>

#include "arithmetic/integer.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

// Private function declarations

auto iterate_vector(SVector const &vs2, SVector const &vs1, SVector &vd, SVRegister const &vm, bool mask,
                    VARITH_INT::IntFunction func, std::size_t start_index, bool signed_vs2, bool signed_vs1,
                    bool mask_is_data) -> void;

auto iterate_vector(SVector const &vs2, std::uint64_t scalar, SVector &vd, SVRegister const &vm, bool mask,
                    VARITH_INT::IntFunction func, std::size_t start_index, bool signed_vs2, bool mask_is_data) -> void;

auto iterate_vector_to_register(SVector const &vs2, SVector const &vs1, SVRegister &vd, SVRegister const &vm, bool mask,
                                VARITH_INT::IntRegisterFunction func, std::size_t start_index, bool signed_vs2,
                                bool signed_vs1, bool mask_is_data, std::size_t sew) -> void;

auto iterate_vector_to_register(SVector const &vs2, std::uint64_t scalar, SVRegister &vd, SVRegister const &vm,
                                bool mask, VARITH_INT::IntRegisterFunction func, std::size_t start_index,
                                bool signed_vs2, bool mask_is_data, std::size_t sew) -> void;

// Private function definitions

auto iterate_vector(SVector const &vs2, SVector const &vs1, SVector &vd, SVRegister const &vm, bool mask,
                    VARITH_INT::IntFunction func, std::size_t start_index, bool signed_vs2, bool signed_vs1,
                    bool mask_is_data) -> void
{
    for (std::size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        auto mask_bit = vm.get_bit(i_element);
        if (!mask || mask_bit || mask_is_data)
        {
            auto lhs = signed_vs2 ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            auto rhs = signed_vs1 ? vs1[i_element].to_i64() : vs1[i_element].to_u64();
            func(lhs, rhs, vd[i_element], mask && mask_bit && mask_is_data);
        }
    }
}

auto iterate_vector(SVector const &vs2, std::uint64_t scalar, SVector &vd, SVRegister const &vm, bool mask,
                    VARITH_INT::IntFunction func, std::size_t start_index, bool signed_vs2, bool mask_is_data) -> void
{
    for (std::size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        auto mask_bit = vm.get_bit(i_element);
        if (!mask || mask_bit || mask_is_data)
        {
            std::uint64_t lhs = signed_vs2 ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            func(lhs, scalar, vd[i_element], mask && mask_bit && mask_is_data);
        }
    }
}

auto iterate_vector_to_register(SVector const &vs2, SVector const &vs1, SVRegister &vd, SVRegister const &vm, bool mask,
                                VARITH_INT::IntRegisterFunction func, std::size_t start_index, bool signed_vs2,
                                bool signed_vs1, bool mask_is_data, std::size_t sew) -> void
{
    for (std::size_t i_element = start_index; i_element < vs2.length_; ++i_element)
    {
        auto mask_bit = vm.get_bit(i_element);
        if (!mask || mask_bit || mask_is_data)
        {
            auto lhs = signed_vs2 ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            auto rhs = signed_vs1 ? vs1[i_element].to_i64() : vs1[i_element].to_u64();
            auto ret = func(lhs, rhs, sew, mask && mask_bit && mask_is_data);
            ret ? vd.set_bit(i_element) : vd.reset_bit(i_element);
        }
    }
}

auto iterate_vector_to_register(SVector const &vs2, std::uint64_t scalar, SVRegister &vd, SVRegister const &vm,
                                bool mask, VARITH_INT::IntRegisterFunction func, std::size_t start_index,
                                bool signed_vs2, bool mask_is_data, std::size_t sew) -> void
{
    for (std::size_t i_element = start_index; i_element < vs2.length_; ++i_element)
    {
        auto mask_bit = vm.get_bit(i_element);
        if (!mask || mask_bit || mask_is_data)
        {
            auto lhs = signed_vs2 ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            auto ret = func(lhs, scalar, sew, mask && mask_bit && mask_is_data);
            ret ? vd.set_bit(i_element) : vd.reset_bit(i_element);
        }
    }
}

// Public function definitions

VILL::vpu_return_t VARITH_INT::int_op_vv(std::uint8_t *vec_reg_mem, const VInstrInfo &v_instr_info,
                                         const IntInstrInfo &int_instr_info, std::uint16_t reg_vd,
                                         std::uint16_t reg_vs1, std::uint16_t reg_vs2, IntFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(2 * v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto alignment_exception =
        check_alignment(V, V_wide, reg_vd, reg_vs2, reg_vs1, v_instr_info.wide_vd, v_instr_info.wide_vs2);
    if (alignment_exception != VILL::vpu_return_t::NO_EXCEPT)
    {
        return alignment_exception;
    }

    V.init();
    if (v_instr_info.wide_vd || v_instr_info.wide_vs2)
    {
        V_wide.init();
    }

    RVVector &vs1 = V.get_vec(reg_vs1);
    RVVector &vs2 = v_instr_info.wide_vs2 ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    RVVector &vd = v_instr_info.wide_vd ? V_wide.get_vec(reg_vd) : V.get_vec(reg_vd);

    // Mixed-signed: vs2 is signed if it is a signed-unsigned instruction
    auto signed_vs2 = int_instr_info.mixed_signed ? int_instr_info.mixed_signed_vs2_signed : v_instr_info.signed_op;
    // Mixed-signed: vs1 is signed if vs2 is unsigned and vice versa
    auto signed_vs1 = int_instr_info.mixed_signed ? !signed_vs2 : v_instr_info.signed_op;

    iterate_vector(vs2, vs1, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element, signed_vs2,
                   signed_vs1, int_instr_info.mask_is_data);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::int_op_vi(std::uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                         IntInstrInfo const &int_instr_info, std::uint16_t reg_vd,
                                         std::uint16_t reg_vs2, std::uint8_t imm5, IntFunction func)
{
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

    // For instructions with specific uimm, just zero extend, otherwise sign extend
    std::uint64_t imm = v_instr_info.zero_extend_immediate ? zero_extend_immediate(imm5) : sign_extend_immediate(imm5);

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = V.get_vec(reg_vd);

    iterate_vector(vs2, imm, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element,
                   v_instr_info.signed_op, int_instr_info.mask_is_data);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::int_op_vx(std::uint8_t *vec_reg_mem, const VInstrInfo &v_instr_info,
                                         const IntInstrInfo &int_instr_info, std::uint16_t reg_vd,
                                         std::uint16_t reg_vs2, std::uint8_t *scalar_reg_mem,
                                         std::uint8_t scalar_reg_len_bytes, IntFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(2 * v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto alignment_exception = check_alignment(V, V_wide, reg_vd, reg_vs2, v_instr_info.wide_vd, v_instr_info.wide_vs2);
    if (alignment_exception != VILL::vpu_return_t::NO_EXCEPT)
    {
        return alignment_exception;
    }

    V.init();
    if (v_instr_info.wide_vd || v_instr_info.wide_vs2)
    {
        V_wide.init();
    }

    std::uint64_t scalar = (scalar_reg_len_bytes > xlen_32_bytes)
                               ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                               : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));

    // Mixed-signed: vs2 is signed if it is a signed-unsigned instruction
    auto signed_vs2 = int_instr_info.mixed_signed ? int_instr_info.mixed_signed_vs2_signed : v_instr_info.signed_op;
    // Mixed-signed: Scalar is signed if vs2 is unsigned and vice versa
    auto signed_scalar = int_instr_info.mixed_signed ? !signed_vs2 : v_instr_info.signed_op;

    scalar = mask_and_sign_extend_scalar(scalar, v_instr_info.sew, signed_scalar);

    RVVector &vs2 = v_instr_info.wide_vs2 ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    RVVector &vd = v_instr_info.wide_vd ? V_wide.get_vec(reg_vd) : V.get_vec(reg_vd);

    iterate_vector(vs2, scalar, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element, signed_vs2,
                   int_instr_info.mask_is_data);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

auto VARITH_INT::int_op_vv_to_register(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                       IntInstrInfo const &int_instr_info, uint16_t reg_vd, uint16_t reg_vs1,
                                       uint16_t reg_vs2, IntRegisterFunction func) -> VILL::vpu_return_t
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(2 * v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto alignment_exception =
        check_alignment(V, V_wide, reg_vd, reg_vs2, reg_vs1, v_instr_info.wide_vd, v_instr_info.wide_vs2);
    if (alignment_exception != VILL::vpu_return_t::NO_EXCEPT)
    {
        return alignment_exception;
    }

    V.init();
    if (v_instr_info.wide_vd || v_instr_info.wide_vs2)
    {
        V_wide.init();
    }

    RVVector &vs1 = V.get_vec(reg_vs1);
    RVVector &vs2 = v_instr_info.wide_vs2 ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    SVRegister &vd = v_instr_info.wide_vd ? V_wide.get_vecreg(reg_vd) : V.get_vecreg(reg_vd);

    // Mixed-signed: vs2 is signed if it is a signed-unsigned instruction
    auto signed_vs2 = int_instr_info.mixed_signed ? int_instr_info.mixed_signed_vs2_signed : v_instr_info.signed_op;
    // Mixed-signed: vs1 is signed if vs2 is unsigned and vice versa
    auto signed_vs1 = int_instr_info.mixed_signed ? !signed_vs2 : v_instr_info.signed_op;

    iterate_vector_to_register(vs2, vs1, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element,
                               signed_vs2, signed_vs1, int_instr_info.mask_is_data, v_instr_info.sew);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

auto VARITH_INT::int_op_vi_to_register(std::uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                       IntInstrInfo const &int_instr_info, std::uint16_t reg_vd, std::uint16_t reg_vs2,
                                       std::uint8_t imm5, IntRegisterFunction func) -> VILL::vpu_return_t
{
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

    std::uint64_t imm = v_instr_info.zero_extend_immediate ? zero_extend_immediate(imm5) : sign_extend_immediate(imm5);

    RVVector &vs2 = V.get_vec(reg_vs2);
    SVRegister &vd = V.get_vecreg(reg_vd);

    iterate_vector_to_register(vs2, imm, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element,
                               v_instr_info.signed_op, int_instr_info.mask_is_data, v_instr_info.sew);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

auto VARITH_INT::int_op_vx_to_register(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                       VARITH_INT::IntInstrInfo const &int_instr_info, uint16_t reg_vd,
                                       uint16_t reg_vs2, uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes,
                                       VARITH_INT::IntRegisterFunction func) -> VILL::vpu_return_t
{
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

    std::uint64_t imm = (scalar_reg_len_bytes > xlen_32_bytes) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                               : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
    imm = mask_and_sign_extend_scalar(imm, v_instr_info.sew, v_instr_info.signed_op);
    RVVector &vs2 = V.get_vec(reg_vs2);
    SVRegister &vd = V.get_vecreg(reg_vd);

    iterate_vector_to_register(vs2, imm, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element,
                               v_instr_info.signed_op, int_instr_info.mask_is_data, v_instr_info.sew);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f,
                                      bool dir_f, bool signed_f)
{
    // TODO: Remove nesting for returning branches
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {

        RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

        if (!VD.vec_reg_is_aligned(dst_vec_reg))
        {
            return (VILL::VPU_RETURN::DST_VEC_ILL);
        }

        V.init();
        VD.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = VD.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
        else if (vd.check_mem_overlap(vs1))
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                      std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                      bool dir_f, bool signed_f, std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {

        RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

        if (!VD.vec_reg_is_aligned(dst_vec_reg))
        {
            return (VILL::VPU_RETURN::DST_VEC_ILL);
        }

        V.init();
        VD.init();

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        imm = mask_and_sign_extend_scalar(imm, sew_bytes * 8, signed_f);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = VD.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_wv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f,
                                      bool dir_f, bool signed_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else
    {

        RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

        if (!VD.vec_reg_is_aligned(dst_vec_reg))
        {
            return (VILL::VPU_RETURN::DST_VEC_ILL);
        }
        else if (!VD.vec_reg_is_aligned(src_vec_reg_lhs))
        {
            return (VILL::VPU_RETURN::SRC2_VEC_ILL);
        }

        V.init();
        VD.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = VD.get_vec(src_vec_reg_lhs);
        RVVector &vd = VD.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
        else if (vd.check_mem_overlap(vs1))
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_wx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                      std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                      bool dir_f, bool signed_f, std::uint8_t scalar_reg_len_bytes)
{

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {

        V.init();

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.2. */

/* 11.3. Vector Integer Extension */
VILL::vpu_return_t VARITH_INT::vext_vf(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                       std::uint16_t extension_encoding, std::uint16_t vec_elem_start, bool mask_f)
{
    int divider = 0;
    bool sign = extension_encoding & 1;
    extension_encoding >>= 1;
    if (extension_encoding == 1)
    {
        divider = 8;
    }
    else if (extension_encoding == 2)
    {
        divider = 4;
    }
    else
    {
        divider = 2;
    }

    // Source EEW: 1/8, 1/4, or 1/2 of SEW
    // Source EMUL: (EEW/SEW)*LMUL
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, (sew_bytes * 8) / divider, SVMul(emul_num, emul_denom * divider),
                  vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW (as is for vsext, vzext) if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom; // Destination EMUL = LMUL
    int n_allowed_overlaps = lmul / divider;
    int lowest_allowed_register = dst_vec_reg + lmul - n_allowed_overlaps;
    if ((emul_num < emul_denom * divider || src_vec_reg_lhs < lowest_allowed_register) &&
        vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
    }
    vd.m_vext(vs2, V.get_mask_reg(), !mask_f, sign, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.3. */

VILL::vpu_return_t VARITH_INT::and_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_and(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::and_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_and(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::and_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_and(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::or_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                     std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_or(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::or_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_or(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::or_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_or(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::xor_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_xor(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::xor_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_xor(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::xor_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_xor(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.6. Vector Single-Width Shift Instructions */
VILL::vpu_return_t VARITH_INT::sll_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sll(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sll_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sll(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sll_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sll(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::srl_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_srl(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::srl_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_srl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::srl_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_srl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sra_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                      std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                      std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                      std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sra(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sra_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sra_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.6. */

/* 11.7. Vector Narrowing Integer Right Shift Instructions */
VILL::vpu_return_t VARITH_INT::vnsrl_wv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    // vd and vs1: EEW = SEW, EMUL = LMUL
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    // vs2: EEW = 2*SEW, EMUL = 2*LMUL
    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();
    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsrl.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsrl(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsrl_wi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t u_imm,
                                        std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    std::uint64_t imm = u_imm & 0x1F;
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsrl.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsrl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsrl_wx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                        std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                        std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsrl.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsrl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsra_wv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    // vd and vs1: EEW = SEW, EMUL = LMUL
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    // vs2: EEW = 2*SEW, EMUL = 2*LMUL
    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();
    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsra.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsra(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsra_wi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t u_imm,
                                        std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    std::uint64_t imm = u_imm & 0x1F;
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsra.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsra_wx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                        std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                        std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsra.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.7. */

VILL::vpu_return_t VARITH_INT::mseq_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_eq(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mseq_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_eq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mseq_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_eq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msne_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();
        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_neq(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msne_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        ;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_neq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msne_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_neq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msltu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msltu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mslt_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mslt_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msleu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lte(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msleu_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msleu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msle_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lte(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msle_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msle_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);
        SVRegister v(vs2 <= imm);

        vd.m_s_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgtu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_gt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgtu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgtu_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgt_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_gt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgt_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgt_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.10. Vector Single-Width Integer Multiply Instructions */
VILL::vpu_return_t VARITH_INT::vmul_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmul_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmul(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmulh_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmulh(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
VILL::vpu_return_t VARITH_INT::vmulh_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmulh(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmulhu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumulh(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
VILL::vpu_return_t VARITH_INT::vmulhu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes)
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumulh(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmulhsu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                          std::uint16_t sew_bytes, std::uint16_t vec_len,
                                          std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                          std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                          std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sumulh(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
VILL::vpu_return_t VARITH_INT::vmulhsu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                          std::uint16_t sew_bytes, std::uint16_t vec_len,
                                          std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                          std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                          std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes)
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sumulh(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.11. Vector Single-Width Integer Divide Instructions */
VILL::vpu_return_t VARITH_INT::vdiv_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssdiv(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vdiv_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssdiv(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vdivu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uudiv(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vdivu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uudiv(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vrem_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssrem(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vrem_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                             : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
    imm = mask_and_sign_extend_scalar(imm, sew_bytes * 8, true);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    vd.m_ssrem(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vremu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uurem(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vremu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uurem(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.11. */

/* 11.12. Vector Widening Integer Multiply Instructions */
/* 2*SEW product from SEW*SEW */

// enum class Widening_Mul_Type
VILL::vpu_return_t VARITH_INT::vwmul_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f,
                                        VWMUL_TYPE vwmul_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs1) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmul_type)
    {
    case VWMUL_TYPE::S_S:
        vd.m_ssmul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::U_U:
        vd.m_uumul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::S_U:
        vd.m_sumul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vwmul_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                        std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start, bool mask_f,
                                        std::uint8_t scalar_reg_len_bytes, VWMUL_TYPE vwmul_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmul_type)
    {
    case VWMUL_TYPE::S_S:
        vd.m_ssmul(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::U_U:
        vd.m_uumul(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::S_U:
        vd.m_sumul(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.12. */

/* Regular vector min and max instructions */
VILL::vpu_return_t VARITH_INT::vmax_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmax(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmax_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmax(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmaxu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumax(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmaxu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumax(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmin_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                       std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                       std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                       std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmin(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmin_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmin(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vminu_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumin(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vminu_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumin(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.16. Vector Integer Move Instructions */
VILL::vpu_return_t VARITH_INT::mv_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg, std::uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_assign(vs1, V.get_mask_reg(), false, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mv_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint8_t *scalar_reg_mem,
                                     std::uint16_t vec_elem_start, std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_assign(imm, V.get_mask_reg(), false, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mv_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                     std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                     std::uint16_t dst_vec_reg, std::uint8_t s_imm, std::uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_assign(imm, V.get_mask_reg(), false, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.16. */

/* 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions */
VILL::vpu_return_t VARITH_INT::vadc_vvm(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_adc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vadc_vim(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
                                        std::uint16_t vec_elem_start)
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_adc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vadc_vxm(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                        std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_adc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadc_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_madc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadc_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
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

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_madc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadc_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_madc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vsbc_vvm(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sbc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vsbc_vxm(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_lhs,
                                        std::uint8_t *scalar_reg_mem, std::uint16_t vec_elem_start,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sbc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmsbc_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_msbc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmsbc_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_msbc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.4 */

/* 11.13. Vector Single-Width Integer Multiply-Add Instructions */
VILL::vpu_return_t VARITH_INT::vmacc_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmacc_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmacc(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsac_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsac(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsac_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes)
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsac(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadd_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                        std::uint16_t sew_bytes, std::uint16_t vec_len, std::uint16_t vec_reg_len_bytes,
                                        std::uint16_t dst_vec_reg, std::uint16_t src_vec_reg_rhs,
                                        std::uint16_t src_vec_reg_lhs, std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_madd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadd_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_madd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsub_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsub_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes)
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

        int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.13. */

/* 11.14. Vector Widening Integer Multiply-Add Instructions */
VILL::vpu_return_t VARITH_INT::vwmacc_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, VWMACC_TYPE vwmacc_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs1) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmacc_type)
    {
    case VWMACC_TYPE::S_S:
        vd.m_ssmacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_U:
        vd.m_uumacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::S_U:
        vd.m_sumacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_S:
        vd.m_sumacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vwmacc_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes,
                                         VWMACC_TYPE vwmacc_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    std::uint64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<std::uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmacc_type)
    {
    case VWMACC_TYPE::S_S:
        vd.m_ssmacc(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_U:
        vd.m_uumacc(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::S_U:
        vd.m_sumacc(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_S:
        vd.m_usmacc(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.14. */

/* 11.15. Vector Integer Merge Instructions */
VILL::vpu_return_t VARITH_INT::vmerge_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
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

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    vd.m_merge(vs2, vs1, V.get_mask_reg(), vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmerge_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, std::uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
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

    int64_t imm = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                             : *(reinterpret_cast<int32_t *>(scalar_reg_mem));

    vd.m_merge(vs2, imm, V.get_mask_reg(), vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmerge_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t s_imm,
                                         std::uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

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

    int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
    RVVector &vd = V.get_vec(dst_vec_reg);

    vd.m_merge(vs2, imm, V.get_mask_reg(), vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.15. */