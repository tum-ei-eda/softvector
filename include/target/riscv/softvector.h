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
    uint8_t vtype_extractSEW(uint16_t const vtype //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract LMUL bitfield from VTYPE bitfield
    /// \return Encoded LMUL bitfield
    uint8_t vtype_extractLMUL(uint16_t const vtype //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract TA bitfield from VTYPE bitfield
    /// \return Encoded TA bitfield
    uint8_t vtype_extractTA(uint16_t const vtype //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Extract MA bitfield from VTYPE bitfield
    /// \return Encoded MA bitfield
    uint8_t vtype_extractMA(uint16_t const vtype //!<[in] vtype bitfield
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Concatenate MEW and WIDTH to EEW and return number of bits for EEW
    /// \return Decoded EEW [bits]
    uint16_t vcfg_concatEEW(uint8_t const mew,  //!<[in] MEW bit
                            uint8_t const width //!<[in] WIDTH bits
    );

    /* Vector Loads/Stores Helpers*/
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load encoded (unitstride) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_encoded_unitstride(void *const vector_field, //!<[inout] Vector register field as local memory
                                     uint8_t *const memory,    //!<[inout] Local memory
                                     uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                     uint8_t const vd, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                     uint64_t const mem_offset);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load encoded (strided) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_encoded_stride(void *const vector_field, //!<[inout] Vector register field as local memory
                                 uint8_t *const memory,    //!<[inout] Local memory
                                 uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                 uint8_t const vd, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                 uint64_t const mem_offset, int16_t const stride);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load seqgmented (unitstride) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_segment_unitstride(void *const vector_field, //!<[inout] Vector register field as local memory
                                     uint8_t *const memory,    //!<[inout] Local memory
                                     uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                     uint8_t pNF, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                                     uint32_t const vl, uint64_t const mem_offset);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Load encoded (strided) from memory to target vector (-group)
    /// \return 0 if no exception triggered, else 1
    uint8_t vload_segment_stride(void *const vector_field, //!<[inout] Vector register field as local memory
                                 uint8_t *const memory,    //!<[inout] Local memory
                                 uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                 uint8_t pNF, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                                 uint32_t const vl, uint64_t const mem_offset, int16_t const stride);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store encoded (unitstride) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_encoded_unitstride(void *const vector_field, //!<[inout] Vector register field as local memory
                                      uint8_t *const memory,    //!<[inout] Local memory
                                      uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                      uint8_t const vd, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                      uint64_t const mem_offset);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store encoded (strided) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_encoded_stride(void *const vector_field, //!<[inout] Vector register field as local memory
                                  uint8_t *const memory,    //!<[inout] Local memory
                                  uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                  uint8_t const vd, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                  uint64_t const mem_offset, int16_t const stride);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store segmented (unitstride) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_segment_unitstride(void *const vector_field, //!<[inout] Vector register field as local memory
                                      uint8_t *const memory,    //!<[inout] Local memory
                                      uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                      uint8_t pNF, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                                      uint32_t const vl, uint64_t const mem_offset);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Store segmented (strided) source vector (-group) to memory
    /// \return 0 if no exception triggered, else 1
    uint8_t vstore_segment_stride(void *const vector_field, //!<[inout] Vector register field as local memory
                                  uint8_t *const memory,    //!<[inout] Local memory
                                  uint16_t const vtype, uint8_t const masked_instruction_bit, uint16_t const eew,
                                  uint8_t pNF, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                                  uint32_t const vl, uint64_t const mem_offset, int16_t const stride);

#define VV_OP_DECL(name)                                                                                             \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,               \
                 uint8_t const vd, uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, \
                 uint32_t const vl);

#define VI_OP_DECL(name)                                                                                             \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,               \
                 uint8_t const vd, uint8_t const vs2, uint8_t const imm, uint16_t const vstart, uint32_t const vlen, \
                 uint32_t const vl);

#define VX_OP_DECL(name)                                                                                       \
    uint8_t name(void *const vector_field, void *scalar_field, uint16_t const vtype,                           \
                 uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1, \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    // 11.1. Vector Single-Width Integer Add and Subtract
    VV_OP_DECL(vadd_vv)
    VI_OP_DECL(vadd_vi)
    VX_OP_DECL(vadd_vx)

    VV_OP_DECL(vsub_vv)
    VX_OP_DECL(vsub_vx)

    VX_OP_DECL(vrsub_vx)
    VI_OP_DECL(vrsub_vi)

    // 11.2. Vector Widening Integer Add/Subtract
    VV_OP_DECL(vwaddu_vv)
    VX_OP_DECL(vwaddu_vx)
    VV_OP_DECL(vwsubu_vv)
    VX_OP_DECL(vwsubu_vx)

    VV_OP_DECL(vwadd_vv)
    VX_OP_DECL(vwadd_vx)
    VV_OP_DECL(vwsub_vv)
    VX_OP_DECL(vwsub_vx)

    VV_OP_DECL(vwaddu_w_vv)
    VX_OP_DECL(vwaddu_w_vx)
    VV_OP_DECL(vwsubu_w_vv)
    VX_OP_DECL(vwsubu_w_vx)

    VV_OP_DECL(vwadd_w_vv)
    VX_OP_DECL(vwadd_w_vx)
    VV_OP_DECL(vwsub_w_vv)
    VX_OP_DECL(vwsub_w_vx)

    // 11.3. Vector Integer Extension
    uint8_t vext_vf(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                    uint8_t const vd, uint8_t const vs2, uint8_t const extension_encoding, uint16_t const vstart,
                    uint32_t const vlen, uint32_t const vl);

    // 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry vector-vector
    /// \return 0 if no exception triggered
    uint8_t vadc_vvm(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1,
                     uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vadc_vxm(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
                     uint8_t const vs2, uint8_t pRs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                     uint8_t const xlen);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry vector-immediate
    /// \return 0 if no exception triggered
    uint8_t vadc_vim(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs2, uint8_t pVimm,
                     uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    VV_OP_DECL(vmadc_vv)
    VX_OP_DECL(vmadc_vx)
    VI_OP_DECL(vmadc_vi)

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Difference with borrow vector-vector
    /// \return 0 if no exception triggered
    uint8_t vsbc_vvm(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1,
                     uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Difference with borrow vector-scalar
    /// \return 0 if no exception triggered
    uint8_t vsbc_vxm(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
                     uint8_t const vs2, uint8_t pRs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                     uint8_t const xlen);

    VV_OP_DECL(vmsbc_vv)
    VX_OP_DECL(vmsbc_vx)

    // 11.5. Vector Bitwise Logical Instructions
    VV_OP_DECL(vand_vv)
    VX_OP_DECL(vand_vx)
    VI_OP_DECL(vand_vi)

    VV_OP_DECL(vor_vv)
    VX_OP_DECL(vor_vx)
    VI_OP_DECL(vor_vi)

    VV_OP_DECL(vxor_vv)
    VX_OP_DECL(vxor_vx)
    VI_OP_DECL(vxor_vi)

    // 11.6. Vector Single-Width Shift Instructions
    VV_OP_DECL(vsll_vv)
    VX_OP_DECL(vsll_vx)
    VI_OP_DECL(vsll_vi)

    VV_OP_DECL(vsrl_vv)
    VX_OP_DECL(vsrl_vx)
    VI_OP_DECL(vsrl_vi)

    VV_OP_DECL(vsra_vv)
    VX_OP_DECL(vsra_vx)
    VI_OP_DECL(vsra_vi)

    // 11.7. Vector Narrowing Integer Right Shift Instructions
    VV_OP_DECL(vnsrl_wv)
    VX_OP_DECL(vnsrl_wx)
    VI_OP_DECL(vnsrl_wi)

    VV_OP_DECL(vnsra_wv)
    VX_OP_DECL(vnsra_wx)
    VI_OP_DECL(vnsra_wi)

    // 11.8. Vector Integer Compare Instructions
    VV_OP_DECL(vmseq_vv)
    VX_OP_DECL(vmseq_vx)
    VI_OP_DECL(vmseq_vi)

    VV_OP_DECL(vmsne_vv)
    VX_OP_DECL(vmsne_vx)
    VI_OP_DECL(vmsne_vi)

    VV_OP_DECL(vmsltu_vv)
    VX_OP_DECL(vmsltu_vx)

    VV_OP_DECL(vmslt_vv)
    VX_OP_DECL(vmslt_vx)

    VV_OP_DECL(vmsleu_vv)
    VX_OP_DECL(vmsleu_vx)
    VI_OP_DECL(vmsleu_vi)

    VV_OP_DECL(vmsle_vv)
    VX_OP_DECL(vmsle_vx)
    VI_OP_DECL(vmsle_vi)

    VX_OP_DECL(vmsgtu_vx)
    VI_OP_DECL(vmsgtu_vi)

    VX_OP_DECL(vmsgt_vx)
    VI_OP_DECL(vmsgt_vi)

    // 11.9. Vector Integer Min/Max Instructions
    VV_OP_DECL(vminu_vv)
    VX_OP_DECL(vminu_vx)

    VV_OP_DECL(vmin_vv)
    VX_OP_DECL(vmin_vx)

    VV_OP_DECL(vmaxu_vv)
    VX_OP_DECL(vmaxu_vx)

    VV_OP_DECL(vmax_vv)
    VX_OP_DECL(vmax_vx)

    // 11.10. Vector Single-Width Integer Multiply Instructions
    VV_OP_DECL(vmul_vv)
    VX_OP_DECL(vmul_vx)

    VV_OP_DECL(vmulh_vv)
    VX_OP_DECL(vmulh_vx)

    VV_OP_DECL(vmulhu_vv)
    VX_OP_DECL(vmulhu_vx)

    VV_OP_DECL(vmulhsu_vv)
    VX_OP_DECL(vmulhsu_vx)

    // 11.11. Vector Integer Divide Instructions
    VV_OP_DECL(vdivu_vv)
    VX_OP_DECL(vdivu_vx)

    VV_OP_DECL(vdiv_vv)
    VX_OP_DECL(vdiv_vx)

    VV_OP_DECL(vremu_vv)
    VX_OP_DECL(vremu_vx)

    VV_OP_DECL(vrem_vv)
    VX_OP_DECL(vrem_vx)

    // 11.12. Vector Widening Integer Multiply Instructions
    VV_OP_DECL(vwmul_vv)
    VX_OP_DECL(vwmul_vx)

    VV_OP_DECL(vwmulu_vv)
    VX_OP_DECL(vwmulu_vx)

    VV_OP_DECL(vwmulsu_vv)
    VX_OP_DECL(vwmulsu_vx)

    // 11.13. Vector Single-Width Integer Multiply-Add Instructions
    VV_OP_DECL(vmacc_vv)
    VX_OP_DECL(vmacc_vx)

    VV_OP_DECL(vnmsac_vv)
    VX_OP_DECL(vnmsac_vx)

    VV_OP_DECL(vmadd_vv)
    VX_OP_DECL(vmadd_vx)

    VV_OP_DECL(vnmsub_vv)
    VX_OP_DECL(vnmsub_vx)

    // 11.14. Vector Widening Integer Multiply-Add Instructions
    VV_OP_DECL(vwmaccu_vv)
    VX_OP_DECL(vwmaccu_vx)

    VV_OP_DECL(vwmacc_vv)
    VX_OP_DECL(vwmacc_vx)

    VV_OP_DECL(vwmaccsu_vv)
    VX_OP_DECL(vwmaccsu_vx)

    VX_OP_DECL(vwmaccus_vx)

    // 11.15. Vector Integer Merge Instructions
    uint8_t vmerge_vv(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1,
                      uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    uint8_t vmerge_vi(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs2,
                      uint8_t pVimm, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    uint8_t vmerge_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
                      uint8_t const vs2, uint8_t pRs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                      uint8_t const xlen);

    // 11.16. Vector Integer Move Instructions
    uint8_t vmv_vv(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1,
                   uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    uint8_t vmv_vi(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t pVimm,
                   uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    uint8_t vmv_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
                   uint8_t pRs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

// 12. Vector Fixed-Point Arithmetic Instructions
#define AVG_VV_OP_DECL(name)                                                                                         \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,               \
                 uint8_t const vd, uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, \
                 uint32_t const vl, uint8_t const rounding_mode);

#define AVG_VX_OP_DECL(name)                                                                                   \
    uint8_t name(void *const vector_field, void *scalar_field, uint16_t const vtype,                           \
                 uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1, \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen,            \
                 uint8_t const rounding_mode);

#define AVG_VI_OP_DECL(name)                                                                                         \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,               \
                 uint8_t const vd, uint8_t const vs2, uint8_t const imm, uint16_t const vstart, uint32_t const vlen, \
                 uint32_t const vl, uint8_t const rounding_mode);

    // 12.1. Vector Single-Width Saturating Add and Subtract
    VV_OP_DECL(vsaddu_vv)
    VX_OP_DECL(vsaddu_vx)
    VI_OP_DECL(vsaddu_vi)

    VV_OP_DECL(vsadd_vv)
    VX_OP_DECL(vsadd_vx)
    VI_OP_DECL(vsadd_vi)

    VV_OP_DECL(vssubu_vv)
    VX_OP_DECL(vssubu_vx)

    VV_OP_DECL(vssub_vv)
    VX_OP_DECL(vssub_vx)

    // 12.2. Vector Single-Width Averaging Add and Subtract
    AVG_VV_OP_DECL(vaaddu_vv)
    AVG_VX_OP_DECL(vaaddu_vx)

    AVG_VV_OP_DECL(vaadd_vv)
    AVG_VX_OP_DECL(vaadd_vx)

    AVG_VV_OP_DECL(vasubu_vv)
    AVG_VX_OP_DECL(vasubu_vx)

    AVG_VV_OP_DECL(vasub_vv)
    AVG_VX_OP_DECL(vasub_vx)

    // 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation
    AVG_VV_OP_DECL(vsmul_vv)
    AVG_VX_OP_DECL(vsmul_vx)

    // 12.4. Vector Single-Width Scaling Shift Instructions
    AVG_VV_OP_DECL(vssrl_vv)
    AVG_VX_OP_DECL(vssrl_vx)
    AVG_VI_OP_DECL(vssrl_vi)

    AVG_VV_OP_DECL(vssra_vv)
    AVG_VX_OP_DECL(vssra_vx)
    AVG_VI_OP_DECL(vssra_vi)

    // 12.5. Vector Narrowing Fixed-Point Clip Instructions
    AVG_VV_OP_DECL(vnclipu_wv)
    AVG_VX_OP_DECL(vnclipu_wx)
    AVG_VI_OP_DECL(vnclipu_wi)

    AVG_VV_OP_DECL(vnclip_wv)
    AVG_VX_OP_DECL(vnclip_wx)
    AVG_VI_OP_DECL(vnclip_wi)

// 13. Vector Floating-Point Instructions
#define F_VV_OP_DECL(name)                                                                                           \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,               \
                 uint8_t const vd, uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, \
                 uint32_t const vl, uint8_t const rounding_mode);

#define F_VF_OP_DECL(name)                                                                                     \
    uint8_t name(void *const vector_field, void *const float_scalar_field, uint16_t const vtype,               \
                 uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1, \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen,            \
                 uint8_t const rounding_mode);

    // 13.2. Vector Single-Width Floating-Point Add/Subtract Instructions
    F_VV_OP_DECL(vfadd_vv)
    F_VF_OP_DECL(vfadd_vf)

    F_VV_OP_DECL(vfsub_vv)
    F_VF_OP_DECL(vfsub_vf)
    F_VF_OP_DECL(vfrsub_vf)

    // 13.3. Vector Widening Floating-Point Add/Subtract Instructions
    F_VV_OP_DECL(vfwadd_vv)
    F_VF_OP_DECL(vfwadd_vf)
    F_VV_OP_DECL(vfwsub_vv)
    F_VF_OP_DECL(vfwsub_vf)

    F_VV_OP_DECL(vfwadd_wv)
    F_VF_OP_DECL(vfwadd_wf)
    F_VV_OP_DECL(vfwsub_wv)
    F_VF_OP_DECL(vfwsub_wf)

    // 13.4. Vector Single-Width Floating-Point Multiply/Divide Instructions
    F_VV_OP_DECL(vfmul_vv)
    F_VF_OP_DECL(vfmul_vf)

    F_VV_OP_DECL(vfdiv_vv)
    F_VF_OP_DECL(vfdiv_vf)

    F_VF_OP_DECL(vfrdiv_vf)

    // 13.5. Vector Widening Floating-Point Multiply
    F_VV_OP_DECL(vfwmul_vv)
    F_VF_OP_DECL(vfwmul_vf)

    // 13.6. Vector Single-Width Floating-Point Fused Multiply-Add Instructions
    F_VV_OP_DECL(vfmacc_vv)
    F_VF_OP_DECL(vfmacc_vf)

    F_VV_OP_DECL(vfnmacc_vv)
    F_VF_OP_DECL(vfnmacc_vf)

    F_VV_OP_DECL(vfmsac_vv)
    F_VF_OP_DECL(vfmsac_vf)

    F_VV_OP_DECL(vfnmsac_vv)
    F_VF_OP_DECL(vfnmsac_vf)

    F_VV_OP_DECL(vfmadd_vv)
    F_VF_OP_DECL(vfmadd_vf)

    F_VV_OP_DECL(vfnmadd_vv)
    F_VF_OP_DECL(vfnmadd_vf)

    F_VV_OP_DECL(vfmsub_vv)
    F_VF_OP_DECL(vfmsub_vf)

    F_VV_OP_DECL(vfnmsub_vv)
    F_VF_OP_DECL(vfnmsub_vf)

    // 13.7. Vector Widening Floating-Point Fused Multiply-Add Instructions
    F_VV_OP_DECL(vfwmacc_vv)
    F_VF_OP_DECL(vfwmacc_vf)

    F_VV_OP_DECL(vfwnmacc_vv)
    F_VF_OP_DECL(vfwnmacc_vf)

    F_VV_OP_DECL(vfwmsac_vv)
    F_VF_OP_DECL(vfwmsac_vf)

    F_VV_OP_DECL(vfwnmsac_vv)
    F_VF_OP_DECL(vfwnmsac_vf)

    // 13.8. Vector Floating-Point Square-Root Instruction
    uint8_t vfsqrt_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                     uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                     uint8_t pRm);

    // 13.9. Vector Floating-Point Reciprocal Square-Root Estimate Instruction
    uint8_t vfrsqrt7_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                       uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                       uint32_t const vl, uint8_t pRm);

    // 13.10. Vector Floating-Point Reciprocal Estimate Instruction
    uint8_t vfrec7_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                     uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                     uint8_t pRm);

    // 13.11. Vector Floating-Point MIN/MAX Instructions
    F_VV_OP_DECL(vfmin_vv)
    F_VF_OP_DECL(vfmin_vf)

    F_VV_OP_DECL(vfmax_vv)
    F_VF_OP_DECL(vfmax_vf)

    // 13.12. Vector Floating-Point Sign-Injection Instructions
    F_VV_OP_DECL(vfsgnj_vv)
    F_VF_OP_DECL(vfsgnj_vf)

    F_VV_OP_DECL(vfsgnjn_vv)
    F_VF_OP_DECL(vfsgnjn_vf)

    F_VV_OP_DECL(vfsgnjx_vv)
    F_VF_OP_DECL(vfsgnjx_vf)

    // 13.13. Vector Floating-Point Compare Instructions
    F_VV_OP_DECL(vmfeq_vv)
    F_VF_OP_DECL(vmfeq_vf)

    F_VV_OP_DECL(vmfne_vv)
    F_VF_OP_DECL(vmfne_vf)

    F_VV_OP_DECL(vmflt_vv)
    F_VF_OP_DECL(vmflt_vf)

    F_VV_OP_DECL(vmfle_vv)
    F_VF_OP_DECL(vmfle_vf)

    F_VF_OP_DECL(vmfgt_vf)

    F_VF_OP_DECL(vmfge_vf)

    // 13.14. Vector Floating-Point Classify Instruction
    uint8_t vfclass_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                      uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                      uint32_t const vl, uint8_t pRm);

    // 13.15. Vector Floating-Point Merge Instruction
    uint8_t vfmerge_vfm(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
                        uint8_t const vs2, uint8_t pRs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                        uint8_t const flen);

    // 13.16. Vector Floating-Point Move Instruction
    uint8_t vfmv_v_f(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
                     uint8_t pRs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen);

    // 13.17. Single-Width Floating-Point/Integer Type-Convert Instructions
    uint8_t vfcvt_xu_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    uint8_t vfcvt_x_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                        uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                        uint32_t const vl, uint8_t pRm);

    uint8_t vfcvt_rtz_xu_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                             uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                             uint32_t const vl, uint8_t pRm);

    uint8_t vfcvt_rtz_x_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                            uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                            uint32_t const vl, uint8_t pRm);

    uint8_t vfcvt_f_xu_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    uint8_t vfcvt_f_x_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                        uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                        uint32_t const vl, uint8_t pRm);

    // 13.18. Widening Floating-Point/Integer Type-Convert Instructions
    uint8_t vfwcvt_xu_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                          uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                          uint32_t const vl, uint8_t pRm);

    uint8_t vfwcvt_x_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    uint8_t vfwcvt_rtz_xu_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                              uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                              uint32_t const vl, uint8_t pRm);

    uint8_t vfwcvt_rtz_x_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                             uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                             uint32_t const vl, uint8_t pRm);

    uint8_t vfwcvt_f_xu_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                          uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                          uint32_t const vl, uint8_t pRm);

    uint8_t vfwcvt_f_x_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    uint8_t vfwcvt_f_f_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    // 13.19. Narrowing Floating-Point/Integer Type-Convert Instructions
    uint8_t vfncvt_xu_f_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                          uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                          uint32_t const vl, uint8_t pRm);

    uint8_t vfncvt_x_f_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    uint8_t vfncvt_rtz_xu_f_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                              uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                              uint32_t const vl, uint8_t pRm);

    uint8_t vfncvt_rtz_x_f_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                             uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                             uint32_t const vl, uint8_t pRm);

    uint8_t vfncvt_f_xu_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                          uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                          uint32_t const vl, uint8_t pRm);

    uint8_t vfncvt_f_x_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    uint8_t vfncvt_f_f_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, uint8_t pRm);

    uint8_t vfncvt_rod_f_f_w(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                             uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                             uint32_t const vl, uint8_t pRm);

    // 14. Vector Reduction Operations
    // 14.1. Vector Single-Width Integer Reduction Instructions
    VV_OP_DECL(vredsum_vs)
    VV_OP_DECL(vredmaxu_vs)
    VV_OP_DECL(vredmax_vs)
    VV_OP_DECL(vredminu_vs)
    VV_OP_DECL(vredmin_vs)
    VV_OP_DECL(vredand_vs)
    VV_OP_DECL(vredor_vs)
    VV_OP_DECL(vredxor_vs)

    // 14.2. Vector Widening Integer Reduction Instructions
    VV_OP_DECL(vwredsumu_vs)
    VV_OP_DECL(vwredsum_vs)

    // 14.3. Vector Single-Width Floating-Point Reduction Instructions
    F_VV_OP_DECL(vfredosum_vs)
    F_VV_OP_DECL(vfredusum_vs)
    F_VV_OP_DECL(vfredmax_vs)
    F_VV_OP_DECL(vfredmin_vs)

    // 14.4. Vector Widening Floating-Point Reduction Instructions
    F_VV_OP_DECL(vfwredosum_vs)
    F_VV_OP_DECL(vfwredusum_vs)

    // 15. Vector Mask Instructions
    // 15.1. Vector Mask-Register Logical Instructions
    // These actually don't need the mask bit but get it anyway from CoreDSL, could break if that changes
    VV_OP_DECL(vmand_mm)
    VV_OP_DECL(vmnand_mm)
    VV_OP_DECL(vmandn_mm)
    VV_OP_DECL(vmxor_mm)
    VV_OP_DECL(vmor_mm)
    VV_OP_DECL(vmnor_mm)
    VV_OP_DECL(vmorn_mm)
    VV_OP_DECL(vmxnor_mm)

    // 15.2. Vector count population in mask vcpop.m
    uint8_t vcpop_m(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                    uint8_t const masked_instruction_bit, uint8_t pRd, uint8_t const vs2, uint16_t const vstart,
                    uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    // 15.3. vfirst find-first-set mask bit
    uint8_t vfirst_m(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                     uint8_t const masked_instruction_bit, uint8_t pRd, uint8_t const vs2, uint16_t const vstart,
                     uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    // 15.4. vmsbf.m set-before-first mask bit
    uint8_t vmsbf_m(void *const vector_field, uint16_t const vtype, uint8_t const mask_bit, uint8_t const vd,
                    uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    // 15.5. vmsif.m set-including-first mask bit
    uint8_t vmsif_m(void *const vector_field, uint16_t const vtype, uint8_t const mask_bit, uint8_t const vd,
                    uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    // 15.6. vmsof.m set-only-first mask bit
    uint8_t vmsof_m(void *const vector_field, uint16_t const vtype, uint8_t const mask_bit, uint8_t const vd,
                    uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    // 15.8. Vector Iota Instruction
    uint8_t viota_m(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                    uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    // 15.9. Vector Element Index Instruction
    uint8_t vid_v(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                  uint8_t const vd, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    // 16. Vector Permutation Instructions
    // 16.1. Integer Scalar Move Instructions
    uint8_t vmv_xs(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t pRd,
                   uint8_t const vs2, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    uint8_t vmv_sx(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
                   uint8_t pRs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    // 16.2. Floating-Point Scalar Move Instructions
    uint8_t vfmv_f_s(void *const vector_field, void *pF, uint16_t const vtype, uint8_t pRd, uint8_t const vs2,
                     uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen);

    uint8_t vfmv_s_f(void *const vector_field, void *pF, uint16_t const vtype, uint8_t const vd, uint8_t pRs1,
                     uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen);

    // 16.3. Vector Slide Instructions
    uint8_t vslideup_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                        uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t pRs1,
                        uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    uint8_t vslideup_vi(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                        uint8_t const vd, uint8_t const vs2, uint8_t pVimm, uint16_t const vstart, uint32_t const vlen,
                        uint32_t const vl);

    uint8_t vslidedown_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                          uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t pRs1,
                          uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    uint8_t vslidedown_vi(void *const vector_field, uint16_t const vtype, uint8_t const masked_instruction_bit,
                          uint8_t const vd, uint8_t const vs2, uint8_t pVimm, uint16_t const vstart,
                          uint32_t const vlen, uint32_t const vl);

    uint8_t vslide1up_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                         uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t pRs1,
                         uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    uint8_t vfslide1up_vf(void *const vector_field, void *pF, uint16_t const vtype,
                          uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t pRs1,
                          uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen);

    uint8_t vslide1down_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                           uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t pRs1,
                           uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen);

    uint8_t vfslide1down_vf(void *const vector_field, void *pF, uint16_t const vtype,
                            uint8_t const masked_instruction_bit, uint8_t const vd, uint8_t const vs2, uint8_t pRs1,
                            uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen);

    // 16.4. Vector Register Gather Instructions
    VV_OP_DECL(vrgather_vv)
    VV_OP_DECL(vrgatherei16_vv)
    VX_OP_DECL(vrgather_vx)
    VI_OP_DECL(vrgather_vi)

    // 16.5. Vector Compress Instruction
    uint8_t vcompress_vm(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1,
                         uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    // 16.6. Whole Vector Register Move
    uint8_t vmvr_v(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs2,
                   uint8_t const imm, uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

    // Matrix
    uint8_t vmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                      uint16_t const vs2, uint16_t const vstart, uint32_t const vlen);

    uint8_t vwmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                      uint16_t const vs2, uint16_t const vstart, uint32_t const vlen);

    uint8_t vqwmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                        uint16_t const vs2, uint16_t const vstart, uint32_t const vlen);

    uint8_t vfmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                       uint16_t const vs2, uint16_t const vstart, uint32_t const vlen, uint8_t const rounding_mode);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __SOFTVECTOR_H__ */
