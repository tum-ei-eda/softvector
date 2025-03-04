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
#include "arithmetic/softfloat-extension.hpp"

#ifdef ETISS_SOFTFLOAT
extern "C"
{
#include "softfloat_orig.h"
}
#else
#include "softfloat.hpp"
#endif

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes floating-point arithmetic helpers
namespace VARITH_FLOAT
{

struct FloatInstrInfo
{
    uint8_t rounding_mode = 0U;  //!< Floating point rounding mode
    bool cvt_vs2_is_int = false; //!< Whether vs2 contains integers in float conversion instructions
    bool cvt_rtz = false;        //!< Whether conversion is truncating
    bool ncvt_rod = false;       //!< Whether round-towards-odd is used in narrowing float-float conversion
};

using FloatFunction = std::function<bool(uint64_t, uint64_t, SVElement &, size_t)>;
using FloatConversionFunction = std::function<void(uint64_t /* opL */, SVElement & /* vd */, size_t /* sew */,
                                                   bool /* signed_x */, bool /* rtz */, bool /* rod */)>;

/*
============================================================================================================
The following code is copied from or heavily inspired by https://github.com/ics-jku/riscv-vp-plusplus
Therefore, this marks the start of the following copyright notice:
Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
Copyright (c) 2022-2023 Intitute for Complex Systems, Johannes Kepler University Linz <ics-office@jku.at>
============================================================================================================
*/

/* 13.2. Vector Single-Width Floating-Point Add/Subtract Instructions */
inline FloatFunction add = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction sub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction rsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};
/* End 13.2. */

/* 13.3. Vector Widening Floating-Point Add/Subtract Instructions */
inline FloatFunction wadd = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction wsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

// Wide vs2 (2*SEW)
inline FloatFunction wadd_w = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

// Wide vs2 (2*SEW)
inline FloatFunction wsub_w = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};
/* End 13.3. */

/* 13.4. Vector Single-Width Floating-Point Multiply/Divide Instructions */
inline FloatFunction mul = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction div = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction rdiv = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};
/* End 13.4. */

/* 13.5. Vector Widening Floating-Point Multiply */
inline FloatFunction wmul = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};
/* End 13.5. */

/* 13.6. Vector Single-Width Floating-Point Fused Multiply-Add Instructions */
inline FloatFunction macc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction nmacc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction msac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction nmsac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction madd = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_mulAdd(f16(vd.to_u64()), f16(rhs), f16(opL)).v;
        break;
    case 32:
        vd = f32_mulAdd(f32(vd.to_u64()), f32(rhs), f32(opL)).v;
        break;
    case 64:
        vd = f64_mulAdd(f64(vd.to_u64()), f64(rhs), f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};

inline FloatFunction nmadd = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction msub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction nmsub = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};
/* End 13.6. */

/* 13.7. Vector Widening Floating-Point Fused Multiply-Add Instructions */
inline FloatFunction wmacc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction wnmacc = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction wmsac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};

inline FloatFunction wnmsac = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
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
    return true;
};
/* End 13.7. */

/* 13.8. Vector Floating-Point Square-Root Instruction */
inline FloatFunction sqrt = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_sqrt(f16(opL)).v;
        break;
    case 32:
        vd = f32_sqrt(f32(opL)).v;
        break;
    case 64:
        vd = f64_sqrt(f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};
/* End 13.8. */

/* 13.9. Vector Floating-Point Reciprocal Square-Root Estimate Instruction */
inline FloatFunction rsqrt7 = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_rsqrte7(f16(opL)).v;
        break;
    case 32:
        vd = f32_rsqrte7(f32(opL)).v;
        break;
    case 64:
        vd = f64_rsqrte7(f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};
/* End 13.9. */

/* 13.10. Vector Floating-Point Reciprocal Estimate Instruction */
inline FloatFunction rec7 = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_recip7(f16(opL)).v;
        break;
    case 32:
        vd = f32_recip7(f32(opL)).v;
        break;
    case 64:
        vd = f64_recip7(f64(opL)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};
/* End 13.10. */

/* 13.11. Vector Floating-Point MIN/MAX Instructions */
inline FloatFunction min = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_min(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_min(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_min(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};

inline FloatFunction max = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_max(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_max(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_max(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};
/* End 13.11. */

/* 13.12. Vector Floating-Point Sign-Injection Instructions */
inline FloatFunction sgnj = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_sgnj(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_sgnj(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_sgnj(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};

inline FloatFunction sgnjn = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_sgnjn(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_sgnjn(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_sgnjn(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};

inline FloatFunction sgnjx = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = f16_sgnjx(f16(opL), f16(rhs)).v;
        break;
    case 32:
        vd = f32_sgnjx(f32(opL), f32(rhs)).v;
        break;
    case 64:
        vd = f64_sgnjx(f64(opL), f64(rhs)).v;
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};
/* End 13.12. */

/* 13.13. Vector Floating-Point Compare Instructions */
inline FloatFunction eq = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        return f16_eq(f16(opL), f16(rhs));
    case 32:
        return f32_eq(f32(opL), f32(rhs));
    case 64:
        return f64_eq(f64(opL), f64(rhs));
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction ne = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        return !f16_eq(f16(opL), f16(rhs));
    case 32:
        return !f32_eq(f32(opL), f32(rhs));
    case 64:
        return !f64_eq(f64(opL), f64(rhs));
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction lt = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        return f16_lt(f16(opL), f16(rhs));
    case 32:
        return f32_lt(f32(opL), f32(rhs));
    case 64:
        return f64_lt(f64(opL), f64(rhs));
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction le = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        return f16_le(f16(opL), f16(rhs));
    case 32:
        return f32_le(f32(opL), f32(rhs));
    case 64:
        return f64_le(f64(opL), f64(rhs));
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction gt = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        return f16_lt(f16(rhs), f16(opL));
    case 32:
        return f32_lt(f32(rhs), f32(opL));
    case 64:
        return f64_lt(f64(rhs), f64(opL));
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};

inline FloatFunction ge = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        return f16_le(f16(rhs), f16(opL));
    case 32:
        return f32_le(f32(rhs), f32(opL));
    case 64:
        return f64_le(f64(rhs), f64(opL));
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
};
/* End 13.13. */

/* 13.14. Vector Floating-Point Classify Instruction */
inline FloatFunction classify = [](uint64_t opL, uint64_t rhs, SVElement &vd, size_t sew) -> bool {
    switch (sew)
    {
    case 16:
        vd = 0 | f16_classify(f16(opL));
        break;
    case 32:
        vd = 0 | f32_classify(f32(opL));
        break;
    case 64:
        vd = 0 | f64_classify(f64(opL));
        break;
    default:
        // TODO: Illegal, check for better error handling
        exit(EXIT_FAILURE);
    }
    return true;
};
/* End 13.14. */

/* 13.17. Single-Width Floating-Point/Integer Type-Convert Instructions */
// Float to (un)signed int, same width
inline FloatConversionFunction convert_x_f = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x, bool rtz,
                                                bool rod = false) -> void {
    auto rounding_mode = rtz ? softfloat_round_minMag : softfloat_roundingMode;
    switch (sew)
    {
    case 16:
        vd = signed_x ? f16_to_i16(f16(opL), rounding_mode, true) : f16_to_ui16(f16(opL), rounding_mode, true);
        return;
    case 32:
        vd = signed_x ? f32_to_i32(f32(opL), rounding_mode, true) : f32_to_ui32(f32(opL), rounding_mode, true);
        return;
    case 64:
        vd = signed_x ? f64_to_i64(f64(opL), rounding_mode, true) : f64_to_ui64(f64(opL), rounding_mode, true);
        return;
    default:
        break;
    }
};

// (Un)signed int to float, same width
inline FloatConversionFunction convert_f_x = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x,
                                                bool rtz = false, bool rod = false) -> void {
    switch (sew)
    {
    case 16:
        vd = signed_x ? i32_to_f16(opL).v : ui32_to_f16(opL).v;
        return;
    case 32:
        vd = signed_x ? i32_to_f32(opL).v : ui32_to_f32(opL).v;
        return;
    case 64:
        vd = signed_x ? i64_to_f64(opL).v : ui64_to_f64(opL).v;
        return;
    default:
        break;
    }
};
/* End 13.17. */

/* 13.18. Widening Floating-Point/Integer Type-Convert Instructions */
// Float to (un)signed int, widening
inline FloatConversionFunction convert_widening_x_f = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x,
                                                         bool rtz, bool rod = false) -> void {
    auto rounding_mode = rtz ? softfloat_round_minMag : softfloat_roundingMode;
    switch (sew)
    {
    case 16:
        vd = signed_x ? f16_to_i32(f16(opL), rounding_mode, true) : f16_to_ui32(f16(opL), rounding_mode, true);
        return;
    case 32:
        vd = signed_x ? f32_to_i64(f32(opL), rounding_mode, true) : f32_to_ui64(f32(opL), rounding_mode, true);
        return;
    default:
        break;
    }
};

// (Un)signed int to float, widening
inline FloatConversionFunction convert_widening_f_x = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x,
                                                         bool rtz = false, bool rod = false) -> void {
    switch (sew)
    {
    case 16:
        vd = signed_x ? i32_to_f32(opL).v : ui32_to_f32(opL).v;
        return;
    case 32:
        vd = signed_x ? i32_to_f64(opL).v : ui32_to_f64(opL).v;
        return;
    default:
        break;
    }
};

// Float to float, widening                         pVSTART, pVm
inline FloatConversionFunction convert_widening_f_f = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x,
                                                         bool rtz = false, bool rod = false) -> void {
    switch (sew)
    {
    case 16:
        vd = f16_to_f32(f16(opL)).v;
        return;
    case 32:
        vd = f32_to_f64(f32(opL)).v;
        return;
    default:
        break;
    }
};
/* End 13.18. */

/* 13.19. Narrowing Floating-Point/Integer Type-Convert Instructions */
// Float to (un)signed int, narrowing
inline FloatConversionFunction convert_narrowing_x_f = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x,
                                                          bool rtz, bool rod = false) -> void {
    auto rounding_mode = rtz ? softfloat_round_minMag : softfloat_roundingMode;
    switch (sew)
    {
    case 16:
        vd = signed_x ? f32_to_i16(f32(opL), rounding_mode, true) : f32_to_ui16(f32(opL), rounding_mode, true);
        return;
    case 32:
        vd = signed_x ? f64_to_i32(f64(opL), rounding_mode, true) : f64_to_ui32(f64(opL), rounding_mode, true);
        return;
    default:
        break;
    }
};

// (Un)signed int to float, narrowing
inline FloatConversionFunction convert_narrowing_f_x = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x,
                                                          bool rtz = false, bool rod = false) -> void {
    switch (sew)
    {
    case 16:
        vd = signed_x ? i32_to_f16(opL).v : ui32_to_f16(opL).v;
        return;
    case 32:
        vd = signed_x ? i64_to_f32(opL).v : ui64_to_f32(opL).v;
        return;
    default:
        break;
    }
};

// Float to float, narrowing
inline FloatConversionFunction convert_narrowing_f_f = [](uint64_t opL, SVElement &vd, size_t sew, bool signed_x,
                                                          bool rtz, bool rod) -> void {
    if (rod)
    {
        softfloat_roundingMode = softfloat_round_odd;
    }
    switch (sew)
    {
    case 16:
        vd = f32_to_f16(f32(opL)).v;
        return;
    case 32:
        vd = f64_to_f32(f64(opL)).v;
        return;
    default:
        break;
    }
};
/* End 13.19. */

/*
============================================================================================================
End of the following copyright notice:
Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
Copyright (c) 2022-2023 Intitute for Complex Systems, Johannes Kepler University Linz <ics-office@jku.at>
============================================================================================================
*/

auto vf_op_vv(uint8_t *vec_reg_mem,                   //!< Vector register file memory space. One dimensional
              VInstrInfo const &v_instr_info,         //!< Struct containing vector instruction information
              FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
              uint16_t reg_vd,                        //!< Destination vector D [index]
              uint16_t reg_vs1,                       //!< Source vector R [index]
              uint16_t reg_vs2,                       //!< Source vector L [index]
              FloatFunction func                      //!< Floating-point function lambda
              ) -> VILL::vpu_return_t;

VILL::vpu_return_t vf_op_vf(uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
                            const VInstrInfo &v_instr_info, //!< Struct containing vector instruction information
                            FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
                            uint16_t reg_vd,                        //!< Destination vector D [index]
                            uint16_t reg_vs2,                       //!< Source vector R [index]
                            uint8_t *scalar_reg_mem,                //!< Source vector L [index]
                            uint8_t scalar_reg_len_bytes,
                            FloatFunction func //!< Floating-point function lambda
);

VILL::vpu_return_t vf_op_unary(
    uint8_t *vec_reg_mem,                   //!< Vector register file memory space. One dimensional
    const VInstrInfo &v_instr_info,         //!< Struct containing vector instruction information
    FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
    uint16_t reg_vd,                        //!< Destination vector D [index]
    uint16_t reg_vs2,                       //!< Source vector L [index]
    FloatFunction func                      //!< Floating-point function lambda
);

// Destination is register
VILL::vpu_return_t vf_op_vv_to_reg(
    uint8_t *vec_reg_mem,                   //!< Vector register file memory space. One dimensional
    const VInstrInfo &v_instr_info,         //!< Struct containing vector instruction information
    FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
    uint16_t reg_vd,                        //!< Destination vector D [index]
    uint16_t reg_vs1,                       //!< Source vector R [index]
    uint16_t reg_vs2,                       //!< Source vector L [index]
    FloatFunction func                      //!< Floating-point function lambda
);

// Destination is register
VILL::vpu_return_t vf_op_vf_to_reg(
    uint8_t *vec_reg_mem,                   //!< Vector register file memory space. One dimensional
    const VInstrInfo &v_instr_info,         //!< Struct containing vector instruction information
    FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
    uint16_t reg_vd,                        //!< Destination vector D [index]
    uint16_t reg_vs2,                       //!< Source vector R [index]
    uint8_t *scalar_reg_mem,                //!< Source vector L [index]
    uint8_t scalar_reg_len_bytes,
    FloatFunction func //!< Floating-point function lamb
);

VILL::vpu_return_t vf_merge(uint8_t *vec_reg_mem, const VInstrInfo &v_instr_info, uint16_t reg_vd, uint16_t reg_vs2,
                            uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vf_move(uint8_t *vec_reg_mem, const VInstrInfo &v_instr_info, uint16_t reg_vd,
                           uint8_t *scalar_reg_mem, uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vf_convert(
    uint8_t *vec_reg_mem,                   //!< Vector register file memory space. One dimensional
    const VInstrInfo &v_instr_info,         //!< Struct containing vector instruction information
    FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
    uint16_t reg_vd,                        //!< Destination vector D [index]
    uint16_t reg_vs2,                       //!< Source vector L [index]
    FloatConversionFunction func            //!< Conversion function

);

VILL::vpu_return_t vf_convert_wide(
    uint8_t *vec_reg_mem,                   //!< Vector register file memory space. One dimensional
    const VInstrInfo &v_instr_info,         //!< Struct containing vector instruction information
    FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
    uint16_t reg_vd,                        //!< Destination vector D [index]
    uint16_t reg_vs2,                       //!< Source vector L [index]
    FloatConversionFunction func            //!< Conversion function

);

VILL::vpu_return_t vf_convert_narrow(
    uint8_t *vec_reg_mem,                   //!< Vector register file memory space. One dimensional
    const VInstrInfo &v_instr_info,         //!< Struct containing vector instruction information
    FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
    uint16_t reg_vd,                        //!< Destination vector D [index]
    uint16_t reg_vs2,                       //!< Source vector L [index]
    FloatConversionFunction func            //!< Conversion function
);
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

};     // namespace VARITH_FLOAT
#endif /* __RVVHL_ARITH_FLOATINGPOINT_H__ */
