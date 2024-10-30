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
/// \file lsu.cpp
/// \brief C++ Source for vector load/store helpers for RISC-V ISS
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "lsu/lsu.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

VILL::vpu_return_t VLSU::load_eew(std::function<void(size_t, uint8_t *, size_t)> func_read_mem, uint8_t *vec_reg_mem,
                                  uint64_t emul_num, uint64_t emul_denom, uint16_t eew_bytes, uint16_t vec_len,
                                  uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg, uint64_t src_mem_start,
                                  uint16_t vec_elem_start, uint8_t mask_f, int16_t stride_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, eew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vd = V.get_vec(dst_vec_reg);
        size_t memOffset = src_mem_start;
        for (size_t iElement = 0; iElement < vec_len; ++iElement)
        {
            if (iElement >= vec_elem_start && (mask_f || V.get_mask_reg().get_bit(iElement)))
            {
                func_read_mem(memOffset, vd[iElement].mem_, eew_bytes);
            }
            memOffset += (eew_bytes + stride_bytes);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VLSU::store_eew(std::function<void(size_t, uint8_t *, size_t)> func_write_mem, uint8_t *vec_reg_mem,
                                   uint64_t emul_num, uint64_t emul_denom, uint16_t eew_bytes, uint16_t vec_len,
                                   uint16_t vec_reg_len_bytes, uint16_t src_vec_reg, uint64_t dst_mem_start,
                                   uint16_t vec_elem_start, uint8_t mask_f, int16_t stride_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, eew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg))
    {
        return (VILL::VPU_RETURN::SRC3_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs3 = V.get_vec(src_vec_reg);
        size_t memOffset = dst_mem_start;
        for (size_t iElement = 0; iElement < vec_len; ++iElement)
        {
            if (iElement >= vec_elem_start && (mask_f || V.get_mask_reg().get_bit(iElement)))
            {
                func_write_mem(memOffset, vs3[iElement].mem_, eew_bytes);
            }
            memOffset += (eew_bytes + stride_bytes);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
