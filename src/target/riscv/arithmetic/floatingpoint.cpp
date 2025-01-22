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

inline auto check_alignment(const RVVRegField &V, const RVVRegField &V_wide, std::uint16_t reg_vd,
                            std::uint16_t reg_vs2, std::uint16_t reg_vs1, bool wide_vd, bool wide_vs2, bool wide_vs1)
    -> VILL::vpu_return_t;

inline auto check_alignment(const RVVRegField &V, const RVVRegField &V_wide, std::uint16_t reg_vd,
                            std::uint16_t reg_vs2, bool wide_vd, bool wide_vs2) -> VILL::vpu_return_t;

void iterate_vector(const SVector &vs2, const SVector &vs1, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index);

void iterate_vector(const SVector &vs2, uint64_t scalar, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index);

void iterate_vector_register(const SVector &vs2, const SVector &vs1, SVRegister &vd, const SVRegister &vm, bool mask,
                             VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index);

void iterate_vector_register(const SVector &vs2, uint64_t scalar, SVRegister &vd, const SVRegister &vm, bool mask,
                             VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index);

void iterate_vector_unary(const SVector &vs2, SVector &vd, const SVRegister &vm, bool mask,
                          VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index);

void iterate_vector_merge(const SVector &vs2, uint64_t scalar, SVector &vd, const SVRegister &vm, size_t sew,
                          size_t start_index);

void iterate_vector_move(uint64_t scalar, SVector &vd, const SVRegister &vm, size_t sew, size_t start_index);

void iterate_vector_convert(const SVector &vs2, SVector &vd, const SVRegister &vm, bool mask,
                            VARITH_FLOAT::FloatConversionFunction func, size_t sew, bool signed_x, bool rtz, bool rod,
                            bool vs2_is_int, size_t start_index);

// Private function definitions

inline auto check_alignment(const RVVRegField &V, const RVVRegField &V_wide, std::uint16_t reg_vd,
                            std::uint16_t reg_vs2, std::uint16_t reg_vs1, bool wide_vd, bool wide_vs2)
    -> VILL::vpu_return_t
{

    if (!V.vec_reg_is_aligned(reg_vs1))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if ((!wide_vs2 && !V.vec_reg_is_aligned(reg_vs2)) || (wide_vs2 && !V_wide.vec_reg_is_aligned(reg_vs2)))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if ((!wide_vd && !V.vec_reg_is_aligned(reg_vd)) || (wide_vd && !V_wide.vec_reg_is_aligned(reg_vd)))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    return VILL::VPU_RETURN::NO_EXCEPT;
}

inline auto check_alignment(const RVVRegField &V, const RVVRegField &V_wide, std::uint16_t reg_vd,
                            std::uint16_t reg_vs2, bool wide_vd, bool wide_vs2) -> VILL::vpu_return_t
{
    if ((!wide_vs2 && !V.vec_reg_is_aligned(reg_vs2)) || (wide_vs2 && !V_wide.vec_reg_is_aligned(reg_vs2)))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if ((!wide_vd && !V.vec_reg_is_aligned(reg_vd)) || (wide_vd && !V_wide.vec_reg_is_aligned(reg_vd)))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    return VILL::VPU_RETURN::NO_EXCEPT;
}

void iterate_vector(const SVector &vs2, const SVector &vs1, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(vs2[i_element].to_u64(), vs1[i_element].to_u64(), vd[i_element], sew);
        }
    }
}

void iterate_vector(const SVector &vs2, uint64_t scalar, SVector &vd, const SVRegister &vm, bool mask,
                    VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(vs2[i_element].to_u64(), scalar, vd[i_element], sew);
        }
    }
}

void iterate_vector_register(const SVector &vs2, const SVector &vs1, SVRegister &vd, const SVRegister &vm, bool mask,
                             VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vs2.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto ret = func(vs2[i_element].to_u64(), vs1[i_element].to_u64(), vs2[i_element] /* Dummy */, sew);
            ret ? vd.set_bit(i_element) : vd.reset_bit(i_element);
        }
    }
}

void iterate_vector_register(const SVector &vs2, uint64_t scalar, SVRegister &vd, const SVRegister &vm, bool mask,
                             VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vs2.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto ret = func(vs2[i_element].to_u64(), scalar, vs2[i_element] /* Dummy */, sew);
            ret ? vd.set_bit(i_element) : vd.reset_bit(i_element);
        }
    }
}

void iterate_vector_unary(const SVector &vs2, SVector &vd, const SVRegister &vm, bool mask,
                          VARITH_FLOAT::FloatFunction func, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            func(vs2[i_element].to_u64(), 0 /* Not needed */, vd[i_element], sew);
        }
    }
}

void iterate_vector_merge(const SVector &vs2, uint64_t scalar, SVector &vd, const SVRegister &vm, size_t sew,
                          size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        // 0: use vs2[i], f[rs1] otherwise
        vd[i_element] = vm.get_bit(i_element) ? scalar : vs2[i_element].to_u64();
    }
}

void iterate_vector_move(uint64_t scalar, SVector &vd, const SVRegister &vm, size_t sew, size_t start_index)
{
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        vd[i_element] = scalar;
    }
}

void iterate_vector_convert(const SVector &vs2, SVector &vd, const SVRegister &vm, bool mask,
                            VARITH_FLOAT::FloatConversionFunction func, size_t sew, bool signed_x, bool rtz, bool rod,
                            bool vs2_is_int, size_t start_index)
{

    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            uint64_t vs2_value = vs2_is_int && signed_x ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            func(vs2_value, vd[i_element], sew, signed_x, rtz, rod);
        }
    }
}

// Public function definitions
auto VARITH_FLOAT::vf_op_vv(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                            FloatInstrInfo const &float_instr_info, uint16_t reg_vd, uint16_t reg_vs1, uint16_t reg_vs2,
                            VARITH_FLOAT::FloatFunction func) -> VILL::vpu_return_t
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);
    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(2 * v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    const auto alignment_exception =
        check_alignment(V, V_wide, reg_vd, reg_vs2, reg_vs1, v_instr_info.wide_vd, v_instr_info.wide_vs2);

    if (alignment_exception != VILL::VPU_RETURN::NO_EXCEPT)
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

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector(vs2, vs1, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                   v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vf(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                          FloatInstrInfo const &float_instr_info, uint16_t reg_vd, uint16_t reg_vs2,
                                          uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes, FloatFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(2 * v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    const auto alignment_exception =
        check_alignment(V, V_wide, reg_vd, reg_vs2, v_instr_info.wide_vd, v_instr_info.wide_vs2);

    if (alignment_exception != VILL::VPU_RETURN::NO_EXCEPT)
    {
        return alignment_exception;
    }

    // TODO: check wide V overlap rules

    V.init();
    if (v_instr_info.wide_vd || v_instr_info.wide_vs2)
    {
        V_wide.init();
    }

    RVVector &vs2 = v_instr_info.wide_vs2 ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    RVVector &vd = v_instr_info.wide_vd ? V_wide.get_vec(reg_vd) : V.get_vec(reg_vd);

    uint64_t scalar = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > (v_instr_info.sew >> 3))
    {
        scalar =
            ((v_instr_info.sew >> 3) == 2) ? check_and_unbox_f16(f64(scalar)).v : check_and_unbox_f32(f64(scalar)).v;
    }

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector(vs2, scalar, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                   v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_unary(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                             FloatInstrInfo const &float_instr_info, uint16_t reg_vd, uint16_t reg_vs2,
                                             FloatFunction func)
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

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = V.get_vec(reg_vd);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector_unary(vs2, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                         v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vv_to_reg(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                                 FloatInstrInfo const &float_instr_info, uint16_t reg_vd,
                                                 uint16_t reg_vs1, uint16_t reg_vs2, FloatFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(reg_vs1))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(reg_vs2))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(reg_vs1);
    RVVector &vs2 = V.get_vec(reg_vs2);
    SVRegister &vd = V.get_vecreg(reg_vd);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector_register(vs2, vs1, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                            v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_op_vf_to_reg(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                                 FloatInstrInfo const &float_instr_info, uint16_t reg_vd,
                                                 uint16_t reg_vs2, uint8_t *scalar_reg_mem,
                                                 uint8_t scalar_reg_len_bytes, FloatFunction func)
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

    RVVector &vs2 = V.get_vec(reg_vs2);
    SVRegister &vd = V.get_vecreg(reg_vd);

    uint64_t scalar = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > (v_instr_info.sew >> 3))
    {
        scalar =
            ((v_instr_info.sew >> 3) == 2) ? check_and_unbox_f16(f64(scalar)).v : check_and_unbox_f32(f64(scalar)).v;
    }

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector_register(vs2, scalar, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                            v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_merge(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info, uint16_t reg_vd,
                                          uint16_t reg_vs2, uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes)
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

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = V.get_vec(reg_vd);

    uint64_t scalar = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > (v_instr_info.sew >> 3))
    {
        scalar =
            ((v_instr_info.sew >> 3) == 2) ? check_and_unbox_f16(f64(scalar)).v : check_and_unbox_f32(f64(scalar)).v;
    }

    iterate_vector_merge(vs2, scalar, vd, V.get_mask_reg(), v_instr_info.sew, v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_move(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info, uint16_t reg_vd,
                                         uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vd = V.get_vec(reg_vd);

    uint64_t scalar = (scalar_reg_len_bytes > 4) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                 : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));

    if (scalar_reg_len_bytes > (v_instr_info.sew >> 3))
    {
        scalar =
            ((v_instr_info.sew >> 3) == 2) ? check_and_unbox_f16(f64(scalar)).v : check_and_unbox_f32(f64(scalar)).v;
    }

    iterate_vector_move(scalar, vd, V.get_mask_reg(), v_instr_info.sew, v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_convert(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                            FloatInstrInfo const &float_instr_info, uint16_t reg_vd, uint16_t reg_vs2,
                                            FloatConversionFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                       SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    const auto alignment_exception =
        check_alignment(V, V_wide, reg_vd, reg_vs2, v_instr_info.wide_vd, v_instr_info.wide_vs2);

    if (alignment_exception != VILL::vpu_return_t::NO_EXCEPT)
    {
        return alignment_exception;
    }

    V.init();
    if (v_instr_info.wide_vd || v_instr_info.wide_vs2)
    {
        V_wide.init();
    }

    RVVector &vs2 = v_instr_info.wide_vs2 ? V_wide.get_vec(reg_vs2) : V.get_vec(reg_vs2);
    RVVector &vd = v_instr_info.wide_vd ? V_wide.get_vec(reg_vd) : V.get_vec(reg_vd);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector_convert(vs2, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                           v_instr_info.signed_op, float_instr_info.cvt_rtz, float_instr_info.ncvt_rod,
                           float_instr_info.cvt_vs2_is_int, v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_convert_wide(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                                 FloatInstrInfo const &float_instr_info, uint16_t reg_vd,
                                                 uint16_t reg_vs2, FloatConversionFunction func)
{
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);
    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(2 * v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(reg_vs2))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V_wide.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    V_wide.init();

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = V_wide.get_vec(reg_vd);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector_convert(vs2, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                           v_instr_info.signed_op, float_instr_info.cvt_rtz, float_instr_info.ncvt_rod,
                           float_instr_info.cvt_vs2_is_int, v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

VILL::vpu_return_t VARITH_FLOAT::vf_convert_narrow(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                                                   FloatInstrInfo const &float_instr_info, uint16_t reg_vd,
                                                   uint16_t reg_vs2, FloatConversionFunction func)
{
    // TODO: check
    RVVRegField V_narrow(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                         SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                  SVMul(2 * v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(reg_vs2))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V_narrow.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    V_narrow.init();

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = V_narrow.get_vec(reg_vd);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector_convert(vs2, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                           v_instr_info.signed_op, float_instr_info.cvt_rtz, float_instr_info.ncvt_rod,
                           float_instr_info.cvt_vs2_is_int, v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
