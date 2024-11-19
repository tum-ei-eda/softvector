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

// Private function declarations
void iterate_vector(const SVector &opL, const SVector &rhs, SVector &vd, const SVRegister &vm, bool mask,
                    FloatFunction func, size_t sew, size_t start_index = 0);

void iterate_vector(const SVector &opL, uint64_t rhs, SVector &vd, const SVRegister &vm, bool mask, FloatFunction func,
                    size_t sew, size_t start_index = 0);

void iterate_vector_register(const SVector &opL, const SVector &rhs, SVRegister &vd, const SVRegister &vm, bool mask,
                             FloatFunction func, size_t sew, size_t start_index = 0);

void iterate_vector_register(const SVector &opL, uint64_t rhs, SVRegister &vd, const SVRegister &vm, bool mask,
                             FloatFunction func, size_t sew, size_t start_index = 0);

void iterate_vector_unary(const SVector &opL, SVector &vd, const SVRegister &vm, bool mask, FloatFunction func,
                          size_t sew, size_t start_index = 0);

void iterate_vector_merge(const SVector &opL, uint64_t rhs, SVector &vd, const SVRegister &vm, size_t sew,
                          size_t start_index = 0);

void iterate_vector_move(uint64_t rhs, SVector &vd, const SVRegister &vm, size_t sew, size_t start_index = 0);

void iterate_vector_convert(const SVector &opL, SVector &vd, const SVRegister &vm, bool mask,
                            FloatConversionFunction func, size_t sew, bool signed_x, bool rtz, bool rod,
                            bool vs2_is_int, size_t start_index = 0);

// Private function definitions
void iterate_vector(const SVector &opL, const SVector &rhs, SVector &vd, const SVRegister &vm, bool mask,
                    FloatFunction func, size_t sew, size_t start_index)
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
                    size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(opL[i_element].to_u64(), rhs, vd[i_element], sew);
        }
    }
}

void iterate_vector_register(const SVector &opL, const SVector &rhs, SVRegister &vd, const SVRegister &vm, bool mask,
                             FloatFunction func, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < opL.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto ret = func(opL[i_element].to_u64(), rhs[i_element].to_u64(), opL[i_element] /* Dummy */, sew);
            ret ? vd.set_bit(i_element) : vd.reset_bit(i_element);
        }
    }
}

void iterate_vector_register(const SVector &opL, uint64_t rhs, SVRegister &vd, const SVRegister &vm, bool mask,
                             FloatFunction func, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < opL.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto ret = func(opL[i_element].to_u64(), rhs, opL[i_element] /* Dummy */, sew);
            ret ? vd.set_bit(i_element) : vd.reset_bit(i_element);
        }
    }
}

void iterate_vector_unary(const SVector &opL, SVector &vd, const SVRegister &vm, bool mask, FloatFunction func,
                          size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(opL[i_element].to_u64(), 0 /* Not needed */, vd[i_element], sew);
        }
    }
}

void iterate_vector_merge(const SVector &opL, uint64_t rhs, SVector &vd, const SVRegister &vm, size_t sew,
                          size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        // 0: use vs2[i], f[rs1] otherwise
        vd[i_element] = vm.get_bit(i_element) ? rhs : opL[i_element].to_u64();
    }
}

void iterate_vector_move(uint64_t rhs, SVector &vd, const SVRegister &vm, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        vd[i_element] = rhs;
    }
}

void iterate_vector_convert(const SVector &opL, SVector &vd, const SVRegister &vm, bool mask,
                            FloatConversionFunction func, size_t sew, bool signed_x, bool rtz, bool rod,
                            bool vs2_is_int, size_t start_index)
{

    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            uint64_t opL_value = vs2_is_int && signed_x ? opL[i_element].to_i64() : opL[i_element].to_u64();
            func(opL_value, vd[i_element], sew, signed_x, rtz, rod);
        }
    }
}

// Public function definitions
VILL::vpu_return_t VARITH_FLOAT::vf_op_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                          uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                          uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                          uint16_t vec_elem_start, bool mask_f, FloatFunction func,
                                          uint8_t rounding_mode, bool wide_dest, bool wide_vs2)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    RVVRegField V_wide(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if ((!wide_vs2 && !V.vec_reg_is_aligned(src_vec_reg_lhs)) ||
        (wide_vs2 && !V_wide.vec_reg_is_aligned(src_vec_reg_lhs)))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if ((!wide_dest && !V.vec_reg_is_aligned(dst_vec_reg)) || (wide_dest && !V_wide.vec_reg_is_aligned(dst_vec_reg)))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    // TODO: check wide V overlap rules

    V.init();
    if (wide_dest || wide_vs2)
    {
        V_wide.init();
    }

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = wide_vs2 ? V_wide.get_vec(src_vec_reg_lhs) : V.get_vec(src_vec_reg_lhs);
    RVVector &vd = wide_dest ? V_wide.get_vec(dst_vec_reg) : V.get_vec(dst_vec_reg);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector(vs2, vs1, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vf(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                          uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                          uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                          uint8_t scalar_reg_len_bytes, uint16_t vec_elem_start, bool mask_f,
                                          FloatFunction func, uint8_t rounding_mode, bool wide_dest, bool wide_vs2)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    RVVRegField V_wide(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

    if ((!wide_vs2 && !V.vec_reg_is_aligned(src_vec_reg_lhs)) ||
        (wide_vs2 && !V_wide.vec_reg_is_aligned(src_vec_reg_lhs)))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if ((!wide_dest && !V.vec_reg_is_aligned(dst_vec_reg)) || (wide_dest && !V_wide.vec_reg_is_aligned(dst_vec_reg)))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    // TODO: check wide V overlap rules

    V.init();
    if (wide_dest || wide_vs2)
    {
        V_wide.init();
    }

    RVVector &vs2 = wide_vs2 ? V_wide.get_vec(src_vec_reg_lhs) : V.get_vec(src_vec_reg_lhs);
    RVVector &vd = wide_dest ? V_wide.get_vec(dst_vec_reg) : V.get_vec(dst_vec_reg);

    uint64_t rhs = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                              : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > sew_bytes)
    {
        rhs = (sew_bytes == 2) ? check_and_unbox_f16(f64(rhs)).v : check_and_unbox_f32(f64(rhs)).v;
    }

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector(vs2, rhs, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_unary(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                             uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                             uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                             bool mask_f, FloatFunction func, uint8_t rounding_mode)
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

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector_unary(vs2, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vv_to_reg(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                                 uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                                 uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs,
                                                 uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f,
                                                 FloatFunction func, uint8_t rounding_mode)
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
    SVRegister &vd = V.get_vecreg(dst_vec_reg);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector_register(vs2, vs1, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vf_to_reg(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                                 uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                                 uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs,
                                                 uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes,
                                                 uint16_t vec_elem_start, bool mask_f, FloatFunction func,
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
    SVRegister &vd = V.get_vecreg(dst_vec_reg);

    uint64_t rhs = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                              : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > sew_bytes)
    {
        rhs = (sew_bytes == 2) ? check_and_unbox_f16(f64(rhs)).v : check_and_unbox_f32(f64(rhs)).v;
    }

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector_register(vs2, rhs, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_merge(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                          uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                          uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                          uint8_t scalar_reg_len_bytes, uint16_t vec_elem_start)
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

    uint64_t rhs = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                              : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > sew_bytes)
    {
        rhs = (sew_bytes == 2) ? check_and_unbox_f16(f64(rhs)).v : check_and_unbox_f32(f64(rhs)).v;
    }

    iterate_vector_merge(vs2, rhs, vd, V.get_mask_reg(), sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_move(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes,
                                         uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vd = V.get_vec(dst_vec_reg);

    uint64_t rhs = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                              : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > sew_bytes)
    {
        rhs = (sew_bytes == 2) ? check_and_unbox_f16(f64(rhs)).v : check_and_unbox_f32(f64(rhs)).v;
    }

    iterate_vector_move(rhs, vd, sew_bytes * 8, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_convert(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                            uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                            uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                            bool mask_f, FloatConversionFunction func, uint8_t rounding_mode,
                                            bool signed_x, bool vs2_is_int, bool rtz)
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

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector_convert(vs2, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, signed_x, rtz, /*rod = */ false,
                           vs2_is_int, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_convert_wide(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                                 uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                                 uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs,
                                                 uint16_t vec_elem_start, bool mask_f, FloatConversionFunction func,
                                                 uint8_t rounding_mode, bool signed_x, bool vs2_is_int, bool rtz)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    RVVRegField V_wide(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V_wide.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    V_wide.init();

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V_wide.get_vec(dst_vec_reg);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector_convert(vs2, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, signed_x, rtz, /*rod = */ false,
                           vs2_is_int, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_convert_narrow(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                                   uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                                   uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs,
                                                   uint16_t vec_elem_start, bool mask_f, FloatConversionFunction func,
                                                   uint8_t rounding_mode, bool signed_x, bool vs2_is_int, bool rtz,
                                                   bool rod)
{
    // TODO: check
    RVVRegField V_narrow(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V_narrow.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    V_narrow.init();

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V_narrow.get_vec(dst_vec_reg);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    iterate_vector_convert(vs2, vd, V.get_mask_reg(), !mask_f, func, sew_bytes * 8, signed_x, rtz, rod, vs2_is_int,
                           vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
