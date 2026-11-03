#include "softvector.h"
#include "matrix_helpers.hpp"

#include <array>
#include <cstdint>
#include <cstdio>
#include <type_traits>
#include <vector>

template <typename BaseType>
struct ResultWidener;

template <>
struct ResultWidener<uint8_t>
{
    using doublewide = int16_t;
    using quadwide = int32_t;
    using octwide = int64_t;
};

template <>
struct ResultWidener<int8_t>
{
    using doublewide = int16_t;
    using quadwide = int32_t;
    using octwide = int64_t;
};

template <>
struct ResultWidener<uint16_t>
{
    using doublewide = int32_t;
    using quadwide = int64_t;
};

template <>
struct ResultWidener<int16_t>
{
    using doublewide = int32_t;
    using quadwide = int64_t;
};

template <>
struct ResultWidener<uint32_t>
{
    using doublewide = int64_t;
};

template <>
struct ResultWidener<int32_t>
{
    using doublewide = int64_t;
};

static constexpr auto max_vlen = 1U << 16;
static constexpr auto max_vlen_bytes = max_vlen >> 3;
static constexpr auto n_vector_registers = 32;
static constexpr auto vector_field_bytes = max_vlen_bytes * n_vector_registers;

alignas(64) auto vector_field = std::array<uint8_t, vector_field_bytes>{ 0 };
unsigned n_failed = 0;
unsigned n_tests = 0;

template <typename T>
    requires std::is_integral_v<T>
bool seq_increase_test(unsigned sew, unsigned lmul, unsigned lambda, unsigned vd, unsigned vs1, unsigned vs2,
                       unsigned vlen, bool signed_A, bool signed_B)
{
    auto const vtype = encode_matrix_vtype(sew, lmul, lambda, !signed_A, !signed_B, false);
    auto const decoded_vtype = decode_matrix_vtype(vtype);
    auto const elements_per_register = vlen / decoded_vtype.sew;
    auto const mul_C = elements_per_register / (decoded_vtype.lambda * decoded_vtype.lambda);
    if (!check_mul_C(mul_C))
    {
        std::printf("Illegal MUL_C\n");
        return false;
    }
    n_tests++;
    using ResultType = std::make_signed_t<T>;

    std::vector<T> A;
    A.reserve(elements_per_register * lmul);

    std::vector<T> B;
    B.reserve(elements_per_register * lmul);

    std::vector<ResultType> C;
    // C.reserve(elements_per_register * mul_C);
    C.resize(elements_per_register * mul_C);
    zero_vec(C);

    // Fill vectors sequentially
    // RISC-V
    vid_v(vector_field.data(), static_cast<uint16_t>(vtype), 1, vs1, 0, vlen,
          (vlen / decoded_vtype.sew) * decoded_vtype.lmul);
    vid_v(vector_field.data(), static_cast<uint16_t>(vtype), 1, vs2, 0, vlen,
          (vlen / decoded_vtype.sew) * decoded_vtype.lmul);

    // Golden
    seq_fill(A, decoded_vtype.lmul, decoded_vtype.lambda, 1, elements_per_register * decoded_vtype.lmul);
    seq_fill(B, decoded_vtype.lmul, decoded_vtype.lambda, 1, elements_per_register * decoded_vtype.lmul);

    // MMACC
    vmmacc_vv(vector_field.data(), vtype, vd, vs1, vs2, 0, vlen);
    mmacc(A, B, C, decoded_vtype.lmul, decoded_vtype.lambda, 1, signed_A, signed_B);

    std::vector<ResultType> C_from_RV;

    convert(static_cast<ResultType *>(static_cast<void *>(vector_field.data())), decoded_vtype.lambda, vlen, vd, 1,
            mul_C, false, C_from_RV, true);
    auto is_ok = is_equal(C, C_from_RV);
    if (!is_ok)
    {
        std::printf("SINGLE WIDTH: Result not equal to golden result!\n");
        n_failed++;
    }
    else
    {
        std::printf("\t%sSINGLE WIDTH: Test success%s\n", GREEN, RESET);
    }
    zero_vectors(vector_field.data(), vector_field.size());
    return is_ok;
}

template <typename T>
    requires std::is_integral_v<T>
bool seq_increase_test_double(unsigned sew, unsigned lmul, unsigned lambda, unsigned vd, unsigned vs1, unsigned vs2,
                              unsigned vlen, bool signed_A, bool signed_B)
{
    auto const vtype_wmmacc = encode_matrix_vtype(sew, lmul, lambda, !signed_A, !signed_B, false);
    auto const vtype_vid = encode_matrix_vtype(sew - 1, lmul, lambda, !signed_A, !signed_B, false);
    auto const decoded_vtype = decode_matrix_vtype(vtype_wmmacc);
    auto const elements_per_register = vlen / decoded_vtype.sew;
    auto const mul_C = elements_per_register / (decoded_vtype.lambda * decoded_vtype.lambda);
    if (!check_mul_C(mul_C))
    {
        std::printf("Illegal MUL_C\n");
        return false;
    }
    n_tests++;
    auto const lambda_val = 1U << (lambda - 1);
    // std::printf("QUAD: SEW: %u, LMUL: %u, LAMBDA: %u, VLEN: %u\n", 8u << sew, 1U << lmul, lambda_val, vlen);

    using ResultType = ResultWidener<T>::doublewide;
    static constexpr auto widening = 2;
    std::vector<T> A;
    A.reserve(elements_per_register * lmul * widening);

    std::vector<T> B;
    B.reserve(elements_per_register * lmul * widening);

    std::vector<ResultType> C;
    // C.reserve(elements_per_register * mul_C);
    C.resize(elements_per_register * mul_C);
    zero_vec(C);

    // Fill vectors sequentially
    // RISC-V
    vid_v(vector_field.data(), static_cast<uint16_t>(vtype_vid), 1, vs1, 0, vlen,
          elements_per_register * decoded_vtype.lmul * widening);
    vid_v(vector_field.data(), static_cast<uint16_t>(vtype_vid), 1, vs2, 0, vlen,
          elements_per_register * decoded_vtype.lmul * widening);

    // Golden
    seq_fill(A, decoded_vtype.lmul, decoded_vtype.lambda, widening,
             elements_per_register * decoded_vtype.lmul * widening);
    seq_fill(B, decoded_vtype.lmul, decoded_vtype.lambda, widening,
             elements_per_register * decoded_vtype.lmul * widening);

    // MMACC
    vwmmacc_vv(vector_field.data(), vtype_wmmacc, vd, vs1, vs2, 0, vlen);
    mmacc(A, B, C, decoded_vtype.lmul, decoded_vtype.lambda, widening, signed_A, signed_B);

    std::vector<ResultType> C_from_RV;

    convert(reinterpret_cast<ResultType *>(vector_field.data()), decoded_vtype.lambda, vlen, vd, 1, mul_C, false,
            C_from_RV, true);
    auto is_ok = is_equal(C, C_from_RV);
    if (!is_ok)
    {
        std::printf("DOUBLE: Result not equal to golden result!\n");
        n_failed++;
    }
    else
    {
        std::printf("\t%sDOUBLE:       Test success%s\n", GREEN, RESET);
    }
    zero_vectors(vector_field.data(), vector_field.size());
    return is_ok;
}

template <typename T>
    requires std::is_integral_v<T>
bool seq_increase_test_quad(unsigned sew, unsigned lmul, unsigned lambda, unsigned vd, unsigned vs1, unsigned vs2,
                            unsigned vlen, bool signed_A, bool signed_B)
{
    auto const vtype_qwmmacc = encode_matrix_vtype(sew, lmul, lambda, !signed_A, !signed_B, false);
    auto const vtype_vid = encode_matrix_vtype(sew - 2, lmul, lambda, !signed_A, !signed_B, false);
    auto const decoded_vtype = decode_matrix_vtype(vtype_qwmmacc);
    auto const elements_per_register = vlen / decoded_vtype.sew;
    auto const mul_C = elements_per_register / (decoded_vtype.lambda * decoded_vtype.lambda);
    if (!check_mul_C(mul_C))
    {
        std::printf("Illegal MUL_C\n");
        return false;
    }
    n_tests++;
    auto const lambda_val = 1U << (lambda - 1);
    // std::printf("QUAD: SEW: %u, LMUL: %u, LAMBDA: %u, VLEN: %u\n", 8u << sew, 1U << lmul, lambda_val, vlen);

    using ResultType = ResultWidener<T>::quadwide;
    static constexpr auto widening = 4;
    std::vector<T> A;
    A.reserve(elements_per_register * lmul * widening);

    std::vector<T> B;
    B.reserve(elements_per_register * lmul * widening);

    std::vector<ResultType> C;
    // C.reserve(elements_per_register * mul_C);
    C.resize(elements_per_register * mul_C);
    zero_vec(C);

    // Fill vectors sequentially
    // RISC-V
    vid_v(vector_field.data(), static_cast<uint16_t>(vtype_vid), 1, vs1, 0, vlen,
          elements_per_register * decoded_vtype.lmul * widening);
    vid_v(vector_field.data(), static_cast<uint16_t>(vtype_vid), 1, vs2, 0, vlen,
          elements_per_register * decoded_vtype.lmul * widening);

    // Golden
    seq_fill(A, decoded_vtype.lmul, decoded_vtype.lambda, widening,
             elements_per_register * decoded_vtype.lmul * widening);
    seq_fill(B, decoded_vtype.lmul, decoded_vtype.lambda, widening,
             elements_per_register * decoded_vtype.lmul * widening);

    // MMACC
    vqwmmacc_vv(vector_field.data(), vtype_qwmmacc, vd, vs1, vs2, 0, vlen);
    mmacc(A, B, C, decoded_vtype.lmul, decoded_vtype.lambda, widening, signed_A, signed_B);

    std::vector<ResultType> C_from_RV;

    convert(reinterpret_cast<ResultType *>(vector_field.data()), decoded_vtype.lambda, vlen, vd, 1, mul_C, false,
            C_from_RV, true);
    auto is_ok = is_equal(C, C_from_RV);
    if (!is_ok)
    {
        std::printf("QUAD: Result not equal to golden result!\n");
        n_failed++;
    }
    else
    {
        std::printf("\t%sQUAD:         Test success%s\n", GREEN, RESET);
    }
    zero_vectors(vector_field.data(), vector_field.size());
    return is_ok;
}

int main()
{
    auto const lambdas = std::to_array({ LAMBDA_1, LAMBDA_2, LAMBDA_4, LAMBDA_8, LAMBDA_16, LAMBDA_32, LAMBDA_64 });
    auto const lmuls = std::to_array({ LMUL_M1, LMUL_M2, LMUL_M4, LMUL_M8 });
    auto const sews = std::to_array({ SEW_E8, SEW_E16, SEW_E32, SEW_E64 });
    auto vlen = 64U;

    while (vlen <= max_vlen)
    {
        for (auto &&lmul : lmuls)
        {
            for (auto &&lambda : lambdas)
            {
                for (auto &&sew : sews)
                {
                    for (int signed_A = 0; signed_A < 2; signed_A++)
                    {
                        for (int signed_B = 0; signed_B < 2; signed_B++)
                        {
                            auto const sew_val = 8U << sew;
                            auto const elements_per_register = vlen / sew_val;
                            auto const lambda_val = 1U << (lambda - 1);
                            auto const mul_C = elements_per_register / (lambda_val * lambda_val);
                            if (!check_mul_C(mul_C))
                            {
                                continue;
                            }
                            std::printf("SEW: %u, LMUL: %u, LAMBDA: %u, VLEN: %u, A %s, B %s\n", sew_val, 1U << lmul,
                                        lambda_val, vlen, signed_A ? "signed" : "unsigned",
                                        signed_B ? "signed" : "unsigned");
                            switch (sew_val)
                            {
                            case 8:
                                seq_increase_test<uint8_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A, signed_B);
                                break;
                            case 16:
                                seq_increase_test<uint16_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A, signed_B);
                                seq_increase_test_double<uint8_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A,
                                                                  signed_B);
                                break;
                            case 32:
                                seq_increase_test<uint32_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A, signed_B);
                                seq_increase_test_double<uint16_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A,
                                                                   signed_B);
                                seq_increase_test_quad<uint8_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A, signed_B);
                                break;
                            case 64:
                                seq_increase_test<uint64_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A, signed_B);
                                seq_increase_test_double<uint32_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A,
                                                                   signed_B);
                                seq_increase_test_quad<uint16_t>(sew, lmul, lambda, 16, 0, 8, vlen, signed_A, signed_B);
                                break;
                            default:
                                break;
                            }
                        }
                    }
                }
            }
        }
        vlen <<= 1;
    }

    std::printf("");
    return n_failed;
}