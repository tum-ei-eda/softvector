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
/// \file permutation.hpp
/// \brief Defines helpers implementing permutation operations after
/// https://github.com/riscv/riscv-v-spec/blob/0.9/v-spec.adoc#vector-arithmetic-instruction-formats
/// \date 09/09/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_MISC_PERMUTATION_H__
#define __RVVHL_MISC_PERMUTATION_H__

#include "stdint.h"
#include "base/base.hpp"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes vector permutation operation helpers
namespace VPERM
{
struct PermInstrInfo
{
    bool slide_down = false;   //!< True if slide direction is down, up otherwise
    bool slide_single = false; //!< True for slide1up/down
    bool float_instr = false;  //!< True if floating-point slide instruction, false otherwise
};

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Vector slide instruction from scalar
/// \details
auto perm_op_slide_vx(uint8_t *vec_reg_mem,                 //!< Vector register file memory space. One dimensional
                      VInstrInfo const &v_instr_info,       //!< Struct containing vector instruction information
                      PermInstrInfo const &perm_instr_info, //!< Struct containing permutation instruction information
                      uint16_t reg_vd,                      //!< Destination vector D [index]
                      uint16_t reg_vs2,                     //!< Source vector L [index]
                      uint8_t *scalar_reg_mem,              //!< Memory space holding scalar data (min. _xlenb bytes)
                      uint8_t scalar_reg_len_bytes          //!< Length of scalar [bytes]
                      ) -> VILL::vpu_return_t;

auto perm_op_move_float(uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
                        VInstrInfo const &v_instr_info, //!< Struct containing vector instruction information
                        uint16_t reg_v,                 //!< Destination vector D [index]
                        uint8_t *scalar_reg_mem,        //!< Memory space holding scalar data (min. _xlenb bytes)
                        uint8_t flen,                   //!< Length of scalar [bit]
                        bool vec_is_dest                //!< True if destination is a vector
                        ) -> VILL::vpu_return_t;

/* rvv spec. 17.1. Integer Scalar Move Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move first element (0) of a vector register A to scalar register X := SEW>XLEN ? A[0] : sext(A[0])
/// \details ignore LMUL
VILL::vpu_return_t mv_xs(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                         uint16_t sew_bytes,          //!< Element width [bytes]
                         uint16_t vec_len,            //!< Vector length [elements]
                         uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                         uint16_t src_vec_reg,        //!< Source vector A [index]
                         uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                         uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move scalar register to first element (0) of a vector register A[0] := SEW<XLEN ? X : sext(X)
/// \details ignore LMUL
VILL::vpu_return_t mv_sx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                         uint16_t sew_bytes,          //!< Element width [bytes]
                         uint16_t vec_len,            //!< Vector length [elements]
                         uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                         uint16_t dst_vec_reg,        //!< Destination vector register A [index]
                         uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                         uint16_t vec_elem_start,     //!< Starting element [index]
                         uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

/* rvv spec. 17.2. Floating-Point Scalar Move Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move first element (0) of a vector register A to scalar fp register F := SEW>XLEN ? A[0] : sext(A[0])
/// \details ignore LMUL
VILL::vpu_return_t fmv_fs(uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
                          uint16_t sew_bytes,             //!< Element width [bytes]
                          uint16_t vec_len,               //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes,     //!< Vector register length [bytes]
                          uint16_t src_vec_reg,           //!< Source vector A [index]
                          uint8_t *scalar_fp_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                          uint8_t scalar_fp_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move scalar fp register to first element (0) of a vector register A[0] := SEW<XLEN ? F : sext(F)
/// \details ignore LMUL
VILL::vpu_return_t fmv_sf(
    uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
    uint16_t sew_bytes,             //!< Element width [bytes]
    uint16_t vec_len,               //!< Vector length [elements]
    uint16_t vec_reg_len_bytes,     //!< Vector register length [bytes]
    uint16_t dst_vec_reg,           //!< Destination vector register A [index]
    uint8_t *scalar_fp_reg_mem,     //!< Memory space holding floating-point scalar data (min. _xlenb bytes)
    uint16_t vec_elem_start,        //!< Starting element [index]
    uint8_t scalar_fp_reg_len_bytes //!< Length of floating-point scalar [bytes]
);

/* rvv spec. 17.3. Vector Slide Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslideup.vx vd, vs2, rs1, vm        # vd[i+rs1] = vs2[i]
/// \details
VILL::vpu_return_t slideup_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                              uint64_t emul_num,           //!< Register multiplicity numerator
                              uint64_t emul_denom,         //!< Register multiplicity denominator
                              uint16_t sew_bytes,          //!< Element width [bytes]
                              uint16_t vec_len,            //!< Vector length [elements]
                              uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                              uint16_t dst_vec_reg,        //!< Destination vector D [index]
                              uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                              uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                              uint16_t vec_elem_start,     //!< Starting element [index]
                              bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                              uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslideup.vi vd, vs2, uimm[4:0], vm        # vd[i+rs1] = vs2[i]
/// \details
VILL::vpu_return_t slideup_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                              uint64_t emul_num,          //!< Register multiplicity numerator
                              uint64_t emul_denom,        //!< Register multiplicity denominator
                              uint16_t sew_bytes,         //!< Element width [bytes]
                              uint16_t vec_len,           //!< Vector length [elements]
                              uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                              uint16_t dst_vec_reg,       //!< Destination vector D [index]
                              uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                              uint8_t u_imm,              //!< Sign extending 5-bit immediate
                              uint16_t vec_elem_start,    //!< Starting element [index]
                              bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslidedown.vx vd, vs2, rs1, vm        # vd[i+rs1] = vs2[i]
/// \details
VILL::vpu_return_t slidedown_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                                uint64_t emul_num,           //!< Register multiplicity numerator
                                uint64_t emul_denom,         //!< Register multiplicity denominator
                                uint16_t sew_bytes,          //!< Element width [bytes]
                                uint16_t vec_len,            //!< Vector length [elements]
                                uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                                uint16_t dst_vec_reg,        //!< Destination vector D [index]
                                uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                                uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                                uint16_t vec_elem_start,     //!< Starting element [index]
                                bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                                uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslidedown.vi vd, vs2, uimm[4:0], vm        # vd[i+rs1] = vs2[i]
/// \details
VILL::vpu_return_t slidedown_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                                uint64_t emul_num,          //!< Register multiplicity numerator
                                uint64_t emul_denom,        //!< Register multiplicity denominator
                                uint16_t sew_bytes,         //!< Element width [bytes]
                                uint16_t vec_len,           //!< Vector length [elements]
                                uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                                uint16_t dst_vec_reg,       //!< Destination vector D [index]
                                uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                                uint8_t u_imm,              //!< Sign extending 5-bit immediate
                                uint16_t vec_elem_start,    //!< Starting element [index]
                                bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslide1up.vx vd, vs2, rs1, vm        # vd[0]=x[rs1], vd[i+1] = vs2[i]
/// \details
VILL::vpu_return_t slide1up(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,           //!< Register multiplicity numerator
                            uint64_t emul_denom,         //!< Register multiplicity denominator
                            uint16_t sew_bytes,          //!< Element width [bytes]
                            uint16_t vec_len,            //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,        //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,     //!< Starting element [index]
                            bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vfslide1up.vf vd, vs2, fs1, vm        # vd[0]=f[rs1], vd[i+1] = vs2[i]
/// \details
VILL::vpu_return_t fslide1up(
    uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
    uint64_t emul_num,              //!< Register multiplicity numerator
    uint64_t emul_denom,            //!< Register multiplicity denominator
    uint16_t sew_bytes,             //!< Element width [bytes]
    uint16_t vec_len,               //!< Vector length [elements]
    uint16_t vec_reg_len_bytes,     //!< Vector register length [bytes]
    uint16_t dst_vec_reg,           //!< Destination vector D [index]
    uint16_t src_vec_reg_lhs,       //!< Source vector L [index]
    uint8_t *scalar_fp_reg_mem,     //!< Memory space holding floating-point scalar data (min. _xlenb bytes)
    uint16_t vec_elem_start,        //!< Starting element [index]
    bool mask_f,                    //!< Vector mask flag. 1: masking 0: no masking
    uint8_t scalar_fp_reg_len_bytes //!< Length of floating-point scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vslide1down.vx  vd, vs2, rs1, vm      # vd[i] = vs2[i+1], vd[vl-1]=x[rs1]
/// \details
VILL::vpu_return_t slide1down(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                              uint64_t emul_num,           //!< Register multiplicity numerator
                              uint64_t emul_denom,         //!< Register multiplicity denominator
                              uint16_t sew_bytes,          //!< Element width [bytes]
                              uint16_t vec_len,            //!< Vector length [elements]
                              uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                              uint16_t dst_vec_reg,        //!< Destination vector D [index]
                              uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                              uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                              uint16_t vec_elem_start,     //!< Starting element [index]
                              bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                              uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief vfslide1down.vf vd, vs2, fs1, vm      # vd[i] = vs2[i+1], vd[vl-1]=f[rs1]
/// \details
VILL::vpu_return_t fslide1down(
    uint8_t *vec_reg_mem,           //!< Vector register file memory space. One dimensional
    uint64_t emul_num,              //!< Register multiplicity numerator
    uint64_t emul_denom,            //!< Register multiplicity denominator
    uint16_t sew_bytes,             //!< Element width [bytes]
    uint16_t vec_len,               //!< Vector length [elements]
    uint16_t vec_reg_len_bytes,     //!< Vector register length [bytes]
    uint16_t dst_vec_reg,           //!< Destination vector D [index]
    uint16_t src_vec_reg_lhs,       //!< Source vector L [index]
    uint8_t *scalar_fp_reg_mem,     //!< Memory space holding floating-point scalar data (min. _xlenb bytes)
    uint16_t vec_elem_start,        //!< Starting element [index]
    bool mask_f,                    //!< Vector mask flag. 1: masking 0: no masking
    uint8_t scalar_fp_reg_len_bytes //!< Length of floating-point scalar [bytes]
);

/* 16.4. Vector Register Gather Instructions */
VILL::vpu_return_t vrgather_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                               uint64_t lmul_num,          //!< Register multiplicity numerator
                               uint64_t lmul_denom,        //!< Register multiplicity denominator
                               uint16_t sew_bytes,         //!< Element width [bytes]
                               uint16_t vec_len,           //!< Vector length [elements]
                               uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                               uint16_t dst_vec_reg,       //!< Destination vector D [index]
                               uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                               uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                               uint16_t vec_elem_start,    //!< Starting element [index]
                               bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                               bool ei16                   //!< Instruction is vrgatherei16.vv, not vrgather.vv
);

VILL::vpu_return_t vrgatherei16_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                                   uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                                   uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start,
                                   bool mask_f);

VILL::vpu_return_t vrgather_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                               uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                               uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vrgather_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                               uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                               uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                               uint8_t scalar_reg_len_bytes);
/* End 16.4. */

/* 16.5. Vector Compress Instruction */
VILL::vpu_return_t vcompress_vm(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                                uint64_t lmul_num,          //!< Register multiplicity numerator
                                uint64_t lmul_denom,        //!< Register multiplicity denominator
                                uint16_t sew_bytes,         //!< Element width [bytes]
                                uint16_t vec_len,           //!< Vector length [elements]
                                uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                                uint16_t dst_vec_reg,       //!< Destination vector D [index]
                                uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                                uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                                uint16_t vec_elem_start     //!< Starting element [index]
);
/* End 16.5. */

/* 16.6. Whole Vector Register Move */
VILL::vpu_return_t vmvr_v(uint8_t *vec_reg_mem, uint64_t lmul_num, uint64_t lmul_denom, uint16_t sew_bytes,
                          uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg, uint16_t src_vec_reg_lhs,
                          uint8_t simm, uint16_t vec_elem_start);
/* End 16.6. */

/* rvv spec. 17.4. Vector Register Gather Instruction */
// TODO: ...
/* rvv spec. 17.5. Vector Compress Instruction */
// TODO: ...
/* rvv spec. 17.6. Whole Vector Register Move */
// TODO: ...
} /* namespace VPERM  */

#endif /* __RVVHL_MISC_PERMUTATION_H__ */
