#include <cstring>

#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"
#include "misc/mask.hpp"

// Private function declarations
auto iterate_register_logical(const SVRegister &vs2, const SVRegister &vs1, SVRegister &vd, MaskFunction func,
                              size_t length, size_t start_index = 0) -> void;

auto iterate_register_vcpop(const SVRegister &vs2, const SVRegister &vm, bool mask, size_t length,
                            size_t start_index = 0) -> uint64_t;

auto iterate_register_vfirst(const SVRegister &vs2, const SVRegister &vm, bool mask, size_t length,
                             size_t start_index = 0) -> uint64_t;

// Set before/including/only first
auto iterate_register_sxf(const SVRegister &vs2, SVRegister &vd, const SVRegister &vm, bool mask, bool including_first,
                          bool only_first, size_t length, size_t start_index = 0) -> uint64_t;

auto check_vector_register_overlap(const RVVector &vec, const SVRegister &reg) -> bool;

// Private function definitions
auto iterate_register_logical(const SVRegister &vs2, const SVRegister &vs1, SVRegister &vd, MaskFunction func,
                              size_t length, size_t start_index) -> void
{
    for (size_t i_element = start_index; i_element < length; ++i_element)
    {
        func(vs2.get_bit(i_element), vs1.get_bit(i_element)) ? vd.set_bit(i_element) : vd.reset_bit(i_element);
    }
}

auto iterate_register_vcpop(const SVRegister &vs2, const SVRegister &vm, bool mask, size_t length, size_t start_index)
    -> uint64_t
{
    uint64_t rd_value = 0;
    for (size_t i_element = start_index; i_element < length; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            rd_value += vs2.get_bit(i_element);
        }
    }
    return rd_value;
}

auto iterate_register_vfirst(const SVRegister &vs2, const SVRegister &vm, bool mask, size_t length, size_t start_index)
    -> uint64_t
{
    uint64_t rd_value = -1;
    for (size_t i_element = start_index; i_element < length; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (vs2.get_bit(i_element))
            {
                rd_value = i_element;
                break;
            }
        }
    }
    return rd_value;
}

auto iterate_register_sxf(const SVRegister &vs2, SVRegister &vd, const SVRegister &vm, bool mask, bool including_first,
                          bool only_first, size_t length, size_t start_index) -> uint64_t
{
    auto first_found = false;
    for (size_t i_element = start_index; i_element < length; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (first_found)
            {
                vd.reset_bit(i_element);
                continue;
            }

            auto element = vs2.get_bit(i_element);
            if (element)
            {
                first_found = true;
                (including_first || only_first) ? vd.set_bit(i_element) : vd.reset_bit(i_element);
                continue;
            }
            if (!only_first)
            {
                vd.set_bit(i_element);
            }
        }
    }
}

auto check_vector_register_overlap(const RVVector &vec, const SVRegister &reg) -> bool
{
    if ((reg.mem_ >= vec.mem_) && (reg.mem_ < vec.mem_ + vec.length_ * (vec.activeElement.width_in_bits_ >> 3)))
        return true;
    if ((vec.mem_ >= reg.mem_) && (vec.mem_ < reg.mem_ + (reg.length_bits_ >> 3)))
        return true;
    return false;
}

// Public function definitions
auto VMASK::mask_op_logical(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f,
                            MaskFunction func) -> VILL::vpu_return_t
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

    SVRegister &vs1 = V.get_vecreg(src_vec_reg_rhs);
    SVRegister &vs2 = V.get_vecreg(src_vec_reg_lhs);
    SVRegister &vd = V.get_vecreg(dst_vec_reg);

    iterate_register_logical(vs2, vs1, vd, func, vec_len, vec_elem_start);

    return VILL::VPU_RETURN::NO_EXCEPT;
}

auto VMASK::mask_op_to_scalar(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                              uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t src_vec_reg_lhs,
                              uint8_t *dst_scalar_reg, uint16_t vec_elem_start, bool mask_f,
                              uint8_t scalar_reg_len_bytes, bool is_vcpop) -> VILL::vpu_return_t
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    SVRegister &vs2 = V.get_vecreg(src_vec_reg_lhs);

    uint64_t result;
    if (is_vcpop)
    {
        result = iterate_register_vcpop(vs2, V.get_mask_reg(), !mask_f, vec_len, vec_elem_start);
    }
    else
    {
        result = iterate_register_vfirst(vs2, V.get_mask_reg(), !mask_f, vec_len, vec_elem_start);
    }

    std::memcpy(dst_scalar_reg, &result, scalar_reg_len_bytes);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

auto VMASK::mask_op_sxf(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                        uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs,
                        uint16_t vec_elem_start, bool mask_f, bool including_first, bool only_first)
    -> VILL::vpu_return_t
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

    SVRegister &vs2 = V.get_vecreg(src_vec_reg_lhs);
    SVRegister &vd = V.get_vecreg(dst_vec_reg);
    SVRegister &vm = V.get_mask_reg();

    // Overlap checks
    // TODO: check if correct
    if (dst_vec_reg == src_vec_reg_lhs)
    {
        return VILL::DST_REG_SRC_REG_OVERLAP_ILL;
    }

    if (!mask_f && dst_vec_reg == 0)
    {
        return VILL::DST_REG_MASK_REG_OVERLAP_ILL;
    }

    iterate_register_sxf(vs2, vd, vm, !mask_f, including_first, only_first, vec_len, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

auto VMASK::mask_viota(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs,
                       uint16_t vec_elem_start, bool mask_f) -> VILL::vpu_return_t
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

    SVRegister &vs2 = V.get_vecreg(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);
    SVRegister &vm = V.get_mask_reg();

    // Overlap checks
    // TODO: check if correct
    if (check_vector_register_overlap(vd, vs2))
    {
        return VILL::DST_VEC_SRC_REG_OVERLAP_ILL;
    }

    if (!mask_f && dst_vec_reg == 0)
    {
        return VILL::DST_VEC_SRC_REG_OVERLAP_ILL;
    }

    auto accumulator = 0;

    for (size_t i_element = vec_elem_start; i_element < vs2.length_bits_; ++i_element)
    {
        if (mask_f || vm.get_bit(i_element))
        {
            vd[i_element] = accumulator;
            accumulator += vs2.get_bit(i_element);
        }
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

auto VMASK::mask_vid(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes, uint16_t vec_len,
                     uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg, uint16_t vec_elem_start, bool mask_f)
    -> VILL::vpu_return_t
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vd = V.get_vec(dst_vec_reg);
    SVRegister &vm = V.get_mask_reg();

    for (size_t i_element = vec_elem_start; i_element < vd.length_; ++i_element)
    {
        if (mask_f || vm.get_bit(i_element))
        {
            vd[i_element] = i_element;
        }
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}