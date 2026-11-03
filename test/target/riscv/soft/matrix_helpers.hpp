#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <type_traits>
#include <vector>

inline constexpr auto LMUL_M1 = 0b000;
inline constexpr auto LMUL_M2 = 0b001;
inline constexpr auto LMUL_M4 = 0b010;
inline constexpr auto LMUL_M8 = 0b011;

inline constexpr auto SEW_OFFSET = 3;

inline constexpr auto SEW_E8 = 0b000;
inline constexpr auto SEW_E16 = 0b001;
inline constexpr auto SEW_E32 = 0b010;
inline constexpr auto SEW_E64 = 0b011;

inline constexpr auto LAMBDA_1 = 0b001;
inline constexpr auto LAMBDA_2 = 0b010;
inline constexpr auto LAMBDA_4 = 0b011;
inline constexpr auto LAMBDA_8 = 0b100;
inline constexpr auto LAMBDA_16 = 0b101;
inline constexpr auto LAMBDA_32 = 0b110;
inline constexpr auto LAMBDA_64 = 0b111;

#define RESET "\033[0m"
#define BLACK "\033[30m"              /* Black */
#define RED "\033[31m"                /* Red */
#define GREEN "\033[32m"              /* Green */
#define YELLOW "\033[33m"             /* Yellow */
#define BLUE "\033[34m"               /* Blue */
#define MAGENTA "\033[35m"            /* Magenta */
#define CYAN "\033[36m"               /* Cyan */
#define WHITE "\033[37m"              /* White */
#define BOLDBLACK "\033[1m\033[30m"   /* Bold Black */
#define BOLDRED "\033[1m\033[31m"     /* Bold Red */
#define BOLDGREEN "\033[1m\033[32m"   /* Bold Green */
#define BOLDYELLOW "\033[1m\033[33m"  /* Bold Yellow */
#define BOLDBLUE "\033[1m\033[34m"    /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m" /* Bold Magenta */
#define BOLDCYAN "\033[1m\033[36m"    /* Bold Cyan */
#define BOLDWHITE "\033[1m\033[37m"   /* Bold White */

template <typename T>
inline constexpr auto sign_zero_extend(T value, bool is_signed) -> uint64_t
{
    constexpr auto width = sizeof(T) * 8;
    static_assert(width <= 64);
    return static_cast<uint64_t>(((static_cast<int64_t>(value) << (64 - width)) >> (64 - width)) * is_signed) |
           (static_cast<uint64_t>(value) * !is_signed);
}

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
    requires std::is_integral_v<T>
unsigned constexpr max_decimal_width()
{
    auto max = std::numeric_limits<T>::max();
    auto w = 0;
    while (max)
    {
        max /= 10;
        w++;
    }
    return w;
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

// Row-major order
template <typename T>
    requires std::is_integral_v<T>
void convert(T *const vector_elements, unsigned const lambda, unsigned const vlen, unsigned v_register,
             unsigned widening, unsigned lmul, bool trans, std::vector<T> &serial, bool to_serial)
{
    auto const sew = sizeof(T) * 8;
    auto const elements_per_register = vlen / sew;
    auto const v_base = v_register * elements_per_register;
    auto const row_elems_per_register = lambda * widening;
    auto const cols = lambda * lmul * widening;
    auto const rows = (vlen / sew) / lambda;

    for (size_t row = 0; row < rows; ++row)
    {
        for (size_t col = 0; col < cols; ++col)
        {
            auto const used_col = trans ? row : col;
            auto const used_row = trans ? col : row;
            auto const vreg = used_col / row_elems_per_register;
            auto const velem = (used_row * row_elems_per_register) + used_col % row_elems_per_register;
            if (to_serial)
            {
                serial.push_back(vector_elements[v_base + (vreg * elements_per_register) + velem]);
            }
            else
            {
                vector_elements[v_base + (vreg * elements_per_register) + velem] = serial[row * col];
            }
        }
    }
}

template <typename T>
    requires std::is_integral_v<T>
bool is_equal(std::vector<T> const &A, std::vector<T> const &B)
{
    assert(A.size() == B.size());
    for (size_t i = 0; i < A.size(); ++i)
    {
        if (!(A[i] == B[i]))
        {
            return false;
        }
    }
    return true;
}

template <typename T>
    requires std::is_integral_v<T>
void print_matrix(std::vector<T> const &vec, unsigned const lambda, unsigned const lmul, unsigned const widening)
{
    auto const cols = lambda * lmul * widening;
    for (size_t row = 0; row < vec.size() / cols; ++row)
    {
        for (size_t col = 0; col < cols; ++col)
        {
            if constexpr (std::is_signed_v<T>)
            {
                std::printf("| %-*i ", max_decimal_width<T>() + 1, vec.at(row * cols + col));
            }
            else
            {
                std::printf("| %-*u ", max_decimal_width<T>(), vec.at(row * cols + col));
            }
        }
        std::printf("|\n\n");
    }
}

template <typename T>
    requires std::is_integral_v<T>
void print_rv_matrix(T *const vector_elements, unsigned const lambda, unsigned const vlen, unsigned v_register,
                     unsigned widening, unsigned lmul, bool trans)
{
    auto const sew = sizeof(T) * 8;
    auto const elements_per_register = vlen / sew;
    auto const total_elements = (vlen / sew) * lmul;
    auto const v_base = v_register * elements_per_register;
    auto const row_elems_per_register = lambda * widening;

    // std::printf("SEW %u EpR %u Total %u RElmspR %u\n", sew, elements_per_register, total_elements,
    //             row_elems_per_register);
    std::printf("v%u: ", v_register);

    auto const cols = lmul * row_elems_per_register;
    auto const rows = total_elements / cols;
    // std::printf("%u cols, %lu rows\n", cols, rows);
    for (size_t row = 0; row < rows; ++row)
    {
        for (size_t col = 0; col < cols; ++col)
        {
            auto const used_col = trans ? row : col;
            auto const used_row = trans ? col : row;
            auto const vreg = used_col / row_elems_per_register;
            auto const velem = (used_row * row_elems_per_register) + (used_col % row_elems_per_register);
            // auto const v_offset = (used_col / (lambda * widening)) * elements_per_register;
            // auto const v_element = (used_row * (lambda * widening)) + (used_col % (lambda * widening));
            if constexpr (std::is_signed_v<T>)
            {
                std::printf("| v%u[%u] %-*i ", v_register + vreg, velem, max_decimal_width<T>() + 1,
                            vector_elements[v_base + (vreg * elements_per_register) + velem]);
            }
            else
            {
                std::printf("| v%u[%u] %-*u ", v_register + vreg, velem, max_decimal_width<T>(),
                            vector_elements[v_base + (vreg * elements_per_register) + velem]);
            }
        }
        std::printf("|\n\n");
    }
}

inline constexpr uint32_t encode_matrix_vtype(unsigned sew, unsigned lmul, unsigned lambda, bool altfmt_A,
                                              bool altfmt_B, bool bs)
{
    // Ignore VILL
    return (sew << SEW_OFFSET) | (lmul) | (lambda << 28) | (altfmt_A << 27) | (altfmt_B << 26) | (bs << 25);
}

inline constexpr void zero_vectors(uint8_t *vector_field, size_t size)
{
    std::memset(vector_field, 0, size);
}

inline constexpr bool check_mul_C(unsigned mul_C)
{
    return (mul_C == 1 || mul_C == 2 || mul_C == 4 || mul_C == 8 || mul_C == 16);
}

inline constexpr bool check(unsigned mul_C, unsigned sew)
{
    auto valid_mul_C = (mul_C == 1 || mul_C == 2 || mul_C == 4 || mul_C == 8 || mul_C == 16);
    // std::printf("[Info] Currently only allowing SEW 8");
    auto valid_sew = (sew == SEW_E8);
    return valid_mul_C && valid_sew;
}

template <typename T_I, typename T_O>
    requires std::is_integral_v<T_I>
void mmacc(std::vector<T_I> const &A, std::vector<T_I> const &B, std::vector<T_O> &C, unsigned lmul, unsigned lambda,
           unsigned widening, bool signed_A, bool signed_B)
{
    auto const inner_dim = lmul * lambda * widening;
    auto const C_dim = A.size() / inner_dim;
    // std::printf("inner_dim %u, Cdim %lu\n", inner_dim, C_dim);
    for (size_t row = 0; row < C_dim; ++row)
    {
        for (size_t col = 0; col < C_dim; ++col)
        {
            uint64_t accumulator = 0;
            // std::printf("C_R[%u][%u] = ", row, col);
            for (size_t i = 0; i < inner_dim; ++i)
            {
                // std::printf("at %lu\n", row * inner_dim + i);
                auto const a_v = A.at(row * inner_dim + i);
                auto const b_v = B.at(col * inner_dim + i);
                // std::printf("+ (%lu * %lu) ", a_v, b_v);
                // accumulator += A.at(row * inner_dim + i) * B.at(col * inner_dim + i);
                accumulator += sign_zero_extend(a_v, signed_A) * sign_zero_extend(b_v, signed_B);
            }
            // std::printf("acc %u\n", accumulator);
            C.at(row * C_dim + col) += accumulator;
        }
        // std::printf("\n");
    }
}

template <typename T>
    requires std::is_integral_v<T>
void zero_vec(std::vector<T> &vec)
{
    std::memset(vec.data(), 0, vec.size() * sizeof(T));
}

template <typename T>
    requires std::is_integral_v<T>
void seq_fill(std::vector<T> &vec, unsigned lmul, unsigned lambda, unsigned widening, unsigned total_elements)
{
    auto const row_elms_per_register = lambda * widening;
    auto const cols = lmul * row_elms_per_register;
    auto const rows = total_elements / cols;
    auto const elms_per_register = row_elms_per_register * rows;
    // std::printf("seqfill rows %u cols %u\n", rows, cols);
    for (size_t row = 0; row < rows; ++row)
    {
        for (size_t col = 0; col < cols; ++col)
        {
            auto const vreg = col / row_elms_per_register;
            auto const velem = (row * row_elms_per_register) + (col % row_elms_per_register);
            auto const fill_val = vreg * elms_per_register + velem;
            // auto const fill_val = (col % (lambda * widening)) + (row * lambda * widening) +
            //                       (((col / (lambda * widening)) * rows) * lambda);
            // std::printf("col %u, row %u, Fill %u,\n", col, row, fval);
            vec.push_back(static_cast<T>(fill_val));
        }
    }
}