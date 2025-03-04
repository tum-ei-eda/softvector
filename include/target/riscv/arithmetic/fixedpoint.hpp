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
/// \file fixedpoint.hpp
/// \brief Defines helpers implementing fixed-point arithmetics after
/// https://github.com/riscv/riscv-v-spec/blob/0.9/v-spec.adoc#vector-arithmetic-instruction-formats
/// \date 09/09/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_ARITH_FIXEDPOINT_H__
#define __RVVHL_ARITH_FIXEDPOINT_H__

#include <algorithm>
#include <functional>

#include "stdint.h"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes fixed-point arithmetic helpers
namespace VARITH_FIXP
{

// Returns true if operation was saturating
using FixpointFunction = std::function<bool(uint64_t /* lhs */, uint64_t /* rhs */, SVElement & /* vd */,
                                            size_t /* sew */, uint8_t /* rounding_mode */)>;

struct FpInstrInfo
{
    uint8_t rounding_mode = 0U;
    bool narrowing_op = false;
};

/**
 * @brief Saturates a value to the nearest sew bit boundary
 */
[[nodiscard]] inline auto saturate_boundary_signed(int64_t value, size_t sew) -> int64_t
{
    // E.g. 8 bit:
    // Upper bound = 0111 1111
    // Lower bound = 1000 0000
    // Lower bound is extended to 64 bit, so just AND -1 with the inverted upper bound
    int64_t upper_bound = get_n_bit_mask(sew - 1);
    int64_t lower_bound = -1 & (~upper_bound);
    return std::clamp(value, lower_bound, upper_bound);
}

/**
 * @brief Saturates a value to the unsigned sew bit boundary
 */
[[nodiscard]] inline auto saturate_boundary_unsigned(uint64_t value, size_t sew) -> uint64_t
{
    auto upper_bound = get_n_bit_mask(sew);
    return (value <= upper_bound) ? value : upper_bound;
}

[[nodiscard]] auto roundoff_unsigned(uint64_t value, uint8_t rounding_bits, uint8_t rounding_mode) -> uint64_t;

[[nodiscard]] auto roundoff_signed(int64_t value, uint8_t rounding_bits, uint8_t rounding_mode) -> int64_t;

// TODO: Inline implementations in .ipp file
/* 12.1. Vector Single-Width Saturating Add and Subtract */

inline FixpointFunction sadd = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                  uint8_t rounding_mode) -> bool {
    auto res = static_cast<int64_t>(lhs) + static_cast<int64_t>(rhs);
    auto msb_lhs = msb_is_set(lhs, sew);
    auto msb_rhs = msb_is_set(rhs, sew);
    auto msb_res = msb_is_set(res, sew);

    if (msb_lhs && msb_rhs && !msb_res)
    {
        // Negative overflow
        vd = get_min_signed(sew);
        return true;
    }

    if (!msb_lhs && !msb_rhs && msb_res)
    {
        // Positive overflow
        vd = get_n_bit_mask(sew - 1);
        return true;
    }

    vd = res;
    return false;
};

inline FixpointFunction saddu = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                   uint8_t rounding_mode) -> bool {
    auto sew_mask = get_n_bit_mask(sew);
    auto res = (lhs + rhs) & sew_mask;
    auto sat = false;
    if (res < lhs)
    {
        // Overflow
        res = sew_mask;
        sat = true;
    }
    vd = res;
    return sat;
};

inline FixpointFunction ssub = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                  uint8_t rounding_mode) -> bool {
    auto res = static_cast<int64_t>(lhs) - static_cast<int64_t>(rhs);
    auto msb_lhs = msb_is_set(lhs, sew);
    auto msb_rhs = msb_is_set(rhs, sew);
    auto msb_res = msb_is_set(res, sew);

    if (msb_lhs && !msb_rhs && !msb_res)
    {
        // Negative overflow
        vd = get_min_signed(sew);
        return true;
    }

    if (!msb_lhs && msb_rhs && msb_res)
    {
        // Positive overflow
        vd = get_n_bit_mask(sew - 1);
        return true;
    }

    vd = res;
    return false;
};

inline FixpointFunction ssubu = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                   uint8_t rounding_mode) -> bool {
    auto sew_mask = get_n_bit_mask(sew);
    auto res = (lhs - rhs) & sew_mask;
    auto sat = false;
    if (res > lhs)
    {
        // Overflow
        res = 1_i64 << (sew - 1);
        sat = true;
    }
    vd = res;
    return sat;
};

/* 12.2. Vector Single-Width Averaging Add and Subtract */

inline FixpointFunction aadd = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                  uint8_t rounding_mode) -> bool {
    static constexpr auto rounding_bits = 1;
    auto res = static_cast<int64_t>(lhs) + static_cast<int64_t>(rhs);
    vd = roundoff_signed(res, rounding_bits, rounding_mode);
    return false;
};

inline FixpointFunction aaddu = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                   uint8_t rounding_mode) -> bool {
    static constexpr auto rounding_bits = 1;
    auto res = lhs + rhs;
    vd = roundoff_unsigned(res, rounding_bits, rounding_mode);
    return false;
};

inline FixpointFunction asub = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                  uint8_t rounding_mode) -> bool {
    static constexpr auto rounding_bits = 1;
    auto res = static_cast<int64_t>(lhs) - static_cast<int64_t>(rhs);
    vd = roundoff_signed(res, rounding_bits, rounding_mode);
    return false;
};

inline FixpointFunction asubu = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                   uint8_t rounding_mode) -> bool {
    static constexpr auto rounding_bits = 1;
    auto res = lhs - rhs;
    vd = roundoff_unsigned(res, rounding_bits, rounding_mode);
    return false;
};

/* 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation */

inline FixpointFunction smul = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                  uint8_t rounding_mode) -> bool {
    auto res = (static_cast<int64_t>(lhs) * static_cast<int64_t>(rhs));
    res = roundoff_signed(res, sew - 1, rounding_mode);
    auto clamped_res = saturate_boundary_signed(res, sew);
    vd = clamped_res;
    return clamped_res != res;
};

/* 12.4. Vector Single-Width Scaling Shift Instructions */

inline FixpointFunction ssrl = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                  uint8_t rounding_mode) -> bool {
    // Masking with sew - 1 will provide a bitmask that only uses the lower lg2(SEW) bits.
    auto shiftamount = rhs & (sew - 1);
    auto res = roundoff_signed(lhs, shiftamount, rounding_mode);
    vd = res;
    return false;
};

/* 12.5. Vector Narrowing Fixed-Point Clip Instructions */

inline FixpointFunction clip = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                  uint8_t rounding_mode) -> bool {
    // Masking with (sew << 1) - 1 will provide a bitmask that only uses the lower lg2(2*SEW) bits.
    auto shiftamount = rhs & ((sew << 1) - 1);
    auto res = roundoff_signed(sign_extend(lhs, 2 * sew), shiftamount, rounding_mode);
    auto clamped_res = saturate_boundary_signed(res, sew);
    vd = clamped_res;
    return clamped_res != res;
};

inline FixpointFunction clipu = [](uint64_t lhs, uint64_t rhs, SVElement &vd, size_t sew,
                                   uint8_t rounding_mode) -> bool {
    auto shiftamount = rhs & ((sew << 1) - 1);
    auto res = roundoff_unsigned(lhs, shiftamount, rounding_mode);
    auto clamped_res = saturate_boundary_unsigned(res, sew);
    vd = clamped_res;
    return clamped_res != res;
};

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Fixed-point operation vector-vector
/// \details For all i: D[i] = L[i] op R[i]
VILL::vpu_return_t fixp_op_vv(uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
                              const VInstrInfo &v_instr_info, //!< Struct containing vector instruction information
                              const FpInstrInfo &fixedpoint_info, //!< Struct containing fixed-point op information
                              uint16_t reg_vd,                    //!< Destination vector D [index]
                              uint16_t reg_vs1,                   //!< Source vector R [index]
                              uint16_t reg_vs2,                   //!< Source vector L [index]
                              FixpointFunction func               //!< The inner function
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Fixed-point operation vector-scalar
/// \details For all i: D[i] = L[i] op (signed ? sign_extend(X[rs1]) : X[rs1])
VILL::vpu_return_t fixp_op_vx(uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
                              const VInstrInfo &v_instr_info, //!< Struct containing vector instruction information
                              const FpInstrInfo &fixedpoint_info, //!< Struct containing fixed-point op information
                              uint16_t reg_vd,                    //!< Destination vector D [index]
                              uint16_t reg_vs2,                   //!< Source vector L [index]
                              uint8_t *scalar_reg_mem,        //!< Memory space holding scalar data (min. _xlenb bytes)
                              uint8_t scalar_register_length, //!< Length of scalar (XLEN) [bit]
                              FixpointFunction func           //!< The inner function
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Fixed-point operation vector-immediate
/// \details For all i: D[i] = L[i] op sign_extend(imm)
VILL::vpu_return_t fixp_op_vi(
    uint8_t *vec_reg_mem,               //!< Vector register file memory space. One dimensional
    const VInstrInfo &v_instr_info,     //!< Struct containing vector instruction information
    const FpInstrInfo &fixedpoint_info, //!< Struct containing fixed-point op specific information
    uint16_t reg_vd,                    //!< Destination vector D [index]
    uint16_t reg_vs2,                   //!< Source vector L [index]
    uint8_t imm5,                       //!< Sign or zero extending 5-bit immediate
    FixpointFunction func               //!< The inner function
);

/* 12.1. Vector Single-Width Saturating Add and Subtract */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Saturating addition vector-vector
/// \details For all i: D[i] = L[i] + R[i]
VILL::vpu_return_t vsadd_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed              //!< Signed or unsigned operation
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Saturating addition vector-immediate
/// \details For all i: D[i] = L[i] + sign_extend(imm)
VILL::vpu_return_t vsadd_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint8_t imm,                //!< Sign or zero extending 5-bit immediate
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed              //!< Signed or unsigned operation
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Saturating addition vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X)
VILL::vpu_return_t vsadd_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,           //!< Register multiplicity numerator
                            uint64_t emul_denom,         //!< Register multiplicity denominator
                            uint16_t sew_bytes,          //!< Element width [bytes]
                            uint16_t vec_len,            //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,        //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,     //!< Starting element [index]
                            bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed,              //!< Signed or unsigned operation
                            uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Saturating subtraction vector-vector
/// \details For all i: D[i] = L[i] + R[i]
VILL::vpu_return_t vssub_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed              //!< Signed or unsigned operation
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Saturating subtraction vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X)
VILL::vpu_return_t vssub_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,           //!< Register multiplicity numerator
                            uint64_t emul_denom,         //!< Register multiplicity denominator
                            uint16_t sew_bytes,          //!< Element width [bytes]
                            uint16_t vec_len,            //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,        //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,     //!< Starting element [index]
                            bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed,              //!< Signed or unsigned operation
                            uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);
/* End 12.1. */

/* 12.2. Vector Single-Width Averaging Add and Subtract */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Averaging addition vector-vector
/// \details For all i: D[i] = L[i] + R[i]
VILL::vpu_return_t vaadd_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed,             //!< Signed or unsigned operation
                            uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Averaging addition vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X)
VILL::vpu_return_t vaadd_vx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,            //!< Register multiplicity numerator
                            uint64_t emul_denom,          //!< Register multiplicity denominator
                            uint16_t sew_bytes,           //!< Element width [bytes]
                            uint16_t vec_len,             //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,         //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,      //!< Starting element [index]
                            bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed,               //!< Signed or unsigned operation
                            uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                            uint8_t rounding_mode         //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Averaging subtraction vector-vector
/// \details For all i: D[i] = L[i] + R[i]
VILL::vpu_return_t vasub_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed,             //!< Signed or unsigned operation
                            uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Averaging subtraction vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X)
VILL::vpu_return_t vasub_vx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,            //!< Register multiplicity numerator
                            uint64_t emul_denom,          //!< Register multiplicity denominator
                            uint16_t sew_bytes,           //!< Element width [bytes]
                            uint16_t vec_len,             //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,         //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,      //!< Starting element [index]
                            bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                            bool is_signed,               //!< Signed or unsigned operation
                            uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                            uint8_t rounding_mode         //!< Rounding mode
);
/* End 12.2. */

/* 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed multiply with rounding and saturation vector-vector
/// \details For all i: D[i] = L[i] + R[i]
VILL::vpu_return_t vsmul_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed multiply with rounding and saturation vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X)
VILL::vpu_return_t vsmul_vx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,            //!< Register multiplicity numerator
                            uint64_t emul_denom,          //!< Register multiplicity denominator
                            uint16_t sew_bytes,           //!< Element width [bytes]
                            uint16_t vec_len,             //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,         //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,      //!< Starting element [index]
                            bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                            uint8_t rounding_mode         //!< Rounding mode
);
/* End 12.3. */

/* 12.4. Vector Single-Width Scaling Shift Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Scaling right logical shift vector-vector
/// \details For all i: D[i] = roundoff_unsigned(L[i], R[i])
VILL::vpu_return_t vssrl_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Scaling right logical shift vector-immediate
/// \details For all i: D[i] = roundoff_unsigned(L[i], uimm)
VILL::vpu_return_t vssrl_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint8_t imm,                //!< Sign or zero extending 5-bit immediate
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Scaling right logical shift vector-scalar
/// \details For all i: D[i] = roundoff_unsigned(L[i], x[Rs1])
VILL::vpu_return_t vssrl_vx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,            //!< Register multiplicity numerator
                            uint64_t emul_denom,          //!< Register multiplicity denominator
                            uint16_t sew_bytes,           //!< Element width [bytes]
                            uint16_t vec_len,             //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,         //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,      //!< Starting element [index]
                            bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                            uint8_t rounding_mode         //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Scaling right arithmetic shift vector-vector
/// \details For all i: D[i] = roundoff_signed(L[i], R[i])
VILL::vpu_return_t vssra_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Scaling right arithmetic shift vector-immediate
/// \details For all i: D[i] = roundoff_signed(L[i], uimm)
VILL::vpu_return_t vssra_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint8_t imm,                //!< Sign or zero extending 5-bit immediate
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Scaling right arithmetic shift vector-scalar
/// \details For all i: D[i] = roundoff_signed(L[i], x[Rs1])
VILL::vpu_return_t vssra_vx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,            //!< Register multiplicity numerator
                            uint64_t emul_denom,          //!< Register multiplicity denominator
                            uint16_t sew_bytes,           //!< Element width [bytes]
                            uint16_t vec_len,             //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,         //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,      //!< Starting element [index]
                            bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                            uint8_t rounding_mode         //!< Rounding mode
);
/* End 12.4. */

/* 12.5. Vector Narrowing Fixed-Point Clip Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing unsigned clip vector-vector
/// \details For all i: D[i] = roundoff_signed(L[i], R[i])
VILL::vpu_return_t vnclipu_wv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                              uint64_t emul_num,          //!< Register multiplicity numerator
                              uint64_t emul_denom,        //!< Register multiplicity denominator
                              uint16_t sew_bytes,         //!< Element width [bytes]
                              uint16_t vec_len,           //!< Vector length [elements]
                              uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                              uint16_t dst_vec_reg,       //!< Destination vector D [index]
                              uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                              uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                              uint16_t vec_elem_start,    //!< Starting element [index]
                              bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                              uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing unsigned clip vector-immediate
/// \details For all i: D[i] = roundoff_signed(L[i], uimm)
VILL::vpu_return_t vnclipu_wi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                              uint64_t emul_num,          //!< Register multiplicity numerator
                              uint64_t emul_denom,        //!< Register multiplicity denominator
                              uint16_t sew_bytes,         //!< Element width [bytes]
                              uint16_t vec_len,           //!< Vector length [elements]
                              uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                              uint16_t dst_vec_reg,       //!< Destination vector D [index]
                              uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                              uint8_t imm,                //!< Sign or zero extending 5-bit immediate
                              uint16_t vec_elem_start,    //!< Starting element [index]
                              bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                              uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing unsigned clip vector-scalar
/// \details For all i: D[i] = roundoff_signed(L[i], x[Rs1])
VILL::vpu_return_t vnclipu_wx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                              uint64_t emul_num,            //!< Register multiplicity numerator
                              uint64_t emul_denom,          //!< Register multiplicity denominator
                              uint16_t sew_bytes,           //!< Element width [bytes]
                              uint16_t vec_len,             //!< Vector length [elements]
                              uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                              uint16_t dst_vec_reg,         //!< Destination vector D [index]
                              uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                              uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                              uint16_t vec_elem_start,      //!< Starting element [index]
                              bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                              uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                              uint8_t rounding_mode         //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing signed clip vector-vector
/// \details For all i: D[i] = roundoff_signed(L[i], R[i])
VILL::vpu_return_t vnclip_wv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                             uint64_t emul_num,          //!< Register multiplicity numerator
                             uint64_t emul_denom,        //!< Register multiplicity denominator
                             uint16_t sew_bytes,         //!< Element width [bytes]
                             uint16_t vec_len,           //!< Vector length [elements]
                             uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                             uint16_t dst_vec_reg,       //!< Destination vector D [index]
                             uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                             uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                             uint16_t vec_elem_start,    //!< Starting element [index]
                             bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                             uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing signed clip vector-immediate
/// \details For all i: D[i] = roundoff_signed(L[i], uimm)
VILL::vpu_return_t vnclip_wi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                             uint64_t emul_num,          //!< Register multiplicity numerator
                             uint64_t emul_denom,        //!< Register multiplicity denominator
                             uint16_t sew_bytes,         //!< Element width [bytes]
                             uint16_t vec_len,           //!< Vector length [elements]
                             uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                             uint16_t dst_vec_reg,       //!< Destination vector D [index]
                             uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                             uint8_t imm,                //!< Sign or zero extending 5-bit immediate
                             uint16_t vec_elem_start,    //!< Starting element [index]
                             bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                             uint8_t rounding_mode       //!< Rounding mode
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing signed clip vector-scalar
/// \details For all i: D[i] = roundoff_signed(L[i], x[Rs1])
VILL::vpu_return_t vnclip_wx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                             uint64_t emul_num,            //!< Register multiplicity numerator
                             uint64_t emul_denom,          //!< Register multiplicity denominator
                             uint16_t sew_bytes,           //!< Element width [bytes]
                             uint16_t vec_len,             //!< Vector length [elements]
                             uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                             uint16_t dst_vec_reg,         //!< Destination vector D [index]
                             uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                             uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                             uint16_t vec_elem_start,      //!< Starting element [index]
                             bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                             uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                             uint8_t rounding_mode         //!< Rounding mode
);
/* End 12.5. */
/* End 12. */

} // namespace VARITH_FIXP
#endif /* __RVVHL_ARITH_FIXEDPOINT_H__ */
