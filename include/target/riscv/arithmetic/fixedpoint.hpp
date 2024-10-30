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

#include "stdint.h"
#include "base/base.hpp"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes fixed-point arithmetic helpers
namespace VARITH_FIXP
{

/* rvv spec. 13.1 - Vector Single-Width Saturating Add and Substract */
// TODO: ...
/* rvv spec. 13.2 - Vector Single-Width Averaging Add and Substract */
// TODO: ...
/* rvv spec. 13.3 - Vector Single-Width Fractional Multiply with Rounding and Saturation */
// TODO: ...
/* rvv spec. 13.4 - Vector Single-Width Scaling Shift Instructions */
// TODO: ...
/* rvv spec. 13.5 - Vector Narrowing Fixed-Point Clip Instructions */
// TODO: ...

} // namespace VARITH_FIXP
#endif /* __RVVHL_ARITH_FIXEDPOINT_H__ */
