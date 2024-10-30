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

// unsigned comparisons
SVRegister SVector::op_u_lt(const SVector &rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element].op_u_lt(rhs[i_element]))
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::op_u_lt(const uint64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element].op_u_lt(rhs))
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::op_u_lte(const SVector &rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element].op_u_lte(rhs[i_element]))
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::op_u_lte(const uint64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element].op_u_lte(rhs))
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::op_u_gt(const SVector &rhs) const
{
    return (rhs.op_u_lt(*this));
}

SVRegister SVector::op_u_gt(const uint64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element].op_u_gt(rhs))
            ret.toggle_bit(i_element);
    }
    return (ret);
}

SVRegister SVector::op_u_gte(const SVector &rhs) const
{
    return (rhs.op_u_lte(*this));
}

SVRegister SVector::op_u_gte(const uint64_t rhs) const
{
    SVRegister ret(length_ * (*this)[0].width_in_bits_ / 8);
    for (size_t i_element = 0; i_element < length_; ++i_element)
    {
        if ((*this)[i_element].op_u_gte(rhs))
            ret.toggle_bit(i_element);
    }
    return (ret);
}

// masked ops
void SVector::m_assign(const SVector &vin, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element] = vin[i_element];
    }
}

void SVector::m_assign(const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element] = rhs;
    }
}

SVector &SVector::m_add(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_add(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_add(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_add(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_sub(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sub(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_sub(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sub(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_rsub(const int64_t lhs, const SVector &opR, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = lhs - opR[i_element].to_i64();
        }
    }
    return (*this);
}

// 11.2. Vector Widening Integer Add/Subtract
SVector &SVector::m_waddu(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_u64() + rhs[i_element].to_u64();
        }
    }
    return (*this);
}

SVector &SVector::m_waddu(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_u64() + rhs;
        }
    }
    return (*this);
}

SVector &SVector::m_wsubu(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_u64() - rhs[i_element].to_u64();
        }
    }
    return (*this);
}

SVector &SVector::m_wsubu(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_u64() - rhs;
        }
    }
    return (*this);
}

SVector &SVector::m_wadd(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_i64() + rhs[i_element].to_i64();
        }
    }
    return (*this);
}

SVector &SVector::m_wadd(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_i64() + rhs;
        }
    }
    return (*this);
}

SVector &SVector::m_wsub(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_i64() - rhs[i_element].to_i64();
        }
    }
    return (*this);
}

SVector &SVector::m_wsub(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_i64() - rhs;
        }
    }
    return (*this);
}
// End 11.2.

/* 11.3. Vector Integer Extension */
SVector &SVector::m_vext(const SVector &opL, const SVRegister &vm, bool mask, bool sign, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = sign ? opL[i_element].to_i64() : opL[i_element].to_u64();
        }
    }
    return (*this);
}
/* End 11.3. */

/* 11.10 & 11.12. Vector (Widening) Integer Multiply Instructions */
SVector &SVector::m_ssmul(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // (*this)[i_element].s_ssmul(opL[i_element], rhs[i_element]);
            (*this)[i_element] = opL[i_element].to_i64() * rhs[i_element].to_i64();
        }
    }
    return (*this);
}
SVector &SVector::m_ssmul(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // (*this)[i_element].s_ssmul(opL[i_element], rhs);
            (*this)[i_element] = opL[i_element].to_i64() * rhs;
        }
    }
    return (*this);
}
SVector &SVector::m_uumul(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // (*this)[i_element].s_ssmul(opL[i_element], rhs[i_element]);
            (*this)[i_element] = opL[i_element].to_u64() * rhs[i_element].to_u64();
        }
    }
    return (*this);
}
SVector &SVector::m_uumul(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // (*this)[i_element].s_ssmul(opL[i_element], rhs[i_element]);
            (*this)[i_element] = opL[i_element].to_u64() * rhs;
        }
    }
    return (*this);
}
SVector &SVector::m_ssmulh(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_ssmulh(opL[i_element], rhs[i_element]);
    }
    return (*this);
}
SVector &SVector::m_ssmulh(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_ssmulh(opL[i_element], rhs);
    }
    return (*this);
}
SVector &SVector::m_uumulh(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_uumulh(opL[i_element], rhs[i_element]);
    }
    return (*this);
}
SVector &SVector::m_uumulh(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_uumulh(opL[i_element], rhs);
    }
    return (*this);
}
SVector &SVector::m_sumulh(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sumulh(opL[i_element], rhs[i_element]);
    }
    return (*this);
}
SVector &SVector::m_sumulh(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sumulh(opL[i_element], rhs);
    }
    return (*this);
}
SVector &SVector::m_sumul(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // TODO: Review this
            // This seems to work since:
            // - I * U is automatically cast to U * U
            // - = operator takes signed
            (*this)[i_element] = opL[i_element].to_i64() * rhs[i_element].to_u64();
        }
    }
    return (*this);
}
SVector &SVector::m_sumul(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            // TODO: Review this
            // This seems to work since:
            // - I * U is automatically cast to U * U
            // - = operator takes signed
            (*this)[i_element] = opL[i_element].to_i64() * rhs;
        }
    }
    return (*this);
}

/* 11.11. Vector Integer Divide Instructions */
SVector &SVector::m_ssdiv(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_ssdiv(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_ssdiv(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_ssdiv(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_uudiv(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_uudiv(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_uudiv(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_uudiv(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_ssrem(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_ssrem(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_ssrem(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_ssrem(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_uurem(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_uurem(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_uurem(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_uurem(opL[i_element], rhs);
    }
    return (*this);
}
/* End 11.11. */

SVector &SVector::m_ssmax(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_i64() > rhs[i_element].to_i64())
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs[i_element];
            }
        }
    }
    return (*this);
}

SVector &SVector::m_ssmax(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_i64() > rhs)
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs;
            }
        }
    }
    return (*this);
}

SVector &SVector::m_uumax(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_u64() > rhs[i_element].to_u64())
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs[i_element];
            }
        }
    }
    return (*this);
}

SVector &SVector::m_uumax(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_u64() > rhs)
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs;
            }
        }
    }
    return (*this);
}

SVector &SVector::m_ssmin(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_i64() < rhs[i_element].to_i64())
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs[i_element];
            }
        }
    }
    return (*this);
}

SVector &SVector::m_ssmin(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_i64() < rhs)
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs;
            }
        }
    }
    return (*this);
}

SVector &SVector::m_uumin(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_u64() < rhs[i_element].to_u64())
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs[i_element];
            }
        }
    }
    return (*this);
}

SVector &SVector::m_uumin(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            if (opL[i_element].to_u64() < rhs)
            {
                (*this)[i_element] = opL[i_element];
            }
            else
            {
                (*this)[i_element] = rhs;
            }
        }
    }
    return (*this);
}

SVector &SVector::m_and(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_and(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_and(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_and(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_or(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_or(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_or(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_or(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_xor(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_xor(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_xor(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_xor(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_sll(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sll(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_sll(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sll(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_sra(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sra(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_sra(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_sra(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_srl(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_srl(opL[i_element], rhs[i_element]);
    }
    return (*this);
}

SVector &SVector::m_srl(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element].s_srl(opL[i_element], rhs);
    }
    return (*this);
}

SVector &SVector::m_nsra(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        auto shiftamount_bitmask = opL[i_element].width_in_bits_ - 1;
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_i64() >> (rhs[i_element].to_u64() & shiftamount_bitmask);
        }
    }
    return (*this);
}

SVector &SVector::m_nsra(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        auto shiftamount_bitmask = opL[i_element].width_in_bits_ - 1;
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_i64() >> (rhs & shiftamount_bitmask);
        }
    }
    return (*this);
}

SVector &SVector::m_nsrl(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        auto shiftamount_bitmask = opL[i_element].width_in_bits_ - 1;
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_u64() >> (rhs[i_element].to_u64() & shiftamount_bitmask);
        }
    }
    return (*this);
}

SVector &SVector::m_nsrl(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        auto shiftamount_bitmask = opL[i_element].width_in_bits_ - 1;
        if (!mask || vm.get_bit(i_element))
        {
            (*this)[i_element] = opL[i_element].to_u64() >> (rhs & shiftamount_bitmask);
        }
    }
    return (*this);
}

SVector &SVector::m_slideup(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    size_t max = rhs > start_index ? rhs : start_index;
    for (size_t i_element = max; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
            (*this)[i_element] = opL[i_element];
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

// 11.4. Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions
SVector &SVector::m_adc(const SVector &opL, const SVector &rhs, const SVRegister &vm, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        (*this)[i_element] = opL[i_element].to_i64() + rhs[i_element].to_i64() + vm.get_bit(i_element);
    }
    return (*this);
}

SVector &SVector::m_adc(const SVector &opL, const int64_t rhs, const SVRegister &vm, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        (*this)[i_element] = opL[i_element].to_i64() + rhs + vm.get_bit(i_element);
    }
    return (*this);
}

SVector &SVector::m_sbc(const SVector &opL, const SVector &rhs, const SVRegister &vm, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        (*this)[i_element] = opL[i_element].to_i64() - rhs[i_element].to_i64() - vm.get_bit(i_element);
    }
    return (*this);
}

SVector &SVector::m_sbc(const SVector &opL, const int64_t rhs, const SVRegister &vm, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        (*this)[i_element] = opL[i_element].to_i64() - rhs - vm.get_bit(i_element);
    }
    return (*this);
}
/* End 11.4. */

/* 11.13. Vector Single-Width Integer Multiply-Add Instructions */
SVector &SVector::m_ssmacc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (opL[i_element].to_i64() * rhs[i_element].to_i64()) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_ssmacc(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (opL[i_element].to_i64() * rhs) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_nmsac(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = -(opL[i_element].to_i64() * rhs[i_element].to_i64()) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_nmsac(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = -(opL[i_element].to_i64() * rhs) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_madd(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (current_value * rhs[i_element].to_i64()) + opL[i_element].to_i64();
        }
    }
    return (*this);
}

SVector &SVector::m_madd(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (current_value * rhs) + opL[i_element].to_i64();
        }
    }
    return (*this);
}

SVector &SVector::m_nmsub(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = -(current_value * rhs[i_element].to_i64()) + opL[i_element].to_i64();
        }
    }
    return (*this);
}

SVector &SVector::m_nmsub(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = -(current_value * rhs) + opL[i_element].to_i64();
        }
    }
    return (*this);
}
/* End 11.13. */

/* 11.14. Vector Widening Integer Multiply-Add Instructions */
SVector &SVector::m_uumacc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (opL[i_element].to_u64() * rhs[i_element].to_u64()) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_uumacc(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (opL[i_element].to_u64() * rhs) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_sumacc(const SVector &opL, const SVector &rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (opL[i_element].to_u64() * rhs[i_element].to_i64()) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_sumacc(const SVector &opL, const int64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (opL[i_element].to_u64() * rhs) + current_value;
        }
    }
    return (*this);
}

SVector &SVector::m_usmacc(const SVector &opL, const uint64_t rhs, const SVRegister &vm, bool mask, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        if (!mask || vm.get_bit(i_element))
        {
            auto current_value = (*this)[i_element].to_i64();
            (*this)[i_element] = (opL[i_element].to_i64() * rhs) + current_value;
        }
    }
    return (*this);
}
/* End 11.14.*/

/* 11.15. Vector Integer Merge Instructions */
SVector &SVector::m_merge(const SVector &opL, const SVector &rhs, const SVRegister &vm, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        (*this)[i_element] = vm.get_bit(i_element) ? rhs[i_element] : opL[i_element];
    }
    return (*this);
}

SVector &SVector::m_merge(const SVector &opL, const int64_t rhs, const SVRegister &vm, size_t start_index)
{
    for (size_t i_element = start_index; i_element < length_; ++i_element)
    {
        (*this)[i_element] = vm.get_bit(i_element) ? rhs : opL[i_element].to_i64();
    }
    return (*this);
}
/* End 11.15. */
