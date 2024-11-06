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

/*
This part of the code is copied from or heavily inspired by https://github.com/ics-jku/riscv-vp-plusplus
Therefore, this marks the start of the following copyright notice:
Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
Copyright (c) 2022-2023 Intitute for Complex Systems, Johannes Kepler University Linz <ics-office@jku.at>
*/
constexpr uint16_t F16_SIGN_BIT = 1 << 15;
constexpr uint32_t F32_SIGN_BIT = 1 << 31;
constexpr uint64_t F64_SIGN_BIT = 1ul << 63;

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

inline float16_t f16_neg(float16_t x)
{
    uint16_t res = x.v ^ F16_SIGN_BIT;
    return float16_t{ res };
}

inline float32_t f32_neg(float32_t x)
{
    return float32_t{ x.v ^ F32_SIGN_BIT };
}

inline float64_t f64_neg(float64_t x)
{
    return float64_t{ x.v ^ F64_SIGN_BIT };
}

/* 13.2. Vector Single-Width Floating-Point Add/Subtract Instructions */
inline FloatFunction vfadd = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_add(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_add(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_add(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_sub(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_sub(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_sub(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfrsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_sub(f16(rhs), f16(opL)).v;
        break;
    case 32:
        vd = f32_sub(f32(rhs), f32(opL)).v;
        break;
    case 64:
        vd = f64_sub(f64(rhs), f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};
/* End 13.2. */

/* 13.3. Vector Widening Floating-Point Add/Subtract Instructions */
inline FloatFunction vfwadd = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_add(f16_to_f32(f16(opL)), f16_to_f32(f16(rhs))).v;
        break;
    case 32:
        vd = f64_add(f32_to_f64(f32(opL)), f32_to_f64(f32(rhs))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfwsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_sub(f16_to_f32(f16(opL)), f16_to_f32(f16(rhs))).v;
        break;
    case 32:
        vd = f64_sub(f32_to_f64(f32(opL)), f32_to_f64(f32(rhs))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

// Wide vs2 (2*SEW)
inline FloatFunction vfwadd_w = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_add(f32(opL), f16_to_f32(f16(rhs))).v;
        break;
    case 32:
        vd = f64_add(f64(opL), f32_to_f64(f32(rhs))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

// Wide vs2 (2*SEW)
inline FloatFunction vfwsub_w = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_sub(f32(opL), f16_to_f32(f16(rhs))).v;
        break;
    case 32:
        vd = f64_sub(f64(opL), f32_to_f64(f32(rhs))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};
/* End 13.3. */

/* 13.4. Vector Single-Width Floating-Point Multiply/Divide Instructions */
inline FloatFunction vfmul = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mul(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_mul(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_mul(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfdiv = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_div(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_div(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_div(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfrdiv = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_div(f16(rhs), f16(opL)).v;
        break;
    case 32:
        vd = f32_div(f32(rhs), f32(opL)).v;
        break;
    case 64:
        vd = f64_div(f64(rhs), f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};
/* End 13.4. */

/* 13.5. Vector Widening Floating-Point Multiply */
inline FloatFunction vfwmul = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_mul(f16_to_f32(f16(opL)), f16_to_f32(f16(rhs))).v;
        break;
    case 32:
        vd = f64_mul(f32_to_f64(f32(opL)), f32_to_f64(f32(rhs))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};
/* End 13.5. */

/* 13.6. Vector Single-Width Floating-Point Fused Multiply-Add Instructions */
inline FloatFunction vfmacc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(opL), f16(rhs), f16(vd.to_u64())).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(opL), f32(rhs), f32(vd.to_u64())).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(opL), f64(rhs), f64(vd.to_u64())).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfnmacc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(opL), f16_neg(f16(rhs)), f16_neg(f16(vd.to_u64()))).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(opL), f32_neg(f32(rhs)), f32_neg(f32(vd.to_u64()))).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(opL), f64_neg(f64(rhs)), f64_neg(f64(vd.to_u64()))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfmsac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(opL), f16(rhs), f16_neg(f16(vd.to_u64()))).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(opL), f32(rhs), f32_neg(f32(vd.to_u64()))).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(opL), f64(rhs), f64_neg(f64(vd.to_u64()))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfnmsac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(opL), f16_neg(f16(rhs)), f16(vd.to_u64())).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(opL), f32_neg(f32(rhs)), f32(vd.to_u64())).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(opL), f64_neg(f64(rhs)), f64(vd.to_u64())).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfmadd = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(vd.to_u64()), f16_neg(f16(rhs)), f16(opL)).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(vd.to_u64()), f32_neg(f32(rhs)), f32(opL)).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(vd.to_u64()), f64_neg(f64(rhs)), f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfnmadd = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(vd.to_u64()), f16_neg(f16(rhs)), f16_neg(f16(opL))).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(vd.to_u64()), f32_neg(f32(rhs)), f32_neg(f32(opL))).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(vd.to_u64()), f64_neg(f64(rhs)), f64_neg(f64(opL))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfmsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(vd.to_u64()), f16(rhs), f16_neg(f16(opL))).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(vd.to_u64()), f32(rhs), f32_neg(f32(opL))).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(vd.to_u64()), f64(rhs), f64_neg(f64(opL))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfnmsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(vd.to_u64()), f16_neg(f16(rhs)), f16(opL)).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(vd.to_u64()), f32_neg(f32(rhs)), f32(opL)).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(vd.to_u64()), f64_neg(f64(rhs)), f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};
/* End 13.6. */

/* 13.7. Vector Widening Floating-Point Fused Multiply-Add Instructions */
inline FloatFunction vfwmacc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_mulAdd(f16_to_f32(f16(opL)), f16_to_f32(f16(rhs)), f32(vd.to_u64())).v;
        break;
    case 32:
        vd = f64_mulAdd(f32_to_f64(f32(opL)), f32_to_f64(f32(rhs)), f64(vd.to_u64())).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfwnmacc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_mulAdd(f16_to_f32(f16(opL)), f16_to_f32(f16_neg(f16(rhs))), f32_neg(f32(vd.to_u64()))).v;
        break;
    case 32:
        vd = f64_mulAdd(f32_to_f64(f32(opL)), f32_to_f64(f32_neg(f32(rhs))), f64_neg(f64(vd.to_u64()))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfwmsac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_mulAdd(f16_to_f32(f16(opL)), f16_to_f32(f16(rhs)), f32_neg(f32(vd.to_u64()))).v;
        break;
    case 32:
        vd = f64_mulAdd(f32_to_f64(f32(opL)), f32_to_f64(f32(rhs)), f64_neg(f64(vd.to_u64()))).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction vfwnmsac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> void {
    switch (sew)
    {
    case 16:
        vd = f32_mulAdd(f16_to_f32(f16(opL)), f16_to_f32(f16_neg(f16(rhs))), f32(vd.to_u64())).v;
        break;
    case 32:
        vd = f64_mulAdd(f32_to_f64(f32(opL)), f32_to_f64(f32_neg(f32(rhs))), f64(vd.to_u64())).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};
/* End 13.7. */

/* 13.8. Vector Floating-Point Square-Root Instruction */

/* End 13.8. */

/* 13.9. Vector Floating-Point Reciprocal Square-Root Estimate Instruction */

/* End 13.9. */

/* 13.10. Vector Floating-Point Reciprocal Estimate Instruction */

/* End 13.10. */

/* 13.11. Vector Floating-Point MIN/MAX Instructions */

/* End 13.11. */

/* 13.12. Vector Floating-Point Sign-Injection Instructions */

/* End 13.12. */

/* 13.13. Vector Floating-Point Compare Instructions */

/* End 13.13. */

/* 13.14. Vector Floating-Point Classify Instruction */

/* End 13.14. */

/* 13.15. Vector Floating-Point Merge Instruction */

/* End 13.15. */

/* 13.16. Vector Floating-Point Move Instruction */

/* End 13.16. */

/* 13.17. Single-Width Floating-Point/Integer Type-Convert Instructions */
/* End 13.17. */

/* 13.18. Widening Floating-Point/Integer Type-Convert Instructions */
/* End 13.18. */

/* 13.19. Narrowing Floating-Point/Integer Type-Convert Instructions */
/* End 13.19. */

/* 
End of following copyright notice:
Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
Copyright (c) 2022-2023 Intitute for Complex Systems, Johannes Kepler University Linz <ics-office@jku.at>
*/

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes floating-point arithmetic helpers
namespace VARITH_FLOAT
{
VILL::vpu_return_t vf_op_vv(uint8_t *vec_reg_mem, //!< Vector register file memory space. One dimensional
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

VILL::vpu_return_t vf_op_vf(uint8_t *vec_reg_mem, //!< Vector register file memory space. One dimensional
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
