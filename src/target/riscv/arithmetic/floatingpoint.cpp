#include <functional>

#include "arithmetic/floatingpoint.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

#ifdef ETISS_SOFTFLOAT
extern "C"
{
#include "softfloat_orig.h"
}
#else
#include "softfloat.hpp"
#endif

void iterate_vector(const SVector &opL, const SVector &rhs, SVector &vd, const SVRegister &vm, bool mask,
                    FloatFunction func, size_t sew, size_t start_index = 0)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(opL[i_element].to_u64(), rhs[i_element].to_u64(), vd[i_element], sew);
        }
    }
}

void iterate_vector(const SVector &opL, uint64_t rhs, SVector &vd, const SVRegister &vm, bool mask, FloatFunction func,
                    size_t sew, size_t start_index = 0)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(opL[i_element].to_u64(), rhs, vd[i_element], sew);
        }
    }
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                          uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                          uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                          uint16_t vec_elem_start, bool mask_f, FloatFunction func)
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

    iterate_vector(vs2, vs1, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vf(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                          uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                          uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                          uint8_t scalar_reg_len_bytes, uint16_t vec_elem_start, bool mask_f,
                                          FloatFunction func)
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

    uint64_t rhs = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    iterate_vector(vs2, rhs, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}