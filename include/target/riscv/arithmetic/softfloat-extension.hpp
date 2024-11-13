/*
============================================================================================================
The code in this file is copied from or heavily inspired by https://github.com/ics-jku/riscv-vp-plusplus
Therefore, this marks the start of the following copyright notice:
Copyright (c) 2017-2018 Group of Computer Architecture, University of Bremen <riscv@systemc-verification.org>
Copyright (c) 2022-2023 Intitute for Complex Systems, Johannes Kepler University Linz <ics-office@jku.at>
============================================================================================================
*/

#pragma once

#ifdef ETISS_SOFTFLOAT
extern "C"
{
#include "softfloat_orig.h"
}
#else
#include "softfloat.hpp"
#endif

constexpr uint16_t F16_SIGN_BIT = 1 << 15;
constexpr uint32_t F32_SIGN_BIT = 1 << 31;
constexpr uint64_t F64_SIGN_BIT = 1ul << 63;

constexpr uint16_t defaultNaNF16UI = 0x7E00;
constexpr uint32_t defaultNaNF32UI = 0x7fc00000;
constexpr uint64_t defaultNaNF64UI = 0x7FF8000000000000;

constexpr float16_t f16_defaultNaN = { defaultNaNF16UI };
constexpr float32_t f32_defaultNaN = { defaultNaNF32UI };
constexpr float64_t f64_defaultNaN = { defaultNaNF64UI };

float16_t f16(uint64_t value);

float32_t f32(uint64_t value);

float64_t f64(uint64_t value);

float16_t f16_neg(float16_t x);

float32_t f32_neg(float32_t x);

float64_t f64_neg(float64_t x);

bool is_boxed_f16(float64_t x);

float64_t box_f16(float16_t x);

float16_t unbox_f16(float64_t x);

bool is_boxed_f32(float64_t x);

float32_t unbox_f32(float64_t x);

float64_t box_f32(float32_t x);

float16_t check_and_unbox_f16(float64_t x);

float32_t check_and_unbox_f32(float64_t x);

float16_t f16_sgnj(float16_t f1, float16_t f2);

float16_t f16_sgnjn(float16_t f1, float16_t f2);

float16_t f16_sgnjx(float16_t f1, float16_t f2);

float32_t f32_sgnj(float32_t f1, float32_t f2);

float32_t f32_sgnjn(float32_t f1, float32_t f2);

float32_t f32_sgnjx(float32_t f1, float32_t f2);

float64_t f64_sgnj(float64_t f1, float64_t f2);

float64_t f64_sgnjn(float64_t f1, float64_t f2);

float64_t f64_sgnjx(float64_t f1, float64_t f2);

uint_fast16_t f16_classify(float16_t a);

uint_fast16_t f32_classify(float32_t a);

uint_fast16_t f64_classify(float64_t a);

float16_t f16_max(float16_t a, float16_t b);

float32_t f32_max(float32_t a, float32_t b);

float64_t f64_max(float64_t a, float64_t b);

float16_t f16_min(float16_t a, float16_t b);

float32_t f32_min(float32_t a, float32_t b);

float64_t f64_min(float64_t a, float64_t b);

float16_t f16_rsqrte7(float16_t in);

float16_t f16_recip7(float16_t in);

float32_t f32_rsqrte7(float32_t in);

float32_t f32_recip7(float32_t in);

float64_t f64_rsqrte7(float64_t in);

float64_t f64_recip7(float64_t in);