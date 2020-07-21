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

VILL::vpu_return_t VARITH::add_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_add(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::add_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();

		int64_t imm = (s_imm <= 31 ) ? s_imm : static_cast<int8_t>(s_imm | uint8_t(~0x1F));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_add(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::add_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_add(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::sub_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_sub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::sub_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_sub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::rsub_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_rhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_rsub(imm, vs2, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::rsub_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();

		int64_t imm = (s_imm <= 31 ) ? s_imm : static_cast<int8_t>(s_imm | uint8_t(~0x1F));
		RVVector& vs2 = V.get_vec(src_vec_reg_rhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_rsub(imm, vs2, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::wop_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f,
	bool dir_f,
	bool signed_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else {
		
		RVVRegField VD(vec_reg_len_bytes*8, vec_len, 2*sew_bytes*8, SVMul(2*emul_num, emul_denom), vec_reg_mem);
		
		if (! VD.vec_reg_is_aligned(dst_vec_reg) ) {
			return(VILL::VPU_RETURN::DST_VEC_ILL);	
		}
				
		V.init();
		VD.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = VD.get_vec(dst_vec_reg);
		
		if(vd.check_mem_overlap(vs2) != 0){
			return(VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);	
		} else if (vd.check_mem_overlap(vs1) ) {
			return(VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
		}

		if(signed_f){
			if(dir_f > 0)
				vd.m_wadd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
		} else{
			if(dir_f > 0)
				vd.m_waddu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsubu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
		}
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::wop_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	bool dir_f,
	bool signed_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else {
		
		RVVRegField VD(vec_reg_len_bytes*8, vec_len, 2*sew_bytes*8, SVMul(2*emul_num, emul_denom), vec_reg_mem);
		
		if (! VD.vec_reg_is_aligned(dst_vec_reg) ) {
			return(VILL::VPU_RETURN::DST_VEC_ILL);	
		}
				
		V.init();
		VD.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = VD.get_vec(dst_vec_reg);
		
		if(vd.check_mem_overlap(vs2) != 0){
			return(VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);	
		} 

		if(signed_f){
			if(dir_f > 0)
				vd.m_wadd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
		} else{
			if(dir_f > 0)
				vd.m_waddu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsubu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
		}
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::wop_wv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f,
	bool dir_f,
	bool signed_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else {
		
		RVVRegField VD(vec_reg_len_bytes*8, vec_len, 2*sew_bytes*8, SVMul(2*emul_num, emul_denom), vec_reg_mem);
		
		if (! VD.vec_reg_is_aligned(dst_vec_reg) ) {
			return(VILL::VPU_RETURN::DST_VEC_ILL);	
		} else if (! VD.vec_reg_is_aligned(src_vec_reg_lhs) ) {
			return(VILL::VPU_RETURN::SRC2_VEC_ILL);
		}
				
		V.init();
		VD.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = VD.get_vec(src_vec_reg_lhs);
		RVVector& vd = VD.get_vec(dst_vec_reg);
		
		if(vd.check_mem_overlap(vs2) != 0){
			return(VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);	
		} else if (vd.check_mem_overlap(vs1) ) {
			return(VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS1_ILL);
		}

		if(signed_f){
			if(dir_f > 0)
				vd.m_wadd(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsub(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
		} else{
			if(dir_f > 0)
				vd.m_waddu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsubu(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
		}
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::wop_wx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	bool dir_f,
	bool signed_f,
	uint8_t scalar_reg_len_bytes
)
{
	
	RVVRegField V(vec_reg_len_bytes*8, vec_len, 2*sew_bytes*8, SVMul(2*emul_num, emul_denom), vec_reg_mem);

	if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else {
		
		V.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);
		
		if(vd.check_mem_overlap(vs2) != 0){
			return(VILL::VPU_RETURN::WIDENING_OVERLAP_VD_VS2_ILL);	
		}
		
		if(signed_f){
			if(dir_f > 0)
				vd.m_wadd(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsub(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
		} else{
			if(dir_f > 0)
				vd.m_waddu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
			else
				vd.m_wsubu(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
		}
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

VILL::vpu_return_t VARITH::and_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_and(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::and_vi(
	uint8_t* vec_reg_mem,	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
	)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (s_imm <= 31 ) ? s_imm : static_cast<int8_t>(s_imm | uint8_t(~0x1F));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_and(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::and_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_and(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::or_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_or(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::or_vi(
	uint8_t* vec_reg_mem,	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (s_imm <= 31 ) ? s_imm : static_cast<int8_t>(s_imm | uint8_t(~0x1F));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);
		
		vd.m_or(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::or_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_or(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::xor_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_xor(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::xor_vi(
	uint8_t* vec_reg_mem,	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (s_imm <= 31 ) ? s_imm : static_cast<int8_t>(s_imm | uint8_t(~0x1F));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_xor(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::xor_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		int64_t imm = (scalar_reg_len_bytes > 32) ? *(reinterpret_cast<int64_t*>(scalar_reg_mem)) : *(reinterpret_cast<int32_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_xor(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::sll_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);
		
		vd.m_sll(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}



VILL::vpu_return_t VARITH::sll_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t u_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = u_imm&0x1F;
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_sll(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::sll_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_sll(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}



VILL::vpu_return_t VARITH::srl_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_srl(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}



VILL::vpu_return_t VARITH::srl_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t u_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = u_imm;
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_srl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::srl_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_srl(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

	
VILL::vpu_return_t VARITH::sra_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_sra(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::sra_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t u_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = u_imm;
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_sra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::sra_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		RVVector& vd = V.get_vec(dst_vec_reg);

		vd.m_sra(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::mseq_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_eq(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::mseq_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = s_imm;
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_eq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::mseq_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_eq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msne_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_neq(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msne_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = s_imm;
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_neq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msne_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_neq(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msltu_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_u_lt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msltu_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_u_lt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::mslt_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_s_lt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::mslt_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_s_lt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msleu_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);

		vd.m_u_lte(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msleu_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t u_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = u_imm;
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_u_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msleu_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_u_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msle_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_s_lte(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msle_vi(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t s_imm,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
			
		uint64_t imm = s_imm;
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_s_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msle_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
			
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		SVRegister v( vs2 <= imm);

		vd.m_s_lte(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msgtu_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_u_gt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msgtu_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
			
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_u_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msgt_vv(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_rhs,
	uint16_t src_vec_reg_lhs,
	uint16_t vec_elem_start,
	bool mask_f
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if(! V.vec_reg_is_aligned(src_vec_reg_rhs) ) {
		return(VILL::VPU_RETURN::SRC1_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	}  else {
		V.init();
		
		RVVector& vs1 = V.get_vec(src_vec_reg_rhs);
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_s_gt(vs2, vs1, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}


VILL::vpu_return_t VARITH::msgt_vx(
	uint8_t* vec_reg_mem,
	uint64_t emul_num,
	uint64_t emul_denom,
	uint16_t sew_bytes,
	uint16_t vec_len,
	uint16_t vec_reg_len_bytes,
	uint16_t dst_vec_reg,
	uint16_t src_vec_reg_lhs,
	uint8_t* scalar_reg_mem,
	uint16_t vec_elem_start,
	bool mask_f,
	uint8_t scalar_reg_len_bytes
)
{
	RVVRegField V(vec_reg_len_bytes*8, vec_len, sew_bytes*8, SVMul(emul_num, emul_denom), vec_reg_mem);
	
	if (! V.vec_reg_is_aligned(src_vec_reg_lhs) ) {
		return(VILL::VPU_RETURN::SRC2_VEC_ILL);
	} else if (! V.vec_reg_is_aligned(dst_vec_reg) ) {
		return(VILL::VPU_RETURN::DST_VEC_ILL);	
	} else {
		V.init();
		
		uint64_t imm = *(reinterpret_cast<uint64_t*>(scalar_reg_mem));
		RVVector& vs2 = V.get_vec(src_vec_reg_lhs);
		SVRegister& vd = V.get_vecreg(dst_vec_reg);
		
		vd.m_s_gt(vs2, imm, V.get_mask_reg(), !mask_f, vec_elem_start);
	}
	return(VILL::VPU_RETURN::NO_EXCEPT);
}

