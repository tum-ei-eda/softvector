#include <functional>

#include "arithmetic/fixedpoint.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

// TODO
// VILL::vpu_return_t VARITH_FIXP::op_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t
// sew_bytes,
//                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
//                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
//                                       bool mask_f, Kernel kernel)
// {
//     RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

//     if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
//     {
//         return (VILL::VPU_RETURN::SRC1_VEC_ILL);
//     }
//     if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
//     {
//         return (VILL::VPU_RETURN::SRC2_VEC_ILL);
//     }
//     if (!V.vec_reg_is_aligned(dst_vec_reg))
//     {
//         return (VILL::VPU_RETURN::DST_VEC_ILL);
//     }

//     V.init();

//     RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
//     RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
//     RVVector &vd = V.get_vec(dst_vec_reg);
//     auto sat = false;

//     return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
// }

/* 12.1. Vector Single-Width Saturating Add and Subtract */
VILL::vpu_return_t VARITH_FIXP::vsadd_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed)
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

VILL::vpu_return_t VARITH_FIXP::vsadd_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t imm,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed)
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
        int64_t s_imm = static_cast<uint64_t>(imm & 0x10 ? imm | ~0x1F : imm);
        vd.m_sat_add(vs2, s_imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        uint64_t u_imm = imm & 0x1F;
        vd.m_sat_addu(vs2, u_imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vsadd_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         uint8_t scalar_reg_len_bytes)
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
        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_sat_add(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_sat_addu(vs2, imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssub_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed)
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

VILL::vpu_return_t VARITH_FIXP::vssub_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         uint8_t scalar_reg_len_bytes)
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
        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_sat_sub(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    else
    {
        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_sat_subu(vs2, imm, V.get_mask_reg(), !mask_f, &sat, vec_elem_start);
    }
    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.1. */

/* 12.2. Vector Single-Width Averaging Add and Subtract */
VILL::vpu_return_t VARITH_FIXP::vaadd_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed, uint8_t rounding_mode)
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

VILL::vpu_return_t VARITH_FIXP::vaadd_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         uint8_t scalar_reg_len_bytes, uint8_t rounding_mode)
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
        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_avg_add(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    else
    {
        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_avg_addu(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_FIXP::vasub_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed, uint8_t rounding_mode)
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

VILL::vpu_return_t VARITH_FIXP::vasub_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, bool is_signed,
                                         uint8_t scalar_reg_len_bytes, uint8_t rounding_mode)
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
        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        vd.m_avg_sub(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    else
    {
        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        vd.m_avg_subu(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 12.2. */

/* 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation */
VILL::vpu_return_t VARITH_FIXP::vsmul_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

VILL::vpu_return_t VARITH_FIXP::vsmul_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes,
                                         uint8_t rounding_mode)
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

    int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                              : *(reinterpret_cast<int32_t *>(scalar_reg_mem));

    vd.m_round_sat_mul(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return sat ? VILL::VPU_RETURN::NO_EXCEPT_FP_SAT : VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.3. */

/* 12.4. Vector Single-Width Scaling Shift Instructions */
VILL::vpu_return_t VARITH_FIXP::vssrl_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

VILL::vpu_return_t VARITH_FIXP::vssrl_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t imm,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

    uint64_t u_imm = imm & 0x1F;
    vd.m_scaling_srl(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssrl_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes,
                                         uint8_t rounding_mode)
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

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    vd.m_scaling_srl(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssra_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

VILL::vpu_return_t VARITH_FIXP::vssra_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t imm,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

    uint64_t u_imm = imm & 0x1F;
    vd.m_scaling_sra(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vssra_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes,
                                         uint8_t rounding_mode)
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

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    vd.m_scaling_sra(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.4. */

/* 12.5. Vector Narrowing Fixed-Point Clip Instructions */
VILL::vpu_return_t VARITH_FIXP::vnclipu_wv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

VILL::vpu_return_t VARITH_FIXP::vnclipu_wi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t imm,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

    uint64_t u_imm = imm & 0x1F;
    bool sat = false;
    vd.m_narrowing_clipu(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclipu_wx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes,
                                         uint8_t rounding_mode)
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

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    bool sat = false;
    vd.m_narrowing_clipu(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclip_wv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

VILL::vpu_return_t VARITH_FIXP::vnclip_wi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t imm,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t rounding_mode)
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

    uint64_t u_imm = imm & 0x1F;
    bool sat = false;
    vd.m_narrowing_clip(vs2, u_imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FIXP::vnclip_wx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes,
                                         uint8_t rounding_mode)
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

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    bool sat = false;
    vd.m_narrowing_clip(vs2, imm, V.get_mask_reg(), !mask_f, rounding_mode, &sat, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
/* End 12.5. */
/* End 12. */