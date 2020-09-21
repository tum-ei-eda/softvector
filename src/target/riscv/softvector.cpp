/*
 * Copyright [2020] [Technical University of Munich]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//////////////////////////////////////////////////////////////////////////////////////
/// \file softvector.cpp
/// \brief C++ Source for ETISS JIT libary or independent C application. Implements a C interface when compiled to library beforehand
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "softvector.h"
#include "base/base.hpp"
#include "lsu/lsu.hpp"
#include "arithmetic/integer.hpp"
#include "arithmetic/floatingpoint.hpp"
#include "arithmetic/fixedpoint.hpp"
#include "misc/mask.hpp"
#include "misc/permutation.hpp"
#include "misc/reduction.hpp"
extern "C"
{

int8_t vtype_decode(uint16_t vtype, uint8_t* ta, uint8_t* ma, uint32_t* sew, uint8_t* z_lmul, uint8_t* n_lmul){
	return (VTYPE::decode(vtype, ta , ma, sew, z_lmul, n_lmul));
}

uint16_t vtype_encode(uint16_t sew, uint8_t z_lmul, uint8_t n_lmul, uint8_t ta, uint8_t ma){
	return VTYPE::encode(sew, z_lmul, n_lmul, ta, ma);
}

uint8_t vtype_extractSEW(uint16_t pVTYPE)
{
	return VTYPE::extractSEW(pVTYPE);
}

uint8_t vtype_extractLMUL(uint16_t pVTYPE)
{
	return VTYPE::extractLMUL(pVTYPE);
}

uint8_t vtype_extractTA(uint16_t pVTYPE)
{
	return VTYPE::extractTA(pVTYPE);
}

uint8_t vtype_extractMA(uint16_t pVTYPE)
{
	return VTYPE::extractMA(pVTYPE);
}

uint16_t vcfg_concatEEW(uint8_t mew, uint8_t width){
	return (VTYPE::concatEEW(mew, width));
}

uint8_t vload_encoded_unitstride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t  pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*8) || (_z_emul > _n_emul*8)) return 1;

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_readMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)buff[i] = pM[addr+i];
	};
	VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd, pMSTART, pVSTART, pVm, 0);

	return (0);
}

uint8_t vload_encoded_stride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t  pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pSTRIDE)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*8) || (_z_emul > _n_emul*8)){
		return 1;
	}

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_readMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)buff[i] = pM[addr+i];
	};

	VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd, pMSTART, pVSTART, pVm, pSTRIDE);

	return (0);
}

uint8_t vload_segment_unitstride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*pNF*8) || (_z_emul*pNF > _n_emul*8)) return 1;
	if ((pVd + pNF*_z_emul/_n_emul) > 32) return 1;
	if (pVSTART >= pVL) return (0);

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_readMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)buff[i] = pM[addr+i];
	};

	uint16_t _vstart = pVSTART;
	uint64_t _moffset = pMSTART;

	for(int i = 0; i< pNF; ++i){
		VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd + (i*_z_emul/_n_emul), _moffset, _vstart, pVm, 0);
		_moffset+= (pVL-_vstart)*pEEW/8;
		_vstart = 0;
	}

	return (0);
}

uint8_t vload_segment_stride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pSTRIDE)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*pNF*8) || (_z_emul*pNF > _n_emul*8)) return 1;
	if ((pVd + pNF*_z_emul/_n_emul) > 32) return 1;
	if (pVSTART >= pVL) return (0);

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_readMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)buff[i] = pM[addr+i];
	};

	uint16_t _vstart = pVSTART;
	uint64_t _moffset = pMSTART;

	for(int i = 0; i< pNF; ++i){
		_moffset = pMSTART + i*pEEW/8;
		VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd + (i*_z_emul/_n_emul), _moffset, _vstart, pVm, pSTRIDE);
		_vstart = 0;
	}

	return (0);
}

uint8_t vstore_encoded_unitstride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*8) || (_z_emul > _n_emul*8)) return 1;

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_writeMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)pM[addr+i] = buff[i];
	};

	VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd, pMSTART, pVSTART, pVm, 0);

	return (0);
}

uint8_t vstore_encoded_stride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pStride)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*8) || (_z_emul > _n_emul*8)) return 1;

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_writeMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)pM[addr+i] = buff[i];
	};
	VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd, pMSTART, pVSTART, pVm, pStride);

	return (0);
}

uint8_t vstore_segment_unitstride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*pNF*8) || (_z_emul*pNF > _n_emul*8)) return 1;
	if ((pVd + pNF*_z_emul/_n_emul) > 32) return 1;
	if (pVSTART >= pVL) return (0);

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_writeMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)pM[addr+i] = buff[i];
	};

	uint16_t _vstart = pVSTART;
	uint64_t _moffset = pMSTART;

	for(int i = 0; i< pNF; ++i){
		VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd + (i*_z_emul/_n_emul), _moffset, _vstart, pVm, 0);
		_moffset+= (pVL-_vstart)*pEEW/8;
		_vstart = 0;
	}

	return (0);
}

uint8_t vstore_segment_stride(
	uint8_t* pV,
	uint8_t* pM,
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pStride)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint64_t _z_emul = pEEW*_vt._z_lmul;
	uint64_t _n_emul = _vt._sew*_vt._n_lmul;

	if ((_n_emul > _z_emul*pNF*8) || (_z_emul*pNF > _n_emul*8)) return 1;
	if ((pVd + pNF*_z_emul/_n_emul) > 32) return 1;
	if (pVSTART >= pVL) return (0);

	uint8_t* VectorRegField = pV;
	std::function<void(size_t, uint8_t*, size_t)> f_writeMem = [pM](size_t addr, uint8_t* buff, size_t len) {
		for (size_t i = 0; i<len; ++i)pM[addr+i] = buff[i];
	};

	uint16_t _vstart = pVSTART;
	uint64_t _moffset = pMSTART;
	for(int i = 0; i< pNF; ++i){
		_moffset = pMSTART + i*pEEW/8;
		VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW/8, pVL, pVLEN/8, pVd + (i*_z_emul/_n_emul), _moffset, _vstart, pVm, 0);
		_moffset+= (pVL-_vstart)*pEEW/8;
	   _vstart = 0;
	}

	return (0);
}

/* Vector Arithmetic functions*/

uint8_t vadd_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::add_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vadd_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::add_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vadd_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::add_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vsub_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::sub_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vsub_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::sub_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vwaddu_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, true, false);

	return (0);
}

uint8_t vwadd_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, true, true);

	return (0);
}

uint8_t vwsubu_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, false, false);

	return (0);
}

uint8_t vwsub_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, false, true);

	return (0);
}

uint8_t vwaddu_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, true, false, pXLEN/8);

	return (0);
}

uint8_t vwadd_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, true, true, pXLEN/8);

	return (0);
}

uint8_t vwsubu_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, false, false, pXLEN/8);

	return (0);
}

uint8_t vwsub_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, false, true, pXLEN/8);

	return (0);
}

uint8_t vwaddu_w_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, true, false);

	return (0);
}

uint8_t vwadd_w_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, true, true);

	return (0);
}

uint8_t vwsubu_w_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, false, false);

	return (0);
}

uint8_t vwsub_w_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm, false, true);

	return (0);
}

uint8_t vwaddu_w_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, true, false, pXLEN/8);

	return (0);
}

uint8_t vwadd_w_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, true, true, pXLEN/8);

	return (0);
}

uint8_t vwsubu_w_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, false, false, pXLEN/8);

	return (0);
}

uint8_t vwsub_w_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, false, true, pXLEN/8);

	return (0);
}

uint8_t vand_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::and_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vand_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::and_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vand_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::and_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vor_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;
	VARITH_INT::or_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vor_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::or_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vor_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::or_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vxor_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::xor_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vxor_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::xor_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vxor_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::xor_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vsll_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::sll_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vsll_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::sll_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vsll_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::sll_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vsrl_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::srl_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vsrl_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::srl_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vsrl_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::srl_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vsra_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::sra_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vsra_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::sra_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vsra_vx(
	uint8_t* pV,
	  uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::sra_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmseq_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::mseq_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmseq_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::mseq_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vmseq_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::mseq_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmsne_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msne_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmsne_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msne_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vmsne_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::msne_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmsltu_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msltu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmsltu_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::msltu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmslt_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::mslt_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmslt_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::mslt_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmsleu_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msleu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmsleu_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msleu_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vmsleu_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::msleu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmsle_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msle_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmsle_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msle_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vmsle_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::msle_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmsgtu_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msgtu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmsgtu_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::msgtu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmsgt_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::msgt_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART, pVm);

	return (0);
}

uint8_t vmsgt_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::msgt_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vmv_vv(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL
) {
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::mv_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVSTART);

	return (0);
}

uint8_t vmv_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVd, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL
) {
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VARITH_INT::mv_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVimm, pVSTART);

	return (0);
}

uint8_t vmv_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN
) {
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VARITH_INT::mv_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, ScalarReg, pVSTART, pXLEN/8);

	return (0);
}

uint8_t vmv_xs(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRd*4]);
	else ScalarReg = &(pR[pRd*8]);

	VPERM::mv_xs(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVs2, ScalarReg, pXLEN/8);

	return (0);
}

uint8_t vmv_sx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VPERM::mv_sx(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVd, ScalarReg, pVSTART, pXLEN/8);

	return (0);
}

uint8_t vfmv_fs(
	uint8_t* pV,
	uint8_t* pF,
	uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pF[pRd*4]);
	else ScalarReg = &(pF[pRd*8]);

	VPERM::fmv_fs(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVs2, ScalarReg, pXLEN/8);

	return (0);
}

uint8_t vfmv_sf(
	uint8_t* pV,
	uint8_t* pF,
	uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pF[pRs1*4]);
	else ScalarReg = &(pF[pRs1*8]);

	VPERM::fmv_sf(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVd, ScalarReg, pVSTART, pXLEN/8);

	return (0);
}

uint8_t vslideup_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VPERM::slideup_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vslideup_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VPERM::slideup_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vslidedown_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VPERM::slidedown_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vslidedown_vi(
	uint8_t* pV,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
{
	VTYPE::VTYPE _vt(pVTYPE);

	uint8_t* VectorRegField = pV;

	VPERM::slidedown_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm, pVSTART, pVm);

	return (0);
}

uint8_t vslide1up_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VPERM::slide1up(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vfslide1up(
	uint8_t* pV,
	uint8_t* pF,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pF[pRs1*4]);
	else ScalarReg = &(pF[pRs1*8]);

	VPERM::fslide1up(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vslide1down_vx(
	uint8_t* pV,
	uint8_t* pR,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pR[pRs1*4]);
	else ScalarReg = &(pR[pRs1*8]);

	VPERM::slide1down(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}

uint8_t vfslide1down(
	uint8_t* pV,
	uint8_t* pF,
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
{
	VTYPE::VTYPE _vt(pVTYPE);
	uint8_t* ScalarReg;
	uint8_t* VectorRegField;

	VectorRegField = pV;
	if(pXLEN <= 32) ScalarReg = &(pF[pRs1*4]);
	else ScalarReg = &(pF[pRs1*8]);

	VPERM::fslide1down(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg, pVSTART, pVm, pXLEN/8);

	return (0);
}


} // extern "C"
