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
extern "C"
{
#endif

    /* Vector Configuration Helpers*/
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Decode a VTYPE bitfield and store retrieved fields to Output parameter set
    /// \return If field valid 1, else -1 (e.g. reserved LMUL code)
    int8_t vtype_decode(uint16_t vtype,  //!<[in] vtype bitfield
                        uint8_t *ta,     //!<[out] tail agnostic flag
                        uint8_t *ma,     //!<[out] mask agnostic flag
                        uint32_t *sew,   //!<[out] SEW (decoded) [bits]
                        uint8_t *z_lmul, //!<[out] LMUL nominator
                        uint8_t *n_lmul  //!<[out] LMUL denominator
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Encode Input parameter set of bitfields to a VTYPE bitfield
    /// \return Encoded VTYPE bitfield
    uint16_t vtype_encode(uint16_t sew,   //!<[in] SEW (decoded) [bits]
                          uint8_t z_lmul, //!<[in] LMUL nominator
                          uint8_t n_lmul, //!<[in] LMUL denominator
                          uint8_t ta,     //!<[in] tail agnostic flag
                          uint8_t ma      //!<[in] tail mask flag
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract SEW bitfield from VTYPE bitfield
    /// \return Encoded SEW bitfield
    uint8_t vtype_extractSEW(uint16_t pVTYPE //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract LMUL bitfield from VTYPE bitfield
    /// \return Encoded LMUL bitfield
    uint8_t vtype_extractLMUL(uint16_t pVTYPE //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract TA bitfield from VTYPE bitfield
    /// \return Encoded TA bitfield
    uint8_t vtype_extractTA(uint16_t pVTYPE //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract MA bitfield from VTYPE bitfield
    /// \return Encoded MA bitfield
    uint8_t vtype_extractMA(uint16_t pVTYPE //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Concatenate MEW and WIDTH to EEW and return number of bits for EEW
    /// \return Decoded EEW [bits]
    uint16_t vcfg_concatEEW(uint8_t mew,  //!<[in] MEW bit
                            uint8_t width //!<[in] WIDTH bits
    );

    /* Vector Loads/Stores Helpers*/
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load encoded (unitstride) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_encoded_unitstride(void *pV,    //!<[inout] Vector register field as local memory
                                     uint8_t *pM, //!<[inout] Local memory
                                     uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART,
                                     uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load encoded (strided) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_encoded_stride(void *pV,    //!<[inout] Vector register field as local memory
                                 uint8_t *pM, //!<[inout] Local memory
                                 uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART,
                                 uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pSTRIDE);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load seqgmented (unitstride) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_segment_unitstride(void *pV,    //!<[inout] Vector register field as local memory
                                     uint8_t *pM, //!<[inout] Local memory
                                     uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd,
                                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load encoded (strided) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_segment_stride(void *pV,    //!<[inout] Vector register field as local memory
                                 uint8_t *pM, //!<[inout] Local memory
                                 uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd,
                                 uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pSTRIDE);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store encoded (unitstride) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_encoded_unitstride(void *pV,    //!<[inout] Vector register field as local memory
                                      uint8_t *pM, //!<[inout] Local memory
                                      uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART,
                                      uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store encoded (strided) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_encoded_stride(void *pV,    //!<[inout] Vector register field as local memory
                                  uint8_t *pM, //!<[inout] Local memory
                                  uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd, uint16_t pVSTART,
                                  uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pStride);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store segmented (unitstride) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_segment_unitstride(void *pV,    //!<[inout] Vector register field as local memory
                                      uint8_t *pM, //!<[inout] Local memory
                                      uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd,
                                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store segmented (strided) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_segment_stride(void *pV,    //!<[inout] Vector register field as local memory
                                  uint8_t *pM, //!<[inout] Local memory
                                  uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF, uint8_t pVd,
                                  uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pStride);

    /* Vector Arithmetic Helpers*/

    /* 11.1. Vector Single-Width Integer Add and Subtract */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief ADD vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vadd_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief ADD vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vadd_vi(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief ADD vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vadd_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SUB vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vsub_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SUB vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vsub_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reverse SUB vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vrsub_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reverse SUB vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vrsub_vi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);
    /* End 11.1. */

    /* 11.2. Vector Widening Integer Add/Subtract */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector unsigned ADD vector-vector. SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwaddu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector signed ADD vector-vector. SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwadd_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector unsigned SUB vector-vector. SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsubu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector signed SUB vector-vector. SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsub_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar unsigned ADD SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwaddu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar signed ADD SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwadd_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar unsigned SUB SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsubu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar signed SUB SEW*2: SEW op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsub_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector unsigned ADD vector-vector. SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwaddu_w_vv(void *pV, //!<[inout] Vector register field as local memory
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector signed ADD vector-vector. SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwadd_w_vv(void *pV, //!<[inout] Vector register field as local memory
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector unsigned SUB vector-vector. SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsubu_w_vv(void *pV, //!<[inout] Vector register field as local memory
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-vector signed SUB vector-vector. SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsub_w_vv(void *pV, //!<[inout] Vector register field as local memory
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar unsigned ADD SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwaddu_w_vx(void *pV, //!<[inout] Vector register field as local memory
                        void *pR, //!<[in] Integer/General Purpose register field
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar signed ADD SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwadd_w_vx(void *pV, //!<[inout] Vector register field as local memory
                       void *pR, //!<[in] Integer/General Purpose register field
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar unsigned SUB SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsubu_w_vx(void *pV, //!<[inout] Vector register field as local memory
                        void *pR, //!<[in] Integer/General Purpose register field
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening vector-scalar signed SUB SEW*2: SEW*2 op SEW
    /// \return 0 if no exception triggered, else 1
    uint8_t vwsub_w_vx(void *pV, //!<[inout] Vector register field as local memory
                       void *pR, //!<[in] Integer/General Purpose register field
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.2. */

    /* 11.3. Vector Integer Extension */
    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sign/zero-extend vector
    /// \return 0 if no exception triggered
    uint8_t vext_vf(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t extension_encoding,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);
    /* End 11.3.*/

    /* 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry vector-vector
    /// \return 0 if no exception triggered
    uint8_t vadc_vvm(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vadc_vxm(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry vector-immediate
    /// \return 0 if no exception triggered
    uint8_t vadc_vim(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce carry-out in mask register format vector-vector
    /// \return 0 if no exception triggered
    uint8_t vmadc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce carry-out in mask register format vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vmadc_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce carry-out in mask register format vector-immediate
    /// \return 0 if no exception triggered
    uint8_t vmadc_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Difference with borrow vector-vector
    /// \return 0 if no exception triggered
    uint8_t vsbc_vvm(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Difference with borrow vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vsbc_vxm(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce borrow-out in mask register format vector-vector
    /// \return 0 if no exception triggered
    uint8_t vmsbc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce borrow-out in mask register format vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vmsbc_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.4. */

    /* 11.5. Vector Bitwise Logical Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief AND vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vand_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief AND vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vand_vi(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief AND vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vand_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief OR vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vor_vv(void *pV, //!<[inout] Vector register field as local memory
                   uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                   uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief OR vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vor_vi(void *pV, //!<[inout] Vector register field as local memory
                   uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                   uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief OR vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vor_vx(void *pV, //!<[inout] Vector register field as local memory
                   void *pR, //!<[in] Integer/General Purpose register field
                   uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                   uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief XOR vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vxor_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief XOR vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vxor_vi(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief XOR vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vxor_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.5. */

    /* 11.6. Vector Single-Width Shift Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SLL vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vsll_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SLL vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vsll_vi(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SLL vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vsll_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRL vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vsrl_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRL vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vsrl_vi(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRL vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vsrl_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRA vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vsra_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRA vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vsra_vi(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRA vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vsra_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.6. */

    /* 11.7. Vector Narrowing Integer Right Shift Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Narrowing SRL vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vnsrl_wv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Narrowing SRL vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vnsrl_wi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Narrowing SRL vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vnsrl_wx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Narrowing SRA vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vnsra_wv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Narrowing SRA vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vnsra_wi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Narrowing SRA vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vnsra_wx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.7. */

    /* 11.8. Vector Integer Compare Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSEQ vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vmseq_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSEQ vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vmseq_vi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSEQ vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vmseq_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSNE vector-vector (not equal)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsne_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSNE vector-immediate (not equal)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsne_vi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSNE vector-vector (not equal)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsne_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLTU vector-vector (less than unsigned)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsltu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLTU vector-scalar (less than unsigned)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsltu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLT vector-vector (less than signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmslt_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLT vector-scalar (less than signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmslt_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLEU vector-vector (less than or equal unsigned)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsleu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLEU vector-immediate (less than or equal unsigned)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsleu_vi(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLEU vector-scalar (less than or equal unsigned)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsleu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLE vector-vector (less than or equal signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsle_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLE vector-immediate (less than or equal signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsle_vi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLE vector-scalar (less than or equal signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsle_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSGTU vector-vector (greater than unsigned)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsgtu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSGTU vector-scalar (greater than unsigned)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsgtu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLE vector-immediate (less than or equal signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsgtu_vi(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSGT vector-vector (greater than signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsgt_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSGTU vector-scalar (greater than signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsgt_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MSLE vector-immediate (less than or equal signed)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmsgt_vi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);
    /* End 11.8. */

    /* 11.9. Vector Integer Min/Max Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed MAX vector-vector
    /// \return 0 if no exception triggered
    uint8_t vmax_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed MAX vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vmax_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned MAX vector-vector
    /// \return 0 if no exception triggered
    uint8_t vmaxu_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned MAX vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vmaxu_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed MIN vector-vector
    /// \return 0 if no exception triggered
    uint8_t vmin_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed MIN vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vmin_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned MIN vector-vector
    /// \return 0 if no exception triggered
    uint8_t vminu_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned MIN vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vminu_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.9. */

    /* 11.10. Vector Single-Width Integer Multiply Instructions */
    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief MUL vector-vector low bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmul_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MUL vector-scalar low bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmul_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief MUL vector-vector high bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmulh_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MUL vector-scalar high bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmulh_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief MUL unsigned-unsigned vector-vector high bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmulhu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MUL unsigned-unsigned vector-scalar high bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmulhu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /// \brief MUL signed-unsigned vector-vector high bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmulhsu_vv(void *pV, //!<[inout] Vector register field as local memory
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief MUL signed-unsigned vector-scalar high bit of product
    /// \return 0 if no exception triggered, else 1
    uint8_t vmulhsu_vx(void *pV, //!<[inout] Vector register field as local memory
                       void *pR, //!<[in] Integer/General Purpose register field
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.10. */

    /* 11.11. Vector Integer Divide Instructions */
    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed DIV vector-vector
    /// \return 0 if no exception triggered
    uint8_t vdiv_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed DIV vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vdiv_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned DIV vector-vector
    /// \return 0 if no exception triggered
    uint8_t vdivu_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned DIV vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vdivu_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed REM vector-vector
    /// \return 0 if no exception triggered
    uint8_t vrem_vv(void *pV, //!<[inout] Vector register field as local memory
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed REM vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vrem_vx(void *pV, //!<[inout] Vector register field as local memory
                    void *pR, //!<[in] Integer/General Purpose register field
                    uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned REM vector-vector
    /// \return 0 if no exception triggered
    uint8_t vremu_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned REM vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vremu_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.11. */

    /* 11.12. Vector Widening Integer Multiply Instructions */
    /// \brief Widening MUL signed-signed vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vwmul_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MUL signed-signed vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vwmul_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /// \brief Widening MUL unsigned-unsigned vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vwmulu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MUL unsigned-unsigned vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vwmulu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /// \brief Widening MUL signed-unsigned vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vwmulsu_vv(void *pV, //!<[inout] Vector register field as local memory
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MUL signed-unsigned vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vwmulsu_vx(void *pV, //!<[inout] Vector register field as local memory
                       void *pR, //!<[in] Integer/General Purpose register field
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.12. */

    /* 11.13. Vector Single-Width Integer Multiply-Add Instructions */
    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief MACC vector-vector
    /// \return 0 if no exception triggered
    uint8_t vmacc_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief MACC vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vmacc_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSAC vector-vector
    /// \return 0 if no exception triggered
    uint8_t vnmsub_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSAC vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vnmsub_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief MADD vector-vector
    /// \return 0 if no exception triggered
    uint8_t vmadd_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief MADD vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vmadd_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSUB vector-vector
    /// \return 0 if no exception triggered
    uint8_t vnmsub_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSUB vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vnmsub_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.13. */

    /* 11.14. Vector Widening Integer Multiply-Add Instructions */
    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned MACC vector-vector
    /// \return 0 if no exception triggered
    uint8_t vwmaccu_vv(void *pV, //!<[inout] Vector register field as local memory
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned MACC vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vwmaccu_vx(void *pV, //!<[inout] Vector register field as local memory
                       void *pR, //!<[in] Integer/General Purpose register field
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MACC vector-vector
    /// \return 0 if no exception triggered
    uint8_t vwmacc_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MACC vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vwmacc_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MACC vector-vector signed(vs1)-unsigned(vs2)
    /// \return 0 if no exception triggered
    uint8_t vwmaccsu_vv(void *pV, //!<[inout] Vector register field as local memory
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MACC vector-scalar signed(rs1)-unsigned(vs2)
    /// \return 0 if no exception triggered
    uint8_t vwmaccsu_vx(void *pV, //!<[inout] Vector register field as local memory
                        void *pR, //!<[in] Integer/General Purpose register field
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening MACC vector-scalar unsigned(rs1)-signed(vs2)
    /// \return 0 if no exception triggered
    uint8_t vwmaccus_vx(void *pV, //!<[inout] Vector register field as local memory
                        void *pR, //!<[in] Integer/General Purpose register field
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.14. */

    /* 11.15. Vector Integer Merge Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Merge vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vmerge_vv(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Merge vector-scalar (immediate)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmerge_vi(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Merge vector-scalar (register)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmerge_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 11.15. */

    /* 11.16. Vector Integer Move Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move/Copy vector to vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vmv_vv(void *pV, //!<[in] Vector register field as local memory
                   uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move scalar (signed immediate) to vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vmv_vi(void *pV, //!<[inout] Vector register field as local memory
                   uint16_t pVTYPE, uint8_t pVd, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move (signed) scalar register to vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vmv_vx(void *pV, //!<[inout] Vector register field as local memory
                   void *pR, //!<[in] Integer/General Purpose register field
                   uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL,
                   uint8_t pXLEN);
    /* End 11.16. */
    /* End 11. */

    /* 12. Vector Fixed-Point Arithmetic Instructions */
    /* 12.1. Vector Single-Width Saturating Add and Subtract */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Saturating unsigned ADD vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vsaddu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Saturating unsigned ADD vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vsaddu_vi(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Saturating unsigned ADD vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vsaddu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /// \brief Saturating signed ADD vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vsadd_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Saturating signed ADD vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vsadd_vi(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Saturating signed ADD vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vsadd_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /// \brief Saturating unsigned SUB vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vssubu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Saturating unsigned SUB vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vssubu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    /// \brief Saturating signed SUB vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vssub_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Saturating signed SUB vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vssub_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 12.1. */

    /* 12.2. Vector Single-Width Averaging Add and Subtract */
    /// \brief Averaging unsigned ADD vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vaaddu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Averaging unsigned ADD vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vaaddu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm);

    /// \brief Averaging signed ADD vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vaadd_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Averaging signed ADD vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vaadd_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm);

    /// \brief Averaging unsigned SUB vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vasubu_vv(void *pV, //!<[inout] Vector register field as local memory
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Averaging unsigned SUB vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vasubu_vx(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm);

    /// \brief Averaging signed SUB vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vasub_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Averaging signed SUB vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vasub_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm);
    /* End 12.2. */

    /* 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation */
    /// \brief Signed fractional MUL with rounding and saturation vector-vector
    /// \return 0 if not saturated, else 1
    uint8_t vsmul_vv(void *pV, //!<[inout] Vector register field as local memory
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed fractional MUL with rounding and saturation vector-scalar
    /// \return 0 if not saturated, else 1
    uint8_t vsmul_vx(void *pV, //!<[inout] Vector register field as local memory
                     void *pR, //!<[in] Integer/General Purpose register field
                     uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm);
    /* End 12.3. */

    /* 12.4. Vector Single-Width Scaling Shift Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Scaling logical right shift vector-vector
    uint8_t vssrl_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Scaling logical right shift vector-immediate
    uint8_t vssrl_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Scaling logical right shift vector-scalar
    uint8_t vssrl_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Scaling arithmetic right shift vector-vector
    uint8_t vssra_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Scaling arithmetic right shift vector-immediate
    uint8_t vssra_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Scaling arithmetic right shift vector-scalar
    uint8_t vssra_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm);

    /* End 12.4. */

    /* 12.5. Vector Narrowing Fixed-Point Clip Instructions */

    /* End 12.5. */
    /* End 12. */

    /* 16. Vector Permutation Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move first element (0) of a vector register A to scalar register X := SEW>XLEN ? A[0] : sext(A[0])
    /// \return 0 if no exception triggered, else 1
    uint8_t vmv_xs(void *pV, //!<[in] Vector register field as local memory
                   void *pR, //!<[inout] Integer/General Purpose register field
                   uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move scalar register to first element (0) of a vector register A[0] := SEW<XLEN ? X : sext(X)
    /// \return 0 if no exception triggered, else 1
    uint8_t vmv_sx(void *pV, //!<[inout] Vector register field as local memory
                   void *pR, //!<[in] Integer/General Purpose register field
                   uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL,
                   uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move first element (0) of a vector register A to scalar fp register F := SEW>XLEN ? A[0] : sext(A[0])
    /// \return 0 if no exception triggered, else 1
    uint8_t vfmv_fs(void *pV, //!<[in] Vector register field as local memory
                    void *pF, //!<[inout] Floating point register field
                    uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Move scalar fp register to first element (0) of a vector register A[0] := SEW<XLEN ? F : sext(F)
    /// \return 0 if no exception triggered, else 1
    uint8_t vfmv_sf(void *pV, //!<[in] Vector register field as local memory
                    void *pF, //!<[inout] Floating point register field
                    uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL,
                    uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vslideup.vx vd, vs2, rs1, vm        # vd[i+rs1] = vs2[i]
    /// \return 0 if no exception triggered, else 1
    uint8_t vslideup_vx(void *pV, //!<[inout] Vector register field as local memory
                        void *pR, //!<[in] Integer/General Purpose register field
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vslideup.vi vd, vs2, uimm[4:0], vm        # vd[i+rs1] = vs2[i]
    /// \return 0 if no exception triggered, else 1
    uint8_t vslideup_vi(void *pV, //!<[inout] Vector register field as local memory
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vslidedown.vx vd, vs2, rs1, vm        # vd[i+rs1] = vs2[i]
    /// \return 0 if no exception triggered, else 1
    uint8_t vslidedown_vx(void *pV, //!<[inout] Vector register field as local memory
                          void *pR, //!<[in] Integer/General Purpose register field
                          uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vslidedown.vi vd, vs2, uimm[4:0], vm        # vd[i+rs1] = vs2[i]
    /// \return 0 if no exception triggered, else 1
    uint8_t vslidedown_vi(void *pV, //!<[inout] Vector register field as local memory
                          uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vslide1up.vx vd, vs2, rs1, vm        # vd[0]=x[rs1], vd[i+1] = vs2[i]
    /// \return 0 if no exception triggered, else 1
    uint8_t vslide1up(void *pV, //!<[inout] Vector register field as local memory
                      void *pR, //!<[in] Integer/General Purpose register field
                      uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vfslide1up.vf vd, vs2, fs1, vm        # vd[0]=f[rs1], vd[i+1] = vs2[i]
    /// \return 0 if no exception triggered, else 1
    uint8_t vfslide1up(void *pV, //!<[inout] Vector register field as local memory
                       void *pF, //!<[inout] Floating point register field
                       uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vslide1down.vx  vd, vs2, rs1, vm      # vd[i] = vs2[i+1], vd[vl-1]=x[rs1]
    /// \return 0 if no exception triggered, else 1
    uint8_t vslide1down(void *pV, //!<[inout] Vector register field as local memory
                        void *pR, //!<[in] Integer/General Purpose register field
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief vfslide1down.vf vd, vs2, fs1, vm      # vd[i] = vs2[i+1], vd[vl-1]=f[rs1]
    /// \return 0 if no exception triggered, else 1
    uint8_t vfslide1down(void *pV, //!<[inout] Vector register field as local memory
                         void *pF, //!<[inout] Floating point register field
                         uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 16. */

    /* 13. Vector Floating-Point Instructions */
    /* 13.2. Vector Single-Width Floating-Point Add/Subtract Instructions */
    uint8_t vfadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfrsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.2. */

    /* 13.3. Vector Widening Floating-Point Add/Subtract Instructions */
    uint8_t vfwadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfwsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfwadd_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwadd_wf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfwadd_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwadd_wf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.3. */

    /* 13.4. Vector Single-Width Floating-Point Multiply/Divide Instructions */
    uint8_t vfmul_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfmul_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfdiv_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfdiv_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfrdiv_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.4. */

    /* 13.5. Vector Widening Floating-Point Multiply */
    uint8_t vfwmul_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwmul_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.5. */

    /* 13.6. Vector Single-Width Floating-Point Fused Multiply-Add Instructions */
    uint8_t vfmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfnmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfnmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfnmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfnmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfmadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfmadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfnmadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfnmadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfmsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfmsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfnmsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfnmsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.6. */

    /* 13.7. Vector Widening Floating-Point Fused Multiply-Add Instructions */
    uint8_t vfwmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfwnmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwnmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfwmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfwnmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwnmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.7. */

    /* 13.8. Vector Floating-Point Square-Root Instruction */
    uint8_t vfsqrt_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 13.8. */

    /* 13.9. Vector Floating-Point Reciprocal Square-Root Estimate Instruction */
    uint8_t vfrsqrt7_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 13.9. */

    /* 13.10. Vector Floating-Point Reciprocal Estimate Instruction */
    uint8_t vfrec7_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 13.10.*/

    /* 13.11. Vector Floating-Point MIN/MAX Instructions */
    uint8_t vfmin_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfmin_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfmax_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfmax_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.11.*/

    /* 13.12. Vector Floating-Point Sign-Injection Instructions */
    uint8_t vfsgnj_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfsgnj_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfsgnjn_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfsgnjn_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vfsgnjx_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfsgnjx_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.12.*/

    /* 13.13. Vector Floating-Point Compare Instructions */
    uint8_t vmfeq_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vmfeq_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vmfne_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vmfne_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vmflt_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vmflt_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vmfle_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vmfle_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vmfgt_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);

    uint8_t vmfge_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm);
    /* End 13.13.*/

    /* 13.14. Vector Floating-Point Classify Instruction */
    uint8_t vfclass_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 13.14.*/

    /* 13.15. Vector Floating-Point Merge Instruction */
    uint8_t vfmerge_vfm(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN);
    /* End 13.15.*/

    /* 13.16. Vector Floating-Point Move Instruction */
    uint8_t vfmv_v_f(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN,
                     uint16_t pVL, uint8_t pFLEN);
    /* End 13.16.*/

    /* 13.17. Single-Width Floating-Point/Integer Type-Convert Instructions */
    uint8_t vfcvt_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfcvt_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfcvt_rtz_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfcvt_rtz_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                            uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfcvt_f_xu_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfcvt_f_x_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 13.17.*/

    /* 13.18. Widening Floating-Point/Integer Type-Convert Instructions */
    uint8_t vfwcvt_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwcvt_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwcvt_rtz_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                              uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwcvt_rtz_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwcvt_f_xu_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwcvt_f_x_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwcvt_f_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 13.18.*/

    /* 13.19. Narrowing Floating-Point/Integer Type-Convert Instructions */
    uint8_t vfncvt_xu_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfncvt_x_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfncvt_rtz_xu_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                              uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfncvt_rtz_x_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfncvt_f_xu_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfncvt_f_x_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfncvt_f_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfncvt_rod_f_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 13.19.*/

    /* 14. Vector Reduction Operations */
    /* 14.1. Vector Single-Width Integer Reduction Instructions */
    uint8_t vredsum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    uint8_t vredmaxu_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    uint8_t vredmax_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    uint8_t vredminu_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    uint8_t vredmin_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    uint8_t vredand_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    uint8_t vredor_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    uint8_t vredxor_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    /* End 14.1. */
    /* 14.2. Vector Widening Integer Reduction Instructions */
    uint8_t vwredsumu_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);

    uint8_t vwredsum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);
    /* End 14.2. */
    /* 14.3. Vector Single-Width Floating-Point Reduction Instructions */
    uint8_t vfredosum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfredusum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfredmax_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfredmin_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 14.3. */
    /* 14.4. Vector Widening Floating-Point Reduction Instructions */
    uint8_t vfwredosum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                          uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);

    uint8_t vfwredusum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                          uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm);
    /* End 14.4. */
    /* End 14. */

    /* 15. Vector Mask Instructions */
    /* 15.1. Vector Mask-Register Logical Instructions */
    uint8_t vmand_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    uint8_t vmnand_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    uint8_t vmandn_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);

    uint8_t vmxor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    uint8_t vmor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL);

    uint8_t vmnor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    uint8_t vmorn_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL);

    uint8_t vmxnor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL);
    /* End 15.1. */
    /* 15.2. Vector count population in mask vcpop.m */
    uint8_t vcpop_m(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pRd, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 15.2. */
    /* 15.3. vfirst find-first-set mask bit */
    uint8_t vfirst_m(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pRd, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 15.3. */
    /* 15.4. vmsbf.m set-before-first mask bit */
    uint8_t vmsbf_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL);
    /* End 15.4. */
    /* 15.5. vmsif.m set-including-first mask bit */
    uint8_t vmsif_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL);
    /* End 15.5. */
    /* 15.6. vmsof.m set-only-first mask bit */
    uint8_t vmsof_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL);
    /* End 15.6. */
    /* 15.8. Vector Iota Instruction */
    uint8_t viota_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL);
    /* End 15.8. */
    /* 15.9. Vector Element Index Instruction */
    uint8_t vid_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL);
    /* End 15.9. */
    /* End 15. */
    /* 16. Vector Permutation Instructions */
    /* 16.1. Integer Scalar Move Instructions */
    /* End 16.1. */
    /* 16.2. Floating-Point Scalar Move Instructions */
    /* End 16.2. */
    /* 16.3. Vector Slide Instructions */
    /* End 16.3. */
    /* 16.4. Vector Register Gather Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief VRGATHER vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vrgather_vv(void *pV, //!<[inout] Vector register field as local memory
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief VRGATHEREI16 vector-vector
    /// \return 0 if no exception triggered, else 1
    uint8_t vrgatherei16_vv(void *pV, //!<[inout] Vector register field as local memory
                            uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                            uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief VRGATHER vector-immediate
    /// \return 0 if no exception triggered, else 1
    uint8_t vrgather_vi(void *pV, //!<[inout] Vector register field as local memory
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief VRGATHER vector-scalar
    /// \return 0 if no exception triggered, else 1
    uint8_t vrgather_vx(void *pV, //!<[inout] Vector register field as local memory
                        void *pR, //!<[in] Integer/General Purpose register field
                        uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN);
    /* End 16.4. */

    /* 16.5. Vector Compress Instruction */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief VCOMPRESS vector - mask register
    /// \return 0 if no exception triggered, else 1
    uint8_t vrgather_vm(void *pV, //!<[inout] Vector register field as local memory
                        uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                        uint16_t pVL);
    /* End 16.5. */

    /* 16.6. Whole Vector Register Move */
    uint8_t vmvr_v(void *pV, //!<[inout] Vector register field as local memory
                   uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t simm5, uint16_t pVSTART, uint16_t pVLEN,
                   uint16_t pVL);
    /* End 16.6. */
    /* End 16. */

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __SOFTVECTOR_H__ */
