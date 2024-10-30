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
/// \file base.hpp
/// \brief Defines helpers implementing configuration-specifics
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __RVVHL_BASE_H__
#define __RVVHL_BASE_H__

#include "stdint.h"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes basic helpers for Illegal-Instruction-related stuff.
namespace VILL
{

typedef enum VPU_RETURN
{
    NO_EXCEPT = 0,
    DST_VEC_ILL,
    SRC1_VEC_ILL,
    SRC2_VEC_ILL,
    SRC3_VEC_ILL,
    WIDENING_OVERLAP_VD_VS1_ILL,
    WIDENING_OVERLAP_VD_VS2_ILL,
    NARROWING_OVERLAP_VD_VS2_ILL,
} vpu_return_t;

}

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes basic helpers for VTYPE-related configuration fields.
namespace VTYPE
{

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Bit-wise masks for VTYPE bitfield
typedef enum MASK
{
    // MSKSEW = 0x1C,
    MSKSEW = 0x38,
    // MSKLMUL = 0x03,
    MSKLMUL = 0x07,
    MSKFLMUL = 0x20, // ?
    MSKTA = 0x40,
    MSKMA = 0x80,
} mask_t;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief VTYPE bitfield element offsets
typedef enum OFFSETS
{
    // OFFSEW = 2,
    OFFSEW = 3,
    OFFFLMUL = 3
} offsets_t;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Bit coding for SEW
typedef enum BITS_SEW
{
    E8 = 0x0,
    E16 = 0x1,
    E32 = 0x2,
    E64 = 0x3,
    E128 = 0x4,
    E256 = 0x5,
    E512 = 0x6,
    E1024 = 0x7
} bits_sew_t;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Bit coding for LMUL
typedef enum BITS_LMUL
{
    RES = 0x4,
    MF8 = 0x5,
    MF4 = 0x6,
    MF2 = 0x7,
    M1 = 0x0,
    M2 = 0x1,
    M4 = 0x2,
    M8 = 0x3
} bits_LMUL_t;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Bit coding for TA
typedef enum BITS_TA
{
    TAU = 0x0,
    TAGN = 0x1
} bits_TA_t;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Bit coding for MA
typedef enum BITS_MA
{
    MAU = 0x0,
    MAGN = 0x1
} bits_MA_t;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Bit coding for EEW
typedef enum BITS_EEW
{
    EEW_8 = 0x0,
    EEW_16 = 0x5,
    EEW_32 = 0x6,
    EEW_64 = 0x7,
    EEW_128 = 0x8,
    EEW_256 = 0xd,
    EEW_512 = 0xe,
    EEW_1024 = 0xf
} bits_eew_t;

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Decode a VTYPE bitfield and store retrieved fields to Output parameter set
/// \return If field valid 1, else -1 (e.g. reserved LMUL code)
int8_t decode(uint16_t vtype, uint8_t *ta, uint8_t *ma, uint32_t *sew, uint8_t *z_lmul, uint8_t *n_lmul);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Encode Input parameter set of bitfields to a VTYPE bitfield
/// \return Encoded VTYPE bitfield
uint16_t encode(uint16_t sew, uint8_t z_lmul, uint8_t n_lmul, uint8_t ta, uint8_t ma);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract SEW bitfield from VTYPE bitfield
/// \return Encoded SEW bitfield
uint8_t extractSEW(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract LMUL bitfield from VTYPE bitfield
/// \return Encoded LMUL bitfield
uint8_t extractLMUL(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract TA bitfield from VTYPE bitfield
/// \return Encoded TA bitfield
uint8_t extractTA(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract MA bitfield from VTYPE bitfield
/// \return Encoded MA bitfield
uint8_t extractMA(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Concatenate MEW and WIDTH to EEW and return number of bits for EEW
/// \return Decoded EEW [bits]
uint16_t concatEEW(uint8_t mew, uint8_t width);

//////////////////////////////////////////////////////////////////////////////////////
/// \class VTYPE
/// \brief decodes (vtype bitvector) or encodes (variables to vtype bv) on construction
class VTYPE
{
  public:
    uint16_t _bitfield{};
    uint8_t _z_lmul{}, _n_lmul{}, _ta{}, _ma{};
    uint32_t _sew{};
    VTYPE(uint16_t _vtype_bitfield) : _bitfield(_vtype_bitfield)
    {
        decode(_bitfield, &_ta, &_ma, &_sew, &_z_lmul, &_n_lmul);
    }
    VTYPE(uint16_t sew, uint8_t z_lmul, uint8_t n_lmul, uint8_t ta, uint8_t ma)
        : _z_lmul(z_lmul), _n_lmul(n_lmul), _ta(ta), _ma(ma), _sew(sew)
    {
        _bitfield = encode(_sew, _z_lmul, _n_lmul, _ta, _ma);
    }
};
} // namespace VTYPE

#endif /* __RVVHL_BASE_H__ */
