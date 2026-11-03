/*
============================================================================================================
The code in this file is copied from or heavily inspired by https://github.com/ics-jku/riscv-vp-plusplus
Therefore, this marks the start of the following copyright notice:
Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
Copyright (c) 2022-2023 Intitute for Complex Systems, Johannes Kepler University Linz <ics-office@jku.at>
============================================================================================================
*/

#include <assert.h>

#include "arithmetic/softfloat-extension.hpp"

extern "C"
{
#include "internals.h"
#include "specialize.h"
}

float16_t f16(uint64_t value)
{
    float16_t cast_f16{ (uint16_t)value };
    return cast_f16;
}

float32_t f32(uint64_t value)
{
    float32_t cast_f32{ (uint32_t)value };
    return cast_f32;
}

float64_t f64(uint64_t value)
{
    float64_t cast_f64{ (uint64_t)value };
    return cast_f64;
}

float16_t f16_neg(float16_t x)
{
    uint16_t res = x.v ^ F16_SIGN_BIT;
    return float16_t{ res };
}

float32_t f32_neg(float32_t x)
{
    return float32_t{ x.v ^ F32_SIGN_BIT };
}

float64_t f64_neg(float64_t x)
{
    return float64_t{ x.v ^ F64_SIGN_BIT };
}

bool is_boxed_f16(float64_t x)
{
    return (x.v >> 16) == 0xFFFFFFFFFFFF;
}

float64_t box_f16(float16_t x)
{
    return float64_t{ (uint64_t)x.v | 0xFFFFFFFFFFFF0000 };
}

float16_t unbox_f16(float64_t x)
{
    return float16_t{ (uint16_t)x.v };
}

bool is_boxed_f32(float64_t x)
{
    return (x.v >> 32) == (uint32_t)-1;
}

float32_t unbox_f32(float64_t x)
{
    return float32_t{ (uint32_t)x.v };
}

float64_t box_f32(float32_t x)
{
    return float64_t{ (uint64_t)x.v | 0xFFFFFFFF00000000 };
}

float16_t check_and_unbox_f16(float64_t x)
{
    if (is_boxed_f32(x))
        return unbox_f16(x);
    else
        return f16_defaultNaN;
}

float32_t check_and_unbox_f32(float64_t x)
{
    if (is_boxed_f32(x))
        return unbox_f32(x);
    else
        return f32_defaultNaN;
}

float16_t f16_sgnj(float16_t f1, float16_t f2)
{
    uint16_t res = (f1.v & ~F16_SIGN_BIT) | (f2.v & F16_SIGN_BIT);
    return float16_t{ res };
}

float16_t f16_sgnjn(float16_t f1, float16_t f2)
{
    uint16_t res = (f1.v & ~F16_SIGN_BIT) | (~f2.v & F16_SIGN_BIT);
    return float16_t{ res };
}

float16_t f16_sgnjx(float16_t f1, float16_t f2)
{
    uint16_t res = f1.v ^ (f2.v & F16_SIGN_BIT);
    return float16_t{ res };
}

float32_t f32_sgnj(float32_t f1, float32_t f2)
{
    return float32_t{ (f1.v & ~F32_SIGN_BIT) | (f2.v & F32_SIGN_BIT) };
}

float32_t f32_sgnjn(float32_t f1, float32_t f2)
{
    return float32_t{ (f1.v & ~F32_SIGN_BIT) | (~f2.v & F32_SIGN_BIT) };
}

float32_t f32_sgnjx(float32_t f1, float32_t f2)
{
    return float32_t{ f1.v ^ (f2.v & F32_SIGN_BIT) };
}

float64_t f64_sgnj(float64_t f1, float64_t f2)
{
    return float64_t{ (f1.v & ~F64_SIGN_BIT) | (f2.v & F64_SIGN_BIT) };
}

float64_t f64_sgnjn(float64_t f1, float64_t f2)
{
    return float64_t{ (f1.v & ~F64_SIGN_BIT) | (~f2.v & F64_SIGN_BIT) };
}

float64_t f64_sgnjx(float64_t f1, float64_t f2)
{
    return float64_t{ f1.v ^ (f2.v & F64_SIGN_BIT) };
}

uint_fast16_t f16_classify(float16_t a)
{
    union ui16_f16 uA;
    uint_fast16_t uiA;

    uA.f = a;
    uiA = uA.ui;

    uint_fast16_t infOrNaN = expF16UI(uiA) == 0x1F;
    uint_fast16_t subnormalOrZero = expF16UI(uiA) == 0;
    bool sign = signF16UI(uiA);
    bool fracZero = fracF16UI(uiA) == 0;
    bool isNaN = isNaNF16UI(uiA);
    bool isSNaN = softfloat_isSigNaNF16UI(uiA);

    return (sign && infOrNaN && fracZero) << 0 | (sign && !infOrNaN && !subnormalOrZero) << 1 |
           (sign && subnormalOrZero && !fracZero) << 2 | (sign && subnormalOrZero && fracZero) << 3 |
           (!sign && infOrNaN && fracZero) << 7 | (!sign && !infOrNaN && !subnormalOrZero) << 6 |
           (!sign && subnormalOrZero && !fracZero) << 5 | (!sign && subnormalOrZero && fracZero) << 4 |
           (isNaN && isSNaN) << 8 | (isNaN && !isSNaN) << 9;
}

uint_fast16_t f32_classify(float32_t a)
{
    union ui32_f32 uA;
    uint_fast32_t uiA;

    uA.f = a;
    uiA = uA.ui;

    uint_fast16_t infOrNaN = expF32UI(uiA) == 0xFF;
    uint_fast16_t subnormalOrZero = expF32UI(uiA) == 0;
    bool sign = signF32UI(uiA);
    bool fracZero = fracF32UI(uiA) == 0;
    bool isNaN = isNaNF32UI(uiA);
    bool isSNaN = softfloat_isSigNaNF32UI(uiA);

    return (sign && infOrNaN && fracZero) << 0 | (sign && !infOrNaN && !subnormalOrZero) << 1 |
           (sign && subnormalOrZero && !fracZero) << 2 | (sign && subnormalOrZero && fracZero) << 3 |
           (!sign && infOrNaN && fracZero) << 7 | (!sign && !infOrNaN && !subnormalOrZero) << 6 |
           (!sign && subnormalOrZero && !fracZero) << 5 | (!sign && subnormalOrZero && fracZero) << 4 |
           (isNaN && isSNaN) << 8 | (isNaN && !isSNaN) << 9;
}

uint_fast16_t f64_classify(float64_t a)
{
    union ui64_f64 uA;
    uint_fast64_t uiA;

    uA.f = a;
    uiA = uA.ui;

    uint_fast16_t infOrNaN = expF64UI(uiA) == 0x7FF;
    uint_fast16_t subnormalOrZero = expF64UI(uiA) == 0;
    bool sign = signF64UI(uiA);
    bool fracZero = fracF64UI(uiA) == 0;
    bool isNaN = isNaNF64UI(uiA);
    bool isSNaN = softfloat_isSigNaNF64UI(uiA);

    return (sign && infOrNaN && fracZero) << 0 | (sign && !infOrNaN && !subnormalOrZero) << 1 |
           (sign && subnormalOrZero && !fracZero) << 2 | (sign && subnormalOrZero && fracZero) << 3 |
           (!sign && infOrNaN && fracZero) << 7 | (!sign && !infOrNaN && !subnormalOrZero) << 6 |
           (!sign && subnormalOrZero && !fracZero) << 5 | (!sign && subnormalOrZero && fracZero) << 4 |
           (isNaN && isSNaN) << 8 | (isNaN && !isSNaN) << 9;
}

/* Min. & Max. */
#define COMPARE_MAX(a, b, bits)                                                                  \
    float##bits##_t f##bits##_max(float##bits##_t a, float##bits##_t b)                          \
    {                                                                                            \
        bool greater = f##bits##_lt_quiet(b, a) || (f##bits##_eq(b, a) && signF##bits##UI(b.v)); \
                                                                                                 \
        if (isNaNF##bits##UI(a.v) && isNaNF##bits##UI(b.v))                                      \
        {                                                                                        \
            union ui##bits##_f##bits ui;                                                         \
            ui.ui = defaultNaNF##bits##UI;                                                       \
            return ui.f;                                                                         \
        }                                                                                        \
        else                                                                                     \
        {                                                                                        \
            return greater || isNaNF##bits##UI((b).v) ? a : b;                                   \
        }                                                                                        \
    }

#define COMPARE_MIN(a, b, bits)                                                               \
    float##bits##_t f##bits##_min(float##bits##_t a, float##bits##_t b)                       \
    {                                                                                         \
        bool less = f##bits##_lt_quiet(a, b) || (f##bits##_eq(a, b) && signF##bits##UI(a.v)); \
                                                                                              \
        if (isNaNF##bits##UI(a.v) && isNaNF##bits##UI(b.v))                                   \
        {                                                                                     \
            union ui##bits##_f##bits ui;                                                      \
            ui.ui = defaultNaNF##bits##UI;                                                    \
            return ui.f;                                                                      \
        }                                                                                     \
        else                                                                                  \
        {                                                                                     \
            return less || isNaNF##bits##UI((b).v) ? a : b;                                   \
        }                                                                                     \
    }

COMPARE_MAX(a, b, 16);
COMPARE_MAX(a, b, 32);
COMPARE_MAX(a, b, 64);

COMPARE_MIN(a, b, 16);
COMPARE_MIN(a, b, 32);
COMPARE_MIN(a, b, 64);

static inline uint64_t extract64(uint64_t val, int pos, int len)
{
    assert(pos >= 0 && len > 0 && len <= 64 - pos);
    return (val >> pos) & (~UINT64_C(0) >> (64 - len));
}

static inline uint64_t make_mask64(int pos, int len)
{
    assert(pos >= 0 && len > 0 && pos < 64 && len <= 64);
    return (UINT64_MAX >> (64 - len)) << pos;
}

// user needs to truncate output to required length
static inline uint64_t rsqrte7(uint64_t val, int e, int s, bool sub)
{
    uint64_t exp = extract64(val, s, e);
    uint64_t sig = extract64(val, 0, s);
    uint64_t sign = extract64(val, s + e, 1);
    const int p = 7;

    static const uint8_t table[] = { 52,  51,  50,  48,  47,  46,  44,  43,  42,  41,  40,  39,  38,  36,  35,  34,
                                     33,  32,  31,  30,  30,  29,  28,  27,  26,  25,  24,  23,  23,  22,  21,  20,
                                     19,  19,  18,  17,  16,  16,  15,  14,  14,  13,  12,  12,  11,  10,  10,  9,
                                     9,   8,   7,   7,   6,   6,   5,   4,   4,   3,   3,   2,   2,   1,   1,   0,
                                     127, 125, 123, 121, 119, 118, 116, 114, 113, 111, 109, 108, 106, 105, 103, 102,
                                     100, 99,  97,  96,  95,  93,  92,  91,  90,  88,  87,  86,  85,  84,  83,  82,
                                     80,  79,  78,  77,  76,  75,  74,  73,  72,  71,  70,  70,  69,  68,  67,  66,
                                     65,  64,  63,  63,  62,  61,  60,  59,  59,  58,  57,  56,  56,  55,  54,  53 };

    if (sub)
    {
        while (extract64(sig, s - 1, 1) == 0)
            exp--, sig <<= 1;

        sig = (sig << 1) & make_mask64(0, s);
    }

    int idx = ((exp & 1) << (p - 1)) | (sig >> (s - p + 1));
    uint64_t out_sig = (uint64_t)(table[idx]) << (s - p);
    uint64_t out_exp = (3 * make_mask64(0, e - 1) + ~exp) / 2;

    return (sign << (s + e)) | (out_exp << s) | out_sig;
}

float16_t f16_rsqrte7(float16_t in)
{
    union ui16_f16 uA;

    uA.f = in;
    unsigned int ret = f16_classify(in);
    bool sub = false;
    switch (ret)
    {
    case 0x001: // -inf
    case 0x002: // -normal
    case 0x004: // -subnormal
    case 0x100: // sNaN
        softfloat_exceptionFlags |= softfloat_flag_invalid;
    case 0x200: // qNaN
        uA.ui = defaultNaNF16UI;
        break;
    case 0x008: // -0
        uA.ui = 0xfc00;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x010: // +0
        uA.ui = 0x7c00;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x080: //+inf
        uA.ui = 0x0;
        break;
    case 0x020: //+ sub
        sub = true;
    default: // +num
        uA.ui = rsqrte7(uA.ui, 5, 10, sub);
        break;
    }

    return uA.f;
}

float32_t f32_rsqrte7(float32_t in)
{
    union ui32_f32 uA;

    uA.f = in;
    unsigned int ret = f32_classify(in);
    bool sub = false;
    switch (ret)
    {
    case 0x001: // -inf
    case 0x002: // -normal
    case 0x004: // -subnormal
    case 0x100: // sNaN
        softfloat_exceptionFlags |= softfloat_flag_invalid;
    case 0x200: // qNaN
        uA.ui = defaultNaNF32UI;
        break;
    case 0x008: // -0
        uA.ui = 0xff800000;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x010: // +0
        uA.ui = 0x7f800000;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x080: //+inf
        uA.ui = 0x0;
        break;
    case 0x020: //+ sub
        sub = true;
    default: // +num
        uA.ui = rsqrte7(uA.ui, 8, 23, sub);
        break;
    }

    return uA.f;
}

float64_t f64_rsqrte7(float64_t in)
{
    union ui64_f64 uA;

    uA.f = in;
    unsigned int ret = f64_classify(in);
    bool sub = false;
    switch (ret)
    {
    case 0x001: // -inf
    case 0x002: // -normal
    case 0x004: // -subnormal
    case 0x100: // sNaN
        softfloat_exceptionFlags |= softfloat_flag_invalid;
    case 0x200: // qNaN
        uA.ui = defaultNaNF64UI;
        break;
    case 0x008: // -0
        uA.ui = 0xfff0000000000000ul;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x010: // +0
        uA.ui = 0x7ff0000000000000ul;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x080: //+inf
        uA.ui = 0x0;
        break;
    case 0x020: //+ sub
        sub = true;
    default: // +num
        uA.ui = rsqrte7(uA.ui, 11, 52, sub);
        break;
    }

    return uA.f;
}

// user needs to truncate output to required length
static inline uint64_t recip7(uint64_t val, int e, int s, int rm, bool sub, bool *round_abnormal)
{
    uint64_t exp = extract64(val, s, e);
    uint64_t sig = extract64(val, 0, s);
    uint64_t sign = extract64(val, s + e, 1);
    const int p = 7;

    static const uint8_t table[] = { 127, 125, 123, 121, 119, 117, 116, 114, 112, 110, 109, 107, 105, 104, 102, 100,
                                     99,  97,  96,  94,  93,  91,  90,  88,  87,  85,  84,  83,  81,  80,  79,  77,
                                     76,  75,  74,  72,  71,  70,  69,  68,  66,  65,  64,  63,  62,  61,  60,  59,
                                     58,  57,  56,  55,  54,  53,  52,  51,  50,  49,  48,  47,  46,  45,  44,  43,
                                     42,  41,  40,  40,  39,  38,  37,  36,  35,  35,  34,  33,  32,  31,  31,  30,
                                     29,  28,  28,  27,  26,  25,  25,  24,  23,  23,  22,  21,  21,  20,  19,  19,
                                     18,  17,  17,  16,  15,  15,  14,  14,  13,  12,  12,  11,  11,  10,  9,   9,
                                     8,   8,   7,   7,   6,   5,   5,   4,   4,   3,   3,   2,   2,   1,   1,   0 };

    if (sub)
    {
        while (extract64(sig, s - 1, 1) == 0)
            exp--, sig <<= 1;

        sig = (sig << 1) & make_mask64(0, s);

        if (exp != 0 && exp != UINT64_MAX)
        {
            *round_abnormal = true;
            if (rm == 1 || (rm == 2 && !sign) || (rm == 3 && sign))
                return ((sign << (s + e)) | make_mask64(s, e)) - 1;
            else
                return (sign << (s + e)) | make_mask64(s, e);
        }
    }

    int idx = sig >> (s - p);
    uint64_t out_sig = (uint64_t)(table[idx]) << (s - p);
    uint64_t out_exp = 2 * make_mask64(0, e - 1) + ~exp;
    if (out_exp == 0 || out_exp == UINT64_MAX)
    {
        out_sig = (out_sig >> 1) | make_mask64(s - 1, 1);
        if (out_exp == UINT64_MAX)
        {
            out_sig >>= 1;
            out_exp = 0;
        }
    }

    return (sign << (s + e)) | (out_exp << s) | out_sig;
}

float16_t f16_recip7(float16_t in)
{
    union ui16_f16 uA;

    uA.f = in;
    unsigned int ret = f16_classify(in);
    bool sub = false;
    bool round_abnormal = false;
    switch (ret)
    {
    case 0x001: // -inf
        uA.ui = 0x8000;
        break;
    case 0x080: //+inf
        uA.ui = 0x0;
        break;
    case 0x008: // -0
        uA.ui = 0xfc00;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x010: // +0
        uA.ui = 0x7c00;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x100: // sNaN
        softfloat_exceptionFlags |= softfloat_flag_invalid;
    case 0x200: // qNaN
        uA.ui = defaultNaNF16UI;
        break;
    case 0x004: // -subnormal
    case 0x020: //+ sub
        sub = true;
    default: // +- normal
        uA.ui = recip7(uA.ui, 5, 10, softfloat_roundingMode, sub, &round_abnormal);
        if (round_abnormal)
            softfloat_exceptionFlags |= softfloat_flag_inexact | softfloat_flag_overflow;
        break;
    }

    return uA.f;
}

float32_t f32_recip7(float32_t in)
{
    union ui32_f32 uA;

    uA.f = in;
    unsigned int ret = f32_classify(in);
    bool sub = false;
    bool round_abnormal = false;
    switch (ret)
    {
    case 0x001: // -inf
        uA.ui = 0x80000000;
        break;
    case 0x080: //+inf
        uA.ui = 0x0;
        break;
    case 0x008: // -0
        uA.ui = 0xff800000;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x010: // +0
        uA.ui = 0x7f800000;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x100: // sNaN
        softfloat_exceptionFlags |= softfloat_flag_invalid;
    case 0x200: // qNaN
        uA.ui = defaultNaNF32UI;
        break;
    case 0x004: // -subnormal
    case 0x020: //+ sub
        sub = true;
    default: // +- normal
        uA.ui = recip7(uA.ui, 8, 23, softfloat_roundingMode, sub, &round_abnormal);
        if (round_abnormal)
            softfloat_exceptionFlags |= softfloat_flag_inexact | softfloat_flag_overflow;
        break;
    }

    return uA.f;
}

float64_t f64_recip7(float64_t in)
{
    union ui64_f64 uA;

    uA.f = in;
    unsigned int ret = f64_classify(in);
    bool sub = false;
    bool round_abnormal = false;
    switch (ret)
    {
    case 0x001: // -inf
        uA.ui = 0x8000000000000000;
        break;
    case 0x080: //+inf
        uA.ui = 0x0;
        break;
    case 0x008: // -0
        uA.ui = 0xfff0000000000000;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x010: // +0
        uA.ui = 0x7ff0000000000000;
        softfloat_exceptionFlags |= softfloat_flag_infinite;
        break;
    case 0x100: // sNaN
        softfloat_exceptionFlags |= softfloat_flag_invalid;
    case 0x200: // qNaN
        uA.ui = defaultNaNF64UI;
        break;
    case 0x004: // -subnormal
    case 0x020: //+ sub
        sub = true;
    default: // +- normal
        uA.ui = recip7(uA.ui, 11, 52, softfloat_roundingMode, sub, &round_abnormal);
        if (round_abnormal)
            softfloat_exceptionFlags |= softfloat_flag_inexact | softfloat_flag_overflow;
        break;
    }

    return uA.f;
}

int_fast8_t f16_to_i8(float16_t a, uint_fast8_t roundingMode, bool exact)
{
    uint_fast8_t old_flags = softfloat_exceptionFlags;

    int_fast32_t sig32 = f16_to_i32(a, roundingMode, exact);

    if (sig32 > INT8_MAX)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return i8_fromPosOverflow;
    }
    else if (sig32 < INT8_MIN)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return i8_fromNegOverflow;
    }
    else
    {
        return sig32;
    }
}

int_fast16_t f32_to_i16(float32_t a, uint_fast8_t roundingMode, bool exact)
{
    uint_fast8_t old_flags = softfloat_exceptionFlags;

    int_fast32_t sig32 = f32_to_i32(a, roundingMode, exact);

    if (sig32 > INT16_MAX)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return i16_fromPosOverflow;
    }
    else if (sig32 < INT16_MIN)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return i16_fromNegOverflow;
    }
    else
    {
        return sig32;
    }
}

int_fast32_t f64_to_i32(float64_t a, uint_fast8_t roundingMode, bool exact)
{
    union ui64_f64 uA;
    uint_fast64_t uiA;
    bool sign;
    int_fast16_t exp;
    uint_fast64_t sig;
    int_fast16_t shiftDist;

    /*------------------------------------------------------------------------
     *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    sign = signF64UI(uiA);
    exp = expF64UI(uiA);
    sig = fracF64UI(uiA);
    /*------------------------------------------------------------------------
     *------------------------------------------------------------------------*/
#if (i32_fromNaN != i32_fromPosOverflow) || (i32_fromNaN != i32_fromNegOverflow)
    if ((exp == 0x7FF) && sig)
    {
#if (i32_fromNaN == i32_fromPosOverflow)
        sign = 0;
#elif (i32_fromNaN == i32_fromNegOverflow)
        sign = 1;
#else
        softfloat_raiseFlags(softfloat_flag_invalid);
        return i32_fromNaN;
#endif
    }
#endif
    /*------------------------------------------------------------------------
     *------------------------------------------------------------------------*/
    if (exp)
        sig |= UINT64_C(0x0010000000000000);
    shiftDist = 0x427 - exp;
    if (0 < shiftDist)
        sig = softfloat_shiftRightJam64(sig, shiftDist);
    return softfloat_roundToI32(sign, sig, roundingMode, exact);
}

uint_fast8_t f16_to_ui8(float16_t a, uint_fast8_t roundingMode, bool exact)
{
    uint_fast8_t old_flags = softfloat_exceptionFlags;

    uint_fast32_t sig32 = f16_to_ui32(a, roundingMode, exact);

    if (sig32 > UINT8_MAX)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return ui8_fromPosOverflow;
    }
    else
    {
        return sig32;
    }
}

uint_fast16_t f32_to_ui16(float32_t a, uint_fast8_t roundingMode, bool exact)
{
    uint_fast8_t old_flags = softfloat_exceptionFlags;

    uint_fast32_t sig32 = f32_to_ui32(a, roundingMode, exact);

    if (sig32 > UINT16_MAX)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return ui16_fromPosOverflow;
    }
    else
    {
        return sig32;
    }
}

uint_fast32_t f64_to_ui32(float64_t a, uint_fast8_t roundingMode, bool exact)
{
    union ui64_f64 uA;
    uint_fast64_t uiA;
    bool sign;
    int_fast16_t exp;
    uint_fast64_t sig;
    int_fast16_t shiftDist;

    /*------------------------------------------------------------------------
     *------------------------------------------------------------------------*/
    uA.f = a;
    uiA = uA.ui;
    sign = signF64UI(uiA);
    exp = expF64UI(uiA);
    sig = fracF64UI(uiA);
    /*------------------------------------------------------------------------
     *------------------------------------------------------------------------*/
#if (ui32_fromNaN != ui32_fromPosOverflow) || (ui32_fromNaN != ui32_fromNegOverflow)
    if ((exp == 0x7FF) && sig)
    {
#if (ui32_fromNaN == ui32_fromPosOverflow)
        sign = 0;
#elif (ui32_fromNaN == ui32_fromNegOverflow)
        sign = 1;
#else
        softfloat_raiseFlags(softfloat_flag_invalid);
        return ui32_fromNaN;
#endif
    }
#endif
    /*------------------------------------------------------------------------
     *------------------------------------------------------------------------*/
    if (exp)
        sig |= UINT64_C(0x0010000000000000);
    shiftDist = 0x427 - exp;
    if (0 < shiftDist)
        sig = softfloat_shiftRightJam64(sig, shiftDist);
    return softfloat_roundToUI32(sign, sig, roundingMode, exact);
}

int_fast16_t f16_to_i16(float16_t a, uint_fast8_t roundingMode, bool exact)
{
    uint_fast8_t old_flags = softfloat_exceptionFlags;

    int_fast32_t sig32 = f16_to_i32(a, roundingMode, exact);

    if (sig32 > INT16_MAX)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return i16_fromPosOverflow;
    }
    else if (sig32 < INT16_MIN)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return i16_fromNegOverflow;
    }
    else
    {
        return sig32;
    }
}

uint_fast16_t f16_to_ui16(float16_t a, uint_fast8_t roundingMode, bool exact)
{
    uint_fast8_t old_flags = softfloat_exceptionFlags;

    uint_fast32_t sig32 = f16_to_ui32(a, roundingMode, exact);

    if (sig32 > UINT16_MAX)
    {
        softfloat_exceptionFlags = old_flags | softfloat_flag_invalid;
        return ui16_fromPosOverflow;
    }
    else
    {
        return sig32;
    }
}