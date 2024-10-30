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
/// \file lsu.hpp
/// \brief Defines helpers implementing load/stores after
/// https://github.com/riscv/riscv-v-spec/blob/0.9/v-spec.adoc#vector-loads-and-stores 
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_VLSU_H__
#define __RVVHL_VLSU_H__

#include "stdint.h"
#include <functional>
#include "base/base.hpp"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes load-store helpers
namespace VLSU
{

/* EEW-based */
//////////////////////////////////////////////////////////////////////////////////////
/// @brief Load <vl>-times <eew>-elements through readMem function into vector register file
VILL::vpu_return_t load_eew(
    std::function<void(size_t, uint8_t *, size_t)> func_read_mem, //!< Function for memory read access
    uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional [0..32*VLEN-1] byte array
    uint64_t emul_num,          //!< Effective register multiplicity numerator
    uint64_t emul_denom,        //!< Effective register multiplicity denominator
    uint16_t eew_bytes,         //!< Effective element width [bytes]
    uint16_t vec_len,           //!< Vector length [elements]
    uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
    uint16_t dst_vec_reg,       //!< Destination vector [index]
    uint64_t src_mem_start,     //!< Source memory start address
    uint16_t vec_elem_start,    //!< Starting element [index]
    uint8_t mask_f,             //!< Vector mask flag. 1: masking 0: no masking
    int16_t stride_bytes        //!< Stride length [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Store <vl>-times <eew>-elements through func_write_mem function from vector register file
VILL::vpu_return_t store_eew(
    std::function<void(size_t, uint8_t *, size_t)> func_write_mem, //!< Function for memory write access
    uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional [0..32*VLEN-1] byte array
    uint64_t emul_num,          //!< Effective register multiplicity numerator
    uint64_t emul_denom,        //!< Effective register multiplicity denominator
    uint16_t eew_bytes,         //!< Effective element width [bytes]
    uint16_t vec_len,           //!< Vector length [elements]
    uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
    uint16_t src_vec_reg,       //!< Source vector register [index]
    uint64_t dst_mem_start,     //!< Destination memory start address
    uint16_t vec_elem_start,    //!< Starting element [index]
    uint8_t mask_f,             //!< Vector mask flag. 1: masking 0: no masking
    int16_t stride_bytes        //!< Stride length [bytes]
);

} // namespace VLSU

#endif /* __RVVHL_VLSU_H__ */
