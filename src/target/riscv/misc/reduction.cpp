#include <functional>
#include <cstdint>
#include <cstddef>

#include "misc/reduction.hpp"
#include "arithmetic/integer.hpp"
#include "arithmetic/floatingpoint.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

// Private function declarations
void iterate_vector_int(SVector const &vs2, std::uint64_t const vs1_first, SVector &vd, SVRegister const &vm,
                        bool const mask, VARITH_INT::IntFunction func, bool const signed_op,
                        std::size_t const start_index);

void iterate_vector_float(SVector const &vs2, std::uint64_t vs1_first, SVector &vd, SVRegister const &vm, bool mask,
                          VARITH_FLOAT::FloatFunction func, std::size_t sew, std::size_t start_index, bool widening);

// Private function definitions
void iterate_vector_int(SVector const &vs2, std::uint64_t const vs1_first, SVector &vd, SVRegister const &vm,
                        bool const mask, VARITH_INT::IntFunction func, bool const signed_op,
                        std::size_t const start_index)
{
    if (vs2.length_ == 0)
    {
        return;
    }

    std::uint64_t accumulator = vs1_first;
    for (size_t i_element = start_index; i_element < vs2.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            std::uint64_t lhs = signed_op ? vs2[i_element].to_i64() : vs2[i_element].to_u64();
            // Mask bit is never data in reduction instructions
            accumulator = func(lhs, accumulator, /* mask_bit */ false);
        }
    }
    vd[0] = accumulator;
}

void iterate_vector_float(SVector const &vs2, std::uint64_t vs1_first, SVector &vd, SVRegister const &vm, bool mask,
                          VARITH_FLOAT::FloatFunction func, std::size_t sew, std::size_t start_index, bool widening)
{
    auto first = true;
    for (size_t i_element = start_index; i_element < vd.length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {

            std::uint64_t lhs = vs2[i_element].to_u64();
            std::uint64_t rhs = 0;
            if (first)
            {
                rhs = vs1_first;
                first = false;
            }
            else
            {
                rhs = vd[0].to_u64();
            }
            func(lhs, rhs, vd[0], sew);
        }
    }
    if (first)
    {
        // All elements masked
        vd[0] = vs1_first;
    }
}

// Public function definitions
auto VREDUC::red_op_int(std::uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info, std::uint16_t const reg_vd,
                        std::uint16_t const reg_vs1, std::uint16_t const reg_vs2,
                        VARITH_INT::IntFunction func) -> VILL::vpu_return_t
{
    if (v_instr_info.vector_length == 0)
    {
        return VILL::VPU_RETURN::NO_EXCEPT;
    }
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    // Reduction operation: don't increase EMUL for wide field!
    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto alignment_exception =
        check_alignment(V, V_wide, reg_vd, reg_vs2, reg_vs1, v_instr_info.wide_vd, v_instr_info.wide_vs2);
    if (alignment_exception != VILL::vpu_return_t::NO_EXCEPT)
    {
        return alignment_exception;
    }

    // TODO: check wide V overlap rules

    V.init();
    if (v_instr_info.wide_vd || v_instr_info.wide_vs1)
    {
        V_wide.init();
    }

    std::uint64_t vs1_first = 0;
    if (v_instr_info.wide_vs1)
    {
        vs1_first = v_instr_info.signed_op ? V_wide.get_vec(reg_vs1)[0].to_i64() : V_wide.get_vec(reg_vs1)[0].to_u64();
    }
    else
    {
        vs1_first = v_instr_info.signed_op ? V.get_vec(reg_vs1)[0].to_i64() : V.get_vec(reg_vs1)[0].to_u64();
    }

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = v_instr_info.wide_vd ? V_wide.get_vec(reg_vd) : V.get_vec(reg_vd);

    iterate_vector_int(vs2, vs1_first, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.signed_op,
                       v_instr_info.start_element);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

auto VREDUC::red_op_float(uint8_t *vec_reg_mem, VInstrInfo const &v_instr_info,
                          VARITH_FLOAT::FloatInstrInfo const &float_instr_info, std::uint16_t const reg_vd,
                          std::uint16_t const reg_vs1, std::uint16_t const reg_vs2,
                          VARITH_FLOAT::FloatFunction func) -> VILL::vpu_return_t
{
    if (v_instr_info.vector_length == 0)
    {
        return VILL::VPU_RETURN::NO_EXCEPT;
    }
    RVVRegField V(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                  SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);
    RVVRegField V_wide(v_instr_info.vector_register_length, v_instr_info.vector_length, 2 * v_instr_info.sew,
                       SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    const auto alignment_exception =
        check_alignment(V, V_wide, reg_vd, reg_vs1, v_instr_info.wide_vd, v_instr_info.wide_vd);

    if (alignment_exception != VILL::VPU_RETURN::NO_EXCEPT)
    {
        return alignment_exception;
    }

    V.init();
    if (v_instr_info.wide_vd || v_instr_info.wide_vs1)
    {
        V_wide.init();
    }

    std::uint64_t vs1_first =
        v_instr_info.wide_vs1 ? V_wide.get_vec(reg_vs1)[0].to_u64() : V.get_vec(reg_vs1)[0].to_u64();

    RVVector &vs2 = V.get_vec(reg_vs2);
    RVVector &vd = v_instr_info.wide_vd ? V_wide.get_vec(reg_vd) : V.get_vec(reg_vd);

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = float_instr_info.rounding_mode;

    iterate_vector_float(vs2, vs1_first, vd, V.get_mask_reg(), v_instr_info.masked, func, v_instr_info.sew,
                         v_instr_info.start_element, v_instr_info.wide_vd);

    return VILL::VPU_RETURN::NO_EXCEPT;
}
