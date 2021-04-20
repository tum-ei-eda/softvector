/*
 * Copyright [2020] [Technical University of Munich]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	 http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//////////////////////////////////////////////////////////////////////////////////////
/// \file softvector.h
/// \brief C/C++ Header for JIT libary or independent C application
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __SOFTVECTOR_H__
#define __SOFTVECTOR_H__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Vector Configuration Helpers*/
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Decode a VTYPE bitfield and store retrieved fields to Output parameter set
/// \return If field valid 1, else -1 (e.g. reserved LMUL code)
int8_t vtype_decode(
	uint16_t vtype, 	//!<[in] vtype bitfield
	uint8_t* ta, 		//!<[out] tail agnostic flag
	uint8_t* ma, 		//!<[out] mask agnostic flag
	uint32_t* sew, 		//!<[out] SEW (decoded) [bits]
	uint8_t* z_lmul, 	//!<[out] LMUL nominator
	uint8_t* n_lmul		//!<[out] LMUL denominator
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Encode Input parameter set of bitfields to a VTYPE bitfield
/// \return Encoded VTYPE bitfield
uint16_t vtype_encode(
	uint16_t sew, 	//!<[in] SEW (decoded) [bits]
	uint8_t z_lmul, 	//!<[in] LMUL nominator
	uint8_t n_lmul, 	//!<[in] LMUL denominator
	uint8_t ta, 		//!<[in] tail agnostic flag
	uint8_t ma		//!<[in] tail mask flag
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract SEW bitfield from VTYPE bitfield
/// \return Encoded SEW bitfield
uint8_t vtype_extractSEW(
	uint16_t pVTYPE //!<[in] vtype bitfield
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract LMUL bitfield from VTYPE bitfield
/// \return Encoded LMUL bitfield
uint8_t vtype_extractLMUL(
	uint16_t pVTYPE //!<[in] vtype bitfield
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract TA bitfield from VTYPE bitfield
/// \return Encoded TA bitfield
uint8_t vtype_extractTA(
	uint16_t pVTYPE //!<[in] vtype bitfield
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract MA bitfield from VTYPE bitfield
/// \return Encoded MA bitfield
uint8_t vtype_extractMA(
	uint16_t pVTYPE //!<[in] vtype bitfield
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Concatenate MEW and WIDTH to EEW and return number of bits for EEW
/// \return Decoded EEW [bits]
uint16_t vcfg_concatEEW(uint8_t mew, 	//!<[in] MEW bit
	uint8_t width	//!<[in] WIDTH bits
);

/* Vector Loads/Stores Helpers*/
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Load encoded (unitstride) from memory to target vector (-group)
/// \return 0 if no exception triggered, else 1
uint8_t vload_encoded_unitstride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t  pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Load encoded (strided) from memory to target vector (-group)
/// \return 0 if no exception triggered, else 1
uint8_t vload_encoded_stride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t  pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pSTRIDE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Load seqgmented (unitstride) from memory to target vector (-group)
/// \return 0 if no exception triggered, else 1
uint8_t vload_segment_unitstride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Load encoded (strided) from memory to target vector (-group)
/// \return 0 if no exception triggered, else 1
uint8_t vload_segment_stride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pSTRIDE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Store encoded (unitstride) source vector (-group) to memory
/// \return 0 if no exception triggered, else 1
uint8_t vstore_encoded_unitstride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Store encoded (strided) source vector (-group) to memory
/// \return 0 if no exception triggered, else 1
uint8_t vstore_encoded_stride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pStride);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Store segmented (unitstride) source vector (-group) to memory
/// \return 0 if no exception triggered, else 1
uint8_t vstore_segment_unitstride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Store segmented (strided) source vector (-group) to memory
/// \return 0 if no exception triggered, else 1
uint8_t vstore_segment_stride(
	void* pV,						//!<[inout] Vector register field as local memory
	uint8_t* pM,						//!<[inout] Local memory
	uint16_t pVTYPE, uint8_t  pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pStride);

/* Vector Arithmetic Helpers*/
//////////////////////////////////////////////////////////////////////////////////////
/// \brief ADD vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vadd_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief ADD vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vadd_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief ADD vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vadd_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SUB vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vsub_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SUB vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vsub_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector unsigned ADD vector-vector. SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwaddu_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector signed ADD vector-vector. SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwadd_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector unsigned SUB vector-vector. SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsubu_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector signed SUB vector-vector. SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsub_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar unsigned ADD SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwaddu_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar signed ADD SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwadd_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar unsigned SUB SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsubu_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar signed SUB SEW*2: SEW op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsub_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector unsigned ADD vector-vector. SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwaddu_w_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector signed ADD vector-vector. SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwadd_w_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector unsigned SUB vector-vector. SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsubu_w_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-vector signed SUB vector-vector. SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsub_w_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar unsigned ADD SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwaddu_w_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar signed ADD SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwadd_w_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar unsigned SUB SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsubu_w_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening vector-scalar signed SUB SEW*2: SEW*2 op SEW
/// \return 0 if no exception triggered, else 1
uint8_t vwsub_w_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief AND vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vand_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief AND vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vand_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief AND vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vand_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief OR vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vor_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief OR vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vor_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief OR vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vor_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);


//////////////////////////////////////////////////////////////////////////////////////
/// \brief XOR vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vxor_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief XOR vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vxor_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief XOR vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vxor_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SLL vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vsll_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SLL vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vsll_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SLL vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vsll_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRL vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vsrl_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRL vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vsrl_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRL vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vsrl_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRA vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vsra_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRA vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vsra_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRA vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vsra_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSEQ vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vmseq_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSEQ vector-immediate
/// \return 0 if no exception triggered, else 1
uint8_t vmseq_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSEQ vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vmseq_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSNE vector-vector (not equal)
/// \return 0 if no exception triggered, else 1
uint8_t vmsne_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSNE vector-immediate (not equal)
/// \return 0 if no exception triggered, else 1
uint8_t vmsne_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSNE vector-vector (not equal)
/// \return 0 if no exception triggered, else 1
uint8_t vmsne_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLTU vector-vector (less than unsigned)
/// \return 0 if no exception triggered, else 1
uint8_t vmsltu_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLTU vector-scalar (less than unsigned)
/// \return 0 if no exception triggered, else 1
uint8_t vmsltu_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLT vector-vector (less than signed)
/// \return 0 if no exception triggered, else 1
uint8_t vmslt_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLT vector-scalar (less than signed)
/// \return 0 if no exception triggered, else 1
uint8_t vmslt_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLEU vector-vector (less than or equal unsigned)
/// \return 0 if no exception triggered, else 1
uint8_t vmsleu_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLEU vector-immediate (less than or equal unsigned)
/// \return 0 if no exception triggered, else 1
uint8_t vmsleu_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLEU vector-scalar (less than or equal unsigned)
/// \return 0 if no exception triggered, else 1
uint8_t vmsleu_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLE vector-vector (less than or equal signed)
/// \return 0 if no exception triggered, else 1
uint8_t vmsle_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLE vector-immediate (less than or equal signed)
/// \return 0 if no exception triggered, else 1
uint8_t vmsle_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSLE vector-scalar (less than or equal signed)
/// \return 0 if no exception triggered, else 1
uint8_t vmsle_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSGTU vector-vector (greater than unsigned)
/// \return 0 if no exception triggered, else 1
uint8_t vmsgtu_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSGTU vector-scalar (greater than unsigned)
/// \return 0 if no exception triggered, else 1
uint8_t vmsgtu_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSGT vector-vector (greater than signed)
/// \return 0 if no exception triggered, else 1
uint8_t vmsgt_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief MSGTU vector-scalar (greater than signed)
/// \return 0 if no exception triggered, else 1
uint8_t vmsgt_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);


//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move/Copy vector to vector
/// \return 0 if no exception triggered, else 1
uint8_t vmv_vv(
	void* pV,						//!<[in] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move scalar (signed immediate) to vector
/// \return 0 if no exception triggered, else 1
uint8_t vmv_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVd, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move (signed) scalar register to vector
/// \return 0 if no exception triggered, else 1
uint8_t vmv_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

/* PERMUTATION */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move first element (0) of a vector register A to scalar register X := SEW>XLEN ? A[0] : sext(A[0])
/// \return 0 if no exception triggered, else 1
uint8_t vmv_xs(
	void* pV,						//!<[in] Vector register field as local memory
	void* pR,						//!<[inout] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move scalar register to first element (0) of a vector register A[0] := SEW<XLEN ? X : sext(X)
/// \return 0 if no exception triggered, else 1
uint8_t vmv_sx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move first element (0) of a vector register A to scalar fp register F := SEW>XLEN ? A[0] : sext(A[0])
/// \return 0 if no exception triggered, else 1
uint8_t vfmv_fs(
	void* pV,						//!<[in] Vector register field as local memory
	void* pF,						//!<[inout] Floating point register field
	uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move scalar fp register to first element (0) of a vector register A[0] := SEW<XLEN ? F : sext(F)
/// \return 0 if no exception triggered, else 1
uint8_t vfmv_sf(
	void* pV,						//!<[in] Vector register field as local memory
	void* pF,						//!<[inout] Floating point register field
	uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslideup.vx vd, vs2, rs1, vm        # vd[i+rs1] = vs2[i]
/// \return 0 if no exception triggered, else 1
uint8_t vslideup_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslideup.vi vd, vs2, uimm[4:0], vm        # vd[i+rs1] = vs2[i]
/// \return 0 if no exception triggered, else 1
uint8_t vslideup_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslidedown.vx vd, vs2, rs1, vm        # vd[i+rs1] = vs2[i]
/// \return 0 if no exception triggered, else 1
uint8_t vslidedown_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslidedown.vi vd, vs2, uimm[4:0], vm        # vd[i+rs1] = vs2[i]
/// \return 0 if no exception triggered, else 1
uint8_t vslidedown_vi(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslide1up.vx vd, vs2, rs1, vm        # vd[0]=x[rs1], vd[i+1] = vs2[i]
/// \return 0 if no exception triggered, else 1
uint8_t vslide1up(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vfslide1up.vf vd, vs2, fs1, vm        # vd[0]=f[rs1], vd[i+1] = vs2[i]
/// \return 0 if no exception triggered, else 1
uint8_t vfslide1up(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pF,						//!<[inout] Floating point register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslide1down.vx  vd, vs2, rs1, vm      # vd[i] = vs2[i+1], vd[vl-1]=x[rs1]
/// \return 0 if no exception triggered, else 1
uint8_t vslide1down(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vfslide1down.vf vd, vs2, fs1, vm      # vd[i] = vs2[i+1], vd[vl-1]=f[rs1]
/// \return 0 if no exception triggered, else 1
uint8_t vfslide1down(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pF,						//!<[inout] Floating point register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
/////////////////////////////////////////////////////////////////////////////////////
/// \brief ADD vector-vector
/// \return 0 if no exception triggered, else 1
uint8_t vmul_vv(
	void* pV,						//!<[inout] Vector register field as local memory
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief ADD vector-scalar
/// \return 0 if no exception triggered, else 1
uint8_t vmul_vx(
	void* pV,						//!<[inout] Vector register field as local memory
	void* pR,						//!<[in] Integer/General Purpose register field
	uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __SOFTVECTOR_H__ */
