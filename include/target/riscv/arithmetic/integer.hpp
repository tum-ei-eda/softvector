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
/// \file integer.hpp
/// \brief Defines helpers implementing integer arithmetics after
/// https://github.com/riscv/riscv-v-spec/blob/0.9/v-spec.adoc#vector-arithmetic-instruction-formats
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_ARITH_INTEGER_H__
#define __RVVHL_ARITH_INTEGER_H__

#include "stdint.h"
#include "base/base.hpp"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes integer arithmetic helpers
namespace VARITH_INT
{

/* rvv spec. 12.1 - Vector Single-Width Add and Substract */
/* ADD */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Add vector-vector
/// \details For all i: D[i] = L[i] + R[i]
VILL::vpu_return_t add_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Add vector-immediate
/// \details For all i: D[i] = L[i] + sign_extend(_vimm)
VILL::vpu_return_t add_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint8_t s_imm,              //!< Sign or zero extending 5-bit immediate
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Add vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X)
VILL::vpu_return_t add_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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

/* SUB */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Sub vector-vector
/// \details For all i: D[i] = L[i] - R[i]
VILL::vpu_return_t sub_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Sub vector-scalar
/// \details For all i: D[i] = L[i] - sign_extend(*X). No sub_vi (use add_vi with negative immediate
VILL::vpu_return_t sub_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* Reverse-SUB */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Sub vector-scalar
/// \details For all i: D[i] = sign_extend(*X) - R[i].
VILL::vpu_return_t rsub_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,           //!< Register multiplicity numerator
                           uint64_t emul_denom,         //!< Register multiplicity denominator
                           uint16_t sew_bytes,          //!< Element width [bytes]
                           uint16_t vec_len,            //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,        //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,    //!< Source vector R [index]
                           uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                           uint16_t vec_elem_start,     //!< Starting element [index]
                           bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                           uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Sub vector-scalar
/// \details For all i: D[i] = sign_extend(imm) - R[i].
VILL::vpu_return_t rsub_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint8_t s_imm,              //!< Sign or zero extending 5-bit immediate
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

/* rvv spec. 12.2 - Vector Widening Add/Substract */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening unsigned <OP> Add vector-vector
/// \details For all i: D[i] = L[i] + R[i]. w/ D:2*SEW, L:SEW,  R:SEW
VILL::vpu_return_t wop_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                          bool dir_f,                 //!< Operation type: val > 0: ADD else SUB
                          bool signed_f               //!< Signed or unsigned operation type: vaL = true: signed
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening unsigned <OP> Add vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X). w/ D:2*SEW, L:SEW
VILL::vpu_return_t wop_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
                          bool dir_f,                  //!< Operation type: val > 0: ADD else SUB
                          bool signed_f,               //!< Signed or unsigned operation type: vaL = true: signed
                          uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening integer <OP> vector-vector
/// \details For all i: D[i] = L[i] + R[i]. w/ D:2*SEW, L:SEW,  R:SEW
VILL::vpu_return_t wop_wv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                          bool dir_f,                 //!< Operation type: val > 0: ADD else SUB
                          bool signed_f               //!< Signed or unsigned operation type: vaL = true: signed
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening unsigned <OP> Add vector-scalar
/// \details For all i: D[i] = L[i] + sign_extend(*X). w/ D:2*SEW, L:SEW
VILL::vpu_return_t wop_wx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
                          bool dir_f,                  //!< Operation type: val > 0: ADD else SUB
                          bool signed_f,               //!< Signed or unsigned operation type: vaL = true: signed
                          uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

/// TODO: vzext.vf{2,4,8}
/// TODO: vsext.vf{2,4,8}

/* 11.3. Vector Integer Extension */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Sign/zero-extend vector
/// \details Sign/zero-extend SEW / {2|4|8} source to SEW destination
VILL::vpu_return_t vext_vf(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,           //!< Register multiplicity numerator
                           uint64_t emul_denom,         //!< Register multiplicity denominator
                           uint16_t sew_bytes,          //!< Element width [bytes]
                           uint16_t vec_len,            //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,        //!< Destination vector D [index]
                           uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                           uint16_t extension_encoding, //!< Encoding of sign/zero and divider
                           uint16_t vec_elem_start,     //!< Starting element [index]
                           bool mask_f                  //!< Vector mask flag. 1: masking 0: no masking
);
/* End 11.3. */

/* 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions */
VILL::vpu_return_t vadc_vvm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start);

VILL::vpu_return_t vadc_vim(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start);

VILL::vpu_return_t vadc_vxm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                            uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vmadc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vmadc_vi(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t s_imm, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vmadc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                            uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vsbc_vvm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start);

VILL::vpu_return_t vsbc_vxm(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start,
                            uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vmsbc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vmsbc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                            uint8_t scalar_reg_len_bytes);
/* End 11.4 */

/* 11.5 - Vector Bitwise Logical Instructions */
/* AND */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief AND vector-vector
/// \details For all i: D[i] = L[i] & R[i]
VILL::vpu_return_t and_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief AND vector-immediate
/// \details For all i: D[i] = L[i] & sign_extend(_vimm)
VILL::vpu_return_t and_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint8_t s_imm,              //!< Sign or zero extending 5-bit immediate
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief AND vector-scalar
/// \details For all i: D[i] = L[i] & sign_extend(*X)
VILL::vpu_return_t and_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,           //!< Register multiplicity numerator
                          uint64_t emul_denom,         //!< Register multiplicity denominator
                          uint16_t sew_bytes,          //!< Element width [bytes]
                          uint16_t vec_len,            //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,        //!< Destination vector D [index]
                          uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
                          uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes).
                          uint16_t vec_elem_start,     //!< Starting element [index]
                          bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
                          uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

/* OR */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief OR vector-vector
/// \details For all i: D[i] = L[i] | R[i]
VILL::vpu_return_t or_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                         uint64_t emul_num,          //!< Register multiplicity numerator
                         uint64_t emul_denom,        //!< Register multiplicity denominator
                         uint16_t sew_bytes,         //!< Element width [bytes]
                         uint16_t vec_len,           //!< Vector length [elements]
                         uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                         uint16_t dst_vec_reg,       //!< Destination vector D [index]
                         uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                         uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                         uint16_t vec_elem_start,    //!< Starting element [index]
                         bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief OR vector-immediate
/// \details For all i: D[i] = L[i] | sign_extend(_vimm)
VILL::vpu_return_t or_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                         uint64_t emul_num,          //!< Register multiplicity numerator
                         uint64_t emul_denom,        //!< Register multiplicity denominator
                         uint16_t sew_bytes,         //!< Element width [bytes]
                         uint16_t vec_len,           //!< Vector length [elements]
                         uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                         uint16_t dst_vec_reg,       //!< Destination vector D [index]
                         uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                         uint8_t s_imm,              //!< Sign or zero extending 5-bit immediate
                         uint16_t vec_elem_start,    //!< Starting element [index]
                         bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief OR vector-scalar
/// \details For all i: D[i] = L[i] | sign_extend(*X)
VILL::vpu_return_t or_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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

/* XOR */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief XOR vector-vector
/// \details For all i: D[i] = L[i] ^ R[i]
VILL::vpu_return_t xor_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief XOR vector-immediate
/// \details For all i: D[i] = L[i] ^ sign_extend(_vimm)
VILL::vpu_return_t xor_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint8_t s_imm,              //!< Sign or zero extending 5-bit immediate
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief XOR vector-scalar
/// \details For all i: D[i] = L[i] ^ X.
VILL::vpu_return_t xor_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* End 11.5. */

/* 11.6. Vector Single-Width Bit Shift Instructions */
/* SLL */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SLL vector-vector
/// \details For all i: D[i] = R[i] << (L[i] & possible SEW bits)
VILL::vpu_return_t sll_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SLL vector-immediate
/// \details For all i: D[i] = L[i] << (uimm)
VILL::vpu_return_t sll_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint8_t u_imm,              //!< Zero extending 5-bit immediate
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SLL vector-scalar
/// \details For all i: D[i] = L[i] << (X & possible SEW bits)
VILL::vpu_return_t sll_vx(
    uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
    uint64_t emul_num,           //!< Register multiplicity numerator
    uint64_t emul_denom,         //!< Register multiplicity denominator
    uint16_t sew_bytes,          //!< Element width [bytes]
    uint16_t vec_len,            //!< Vector length [elements]
    uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
    uint16_t dst_vec_reg,        //!< Destination vector D [index]
    uint16_t src_vec_reg_lhs,    //!< Source vector L [index]
    uint8_t *scalar_reg_mem,     //!< X: Memory space holding scalar data (min. scalar_reg_len_bytes bytes)
    uint16_t vec_elem_start,     //!< Starting element [index]
    bool mask_f,                 //!< Vector mask flag. 1: masking 0: no masking
    uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

/* SRL */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRL vector-vector
/// \details For all i: D[i] = R[i] >> (L[i] & possible SEW bits)
VILL::vpu_return_t srl_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRL vector-immediate
/// \details For all i: D[i] = L[i] >> (uimm)
VILL::vpu_return_t srl_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint8_t u_imm,              //!< Zero extending 5-bit immediate
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRL vector-scalar
/// \details For all i: D[i] = L[i] >> (X & possible SEW bits)
VILL::vpu_return_t srl_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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

/* SRA */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRA vector-vector
/// \details For all i: D[i] = R[i] >> (L[i] & possible SEW bits)
VILL::vpu_return_t sra_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRA vector-immediate
/// \details For all i: D[i] = L[i] >> (uimm)
VILL::vpu_return_t sra_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                          uint64_t emul_num,          //!< Register multiplicity numerator
                          uint64_t emul_denom,        //!< Register multiplicity denominator
                          uint16_t sew_bytes,         //!< Element width [bytes]
                          uint16_t vec_len,           //!< Vector length [elements]
                          uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                          uint16_t dst_vec_reg,       //!< Destination vector D [index]
                          uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                          uint8_t u_imm,              //!< Zero extending 5-bit immediate
                          uint16_t vec_elem_start,    //!< Starting element [index]
                          bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief SRA vector-scalar
/// \details For all i: D[i] = L[i] >> (X & possible SEW bits)
VILL::vpu_return_t sra_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* End 11.6. */

/* 11.7. Vector Narrowing Integer Right Shift Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing SRL vector-vector
/// \details For all i: D[i] = R[i] >> (L[i] & possible SEW bits), SEW = 2*SEW >> SEW
VILL::vpu_return_t vnsrl_wv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing SRL vector-immediate
/// \details For all i: D[i] = L[i] >> (uimm), SEW = 2*SEW >> SEW
VILL::vpu_return_t vnsrl_wi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint8_t u_imm,              //!< Zero extending 5-bit immediate
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing SRL vector-scalar
/// \details For all i: D[i] = L[i] >> (X & possible SEW bits), SEW = 2*SEW >> SEW
VILL::vpu_return_t vnsrl_wx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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

/* SRA */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing SRA vector-vector
/// \details For all i: D[i] = R[i] >> (L[i] & possible SEW bits), SEW = 2*SEW >> SEW
VILL::vpu_return_t vnsra_wv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing SRA vector-immediate
/// \details For all i: D[i] = L[i] >> (uimm), SEW = 2*SEW >> SEW
VILL::vpu_return_t vnsra_wi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint8_t u_imm,              //!< Zero extending 5-bit immediate
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Narrowing SRA vector-scalar
/// \details For all i: D[i] = L[i] >> (X & possible SEW bits), SEW = 2*SEW >> SEW
VILL::vpu_return_t vnsra_wx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* End 11.7. */

/* 12.8 Vector Integer Comparison Instructions*/
/* MSEQ */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SEQ vector-vector
/// \details For all i: MaskReg[i] = R[i] == L[i]
VILL::vpu_return_t mseq_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SEQ vector-immediate
/// \details For all i: MaskReg[i] = R[i] == immediate
VILL::vpu_return_t mseq_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint8_t s_imm,              //!< Sign extending 5-bit immediate
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SEQ vector-scalar
/// \details For all i: MaskReg[i] = R[i] == zero_extend(X)
VILL::vpu_return_t mseq_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* MSNE */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SNE vector-vector
/// \details For all i: MaskReg[i] = R[i] != L[i]
VILL::vpu_return_t msne_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SNE vector-immediate
/// \details For all i: MaskReg[i] = R[i] != immediate
VILL::vpu_return_t msne_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint8_t s_imm,              //!< Sign extending 5-bit immediate
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SNE vector-scalar
/// \details For all i: MaskReg[i] = R[i] != zero_extend(X)
VILL::vpu_return_t msne_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* MSLTU */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLTU vector-vector (less than unsigned)
/// \details For all i: MaskReg[i] = R[i] < L[i]
VILL::vpu_return_t msltu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLTU vector-scalar (less than unsigned)
/// \details For all i: MaskReg[i] = R[i] < zero_extend(X)
VILL::vpu_return_t msltu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* MSLT */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLT vector-vector (less than signed)
/// \details For all i: MaskReg[i] = R[i] < L[i]
VILL::vpu_return_t mslt_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLT vector-scalar (less than signed)
/// \details For all i: MaskReg[i] = R[i] < zero_extend(X)
VILL::vpu_return_t mslt_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* MSLEU */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLEU vector-vector (less than or equal unsigned)
/// \details For all i: MaskReg[i] = R[i] < L[i]
VILL::vpu_return_t msleu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLEU vector-immediate (less than or equal unsigned)
/// \details For all i: MaskReg[i] = R[i] < immediate
VILL::vpu_return_t msleu_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint8_t u_imm,              //!< Zero extending 5-bit immediate
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLEU vector-scalar (less than or equal unsigned)
/// \details For all i: MaskReg[i] = R[i] < zero_extend(X)
VILL::vpu_return_t msleu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* MSLE */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLE vector-vector (less than or equal signed)
/// \details For all i: MaskReg[i] = R[i] <= L[i]
VILL::vpu_return_t msle_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLE vector-immediate (less than or equal signed)
/// \details For all i: MaskReg[i] = R[i] < immediate
VILL::vpu_return_t msle_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint8_t s_imm,              //!< Sign extending 5-bit immediate
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SLE vector-scalar (less than or equal signed)
/// \details For all i: MaskReg[i] = R[i] < zero_extend(X)
VILL::vpu_return_t msle_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* MSGTU */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SGTU vector-vector (less than unsigned)
/// \details For all i: MaskReg[i] = R[i] > L[i]
VILL::vpu_return_t msgtu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SGTU vector-scalar (less than unsigned)
/// \details For all i: MaskReg[i] = R[i] > zero_extend(X)
VILL::vpu_return_t msgtu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief (Mask register) SGTU vector-immediate (less than or equal signed)
/// \details For all i: MaskReg[i] = R[i] < immediate
VILL::vpu_return_t msgtu_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint8_t s_imm,              //!< Sign extending 5-bit immediate
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
/* MSGT */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SGT vector-vector (less than signed)
/// \details For all i: MaskReg[i] = R[i] > L[i]
VILL::vpu_return_t msgt_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief (Mask register) SGT vector-scalar (less than signed)
/// \details For all i: MaskReg[i] = R[i] > zero_extend(X)
VILL::vpu_return_t msgt_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief (Mask register) GTU vector-immediate (less than or equal signed)
/// \details For all i: MaskReg[i] = R[i] < immediate
VILL::vpu_return_t msgt_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint8_t s_imm,              //!< Sign extending 5-bit immediate
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

/* 12.10. Vector Single-Width Integer Multiply Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed Multiplication vector-vector low bits of product
/// \details For all i: D[i] = L[i] * R[i]
VILL::vpu_return_t vmul_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed Multiplication vector-scalar low bits of product
/// \details For all i: D[i] = L[i] * sign_extend(*X)
VILL::vpu_return_t vmul_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Signed Multiplication vector-vector high bits of product
/// \details For all i: D[i] = L[i] * R[i]
VILL::vpu_return_t vmulh_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed Multiplication vector-scalar high bits of product
/// \details For all i: D[i] = L[i] * sign_extend(*X)
VILL::vpu_return_t vmulh_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Unsigned Multiplication vector-vector high bits of product
/// \details For all i: D[i] = L[i] * R[i]
VILL::vpu_return_t vmulhu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                             uint64_t emul_num,          //!< Register multiplicity numerator
                             uint64_t emul_denom,        //!< Register multiplicity denominator
                             uint16_t sew_bytes,         //!< Element width [bytes]
                             uint16_t vec_len,           //!< Vector length [elements]
                             uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                             uint16_t dst_vec_reg,       //!< Destination vector D [index]
                             uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                             uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                             uint16_t vec_elem_start,    //!< Starting element [index]
                             bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Unsigned Multiplication vector-scalar high bits of product
/// \details For all i: D[i] = L[i] * sign_extend(*X)
VILL::vpu_return_t vmulhu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Signed * Unsigned Multiplication vector-vector high bits of product
/// \details For all i: D[i] = L[i] * R[i]
VILL::vpu_return_t vmulhsu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                              uint64_t emul_num,          //!< Register multiplicity numerator
                              uint64_t emul_denom,        //!< Register multiplicity denominator
                              uint16_t sew_bytes,         //!< Element width [bytes]
                              uint16_t vec_len,           //!< Vector length [elements]
                              uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                              uint16_t dst_vec_reg,       //!< Destination vector D [index]
                              uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                              uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                              uint16_t vec_elem_start,    //!< Starting element [index]
                              bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed * Unsigned Multiplication vector-scalar high bits of product
/// \details For all i: D[i] = L[i] * sign_extend(*X)
VILL::vpu_return_t vmulhsu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* End 11.10 */

/* 11.11. Vector Single-Width Integer Divide Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed division vector-vector
/// \details For all i: D[i] = L[i] / R[i]
VILL::vpu_return_t vdiv_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed division vector-vector
/// \details For all i: D[i] = L[i] / sign_extend(X[rs1])
VILL::vpu_return_t vdiv_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Unsigned division vector-vector
/// \details For all i: D[i] = L[i] / R[i]
VILL::vpu_return_t vdivu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Unsigned division vector-scalar
/// \details For all i: D[i] = L[i] / X[rs1]
VILL::vpu_return_t vdivu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Signed remainder vector-vector
/// \details For all i: D[i] = L[i] % R[i]
VILL::vpu_return_t vrem_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed remainder vector-vector
/// \details For all i: D[i] = L[i] % sign_extend(X[rs1])
VILL::vpu_return_t vrem_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Unsigned remainder vector-vector
/// \details For all i: D[i] = L[i] % R[i]
VILL::vpu_return_t vremu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Unsigned remainder vector-scalar
/// \details For all i: D[i] = L[i] % X[rs1]
VILL::vpu_return_t vremu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* End 11.11. */

/* 11.12. Vector Widening Integer Multiply Instructions*/
enum class VWMUL_TYPE
{
    S_S, // signed-signed
    U_U, // unsigned-unsigned
    S_U  // signed(vs2)-unsigned
};
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening signed Multiplication vector-vector
/// \details For all i: D[i] = L[i] * R[i]
VILL::vpu_return_t vwmul_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f,                //!< Vector mask flag. 1: masking 0: no masking
                            VWMUL_TYPE vwmul_type       //!< Type of multiplication
);
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening signed multiplication vector-scalar
/// \details For all i: D[i] = L[i] * sign_extend(*X)
VILL::vpu_return_t vwmul_vx(uint8_t *vec_reg_mem,         //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,            //!< Register multiplicity numerator
                            uint64_t emul_denom,          //!< Register multiplicity denominator
                            uint16_t sew_bytes,           //!< Element width [bytes]
                            uint16_t vec_len,             //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes,   //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,         //!< Destination vector D [index]
                            uint16_t src_vec_reg_lhs,     //!< Source vector L [index]
                            uint8_t *scalar_reg_mem,      //!< Memory space holding scalar data (min. _xlenb bytes)
                            uint16_t vec_elem_start,      //!< Starting element [index]
                            bool mask_f,                  //!< Vector mask flag. 1: masking 0: no masking
                            uint8_t scalar_reg_len_bytes, //!< Length of scalar [bytes]
                            VWMUL_TYPE vwmul_type         //!< Type of multiplication
);
/* End 11.12. */

/* 11.9. Vector Integer Min/Max Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed maximum vector-vector
/// \details For all i: D[i] = max(L[i], R[i])
VILL::vpu_return_t vmax_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed maximum vector-scalar
/// \details For all i: D[i] = max(L[i], sign_extend(*X))
VILL::vpu_return_t vmax_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Unsigned maximum vector-vector
/// \details For all i: D[i] = max(L[i], R[i])
VILL::vpu_return_t vmaxu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Unsigned maximum vector-scalar
/// \details For all i: D[i] = max(L[i], *X)
VILL::vpu_return_t vmaxu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Signed minimum vector-vector
/// \details For all i: D[i] = min(L[i], R[i])
VILL::vpu_return_t vmin_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                           uint64_t emul_num,          //!< Register multiplicity numerator
                           uint64_t emul_denom,        //!< Register multiplicity denominator
                           uint16_t sew_bytes,         //!< Element width [bytes]
                           uint16_t vec_len,           //!< Vector length [elements]
                           uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                           uint16_t dst_vec_reg,       //!< Destination vector D [index]
                           uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                           uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                           uint16_t vec_elem_start,    //!< Starting element [index]
                           bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Signed minimum vector-scalar
/// \details For all i: D[i] = min(L[i], sign_extend(*X))
VILL::vpu_return_t vmin_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/// \brief Unsigned minimum vector-vector
/// \details For all i: D[i] = min(L[i], R[i])
VILL::vpu_return_t vminu_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                            uint64_t emul_num,          //!< Register multiplicity numerator
                            uint64_t emul_denom,        //!< Register multiplicity denominator
                            uint16_t sew_bytes,         //!< Element width [bytes]
                            uint16_t vec_len,           //!< Vector length [elements]
                            uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                            uint16_t dst_vec_reg,       //!< Destination vector D [index]
                            uint16_t src_vec_reg_rhs,   //!< Source vector R [index]
                            uint16_t src_vec_reg_lhs,   //!< Source vector L [index]
                            uint16_t vec_elem_start,    //!< Starting element [index]
                            bool mask_f                 //!< Vector mask flag. 1: masking 0: no masking
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Unsigned minimum vector-scalar
/// \details For all i: D[i] = min(L[i], *X)
VILL::vpu_return_t vminu_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
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
/* End 11.9. */

/*12.12. Vector Widening Integer Multiply Instructions */
// TODO: ...
/*12.13. Vector Single-Width Integer Multiply-Add Instructions */
// TODO: ...
/*12.14. Vector Widening Integer Multiply-Add Instructions */
// TODO: ...
/*12.15. Vector Quad-Widening Integer Multiply-Add Instructions (Extension Zvqmac) */
// TODO: ...
/*12.16. Vector Integer Merge Instructions */
// TODO: ...

/* 11.13. Vector Single-Width Integer Multiply-Add Instructions */
VILL::vpu_return_t vmacc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vmacc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                            uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vnmsac_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                             uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                             uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vnmsac_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                             uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                             uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                             uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vmadd_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vmadd_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                            uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                            uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                            uint8_t scalar_reg_len_bytes);

VILL::vpu_return_t vnmsub_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                             uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                             uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f);

VILL::vpu_return_t vnmsub_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                             uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                             uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                             uint8_t scalar_reg_len_bytes);
/* End 11.13. */

/* 11.14. Vector Widening Integer Multiply-Add Instructions */
enum class VWMACC_TYPE
{
    S_S, // signed-signed
    U_U, // unsigned-unsigned
    S_U, // signed(rs1/vs1)-unsigned(vs2)
    U_S  // unsigned(rs1/vs1)-signed(vs2)
};

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening multiply-accumulate vector-vector
/// Type of MACC (signed, unsigned, etc.) depends on vwmacc_type
VILL::vpu_return_t vwmacc_vv(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                             uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                             uint16_t src_vec_reg_rhs, uint16_t src_vec_reg_lhs, uint16_t vec_elem_start, bool mask_f,
                             VWMACC_TYPE vwmacc_type);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Widening multiply-accumulate vector-scalar
/// Type of MACC (signed, unsigned, etc.) depends on vwmacc_type
VILL::vpu_return_t vwmacc_vx(uint8_t *vec_reg_mem, uint64_t emul_num, uint64_t emul_denom, uint16_t sew_bytes,
                             uint16_t vec_len, uint16_t vec_reg_len_bytes, uint16_t dst_vec_reg,
                             uint16_t src_vec_reg_lhs, uint8_t *scalar_reg_mem, uint16_t vec_elem_start, bool mask_f,
                             uint8_t scalar_reg_len_bytes, VWMACC_TYPE vwmacc_type);
/* End 11.14. */

/* 11.15. Vector Integer Merge Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Merge vector-vector: vd[i] = v0.mask[i] ? vs1[i] : vs2[i]
VILL::vpu_return_t vmerge_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                             uint64_t emul_num,          //!< Register multiplicity numerator
                             uint64_t emul_denom,        //!< Register multiplicity denominator
                             uint16_t sew_bytes,         //!< Element width [bytes]
                             uint16_t vec_len,           //!< Vector length [elements]
                             uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                             uint16_t dst_vec_reg,       //!< Destination vector A [index]
                             uint16_t src_vec_reg_rhs,   //!< Source vector vs1 [index]
                             uint16_t src_vec_reg_lhs,   //!< Source vector vs2 [index]
                             uint16_t vec_elem_start     //!< Starting element [index]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Merge vector-scalar: vd[i] = v0.mask[i] ? x[rs1] : vs2[i]
VILL::vpu_return_t vmerge_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                             uint64_t emul_num,           //!< Register multiplicity numerator
                             uint64_t emul_denom,         //!< Register multiplicity denominator
                             uint16_t sew_bytes,          //!< Element width [bytes]
                             uint16_t vec_len,            //!< Vector length [elements]
                             uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                             uint16_t dst_vec_reg,        //!< Destination vector A [index]
                             uint16_t src_vec_reg_lhs,    //!< Source vector vs2 [index]
                             uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                             uint16_t vec_elem_start,     //!< Starting element [index]
                             uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Merge vector-scalar: vd[i] = v0.mask[i] ? imm : vs2[i]
VILL::vpu_return_t vmerge_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                             uint64_t emul_num,          //!< Register multiplicity numerator
                             uint64_t emul_denom,        //!< Register multiplicity denominator
                             uint16_t sew_bytes,         //!< Element width [bytes]
                             uint16_t vec_len,           //!< Vector length [elements]
                             uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                             uint16_t dst_vec_reg,       //!< Destination vector D [index]
                             uint16_t src_vec_reg_lhs,   //!< Source vector vs2 [index]
                             uint8_t s_imm,              //!< Sign extending 5-bit immediate
                             uint16_t vec_elem_start     //!< Starting element [index]
);
/* End 11.15. */

/* 11.16. Vector Integer Move Instructions */
//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move vector vd[i] = vs1[i]
VILL::vpu_return_t mv_vv(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                         uint64_t emul_num,          //!< Register multiplicity numerator
                         uint64_t emul_denom,        //!< Register multiplicity denominator
                         uint16_t sew_bytes,         //!< Element width [bytes]
                         uint16_t vec_len,           //!< Vector length [elements]
                         uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                         uint16_t dst_vec_reg,       //!< Destination vector A [index]
                         uint16_t src_vec_reg,       //!< Source vector A [index]
                         uint16_t vec_elem_start     //!< Starting element [index]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move vector vd[i] = X[rs1]
VILL::vpu_return_t mv_vx(uint8_t *vec_reg_mem,        //!< Vector register file memory space. One dimensional
                         uint64_t emul_num,           //!< Register multiplicity numerator
                         uint64_t emul_denom,         //!< Register multiplicity denominator
                         uint16_t sew_bytes,          //!< Element width [bytes]
                         uint16_t vec_len,            //!< Vector length [elements]
                         uint16_t vec_reg_len_bytes,  //!< Vector register length [bytes]
                         uint16_t dst_vec_reg,        //!< Destination vector A [index]
                         uint8_t *scalar_reg_mem,     //!< Memory space holding scalar data (min. _xlenb bytes)
                         uint16_t vec_elem_start,     //!< Starting element [index]
                         uint8_t scalar_reg_len_bytes //!< Length of scalar [bytes]
);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Move signed immediate to vector vd[i] = simm
VILL::vpu_return_t mv_vi(uint8_t *vec_reg_mem,       //!< Vector register file memory space. One dimensional
                         uint64_t emul_num,          //!< Register multiplicity numerator
                         uint64_t emul_denom,        //!< Register multiplicity denominator
                         uint16_t sew_bytes,         //!< Element width [bytes]
                         uint16_t vec_len,           //!< Vector length [elements]
                         uint16_t vec_reg_len_bytes, //!< Vector register length [bytes]
                         uint16_t dst_vec_reg,       //!< Destination vector D [index]
                         uint8_t s_imm,              //!< Sign extending 5-bit immediate
                         uint16_t vec_elem_start     //!< Starting element [index]
);
/* End 11.16. */

}; // namespace VARITH_INT

#endif /* __RVVHL_ARITH_INTEGER_H__ */
