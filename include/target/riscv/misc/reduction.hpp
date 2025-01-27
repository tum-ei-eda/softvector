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
/// \file reduction.hpp
/// \brief Defines helpers implementing reduction operations after
/// https://github.com/riscv/riscv-v-spec/blob/0.9/v-spec.adoc#vector-arithmetic-instruction-formats
/// \date 09/09/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_MISC_REDUCTION_H_
#define __RVVHL_MISC_REDUCTION_H_

#include <functional>
#include <algorithm>

#include "stdint.h"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "arithmetic/integer.hpp"
#include "arithmetic/floatingpoint.hpp"

#ifdef ETISS_SOFTFLOAT
extern "C"
{
#include "softfloat_orig.h"
}
#else
#include "softfloat.hpp"
#endif

#include "arithmetic/softfloat-extension.hpp"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes vector reduction operations helpers
namespace VREDUC
{

using ReductionFunction =
    std::function<void(SVElement & /* opL */, SVElement & /* vd */, bool /* is_signed */, size_t sew)>;

inline ReductionFunction red_sum = [](SVElement &vsx_element, SVElement &vd, bool is_signed, size_t sew) -> void {
    auto res = is_signed ? vd.to_i64() : vd.to_u64();
    res += is_signed ? vsx_element.to_i64() : vsx_element.to_u64();
    vd = res;
};

inline ReductionFunction red_max = [](SVElement &vsx_element, SVElement &vd, bool is_signed, size_t sew) -> void {
    if (is_signed)
    {
        auto res = vd.to_i64();
        auto vsx_element_value = vsx_element.to_i64();
        res = std::max(res, vsx_element_value);
        vd = res;
        return;
    }
    auto res = vd.to_u64();
    auto vsx_element_value = vsx_element.to_u64();
    res = std::max(res, vsx_element_value);
    vd = res;
};

inline ReductionFunction red_min = [](SVElement &vsx_element, SVElement &vd, bool is_signed, size_t sew) -> void {
    if (is_signed)
    {
        auto res = vd.to_i64();
        auto vsx_element_value = vsx_element.to_i64();
        res = std::min(res, vsx_element_value);
        vd = res;
        return;
    }
    auto res = vd.to_u64();
    auto vsx_element_value = vsx_element.to_u64();
    res = std::min(res, vsx_element_value);
    vd = res;
};

inline ReductionFunction red_logical_and = [](SVElement &vsx_element, SVElement &vd, bool is_signed,
                                              size_t sew) -> void {
    auto res = is_signed ? vd.to_i64() : vd.to_u64();
    res &= is_signed ? vsx_element.to_i64() : vsx_element.to_u64();
    vd = res;
};

inline ReductionFunction red_logical_or = [](SVElement &vsx_element, SVElement &vd, bool is_signed,
                                             size_t sew) -> void {
    auto res = is_signed ? vd.to_i64() : vd.to_u64();
    res |= is_signed ? vsx_element.to_i64() : vsx_element.to_u64();
    vd = res;
};

inline ReductionFunction red_logical_xor = [](SVElement &vsx_element, SVElement &vd, bool is_signed,
                                              size_t sew) -> void {
    auto res = is_signed ? vd.to_i64() : vd.to_u64();
    res ^= is_signed ? vsx_element.to_i64() : vsx_element.to_u64();
    vd = res;
};

inline ReductionFunction red_float_sum = [](SVElement &vsx_element, SVElement &vd, bool is_signed, size_t sew) -> void {
    auto res = vd.to_u64();
    auto vsx_element_value = vsx_element.to_u64();
    switch (sew)
    {
    case 16:
        vd = f16_add(f16(res), f16(vsx_element_value)).v;
        break;
    case 32:
        vd = f32_add(f32(res), f32(vsx_element_value)).v;
        break;
    case 64:
        vd = f64_add(f64(res), f64(vsx_element_value)).v;
        break;
    default:
        break;
    }
};

inline ReductionFunction red_float_max = [](SVElement &vsx_element, SVElement &vd, bool is_signed, size_t sew) -> void {
    auto res = vd.to_u64();
    auto vsx_element_value = vsx_element.to_u64();
    switch (sew)
    {
    case 16:
        vd = f16_max(f16(res), f16(vsx_element_value)).v;
        break;
    case 32:
        vd = f32_max(f32(res), f32(vsx_element_value)).v;
        break;
    case 64:
        vd = f64_max(f64(res), f64(vsx_element_value)).v;
        break;
    default:
        break;
    }
};

inline ReductionFunction red_float_min = [](SVElement &vsx_element, SVElement &vd, bool is_signed, size_t sew) -> void {
    auto res = vd.to_u64();
    auto vsx_element_value = vsx_element.to_u64();
    switch (sew)
    {
    case 16:
        vd = f16_min(f16(res), f16(vsx_element_value)).v;
        break;
    case 32:
        vd = f32_min(f32(res), f32(vsx_element_value)).v;
        break;
    case 64:
        vd = f64_min(f64(res), f64(vsx_element_value)).v;
        break;
    default:
        break;
    }
};

inline ReductionFunction red_widening_float_sum = [](SVElement &vsx_element, SVElement &vd, bool is_signed,
                                                     size_t sew) -> void {
    auto res = vd.to_u64();
    auto vsx_element_value = vsx_element.to_u64();
    switch (sew)
    {
    case 16:
        vd = f32_add(f16_to_f32(f16(res)), f16_to_f32(f16(vsx_element_value))).v;
        break;
    case 32:
        vd = f64_add(f32_to_f64(f32(res)), f32_to_f64(f32(vsx_element_value))).v;
        break;
    default:
        break;
    }
};

auto red_op_int(uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
                VInstrInfo const &v_instr_info, //!< Struct containing vector instruction information
                uint16_t reg_vd,                //!< Destination vector D [index]
                uint16_t reg_vs1,               //!< Source vector R [index]
                uint16_t reg_vs2,               //!< Source vector L [index]
                VARITH_INT::IntFunction func    //!< Reduction function
                ) -> VILL::vpu_return_t;

auto red_op_float(
    uint8_t *vec_reg_mem,                                 //!< Vector register file memory space. One dimensional
    VInstrInfo const &v_instr_info,                       //!< Struct containing vector instruction information
    VARITH_FLOAT::FloatInstrInfo const &float_instr_info, //!< Struct containing float instruction information
    uint16_t reg_vd,                                      //!< Destination vector D [index]
    uint16_t reg_vs1,                                     //!< Source vector R [index]
    uint16_t reg_vs2,                                     //!< Source vector L [index]
    VARITH_FLOAT::FloatFunction func                      //!< Reduction function
    ) -> VILL::vpu_return_t;

} // namespace VREDUC
#endif /* __RVVHL_MISC_REDUCTION_H_ */
