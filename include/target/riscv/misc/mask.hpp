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
/// \file mask.hpp
/// \brief Defines helpers implementing masking operations after
/// https://github.com/riscv/riscv-v-spec/blob/0.9/v-spec.adoc#vector-arithmetic-instruction-formats
/// \date 09/09/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_MISC_MASK_H__
#define __RVVHL_MISC_MASK_H__

#include <functional>

#include "stdint.h"
#include "base/base.hpp"

using MaskFunction = std::function<bool(bool /* vs1_bit */, bool /* vs2_bit */)>;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes arithmetic helpers
namespace VMASK
{

inline MaskFunction logical_and = [](bool vs2_bit, bool vs1_bit) -> bool { return vs2_bit & vs1_bit; };

inline MaskFunction logical_nand = [](bool vs2_bit, bool vs1_bit) -> bool { return !(vs2_bit & vs1_bit); };

inline MaskFunction logical_andn = [](bool vs2_bit, bool vs1_bit) -> bool { return vs2_bit & (!vs1_bit); };

inline MaskFunction logical_xor = [](bool vs2_bit, bool vs1_bit) -> bool { return vs2_bit ^ vs1_bit; };

inline MaskFunction logical_or = [](bool vs2_bit, bool vs1_bit) -> bool { return vs2_bit | vs1_bit; };

inline MaskFunction logical_nor = [](bool vs2_bit, bool vs1_bit) -> bool { return !(vs2_bit | vs1_bit); };

inline MaskFunction logical_orn = [](bool vs2_bit, bool vs1_bit) -> bool { return vs2_bit | (!vs1_bit); };

inline MaskFunction logical_xnor = [](bool vs2_bit, bool vs1_bit) -> bool { return !(vs2_bit ^ vs1_bit); };

auto mask_op_logical(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
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
                     MaskFunction func           //!< Mask function
                     ) -> VILL::vpu_return_t;

auto mask_op_to_scalar(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                       uint64_t emul_num,            //!< Register multiplicity numerator
                       uint64_t emul_denom,          //!< Register multiplicity denominator
                       uint16_t sew_bytes,           //!< Element width [bytes]
                       uint16_t vec_len,             //!< Vector length [elements]
                       uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                       uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                       uint8_t *dst_scalar_reg,      //!< Scalar destination register
                       uint16_t vec_elem_start,      //!< Starting element [index]
                       bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                       uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                       bool is_vcpop) -> VILL::vpu_return_t;

auto mask_op_sxf(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                 uint64_t emul_num,          //!< Register multiplicity numerator
                 uint64_t emul_denom,        //!< Register multiplicity denominator
                 uint16_t sew_bytes,         //!< Element width [bytes]
                 uint16_t vec_len,           //!< Vector length [elements]
                 uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                 uint16_t dst_vec_reg,       //!< Destination vector D [index]
                 uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                 uint16_t vec_elem_start,    //!< Starting element [index]
                 bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                 bool including_first,       //!< Set bit on first found 1
                 bool only_first             //!< Set bit only on first found 1
                 ) -> VILL::vpu_return_t;

auto mask_viota(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                uint64_t emul_num,          //!< Register multiplicity numerator
                uint64_t emul_denom,        //!< Register multiplicity denominator
                uint16_t sew_bytes,         //!< Element width [bytes]
                uint16_t vec_len,           //!< Vector length [elements]
                uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                uint16_t dst_vec_reg,       //!< Destination vector D [index]
                uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                uint16_t vec_elem_start,    //!< Starting element [index]
                bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
                ) -> VILL::vpu_return_t;

auto mask_vid(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
              uint64_t emul_num,          //!< Register multiplicity numerator
              uint64_t emul_denom,        //!< Register multiplicity denominator
              uint16_t sew_bytes,         //!< Element width [bytes]
              uint16_t vec_len,           //!< Vector length [elements]
              uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
              uint16_t dst_vec_reg,       //!< Destination vector D [index]
              uint16_t vec_elem_start,    //!< Starting element [index]
              bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
              ) -> VILL::vpu_return_t;

} // namespace VMASK
#endif /* __RVVHL_MISC_MASK_H__ */
