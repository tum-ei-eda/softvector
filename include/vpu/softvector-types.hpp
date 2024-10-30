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
/// \file softvector-types.hpp
/// \brief Extended types for softvector representation
/// \date 07/03/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __SOFTVECTOR_TYPES_H__
#define __SOFTVECTOR_TYPES_H__

#include "stdint.h"
#include <vector>
#include <array>
#include <cstring>

class SVector;

//////////////////////////////////////////////////////////////////////////////////////
/// \class SVRegister
/// \brief Simple register class.
class SVRegister
{
    bool reference_mem_{
        true
    }; //!< If true the memory is allocated somewhere at a different context. Else SVRegister allocates itself.
  public:
    size_t length_bits_; //!< Memory length in bits
    uint8_t *mem_;       //!< Main memory

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get the value of a single bit
    bool get_bit(size_t bit_index //!< Index (bit number) of the target bit
    ) const
    {
        return (mem_[bit_index / 8] & (1 << (bit_index % 8)));
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Set the value of a single bit
    void set_bit(size_t bit_index, //!< Index (bit number) of the target bit
                 bool val = true)
    {
        mem_[bit_index / 8] |= (1 << (bit_index % 8));
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reset the value of a single bit
    void reset_bit(size_t bit_index //!< Index (bit number) of the target bit
    )
    {
        mem_[bit_index / 8] &= ~(1 << (bit_index % 8));
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Toggle the value of a single bit
    void toggle_bit(size_t bit_index //!< Index (bit number) of the target bit
    )
    {
        mem_[bit_index / 8] ^= (1 << (bit_index % 8));
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked assignment. Updates bits in main memory with input register only where mask register is true
    void m_assign(const SVRegister &in,  //!< Input
                  const SVRegister &vm,  //!< Mask register
                  size_t start_index = 0 //!< Start (bit) index
    );

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_eq(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_eq(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked NOT EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_neq(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked NOT EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_neq(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed LESS THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_s_lt(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed LESS THAN for right-hand-side signed 64 bit value. rhs is sign extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_s_lt(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed LESS THAN OR EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_s_lte(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask,
                        size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed LESS THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element
    /// index 0
    SVRegister &m_s_lte(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed GREATER THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_s_gt(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed GREATER THAN for right-hand-side signed 64 bit value. rhs is sign extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_s_gt(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed GREATER THAN OR EQUAL for right-hand-side SVector.
    SVRegister &m_s_gte(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask,
                        size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked signed GREATER THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to
    /// element size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for
    /// element index 0
    SVRegister &m_s_gte(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned LESS THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_u_lt(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned LESS THAN for right-hand-side signed 64 bit value. rhs is zero extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_u_lt(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned LESS THAN OR EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_u_lte(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask,
                        size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned LESS THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is zero extended to
    /// element size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for
    /// element index 0
    SVRegister &m_u_lte(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask,
                        size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned GREATER THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_u_gt(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned GREATER THAN for right-hand-side signed 64 bit value. rhs is zero extended to element
    /// size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element
    /// index 0
    SVRegister &m_u_gt(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned GREATER THAN OR EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister &m_u_gte(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask,
                        size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked unsigned GREATER THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is zero extended to
    /// element size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for
    /// element index 0
    SVRegister &m_u_gte(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask,
                        size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce carry-out for right-hand-side SVector. If mask is false, use carry-in.
    /// \return A SVRegister, which each bit holds the carry-out of the relevant addition
    SVRegister &m_madc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce carry-out for right-hand-side signed 64 bit value. If mask is false, use carry-in.
    /// \return A SVRegister, which each bit holds the carry-out of the relevant addition
    SVRegister &m_madc(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce borrow-out for right-hand-side SVector. If mask is false, use borrow-in.
    /// \return A SVRegister, which each bit holds the borrow-out of the relevant subtraction
    SVRegister &m_msbc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Produce borrow-out for right-hand-side signed 64 bit value. If mask is false, use borrow-in.
    /// \return A SVRegister, which each bit holds the borrow-out of the relevant subtraction
    SVRegister &m_msbc(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded array subscript to return the indexed byte of memory as reference
    uint8_t &operator[](const size_t index) const { return ((mem_[index])); }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded assignment for right-hand-side SVRegister
    SVRegister &operator=(const SVRegister &rhs);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded assignment for right-hand-side signed 64 bit value
    SVRegister &operator=(const int64_t rhs);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Default Constructor for referenced main memory, i.e. externally allocated memory, init() has to be called
    /// before usage.
    SVRegister(void) : reference_mem_(true), length_bits_(0), mem_(nullptr) {}

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reference init method.
    void init_ref(const size_t length_bits, uint8_t *mem)
    {
        mem_ = mem;
        length_bits_ = length_bits;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for referenced main memory, i.e. externally allocated memory
    SVRegister(const size_t length_bits, uint8_t *mem) : reference_mem_(true), length_bits_(length_bits), mem_(mem) {}

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for internal main memory, i.e. internally allocated memory
    SVRegister(const size_t length_bits)
        : reference_mem_(false), length_bits_(length_bits), mem_(new uint8_t[length_bits / 8])
    {
        std::memset(mem_, 0, length_bits / 8);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Copy Constructor. Always internal memory. Copies values from other.
    SVRegister(const SVRegister &other)
        : reference_mem_(false), length_bits_(other.length_bits_), mem_(new uint8_t[other.length_bits_ / 8])
    {
        *this = other;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor. Deallocates internal memory if generated.
    virtual ~SVRegister(void)
    {
        if (!reference_mem_ && mem_)
        {
            delete[] mem_;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////
/// \class SVElement
/// \brief Softvector (SV) Element. Element size in multiples of 8-bit. Implements arithmetic on top.
class SVElement
{
    bool reference_mem_{
        true
    }; //!< If true the memory is allocated somewhere at a different context. Else SVElement allocates itself.
  public:
    const size_t width_in_bits_; //!< Element size (width) in bits
    uint8_t *mem_;               //!< Main memory

    auto to_i64() const -> int64_t;
    auto to_u64() const -> uint64_t;

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded array subscript to return the indexed byte of memory as reference
    uint8_t &operator[](const size_t index) const { return ((mem_[index])); }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded assignment for right-hand-side SVElement
    SVElement &operator=(const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded assignment for right-hand-side signed 64 bit value. Value is sign extended to element size
    SVElement &operator=(const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded prefix increment
    SVElement &operator++();
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded prefix decrement
    SVElement &operator--();
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded postfix increment. Returns copy of element.
    SVElement operator++(int);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded postfix decrement. Returns copy of element.
    SVElement operator--(int);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Makes two's complement of element, i.e. ~self + 1
    void twos_complement(void);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Makes inverse two's complement of element, i.e. ~self - 1
    void inv_twos_complement(void);

    /* ALU */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded ADD for right-hand-side SVElement
    SVElement operator+(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded ADD for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement operator+(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SUB for right-hand-side SVElement
    SVElement operator-(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement operator-(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise AND for right-hand-side SVElement
    SVElement operator&(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise AND for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement operator&(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise OR for right-hand-side SVElement
    SVElement operator|(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise OR for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement operator|(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise XOR for right-hand-side SVElement
    SVElement operator^(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise XOR for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement operator^(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SLL/SLA for right-hand-side SVElement.
    SVElement operator<<(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SLL/SLA for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement operator<<(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRA for right-hand-side SVElement.
    SVElement operator>>(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRA for right-hand-side unsigned 64 bit value. rhs is zero extended to element size
    SVElement operator>>(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRL for right-hand-side SVElement.
    SVElement srl(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRA for right-hand-side unsigned 64 bit value. rhs is zero extended to element size
    SVElement srl(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded EQUAL for right-hand-side SVElement.
    bool operator==(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded EQUAL for right-hand-side 64 bit value. rhs is sign extended to element size
    bool operator==(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded NOT EQUAL for right-hand-side SVElement.
    bool operator!=(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded NOT EQUAL for right-hand-side 64 bit value. rhs is sign extended to element size
    bool operator!=(const int64_t rhs) const;
    // signed comparisons
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN for right-hand-side SVElement.
    bool operator<(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN for right-hand-side 64 bit value. rhs is sign extended to element size
    bool operator<(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN OR EQUAL for right-hand-side SVElement.
    bool operator<=(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN OR EQUAL for right-hand-side 64 bit value. rhs is sign extended to element
    /// size
    bool operator<=(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN for right-hand-side SVElement.
    bool operator>(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN for right-hand-side 64 bit value. rhs is sign extended to element size
    bool operator>(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN OR EQUAL for right-hand-side SVElement.
    bool operator>=(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN OR EQUAL for right-hand-side 64 bit value. rhs is sign extended to element
    /// size
    bool operator>=(const int64_t rhs) const;
    // unsigned comparisons
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned LESS THAN for right-hand-side SVElement.
    bool op_u_lt(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned LESS THAN for right-hand-side 64 bit value. rhs is zero extended to element size
    bool op_u_lt(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned LESS THAN OR EQUAL for right-hand-side SVElement.
    bool op_u_lte(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned LESS THAN OR EQUAL for right-hand-side 64 bit value. rhs is zero extended to element
    /// size
    bool op_u_lte(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned GREATER THAN for right-hand-side SVElement.
    bool op_u_gt(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned GREATER THAN for right-hand-side 64 bit value. rhs is zero extended to element size
    bool op_u_gt(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned GREATER THAN OR EQUAL for right-hand-side SVElement.
    bool op_u_gte(const SVElement &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Operation unsigned GREATER THAN OR EQUAL for right-hand-side 64 bit value. rhs is zero extended to
    /// element size
    bool op_u_gte(const uint64_t rhs) const;

    // (self assign) operations
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief ADD and assign for right-hand-side SVElement
    SVElement &s_add(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief ADD and assign for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_add(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SUB and assign for right-hand-side SVElement
    SVElement &s_sub(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_sub(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Reverse-SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_rsub(const int64_t lhs, const SVElement &opR);

    // 11.2. Vector Widening Integer Add/Subtract
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned ADD and assign for right-hand-side SVElement
    SVElement &s_waddu(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned ADD and assign for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVElement &s_waddu(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned SUB and assign for right-hand-side SVElement
    SVElement &s_wsubu(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_wsubu(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed ADD and assign for right-hand-side SVElement
    SVElement &s_wadd(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed ADD and assign for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVElement &s_wadd(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed SUB and assign for right-hand-side SVElement
    SVElement &s_wsub(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_wsub(const SVElement &opL, const int64_t rhs);
    // End 11.2.

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief bit-wise AND and assign for right-hand-side SVElement
    SVElement &s_and(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief bit-wise AND and assign for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_and(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief bit-wise OR and assign for right-hand-side SVElement
    SVElement &s_or(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief bit-wise OR and assign for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_or(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief bit-wise XOR and assign for right-hand-side SVElement
    SVElement &s_xor(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief bit-wise XOR and assign for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVElement &s_xor(const SVElement &opL, const int64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SLL/SLA and assign for right-hand-side SVElement
    SVElement &s_sll(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SLL/SLA and assign for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVElement &s_sll(const SVElement &opL, const uint64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRA and assign for right-hand-side SVElement
    SVElement &s_sra(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRA and assign for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVElement &s_sra(const SVElement &opL, const uint64_t rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRL and assign for right-hand-side SVElement
    SVElement &s_srl(const SVElement &opL, const SVElement &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief SRL and assign for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVElement &s_srl(const SVElement &opL, const uint64_t rhs);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief signed*Signed Multiplikation for right-hand-side SVElement low part of product
    SVElement &s_ssmul(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief signed*Signed Multiplikation for right-hand-side 64 bit value low part of product
    SVElement &s_ssmul(const SVElement &opL, const int64_t rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief signed*Signed Multiplikation for right-hand-side SVElement high part of product
    SVElement &s_ssmulh(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief signed*Signed Multiplikation for right-hand-side 64 bit value high part of product
    SVElement &s_ssmulh(const SVElement &opL, const int64_t rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Unsigned*Unsigned Multiplikation for right-hand-side SVElement high part of product
    SVElement &s_uumulh(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Unsigned*Unsigned Multiplikation for right-hand-side 64 bit value high part of product
    SVElement &s_uumulh(const SVElement &opL, const int64_t rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed*Unsigned Multiplikation for right-hand-side SVElement high part product
    SVElement &s_sumulh(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed*Unsigned Multiplikation for right-hand-side 64 bit value high part of product
    SVElement &s_sumulh(const SVElement &opL, const int64_t rhs);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed division for right-hand-side SVElement
    SVElement &s_ssdiv(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed division for right-hand-side 64 bit value
    SVElement &s_ssdiv(const SVElement &opL, const int64_t rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed division for right-hand-side SVElement
    SVElement &s_uudiv(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed division for right-hand-side 64 bit value
    SVElement &s_uudiv(const SVElement &opL, const uint64_t rhs);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed remainder for right-hand-side SVElement
    SVElement &s_ssrem(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed remainder for right-hand-side 64 bit value
    SVElement &s_ssrem(const SVElement &opL, const int64_t rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed remainder for right-hand-side SVElement
    SVElement &s_uurem(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed/signed remainder for right-hand-side 64 bit value
    SVElement &s_uurem(const SVElement &opL, const uint64_t rhs);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Unsigned*unsigned multiplication for right-hand-side SVElement
    SVElement &s_uumul(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Unsigned*unsigned multiplication for right-hand-side 64 bit value
    SVElement &s_uumul(const SVElement &opL, const uint64_t rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed*unsigned multiplication for right-hand-side SVElement
    SVElement &s_sumul(const SVElement &opL, const SVElement &rhs);
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //// \brief Signed*unsigned multiplication for right-hand-side 64 bit value
    SVElement &s_sumul(const SVElement &opL, const uint64_t rhs);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for referenced main memory, i.e. externally allocated memory
    SVElement(const size_t width_in_bits, uint8_t *mem_)
        : reference_mem_(true), width_in_bits_(width_in_bits), mem_(mem_)
    {
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for internal main memory, i.e. internally allocated memory
    SVElement(const size_t width_in_bits)
        : reference_mem_(false), width_in_bits_(width_in_bits), mem_(new uint8_t[width_in_bits / 8])
    {
        std::memset(mem_, 0, width_in_bits / 8);
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Copy Constructor. Always internal memory. Copies values from other.
    SVElement(const SVElement &other)
        : reference_mem_(false), width_in_bits_(other.width_in_bits_), mem_(new uint8_t[other.width_in_bits_ / 8])
    {
        for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
        {
            (*this)[i_byte] = other[i_byte];
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor. Deallocates internal memory if generated.
    virtual ~SVElement(void)
    {
        if (!reference_mem_ && mem_)
        {
            delete[] mem_;
        }
    }
};

//////////////////////////////////////////////////////////////////////////////////////
/// \class SVector
/// \brief Softvector (SV) Vector. Vector length in multiples SVElement. Implements arithmetic on top
class SVector
{
    bool reference_mem_{
        true
    }; //!< If true the memory is allocated somewhere at a different context. Else SVector allocates itself.
  public:
    std::vector<SVElement *> elements_; //!< Elements (main memory)
    const size_t length_;               //!< Vector length in elements
    const size_t start_reg_index_;      //!< If build upon a SVRegister, this holds the register number

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Assignment. All elements before start_index are not updated
    void assign(const SVector &vin, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded array subscript to return the indexed SVElement as reference
    virtual SVElement &operator[](const size_t index) const { return (*(elements_[index])); }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded array subscript to return the indexed element as reference
    SVector &operator=(const SVector &rhs);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded ADD for right-hand-side SVector
    SVector operator+(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded ADD for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector operator+(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SUB for right-hand-side SVector
    SVector operator-(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector operator-(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise AND for right-hand-side SVector
    SVector operator&(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise AND for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector operator&(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise OR for right-hand-side SVector
    SVector operator|(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise OR for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector operator|(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise XOR for right-hand-side SVector
    SVector operator^(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded bit-wise XOR for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector operator^(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SLL/SLA for right-hand-side SVector
    SVector operator<<(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SLL/SLA for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector operator<<(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRA for right-hand-side SVector
    SVector operator>>(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRA for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector operator>>(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRL for right-hand-side SVector
    SVector srl(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded SRL for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector srl(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator==(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator==(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded NOT EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator!=(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded NOT EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator!=(const int64_t rhs) const;
    // signed comparisons
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator<(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN for right-hand-side signed 64 bit value. rhs is sign extended to element size
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator<(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN OR EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator<=(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed LESS THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to
    /// element size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for
    /// element index 0
    SVRegister operator<=(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister operator>(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element
    /// index 0
    SVRegister operator>(const int64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN OR EQUAL for right-hand-side SVector.
    SVRegister operator>=(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded signed GREATER THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is sign extended to
    /// element size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for
    /// element index 0
    SVRegister operator>=(const int64_t rhs) const;
    // unsigned comparisons
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned LESS THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister op_u_lt(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned LESS THAN for right-hand-side signed 64 bit value. rhs is zero extended to element
    /// size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element
    /// index 0
    SVRegister op_u_lt(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned LESS THAN OR EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister op_u_lte(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned LESS THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is zero extended to
    /// element size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for
    /// element index 0
    SVRegister op_u_lte(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned GREATER THAN for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister op_u_gt(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned GREATER THAN for right-hand-side signed 64 bit value. rhs is zero extended to element
    /// size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element
    /// index 0
    SVRegister op_u_gt(const uint64_t rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned GREATER THAN OR EQUAL for right-hand-side SVector.
    /// \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB for element index
    /// 0
    SVRegister op_u_gte(const SVector &rhs) const;
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded unsigned GREATER THAN OR EQUAL for right-hand-side signed 64 bit value. rhs is zero extended
    /// to element size \return A SVRegister, which each bit holds the on-element logic comparison's result. I.e. LSB
    /// for element index 0
    SVRegister op_u_gte(const uint64_t rhs) const;

    // masked (self assign) operations
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked assignment. Updates elements only where mask register's respective bit is true. All elements
    /// before start_index are not updated
    void m_assign(const SVector &vin,    //!< Input Vector
                  const SVRegister &vm,  //!< Mask register
                  bool mask,             //!< Mask flag
                  size_t start_index = 0 //!< Start (bit) index
    );
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked assign/copy/move for right-hand-side signed 64 bit value. rhs is sign extended to element size
    void m_assign(const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked ADD for right-hand-side SVector
    SVector &m_add(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked ADD for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_add(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SUB for right-hand-side SVector
    SVector &m_sub(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_sub(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked Reverse-SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_rsub(const int64_t lhs, const SVector &opR, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned Masked ADD for right-hand-side SVector
    SVector &m_waddu(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned Masked ADD for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_waddu(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned Masked SUB for right-hand-side SVector
    SVector &m_wsubu(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening unsigned Masked SUB for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_wsubu(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed Masked ADD for right-hand-side SVector
    SVector &m_wadd(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed Masked ADD for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_wadd(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed Masked SUB for right-hand-side SVector
    SVector &m_wsub(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Widening signed Masked SUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_wsub(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    // End 11.2.

    // 11.3. Vector Integer Extension
    SVector &m_vext(const SVector &opL, const SVRegister &vm, bool mask, bool sign, size_t start_index = 0);
    // End 11.3.

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked bit-wise AND for right-hand-side SVector
    SVector &m_and(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked bit-wise AND for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_and(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked bit-wise OR for right-hand-side SVector
    SVector &m_or(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked bit-wise OR for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_or(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked bit-wise XOR for right-hand-side SVector
    SVector &m_xor(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked bit-wise XOR for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_xor(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SLL/SLA for right-hand-side SVector
    SVector &m_sll(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SLL/SLA for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector &m_sll(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SRA for right-hand-side SVector
    SVector &m_sra(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SRA for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector &m_sra(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SRL for right-hand-side SVector
    SVector &m_srl(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked SRL for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector &m_srl(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    // 11.7. Vector Narrowing Integer Right Shift Instructions
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked narrowing SRA for right-hand-side SVector
    SVector &m_nsra(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked narrowing SRA for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector &m_nsra(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked narrowing SRL for right-hand-side SVector
    SVector &m_nsrl(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked narrowing SRL for right-hand-side signed 64 bit value. rhs is zero extended to element size
    SVector &m_nsrl(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    // End 11.7.

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Slide the Vector elements up by right-hand-side unsigned 64 bit value. rhs is zero extended to element
    /// size
    SVector &m_slideup(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Slide the Vector elements down by right-hand-side unsigned 64 bit value. rhs is zero extended to element
    /// size
    SVector &m_slidedown(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t vlmax,
                         size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*signed MUL for right-hand-side SVector
    SVector &m_ssmul(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*signed MUL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_ssmul(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*signed MUL for right-hand-side SVector
    SVector &m_uumul(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*signed MUL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_uumul(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*signed MUL for right-hand-side SVector
    SVector &m_ssmulh(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*signed MUL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_ssmulh(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned*unsigned MUL for right-hand-side SVector
    SVector &m_uumulh(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned*unsigned MUL for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_uumulh(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned*unsigned MUL for right-hand-side SVector
    SVector &m_sumulh(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned*unsigned MUL for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_sumulh(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*unsigned MUL for right-hand-side SVector
    SVector &m_sumul(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed*unsigned MUL for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_sumul(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed DIV for right-hand-side SVector
    SVector &m_ssdiv(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed DIV for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_ssdiv(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned DIV for right-hand-side SVector
    SVector &m_uudiv(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned DIV for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_uudiv(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed REM for right-hand-side SVector
    SVector &m_ssrem(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed REM for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_ssrem(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned REM for right-hand-side SVector
    SVector &m_uurem(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned REM for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_uurem(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed MAX for right-hand-side SVector
    SVector &m_ssmax(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed MAX for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_ssmax(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned MAX for right-hand-side SVector
    SVector &m_uumax(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned MAX for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_uumax(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed MIN for right-hand-side SVector
    SVector &m_ssmin(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked signed/signed MIN for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_ssmin(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned MIN for right-hand-side SVector
    SVector &m_uumin(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief masked unsigned/unsigned MIN for right-hand-side signed 64 bit value. rhs is sign extended to element
    /// size
    SVector &m_uumin(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);

    // 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry for right-hand-side SVector.
    SVector &m_adc(const SVector &opL, const SVector &rhs, const SVRegister &vm, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Sum with carry for right-hand-side signed 64 bit value.
    SVector &m_adc(const SVector &opL, const int64_t rhs, const SVRegister &vm, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Difference with borrow for right-hand-side SVector.
    SVector &m_sbc(const SVector &opL, const SVector &rhs, const SVRegister &vm, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Difference with borrow for right-hand-side signed 64 bit value.
    SVector &m_sbc(const SVector &opL, const int64_t rhs, const SVRegister &vm, size_t start_index = 0);
    /* End 11.4. */

    /* 11.13. Vector Single-Width Integer Multiply-Add Instructions */
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed-signed MACC for right-hand-side SVector
    SVector &m_ssmacc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed-signed MACC for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_ssmacc(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSAC for right-hand-side SVector
    SVector &m_nmsac(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSAC for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_nmsac(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief MADD for right-hand-side SVector
    SVector &m_madd(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief MADD for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_madd(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSUB for right-hand-side SVector
    SVector &m_nmsub(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief NMSUB for right-hand-side signed 64 bit value. rhs is sign extended to element size
    SVector &m_nmsub(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /* End 11.13. */

    /* 11.14. Vector Widening Integer Multiply-Add Instructions */
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned-unsigned MACC for right-hand-side SVector
    SVector &m_uumacc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned-unsigned MACC for right-hand-side unsigned 64 bit value. rhs is sign extended to element size
    SVector &m_uumacc(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed(vs1)-unsigned(vs2) MACC for right-hand-side SVector
    SVector &m_sumacc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Signed(rs1)-unsigned(vs2) MACC for right-hand-side signed 64 bit value.
    /// \brief rhs is sign extended to element size
    SVector &m_sumacc(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Unsigned(rs1)-Signed(vs2) MACC for right-hand-side unsigned 64 bit value.
    /// \brief rhs is sign extended to element size
    SVector &m_usmacc(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    /* End 11.14. */

    /* */
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Vector merge for right-hand-side vector.
    SVector &m_merge(const SVector &opL, const SVector &rhs, const SVRegister &vm, size_t start_index = 0);
    /////////////////////////////////////////////////////////////////////////////////////////////////////////////
    /// \brief Vector merge for right-hand-side 64 bit value.
    SVector &m_merge(const SVector &opL, const int64_t rhs, const SVRegister &vm, size_t start_index = 0);
    /* End 11.15. */

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for referenced elements, i.e. externally allocated elements
    SVector(const size_t length_elements, const size_t start_reg_index)
        : reference_mem_(true), elements_(), length_(length_elements), start_reg_index_(start_reg_index)
    {
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for referenced elements, i.e. externally allocated elements
    SVector(std::vector<SVElement *> &elements, const size_t start_reg_index)
        : reference_mem_(true), elements_(elements), length_(elements.size()), start_reg_index_(start_reg_index)
    {
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for referenced memory but missing element interpretation, i.e. externally allocated memory
    /// space
    SVector(const size_t length_elements,           //!< VL
            const size_t single_element_width_bits, //!< SEW
            const size_t start_reg_index,           //!< Index of first register associated with this vector
            uint8_t *mem                            //!< Start adress of vector
            )
        : reference_mem_(false), elements_(), length_(length_elements), start_reg_index_(start_reg_index)
    {
        for (size_t i = 0; i < length_; ++i)
        {
            elements_.push_back(new SVElement(single_element_width_bits, mem));
            mem += single_element_width_bits / 8;
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for internal elements, thus, memory, i.e. internally allocated memory
    SVector(const size_t length_elements,           //!< VL
            const size_t single_element_width_bits, //!< SEW
            const size_t start_reg_index            //!< Index of first register associated with this vector
            )
        : reference_mem_(false), elements_(), length_(length_elements), start_reg_index_(start_reg_index)
    {
        for (size_t i = 0; i < length_; ++i)
        {
            elements_.push_back(new SVElement(single_element_width_bits));
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Copy Constructor. Always internal elements, thus, memory. Copies values from other element-wise.
    SVector(const SVector &other)
        : reference_mem_(other.reference_mem_), length_(other.length_), start_reg_index_(other.start_reg_index_)
    {
        for (size_t i = 0; i < length_; ++i)
        {
            elements_.push_back(new SVElement(other.elements_[0]->width_in_bits_));
            (*this)[i] = other[i];
        }
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor. Deallocates internal elements if generated.
    virtual ~SVector(void)
    {
        if (!reference_mem_)
        {
            for (auto &it : elements_)
            {
                delete it;
            }
        }
    }
};

#endif /* __SOFTVECTOR_TYPES_H__ */
