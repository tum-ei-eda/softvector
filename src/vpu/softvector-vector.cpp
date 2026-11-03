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
/// \file softvector-vector.cpp
/// \brief Extended vector type for softvector representation
/// \date 07/03/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "vpu/softvector-types.hpp"
#include <cassert>
#include <cstdint>
#include <cstddef>

namespace Deprecated
{

auto roundoff_unsigned(uint64_t value, uint8_t rounding_bits, uint8_t rounding_mode) -> uint64_t;

auto roundoff_signed(int64_t value, uint8_t rounding_bits, uint8_t rounding_mode) -> int64_t;

auto roundoff_unsigned(uint64_t value, uint8_t rounding_bits, uint8_t rounding_mode) -> uint64_t
{
    // Only lower 2 bits are used
    rounding_mode &= 0b11;

    if (rounding_bits == 0)
    {
        return value;
    }
    auto rounding_increment = false;
    auto range_zero_check = false;
    auto bitmask = 0U;

    switch (rounding_mode)
    {
    case 0:
    {
        rounding_increment = static_cast<bool>(value & (1U << (rounding_bits - 1)));
        break;
    }
    case 1:
    {
        // Needs check v[d-2:0] != 0
        if (rounding_bits >= 2)
        {
            // Bitmask for v[d-2 : 0]
            bitmask = (1 << (rounding_bits - 1)) - 1;
            range_zero_check = value & bitmask;
        }
        // v[d-1] & (v[d-2:0] != 0 | v[d])
        bool condition_1 = (value & (1 << (rounding_bits - 1)));
        bool condition_2 = static_cast<bool>(range_zero_check || (value & (1 << rounding_bits)));
        rounding_increment = condition_1 && condition_2;
        break;
    }
    case 2:
    {
        // rounding_increment = 0;
        break;
    }
    case 3:
    {
        // Bitmask for v[d-1 : 0]
        bitmask = (1 << (rounding_bits)) - 1;
        // Needs check v[d-1:0] != 0
        range_zero_check = value & bitmask;
        rounding_increment = !static_cast<bool>(value & (1 << rounding_bits)) && range_zero_check;
        break;
    }
    default:
    {
        // Illegal!
        break;
    }
    }

    return (value >> rounding_bits) + rounding_increment;
}

auto roundoff_signed(int64_t value, uint8_t rounding_bits, uint8_t rounding_mode) -> int64_t
{
    if (rounding_bits == 0)
    {
        return value;
    }

    // Only lower 2 bits are used
    rounding_mode &= 0b11;
    auto range_zero_check = false;
    int64_t bitmask = 0;

    auto rounding_increment = false;
    switch (rounding_mode)
    {
    case 0:
    {
        rounding_increment = static_cast<bool>(value & (1 << (rounding_bits - 1)));
        break;
    }
    case 1:
    {
        // Needs check v[d-2:0] != 0
        if (rounding_bits >= 2)
        {
            // Bitmask for v[d-2 : 0]
            bitmask = (1 << (rounding_bits - 1)) - 1;
            range_zero_check = value & bitmask;
        }
        // v[d-1] & (v[d-2:0] != 0 | v[d])
        bool condition_1 = (value & (1 << (rounding_bits - 1)));
        bool condition_2 = static_cast<bool>(range_zero_check || (value & (1 << rounding_bits)));
        rounding_increment = condition_1 && condition_2;
        break;
    }
    case 2:
    {
        // rounding_increment = 0;
        break;
    }
    case 3:
    {
        // Bitmask for v[d-1 : 0]
        bitmask = (1 << (rounding_bits)) - 1;
        // Needs check v[d-1:0] != 0
        range_zero_check = value & bitmask;
        rounding_increment = !static_cast<bool>(value & (1 << rounding_bits)) && range_zero_check;
        break;
    }
    default:
    {
        // Illegal!
        break;
    }
    }

    return (value >> rounding_bits) + rounding_increment;
}
} // namespace Deprecated

void SVector::assign(const SVector &vin, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        (*this)[i_element] = vin[i_element];
    }
}

SVector &SVector::operator=(const SVector &rhs)
{
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        (*this)[i_element] = rhs[i_element];
    }
    return (*this);
}

SVector SVector::operator+(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] + rhs[i_element];
    }
    return (ret);
}

SVector SVector::operator+(const int64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] + rhs;
    }
    return (ret);
}

SVector SVector::operator&(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] & rhs[i_element];
    }
    return (ret);
}

SVector SVector::operator&(const int64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] & rhs;
    }
    return (ret);
}

SVector SVector::operator|(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] | rhs[i_element];
    }
    return (ret);
}

SVector SVector::operator|(const int64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] | rhs;
    }
    return (ret);
}

SVector SVector::operator^(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] ^ rhs[i_element];
    }
    return (ret);
}

SVector SVector::operator^(const int64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] ^ rhs;
    }
    return (ret);
}

SVector SVector::operator-(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] - rhs[i_element];
    }
    return (ret);
}

SVector SVector::operator-(const int64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] - rhs;
    }
    return (ret);
}

SVector SVector::operator<<(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] << rhs[i_element];
    }
    return (ret);
}

SVector SVector::operator<<(const uint64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] << rhs;
    }
    return (ret);
}

SVector SVector::operator>>(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] >> rhs[i_element];
    }
    return (ret);
}

SVector SVector::operator>>(const uint64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element] >> rhs;
    }
    return (ret);
}

SVector SVector::srl(const SVector &rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element].srl(rhs[i_element]);
    }
    return (ret);
}

SVector SVector::srl(const uint64_t rhs) const
{
    SVector ret(length_, elements_[0]->width_in_bits_, start_reg_index_);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        ret[i_element] = (*this)[i_element].srl(rhs);
    }
    return (ret);
}

SVRegister SVector::operator==(const SVector &rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] == rhs[i_element])
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator==(const int64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] == rhs)
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator!=(const SVector &rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] != rhs[i_element])
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator!=(const int64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] != rhs)
            ret.toggle_bit(i_element);
    }
    return (ret);
}

// signed comparisons
SVRegister SVector::operator<(const SVector &rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] < rhs[i_element])
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator<(const int64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] < rhs)
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator<=(const SVector &rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] <= rhs[i_element])
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator<=(const int64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] <= rhs)
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator>(const SVector &rhs) const
{
    return (rhs < *this);
}

SVRegister SVector::operator>(const int64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] > rhs)
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::operator>=(const SVector &rhs) const
{
    return (rhs <= *this);
}

SVRegister SVector::operator>=(const int64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element] >= rhs)
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVector &SVector::m_slideup(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    size_t max = rhs > start_index ? rhs : start_index;
    for (size_t i_element = max; i_element < length_; ++i_element)
    {
        // i_element >= max -> i_element - max >= 0
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element] = opL[i_element - max];
    }
    return (*this);
}

SVector &SVector::m_slidedown(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t vlmax,
                              size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            size_t i_src_element = i_element + rhs;
            if (i_src_element < length_)
            {
                (*this)[i_element] = opL[i_element + rhs];
            }
            else
            {
                if (i_src_element < vlmax)
                {
                    (*this)[i_element] =
                        SVElement(opL[0].width_in_bits_, opL[0].mem_ + i_src_element * opL[0].width_in_bits_ / 8);
                }
                else
                {
                    (*this)[i_element] = 0;
                }
            }
        }
    }
    return (*this);
}

SVector &SVector::m_vrgather(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t vlmax,
                             size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // Risky?
            auto i_rhs = rhs[i_element].to_u64();
            (*this)[i_element] = i_rhs >= vlmax ? 0 : opL[i_rhs].to_i64();
        }
    }
    return (*this);
}

SVector &SVector::m_vrgather(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t vlmax,
                             size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // Risky?
            (*this)[i_element] = rhs >= vlmax ? 0 : opL[rhs].to_i64();
        }
    }
    return (*this);
}

SVector &SVector::m_vcompress(const SVector &opL, const SVRegister &vm, size_t start_index)
{
    size_t i_dest = 0;
    for (size_t i_element = start_index; i_element < this->length_; ++i_element)
    {
        if (vm.get_bit(i_element))
        {
            (*this)[i_dest] = opL[i_element];
            i_dest++;
        }
    }
    return (*this);
}