#include <functional>
#include <cstdint>
#include <cstddef>

#include "arithmetic/fixedpoint.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

// Private function declarations

auto iterate_vector(const SVector &vs2, std::uint64_t scalar, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FIXP::FixpointFunction func, std::size_t start_index, bool signed_op, std::size_t sew,
                    std::uint8_t rounding_mode) -> bool;

auto iterate_vector(const SVector &vs2, std::uint64_t scalar, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FIXP::FixpointFunction func, std::size_t start_index, bool signed_op, std::size_t sew,
                    std::uint8_t rounding_mode) -> bool;

// Private function definitions

auto iterate_vector(const SVector &vs2, const SVector &vs1, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FIXP::FixpointFunction func, std::size_t start_index, bool signed_op, std::size_t sew,
                    std::uint8_t rounding_mode) -> bool
{
    auto sat = false;
    // TODO: If OMP is used, local sat results can be OR reduced to final sat
    for (std::size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            std::uint64_t lhs = signed_op ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            std::uint64_t rhs = signed_op ? vs1[i_element].to_i64() : vs1[i_element].to_u64();
            sat |= func(lhs, rhs, vd[i_element], sew, rounding_mode);
        }
    }
    return sat;
}

auto iterate_vector(const SVector &vs2, std::uint64_t scalar, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FIXP::FixpointFunction func, std::size_t start_index, bool signed_op, std::size_t sew,
                    std::uint8_t rounding_mode) -> bool
{
    auto sat = false;
    for (std::size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            std::uint64_t lhs = signed_op ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            sat |= func(lhs, scalar, vd[i_element], sew, rounding_mode);
        }
    }
    return sat;
}

// Public function definitions

auto VARITH_FIXP::roundoff_unsigned(std::uint64_t value, std::uint8_t rounding_bits, std::uint8_t rounding_mode)
    -> std::uint64_t
{
    // Only lower 2 bits are used
    rounding_mode &= 0b11;

    if (rounding_bits == 0)
    {
        return value;
    }
    auto rounding_increment = false;
    auto range_zero_check = false;
    auto bitmask = 0_u64;

    switch (rounding_mode)
    {
    case 0:
    {
        rounding_increment = static_cast<bool>(value & (1_u64 << (rounding_bits - 1)));
        break;
    }
    case 1:
    {
        // Needs check v[d-2:0] != 0
        if (rounding_bits >= 2)
        {
            // Bitmask for v[d-2 : 0]
            bitmask = (1_u64 << (rounding_bits - 1)) - 1;
            range_zero_check = value & bitmask;
        }
        // v[d-1] & (v[d-2:0] != 0 | v[d])
        bool condition_1 = (value & (1_u64 << (rounding_bits - 1)));
        bool condition_2 = static_cast<bool>(range_zero_check || (value & (1_u64 << rounding_bits)));
        rounding_increment = condition_1 && condition_2;
        break;
    }
    case 2:
    {
        // rounding_increment = 0;
        break;
    }
    case 3:
    {
        // Bitmask for v[d-1 : 0]
        bitmask = (1_u64 << (rounding_bits)) - 1;
        // Needs check v[d-1:0] != 0
        range_zero_check = value & bitmask;
        rounding_increment = !static_cast<bool>(value & (1_u64 << rounding_bits)) && range_zero_check;
        break;
    }
    default:
    {
        // Illegal!
        break;
    }
    }

    return (value >> rounding_bits) + rounding_increment;
}

// TODO: Template
auto VARITH_FIXP::roundoff_signed(std::int64_t value, std::uint8_t rounding_bits, std::uint8_t rounding_mode)
    -> std::int64_t
{
    if (rounding_bits == 0)
    {
        return value;
    }

    // Only lower 2 bits are used
    rounding_mode &= 0b11;
    auto range_zero_check = false;
    auto bitmask = 0_i64;

    auto rounding_increment = false;
    switch (rounding_mode)
    {
    case 0:
    {
        rounding_increment = static_cast<bool>(value & (1_i64 << (rounding_bits - 1)));
        break;
    }
    case 1:
    {
        // Needs check v[d-2:0] != 0
        if (rounding_bits >= 2)
        {
            // Bitmask for v[d-2 : 0]
            bitmask = (1_i64 << (rounding_bits - 1)) - 1;
            range_zero_check = value & bitmask;
        }
        // v[d-1] & (v[d-2:0] != 0 | v[d])
        bool condition_1 = (value & (1_i64 << (rounding_bits - 1)));
        bool condition_2 = static_cast<bool>(range_zero_check || (value & (1_i64 << rounding_bits)));
        rounding_increment = condition_1 && condition_2;
        break;
    }
    case 2:
    {
        // rounding_increment = 0;
        break;
    }
    case 3:
    {
        // Bitmask for v[d-1 : 0]
        bitmask = (1_i64 << (rounding_bits)) - 1;
        // Needs check v[d-1:0] != 0
        range_zero_check = value & bitmask;
        rounding_increment = !static_cast<bool>(value & (1_i64 << rounding_bits)) && range_zero_check;
        break;
    }
    default:
    {
        // Illegal!
        break;
    }
    }

    return (value >> rounding_bits) + rounding_increment;
}

VILL::vpu_return_t VARITH_FIXP::fixp_op_vv(std::uint8_t *vec_reg_mem, const v_instr_info_t &v_instr_info,
                                           const fixedpoint_info_t &fixedpoint_info, std::uint16_t reg_vd,
                                           std::uint16_t reg_vs1, std::uint16_t reg_vs2, FixpointFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.emul_num, v_instr_info.emul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew * 2,
                       SVMul(v_instr_info.emul_num * 2, v_instr_info.emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(reg_vs1))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if ((!fixedpoint_info.narrowing_op && !V.vec_reg_is_aligned(reg_vs2)) ||
        (fixedpoint_info.narrowing_op && !V_wide.vec_reg_is_aligned(reg_vs2)))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    if (fixedpoint_info.narrowing_op)
    {
        V_wide.init();
    }

    RVVector &vs1 = V.get_vec(reg_vs1);
    RVVector &vs2 = fixedpoint_info.narrowing_op ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    RVVector &vd = V.get_vec(reg_vd);

    auto sat = iterate_vector(vs2, vs1, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element,
                              v_instr_info.signed_op, v_instr_info.sew, fixedpoint_info.rounding_mode);

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::fixp_op_vx(std::uint8_t *vec_reg_mem, const v_instr_info_t &v_instr_info,
                                           const fixedpoint_info_t &fixedpoint_info, std::uint16_t reg_vd,
                                           std::uint16_t reg_vs2, std::uint8_t *scalar_reg_mem,
                                           std::uint8_t scalar_register_length, FixpointFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.emul_num, v_instr_info.emul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew * 2,
                       SVMul(v_instr_info.emul_num * 2, v_instr_info.emul_denom), vec_reg_mem);

    if ((!fixedpoint_info.narrowing_op && !V.vec_reg_is_aligned(reg_vs2)) ||
        (fixedpoint_info.narrowing_op && !V_wide.vec_reg_is_aligned(reg_vs2)))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    if (fixedpoint_info.narrowing_op)
    {
        V_wide.init();
    }

    RVVector &vs2 = fixedpoint_info.narrowing_op ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    RVVector &vd = V.get_vec(reg_vd);

    std::uint64_t imm = (scalar_register_length > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                      : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    imm = mask_and_sign_extend_scalar(imm, v_instr_info.sew, v_instr_info.signed_op);

    auto sat = iterate_vector(vs2, imm, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element,
                              v_instr_info.signed_op, v_instr_info.sew, fixedpoint_info.rounding_mode);

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::fixp_op_vi(std::uint8_t *vec_reg_mem, const v_instr_info_t &v_instr_info,
                                           const fixedpoint_info_t &fixedpoint_info, std::uint16_t reg_vd,
                                           std::uint16_t reg_vs2, std::uint8_t imm5, FixpointFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.emul_num, v_instr_info.emul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew * 2,
                       SVMul(v_instr_info.emul_num * 2, v_instr_info.emul_denom), vec_reg_mem);

    if ((!fixedpoint_info.narrowing_op && !V.vec_reg_is_aligned(reg_vs2)) ||
        (fixedpoint_info.narrowing_op && !V_wide.vec_reg_is_aligned(reg_vs2)))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    if (fixedpoint_info.narrowing_op)
    {
        V_wide.init();
    }

    RVVector &vs2 = fixedpoint_info.narrowing_op ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    RVVector &vd = V.get_vec(reg_vd);

    // For instruction with specific uimm, just zero extend, otherwise sign extend
    std::uint64_t imm = v_instr_info.zero_extend_immediate ? zero_extend_immediate(imm5) : sign_extend_immediate(imm5);

    // However, if the instruction is unsigned, we must zero out the upper 64 - SEW bits!
    imm = v_instr_info.signed_op ? imm : imm & get_n_bit_mask(v_instr_info.sew);

    auto sat = iterate_vector(vs2, imm, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.start_element,
                              v_instr_info.signed_op, v_instr_info.sew, fixedpoint_info.rounding_mode);

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

/* 12.1. Vector Single-Width Saturating Add and Subtract */
VILL::vpu_return_t VARITH_FIXP::vsadd_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);
    auto sat = false;

    if (is_signed)
    {
        vd.m_sat_add(vs2, vs1, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        vd.m_sat_addu(vs2, vs1, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vsadd_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t imm, std::uint16_t vec_elem_start,
                                         bool mask_f, bool is_signed)
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
    auto sat = false;

    if (is_signed)
    {
        std::int64_t s_imm = static_cast<std::uint64_t>(imm & 0x10 ? imm | ~0x1F : imm);
        vd.m_sat_add(vs2, s_imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        std::uint64_t u_imm = imm & 0x1F;
        vd.m_sat_addu(vs2, u_imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vsadd_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed,
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

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);
    auto sat = false;

    if (is_signed)
    {
        std::int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::int64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_sat_add(vs2, static_cast<std::int64_t>(imm), V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                        : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_sat_addu(vs2, imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssub_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);
    auto sat = false;

    if (is_signed)
    {
        vd.m_sat_sub(vs2, vs1, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        vd.m_sat_subu(vs2, vs1, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssub_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed,
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

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);
    auto sat = false;

    if (is_signed)
    {
        std::int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::int64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_sat_sub(vs2, static_cast<std::int64_t>(imm), V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                        : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_sat_subu(vs2, imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.1. */

/* 12.2. Vector Single-Width Averaging Add and Subtract */
VILL::vpu_return_t VARITH_FIXP::vaadd_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         std::uint8_t rounding_mode)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    if (is_signed)
    {
        vd.m_avg_add(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    else
    {
        vd.m_avg_addu(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_FIXP::vaadd_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         std::uint8_t scalar_reg_len_bytes, std::uint8_t rounding_mode)
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

    if (is_signed)
    {
        std::int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::int64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_avg_add(vs2, static_cast<std::int64_t>(imm), V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    else
    {
        std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                        : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_avg_addu(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_FIXP::vasub_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         std::uint8_t rounding_mode)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    if (is_signed)
    {
        vd.m_avg_sub(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    else
    {
        vd.m_avg_subu(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_FIXP::vasub_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         std::uint8_t scalar_reg_len_bytes, std::uint8_t rounding_mode)
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

    if (is_signed)
    {
        std::int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::int64_t *>(scalar_reg_mem))
                                                       : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_avg_sub(vs2, static_cast<std::int64_t>(imm), V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    else
    {
        std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                        : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_avg_subu(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 12.2. */

/* 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation */
VILL::vpu_return_t VARITH_FIXP::vsmul_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t rounding_mode)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);
    auto sat = false;

    vd.m_round_sat_mul(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vsmul_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes,
                                         std::uint8_t rounding_mode)
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
    auto sat = false;

    std::int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::int64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<int32_t *>(scalar_reg_mem));

    vd.m_round_sat_mul(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.3. */

/* 12.4. Vector Single-Width Scaling Shift Instructions */
VILL::vpu_return_t VARITH_FIXP::vssrl_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t rounding_mode)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    vd.m_scaling_srl(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssrl_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t imm, std::uint16_t vec_elem_start,
                                         bool mask_f, std::uint8_t rounding_mode)
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

    std::uint64_t u_imm = imm & 0x1F;
    vd.m_scaling_srl(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssrl_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes,
                                         std::uint8_t rounding_mode)
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

    std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                    : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    vd.m_scaling_srl(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssra_vv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t rounding_mode)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    vd.m_scaling_sra(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssra_vi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t imm, std::uint16_t vec_elem_start,
                                         bool mask_f, std::uint8_t rounding_mode)
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

    std::uint64_t u_imm = imm & 0x1F;
    vd.m_scaling_sra(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssra_vx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                         std::uint16_t sew_bytes, std::uint16_t vec_len,
                                         std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                         std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                         std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes,
                                         std::uint8_t rounding_mode)
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

    std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                    : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    vd.m_scaling_sra(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.4. */

/* 12.5. Vector Narrowing Fixed-Point Clip Instructions */
VILL::vpu_return_t VARITH_FIXP::vnclipu_wv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                           std::uint16_t sew_bytes, std::uint16_t vec_len,
                                           std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                           std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                           std::uint16_t vec_elem_start, bool mask_f, std::uint8_t rounding_mode)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);
    bool sat = false;
    vd.m_narrowing_clipu(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclipu_wi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                           std::uint16_t sew_bytes, std::uint16_t vec_len,
                                           std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                           std::uint16_t src_vec_reg_lhs, std::uint8_t imm,
                                           std::uint16_t vec_elem_start, bool mask_f, std::uint8_t rounding_mode)
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

    std::uint64_t u_imm = imm & 0x1F;
    bool sat = false;
    vd.m_narrowing_clipu(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclipu_wx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                           std::uint16_t sew_bytes, std::uint16_t vec_len,
                                           std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                           std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                           std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes,
                                           std::uint8_t rounding_mode)
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

    std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                    : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    bool sat = false;
    vd.m_narrowing_clipu(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclip_wv(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                          std::uint16_t sew_bytes, std::uint16_t vec_len,
                                          std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                          std::uint16_t src_vec_reg_rhs, std::uint16_t src_vec_reg_lhs,
                                          std::uint16_t vec_elem_start, bool mask_f, std::uint8_t rounding_mode)
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
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    bool sat = false;
    vd.m_narrowing_clip(vs2, vs1, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclip_wi(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                          std::uint16_t sew_bytes, std::uint16_t vec_len,
                                          std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                          std::uint16_t src_vec_reg_lhs, std::uint8_t imm, std::uint16_t vec_elem_start,
                                          bool mask_f, std::uint8_t rounding_mode)
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

    std::uint64_t u_imm = imm & 0x1F;
    bool sat = false;
    vd.m_narrowing_clip(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclip_wx(std::uint8_t *vec_reg_mem, std::uint64_t emul_num, std::uint64_t emul_denom,
                                          std::uint16_t sew_bytes, std::uint16_t vec_len,
                                          std::uint16_t vec_reg_len_bytes, std::uint16_t dst_vec_reg,
                                          std::uint16_t src_vec_reg_lhs, std::uint8_t *scalar_reg_mem,
                                          std::uint16_t vec_elem_start, bool mask_f, std::uint8_t scalar_reg_len_bytes,
                                          std::uint8_t rounding_mode)
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

    std::uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<std::uint64_t *>(scalar_reg_mem))
                                                    : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    bool sat = false;
    vd.m_narrowing_clip(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.5. */
/* End 12. */