#include <functional>

#include "misc/reduction.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

// Private function declarations
void iterate_vector(const SVector &opL, const SVector &rhs, SVector &vd, const SVRegister &vm, bool mask,
                    ReductionFunction func, size_t sew, bool is_signed, size_t start_index = 0);

// Private function definitions
void iterate_vector(const SVector &opL, const SVector &rhs, SVector &vd, const SVRegister &vm, bool mask,
                    ReductionFunction func, size_t sew, bool is_signed, size_t start_index)
{
    // func(rhs[start_index], vd[start_index], is_signed, sew);
    vd[start_index] = rhs[start_index];
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(opL[i_element], vd[start_index], is_signed, sew);
        }
    }
}

// Public function definitions
VILL::vpu_return_t VREDUC::red_op(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                          uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs,
                          uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f, ReductionFunction func,
                          bool is_signed, bool wide_dest, bool is_float_instr, uint8_t rounding_mode)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    RVVRegField V_wide(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

    if ((!wide_dest && !V.vec_reg_is_aligned(src_vec_reg_rhs)) ||
        (wide_dest && !V_wide.vec_reg_is_aligned(src_vec_reg_rhs)))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if ((!wide_dest && !V.vec_reg_is_aligned(dst_vec_reg)) || (wide_dest && !V_wide.vec_reg_is_aligned(dst_vec_reg)))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    // TODO: check wide V overlap rules

    V.init();
    if (wide_dest)
    {
        V_wide.init();
    }

    RVVector &vs1 = wide_dest ? V_wide.get_vec(src_vec_reg_rhs) : V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = wide_dest ? V_wide.get_vec(dst_vec_reg) : V.get_vec(dst_vec_reg);

    if (is_float_instr)
    {
        softfloat_exceptionFlags = 0;
        softfloat_roundingMode = rounding_mode;
    }

    iterate_vector(vs2, vs1, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, is_signed, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
