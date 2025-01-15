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

#include <cstdint>
#include <cstddef>
#include "stdint.h"
#include "stddef.h"

//////////////////////////////////////////////////////////////////////////////////////
/// \brief This space concludes basic helpers for Illegal-Instruction-related stuff.
namespace VILL
{

typedef enum VPU_RETURN
{
    NO_EXCEPT = 0,
    NO_EXCEPT_FP_SAT, // Saturating FP operation did saturate
    DST_VEC_ILL,
    SRC1_VEC_ILL,
    SRC2_VEC_ILL,
    SRC3_VEC_ILL,
    WIDENING_OVERLAP_VD_VS1_ILL,
    WIDENING_OVERLAP_VD_VS2_ILL,
    NARROWING_OVERLAP_VD_VS2_ILL,
    DST_REG_SRC_REG_OVERLAP_ILL,
    DST_REG_MASK_REG_OVERLAP_ILL,
    DST_VEC_SRC_REG_OVERLAP_ILL,
    DST_VEC_MASK_REG_OVERLAP_ILL,
    VMVR_SIMM_ILL
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
int8_t decode(uint16_t vtype, std::uint8_t *ta, std::uint8_t *ma, uint32_t *sew, std::uint8_t *z_lmul,
              std::uint8_t *n_lmul);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Encode Input parameter set of bitfields to a VTYPE bitfield
/// \return Encoded VTYPE bitfield
uint16_t encode(uint16_t sew, std::uint8_t z_lmul, std::uint8_t n_lmul, std::uint8_t ta, std::uint8_t ma);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract SEW bitfield from VTYPE bitfield
/// \return Encoded SEW bitfield
std::uint8_t extractSEW(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract LMUL bitfield from VTYPE bitfield
/// \return Encoded LMUL bitfield
std::uint8_t extractLMUL(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract TA bitfield from VTYPE bitfield
/// \return Encoded TA bitfield
std::uint8_t extractTA(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Extract MA bitfield from VTYPE bitfield
/// \return Encoded MA bitfield
std::uint8_t extractMA(uint16_t pVTYPE);

//////////////////////////////////////////////////////////////////////////////////////
/// \brief Concatenate MEW and WIDTH to EEW and return number of bits for EEW
/// \return Decoded EEW [bits]
uint16_t concatEEW(std::uint8_t mew, std::uint8_t width);

//////////////////////////////////////////////////////////////////////////////////////
/// \class VTYPE
/// \brief decodes (vtype bitvector) or encodes (variables to vtype bv) on construction
class VTYPE
{
  public:
    uint16_t _bitfield{};
    std::uint8_t _z_lmul{}, _n_lmul{}, _ta{}, _ma{};
    uint32_t _sew{};
    VTYPE(uint16_t _vtype_bitfield) : _bitfield(_vtype_bitfield)
    {
        decode(_bitfield, &_ta, &_ma, &_sew, &_z_lmul, &_n_lmul);
    }
    VTYPE(uint16_t sew, std::uint8_t z_lmul, std::uint8_t n_lmul, std::uint8_t ta, std::uint8_t ma)
        : _z_lmul(z_lmul), _n_lmul(n_lmul), _ta(ta), _ma(ma), _sew(sew)
    {
        _bitfield = encode(_sew, _z_lmul, _n_lmul, _ta, _ma);
    }
};
} // namespace VTYPE

// General helper constants, functions, and structs, etc.

inline constexpr auto operator"" _u64(unsigned long long value) -> std::uint64_t
{
    return static_cast<std::uint64_t>(value);
}

inline constexpr auto operator"" _i64(unsigned long long value) -> std::int64_t
{
    return static_cast<std::int64_t>(value);
}

struct v_instr_info_t
{
    uint64_t emul_num = 1_u64;               //!< EMUL numerator
    uint64_t emul_denom = 1_u64;             //!< EMUL denominator
    uint32_t sew = 8_u64;                    //!< Selected element width (bit)
    uint16_t vector_length = 0U;          //!< Vector length (elements)
    uint16_t vector_register_length = 0U; //!< Length of a vector register (bit)
    uint16_t start_element = 0U;          //!< First element to be processed (index)
    bool masked = false;                  //!< True if masked instruction, false otherwise
    bool signed_op = false;               //!< True if the operation is signed, false otherwise
    bool zero_extend_immediate = false;   //!< True if the immediate is to be explicitly zero extended
    bool wide_vd = false;                 //!< True if this vector uses width 2*SEW
    bool wide_vs2 = false;                //!< True if this vector uses width 2*SEW
    bool wide_vs1 = false;                //!< True if this vector uses width 2*SEW
};

inline constexpr auto xlen_32_bytes = 4;

// Masks for 5 bit immediate
inline constexpr uint64_t imm_msb_mask = 0x10_u64;
inline constexpr uint64_t imm_width_mask = 0x1F_u64;
inline constexpr uint64_t imm_ext_mask = ~imm_width_mask;

inline auto sign_extend_immediate(std::uint8_t imm5) -> uint64_t
{
    return (imm5 & imm_msb_mask) ? (imm5 | imm_ext_mask) : (imm5 & imm_width_mask);
}

inline auto zero_extend_immediate(std::uint8_t imm5) -> uint64_t
{
    return imm5 & imm_width_mask;
}

inline auto get_n_bit_mask(std::size_t n_bits) -> uint64_t
{
    return (1_u64 << (n_bits)) - 1;
}

inline auto get_min_signed(std::size_t sew) -> int64_t
{
    return -1_i64 & (~get_n_bit_mask(sew - 1));
}

inline auto msb_is_set(uint64_t value, std::size_t sew) -> bool
{
    return value & (1_u64 << (sew - 1));
}

inline auto sign_extend(uint64_t value, std::size_t sew) -> uint64_t
{
    uint64_t sew_mask = (1_u64 << sew) - 1;
    uint64_t ext_mask = msb_is_set(value, sew) * (~sew_mask);
    return value | ext_mask;
}

inline auto mask_and_sign_extend_scalar(uint64_t value, std::size_t sew, bool signed_scalar) -> uint64_t
{
    if (sew == 64)
    {
        return value;
    }

    // Use least significant SEW bits
    uint64_t sew_mask = (1_u64 << sew) - 1;
    value &= sew_mask;

    bool sign_extend = signed_scalar && msb_is_set(value, sew);
    return value | (sign_extend * (~sew_mask));
};

#endif /* __RVVHL_BASE_H__ */
