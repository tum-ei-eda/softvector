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
/// \file integer.cpp
/// \brief C++ Source for vector integer arithmetic helpers for RISC-V ISS
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "arithmetic/integer.hpp"
#include "base/base.hpp"
#include "vpu/softvector-types.hpp"
#include "base/softvector-platform-types.hpp"

VILL::vpu_return_t VARITH_INT::add_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_add(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::add_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_add(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::add_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_add(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sub_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sub_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::rsub_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_rhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_rsub(imm, vs2, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::rsub_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_rhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_rsub(imm, vs2, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f, bool dir_f, bool signed_f)
{
    // TODO: Remove nesting for returning branches
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {

        RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

        if (!VD.vec_reg_is_aligned(dst_vec_reg))
        {
            return (VILL::VPU_RETURN::DST_VEC_ILL);
        }

        V.init();
        VD.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = VD.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
        else if (vd.check_mem_overlap(vs1))
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, bool dir_f, bool signed_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {

        RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

        if (!VD.vec_reg_is_aligned(dst_vec_reg))
        {
            return (VILL::VPU_RETURN::DST_VEC_ILL);
        }

        V.init();
        VD.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = VD.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_wv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f, bool dir_f, bool signed_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else
    {

        RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

        if (!VD.vec_reg_is_aligned(dst_vec_reg))
        {
            return (VILL::VPU_RETURN::DST_VEC_ILL);
        }
        else if (!VD.vec_reg_is_aligned(src_vec_reg_lhs))
        {
            return (VILL::VPU_RETURN::SRC2_VEC_ILL);
        }

        V.init();
        VD.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = VD.get_vec(src_vec_reg_lhs);
        RVVector &vd = VD.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
        else if (vd.check_mem_overlap(vs1))
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::wop_wx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, bool dir_f, bool signed_f, uint8_t scalar_reg_len_bytes)
{

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {

        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }

        if (signed_f)
        {
            if (dir_f > 0)
                vd.m_wadd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
        else
        {
            if (dir_f > 0)
                vd.m_waddu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
            else
                vd.m_wsubu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        }
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.2. */

/* 11.3. Vector Integer Extension */
VILL::vpu_return_t VARITH_INT::vext_vf(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint16_t extension_encoding, uint16_t vec_elem_start,
                                       bool mask_f)
{
    int divider = 0;
    bool sign = extension_encoding & 1;
    extension_encoding >>= 1;
    if (extension_encoding == 1)
    {
        divider = 8;
    }
    else if (extension_encoding == 2)
    {
        divider = 4;
    }
    else
    {
        divider = 2;
    }

    // Source EEW: 1/8, 1/4, or 1/2 of SEW
    // Source EMUL: (EEW/SEW)*LMUL
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, (sew_bytes * 8) / divider, SVMul(emul_num, emul_denom * divider),
                  vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW (as is for vsext, vzext) if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom; // Destination EMUL = LMUL
    int n_allowed_overlaps = lmul / divider;
    int lowest_allowed_register = dst_vec_reg + lmul - n_allowed_overlaps;
    if ((emul_num < emul_denom * divider || src_vec_reg_lhs < lowest_allowed_register) &&
        vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
    }
    vd.m_vext(vs2, V.get_mask_reg(), !mask_f, sign, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.3. */

VILL::vpu_return_t VARITH_INT::and_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_and(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::and_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{

    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_and(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::and_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_and(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::or_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                     bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_or(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::or_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_or(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::or_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                     bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_or(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::xor_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_xor(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::xor_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_xor(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::xor_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_xor(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.6. Vector Single-Width Shift Instructions */
VILL::vpu_return_t VARITH_INT::sll_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sll(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sll_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t u_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sll(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sll_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sll(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::srl_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_srl(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::srl_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t u_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_srl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::srl_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_srl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sra_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                      bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sra(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sra_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t u_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::sra_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                      uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                      uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                      bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.6. */

/* 11.7. Vector Narrowing Integer Right Shift Instructions */
VILL::vpu_return_t VARITH_INT::vnsrl_wv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    // vd and vs1: EEW = SEW, EMUL = LMUL
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    // vs2: EEW = 2*SEW, EMUL = 2*LMUL
    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();
    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsrl.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsrl(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsrl_wi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t u_imm,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    uint64_t imm = u_imm & 0x1F;
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsrl.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsrl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsrl_wx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsrl.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsrl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsra_wv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    // vd and vs1: EEW = SEW, EMUL = LMUL
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    // vs2: EEW = 2*SEW, EMUL = 2*LMUL
    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();
    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsra.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsra(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsra_wi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t u_imm,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VS.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    uint64_t imm = u_imm & 0x1F;
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsra.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnsra_wx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    RVVRegField VS(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    V.init();
    VS.init();

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = VS.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    // Overlap rules for narrowing instructions:
    // Destination and source (vs2) can overlap if the overlap is in the lowest-numbered part of the source register
    // group, e.g., when LMUL=1, vnsra.wi v0, v0, 3 is legal, but a destination of v1 is not.
    if (src_vec_reg_lhs != dst_vec_reg && vd.check_mem_overlap(vs2) != 0)
    {
        return (VILL::VPU_RETURN::NARROWING_OVERLAP_VD_VS2_ILL);
    }

    vd.m_nsra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.7. */

VILL::vpu_return_t VARITH_INT::mseq_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_eq(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mseq_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_eq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mseq_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_eq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msne_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();
        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_neq(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msne_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        ;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_neq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msne_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_neq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msltu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msltu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mslt_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mslt_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msleu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lte(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msleu_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t u_imm,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = u_imm & 0x1F;
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msleu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msle_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lte(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msle_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msle_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);
        SVRegister v(vs2 <= imm);

        vd.m_s_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgtu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_gt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgtu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgtu_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t s_imm,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_u_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgt_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_gt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgt_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::msgt_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_s_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.10. Vector Single-Width Integer Multiply Instructions */
VILL::vpu_return_t VARITH_INT::vmul_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmul_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmul(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmulh_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmulh(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
VILL::vpu_return_t VARITH_INT::vmulh_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmulh(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmulhu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumulh(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
VILL::vpu_return_t VARITH_INT::vmulhu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumulh(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmulhsu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                          uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                          uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                          uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sumulh(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
VILL::vpu_return_t VARITH_INT::vmulhsu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                          uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                          uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                          uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sumulh(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.11. Vector Single-Width Integer Divide Instructions */
VILL::vpu_return_t VARITH_INT::vdiv_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssdiv(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vdiv_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssdiv(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vdivu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uudiv(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vdivu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uudiv(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vrem_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssrem(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vrem_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssrem(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vremu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uurem(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vremu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                   : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uurem(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.11. */

/* 11.12. Vector Widening Integer Multiply Instructions */
/* 2*SEW product from SEW*SEW */

// enum class Widening_Mul_Type
VILL::vpu_return_t VARITH_INT::vwmul_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f, VWMUL_TYPE vwmul_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs1) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmul_type)
    {
    case VWMUL_TYPE::S_S:
        vd.m_ssmul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::U_U:
        vd.m_uumul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::S_U:
        vd.m_sumul(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vwmul_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes,
                                        VWMUL_TYPE vwmul_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmul_type)
    {
    case VWMUL_TYPE::S_S:
        vd.m_ssmul(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::U_U:
        vd.m_uumul(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMUL_TYPE::S_U:
        vd.m_sumul(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.12. */

/* Regular vector min and max instructions */
VILL::vpu_return_t VARITH_INT::vmax_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmax(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmax_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmax(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmaxu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumax(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmaxu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumax(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmin_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                       bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmin(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmin_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                       uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                       uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                                       bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmin(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vminu_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumin(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vminu_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_uumin(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

/* 11.16. Vector Integer Move Instructions */
VILL::vpu_return_t VARITH_INT::mv_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint16_t src_vec_reg, uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_assign(vs1, V.get_mask_reg(), false, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mv_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                     uint8_t *scalar_reg_mem, uint16_t vec_elem_start, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_assign(imm, V.get_mask_reg(), false, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::mv_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                     uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg, uint8_t s_imm,
                                     uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_assign(imm, V.get_mask_reg(), false, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.16. */

/* 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions */
VILL::vpu_return_t VARITH_INT::vadc_vvm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_adc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vadc_vim(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t s_imm,
                                        uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_adc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vadc_vxm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_adc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_madc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadc_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t s_imm,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_madc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_madc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vsbc_vvm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sbc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vsbc_vxm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_sbc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmsbc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_msbc(vs2, vs1, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmsbc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        SVRegister &vd = V.get_vecreg(dst_vec_reg);

        vd.m_msbc(vs2, imm, V.get_mask_reg(), vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.4 */

/* 11.13. Vector Single-Width Integer Multiply-Add Instructions */
VILL::vpu_return_t VARITH_INT::vmacc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmacc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_ssmacc(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsac_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsac(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsac_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsac(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadd_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                        uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_madd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmadd_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                        uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                        uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                        uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_madd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsub_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vnmsub_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    else if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }
    else
    {
        V.init();

        int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                                  : *(reinterpret_cast<int32_t *>(scalar_reg_mem));
        RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
        RVVector &vd = V.get_vec(dst_vec_reg);

        vd.m_nmsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.13. */

/* 11.14. Vector Widening Integer Multiply-Add Instructions */
VILL::vpu_return_t VARITH_INT::vwmacc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start, bool mask_f, VWMACC_TYPE vwmacc_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_rhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs1) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
        }
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmacc_type)
    {
    case VWMACC_TYPE::S_S:
        vd.m_ssmacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_U:
        vd.m_uumacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::S_U:
        vd.m_sumacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_S:
        vd.m_sumacc(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }
    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vwmacc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, bool mask_f, uint8_t scalar_reg_len_bytes,
                                         VWMACC_TYPE vwmacc_type)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }

    RVVRegField VD(vec_reg_len_bytes * 8, vec_len, 2 * sew_bytes * 8, SVMul(2 * emul_num, emul_denom), vec_reg_mem);
    if (!VD.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();
    VD.init();

    uint64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<uint64_t *>(scalar_reg_mem))
                                               : *(reinterpret_cast<uint32_t *>(scalar_reg_mem));
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = VD.get_vec(dst_vec_reg);

    // RVV1.0: Overlap allowed when destination EEW > source EEW if:
    // - source EMUL >= 1
    // - the overlap is in the highest-numbered part of the destination register group (e.g., when LMUL=8, vzext.vf4 v0,
    // v6 is legal, but a source of v0, v2, or v4 is not).
    int lmul = emul_num / emul_denom;
    // Source EMUL = LMUL, destination EMUL = 2*LMUL
    int lowest_allowed_register = dst_vec_reg + lmul;
    if (emul_num < emul_denom || src_vec_reg_lhs < lowest_allowed_register)
    {
        if (vd.check_mem_overlap(vs2) != 0)
        {
            return (VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);
        }
    }

    switch (vwmacc_type)
    {
    case VWMACC_TYPE::S_S:
        vd.m_ssmacc(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_U:
        vd.m_uumacc(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::S_U:
        vd.m_sumacc(vs2, static_cast<int64_t>(imm), V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    case VWMACC_TYPE::U_S:
        vd.m_usmacc(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
        break;
    default:
        // TODO: Does this need error handling?
        break;
    }

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.14. */

/* 11.15. Vector Integer Merge Instructions */
VILL::vpu_return_t VARITH_INT::vmerge_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs,
                                         uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC1_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs1 = V.get_vec(src_vec_reg_rhs);
    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    vd.m_merge(vs2, vs1, V.get_mask_reg(), vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmerge_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem,
                                         uint16_t vec_elem_start, uint8_t scalar_reg_len_bytes)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);
    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);
    RVVector &vd = V.get_vec(dst_vec_reg);

    int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t *>(scalar_reg_mem))
                                              : *(reinterpret_cast<int32_t *>(scalar_reg_mem));

    vd.m_merge(vs2, imm, V.get_mask_reg(), vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH_INT::vmerge_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom,
                                         uint16_t sew_bytes, uint16_t vec_len, uint16_t vec_reg_len_bytes,
                                         uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs, uint8_t s_imm,
                                         uint16_t vec_elem_start)
{
    RVVRegField V(vec_reg_len_bytes * 8, vec_len, sew_bytes * 8, SVMul(emul_num, emul_denom), vec_reg_mem);

    if (!V.vec_reg_is_aligned(src_vec_reg_lhs))
    {
        return (VILL::VPU_RETURN::SRC2_VEC_ILL);
    }
    if (!V.vec_reg_is_aligned(dst_vec_reg))
    {
        return (VILL::VPU_RETURN::DST_VEC_ILL);
    }

    V.init();

    RVVector &vs2 = V.get_vec(src_vec_reg_lhs);

    int64_t imm = static_cast<int64_t>(s_imm & 0x10 ? s_imm | ~0x1F : s_imm);
    RVVector &vd = V.get_vec(dst_vec_reg);

    vd.m_merge(vs2, imm, V.get_mask_reg(), vec_elem_start);

    return (VILL::VPU_RETURN::NO_EXCEPT);
}
/* End 11.15. */