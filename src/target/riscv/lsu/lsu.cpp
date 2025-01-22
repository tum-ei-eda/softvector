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

    V.init();

    RVVector &vd = V.get_vec(dst_vec_reg);
    size_t memOffset = src_mem_start;
    for (size_t iElement = 0; iElement < vec_len; ++iElement)
    {
        if (iElement >= vec_elem_start && (mask_f || V.get_mask_reg().get_bit(iElement)))
        {
            func_read_mem(memOffset, vd[iElement].mem_, eew_bytes);
        }
        memOffset += stride_bytes;
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

    V.init();

    RVVector &vs3 = V.get_vec(src_vec_reg);
    size_t memOffset = dst_mem_start;
    for (size_t iElement = 0; iElement < vec_len; ++iElement)
    {
        if (iElement >= vec_elem_start && (mask_f || V.get_mask_reg().get_bit(iElement)))
        {
            func_write_mem(memOffset, vs3[iElement].mem_, eew_bytes);
        }
        memOffset += stride_bytes;
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

auto VLSU::load_indices(
    std::function<void(size_t, uint8_t *, size_t)> func_read_mem, //!< Function for memory read access
    uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional [0..32*VLEN-1] byte array
    VInstrInfo const &v_instr_info, //!< Struct containing vector instruction information
    uint16_t reg_vd,                //!< Destination vector [index]
    uint16_t reg_vs2,               //!< Index source vector [index]
    uint64_t src_mem_start,         //!< Source memory start address
    uint16_t eew                    //!< Effective element width [bits]
    ) -> VILL::vpu_return_t
{
    RVVRegField V_dest(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                       SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto const emul_num = eew * v_instr_info.lmul_denom;
    auto const emul_denom = v_instr_info.sew * v_instr_info.lmul_denom;

    RVVRegField V_indices(v_instr_info.vector_register_length, v_instr_info.vector_length, eew,
                          SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V_dest.vec_reg_is_aligned(reg_vd))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    if (!V_indices.vec_reg_is_aligned(reg_vs2))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V_dest.init();
    V_indices.init();

    RVVector &vd = V_dest.get_vec(reg_vd);
    RVVector &vs2 = V_indices.get_vec(reg_vs2);

    auto const eew_bytes = eew >> 3;
    auto const sew_bytes = v_instr_info.sew >> 3;

    for (size_t i = 0; i < v_instr_info.vector_length; ++i)
    {
        // TODO: Overflow possible? Checking?
        auto index_offset = vs2[i].to_i64();
        size_t mem_offset = src_mem_start + index_offset;
        if (i >= v_instr_info.start_element && (!v_instr_info.masked || V_dest.get_mask_reg().get_bit(i)))
        {
            func_read_mem(mem_offset, vd[i].mem_, sew_bytes);
        }
    }

    return VILL::VPU_RETURN::NO_EXCEPT;
}

auto VLSU::store_indices(
    std::function<void(size_t, uint8_t *, size_t)> func_write_mem, //!< Function for memory read access
    uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional [0..32*VLEN-1] byte array
    VInstrInfo const &v_instr_info, //!< Struct containing vector instruction information
    uint16_t reg_vs3,               //!< Destination vector [index]
    uint16_t reg_vs2,               //!< Index source vector [index]
    uint64_t dst_mem_start,         //!< Source memory start address
    uint16_t eew                    //!< Effective element width [bits]
    ) -> VILL::vpu_return_t
{
    RVVRegField V_src(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                      SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto const emul_num = eew * v_instr_info.lmul_denom;
    auto const emul_denom = v_instr_info.sew * v_instr_info.lmul_denom;

    RVVRegField V_indices(v_instr_info.vector_register_length, v_instr_info.vector_length, eew,
                          SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V_src.vec_reg_is_aligned(reg_vs3))
    {
        return (VILL::VPU_RETURN::SRC3_VEC_ILL);
    }
    if (!V_indices.vec_reg_is_aligned(reg_vs2))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V_src.init();
    V_indices.init();

    RVVector &vs3 = V_src.get_vec(reg_vs3);
    RVVector &vs2 = V_indices.get_vec(reg_vs2);

    auto const eew_bytes = eew >> 3;
    auto const sew_bytes = v_instr_info.sew >> 3;

    for (size_t i = 0; i < v_instr_info.vector_length; ++i)
    {
        // TODO: Overflow possible? Checking?
        size_t memOffset = dst_mem_start + vs2[i].to_i64();
        if (i >= v_instr_info.start_element && (!v_instr_info.masked || V_src.get_mask_reg().get_bit(i)))
        {
            func_write_mem(memOffset, vs3[i].mem_, sew_bytes);
        }
    }

    return VILL::VPU_RETURN::NO_EXCEPT;
}
