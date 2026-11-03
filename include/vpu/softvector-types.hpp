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
                 [[maybe_unused]]bool val = true)
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
    auto msb_is_set() const -> bool;
    auto get_max_unsigned() const -> uint64_t;
    auto get_max_signed() const -> int64_t;
    auto set_max_signed() const -> void;
    auto set_min_signed() const -> void;

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

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Slide the Vector elements up by right-hand-side unsigned 64 bit value. rhs is zero extended to element
    /// size
    SVector &m_slideup(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Slide the Vector elements down by right-hand-side unsigned 64 bit value. rhs is zero extended to element
    /// size
    SVector &m_slidedown(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t vlmax,
                         size_t start_index = 0);

    /* 16.4. Vector Register Gather Instructions */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked VRGATHER(EI16) for right-hand-side SVector.
    SVector &m_vrgather(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t vlmax,
                        size_t start_index = 0);
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Masked VRGATHER for right-hand-side 64 bit value.
    SVector &m_vrgather(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t vlmax,
                        size_t start_index = 0);
    /* End 16.4 */

    /* 16.5. Vector Compress Instruction */
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief VCOMPRESS for right-hand-side SVector.
    SVector &m_vcompress(const SVector &opL, const SVRegister &vm, size_t start_index = 0);
    /* End 16.5. Vector Compress Instruction */

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
