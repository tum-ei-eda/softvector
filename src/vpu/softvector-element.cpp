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
/// \file softvector-element.cpp
/// \brief Extended element type for softvector representation
/// \date 07/03/2020
//////////////////////////////////////////////////////////////////////////////////////

#include <cstring>

#include "vpu/softvector-types.hpp"

auto SVElement::to_i64() const -> int64_t
{
    auto width_in_bytes = width_in_bits_ >> 3;
    auto sign_bit = mem_[width_in_bytes - 1] >> 7;
    int64_t value = sign_bit ? -1 : 0;
    std::memcpy(&value, mem_, width_in_bytes);
    return value;
}

auto SVElement::to_u64() const -> uint64_t
{
    uint64_t value = 0U;
    std::memcpy(&value, mem_, width_in_bits_ >> 3);
    return value;
}

auto SVElement::msb_is_set() const -> bool
{
    auto width_in_bytes = width_in_bits_ >> 3;
    return mem_[width_in_bytes - 1] >> 7;
}

auto SVElement::get_max_signed() const -> int64_t
{
    return (1 << (width_in_bits_ - 1)) - 1;
}

auto SVElement::get_max_unsigned() const -> uint64_t
{
    return ((1 << (width_in_bits_ - 1)) - 1) | (1 << (width_in_bits_ - 1));
}

auto SVElement::set_max_signed() const -> void
{
    auto width_in_bytes = width_in_bits_ >> 3;
    for (size_t i = 0; i < width_in_bytes - 1; i++)
    {
        mem_[i] = -1;
    }
    mem_[width_in_bytes - 1] = 0x7F;
}

auto SVElement::set_min_signed() const -> void
{
    auto width_in_bytes = width_in_bits_ >> 3;
    for (size_t i = 0; i < width_in_bytes - 1; i++)
    {
        mem_[i] = 0;
    }
    mem_[width_in_bytes - 1] = 0x80;
}

inline size_t get_shiftamount(size_t target_width_bits, const uint8_t *rhs)
{
    size_t numberofbits = 0;
    size_t shiftamount = 0;
    target_width_bits >>= 1;
    while (target_width_bits)
    {
        target_width_bits = target_width_bits >> 1;
        shiftamount |= rhs[numberofbits / 8] & (1 << numberofbits);
        numberofbits++;
    }
    return (shiftamount);
}

inline size_t get_shiftamount(size_t target_width_bits, uint64_t rhs)
{
    size_t numberofbits = 0;
    size_t shiftamount = 0;
    target_width_bits >>= 1;
    while (target_width_bits)
    {
        target_width_bits = target_width_bits >> 1;
        shiftamount |= rhs & (1 << numberofbits);
        numberofbits++;
    }
    return (shiftamount);
}

inline uint8_t arr_shift_left(uint8_t *dat, size_t size)
{
    uint8_t cin = 0;
    for (size_t i = 0; i < size; ++i)
    {
        uint8_t cout = (dat[i] & 0x80) ? 0x1 : 0;
        dat[i] = cin | (dat[i] << 1);
        cin = cout;
    }
    return (cin);
}

inline uint8_t arr_shift_right(uint8_t *dat, size_t size, bool arith = false)
{
    uint8_t msb = dat[size - 1] & 0x80;
    uint8_t cin = arith ? msb : 0;
    for (int i = size - 1; i >= 0; --i)
    {
        uint8_t cout = (dat[i] & 0x01) ? 0x80 : 0;
        dat[i] = cin | (dat[i] >> 1);
        cin = cout;
    }
    return (cin);
}

SVElement &SVElement::operator=(const SVElement &rhs)
{
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        (*this)[i_byte] = rhs[i_byte];
    }
    return (*this);
}

SVElement &SVElement::operator=(const int64_t rhs)
{
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        if (i_byte < 8)
            (*this)[i_byte] = 0xFF & (rhs >> 8 * i_byte);
        else
            (*this)[i_byte] = (rhs >= 0) ? 0 : 0xFF;
    }
    return (*this);
}

SVElement &SVElement::operator++()
{
    *this = *this + 1;
    return (*this);
}

SVElement SVElement::operator++(int)
{
    SVElement temp(*this);
    *this = *this + 1;
    return (temp);
}

SVElement &SVElement::operator--()
{
    *this = *this + (-1);
    return (*this);
}

SVElement SVElement::operator--(int)
{
    SVElement temp(*this);
    *this = *this + (-1);
    return (temp);
}

void SVElement::twos_complement(void)
{

    for (size_t i = 0; i < width_in_bits_ / 8; ++i)
    {
        mem_[i] = ~(mem_[i]);
    }
    ++(*this);
}

void SVElement::inv_twos_complement(void)
{

    for (size_t i = 0; i < width_in_bits_ / 8; ++i)
    {
        mem_[i] = ~(mem_[i]);
    }
    ++(*this);
}

SVElement SVElement::operator+(const SVElement &rhs) const
{
    SVElement ret(width_in_bits_);
    uint8_t carry = 0;
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = (*this)[i_byte] + rhs[i_byte] + carry;
        carry = (x & (0xFF00)) ? 1 : 0;
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator+(const int64_t rhs) const
{
    SVElement ret(width_in_bits_);
    uint8_t carry = 0;
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = 0;
        if (i_byte < 8)
            x = (*this)[i_byte] + (0xFF & (rhs >> 8 * i_byte)) + carry;
        else
            x = (*this)[i_byte] + ((rhs >= 0) ? 0 : 0xFF) + carry;
        carry = (x & 0xFF00) ? 1 : 0;
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator-(const SVElement &rhs) const
{
    SVElement ret(width_in_bits_);
    SVElement twos(rhs);
    twos.twos_complement();
    uint8_t carry = 0;
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = (*this)[i_byte] + twos[i_byte] + carry;
        carry = (x & (0xFF00)) ? 1 : 0;
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator-(const int64_t rhs) const
{
    SVElement ret(width_in_bits_);
    SVElement twos(width_in_bits_);
    twos = rhs;
    return (*this - twos);
}

SVElement SVElement::operator&(const SVElement &rhs) const
{
    SVElement ret(width_in_bits_);
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = (*this)[i_byte] & rhs[i_byte];
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator&(const int64_t rhs) const
{
    SVElement ret(width_in_bits_);
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = 0;
        if (i_byte < 8)
            x = (*this)[i_byte] & (0xFF & (rhs >> 8 * i_byte));
        else
            x = (*this)[i_byte] & ((rhs >= 0) ? 0 : 0xFF);
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator|(const SVElement &rhs) const
{
    SVElement ret(width_in_bits_);
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = (*this)[i_byte] | rhs[i_byte];
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator|(const int64_t rhs) const
{
    SVElement ret(width_in_bits_);
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = 0;
        if (i_byte < 8)
            x = (*this)[i_byte] | (0xFF & (rhs >> 8 * i_byte));
        else
            x = (*this)[i_byte] | ((rhs >= 0) ? 0 : 0xFF);
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator^(const SVElement &rhs) const
{
    SVElement ret(width_in_bits_);
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = (*this)[i_byte] ^ rhs[i_byte];
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator^(const int64_t rhs) const
{
    SVElement ret(width_in_bits_);
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint16_t x = 0;
        if (i_byte < 8)
            x = (*this)[i_byte] ^ (0xFF & (rhs >> 8 * i_byte));
        else
            x = (*this)[i_byte] ^ ((rhs >= 0) ? 0 : 0xFF);
        ret[i_byte] = static_cast<uint8_t>(x);
    }
    return (ret);
}

SVElement SVElement::operator<<(const SVElement &rhs) const
{
    SVElement ret(*this);
    size_t shiftamount = get_shiftamount(width_in_bits_, &(rhs[0]));
    while (shiftamount--)
    {
        arr_shift_left(&(ret[0]), width_in_bits_ / 8);
    }
    return (ret);
}

SVElement SVElement::operator<<(const uint64_t rhs) const
{
    SVElement ret(*this);
    size_t shiftamount = get_shiftamount(width_in_bits_, rhs);
    while (shiftamount--)
    {
        arr_shift_left(&(ret[0]), width_in_bits_ / 8);
    }
    return (ret);
}

SVElement SVElement::operator>>(const SVElement &rhs) const
{
    SVElement ret(*this);
    size_t shiftamount = get_shiftamount(width_in_bits_, &(rhs[0]));
    while (shiftamount--)
    {
        arr_shift_right(&(ret[0]), width_in_bits_ / 8, true);
    }
    return (ret);
}

SVElement SVElement::operator>>(const uint64_t rhs) const
{
    SVElement ret(*this);
    size_t shiftamount = get_shiftamount(width_in_bits_, rhs);
    while (shiftamount--)
    {
        arr_shift_right(&(ret[0]), width_in_bits_ / 8, true);
    }
    return (ret);
}

SVElement SVElement::srl(const SVElement &rhs) const
{
    SVElement ret(*this);
    size_t shiftamount = get_shiftamount(width_in_bits_, &(rhs[0]));
    while (shiftamount--)
    {
        arr_shift_right(&(ret[0]), width_in_bits_ / 8, false);
    }
    return (ret);
}

SVElement SVElement::srl(const uint64_t rhs) const
{
    SVElement ret(*this);
    size_t shiftamount = get_shiftamount(width_in_bits_, rhs);
    while (shiftamount--)
    {
        arr_shift_right(&(ret[0]), width_in_bits_ / 8, false);
    }
    return (ret);
}

bool SVElement::operator==(const SVElement &rhs) const
{
    bool ret = true;
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        if ((*this)[i_byte] != rhs[i_byte])
        {
            ret = false;
            break;
        }
    }
    return (ret);
}

bool SVElement::operator==(const int64_t rhs) const
{
    bool ret = true;
    for (size_t i_byte = 0; i_byte < width_in_bits_ / 8; ++i_byte)
    {
        uint8_t x = (i_byte < 8) ? (rhs >> i_byte * 8) : ((rhs >= 0) ? 0x00 : 0xFF);
        if ((*this)[i_byte] != x)
        {
            ret = false;
            break;
        }
    }
    return (ret);
}

bool SVElement::operator!=(const SVElement &rhs) const
{
    return (!((*this) == rhs));
}

bool SVElement::operator!=(const int64_t rhs) const
{
    return (!((*this) == rhs));
}

// signed comparisons
bool SVElement::operator<(const SVElement &rhs) const
{
    SVElement x(*this);
    x = *this - rhs;
    if (x[x.width_in_bits_ / 8 - 1] & 0x80)
        return (true);
    else
        return (false);
}

bool SVElement::operator<(const int64_t rhs) const
{
    SVElement x(*this);
    x = *this - rhs;
    if (x[x.width_in_bits_ / 8 - 1] & 0x80)
        return (true);
    else
        return (false);
}

bool SVElement::operator<=(const SVElement &rhs) const
{
    return (!(*this > rhs));
}
bool SVElement::operator<=(const int64_t rhs) const
{
    return (!(*this > rhs));
}

bool SVElement::operator>(const SVElement &rhs) const
{
    return (rhs < *this);
}

bool SVElement::operator>(const int64_t rhs) const
{
    SVElement x(*this);
    x = *this - rhs;
    if (x[x.width_in_bits_ / 8 - 1] & 0x80)
        return (false);
    else
        return (true);
}

bool SVElement::operator>=(const SVElement &rhs) const
{
    return (!(*this < rhs));
}

bool SVElement::operator>=(const int64_t rhs) const
{
    return (!(*this < rhs));
}