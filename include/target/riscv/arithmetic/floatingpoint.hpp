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
/// \file floatingpoint.hpp
/// \brief Defines helpers implementing floating-point arithmetics after
/// https://github.com/riscv/riscv-v-spec/blob/0.9/v-spec.adoc#vector-arithmetic-instruction-formats
/// \date 09/09/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_ARITH_FLOATINGPOINT_H__
#define __RVVHL_ARITH_FLOATINGPOINT_H__

#include <functional>

#include "stdint.h"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"

#ifdef ETISS_SOFTFLOAT
extern "C"
{
#include "softfloat_orig.h"
}
#else
#include "softfloat.hpp"
#endif

using FloatFunction = std::function<void(uint64_t, uint64_t, SVElement &, size_t)>;

inline float16_t f16(uint64_t value)
{
    float16_t cast_f16{ (uint16_t)value };
    return cast_f16;
}

inline float32_t f32(uint64_t value)
{
    float32_t cast_f32{ (uint32_t)value };
    return cast_f32;
}

inline float64_t f64(uint64_t value)
{
    float64_t cast_f64{ (uint64_t)value };
    return cast_f64;
}

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes floating-point arithmetic helpers
namespace VARITH_FLOAT
{
VILL::vpu_return_t vf_single_width_op_vv(uint8_t *vec_reg_mem, //!< Vector register file memory space. One dimensional
                                         uint64_t emul_num,    //!< Register multiplicity numerator
                                         uint64_t emul_denom,  //!< Register multiplicity denominator
                                         uint16_t sew_bytes,   //!< Element width [bytes]
                                         uint16_t vec_len,     //!< Vector length [elements]
                                         uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                                         uint16_t dst_vec_reg,       //!< Destination vector D [index]
                                         uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                                         uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                                         uint16_t vec_elem_start,    //!< Starting element [index]
                                         bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                                         FloatFunction func);

VILL::vpu_return_t vf_single_width_op_vf(uint8_t *vec_reg_mem, //!< Vector register file memory space. One dimensional
                                         uint64_t emul_num,    //!< Register multiplicity numerator
                                         uint64_t emul_denom,  //!< Register multiplicity denominator
                                         uint16_t sew_bytes,   //!< Element width [bytes]
                                         uint16_t vec_len,     //!< Vector length [elements]
                                         uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                                         uint16_t dst_vec_reg,       //!< Destination vector D [index]
                                         uint16_t src_vec_reg_lhs,   //!< Source vector R [index]
                                         uint8_t *scalar_reg_mem,    //!< Source vector L [index]
                                         uint8_t scalar_reg_len_bytes,
                                         uint16_t vec_elem_start, //!< Starting element [index]
                                         bool mask_f,             //!< Vector mask flag. 1: masking 0: no masking
                                         FloatFunction func);
/* rvv spec. 14.1. Vector Floating-Point Exception Flags */
// TODO: ...
/* rvv spec. 14.2. Vector Single-Width Floating-Point Add/Subtract Instructions */
// TODO: ...
/* rvv spec. 14.3. Vector Widening Floating-Point Add/Subtract Instructions */
// TODO: ...
/* rvv spec. 14.4. Vector Single-Width Floating-Point Multiply/Divide Instructions */
// TODO: ...
/* rvv spec. 14.5. Vector Widening Floating-Point Multiply */
// TODO: ...
/* rvv spec. 14.6. Vector Single-Width Floating-Point Fused Multiply-Add Instructions */
// TODO: ...
/* rvv spec. 14.7. Vector Widening Floating-Point Fused Multiply-Add Instructions */
// TODO: ...
/* rvv spec. 14.8. Vector Floating-Point Square-Root Instruction */
// TODO: ...
/* rvv spec. 14.9. Vector Floating-Point MIN/MAX Instructions */
// TODO: ...
/* rvv spec. 14.10. Vector Floating-Point Sign-Injection Instructions */
// TODO: ...
/* rvv spec. 14.11. Vector Floating-Point Compare Instructions */
// TODO: ...
/* rvv spec. 14.12. Vector Floating-Point Classify Instruction */
// TODO: ...
/* rvv spec. 14.13. Vector Floating-Point Merge Instruction */
// TODO: ...
/* rvv spec. 14.14. Vector Floating-Point Move Instruction */
// TODO: ...
/* rvv spec. 14.15. Single-Width Floating-Point/Integer Type-Convert Instructions */
// TODO: ...
/* rvv spec. 14.16. Widening Floating-Point/Integer Type-Convert Instructions */
// TODO: ...
/* rvv spec. 14.17. Narrowing Floating-Point/Integer Type-Convert Instructions */
// TODO: ...

} // namespace VARITH_FLOAT
#endif /* __RVVHL_ARITH_FLOATINGPOINT_H__ */
