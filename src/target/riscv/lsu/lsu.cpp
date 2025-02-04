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

#include <vector>
#include <algorithm>

#include "base/base.hpp"
#include "base/softvector-platform-types.hpp"
#include "vpu/softvector-types.hpp"
#include "lsu/lsu.hpp"

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

auto VLSU::load_indices(std::function<void(size_t, uint8_t *, size_t)> func_read_mem, uint8_t *vec_reg_mem,
                        VInstrInfo const &v_instr_info, uint16_t reg_vd, uint16_t reg_vs2, uint64_t src_mem_start,
                        uint16_t eew) -> VILL::vpu_return_t
{
    RVVRegField V_dest(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                       SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto const emul_num = eew * v_instr_info.lmul_num;
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
        auto index_offset = vs2[i].to_u64();
        size_t mem_offset = src_mem_start + index_offset;
        if (i >= v_instr_info.start_element && (!v_instr_info.masked || V_dest.get_mask_reg().get_bit(i)))
        {
            func_read_mem(mem_offset, vd[i].mem_, sew_bytes);
        }
    }

    return VILL::VPU_RETURN::NO_EXCEPT;
}

auto VLSU::store_indices(std::function<void(size_t, uint8_t *, size_t)> func_write_mem, uint8_t *vec_reg_mem,
                         VInstrInfo const &v_instr_info, uint16_t reg_vs3, uint16_t reg_vs2, uint64_t dst_mem_start,
                         uint16_t eew, uint8_t nf) -> VILL::vpu_return_t
{
    RVVRegField V_src(v_instr_info.vector_register_length, v_instr_info.vector_length, v_instr_info.sew,
                      SVMul(v_instr_info.lmul_num, v_instr_info.lmul_denom), vec_reg_mem);

    auto const emul_num = eew * v_instr_info.lmul_num;
    auto const emul_denom = v_instr_info.sew * v_instr_info.lmul_denom;

    RVVRegField V_indices(v_instr_info.vector_register_length, v_instr_info.vector_length, eew,
                          SVMul(emul_num, emul_denom), vec_reg_mem);

    for (size_t i = 0; i < nf; i++)
    {
        auto reg = reg_vs3 + std::max(i, i * (v_instr_info.lmul_num / v_instr_info.lmul_denom));
        if (!V_src.vec_reg_is_aligned(reg))
        {
            return (VILL::VPU_RETURN::SRC3_VEC_ILL);
        }
    }
    if (!V_indices.vec_reg_is_aligned(reg_vs2))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V_src.init();
    V_indices.init();

    auto vectors = std::vector<std::reference_wrapper<RVVector>>();
    for (size_t i = 0; i < nf; i++)
    {
        auto reg = reg_vs3 + std::max(i, i * (v_instr_info.lmul_num / v_instr_info.lmul_denom));
        RVVector &v = V_src.get_vec(reg);
        vectors.push_back(v);
    }

    // RVVector &vs3 = V_src.get_vec(reg_vs3);
    RVVector &vs2 = V_indices.get_vec(reg_vs2);

    auto const eew_bytes = eew >> 3;
    auto const sew_bytes = v_instr_info.sew >> 3;

    for (size_t i = 0; i < v_instr_info.vector_length; ++i)
    {
        if (i >= v_instr_info.start_element && (!v_instr_info.masked || V_src.get_mask_reg().get_bit(i)))
        {
            size_t mem_offset = dst_mem_start + vs2[i].to_u64();
            for (size_t field = 0; field < nf; field++)
            {
                func_write_mem(mem_offset, vectors[field].get()[i].mem_, sew_bytes);
                mem_offset += sew_bytes;
            }
        }
    }

    return VILL::VPU_RETURN::NO_EXCEPT;
}
