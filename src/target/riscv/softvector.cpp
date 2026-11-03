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
/// \brief C++ Source for ETISS JIT libary or independent C application.
/// Implements a C interface when compiled to library beforehand
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

/*
 * TODOs:
 * - Iterators should handle correct element size / truncating, but inner operations do it as well (SEW mask)
 *      -> Check if this is necessary
 *      -> E.g. Saturating add / sub
 * - Split up cpp/header logically again (e.g. Integer, Float, ...)
 * - Look over type punning again (vector field pointer)
 *      -> GCC allows union type punning
 *      -> Field pointer will likely switch to uint8_t * instead of void *
 * - Adapt header: Save space with macros
 * - Replace Load/Store instructions
 */

#include <bit>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>

#include "softvector.h"
#include "arithmetic/softfloat-extension.hpp"
#include "operations.hpp"

#include "base/base.hpp"
#include "lsu/lsu.hpp"

#include "softfloat.h"

/* --- Private structs & enums --- */

enum class SignType
{
    Signed,
    Unsigned
};

enum class MaskType
{
    Masked,
    Unmasked
};

enum class ImmExtensionType
{
    SignExtend,
    ZeroExtend
};

enum class SxfType
{
    Sbf,
    Sif,
    Sof
};

/* --- Questionable Template Stuff --- */

template <typename BaseType>
struct TypeWidener;

template <>
struct TypeWidener<uint8_t>
{
    using wide_type = uint16_t;
};
template <>
struct TypeWidener<uint16_t>
{
    using wide_type = uint32_t;
};
template <>
struct TypeWidener<uint32_t>
{
    using wide_type = uint64_t;
};
template <>
struct TypeWidener<int8_t>
{
    using wide_type = int16_t;
};
template <>
struct TypeWidener<int16_t>
{
    using wide_type = int32_t;
};
template <>
struct TypeWidener<int32_t>
{
    using wide_type = int64_t;
};

template <uint8_t Sew, SignType Sign>
struct ElementTypeMap;

template <>
struct ElementTypeMap<8, SignType::Signed>
{
    using element_type = int8_t;
};
template <>
struct ElementTypeMap<16, SignType::Signed>
{
    using element_type = int16_t;
};
template <>
struct ElementTypeMap<32, SignType::Signed>
{
    using element_type = int32_t;
};
template <>
struct ElementTypeMap<64, SignType::Signed>
{
    using element_type = int64_t;
};
template <>
struct ElementTypeMap<8, SignType::Unsigned>
{
    using element_type = uint8_t;
};
template <>
struct ElementTypeMap<16, SignType::Unsigned>
{
    using element_type = uint16_t;
};
template <>
struct ElementTypeMap<32, SignType::Unsigned>
{
    using element_type = uint32_t;
};
template <>
struct ElementTypeMap<64, SignType::Unsigned>
{
    using element_type = uint64_t;
};

template <uint8_t Sew, SignType Sign, uint8_t Factor>
struct ExtTypes;

template <>
struct ExtTypes<16, SignType::Signed, 2>
{
    using source_type = int8_t;
    using dest_type = int16_t;
};

template <>
struct ExtTypes<16, SignType::Unsigned, 2>
{
    using source_type = uint8_t;
    using dest_type = uint16_t;
};

template <>
struct ExtTypes<32, SignType::Signed, 2>
{
    using source_type = int16_t;
    using dest_type = int32_t;
};

template <>
struct ExtTypes<32, SignType::Unsigned, 2>
{
    using source_type = uint16_t;
    using dest_type = uint32_t;
};

template <>
struct ExtTypes<32, SignType::Signed, 4>
{
    using source_type = int8_t;
    using dest_type = int32_t;
};

template <>
struct ExtTypes<32, SignType::Unsigned, 4>
{
    using source_type = uint8_t;
    using dest_type = uint32_t;
};

template <>
struct ExtTypes<64, SignType::Signed, 2>
{
    using source_type = int32_t;
    using dest_type = int64_t;
};

template <>
struct ExtTypes<64, SignType::Unsigned, 2>
{
    using source_type = uint32_t;
    using dest_type = uint64_t;
};

template <>
struct ExtTypes<64, SignType::Signed, 4>
{
    using source_type = int16_t;
    using dest_type = int64_t;
};

template <>
struct ExtTypes<64, SignType::Unsigned, 4>
{
    using source_type = uint16_t;
    using dest_type = uint64_t;
};

template <>
struct ExtTypes<64, SignType::Signed, 8>
{
    using source_type = int8_t;
    using dest_type = int64_t;
};

template <>
struct ExtTypes<64, SignType::Unsigned, 8>
{
    using source_type = uint8_t;
    using dest_type = uint64_t;
};

union PointerPunner
{
    void *const field;
    uint8_t *const u8;
    uint16_t *const u16;
    uint32_t *const u32;
    uint64_t *const u64;
    int8_t *const i8;
    int16_t *const i16;
    int32_t *const i32;
    int64_t *const i64;
    template <typename T>
    T *const get()
    {
        if constexpr (std::is_same_v<T, uint8_t>)
        {
            return u8;
        }
        else if constexpr (std::is_same_v<T, uint16_t>)
        {
            return u16;
        }
        else if constexpr (std::is_same_v<T, uint32_t>)
        {
            return u32;
        }
        else if constexpr (std::is_same_v<T, uint64_t>)
        {
            return u64;
        }
        else if constexpr (std::is_same_v<T, int8_t>)
        {
            return i8;
        }
        else if constexpr (std::is_same_v<T, int16_t>)
        {
            return i16;
        }
        else if constexpr (std::is_same_v<T, int32_t>)
        {
            return i32;
        }
        else if constexpr (std::is_same_v<T, int64_t>)
        {
            return i64;
        }
        else
        {
            static_assert(false, "Illegal Type");
        }
    }
};

/* --- Private globals --- */

constexpr auto sew_8 = 8;
constexpr auto sew_16 = 16;
constexpr auto sew_32 = 32;
constexpr auto sew_64 = 64;

auto g_fp_rounding_mode = FPRoundingMode::rnu;

/* --- Private function declarations --- */

// Helper declarations

template <typename VectorElementType, bool IsSigned>
auto dump_v_register(unsigned v_register, unsigned vlen, void *const vector_field) -> void;

inline constexpr auto decode_sew(uint32_t vtype) -> unsigned;

inline constexpr auto is_masked_instruction(bool const instruction_mask_bit) -> bool;

inline constexpr auto is_masked_element(bool const element_mask_bit) -> bool;

template <SignType Sign>
inline constexpr uint64_t get_scalar(void *const scalar_field, unsigned const sew, unsigned const xlen,
                                     unsigned const rs1);

inline constexpr uint64_t get_raw_scalar(void *const scalar_field, unsigned const xlen, unsigned const rs1);

inline constexpr uint64_t get_float_scalar(void *const float_scalar_field, unsigned const sew, unsigned const flen,
                                           unsigned const rs1);

// Dispatcher declarations

#define VV_DISPATCHER_DECL(iterator)                                                                                   \
    template <SignType Sign, typename OpType>                                                                          \
    inline constexpr void iterator##_dispatch(void *const vector_field, uint16_t const vtype,                          \
                                              uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1, \
                                              uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,           \
                                              uint32_t const vl, OpType const op);

#define VX_DISPATCHER_DECL(iterator)                                                                                  \
    template <SignType Sign, typename OpType>                                                                         \
    inline constexpr void iterator##_dispatch(                                                                        \
        void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const instruction_mask_bit, \
        uint8_t const vd, uint8_t const vs2, uint8_t const rs1, uint16_t const vstart, uint32_t const vlen,           \
        uint16_t xlen, uint32_t const vl, OpType const op);

VV_DISPATCHER_DECL(vv)
VV_DISPATCHER_DECL(widening_vv)
VV_DISPATCHER_DECL(widening_wv)
VV_DISPATCHER_DECL(narrowing_wv)
VV_DISPATCHER_DECL(reduce)
VV_DISPATCHER_DECL(widening_reduce)

VX_DISPATCHER_DECL(vx)
VX_DISPATCHER_DECL(widening_vx)
VX_DISPATCHER_DECL(widening_wx)
VX_DISPATCHER_DECL(narrowing_wx)

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr void vi_dispatch(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                                  uint8_t const vd, uint8_t const vs2, uint8_t immediate, uint16_t const vstart,
                                  uint32_t const vlen, uint32_t const vl, OpType const op);

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr void narrowing_wi_dispatch(void *const vector_field, uint16_t const vtype,
                                            uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                            uint8_t const immediate, uint16_t const vstart, uint32_t const vlen,
                                            uint32_t const vl, OpType const op);

template <unsigned Sew, unsigned Factor>
inline constexpr void dispatch_iterate_vext(void *const vector_field, bool const is_masked, uint8_t const vd,
                                            uint8_t const vs2, bool const is_signed, uint16_t const vstart,
                                            uint32_t const vlen, uint32_t const vl);

template <SignType Sign, typename OpType>
inline constexpr void dispatch_iterate_v_unary(void *const vector_field, uint16_t const vtype,
                                               uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                               uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                               OpType const op);

template <typename OpType, SignType Sign = SignType::Unsigned>
inline constexpr void vf_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                  uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                  uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint16_t flen,
                                  uint32_t const vl, OpType const op);

template <typename OpType, SignType Sign = SignType::Unsigned>
inline constexpr void dispatch_iterate_widening_vf(void *const vector_field, void *const scalar_field,
                                                   uint16_t const vtype, uint8_t const instruction_mask_bit,
                                                   uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                                                   uint16_t const vstart, uint32_t const vlen, uint16_t flen,
                                                   uint32_t const vl, OpType const op);

template <typename OpType, SignType Sign = SignType::Unsigned>
inline constexpr void dispatch_iterate_widening_wf(void *const vector_field, void *const scalar_field,
                                                   uint16_t const vtype, uint8_t const instruction_mask_bit,
                                                   uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                                                   uint16_t const vstart, uint32_t const vlen, uint16_t flen,
                                                   uint32_t const vl, OpType const op);

template <SignType Sign, typename OpType>
inline constexpr bool sat_vv_dispatch(void *const vector_field, uint16_t const vtype,
                                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                      uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                      OpType const op);

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr bool sat_vi_dispatch(void *const vector_field, uint16_t const vtype,
                                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                      uint8_t immediate, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                      OpType const op);

template <SignType Sign, typename OpType>
inline constexpr bool sat_vx_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                      uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint16_t xlen,
                                      uint32_t const vl, OpType const op);

template <SignType Sign, typename OpType>
inline constexpr bool narrowing_sat_wv_dispatch(void *const vector_field, uint16_t const vtype,
                                                uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                                uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                                                uint32_t const vl, OpType const op);

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr bool narrowing_sat_wi_dispatch(void *const vector_field, uint16_t const vtype,
                                                uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                                uint8_t immediate, uint16_t const vstart, uint32_t const vlen,
                                                uint32_t const vl, OpType const op);

template <SignType Sign, typename OpType>
inline constexpr bool narrowing_sat_wx_dispatch(void *const vector_field, void *const scalar_field,
                                                uint16_t const vtype, uint8_t const instruction_mask_bit,
                                                uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                                                uint16_t const vstart, uint32_t const vlen, uint16_t xlen,
                                                uint32_t const vl, OpType const op);

// Iterator declarations
#define VV_ITERATOR_DECL(name)                                                                                     \
    template <typename VectorElementType, MaskType Mask, typename OpType>                                          \
        requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>                              \
    inline constexpr void name##_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,       \
                                         unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base, \
                                         OpType const op);

#define VXI_ITERATOR_DECL(name)                                                                                  \
    template <typename VectorElementType, MaskType Mask, typename OpType>                                        \
        requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>                            \
    inline constexpr void name##_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,     \
                                         unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base, \
                                         OpType const op);

VV_ITERATOR_DECL(vv)
VV_ITERATOR_DECL(reduce)
VV_ITERATOR_DECL(widening_reduce)
VV_ITERATOR_DECL(widening_vv)
VV_ITERATOR_DECL(widening_wv)
VV_ITERATOR_DECL(narrowing_wv)

VXI_ITERATOR_DECL(vxi)
VXI_ITERATOR_DECL(widening_vx)
VXI_ITERATOR_DECL(widening_wx)
VXI_ITERATOR_DECL(narrowing_wxi)

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void unary_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                    unsigned const vd_base, unsigned const vs2_base, OpType const op);

template <typename SourceType, typename DestType, MaskType Masked>
    requires ValidVectorElementType<SourceType> and ValidVectorElementType<DestType>
inline constexpr void vext_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                   unsigned const vd_base, unsigned const vs2_base);

template <SxfType Sxf, MaskType Mask>
inline constexpr void sxf_iterate(void *const vector_field, unsigned const vd, unsigned const vs2,
                                  uint16_t const vstart, uint32_t const vlen, uint32_t const vl);

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool sat_vv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                     unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                     OpType const op);

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool narrowing_sat_wv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                               unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                               OpType const op);

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool sat_vxi_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                      unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                      OpType const op);

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool narrowing_sat_wxi_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                                unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                                OpType const op);

/* --- Public function definitions --- */

#define VRED_OP(name, inner_op, sign)                                                                                  \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        reduce_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl, inner_op);    \
        return 0;                                                                                                      \
    }

#define W_VRED_OP(name, inner_op, sign)                                                                                \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        widening_reduce_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl,      \
                                       inner_op);                                                                      \
        return 0;                                                                                                      \
    }

#define VFRED_OP(name, inner_op)                                                                                       \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,  \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        softfloat_exceptionFlags = 0;                                                                                  \
        softfloat_roundingMode = rounding_mode;                                                                        \
        reduce_dispatch<SignType::Unsigned>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl, \
                                            inner_op);                                                                 \
        return 0;                                                                                                      \
    }

#define W_VFRED_OP(name, inner_op)                                                                                     \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,  \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        softfloat_exceptionFlags = 0;                                                                                  \
        softfloat_roundingMode = rounding_mode;                                                                        \
        widening_reduce_dispatch<SignType::Unsigned>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart,  \
                                                     vlen, vl, inner_op);                                              \
        return 0;                                                                                                      \
    }

#define VV_OP(name, inner_op, sign)                                                                                    \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        vv_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl, inner_op);        \
        return 0;                                                                                                      \
    }

#define W_VV_OP(name, inner_op, sign)                                                                                  \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        widening_vv_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl,          \
                                   inner_op);                                                                          \
        return 0;                                                                                                      \
    }

#define W_WV_OP(name, inner_op, sign)                                                                                  \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        widening_wv_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl,          \
                                   inner_op);                                                                          \
        return 0;                                                                                                      \
    }

#define N_WV_OP(name, inner_op, sign)                                                                                  \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        narrowing_wv_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl,         \
                                    inner_op);                                                                         \
        return 0;                                                                                                      \
    }

#define VI_OP(name, inner_op, sign, imm_extension)                                                                     \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs2, uint8_t imm, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)        \
    {                                                                                                                  \
        vi_dispatch<sign, imm_extension>(vector_field, vtype, instruction_mask_bit, vd, vs2, imm, vstart, vlen, vl,    \
                                         inner_op);                                                                    \
        return 0;                                                                                                      \
    }

#define N_WI_OP(name, inner_op, sign, imm_extension)                                                                   \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs2, uint8_t const imm, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        narrowing_wi_dispatch<sign, imm_extension>(vector_field, vtype, instruction_mask_bit, vd, vs2, imm, vstart,    \
                                                   vlen, vl, inner_op);                                                \
        return 0;                                                                                                      \
    }

#define VX_OP(name, inner_op, sign)                                                                                  \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype,                           \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,         \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)                  \
    {                                                                                                                \
        vx_dispatch<sign>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, vstart, vlen, xlen, \
                          vl, inner_op);                                                                             \
        return 0;                                                                                                    \
    }

#define W_VX_OP(name, inner_op, sign)                                                                             \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype,                        \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,      \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)               \
    {                                                                                                             \
        widening_vx_dispatch<sign>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, vstart, \
                                   vlen, xlen, vl, inner_op);                                                     \
        return 0;                                                                                                 \
    }

#define W_WX_OP(name, inner_op, sign)                                                                             \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype,                        \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,      \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)               \
    {                                                                                                             \
        widening_wx_dispatch<sign>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, vstart, \
                                   vlen, xlen, vl, inner_op);                                                     \
        return 0;                                                                                                 \
    }

#define N_WX_OP(name, inner_op, sign)                                                                              \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype,                         \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,       \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)                \
    {                                                                                                              \
        narrowing_wx_dispatch<sign>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, vstart, \
                                    vlen, xlen, vl, inner_op);                                                     \
        return 0;                                                                                                  \
    }

#define F_VV_OP(name, inner_op)                                                                                        \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,  \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        softfloat_exceptionFlags = 0;                                                                                  \
        softfloat_roundingMode = rounding_mode;                                                                        \
        vv_dispatch<SignType::Unsigned>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl,     \
                                        inner_op);                                                                     \
        return 0;                                                                                                      \
    }

#define W_F_VV_OP(name, inner_op)                                                                                      \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,  \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        softfloat_exceptionFlags = 0;                                                                                  \
        softfloat_roundingMode = rounding_mode;                                                                        \
        widening_vv_dispatch<SignType::Unsigned>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart,      \
                                                 vlen, vl, inner_op);                                                  \
        return 0;                                                                                                      \
    }

#define W_F_WV_OP(name, inner_op)                                                                                      \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,  \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        softfloat_exceptionFlags = 0;                                                                                  \
        softfloat_roundingMode = rounding_mode;                                                                        \
        widening_wv_dispatch<SignType::Unsigned>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart,      \
                                                 vlen, vl, inner_op);                                                  \
        return 0;                                                                                                      \
    }

#define F_VF_OP(name, inner_op)                                                                                      \
    uint8_t name(void *const vector_field, void *const float_scalar_field, uint16_t const vtype,                     \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,         \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t flen, uint8_t rounding_mode) \
    {                                                                                                                \
        softfloat_exceptionFlags = 0;                                                                                \
        softfloat_roundingMode = rounding_mode;                                                                      \
        vf_dispatch(vector_field, float_scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, vstart, vlen, flen, \
                    vl, inner_op);                                                                                   \
        return 0;                                                                                                    \
    }

#define W_F_VF_OP(name, inner_op)                                                                                    \
    uint8_t name(void *const vector_field, void *const float_scalar_field, uint16_t const vtype,                     \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,         \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t flen, uint8_t rounding_mode) \
    {                                                                                                                \
        softfloat_exceptionFlags = 0;                                                                                \
        softfloat_roundingMode = rounding_mode;                                                                      \
        dispatch_iterate_widening_vf(vector_field, float_scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1,    \
                                     vstart, vlen, flen, vl, inner_op);                                              \
        return 0;                                                                                                    \
    }

#define W_F_WF_OP(name, inner_op)                                                                                    \
    uint8_t name(void *const vector_field, void *const float_scalar_field, uint16_t const vtype,                     \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,         \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t flen, uint8_t rounding_mode) \
    {                                                                                                                \
        softfloat_exceptionFlags = 0;                                                                                \
        softfloat_roundingMode = rounding_mode;                                                                      \
        dispatch_iterate_widening_wf(vector_field, float_scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1,    \
                                     vstart, vlen, flen, vl, inner_op);                                              \
        return 0;                                                                                                    \
    }

#define F_V_UNARY_OP(name, inner_op)                                                                                   \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,                     \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        softfloat_exceptionFlags = 0;                                                                                  \
        softfloat_roundingMode = rounding_mode;                                                                        \
        dispatch_iterate_v_unary<SignType::Unsigned>(vector_field, vtype, instruction_mask_bit, vd, vs2, vstart, vlen, \
                                                     vl, inner_op);                                                    \
        return 0;                                                                                                      \
    }

#define VV_OP_MASKED_ONLY(name, inner_op, sign)                                                       \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1, \
                 uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)    \
    {                                                                                                 \
        vv_dispatch<sign>(vector_field, vtype, 0, vd, vs1, vs2, vstart, vlen, vl, inner_op);          \
        return 0;                                                                                     \
    }

#define VX_OP_MASKED_ONLY(name, inner_op, sign)                                                                       \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,          \
                 uint8_t const vs2, uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, \
                 uint8_t const xlen)                                                                                  \
    {                                                                                                                 \
        vx_dispatch<sign>(vector_field, scalar_field, vtype, 0, vd, vs2, rs1, vstart, vlen, xlen, vl, inner_op);      \
        return 0;                                                                                                     \
    }

#define VI_OP_MASKED_ONLY(name, inner_op, sign, imm_extension)                                              \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs2,       \
                 uint8_t const imm, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)          \
    {                                                                                                       \
        vi_dispatch<sign, imm_extension>(vector_field, vtype, 0, vd, vs2, imm, vstart, vlen, vl, inner_op); \
        return 0;                                                                                           \
    }

// 11. Vector Integer Arithmetic Instructions
// 11.1. Vector Single-Width Integer Add and Subtrac
VV_OP(vadd_vv, add_int, SignType::Signed)
VI_OP(vadd_vi, add_int, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vadd_vx, add_int, SignType::Signed)
VV_OP(vsub_vv, sub_int, SignType::Signed)
VX_OP(vsub_vx, sub_int, SignType::Signed)
VX_OP(vrsub_vx, rsub_int, SignType::Signed)
VI_OP(vrsub_vi, rsub_int, SignType::Signed, ImmExtensionType::SignExtend)

// 11.2. Vector Widening Integer Add/Subtract
W_VV_OP(vwaddu_vv, add_int, SignType::Unsigned)
W_VX_OP(vwaddu_vx, add_int, SignType::Unsigned)
W_VV_OP(vwsubu_vv, sub_int, SignType::Unsigned)
W_VX_OP(vwsubu_vx, sub_int, SignType::Unsigned)
W_VV_OP(vwadd_vv, add_int, SignType::Signed)
W_VX_OP(vwadd_vx, add_int, SignType::Signed)
W_VV_OP(vwsub_vv, sub_int, SignType::Signed)
W_VX_OP(vwsub_vx, sub_int, SignType::Signed)
W_WV_OP(vwaddu_w_vv, add_int, SignType::Unsigned)
W_WX_OP(vwaddu_w_vx, add_int, SignType::Unsigned)
W_WV_OP(vwsubu_w_vv, sub_int, SignType::Unsigned)
W_WX_OP(vwsubu_w_vx, sub_int, SignType::Unsigned)
W_WV_OP(vwadd_w_vv, add_int, SignType::Signed)
W_WX_OP(vwadd_w_vx, add_int, SignType::Signed)
W_WV_OP(vwsub_w_vv, sub_int, SignType::Signed)
W_WX_OP(vwsub_w_vx, sub_int, SignType::Signed)

// 11.3. Vector Integer Extension
uint8_t vext_vf(void *const vector_field, uint16_t const vtype, uint8_t const vm, uint8_t const vd, uint8_t const vs2,
                uint8_t const extension_encoding, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    auto const sew = decode_sew(vtype);

    constexpr auto f8 = 0b1;
    constexpr auto f4 = 0b10;
    constexpr auto f2 = 0b11;

    switch (extension_encoding >> 1)
    {
    case f8:
        switch (sew)
        {
        case 64:
            dispatch_iterate_vext<64, 8>(vector_field, is_masked_instruction(vm), vd, vs2,
                                         static_cast<bool>(extension_encoding & 1), vstart, vlen, vl);
            break;
        default:
            break;
        }
        break;
    case f4:
        switch (sew)
        {
        case 32:
            dispatch_iterate_vext<32, 4>(vector_field, is_masked_instruction(vm), vd, vs2,
                                         static_cast<bool>(extension_encoding & 1), vstart, vlen, vl);
            break;
        case 64:
            dispatch_iterate_vext<64, 4>(vector_field, is_masked_instruction(vm), vd, vs2,
                                         static_cast<bool>(extension_encoding & 1), vstart, vlen, vl);
            break;
        default:
            break;
        }
        break;
    case f2:
        switch (sew)
        {
        case 16:
            dispatch_iterate_vext<16, 2>(vector_field, is_masked_instruction(vm), vd, vs2,
                                         static_cast<bool>(extension_encoding & 1), vstart, vlen, vl);
            break;
        case 32:
            dispatch_iterate_vext<32, 2>(vector_field, is_masked_instruction(vm), vd, vs2,
                                         static_cast<bool>(extension_encoding & 1), vstart, vlen, vl);
            break;
        case 64:
            dispatch_iterate_vext<64, 2>(vector_field, is_masked_instruction(vm), vd, vs2,
                                         static_cast<bool>(extension_encoding & 1), vstart, vlen, vl);
            break;
        default:
            break;
        }
        break;
    }

    return 0;
}

// 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions
// TODO: This could just take vm (change CoreDSL description), so the signature is the same as other VV instructions and
// we can use the same macro
VV_OP_MASKED_ONLY(vadc_vvm, adc_int, SignType::Signed)
VX_OP_MASKED_ONLY(vadc_vxm, adc_int, SignType::Signed)
VI_OP_MASKED_ONLY(vadc_vim, adc_int, SignType::Signed, ImmExtensionType::SignExtend)

VV_OP(vmadc_vv, madc, SignType::Signed)
VI_OP(vmadc_vi, madc, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vmadc_vx, madc, SignType::Signed)

VV_OP_MASKED_ONLY(vsbc_vvm, sbc_int, SignType::Signed)
VX_OP_MASKED_ONLY(vsbc_vxm, sbc_int, SignType::Signed)
VI_OP_MASKED_ONLY(vsbc_vim, sbc_int, SignType::Signed, ImmExtensionType::SignExtend)

VV_OP(vmsbc_vv, msbc, SignType::Signed)
VI_OP(vmsbc_vi, msbc, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vmsbc_vx, msbc, SignType::Signed)

// 11.5. Vector Bitwise Logical Instructions
VV_OP(vand_vv, and_int, SignType::Signed)
VI_OP(vand_vi, and_int, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vand_vx, and_int, SignType::Signed)

VV_OP(vor_vv, or_int, SignType::Signed)
VI_OP(vor_vi, or_int, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vor_vx, or_int, SignType::Signed)

VV_OP(vxor_vv, xor_int, SignType::Signed)
VI_OP(vxor_vi, xor_int, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vxor_vx, xor_int, SignType::Signed)

// 11.6. Vector Single-Width Shift Instructions
VV_OP(vsll_vv, sll_int, SignType::Unsigned);
VI_OP(vsll_vi, sll_int, SignType::Unsigned, ImmExtensionType::ZeroExtend);
VX_OP(vsll_vx, sll_int, SignType::Unsigned);

VV_OP(vsrl_vv, srl_int, SignType::Unsigned);
VI_OP(vsrl_vi, srl_int, SignType::Unsigned, ImmExtensionType::ZeroExtend);
VX_OP(vsrl_vx, srl_int, SignType::Unsigned);

VV_OP(vsra_vv, sra_int, SignType::Signed);
VI_OP(vsra_vi, sra_int, SignType::Signed, ImmExtensionType::ZeroExtend);
VX_OP(vsra_vx, sra_int, SignType::Signed);

// 11.7. Vector Narrowing Integer Right Shift Instructions
N_WV_OP(vnsrl_wv, srl_int, SignType::Unsigned)
N_WI_OP(vnsrl_wi, srl_int, SignType::Unsigned, ImmExtensionType::ZeroExtend)
N_WX_OP(vnsrl_wx, srl_int, SignType::Unsigned)

N_WV_OP(vnsra_wv, sra_int, SignType::Signed)
N_WI_OP(vnsra_wi, sra_int, SignType::Signed, ImmExtensionType::ZeroExtend)
N_WX_OP(vnsra_wx, sra_int, SignType::Signed)

// 11.8. Vector Integer Compare Instructions
VV_OP(vmseq_vv, eq_int, SignType::Signed)
VI_OP(vmseq_vi, eq_int, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vmseq_vx, eq_int, SignType::Signed)

VV_OP(vmsne_vv, ne_int, SignType::Signed)
VI_OP(vmsne_vi, ne_int, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vmsne_vx, ne_int, SignType::Signed)

VV_OP(vmsltu_vv, ltu_int, SignType::Unsigned)
VX_OP(vmsltu_vx, ltu_int, SignType::Unsigned)

VV_OP(vmslt_vv, lt_int, SignType::Signed)
VX_OP(vmslt_vx, lt_int, SignType::Signed)

VV_OP(vmsleu_vv, leu_int, SignType::Unsigned)
VI_OP(vmsleu_vi, leu_int, SignType::Unsigned, ImmExtensionType::SignExtend)
VX_OP(vmsleu_vx, leu_int, SignType::Unsigned)

VV_OP(vmsle_vv, le_int, SignType::Signed)
VI_OP(vmsle_vi, le_int, SignType::Signed, ImmExtensionType::SignExtend)
VX_OP(vmsle_vx, le_int, SignType::Signed)

VX_OP(vmsgtu_vx, gtu_int, SignType::Unsigned)
VI_OP(vmsgtu_vi, gtu_int, SignType::Unsigned, ImmExtensionType::SignExtend)

VX_OP(vmsgt_vx, gt_int, SignType::Signed)
VI_OP(vmsgt_vi, gt_int, SignType::Signed, ImmExtensionType::SignExtend)

// 11.9. Vector Integer Min/Max Instructions
VV_OP(vminu_vv, minu_int, SignType::Unsigned)
VX_OP(vminu_vx, minu_int, SignType::Unsigned)

VV_OP(vmin_vv, min_int, SignType::Signed)
VX_OP(vmin_vx, min_int, SignType::Signed)

VV_OP(vmaxu_vv, maxu_int, SignType::Unsigned)
VX_OP(vmaxu_vx, maxu_int, SignType::Unsigned)

VV_OP(vmax_vv, max_int, SignType::Signed)
VX_OP(vmax_vx, max_int, SignType::Signed)

// 11.10. Vector Single-Width Integer Multiply Instructions
VV_OP(vmul_vv, mul_int, SignType::Signed)
VX_OP(vmul_vx, mul_int, SignType::Signed)

VV_OP(vmulh_vv, mulh_int, SignType::Signed)
VX_OP(vmulh_vx, mulh_int, SignType::Signed)

VV_OP(vmulhu_vv, mulhu_int, SignType::Unsigned)
VX_OP(vmulhu_vx, mulhu_int, SignType::Unsigned)

// Use signed instruction, mask off unsigned operand later in mulhsu_int
VV_OP(vmulhsu_vv, mulhsu_int, SignType::Signed)
VX_OP(vmulhsu_vx, mulhsu_int, SignType::Signed)

// 11.11. Vector Integer Divide Instructions
VV_OP(vdivu_vv, divu_int, SignType::Unsigned)
VX_OP(vdivu_vx, divu_int, SignType::Unsigned)

VV_OP(vdiv_vv, div_int, SignType::Signed)
VX_OP(vdiv_vx, div_int, SignType::Signed)

VV_OP(vremu_vv, remu_int, SignType::Unsigned)
VX_OP(vremu_vx, remu_int, SignType::Unsigned)

VV_OP(vrem_vv, rem_int, SignType::Signed)
VX_OP(vrem_vx, rem_int, SignType::Signed)

// 11.12. Vector Widening Integer Multiply Instructions
W_VV_OP(vwmul_vv, mul_int, SignType::Signed)
W_VX_OP(vwmul_vx, mul_int, SignType::Signed)

W_VV_OP(vwmulu_vv, mulu_int, SignType::Unsigned)
W_VX_OP(vwmulu_vx, mulu_int, SignType::Unsigned)

W_VV_OP(vwmulsu_vv, mulsu_int, SignType::Signed)
W_VX_OP(vwmulsu_vx, mulsu_int, SignType::Signed)

// 11.13. Vector Single-Width Integer Multiply-Add Instructions
VV_OP(vmacc_vv, macc, SignType::Signed)
VX_OP(vmacc_vx, macc, SignType::Signed)

VV_OP(vnmsac_vv, nmsac, SignType::Signed)
VX_OP(vnmsac_vx, nmsac, SignType::Signed)

VV_OP(vmadd_vv, madd, SignType::Signed)
VX_OP(vmadd_vx, madd, SignType::Signed)

VV_OP(vnmsub_vv, nmsub, SignType::Signed)
VX_OP(vnmsub_vx, nmsub, SignType::Signed)

// 11.14. Vector Widening Integer Multiply-Add Instructions
W_VV_OP(vwmaccu_vv, macc, SignType::Unsigned)
W_VX_OP(vwmaccu_vx, macc, SignType::Unsigned)

W_VV_OP(vwmacc_vv, macc, SignType::Signed)
W_VX_OP(vwmacc_vx, macc, SignType::Signed)

W_VV_OP(vwmaccsu_vv, maccsu, SignType::Signed)
W_VX_OP(vwmaccsu_vx, maccsu, SignType::Signed)

W_VX_OP(vwmaccus_vx, maccus, SignType::Signed)

// 11.15. Vector Integer Merge Instructions
VV_OP_MASKED_ONLY(vmerge_vv, merge, SignType::Unsigned)
VX_OP_MASKED_ONLY(vmerge_vx, merge, SignType::Unsigned)
VI_OP_MASKED_ONLY(vmerge_vi, merge, SignType::Unsigned, ImmExtensionType::SignExtend)

// 11.16. Vector Integer Move Instructions
uint8_t vmv_vv(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1,
               uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    auto const sew_bytes = decode_sew(vtype) >> 3;

    // Always unmasked, so we can just memcopy all the data
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);

    // Add vstart in bytes to pointers, so we don't have to do it afterwards
    auto *const vd_ptr = vector_elements + (vd * (vlen >> 3)) + (vstart * sew_bytes);
    auto *const vs1_ptr = vector_elements + (vs1 * (vlen >> 3)) + (vstart * sew_bytes);

    // TODO: is memcpy always correct?
    std::memcpy(vd_ptr, vs1_ptr, (vl - vstart) * sew_bytes);
    return 0;
}

uint8_t vmv_vi(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const imm,
               uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto const scalar = sign_extend_immediate(imm);
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);
    auto *const vd_ptr = vector_elements + (vd * (vlen >> 3));
    for (size_t i = vstart; i < vl; ++i)
    {
        std::memcpy(vd_ptr + (i * sew_bytes), &scalar, sew_bytes);
    }
    return 0;
}

uint8_t vmv_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
               uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto const scalar = get_scalar<SignType::Signed>(scalar_field, sew, xlen, rs1);
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);
    auto *const vd_ptr = vector_elements + (vd * (vlen >> 3));
    for (size_t i = vstart; i < vl; ++i)
    {
        std::memcpy(vd_ptr + (i * sew_bytes), &scalar, sew_bytes);
    }
    return 0;
}

// 12. Vector Fixed-Point Arithmetic Instructions
#define SAT_FP_VV_OP(name, inner_op, sign)                                                                             \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)  \
    {                                                                                                                  \
        return sat_vv_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl,        \
                                     inner_op);                                                                        \
    }

#define SAT_FP_VI_OP(name, inner_op, sign, imm_extension)                                                              \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs2, uint8_t const immediate, uint16_t const vstart, uint32_t const vlen,               \
                 uint32_t const vl)                                                                                    \
    {                                                                                                                  \
        return sat_vi_dispatch<sign, imm_extension>(vector_field, vtype, instruction_mask_bit, vd, vs2, immediate,     \
                                                    vstart, vlen, vl, inner_op);                                       \
    }

#define SAT_FP_VX_OP(name, inner_op, sign)                                                                          \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype,                          \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,        \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)                 \
    {                                                                                                               \
        return sat_vx_dispatch<sign>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, vstart, \
                                     vlen, xlen, vl, inner_op);                                                     \
    }

#define ROUND_FP_VV_OP(name, inner_op, sign)                                                                           \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,  \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);                                               \
        vv_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl, inner_op);        \
        return 0;                                                                                                      \
    }

#define ROUND_FP_VI_OP(name, inner_op, sign, imm_extension)                                                            \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs2, uint8_t const immediate, uint16_t const vstart, uint32_t const vlen,               \
                 uint32_t const vl, uint8_t const rounding_mode)                                                       \
    {                                                                                                                  \
        g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);                                               \
        vi_dispatch<sign, imm_extension>(vector_field, vtype, instruction_mask_bit, vd, vs2, immediate, vstart, vlen,  \
                                         vl, inner_op);                                                                \
        return 0;                                                                                                      \
    }

#define ROUND_FP_VX_OP(name, inner_op, sign)                                                                         \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype,                           \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,         \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen,                  \
                 uint8_t const rounding_mode)                                                                        \
    {                                                                                                                \
        g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);                                             \
        vx_dispatch<sign>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, vstart, vlen, xlen, \
                          vl, inner_op);                                                                             \
        return 0;                                                                                                    \
    }

#define NARROWING_SAT_ROUND_FP_WV_OP(name, inner_op, sign)                                                             \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,  \
                 uint8_t const rounding_mode)                                                                          \
    {                                                                                                                  \
        g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);                                               \
        return narrowing_sat_wv_dispatch<sign>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen,  \
                                               vl, inner_op);                                                          \
    }

#define NARROWING_SAT_ROUND_FP_WI_OP(name, inner_op, sign, imm_extension)                                              \
    uint8_t name(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd, \
                 uint8_t const vs2, uint8_t const immediate, uint16_t const vstart, uint32_t const vlen,               \
                 uint32_t const vl, uint8_t const rounding_mode)                                                       \
    {                                                                                                                  \
        g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);                                               \
        return narrowing_sat_wi_dispatch<sign, imm_extension>(vector_field, vtype, instruction_mask_bit, vd, vs2,      \
                                                              immediate, vstart, vlen, vl, inner_op);                  \
    }

#define NARROWING_SAT_ROUND_FP_WX_OP(name, inner_op, sign)                                                            \
    uint8_t name(void *const vector_field, void *const scalar_field, uint16_t const vtype,                            \
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,          \
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen,                   \
                 uint8_t const rounding_mode)                                                                         \
    {                                                                                                                 \
        g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);                                              \
        return narrowing_sat_wx_dispatch<sign>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1, \
                                               vstart, vlen, xlen, vl, inner_op);                                     \
    }

// 12.1. Vector Single-Width Saturating Add and Subtract
SAT_FP_VV_OP(vsaddu_vv, saddu, SignType::Unsigned)
SAT_FP_VI_OP(vsaddu_vi, saddu, SignType::Unsigned, ImmExtensionType::SignExtend)
SAT_FP_VX_OP(vsaddu_vx, saddu, SignType::Unsigned)

SAT_FP_VV_OP(vsadd_vv, sadd, SignType::Signed)
SAT_FP_VI_OP(vsadd_vi, sadd, SignType::Signed, ImmExtensionType::SignExtend)
SAT_FP_VX_OP(vsadd_vx, sadd, SignType::Signed)

SAT_FP_VV_OP(vssubu_vv, ssubu, SignType::Unsigned)
SAT_FP_VX_OP(vssubu_vx, ssubu, SignType::Unsigned)

SAT_FP_VV_OP(vssub_vv, ssub, SignType::Signed)
SAT_FP_VX_OP(vssub_vx, ssub, SignType::Signed)

// 12.2. Vector Single-Width Averaging Add and Subtract
ROUND_FP_VV_OP(vaaddu_vv, aaddu, SignType::Unsigned)
ROUND_FP_VX_OP(vaaddu_vx, aaddu, SignType::Unsigned)

ROUND_FP_VV_OP(vaadd_vv, aadd, SignType::Signed)
ROUND_FP_VX_OP(vaadd_vx, aadd, SignType::Signed)

ROUND_FP_VV_OP(vasubu_vv, asubu, SignType::Unsigned)
ROUND_FP_VX_OP(vasubu_vx, asubu, SignType::Unsigned)

ROUND_FP_VV_OP(vasub_vv, asub, SignType::Signed)
ROUND_FP_VX_OP(vasub_vx, asub, SignType::Signed)

// 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation
uint8_t vsmul_vv(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd,
                 uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                 uint8_t const rounding_mode)
{
    g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);
    return sat_vv_dispatch<SignType::Signed>(vector_field, vtype, instruction_mask_bit, vd, vs1, vs2, vstart, vlen, vl,
                                             smul);
}

uint8_t vsmul_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                 uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                 uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen,
                 uint8_t const rounding_mode)
{
    g_fp_rounding_mode = static_cast<FPRoundingMode>(rounding_mode);
    return sat_vx_dispatch<SignType::Signed>(vector_field, scalar_field, vtype, instruction_mask_bit, vd, vs2, rs1,
                                             vstart, vlen, xlen, vl, smul);
}

// 12.4. Vector Single-Width Scaling Shift Instructions
ROUND_FP_VV_OP(vssrl_vv, ssrl, SignType::Unsigned)
ROUND_FP_VX_OP(vssrl_vx, ssrl, SignType::Unsigned)
ROUND_FP_VI_OP(vssrl_vi, ssrl, SignType::Unsigned, ImmExtensionType::ZeroExtend)

ROUND_FP_VV_OP(vssra_vv, ssra, SignType::Signed)
ROUND_FP_VX_OP(vssra_vx, ssra, SignType::Signed)
ROUND_FP_VI_OP(vssra_vi, ssra, SignType::Signed, ImmExtensionType::ZeroExtend)

// 12.5. Vector Narrowing Fixed-Point Clip Instructions
NARROWING_SAT_ROUND_FP_WV_OP(vnclipu_wv, clipu, SignType::Unsigned)
NARROWING_SAT_ROUND_FP_WX_OP(vnclipu_wx, clipu, SignType::Unsigned)
NARROWING_SAT_ROUND_FP_WI_OP(vnclipu_wi, clipu, SignType::Unsigned, ImmExtensionType::ZeroExtend)

NARROWING_SAT_ROUND_FP_WV_OP(vnclip_wv, clip, SignType::Signed)
NARROWING_SAT_ROUND_FP_WX_OP(vnclip_wx, clip, SignType::Signed)
NARROWING_SAT_ROUND_FP_WI_OP(vnclip_wi, clip, SignType::Signed, ImmExtensionType::ZeroExtend)

// 13. Vector Floating-Point Instructions
// 13.2. Vector Single-Width Floating-Point Add/Subtract Instructions
F_VV_OP(vfadd_vv, add_float)
F_VF_OP(vfadd_vf, add_float)

F_VV_OP(vfsub_vv, sub_float)
F_VF_OP(vfsub_vf, sub_float)

F_VF_OP(vfrsub_vf, rsub_float)

// 13.3. Vector Widening Floating-Point Add/Subtract Instructions
W_F_VV_OP(vfwadd_vv, wadd_float)
W_F_VF_OP(vfwadd_vf, wadd_float)

W_F_VV_OP(vfwsub_vv, wsub_float)
W_F_VF_OP(vfwsub_vf, wsub_float)

W_F_WV_OP(vfwadd_wv, wadd_w_float)
W_F_WF_OP(vfwadd_wf, wadd_w_float)

W_F_WV_OP(vfwsub_wv, wsub_w_float)
W_F_WF_OP(vfwsub_wf, wsub_w_float)

// 13.4. Vector Single-Width Floating-Point Multiply/Divide Instructions
F_VV_OP(vfmul_vv, mul_float)
F_VF_OP(vfmul_vf, mul_float)

F_VV_OP(vfdiv_vv, div_float)
F_VF_OP(vfdiv_vf, div_float)

F_VF_OP(vfrdiv_vf, rdiv_float)

// 13.5. Vector Widening Floating-Point Multiply
W_F_VV_OP(vfwmul_vv, wmul_float)
W_F_VF_OP(vfwmul_vf, wmul_float)

// 13.6. Vector Single-Width Floating-Point Fused Multiply-Add Instructions
F_VV_OP(vfmacc_vv, macc_float)
F_VF_OP(vfmacc_vf, macc_float)

F_VV_OP(vfnmacc_vv, nmacc_float)
F_VF_OP(vfnmacc_vf, nmacc_float)

F_VV_OP(vfmsac_vv, msac_float)
F_VF_OP(vfmsac_vf, msac_float)

F_VV_OP(vfnmsac_vv, nmsac_float)
F_VF_OP(vfnmsac_vf, nmsac_float)

F_VV_OP(vfmadd_vv, madd_float)
F_VF_OP(vfmadd_vf, madd_float)

F_VV_OP(vfnmadd_vv, nmadd_float)
F_VF_OP(vfnmadd_vf, nmadd_float)

F_VV_OP(vfmsub_vv, msub_float)
F_VF_OP(vfmsub_vf, msub_float)

F_VV_OP(vfnmsub_vv, nmsub_float)
F_VF_OP(vfnmsub_vf, nmsub_float)

// 13.7. Vector Widening Floating-Point Fused Multiply-Add Instructions
W_F_VV_OP(vfwmacc_vv, wmacc_float)
W_F_VF_OP(vfwmacc_vf, wmacc_float)

W_F_VV_OP(vfwnmacc_vv, wnmacc_float)
W_F_VF_OP(vfwnmacc_vf, wnmacc_float)

W_F_VV_OP(vfwmsac_vv, wmsac_float)
W_F_VF_OP(vfwmsac_vf, wmsac_float)

W_F_VV_OP(vfwnmsac_vv, wnmsac_float)
W_F_VF_OP(vfwnmsac_vf, wnmsac_float)

// 13.8. Vector Floating-Point Square-Root Instruction
F_V_UNARY_OP(vfsqrt_v, sqrt_float)

// 13.9. Vector Floating-Point Reciprocal Square-Root Estimate Instruction
F_V_UNARY_OP(vfrsqrt7_v, rsqrt7_float)

// 13.10. Vector Floating-Point Reciprocal Estimate Instruction
F_V_UNARY_OP(vfrec7_v, rec7_float)

// 13.11. Vector Floating-Point MIN/MAX Instructions
F_VV_OP(vfmin_vv, min_float)
F_VF_OP(vfmin_vf, min_float)

F_VV_OP(vfmax_vv, max_float)
F_VF_OP(vfmax_vf, max_float)

// 13.12. Vector Floating-Point Sign-Injection Instructions
F_VV_OP(vfsgnj_vv, sgnj_float)
F_VF_OP(vfsgnj_vf, sgnj_float)

F_VV_OP(vfsgnjn_vv, sgnjn_float)
F_VF_OP(vfsgnjn_vf, sgnjn_float)

F_VV_OP(vfsgnjx_vv, sgnjx_float)
F_VF_OP(vfsgnjx_vf, sgnjx_float)

// 13.13. Vector Floating-Point Compare Instructions
F_VV_OP(vmfeq_vv, eq_float)
F_VF_OP(vmfeq_vf, eq_float)

F_VV_OP(vmfne_vv, ne_float)
F_VF_OP(vmfne_vf, ne_float)

F_VV_OP(vmflt_vv, lt_float)
F_VF_OP(vmflt_vf, lt_float)

F_VV_OP(vmfle_vv, le_float)
F_VF_OP(vmfle_vf, le_float)

F_VF_OP(vmfgt_vf, gt_float)

F_VF_OP(vmfge_vf, ge_float)

// 13.14. Vector Floating-Point Classify Instruction
F_V_UNARY_OP(vfclass_v, classify_float)

// 13.15. Vector Floating-Point Merge Instruction
uint8_t vfmerge_vfm(void *const vector_field, void *const float_scalar_field, uint16_t const vtype, uint8_t const vd,
                    uint8_t const vs2, uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                    uint8_t const flen)
{
    vf_dispatch(vector_field, float_scalar_field, vtype, 0, vd, vs2, rs1, vstart, vlen, flen, vl, merge);
    return 0;
}

// 13.16. Vector Floating-Point Move Instruction
uint8_t vfmv_v_f(void *const vector_field, void *const float_scalar_field, uint16_t const vtype, uint8_t const vd,
                 uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto const scalar = get_float_scalar(float_scalar_field, sew, flen, rs1);
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);
    auto *const vd_ptr = vector_elements + (vd * (vlen >> 3));
    for (size_t i = vstart; i < vl; ++i)
    {
        std::memcpy(vd_ptr + (i * sew_bytes), &scalar, sew_bytes);
    }
    return 0;
}

// 13.17. Single-Width Floating-Point/Integer Type-Convert Instructions
F_V_UNARY_OP(vfcvt_xu_f_v, convert_xu_f)
F_V_UNARY_OP(vfcvt_x_f_v, convert_x_f)

F_V_UNARY_OP(vfcvt_rtz_xu_f_v, convert_rtz_xu_f)
F_V_UNARY_OP(vfcvt_rtz_x_f_v, convert_rtz_x_f)

F_V_UNARY_OP(vfcvt_f_xu_v, convert_f_xu)
F_V_UNARY_OP(vfcvt_f_x_v, convert_f_x)

// 13.18. Widening Floating-Point/Integer Type-Convert Instructions
F_V_UNARY_OP(vfwcvt_xu_f_v, convert_widening_xu_f)
F_V_UNARY_OP(vfwcvt_x_f_v, convert_widening_x_f)

F_V_UNARY_OP(vfwcvt_rtz_xu_f_v, convert_widening_rtz_xu_f)
F_V_UNARY_OP(vfwcvt_rtz_x_f_v, convert_widening_rtz_x_f)

F_V_UNARY_OP(vfwcvt_f_xu_v, convert_widening_f_xu)
F_V_UNARY_OP(vfwcvt_f_x_v, convert_widening_f_x)

F_V_UNARY_OP(vfwcvt_f_f_v, convert_widening_f_f)

// 13.19. Narrowing Floating-Point/Integer Type-Convert Instructions
F_V_UNARY_OP(vfncvt_xu_f_w, convert_narrowing_xu_f)
F_V_UNARY_OP(vfncvt_x_f_w, convert_narrowing_x_f)

F_V_UNARY_OP(vfncvt_rtz_xu_f_w, convert_narrowing_rtz_xu_f)
F_V_UNARY_OP(vfncvt_rtz_x_f_w, convert_narrowing_rtz_x_f)

F_V_UNARY_OP(vfncvt_f_xu_w, convert_narrowing_f_xu)
F_V_UNARY_OP(vfncvt_f_x_w, convert_narrowing_f_x)

F_V_UNARY_OP(vfncvt_f_f_w, convert_narrowing_f_f)

uint8_t vfncvt_rod_f_f_w(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                         uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                         uint32_t const vl, [[maybe_unused]] uint8_t const rounding_mode)
{
    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = softfloat_round_odd;
    dispatch_iterate_v_unary<SignType::Unsigned>(vector_field, vtype, instruction_mask_bit, vd, vs2, vstart, vlen, vl,
                                                 convert_narrowing_f_f);
    return 0;
}

// 14. Vector Reduction Operations
// 14.1. Vector Single-Width Integer Reduction Instructions
VRED_OP(vredsum_vs, add_int, SignType::Signed)
VRED_OP(vredmaxu_vs, maxu_int, SignType::Unsigned)
VRED_OP(vredmax_vs, max_int, SignType::Signed)
VRED_OP(vredminu_vs, minu_int, SignType::Unsigned)
VRED_OP(vredmin_vs, min_int, SignType::Signed)
VRED_OP(vredand_vs, and_int, SignType::Signed)
VRED_OP(vredor_vs, or_int, SignType::Signed)
VRED_OP(vredxor_vs, xor_int, SignType::Signed)

// 14.2. Vector Widening Integer Reduction Instructions
W_VRED_OP(vwredsumu_vs, add_int, SignType::Unsigned)
W_VRED_OP(vwredsum_vs, add_int, SignType::Signed)

// 14.3. Vector Single-Width Floating-Point Reduction Instructions
VFRED_OP(vfredosum_vs, add_float)
VFRED_OP(vfredusum_vs, add_float)
VFRED_OP(vfredmax_vs, max_float)
VFRED_OP(vfredmin_vs, min_float)

// 14.4. Vector Widening Floating-Point Reduction Instructions
W_VFRED_OP(vfwredosum_vs, add_float)
W_VFRED_OP(vfwredusum_vs, add_float)

// 15. Vector Mask Instructions
// 15.1. Vector Mask-Register Logical Instructions
VV_OP(vmand_mm, and_mask, SignType::Unsigned)
VV_OP(vmnand_mm, nand_mask, SignType::Unsigned)
VV_OP(vmandn_mm, andn_mask, SignType::Unsigned)
VV_OP(vmxor_mm, xor_mask, SignType::Unsigned)
VV_OP(vmor_mm, or_mask, SignType::Unsigned)
VV_OP(vmnor_mm, nor_mask, SignType::Unsigned)
VV_OP(vmorn_mm, orn_mask, SignType::Unsigned)
VV_OP(vmxnor_mm, xnor_mask, SignType::Unsigned)

// 15.2. Vector count population in mask vcpop.m
uint8_t vcpop_m(void *const vector_field, void *const scalar_field, [[maybe_unused]] uint16_t const vtype,
                uint8_t const instruction_mask_bit, uint8_t const rd, uint8_t const vs2,
                [[maybe_unused]] uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    // vstart must be 0
    uint64_t const n_mask_bytes = vl >> 3;
    auto const n_trailing_elements = vl & 0b111;
    auto const vs2_base = vs2 * (vlen >> 3);
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);

    uint64_t sum = 0;
    // Bytewise computation
    for (size_t i = 0; i < n_mask_bytes; ++i)
    {
        uint8_t mask = (-instruction_mask_bit) | vector_elements[i];
        sum += std::popcount(static_cast<uint8_t>(vector_elements[vs2_base + i] & mask));
    }

    // Trailing elements
    if (n_trailing_elements != 0)
    {
        uint8_t mask = ((-instruction_mask_bit) | vector_elements[n_mask_bytes]) & ((1 << n_trailing_elements) - 1);
        sum += std::popcount(static_cast<uint8_t>(vector_elements[vs2_base + n_mask_bytes] & mask));
    }

    switch (xlen)
    {
    case 32:
        static_cast<uint32_t *>(scalar_field)[rd] = sum;
        break;
    case 64:
        static_cast<uint64_t *>(scalar_field)[rd] = sum;
        break;
    default:
        // Illegal
        break;
    }

    return 0;
}

// 15.3. vfirst find-first-set mask bit
uint8_t vfirst_m(void *const vector_field, void *const scalar_field, [[maybe_unused]] uint16_t const vtype,
                 uint8_t const instruction_mask_bit, uint8_t const rd, uint8_t const vs2,
                 [[maybe_unused]] uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    // vstart must be 0
    uint64_t const n_mask_bytes = vl >> 3;
    auto const n_trailing_elements = vl & 0b111;
    auto const vs2_base = vs2 * (vlen >> 3);
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);

    size_t i = 0;
    uint64_t running_index = 0;
    auto found = false;

    // Bytewise computation
    for (; i < n_mask_bytes && !found; ++i)
    {
        uint8_t mask = (-instruction_mask_bit) | vector_elements[i];
        auto const idx = std::countr_zero(static_cast<uint8_t>(vector_elements[vs2_base + i] & mask));
        running_index += idx;
        found = idx < 8;
    }

    // Trailing elements
    if ((n_trailing_elements != 0) && !found)
    {
        uint8_t mask = ((-instruction_mask_bit) | vector_elements[n_mask_bytes]) & ((1 << n_trailing_elements) - 1);
        auto const idx = std::countr_zero(static_cast<uint8_t>(vector_elements[vs2_base + n_mask_bytes] & mask));
        running_index += idx;
        found = idx < 8;
    }

    auto const final_index = (!found * -1_i64) | running_index;

    switch (xlen)
    {
    case 32:
        static_cast<uint32_t *>(scalar_field)[rd] = final_index;
        break;
    case 64:
        static_cast<uint64_t *>(scalar_field)[rd] = final_index;
        break;
    default:
        // Illegal
        break;
    }

    return 0;
}

// 15.4. vmsbf.m set-before-first mask bit
uint8_t vmsbf_m(void *const vector_field, [[maybe_unused]] uint16_t const vtype, uint8_t const instruction_mask_bit,
                uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    if (is_masked_instruction(instruction_mask_bit))
    {
        sxf_iterate<SxfType::Sbf, MaskType::Masked>(vector_field, vd, vs2, vstart, vlen, vl);
    }
    else
    {
        sxf_iterate<SxfType::Sbf, MaskType::Unmasked>(vector_field, vd, vs2, vstart, vlen, vl);
    }
    return 0;
}

// 15.5. vmsif.m set-including-first mask bit
uint8_t vmsif_m(void *const vector_field, [[maybe_unused]] uint16_t const vtype, uint8_t const instruction_mask_bit,
                uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    if (is_masked_instruction(instruction_mask_bit))
    {
        sxf_iterate<SxfType::Sif, MaskType::Masked>(vector_field, vd, vs2, vstart, vlen, vl);
    }
    else
    {
        sxf_iterate<SxfType::Sif, MaskType::Unmasked>(vector_field, vd, vs2, vstart, vlen, vl);
    }
    return 0;
}

// 15.6. vmsof.m set-only-first mask bit
uint8_t vmsof_m(void *const vector_field, [[maybe_unused]] uint16_t const vtype, uint8_t const instruction_mask_bit,
                uint8_t const vd, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    if (is_masked_instruction(instruction_mask_bit))
    {
        sxf_iterate<SxfType::Sof, MaskType::Masked>(vector_field, vd, vs2, vstart, vlen, vl);
    }
    else
    {
        sxf_iterate<SxfType::Sof, MaskType::Unmasked>(vector_field, vd, vs2, vstart, vlen, vl);
    }
    return 0;
}

// 15.7. does not refer to an instruction

// 15.8. Vector Iota Instruction
uint8_t viota_m(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd,
                uint8_t const vs2, [[maybe_unused]] uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto const vd_base = vd * (vlen >> 3);
    auto const vs2_base = vs2 * (vlen >> 3);
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);

    uint64_t accumulator = 0;

    // vstart must be 0
    for (size_t i = 0; i < vl; ++i)
    {
        auto const element_mask_bit = (vector_elements[i >> 3] >> (i & 0b111)) & 1;
        if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
        {
            std::memcpy(vector_elements + vd_base + (i * sew_bytes), &accumulator, sew_bytes);
            auto const vs2_bit = (vector_elements[vs2_base + (i >> 3)] >> (i & 0b111)) & 1;
            accumulator += vs2_bit;
        }
    }

    return 0;
}

// 15.9. Vector Element Index Instruction
uint8_t vid_v(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit, uint8_t const vd,
              uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto const vd_base = vd * (vlen >> 3);
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        auto const element_mask_bit = (vector_elements[i >> 3] >> (i & 0b111)) & 1;
        if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
        {
            std::memcpy(vector_elements + vd_base + (i * sew_bytes), &i, sew_bytes);
        }
    }
    return 0;
}

// 16. Vector Permutation Instructions
// 16.1. Integer Scalar Move Instructions
uint8_t vmv_xs(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const rd,
               uint8_t const vs2, uint32_t const vlen, [[maybe_unused]] uint32_t const vl, uint8_t const xlen)
{
    // TODO: does not need vl
    auto const sew_bytes = decode_sew(vtype) >> 3;
    auto const rd_base = rd * (xlen >> 3);
    auto const vs2_base = vs2 * (vlen >> 3);
    std::memcpy(static_cast<uint8_t *>(scalar_field) + rd_base, static_cast<uint8_t *>(vector_field) + vs2_base,
                sew_bytes);
    return 0;
}

uint8_t vmv_sx(void *const vector_field, void *const scalar_field, uint16_t const vtype, uint8_t const vd,
               uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    if (vstart >= vl)
    {
        return 0;
    }
    auto const sew = decode_sew(vtype);
    auto const scalar = get_scalar<SignType::Signed>(scalar_field, sew, xlen, rs1);
    auto const sew_bytes = sew >> 3;
    auto const vd_base = vd * (vlen >> 3);
    std::memcpy(static_cast<uint8_t *>(vector_field) + vd_base, &scalar, sew_bytes);
    return 0;
}

// 16.2. Floating-Point Scalar Move Instructions
inline constexpr auto convert_float_from_vec(uint64_t const raw_value, size_t const sew, size_t const flen) -> uint64_t
{
    // Currently no complete checking for illegal values
    switch (sew)
    {
    case 16:
        return (flen == 32) ? f16_to_f32(f16(raw_value)).v : box_f16(f16(raw_value)).v;
        break;
    case 32:
        return (flen == 32) ? raw_value : box_f32(f32(raw_value)).v;
        break;
    case 64:
        return raw_value;
        break;
    default:
        // Illegal
        exit(EXIT_FAILURE);
        break;
    }

    return -1;
}

uint8_t vfmv_f_s(void *const vector_field, void *const float_scalar_field, uint16_t const vtype, uint8_t const rd,
                 uint8_t const vs2, [[maybe_unused]] uint16_t const vstart, uint32_t const vlen,
                 [[maybe_unused]] uint32_t const vl, uint8_t const flen)
{
    // TODO: does not need vstart or vl
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto const rd_base = rd * (flen >> 3);
    auto const vs2_base = vs2 * (vlen >> 3);
    uint64_t raw_value = 0;
    std::memcpy(&raw_value, static_cast<uint8_t *>(vector_field) + vs2_base, sew_bytes);
    auto const converted_value = convert_float_from_vec(raw_value, sew, flen);
    std::memcpy(static_cast<uint8_t *>(float_scalar_field) + rd_base, &converted_value, flen >> 3);
    return 0;
}

uint8_t vfmv_s_f(void *vector_field, void *float_scalar_field, uint16_t const vtype, uint8_t const vd,
                 uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen)
{
    if (vstart >= vl)
    {
        return 0;
    }

    auto const sew = decode_sew(vtype);
    auto const vd_base = vd * (vlen >> 3);
    auto const value = get_float_scalar(float_scalar_field, sew, flen, rs1);
    std::memcpy(static_cast<uint8_t *>(vector_field) + vd_base, &value, sew >> 3);
    return 0;
}

// 16.3. Vector Slide Instruction
// 16.3.1. Vector Slideup Instructions
template <typename VectorElementType>
    requires std::is_same_v<VectorElementType, uint8_t> or std::is_same_v<VectorElementType, uint16_t> or
             std::is_same_v<VectorElementType, uint32_t> or std::is_same_v<VectorElementType, uint64_t>
constexpr void slideup_masked_iterate(void *const vector_field, unsigned const vd_base, unsigned const vs2_base,
                                      unsigned const offset, unsigned const start, unsigned const vl)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *const>(vector_field);
    for (size_t i = start; i < vl; ++i)
    {
        auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
        if (is_masked_element(element_mask_bit))
        {
            continue;
        }

        // vs2 element calculation OK because offset is <= i
        vector_elements[vd_base + i] = vector_elements[vs2_base + i - offset];
    }
}

constexpr void slideup(void *const vector_field, unsigned const vd, unsigned const vs2, unsigned const vlen,
                       unsigned const sew, bool const instruction_mask_bit, unsigned const vstart,
                       unsigned const offset, unsigned const vl)
{
    auto const start = std::max(vstart, offset);

    if (is_masked_instruction(instruction_mask_bit))
    {
        auto const vd_base = vd * (vlen / sew);
        auto const vs2_base = vs2 * (vlen / sew);
        switch (sew)
        {
        case sew_8:
            slideup_masked_iterate<uint8_t>(vector_field, vd_base, vs2_base, offset, start, vl);
            break;
        case sew_16:
            slideup_masked_iterate<uint16_t>(vector_field, vd_base, vs2_base, offset, start, vl);
            break;
        case sew_32:
            slideup_masked_iterate<uint32_t>(vector_field, vd_base, vs2_base, offset, start, vl);
            break;
        case sew_64:
            slideup_masked_iterate<uint64_t>(vector_field, vd_base, vs2_base, offset, start, vl);
            break;
        default:
            // Illegal
            break;
        }

        return;
    }

    // If not masked, basically a continous copy
    // Memcpy OK because vd and vs2 register groups do not overlap
    auto const vd_base = vd * (vlen >> 3);
    auto const vs2_base = vs2 * (vlen >> 3);
    auto const sew_bytes = sew >> 3;
    auto const n_bytes = (vl - start) * sew_bytes;
    auto const offset_bytes = offset * sew_bytes;
    auto const start_bytes = start * sew_bytes;
    std::memcpy(static_cast<uint8_t *>(vector_field) + vd_base + start_bytes,
                static_cast<uint8_t *>(vector_field) + vs2_base + start_bytes - offset_bytes, n_bytes);
}

uint8_t vslideup_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                    uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                    uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    auto const sew = decode_sew(vtype);
    auto const offset = get_raw_scalar(scalar_field, xlen, rs1);
    if (offset >= vl)
    {
        return 0;
    }
    slideup(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, offset, vl);
    return 0;
}

uint8_t vslideup_vi(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                    uint8_t const vd, uint8_t const vs2, uint8_t const imm, uint16_t const vstart, uint32_t const vlen,
                    uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const offset = static_cast<uint64_t>(imm);
    if (offset >= vl)
    {
        return 0;
    }
    slideup(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, offset, vl);
    return 0;
}

// 16.3.2. Vector Slidedown Instructions
template <typename VectorElementType, MaskType Mask>
    requires std::is_same_v<VectorElementType, uint8_t> or std::is_same_v<VectorElementType, uint16_t> or
             std::is_same_v<VectorElementType, uint32_t> or std::is_same_v<VectorElementType, uint64_t>
constexpr void slidedown_masked_iterate(void *const vector_field, unsigned const vd_base, unsigned const vs2_base,
                                        unsigned const offset, unsigned const start, unsigned const vl,
                                        unsigned const vlmax)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *const>(vector_field);
    size_t i = start;
    for (; i < vl && (i + offset) < vlmax; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        // vs2 element calculation OK because offset is <= i
        vector_elements[vd_base + i] = vector_elements[vs2_base + i + offset];
    }

    // Source elements that go over VLMAX
    for (; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }
        vector_elements[vd_base + i] = 0;
    }
}

constexpr void slidedown(void *const vector_field, unsigned const vd, unsigned const vs2, unsigned const vlen,
                         unsigned const sew, bool const instruction_mask_bit, unsigned const vstart,
                         unsigned const offset, unsigned const vl, unsigned const vlmax)
{
    auto const vd_base = vd * (vlen / sew);
    auto const vs2_base = vs2 * (vlen / sew);

    if (is_masked_instruction(instruction_mask_bit))
    {
        switch (sew)
        {
        case sew_8:
            slidedown_masked_iterate<uint8_t, MaskType::Masked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                vlmax);
            break;
        case sew_16:
            slidedown_masked_iterate<uint16_t, MaskType::Masked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                 vlmax);
            break;
        case sew_32:
            slidedown_masked_iterate<uint32_t, MaskType::Masked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                 vlmax);
            break;
        case sew_64:
            slidedown_masked_iterate<uint64_t, MaskType::Masked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                 vlmax);
            break;
        default:
            // Illegal
            break;
        }
    }
    else
    {
        switch (sew)
        {
        case sew_8:
            slidedown_masked_iterate<uint8_t, MaskType::Unmasked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                  vlmax);
            break;
        case sew_16:
            slidedown_masked_iterate<uint16_t, MaskType::Unmasked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                   vlmax);
            break;
        case sew_32:
            slidedown_masked_iterate<uint32_t, MaskType::Unmasked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                   vlmax);
            break;
        case sew_64:
            slidedown_masked_iterate<uint64_t, MaskType::Unmasked>(vector_field, vd_base, vs2_base, offset, vstart, vl,
                                                                   vlmax);
            break;
        default:
            // Illegal
            break;
        }
    }
}

uint8_t vslidedown_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                      uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    if (vl == 0)
    {
        return 0;
    }
    VTYPE::VTYPE const vt(vtype);
    auto const sew = decode_sew(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);
    auto const offset = get_raw_scalar(scalar_field, xlen, rs1);

    slidedown(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, offset, vl, vlmax);
    return 0;
}

uint8_t vslidedown_vi(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                      uint8_t const vd, uint8_t const vs2, uint8_t const imm, uint16_t const vstart,
                      uint32_t const vlen, uint32_t const vl)
{
    if (vl == 0)
    {
        return 0;
    }
    VTYPE::VTYPE const vt(vtype);
    auto const sew = decode_sew(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);

    slidedown(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, imm, vl, vlmax);
    return 0;
}

// 16.3.3. Vector Slide1up
uint8_t vslide1up_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                     uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                     uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    if (vl == 0)
    {
        return 0;
    }
    auto const sew = decode_sew(vtype);

    if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(static_cast<uint8_t *>(vector_field)[0] & 1))
    {
        // If first element is active, copy scalar to it
        auto const vd_base = vd * (vlen >> 3);
        auto const scalar = get_scalar<SignType::Unsigned>(scalar_field, sew, xlen, rs1);
        std::memcpy(static_cast<uint8_t *>(vector_field) + vd_base, &scalar, sew >> 3);
    }

    // Regular slide up with offset 1
    slideup(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, 1, vl);
    return 0;
}

// 16.3.4. Vector Floating-Point Slide1up Instruction
uint8_t vfslide1up_vf(void *const vector_field, void *const float_scalar_field, uint16_t const vtype,
                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                      uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen)
{
    if (vl == 0)
    {
        return 0;
    }
    auto const sew = decode_sew(vtype);

    if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(static_cast<uint8_t *>(vector_field)[0] & 1))
    {
        // If first element is active, copy float scalar to it
        auto const vd_base = vd * (vlen >> 3);
        auto const scalar = get_float_scalar(float_scalar_field, sew, flen, rs1);
        std::memcpy(static_cast<uint8_t *>(vector_field) + vd_base, &scalar, sew >> 3);
    }

    // Regular slide up with offset 1
    slideup(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, 1, vl);
    return 0;
}

// 16.3.5. Vector Slide1down Instruction
uint8_t vslide1down_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                       uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                       uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    if (vl == 0)
    {
        return 0;
    }
    VTYPE::VTYPE const vt(vtype);
    auto const sew = decode_sew(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);

    if (!is_masked_instruction(instruction_mask_bit) ||
        !is_masked_element((static_cast<uint8_t *>(vector_field)[(vl - 1) >> 3] >> ((vl - 1) & 0b111)) & 1))
    {
        // If last element is active, copy scalar to it
        auto const vd_base = vd * (vlen >> 3);
        auto const sew_bytes = sew >> 3;
        auto const scalar = get_scalar<SignType::Unsigned>(scalar_field, sew, xlen, rs1);
        std::memcpy(static_cast<uint8_t *>(vector_field) + vd_base + ((vl - 1) * sew_bytes), &scalar, sew_bytes);
    }

    // Regular slide down with offset 1
    slidedown(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, 1, vl - 1, vlmax);
    return 0;
}

// 16.3.6. Vector Floating-Point Slide1down Instruction
uint8_t vfslide1down_vf(void *const vector_field, void *const float_scalar_field, uint16_t const vtype,
                        uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                        uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const flen)
{
    if (vl == 0)
    {
        return 0;
    }
    VTYPE::VTYPE const vt(vtype);
    auto const sew = decode_sew(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);

    if (!is_masked_instruction(instruction_mask_bit) ||
        !is_masked_element((static_cast<uint8_t *>(vector_field)[(vl - 1) >> 3] >> ((vl - 1) & 0b111)) & 1))
    {
        // If last element is active, copy float scalar to it
        auto const vd_base = vd * (vlen >> 3);
        auto const sew_bytes = sew >> 3;
        auto const scalar = get_float_scalar(float_scalar_field, sew, flen, rs1);
        std::memcpy(static_cast<uint8_t *>(vector_field) + vd_base + ((vl - 1) * sew_bytes), &scalar, sew_bytes);
    }

    // Regular slide down with offset 1
    slidedown(vector_field, vd, vs2, vlen, sew, instruction_mask_bit, vstart, 1, vl - 1, vlmax);
    return 0;
}

// 16.4. Vector Register Gather Instructions
uint8_t vrgather_vv(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                    uint8_t const vd, uint8_t const vs1, uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                    uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);
    auto const vs1_base = vs1 * (vlen >> 3);
    auto const vs2_base = vs2 * (vlen >> 3);
    auto const vd_base = vd * (vlen >> 3);
    VTYPE::VTYPE const vt(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);

    for (size_t i = vstart; i < vl; ++i)
    {
        auto const element_mask_bit = static_cast<bool>((vector_elements[i >> 3] >> (i & 0b111)) & 1);
        if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
        {
            uint64_t vs1_index = 0;
            std::memcpy(&vs1_index, vector_elements + vs1_base + (i * sew_bytes), sew_bytes);

            if (vs1_index < vlmax)
            {
                // If index stored in vs1[i] < vlmax: write vs2[vs1[i]] to vd[i]
                std::memcpy(vector_elements + vd_base + (i * sew_bytes),
                            vector_elements + vs2_base + (vs1_index * sew_bytes), sew_bytes);
            }
            else
            {
                // Otherwise write 0 to vd[i]
                std::memset(vector_elements + vd_base + (i * sew_bytes), 0, sew_bytes);
            }
        }
    }
    return 0;
}

uint8_t vrgatherei16_vv(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                        uint8_t const vd, uint8_t const vs1, uint8_t const vs2, uint16_t const vstart,
                        uint32_t const vlen, uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);
    auto const vs1_base = vs1 * (vlen >> 3);
    auto const vs2_base = vs2 * (vlen >> 3);
    auto const vd_base = vd * (vlen >> 3);
    VTYPE::VTYPE const vt(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);

    constexpr auto vs1_sew = 16;
    constexpr auto vs1_sew_bytes = vs1_sew >> 3;

    for (size_t i = vstart; i < vl; ++i)
    {
        auto const element_mask_bit = static_cast<bool>((vector_elements[i >> 3] >> (i & 0b111)) & 1);
        if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
        {
            uint16_t vs1_index = 0;
            std::memcpy(&vs1_index, vector_elements + vs1_base + (i * vs1_sew_bytes), vs1_sew_bytes);

            if (vs1_index < vlmax)
            {
                // If index stored in vs1[i] < vlmax: write vs2[vs1[i]] to vd[i]
                std::memcpy(vector_elements + vd_base + (i * sew_bytes),
                            vector_elements + vs2_base + (vs1_index * sew_bytes), sew_bytes);
            }
            else
            {
                // Otherwise write 0 to vd[i]
                std::memset(vector_elements + vd_base + (i * sew_bytes), 0, sew_bytes);
            }
        }
    }
    return 0;
}

uint8_t vrgather_vi(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                    uint8_t const vd, uint8_t const vs2, uint8_t const imm, uint16_t const vstart, uint32_t const vlen,
                    uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);

    auto const vs2_base = vs2 * (vlen >> 3);
    auto const vd_base = vd * (vlen >> 3);
    VTYPE::VTYPE const vt(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);

    // Check once in the beginning to not compare every iteration
    if (imm < vlmax)
    {
        for (size_t i = vstart; i < vl; ++i)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i >> 3] >> (i & 0b111)) & 1);
            if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
            {
                std::memcpy(vector_elements + vd_base + (i * sew_bytes), vector_elements + vs2_base + (imm * sew_bytes),
                            sew_bytes);
            }
        }
    }
    else
    {
        for (size_t i = vstart; i < vl; ++i)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i >> 3] >> (i & 0b111)) & 1);
            if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
            {
                std::memset(vector_elements + vd_base + (i * sew_bytes), 0, sew_bytes);
            }
        }
    }
    return 0;
}

uint8_t vrgather_vx(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                    uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                    uint16_t const vstart, uint32_t const vlen, uint32_t const vl, uint8_t const xlen)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);

    auto const vs2_base = vs2 * (vlen >> 3);
    auto const vd_base = vd * (vlen >> 3);
    VTYPE::VTYPE const vt(vtype);
    auto const vlmax = (vt._z_lmul * vlen) / (vt._n_lmul * sew);
    auto const scalar = get_raw_scalar(scalar_field, xlen, rs1);

    // Check once in the beginning to not compare every iteration
    if (scalar < vlmax)
    {
        for (size_t i = vstart; i < vl; ++i)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i >> 3] >> (i & 0b111)) & 1);
            if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
            {
                std::memcpy(vector_elements + vd_base + (i * sew_bytes),
                            vector_elements + vs2_base + (scalar * sew_bytes), sew_bytes);
            }
        }
    }
    else
    {
        for (size_t i = vstart; i < vl; ++i)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i >> 3] >> (i & 0b111)) & 1);
            if (!is_masked_instruction(instruction_mask_bit) || !is_masked_element(element_mask_bit))
            {
                std::memset(vector_elements + vd_base + (i * sew_bytes), 0, sew_bytes);
            }
        }
    }
    return 0;
}

// 16.5. Vector Compress Instruction
uint8_t vcompress_vm(void *const vector_field, uint16_t const vtype, uint8_t const vd, uint8_t const vs1,
                     uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    auto const sew = decode_sew(vtype);
    auto const sew_bytes = sew >> 3;
    auto *const vector_elements = static_cast<uint8_t *>(vector_field);
    auto const vs1_base = vs1 * (vlen >> 3);
    auto const vs2_base = vs2 * (vlen >> 3);
    auto const vd_base = vd * (vlen >> 3);

    size_t dest_i = 0;
    for (size_t i = vstart; i < vl; ++i)
    {
        auto const active_element = static_cast<bool>((vector_elements[vs1_base + (i >> 3)] >> (i & 0b111)) & 1);
        if (active_element)
        {
            std::memcpy(vector_elements + vd_base + (dest_i * sew_bytes), vector_elements + vs2_base + (i * sew_bytes),
                        sew_bytes);
            dest_i++;
        }
    }
    return 0;
}

// 16.6. Whole Vector Register Move */
uint8_t vmvr_v(void *const vector_field, [[maybe_unused]] uint16_t const vtype, uint8_t const vd, uint8_t const vs2,
               uint8_t const imm, uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    if (vl < vstart)
    {
        return 0;
    }
    auto *const vector_elements = PointerPunner{ .field = vector_field }.u8;
    auto const n_registers = imm + 1;
    auto const vlen_bytes = vlen >> 3;
    auto const vd_base = vd * vlen_bytes;
    auto const vs2_base = vs2 * vlen_bytes;
    std::memcpy(vector_elements + vd_base, vector_elements + vs2_base, n_registers * vlen_bytes);
    return 0;
}

// Zvvmm
struct MatrixVtype
{
    unsigned lmul = 0;
    unsigned sew = 0;
    unsigned lambda = 0;
    bool altfmt_A = false;
    bool altfmt_B = false;
    bool bs = false;
};

MatrixVtype decode_matrix_vtype(uint32_t vtype)
{
    return {
        .lmul = 1U << (vtype & 0b11),
        .sew = 8U << ((vtype >> 3) & 0b11),
        .lambda = 1U << (((vtype >> 28) & 0b111) - 1),
        .altfmt_A = static_cast<bool>((vtype >> 27) & 1),
        .altfmt_B = static_cast<bool>((vtype >> 26) & 1),
        .bs = static_cast<bool>((vtype >> 25) & 1),
    };
}

template <typename T>
void print_v_matrix(T *const vector_elements, unsigned const lambda, unsigned const vlen, unsigned v_register,
                    unsigned widening, unsigned lmul)
{
    auto const sew = sizeof(T) * 8;
    auto const elements_per_register = vlen / sew;
    auto const v_base = v_register * elements_per_register;

    std::printf("v%u: ", v_register);

    auto const cols = lambda * lmul * widening;
    auto const rows = (vlen / sew) / lambda;
    for (size_t row = 0; row < rows; ++row)
    {
        for (size_t col = 0; col < cols; ++col)
        {
            auto const v_offset = (col / lambda) * elements_per_register;
            auto const v_element = (row * lambda) + (col % lambda);
            std::printf("| %x ", vector_elements[v_base + v_offset + v_element]);
        }
        std::printf(" |\n\n");
    }
}

/* --- Private function definitions --- */

// Helper definitions

template <typename VectorElementType, bool IsSigned>
auto dump_v_register(unsigned v_register, unsigned vlen, void *const vector_field) -> void
{
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    auto const sew = sizeof(VectorElementType) * 8;
    auto const elements_per_register = vlen / sew;
    auto const v_base = v_register * elements_per_register;

    std::printf("v%u: ", v_register);
    if constexpr (IsSigned)
    {
        for (int i = 0; i < elements_per_register; ++i)
        {
            std::printf("%x | ", vector_elements[v_base + i]);
        }
        std::printf("\n");
    }
    else
    {
        for (int i = 0; i < elements_per_register; ++i)
        {
            std::printf("%x ", vector_elements[v_base + i]);
        }
        std::printf("\n");
    }
}

template <SignType Sign>
inline constexpr uint64_t get_scalar(void *const scalar_field, unsigned const sew, unsigned const xlen,
                                     unsigned const rs1)
{
    uint64_t scalar = 0;
    uint64_t const sew_mask = (1_u64 << sew) - 1;
    switch (xlen)
    {
    case 32:
        scalar = static_cast<uint64_t>((static_cast<uint32_t *>(scalar_field))[rs1]) & sew_mask;
        break;
    case 64:
        scalar = (static_cast<uint64_t *>(scalar_field))[rs1] & sew_mask;
        break;
    default:
        // Invalid XLEN!
        break;
    }

    if constexpr (Sign == SignType::Signed)
    {
        bool const msb_is_set = scalar & (1 << (sew - 1));
        scalar |= (msb_is_set * (~sew_mask));
    }

    return scalar;
}

inline constexpr uint64_t get_raw_scalar(void *const scalar_field, unsigned const xlen, unsigned const rs1)
{
    switch (xlen)
    {
    case 32:
        return static_cast<uint64_t>((static_cast<uint32_t *>(scalar_field))[rs1]);
        break;
    case 64:
        return (static_cast<uint64_t *>(scalar_field))[rs1];
        break;
    default:
        // Invalid XLEN!
        break;
    }

    return -1;
}

inline constexpr uint64_t get_float_scalar(void *const float_scalar_field, unsigned const sew, unsigned const flen,
                                           unsigned const rs1)
{
    uint64_t scalar = 0;
    // uint64_t const sew_mask = (1_u64 << sew) - 1;
    switch (flen)
    {
    case 32:
        scalar = static_cast<uint64_t>((static_cast<uint32_t *>(float_scalar_field))[rs1]);
        break;
    case 64:
        scalar = (static_cast<uint64_t *>(float_scalar_field))[rs1];
        break;
    default:
        // Invalid XLEN!
        break;
    }

    if (flen > sew)
    {
        switch (sew)
        {
        case 16:
            scalar = check_and_unbox_f16(f64(scalar)).v;
            break;
        case 32:
            scalar = check_and_unbox_f32(f64(scalar)).v;
            break;
        default:
            // Illegal
            break;
        }
    }
    return scalar;
}

inline constexpr auto decode_sew(uint32_t const vtype) -> unsigned
{
    constexpr auto sew_offset = 3;
    constexpr auto sew_bitmask = 0b11;

    auto const vsew = (vtype >> sew_offset) & sew_bitmask;
    return 8 << vsew;
}

inline constexpr auto is_masked_element(bool const element_mask_bit) -> bool
{
    return !element_mask_bit;
}

inline constexpr auto is_masked_instruction(bool const instruction_mask_bit) -> bool
{
    return !instruction_mask_bit;
}

// Iterator definitions

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void reduce_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                     unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                     OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    uint64_t reduction_accumulator = vector_elements[vs1_base];
    if (vl == 0)
    {
        return;
    }

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            reduction_accumulator = op(reduction_accumulator, vector_elements[vs2_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            reduction_accumulator = op(reduction_accumulator, vector_elements[vs2_base + i], static_cast<SewType>(sew));
        }
        else
        {
            static_assert(false, "Invalid operation for reduce");
        }
    }
    vector_elements[vd_base] = reduction_accumulator;
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void widening_reduce_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                              unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                              OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    using WideType = TypeWidener<VectorElementType>::wide_type;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    auto *const wide_elements = static_cast<WideType *>(vector_field);
    uint64_t reduction_accumulator = wide_elements[vs1_base];
    if (vl == 0)
    {
        return;
    }

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            reduction_accumulator = op(reduction_accumulator, vector_elements[vs2_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            reduction_accumulator = op(reduction_accumulator, vector_elements[vs2_base + i], static_cast<SewType>(sew));
        }
        else
        {
            static_assert(false, "Invalid operation for widening reduce");
        }
    }
    wide_elements[vd_base] = reduction_accumulator;
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void vv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                 unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                 OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {

        // Only mask elements if mask is not data
        if constexpr (Mask == MaskType::Masked && !MaskDataOp<OpType>)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            // Casting signed to larger unsigned will sign extend.
            // As vector elements can be interpreted as int or uint, this should already take care of signed/unsigned
            // instructions
            vector_elements[vd_base + i] = op(vector_elements[vs2_base + i], vector_elements[vs1_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            vector_elements[vd_base + i] =
                op(vector_elements[vs2_base + i], vector_elements[vs1_base + i], static_cast<SewType>(sew));
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpMaskData>)
        {
            if constexpr (Mask == MaskType::Unmasked)
            {
                // Can't static assert false this condition since this is actually instantiated
                // Also all masked instructions of this type are just reserved currently, so this case might
                // exist in the future anyway
                std::printf("Illegal instruction\n");
                exit(EXIT_FAILURE);
            }
            auto const mask_data_bit = static_cast<Bit>((vector_elements[i / sew] >> (i % sew)) & 1);
            vector_elements[vd_base + i] =
                op(vector_elements[vs2_base + i], vector_elements[vs1_base + i], mask_data_bit);
        }
        else if constexpr (std::is_same_v<OpType, BitResultOp>)
        {
            // Compute result
            auto const result = op(vector_elements[vs2_base + i], vector_elements[vs1_base + i]) << (i % sew);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= result;
        }
        else if constexpr (std::is_same_v<OpType, BitResultOpSewData>)
        {
            // Compute result
            auto const result =
                op(vector_elements[vs2_base + i], vector_elements[vs1_base + i], static_cast<SewType>(sew))
                << (i % sew);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= result;
        }
        else if constexpr (std::is_same_v<OpType, BitResultOpSewMaskData>)
        {
            // vmadc & vmsbc: If instruction is masked, use mask bit as carry/borrow, otherwise don't use mask data.
            auto const mask_data_bit =
                (Mask == MaskType::Masked) && static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);

            // Compute result
            auto const result = op(vector_elements[vs2_base + i], vector_elements[vs1_base + i],
                                   static_cast<SewType>(sew), mask_data_bit)
                                << (i % sew);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= result;
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOp>)
        {
            // Accumulator instructions have vs2 and vs1 elements switched compared to e.g. vadd.vv
            // I.e. here vs2 is rhs and vs1 lhs
            vector_elements[vd_base + i] =
                op(vector_elements[vs1_base + i], vector_elements[vs2_base + i], vector_elements[vd_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOpSewData>)
        {
            // Accumulator instructions have vs2 and vs1 elements switched compared to e.g. vadd.vv
            // I.e. here vs2 is rhs and vs1 lhs
            vector_elements[vd_base + i] = op(vector_elements[vs1_base + i], vector_elements[vs2_base + i],
                                              vector_elements[vd_base + i], static_cast<SewType>(sew));
        }
        else if constexpr (std::is_same_v<OpType, MaskOp>)
        {
            Bit const lhs = static_cast<Bit>((vector_elements[vs2_base + (i / sew)] >> (i % sew)) & 1);
            Bit const rhs = static_cast<Bit>((vector_elements[vs1_base + (i / sew)] >> (i % sew)) & 1);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= op(lhs, rhs) << (i % sew);
        }
        else if constexpr (std::is_same_v<OpType, AveragingOp>)
        {
            vector_elements[vd_base + i] =
                op(vector_elements[vs2_base + i], vector_elements[vs1_base + i], g_fp_rounding_mode);
        }
        else if constexpr (std::is_same_v<OpType, AveragingOpSewData>)
        {
            vector_elements[vd_base + i] = op(vector_elements[vs2_base + i], vector_elements[vs1_base + i],
                                              static_cast<SewType>(sew), g_fp_rounding_mode);
        }
        else
        {
            static_assert(false, "Invalid operation for vv");
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void widening_vv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                          unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                          OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            wide_elements[vd_base + i] = op(vector_elements[vs2_base + i], vector_elements[vs1_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            wide_elements[vd_base + i] =
                op(vector_elements[vs2_base + i], vector_elements[vs1_base + i], static_cast<SewType>(sew));
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOp>)
        {
            // Accumulator instructions have vs2 and vs1 elements switched compared to e.g. vadd.vv
            // I.e. here vs2 is rhs and vs1 lhs
            wide_elements[vd_base + i] =
                op(vector_elements[vs1_base + i], vector_elements[vs2_base + i], wide_elements[vd_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOpSewData>)
        {
            // Accumulator instructions have vs2 and vs1 elements switched compared to e.g. vadd.vv
            // I.e. here vs2 is rhs and vs1 lhs
            wide_elements[vd_base + i] = op(vector_elements[vs1_base + i], vector_elements[vs2_base + i],
                                            wide_elements[vd_base + i], static_cast<SewType>(sew));
        }
        else
        {
            static_assert(false, "Invalid operation for widening vv");
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void widening_wv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                          unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                          OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            wide_elements[vd_base + i] = op(wide_elements[vs2_base + i], vector_elements[vs1_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            wide_elements[vd_base + i] =
                op(wide_elements[vs2_base + i], vector_elements[vs1_base + i], static_cast<SewType>(sew));
        }
        else
        {
            static_assert(false, "Invalid operation for widening wx (or not implemented)");
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void narrowing_wv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                           unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                           OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }
        if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            // Narrowing Shifts need 2*SEW (to mask of log2(2*SEW) bits of the shift amount)
            vector_elements[vd_base + i] =
                op(wide_elements[vs2_base + i], vector_elements[vs1_base + i], static_cast<SewType>(sew * 2));
        }
        else
        {
            static_assert(false, "Invalid operation for narrowing wv (or not implemented)");
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void narrowing_wxi_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                            unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                            OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }
        if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            // Narrowing Shifts need 2*SEW (to mask of log2(2*SEW) bits of the shift amount)
            vector_elements[vd_base + i] = op(wide_elements[vs2_base + i], scalar, static_cast<SewType>(sew * 2));
        }
        else
        {
            static_assert(false, "Invalid operation for narrowing wxi (or not implemented)");
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void vxi_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                  unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                  OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked && !MaskDataOp<OpType>)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            vector_elements[vd_base + i] = op(vector_elements[vs2_base + i], scalar);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            vector_elements[vd_base + i] = op(vector_elements[vs2_base + i], scalar, static_cast<SewType>(sew));
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpMaskData>)
        {
            if constexpr (Mask == MaskType::Unmasked)
            {
                // Can't static assert false this condition since this is actually instantiated
                // Also all masked instructions of this type are just reserved currently, so this case might
                // exist in the future anyway
                std::printf("(Softvector) Illegal instruction\n");
                return;
            }
            auto const mask_data_bit = static_cast<Bit>((vector_elements[i / sew] >> (i % sew)) & 1);
            vector_elements[vd_base + i] = op(vector_elements[vs2_base + i], scalar, mask_data_bit);
        }
        else if constexpr (std::is_same_v<OpType, BitResultOp>)
        {
            // Compute result
            auto const result = op(vector_elements[vs2_base + i], scalar) << (i % sew);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= result;
        }
        else if constexpr (std::is_same_v<OpType, BitResultOpSewData>)
        {
            // Compute result
            auto const result = op(vector_elements[vs2_base + i], scalar, static_cast<SewType>(sew)) << (i % sew);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= result;
        }
        else if constexpr (std::is_same_v<OpType, BitResultOpMaskData>)
        {
            // vmadc & vmsbc: If instruction is masked, use mask bit as carry/borrow, otherwise don't use mask data.
            auto const mask_data_bit =
                (Mask == MaskType::Masked) && static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);

            // Compute result
            auto const result = op(vector_elements[vs2_base + i], scalar, mask_data_bit) << (i % sew);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= result;
        }
        else if constexpr (std::is_same_v<OpType, BitResultOpSewMaskData>)
        {
            // vmadc & vmsbc: If instruction is masked, use mask bit as carry/borrow, otherwise don't use mask data.
            auto const mask_data_bit =
                (Mask == MaskType::Masked) && static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);

            // Compute result
            auto const result = op(vector_elements[vs2_base + i], scalar, static_cast<SewType>(sew), mask_data_bit)
                                << (i % sew);
            // Clear bit
            vector_elements[vd_base + (i / sew)] &= ~(1_u64 << (i % sew));
            // Conditionally set bit
            vector_elements[vd_base + (i / sew)] |= result;
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOp>)
        {
            // Accumulator instructions have vs2 and vs1 elements switched compared to e.g. vadd.vv
            // I.e. here vs2 is rhs and the scalar is lhs
            vector_elements[vd_base + i] = op(scalar, vector_elements[vs2_base + i], vector_elements[vd_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOpSewData>)
        {
            // Accumulator instructions have vs2 and vs1 elements switched compared to e.g. vadd.vv
            // I.e. here vs2 is rhs and vs1 lhs
            vector_elements[vd_base + i] =
                op(scalar, vector_elements[vs2_base + i], vector_elements[vd_base + i], static_cast<SewType>(sew));
        }
        else if constexpr (std::is_same_v<OpType, AveragingOp>)
        {
            vector_elements[vd_base + i] = op(vector_elements[vs2_base + i], scalar, g_fp_rounding_mode);
        }
        else if constexpr (std::is_same_v<OpType, AveragingOpSewData>)
        {
            vector_elements[vd_base + i] =
                op(vector_elements[vs2_base + i], scalar, static_cast<SewType>(sew), g_fp_rounding_mode);
        }
        else
        {
            static_assert(false, "Invalid operation for vxi (or not implemented)");
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void widening_vx_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                          unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                          OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            wide_elements[vd_base + i] = op(vector_elements[vs2_base + i], scalar);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            wide_elements[vd_base + i] = op(vector_elements[vs2_base + i], scalar, static_cast<SewType>(sew));
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOp>)
        {
            wide_elements[vd_base + i] = op(scalar, vector_elements[vs2_base + i], wide_elements[vd_base + i]);
        }
        else if constexpr (std::is_same_v<OpType, AccumulatorOpSewData>)
        {
            wide_elements[vd_base + i] =
                op(scalar, vector_elements[vs2_base + i], wide_elements[vd_base + i], static_cast<SewType>(sew));
        }
        else
        {
            static_assert(false, "Invalid operation for widening vx");
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void widening_wx_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                          unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                          OpType const op)
{
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);
    constexpr auto wide_sew = sizeof(WideElementType) * 8;

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((wide_elements[i / (wide_sew)] >> (i % wide_sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, ValueResultOp>)
        {
            wide_elements[vd_base + i] = op(wide_elements[vs2_base + i], scalar);
        }
        else if constexpr (std::is_same_v<OpType, ValueResultOpSewData>)
        {
            constexpr auto sew = static_cast<SewType>(sizeof(VectorElementType) * 8);
            wide_elements[vd_base + i] = op(wide_elements[vs2_base + i], scalar, sew);
        }
        else
        {
            static_assert(false, "Invalid operation for widening wx");
        }
    }
}

template <typename SourceType, typename DestType, MaskType Masked>
    requires ValidVectorElementType<SourceType> and ValidVectorElementType<DestType>
inline constexpr void vext_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                   unsigned const vd_base, unsigned const vs2_base)
{
    auto *const source_elements = static_cast<SourceType *>(vector_field);
    auto *const dest_elements = static_cast<DestType *>(vector_field);
    constexpr auto sew = sizeof(DestType) << 3;
    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Masked == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((dest_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }
        dest_elements[vd_base + i] = static_cast<DestType>(source_elements[vs2_base + i]);
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr void unary_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                    unsigned const vd_base, unsigned const vs2_base, OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, UnaryOpSewData>)
        {
            vector_elements[vd_base + i] = op(vector_elements[vs2_base + i], static_cast<SewType>(sew));
        }
        else
        {
            static_assert(false, "Invalid operation for unary");
        }
    }
}

template <SxfType Sxf, MaskType Mask>
inline constexpr void sxf_iterate(void *const vector_field, unsigned const vd, unsigned const vs2,
                                  uint16_t const vstart, uint32_t const vlen, uint32_t const vl)
{
    auto vector_elements = static_cast<uint8_t *>(vector_field);
    constexpr auto element_width = 8;
    auto const vs2_base = vs2 * (vlen / element_width);
    auto const vd_base = vd * (vlen / element_width);

    // SOF: Initialize to 0, otherwise 1
    Bit write_bit = !(Sxf == SxfType::Sof);
    [[maybe_unused]] Bit first_found = false;

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const mask_element_bit = (vector_elements[i / element_width] >> (i % element_width)) & 1;
            if (is_masked_element(mask_element_bit))
            {
                continue;
            }
        }

        auto const read_bit =
            static_cast<Bit>((vector_elements[vs2_base + (i / element_width)] >> (i % element_width)) & 1);

        if constexpr (Sxf == SxfType::Sbf)
        {
            write_bit &= !read_bit;
        }
        else if constexpr (Sxf == SxfType::Sof)
        {
            write_bit = (read_bit && !first_found);
        }

        // Clear bit
        vector_elements[vd_base + (i / element_width)] &= ~(1_u64 << (i % element_width));
        // Conditionally set bit
        vector_elements[vd_base + (i / element_width)] |= write_bit << (i % element_width);

        if constexpr (Sxf == SxfType::Sif)
        {
            write_bit &= !read_bit;
        }
        else if constexpr (Sxf == SxfType::Sof)
        {
            first_found |= read_bit;
        }
    }
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool sat_vv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                     unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                     OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    auto sat = false;

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, SatResultOp>)
        {
            // Casting signed to larger unsigned will sign extend.
            // As vector elements can be interpreted as int or uint, this should already take care of signed/unsigned
            // instructions
            auto const sat_result =
                op(vector_elements[vs2_base + i], vector_elements[vs1_base + i], static_cast<SewType>(sew));

            vector_elements[vd_base + i] = sat_result.result;
            sat |= sat_result.sat;
        }
        else if constexpr (std::is_same_v<OpType, AveragingSatResultOp>)
        {
            auto const sat_result = op(vector_elements[vs2_base + i], vector_elements[vs1_base + i],
                                       static_cast<SewType>(sew), g_fp_rounding_mode);

            vector_elements[vd_base + i] = sat_result.result;
            sat |= sat_result.sat;
        }
        else
        {
            static_assert(false, "Invalid operation for sat vv (or not implemented)");
        }
    }
    return sat;
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool narrowing_sat_wv_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                               unsigned const vd_base, unsigned const vs1_base, unsigned const vs2_base,
                                               OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);
    auto sat = false;

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, AveragingSatResultOp>)
        {
            auto const sat_result = op(wide_elements[vs2_base + i], vector_elements[vs1_base + i],
                                       static_cast<SewType>(2 * sew), g_fp_rounding_mode);

            vector_elements[vd_base + i] = sat_result.result;
            sat |= sat_result.sat;
        }
        else
        {
            static_assert(false, "Invalid operation for narrowing sat vv (or not implemented)");
        }
    }
    return sat;
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool sat_vxi_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                      unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                      OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    auto sat = false;

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, SatResultOp>)
        {
            // Casting signed to larger unsigned will sign extend.
            // As vector elements can be interpreted as int or uint, this should already take care of signed/unsigned
            // instructions
            auto const sat_result = op(vector_elements[vs2_base + i], scalar, static_cast<SewType>(sew));

            vector_elements[vd_base + i] = sat_result.result;
            sat |= sat_result.sat;
        }
        else if constexpr (std::is_same_v<OpType, AveragingSatResultOp>)
        {
            auto const sat_result =
                op(vector_elements[vs2_base + i], scalar, static_cast<SewType>(sew), g_fp_rounding_mode);

            vector_elements[vd_base + i] = sat_result.result;
            sat |= sat_result.sat;
        }
        else
        {
            static_assert(false, "Invalid operation for sat vv (or not implemented)");
        }
    }
    return sat;
}

template <typename VectorElementType, MaskType Mask, typename OpType>
    requires ValidVectorElementType<VectorElementType> and ValidOperation<OpType>
inline constexpr bool narrowing_sat_wxi_iterate(void *const vector_field, uint16_t const vstart, uint32_t const vl,
                                                unsigned const vd_base, uint64_t const scalar, unsigned const vs2_base,
                                                OpType const op)
{
    constexpr auto sew = sizeof(VectorElementType) * 8;
    auto *const vector_elements = static_cast<VectorElementType *>(vector_field);
    using WideElementType = TypeWidener<VectorElementType>::wide_type;
    auto *const wide_elements = static_cast<WideElementType *>(vector_field);
    auto sat = false;

    for (size_t i = vstart; i < vl; ++i)
    {
        if constexpr (Mask == MaskType::Masked)
        {
            auto const element_mask_bit = static_cast<bool>((vector_elements[i / sew] >> (i % sew)) & 1);
            if (is_masked_element(element_mask_bit))
            {
                continue;
            }
        }

        if constexpr (std::is_same_v<OpType, AveragingSatResultOp>)
        {
            auto const sat_result =
                op(wide_elements[vs2_base + i], scalar, static_cast<SewType>(2 * sew), g_fp_rounding_mode);

            vector_elements[vd_base + i] = sat_result.result;
            sat |= sat_result.sat;
        }
        else
        {
            static_assert(false, "Invalid operation for narrowing sat vxi (or not implemented)");
        }
    }
    return sat;
}

// Dispatcher definitions

#define ITERATOR_SWITCH(iterator)                                                                                   \
    using elm_8_t = ElementTypeMap<8, Sign>::element_type;                                                          \
    using elm_16_t = ElementTypeMap<16, Sign>::element_type;                                                        \
    using elm_32_t = ElementTypeMap<32, Sign>::element_type;                                                        \
    using elm_64_t = ElementTypeMap<64, Sign>::element_type;                                                        \
                                                                                                                    \
    switch (sew)                                                                                                    \
    {                                                                                                               \
    case sew_8:                                                                                                     \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                                         \
        {                                                                                                           \
            iterator##_iterate<elm_8_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,    \
                                                          vs2_base, op);                                            \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            iterator##_iterate<elm_8_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,  \
                                                            vs2_base, op);                                          \
        }                                                                                                           \
        break;                                                                                                      \
    case sew_16:                                                                                                    \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                                         \
        {                                                                                                           \
            iterator##_iterate<elm_16_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,   \
                                                           vs2_base, op);                                           \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            iterator##_iterate<elm_16_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar, \
                                                             vs2_base, op);                                         \
        }                                                                                                           \
        break;                                                                                                      \
    case sew_32:                                                                                                    \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                                         \
        {                                                                                                           \
            iterator##_iterate<elm_32_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,   \
                                                           vs2_base, op);                                           \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            iterator##_iterate<elm_32_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar, \
                                                             vs2_base, op);                                         \
        }                                                                                                           \
        break;                                                                                                      \
    case sew_64:                                                                                                    \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                                         \
        {                                                                                                           \
            iterator##_iterate<elm_64_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,   \
                                                           vs2_base, op);                                           \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            iterator##_iterate<elm_64_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar, \
                                                             vs2_base, op);                                         \
        }                                                                                                           \
        break;                                                                                                      \
    default:                                                                                                        \
        break;                                                                                                      \
    }

#define WIDE_ITERATOR_SWITCH(iterator)                                                                              \
    using elm_8_t = ElementTypeMap<8, Sign>::element_type;                                                          \
    using elm_16_t = ElementTypeMap<16, Sign>::element_type;                                                        \
    using elm_32_t = ElementTypeMap<32, Sign>::element_type;                                                        \
    switch (sew)                                                                                                    \
    {                                                                                                               \
    case sew_8:                                                                                                     \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                                         \
        {                                                                                                           \
            iterator##_iterate<elm_8_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,    \
                                                          vs2_base, op);                                            \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            iterator##_iterate<elm_8_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,  \
                                                            vs2_base, op);                                          \
        }                                                                                                           \
        break;                                                                                                      \
    case sew_16:                                                                                                    \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                                         \
        {                                                                                                           \
            iterator##_iterate<elm_16_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,   \
                                                           vs2_base, op);                                           \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            iterator##_iterate<elm_16_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar, \
                                                             vs2_base, op);                                         \
        }                                                                                                           \
        break;                                                                                                      \
    case sew_32:                                                                                                    \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                                         \
        {                                                                                                           \
            iterator##_iterate<elm_32_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar,   \
                                                           vs2_base, op);                                           \
        }                                                                                                           \
        else                                                                                                        \
        {                                                                                                           \
            iterator##_iterate<elm_32_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs1_base_or_scalar, \
                                                             vs2_base, op);                                         \
        }                                                                                                           \
        break;                                                                                                      \
    default:                                                                                                        \
        break;                                                                                                      \
    }

#define SAT_ITERATOR_SWITCH(iterator)                                                                  \
    using elm_8_t = ElementTypeMap<8, Sign>::element_type;                                             \
    using elm_16_t = ElementTypeMap<16, Sign>::element_type;                                           \
    using elm_32_t = ElementTypeMap<32, Sign>::element_type;                                           \
    using elm_64_t = ElementTypeMap<64, Sign>::element_type;                                           \
                                                                                                       \
    switch (sew)                                                                                       \
    {                                                                                                  \
    case sew_8:                                                                                        \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                            \
        {                                                                                              \
            return iterator##_iterate<elm_8_t, MaskType::Masked>(vector_field, vstart, vl, vd_base,    \
                                                                 vs1_base_or_scalar, vs2_base, op);    \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            return iterator##_iterate<elm_8_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base,  \
                                                                   vs1_base_or_scalar, vs2_base, op);  \
        }                                                                                              \
        break;                                                                                         \
    case sew_16:                                                                                       \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                            \
        {                                                                                              \
            return iterator##_iterate<elm_16_t, MaskType::Masked>(vector_field, vstart, vl, vd_base,   \
                                                                  vs1_base_or_scalar, vs2_base, op);   \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            return iterator##_iterate<elm_16_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, \
                                                                    vs1_base_or_scalar, vs2_base, op); \
        }                                                                                              \
        break;                                                                                         \
    case sew_32:                                                                                       \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                            \
        {                                                                                              \
            return iterator##_iterate<elm_32_t, MaskType::Masked>(vector_field, vstart, vl, vd_base,   \
                                                                  vs1_base_or_scalar, vs2_base, op);   \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            return iterator##_iterate<elm_32_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, \
                                                                    vs1_base_or_scalar, vs2_base, op); \
        }                                                                                              \
        break;                                                                                         \
    case sew_64:                                                                                       \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                            \
        {                                                                                              \
            return iterator##_iterate<elm_64_t, MaskType::Masked>(vector_field, vstart, vl, vd_base,   \
                                                                  vs1_base_or_scalar, vs2_base, op);   \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            return iterator##_iterate<elm_64_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, \
                                                                    vs1_base_or_scalar, vs2_base, op); \
        }                                                                                              \
        break;                                                                                         \
    default:                                                                                           \
        break;                                                                                         \
    }

#define WIDE_SAT_ITERATOR_SWITCH(iterator)                                                             \
    using elm_8_t = ElementTypeMap<8, Sign>::element_type;                                             \
    using elm_16_t = ElementTypeMap<16, Sign>::element_type;                                           \
    using elm_32_t = ElementTypeMap<32, Sign>::element_type;                                           \
                                                                                                       \
    switch (sew)                                                                                       \
    {                                                                                                  \
    case sew_8:                                                                                        \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                            \
        {                                                                                              \
            return iterator##_iterate<elm_8_t, MaskType::Masked>(vector_field, vstart, vl, vd_base,    \
                                                                 vs1_base_or_scalar, vs2_base, op);    \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            return iterator##_iterate<elm_8_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base,  \
                                                                   vs1_base_or_scalar, vs2_base, op);  \
        }                                                                                              \
        break;                                                                                         \
    case sew_16:                                                                                       \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                            \
        {                                                                                              \
            return iterator##_iterate<elm_16_t, MaskType::Masked>(vector_field, vstart, vl, vd_base,   \
                                                                  vs1_base_or_scalar, vs2_base, op);   \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            return iterator##_iterate<elm_16_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, \
                                                                    vs1_base_or_scalar, vs2_base, op); \
        }                                                                                              \
        break;                                                                                         \
    case sew_32:                                                                                       \
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))                            \
        {                                                                                              \
            return iterator##_iterate<elm_32_t, MaskType::Masked>(vector_field, vstart, vl, vd_base,   \
                                                                  vs1_base_or_scalar, vs2_base, op);   \
        }                                                                                              \
        else                                                                                           \
        {                                                                                              \
            return iterator##_iterate<elm_32_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, \
                                                                    vs1_base_or_scalar, vs2_base, op); \
        }                                                                                              \
        break;                                                                                         \
    default:                                                                                           \
        break;                                                                                         \
    }

template <SignType Sign, typename OpType>
inline constexpr bool sat_vv_dispatch(void *const vector_field, uint16_t const vtype,
                                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                      uint8_t const vs2, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                      OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs1_base_or_scalar = vs1 * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    SAT_ITERATOR_SWITCH(sat_vv)
    return false;
}

template <SignType Sign, typename OpType>
inline constexpr bool narrowing_sat_wv_dispatch(void *const vector_field, uint16_t const vtype,
                                                uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                                uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                                                uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs1_base_or_scalar = vs1 * elements_per_register;
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    WIDE_SAT_ITERATOR_SWITCH(narrowing_sat_wv)
    return false;
}

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr bool sat_vi_dispatch(void *const vector_field, uint16_t const vtype,
                                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                      uint8_t immediate, uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                      OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    uint64_t vs1_base_or_scalar = immediate;
    if constexpr (ImmExtension == ImmExtensionType::SignExtend)
    {
        vs1_base_or_scalar = sign_extend_immediate(vs1_base_or_scalar);
    }

    SAT_ITERATOR_SWITCH(sat_vxi)
    return 0;
}

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr bool narrowing_sat_wi_dispatch(void *const vector_field, uint16_t const vtype,
                                                uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                                uint8_t immediate, uint16_t const vstart, uint32_t const vlen,
                                                uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    uint64_t vs1_base_or_scalar = immediate;
    if constexpr (ImmExtension == ImmExtensionType::SignExtend)
    {
        vs1_base_or_scalar = sign_extend_immediate(vs1_base_or_scalar);
    }

    WIDE_SAT_ITERATOR_SWITCH(narrowing_sat_wxi)
    return 0;
}

template <SignType Sign, typename OpType>
inline constexpr bool sat_vx_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                      uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                      uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint16_t xlen,
                                      uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    auto const vs1_base_or_scalar = get_scalar<Sign>(scalar_field, sew, xlen, rs1);

    SAT_ITERATOR_SWITCH(sat_vxi)
    return 0;
}

template <SignType Sign, typename OpType>
inline constexpr bool narrowing_sat_wx_dispatch(void *const vector_field, void *const scalar_field,
                                                uint16_t const vtype, uint8_t const instruction_mask_bit,
                                                uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                                                uint16_t const vstart, uint32_t const vlen, uint16_t xlen,
                                                uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    auto const vs1_base_or_scalar = get_scalar<Sign>(scalar_field, sew, xlen, rs1);

    WIDE_SAT_ITERATOR_SWITCH(narrowing_sat_wxi)
    return 0;
}

#define VV_DISPATCHER(iterator)                                                                                        \
    template <SignType Sign, typename OpType>                                                                          \
    inline constexpr void iterator##_dispatch(void *const vector_field, uint16_t const vtype,                          \
                                              uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1, \
                                              uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,           \
                                              uint32_t const vl, OpType const op)                                      \
    {                                                                                                                  \
        auto const sew = decode_sew(vtype);                                                                            \
        auto const elements_per_register = vlen / sew;                                                                 \
        auto const vd_base = vd * elements_per_register;                                                               \
        auto const vs1_base_or_scalar = vs1 * elements_per_register;                                                   \
        auto const vs2_base = vs2 * elements_per_register;                                                             \
                                                                                                                       \
        ITERATOR_SWITCH(iterator)                                                                                      \
    }

VV_DISPATCHER(vv)
VV_DISPATCHER(reduce)

template <SignType Sign, typename OpType>
inline constexpr void widening_vv_dispatch(void *const vector_field, uint16_t const vtype,
                                           uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                           uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                                           uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * (elements_per_register >> 1);
    auto const vs1_base_or_scalar = vs1 * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    WIDE_ITERATOR_SWITCH(widening_vv)
}

template <SignType Sign, typename OpType>
inline constexpr void widening_reduce_dispatch(void *const vector_field, uint16_t const vtype,
                                               uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                               uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                                               uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * (elements_per_register >> 1);
    auto const vs1_base_or_scalar = vs1 * (elements_per_register >> 1);
    auto const vs2_base = vs2 * elements_per_register;

    WIDE_ITERATOR_SWITCH(widening_reduce)
}

template <SignType Sign, typename OpType>
inline constexpr void widening_wv_dispatch(void *const vector_field, uint16_t const vtype,
                                           uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                           uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                                           uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * (elements_per_register >> 1);
    auto const vs1_base_or_scalar = vs1 * elements_per_register;
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    WIDE_ITERATOR_SWITCH(widening_wv)
}

template <SignType Sign, typename OpType>
inline constexpr void narrowing_wv_dispatch(void *const vector_field, uint16_t const vtype,
                                            uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs1,
                                            uint8_t const vs2, uint16_t const vstart, uint32_t const vlen,
                                            uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs1_base_or_scalar = vs1 * elements_per_register;
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    WIDE_ITERATOR_SWITCH(narrowing_wv)
}

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr void vi_dispatch(void *const vector_field, uint16_t const vtype, uint8_t const instruction_mask_bit,
                                  uint8_t const vd, uint8_t const vs2, uint8_t immediate, uint16_t const vstart,
                                  uint32_t const vlen, uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    uint64_t vs1_base_or_scalar = immediate;
    if constexpr (ImmExtension == ImmExtensionType::SignExtend)
    {
        vs1_base_or_scalar = sign_extend_immediate(vs1_base_or_scalar);
    }

    ITERATOR_SWITCH(vxi)
}

template <SignType Sign, ImmExtensionType ImmExtension, typename OpType>
inline constexpr void narrowing_wi_dispatch(void *const vector_field, uint16_t const vtype,
                                            uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                            uint8_t immediate, uint16_t const vstart, uint32_t const vlen,
                                            uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    uint64_t vs1_base_or_scalar = immediate;
    if constexpr (ImmExtension == ImmExtensionType::SignExtend)
    {
        vs1_base_or_scalar = sign_extend_immediate(vs1_base_or_scalar);
    }

    WIDE_ITERATOR_SWITCH(narrowing_wxi)
}

template <SignType Sign, typename OpType>
inline constexpr void vx_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                  uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                  uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint16_t xlen,
                                  uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    auto const vs1_base_or_scalar = get_scalar<Sign>(scalar_field, sew, xlen, rs1);

    ITERATOR_SWITCH(vxi)
}

template <typename OpType, SignType Sign>
inline constexpr void vf_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                  uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                  uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint16_t flen,
                                  uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    auto const vs1_base_or_scalar = get_float_scalar(scalar_field, sew, flen, rs1);

    ITERATOR_SWITCH(vxi)
}

template <typename OpType, SignType Sign>
inline constexpr void dispatch_iterate_widening_vf(void *const vector_field, void *const scalar_field,
                                                   uint16_t const vtype, uint8_t const instruction_mask_bit,
                                                   uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                                                   uint16_t const vstart, uint32_t const vlen, uint16_t flen,
                                                   uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    auto const vs1_base_or_scalar = get_float_scalar(scalar_field, sew, flen, rs1);

    WIDE_ITERATOR_SWITCH(widening_vx)
}

template <typename OpType, SignType Sign>
inline constexpr void dispatch_iterate_widening_wf(void *const vector_field, void *const scalar_field,
                                                   uint16_t const vtype, uint8_t const instruction_mask_bit,
                                                   uint8_t const vd, uint8_t const vs2, uint8_t const rs1,
                                                   uint16_t const vstart, uint32_t const vlen, uint16_t flen,
                                                   uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    auto const vs1_base_or_scalar = get_float_scalar(scalar_field, sew, flen, rs1);

    WIDE_ITERATOR_SWITCH(widening_wx)
}

template <SignType Sign, typename OpType>
inline constexpr void widening_vx_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                           uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                           uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint16_t xlen,
                                           uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * (elements_per_register >> 1);
    auto const vs2_base = vs2 * elements_per_register;

    auto const vs1_base_or_scalar = get_scalar<Sign>(scalar_field, sew, xlen, rs1);

    WIDE_ITERATOR_SWITCH(widening_vx)
}

template <SignType Sign, typename OpType>
inline constexpr void widening_wx_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                           uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                           uint8_t const rs1, uint16_t const vstart, uint32_t const vlen, uint16_t xlen,
                                           uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * (elements_per_register >> 1);
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    auto const vs1_base_or_scalar = get_scalar<Sign>(scalar_field, sew, xlen, rs1);

    WIDE_ITERATOR_SWITCH(widening_wx)
}

template <SignType Sign, typename OpType>
inline constexpr void narrowing_wx_dispatch(void *const vector_field, void *const scalar_field, uint16_t const vtype,
                                            uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                            uint8_t const rs1, uint16_t const vstart, uint32_t const vlen,
                                            uint16_t xlen, uint32_t const vl, OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * (elements_per_register >> 1);

    auto const vs1_base_or_scalar = get_scalar<Sign>(scalar_field, sew, xlen, rs1);

    WIDE_ITERATOR_SWITCH(narrowing_wxi)
}

template <unsigned Sew, unsigned Factor>
inline constexpr void dispatch_iterate_vext(void *const vector_field, bool const is_masked, uint8_t const vd,
                                            uint8_t const vs2, bool const is_signed, uint16_t const vstart,
                                            uint32_t const vlen, uint32_t const vl)
{
    auto const src_elements_per_register = (Factor * vlen) / Sew;
    auto const dest_elements_per_register = vlen / Sew;
    auto const vd_base = vd * dest_elements_per_register;
    auto const vs2_base = vs2 * src_elements_per_register;

    using SourceTypeSigned = ExtTypes<Sew, SignType::Signed, Factor>::source_type;
    using DestTypeSigned = ExtTypes<Sew, SignType::Signed, Factor>::dest_type;
    using SourceTypeUnsigned = ExtTypes<Sew, SignType::Unsigned, Factor>::source_type;
    using DestTypeUnsigned = ExtTypes<Sew, SignType::Unsigned, Factor>::dest_type;

    if (is_signed)
    {
        if (is_masked)
        {
            vext_iterate<SourceTypeSigned, DestTypeSigned, MaskType::Masked>(vector_field, vstart, vl, vd_base,
                                                                             vs2_base);
        }
        else
        {
            vext_iterate<SourceTypeSigned, DestTypeSigned, MaskType::Unmasked>(vector_field, vstart, vl, vd_base,
                                                                               vs2_base);
        }
    }
    else
    {
        if (is_masked)
        {
            vext_iterate<SourceTypeUnsigned, DestTypeUnsigned, MaskType::Masked>(vector_field, vstart, vl, vd_base,
                                                                                 vs2_base);
        }
        else
        {
            vext_iterate<SourceTypeUnsigned, DestTypeUnsigned, MaskType::Unmasked>(vector_field, vstart, vl, vd_base,
                                                                                   vs2_base);
        }
    }
}

template <SignType Sign, typename OpType>
inline constexpr void dispatch_iterate_v_unary(void *const vector_field, uint16_t const vtype,
                                               uint8_t const instruction_mask_bit, uint8_t const vd, uint8_t const vs2,
                                               uint16_t const vstart, uint32_t const vlen, uint32_t const vl,
                                               OpType const op)
{
    auto const sew = decode_sew(vtype);
    auto const elements_per_register = vlen / sew;
    auto const vd_base = vd * elements_per_register;
    auto const vs2_base = vs2 * elements_per_register;

    using elm_8_t = ElementTypeMap<8, Sign>::element_type;
    using elm_16_t = ElementTypeMap<16, Sign>::element_type;
    using elm_32_t = ElementTypeMap<32, Sign>::element_type;
    using elm_64_t = ElementTypeMap<64, Sign>::element_type;

    switch (sew)
    {
    case sew_8:
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))
        {
            unary_iterate<elm_8_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        else
        {
            unary_iterate<elm_8_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        break;
    case sew_16:
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))
        {
            unary_iterate<elm_16_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        else
        {
            unary_iterate<elm_16_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        break;
    case sew_32:
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))
        {
            unary_iterate<elm_32_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        else
        {
            unary_iterate<elm_32_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        break;
    case sew_64:
        if (is_masked_instruction(static_cast<bool>(instruction_mask_bit)))
        {
            unary_iterate<elm_64_t, MaskType::Masked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        else
        {
            unary_iterate<elm_64_t, MaskType::Unmasked>(vector_field, vstart, vl, vd_base, vs2_base, op);
        }
        break;
    default:
        break;
    }
}

int8_t vtype_decode(uint16_t const vtype, uint8_t *ta, uint8_t *ma, uint32_t *sew, uint8_t *z_lmul, uint8_t *n_lmul)
{
    return (VTYPE::decode(vtype, ta, ma, sew, z_lmul, n_lmul));
}

uint16_t vtype_encode(uint16_t sew, uint8_t z_lmul, uint8_t n_lmul, uint8_t ta, uint8_t ma)
{
    return VTYPE::encode(sew, z_lmul, n_lmul, ta, ma);
}

uint8_t vtype_extractSEW(uint16_t const vtype)
{
    return VTYPE::extractSEW(vtype);
}

uint8_t vtype_extractLMUL(uint16_t const vtype)
{
    return VTYPE::extractLMUL(vtype);
}

uint8_t vtype_extractTA(uint16_t const vtype)
{
    return VTYPE::extractTA(vtype);
}

uint8_t vtype_extractMA(uint16_t const vtype)
{
    return VTYPE::extractMA(vtype);
}

uint16_t vcfg_concatEEW(uint8_t const mew, uint8_t const width)
{
    return (VTYPE::concatEEW(mew, width));
}

uint8_t vload_encoded_unitstride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                                 uint16_t const eew, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                                 uint32_t const vl, uint64_t const mem_offset)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
        return 1;

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_readMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            buff[i] = memory[addr + i];
    };

    VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8, vd, mem_offset, vstart, pVm,
                   eew / 8);

    return (0);
}

uint8_t vload_encoded_stride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                             uint16_t const eew, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                             uint32_t const vl, uint64_t const mem_offset, int16_t const stride)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
    {
        return 1;
    }

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_readMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            buff[i] = memory[addr + i];
    };

    VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8, vd, mem_offset, vstart, pVm,
                   stride);

    return (0);
}

uint8_t vload_segment_unitstride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                                 uint16_t const eew, uint8_t pNF, uint8_t const vd, uint16_t const vstart,
                                 uint32_t const vlen, uint32_t const vl, uint64_t const mem_offset)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
        return 1;
    if ((vd + pNF * _z_emul / _n_emul) > 32)
        return 1;
    if (vstart >= vl)
        return (0);

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_readMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            buff[i] = memory[addr + i];
    };

    uint16_t _vstart = vstart;
    uint64_t _moffset = mem_offset;

    for (int i = 0; i < pNF; ++i)
    {
        VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8, vd + (i * _z_emul / _n_emul),
                       _moffset, _vstart, pVm, eew / 8);
        _moffset += (vl - _vstart) * eew / 8;
        _vstart = 0;
    }

    return (0);
}

uint8_t vload_segment_stride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                             uint16_t const eew, uint8_t pNF, uint8_t const vd, uint16_t const vstart,
                             uint32_t const vlen, uint32_t const vl, uint64_t const mem_offset, int16_t const stride)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
        return 1;
    if ((vd + pNF * _z_emul / _n_emul) > 32)
        return 1;
    if (vstart >= vl)
        return (0);

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_readMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            buff[i] = memory[addr + i];
    };

    uint16_t _vstart = vstart;
    uint64_t _moffset = mem_offset;

    for (int i = 0; i < pNF; ++i)
    {
        _moffset = mem_offset + i * eew / 8;
        VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8, vd + (i * _z_emul / _n_emul),
                       _moffset, _vstart, pVm, stride);
        _vstart = 0;
    }

    return (0);
}

uint8_t vstore_encoded_unitstride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                                  uint16_t const eew, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                                  uint32_t const vl, uint64_t const mem_offset)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
        return 1;

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_writeMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            memory[addr + i] = buff[i];
    };

    VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8, vd, mem_offset, vstart, pVm,
                    eew / 8);

    return (0);
}

uint8_t vstore_encoded_stride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                              uint16_t const eew, uint8_t const vd, uint16_t const vstart, uint32_t const vlen,
                              uint32_t const vl, uint64_t const mem_offset, int16_t const stride)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
        return 1;

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_writeMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            memory[addr + i] = buff[i];
    };
    VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8, vd, mem_offset, vstart, pVm,
                    stride);

    return (0);
}

uint8_t vstore_segment_unitstride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                                  uint16_t const eew, uint8_t pNF, uint8_t const vd, uint16_t const vstart,
                                  uint32_t const vlen, uint32_t const vl, uint64_t const mem_offset)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
        return 1;
    if ((vd + pNF * _z_emul / _n_emul) > 32)
        return 1;
    if (vstart >= vl)
        return (0);

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_writeMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            memory[addr + i] = buff[i];
    };

    uint16_t _vstart = vstart;
    uint64_t _moffset = mem_offset;

    for (int i = 0; i < pNF; ++i)
    {
        VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8,
                        vd + (i * _z_emul / _n_emul), _moffset, _vstart, pVm, eew / 8);
        _moffset += (vl - _vstart) * eew / 8;
        _vstart = 0;
    }

    return (0);
}

uint8_t vstore_segment_stride(void *const vector_field, uint8_t *const memory, uint16_t const vtype, uint8_t pVm,
                              uint16_t const eew, uint8_t pNF, uint8_t const vd, uint16_t const vstart,
                              uint32_t const vlen, uint32_t const vl, uint64_t const mem_offset, int16_t const stride)
{
    VTYPE::VTYPE _vt(vtype);
    uint64_t _z_emul = eew * _vt._z_lmul;
    uint64_t _n_emul = _vt._sew * _vt._n_lmul;

    if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
        return 1;
    if ((vd + pNF * _z_emul / _n_emul) > 32)
        return 1;
    if (vstart >= vl)
        return (0);

    uint8_t *VectorRegField;

    VectorRegField = static_cast<uint8_t *>(vector_field);

    std::function<void(std::size_t, uint8_t *, std::size_t)> f_writeMem =
        [memory](std::size_t addr, uint8_t *buff, std::size_t len)
    {
        for (std::size_t i = 0; i < len; ++i)
            memory[addr + i] = buff[i];
    };

    uint16_t _vstart = vstart;
    uint64_t _moffset = mem_offset;
    for (int i = 0; i < pNF; ++i)
    {
        _moffset = mem_offset + i * eew / 8;
        VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, eew / 8, vl, vlen / 8,
                        vd + (i * _z_emul / _n_emul), _moffset, _vstart, pVm, stride);
        _moffset += (vl - _vstart) * eew / 8;
        _vstart = 0;
    }

    return (0);
}
