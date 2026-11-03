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

#include <cstdint>
#include <cstdio>
#include <type_traits>
#include "softvector.h"
#include "softfloat_types.h"
#include "operations.hpp"

template <typename T>
concept ValidFloatType = std::is_same_v<T, float16_t> or std::is_same_v<T, float32_t> or std::is_same_v<T, float64_t>;

struct MatrixVtype
{
    unsigned lmul = 0;
    unsigned sew = 0;
    unsigned lambda = 0;
    bool altfmt_A = false;
    bool altfmt_B = false;
    bool bs = false;
};

template <typename T>
inline constexpr auto sign_zero_extend(T value, bool is_signed) -> uint64_t
{
    constexpr auto width = sizeof(T) * 8;
    static_assert(width <= 64);
    return static_cast<uint64_t>(((static_cast<int64_t>(value) << (64 - width)) >> (64 - width)) * is_signed) |
           (static_cast<uint64_t>(value) * !is_signed);
}

inline constexpr MatrixVtype decode_matrix_vtype(uint32_t vtype)
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
    requires ValidVectorElementType<T>
inline constexpr void mmacc(T *vector_elements, unsigned vd, unsigned vs1, unsigned vs2, unsigned vlen, unsigned lambda,
                            unsigned lmul, unsigned sew, unsigned widening, bool signed_A, bool signed_B)
{
    // Accumulator is always signed
    using ResultType = std::make_signed_t<T>;
    auto *output_elements = reinterpret_cast<ResultType *>(vector_elements);
    auto const elements_per_register = vlen / sew;
    // std::printf("SV VLEN %u, SEW %u\n", vlen, sew);

    // Accumulator C has a register group multiplier of MUL_C = (VLEN / SEW) / (Lambda^2)
    // std::printf("LAMBDA %u\n", lambda);
    // auto const mul_C = elements_per_register / (lambda * lambda);
    // MUL_C in {1, 2, 4, 8, 16}
    // assert(mul_C == 1 || mul_C == 2 || mul_C == 4 || mul_C == 8 || mul_C == 16);
    // The register group start is MUL_C aligned (e.g. MUL_C = 16 -> vd = [0, 16])
    // assert((vd % mul_C) == 0);

    // Multiplication dimension for inputs, i.e. a result element is the sum of K_eff multiplications
    auto const K_eff = lambda * widening * lmul;

    // vs1 marks the start of A
    auto *const A_elements = vector_elements + (vs1 * elements_per_register * widening);
    // vs2 marks the start of B
    auto *const B_elements = vector_elements + (vs2 * elements_per_register * widening);
    // vd marks the start of C
    auto *const C_elements = output_elements + (vd * elements_per_register);

    // Rows & columns of C
    // Dimensions of C, M = N,
    // = ((LMUL * VLEN) / (SEW / W)) / K_eff                | Move W to numerator
    // = ((LMUL * VLEN * W) / SEW) / K_eff                  | Replace K_eff with definition
    // = ((LMUL * VLEN * W) / SEW) / (Lambda * W * LMUL)    | Cross out LMUL & W
    // = (VLEN / SEW) / Lambda
    auto const dim_C = elements_per_register / lambda;
    // std::printf("E per R %u, dim C %u, lambda %u\n", elements_per_register, dim_C, lambda);

    for (size_t row_C = 0; row_C < dim_C; ++row_C)
    {
        for (size_t col_C = 0; col_C < dim_C; ++col_C)
        {
            int64_t accumulator = 0;
            // std::printf("C[%lu][%lu] =", row_C, col_C);
            for (size_t i_input = 0; i_input < K_eff; ++i_input)
            {
                auto const vs_offset = (i_input / (lambda * widening)) * (elements_per_register * widening);
                auto const vs_A_element = (row_C * lambda * widening) + (i_input % (lambda * widening));
                auto const vs_B_element = (col_C * lambda * widening) + (i_input % (lambda * widening));
                // auto const a = A_elements[vs_offset + vs_A_element];
                // auto const b = B_elements[vs_offset + vs_B_element];
                // std::printf("+ (v%lu[%lu] * v%lu[%lu]) ", vs1 + vs_offset, vs_A_element, vs2 + vs_offset,
                // vs_B_element);

                // std::printf("+ ([%u @ v%lu[%lu]] * [%u @ v%lu[%lu]]) ", a,
                //             vs1 + (vs_offset / ((elements_per_register * widening))), vs_A_element, b,
                //             vs2 + (vs_offset / ((elements_per_register * widening))), vs_B_element);

                // std::printf("+ (%u * %u) ", a, b);
                accumulator += sign_zero_extend(A_elements[vs_offset + vs_A_element], signed_A) *
                               sign_zero_extend(B_elements[vs_offset + vs_B_element], signed_B);
            }

            auto const vd_offset = (col_C / lambda) * elements_per_register;
            auto const vd_element = (row_C * lambda) + (col_C % lambda);
            // std::printf("= %lu @ v%lu[%lu] \n", accumulator, vd + (vd_offset / elements_per_register), vd_element);
            // std::printf("%lu | ", accumulator);
            C_elements[vd_offset + vd_element] += accumulator;
        }
        // std::printf("\n\n");
    }
}

template <typename T_I, typename T_O>
    requires ValidVectorElementType<T_I>
inline constexpr void wmmacc(T_I *input_elements, T_O *output_elements, unsigned vd, unsigned vs1, unsigned vs2,
                             unsigned vlen, unsigned lambda, unsigned lmul, unsigned sew, bool signed_A, bool signed_B)
{
    constexpr auto widening = sizeof(T_O) / sizeof(T_I);
    static_assert(sizeof(T_O) >= sizeof(T_I), "Illegal narrowing");
    auto const elements_per_register = vlen / sew;

    // How many elements does a register contribute to a row
    auto const row_elements_per_register = lambda * widening;

    // Multiplication dimension for inputs, i.e. a result element is the sum of K_eff multiplications
    auto const K_eff = lmul * row_elements_per_register;

    // vs1 marks the start of A
    auto *const A_elements = input_elements + (vs1 * elements_per_register * widening);
    // vs2 marks the start of B
    auto *const B_elements = input_elements + (vs2 * elements_per_register * widening);
    // vd marks the start of C
    auto *const C_elements = output_elements + (vd * elements_per_register);

    // Rows & columns of C
    auto const dim_C = elements_per_register / lambda;

    for (size_t row_C = 0; row_C < dim_C; ++row_C)
    {
        for (size_t col_C = 0; col_C < dim_C; ++col_C)
        {
            // std::printf("C[%lu][%lu] =", row_C, col_C);
            int64_t accumulator = 0;
            for (size_t i_input = 0; i_input < K_eff; ++i_input)
            {
                auto const vs_offset = (i_input / row_elements_per_register) * (elements_per_register * widening);
                auto const vs_A_element = (row_C * row_elements_per_register) + (i_input % row_elements_per_register);
                auto const vs_B_element = (col_C * row_elements_per_register) + (i_input % row_elements_per_register);
                // auto const a = A_elements[vs_offset + vs_A_element];
                // auto const b = B_elements[vs_offset + vs_B_element];
                // std::printf("+ ([%u @ v%lu[%lu]] * [%u @ v%lu[%lu]]) ", a,
                //             vs1 + (vs_offset / ((elements_per_register * widening))), vs_A_element, b,
                //             vs2 + (vs_offset / ((elements_per_register * widening))), vs_B_element);
                accumulator += sign_zero_extend(A_elements[vs_offset + vs_A_element], signed_A) *
                               sign_zero_extend(B_elements[vs_offset + vs_B_element], signed_B);
            }

            auto const vd_offset = (col_C / lambda) * elements_per_register;
            auto const vd_element = (row_C * lambda) + (col_C % lambda);
            C_elements[vd_offset + vd_element] += accumulator;
            // std::printf("= %lu @ v%lu[%lu] \n", accumulator, vd + (vd_offset / elements_per_register), vd_element);
        }
        // std::printf("\n");
    }
}

template <typename T>
    requires ValidFloatType<T>
inline constexpr void mmacc_float(T *const vector_elements, unsigned vd, unsigned vs1, unsigned vs2, unsigned vlen,
                                  unsigned lambda, unsigned lmul, unsigned sew, unsigned widening)
{
    auto const elements_per_register = vlen / sew;
    auto const K_eff = lambda * widening * lmul;
    auto const dim_C = elements_per_register / lambda;

    for (size_t row_C = 0; row_C < dim_C; ++row_C)
    {
        for (size_t col_C = 0; col_C < dim_C; ++col_C)
        {
            auto const vd_offset = (col_C / lambda) * elements_per_register;
            auto const vd_element = (row_C * lambda) + (col_C % lambda);
            auto const vd_index = vd * elements_per_register + vd_offset + vd_element;

            T accumulator = vector_elements[vd_index];

            for (size_t i_input = 0; i_input < K_eff; ++i_input)
            {
                auto const vs_offset = (i_input / (lambda * widening)) * (elements_per_register * widening);
                auto const vs_A_element = (row_C * lambda * widening) + (i_input % (lambda * widening));
                auto const vs_B_element = (col_C * lambda * widening) + (i_input % (lambda * widening));

                auto const vs1_index = vs1 * elements_per_register * widening + vs_offset + vs_A_element;
                auto const vs2_index = vs2 * elements_per_register * widening + vs_offset + vs_B_element;

                T const a = vector_elements[vs1_index];
                T const b = vector_elements[vs2_index];

                if constexpr (std::is_same_v<T, float16_t>)
                {
                    accumulator = f16_mulAdd(a, b, accumulator);
                }
                else if constexpr (std::is_same_v<T, float32_t>)
                {
                    accumulator = f32_mulAdd(a, b, accumulator);
                }
                else if constexpr (std::is_same_v<T, float64_t>)
                {
                    accumulator = f64_mulAdd(a, b, accumulator);
                }
            }

            vector_elements[vd_index] = accumulator;
        }
    }
}

// Single width MMACC
uint8_t vmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                  uint16_t const vs2, uint16_t const vstart, uint32_t const vlen)
{
    auto const vtype_decoded = decode_matrix_vtype(vtype);
    // auto punner = PointerPunner(vector_field);

    // Accumulator is always signed
    // auto *const output_elements = punner.i8;

    // For now just try uint8_t * uint8_t = uint8_t (fixed SEW and Widening, ignore altfmt fields)
    // Also ignore bs, as this encodes the block size for microscaling operations (vm = 0)
    // For future reference: bs == 0 -> block size = 32, 16 otherwise
    auto const sew = vtype_decoded.sew;
    auto const lambda = vtype_decoded.lambda;
    auto const lmul = vtype_decoded.lmul;
    auto const widening = 1;

    switch (sew)
    {
    case 8:
        mmacc<uint8_t>(reinterpret_cast<uint8_t *>(vector_field), vd, vs1, vs2, vlen, lambda, lmul, sew, widening,
                       !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    case 16:
        mmacc<uint16_t>(reinterpret_cast<uint16_t *>(vector_field), vd, vs1, vs2, vlen, lambda, lmul, sew, widening,
                        !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    case 32:
        mmacc<uint32_t>(reinterpret_cast<uint32_t *>(vector_field), vd, vs1, vs2, vlen, lambda, lmul, sew, widening,
                        !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    case 64:
        mmacc<uint64_t>(reinterpret_cast<uint64_t *>(vector_field), vd, vs1, vs2, vlen, lambda, lmul, sew, widening,
                        !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    default:
        // Illegal SEW
        break;
    }

    return 0;
}

// Double-widening MMACC
uint8_t vwmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                   uint16_t const vs2, uint16_t const vstart, uint32_t const vlen)
{
    auto const vtype_decoded = decode_matrix_vtype(vtype);
    auto const sew = vtype_decoded.sew;
    auto const lambda = vtype_decoded.lambda;
    auto const lmul = vtype_decoded.lmul;

    switch (sew)
    {
    case 16:
        wmmacc(reinterpret_cast<uint8_t *>(vector_field), reinterpret_cast<int16_t *>(vector_field), vd, vs1, vs2, vlen,
               lambda, lmul, sew, !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    case 32:
        wmmacc(reinterpret_cast<uint16_t *>(vector_field), reinterpret_cast<int32_t *>(vector_field), vd, vs1, vs2,
               vlen, lambda, lmul, sew, !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    case 64:
        wmmacc(reinterpret_cast<uint32_t *>(vector_field), reinterpret_cast<int64_t *>(vector_field), vd, vs1, vs2,
               vlen, lambda, lmul, sew, !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    default:
        // Illegal SEW
        break;
    }

    return 0;
}

// Quad-widening MMACC
uint8_t vqwmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                    uint16_t const vs2, uint16_t const vstart, uint32_t const vlen)
{
    auto const vtype_decoded = decode_matrix_vtype(vtype);
    auto const sew = vtype_decoded.sew;
    auto const lambda = vtype_decoded.lambda;
    auto const lmul = vtype_decoded.lmul;

    switch (sew)
    {
    case 32:
        wmmacc(reinterpret_cast<uint8_t *>(vector_field), reinterpret_cast<int32_t *>(vector_field), vd, vs1, vs2, vlen,
               lambda, lmul, sew, !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    case 64:
        wmmacc(reinterpret_cast<uint16_t *>(vector_field), reinterpret_cast<int64_t *>(vector_field), vd, vs1, vs2,
               vlen, lambda, lmul, sew, !vtype_decoded.altfmt_A, !vtype_decoded.altfmt_B);
        break;
    default:
        // Illegal SEW
        break;
    }

    return 0;
}

uint8_t vfmmacc_vv(uint8_t *const vector_field, uint32_t const vtype, uint16_t const vd, uint16_t const vs1,
                   uint16_t const vs2, uint16_t const vstart, uint32_t const vlen, uint8_t const rounding_mode)
{
    auto const vtype_decoded = decode_matrix_vtype(vtype);
    auto const sew = vtype_decoded.sew;
    auto const lambda = vtype_decoded.lambda;
    auto const lmul = vtype_decoded.lmul;
    auto const widening = 1;

    softfloat_exceptionFlags = 0;
    softfloat_roundingMode = rounding_mode;

    switch (sew)
    {
    case 16:
        mmacc_float<float16_t>(reinterpret_cast<float16_t *>(vector_field), vd, vs1, vs2, vlen, lambda, lmul, sew,
                               widening);
        break;
    case 32:
        mmacc_float<float32_t>(reinterpret_cast<float32_t *>(vector_field), vd, vs1, vs2, vlen, lambda, lmul, sew,
                               widening);
        break;
    case 64:
        mmacc_float<float64_t>(reinterpret_cast<float64_t *>(vector_field), vd, vs1, vs2, vlen, lambda, lmul, sew,
                               widening);
        break;
    default:
        break;
    }

    return 0;
}
