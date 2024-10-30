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
/// \file softvector-platform-types.hpp
/// \brief Extended types for softvector representation
/// \date 07/03/2020
//////////////////////////////////////////////////////////////////////////////////////

#ifndef __SOFTVECTOR_PLATFORM_TYPES_H__
#define __SOFTVECTOR_PLATFORM_TYPES_H__

#include "stdint.h"
#include <vector>
#include <array>

#include "vpu/softvector-types.hpp"

#define SVMaskReg SVRegister

//////////////////////////////////////////////////////////////////////////////////////
/// \class SVMul
/// \brief RVV multiplicity
class SVMul
{
  public:
    uint16_t n_;
    uint16_t d_;
    bool is_frac(void) const { return ((n_ > d_) ? 0 : 1); }
    SVMul(uint16_t n, uint16_t d) : n_(n), d_(d) {}
    SVMul(const SVMul &other) : n_(other.n_), d_(other.d_) {}
};

//////////////////////////////////////////////////////////////////////////////////////
/// \class RVVector
/// \brief RISC-V Vector. Makes use of one single SVElement changing its reference to data storage. This prevents
/// excessive allocation overhead.
class RVVector : public SVector
{
  public:
    uint8_t *mem_;            //!< Main memory.
    SVElement &activeElement; //!< Element referencer, is updated when subscript is applied to an object of this class

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Check if memory space of this RVVector overlaps with another
    /// \param v RVVector to test against
    /// \returns 1 if v starts within this. -1 if this starts within v. 0 if no overlap detected.
    int8_t check_mem_overlap(const RVVector &v)
    {
        if ((v.mem_ >= mem_) && (v.mem_ < mem_ + length_ * (activeElement.width_in_bits_ >> 3)))
            return 1;
        if ((mem_ >= v.mem_) && (mem_ < v.mem_ + v.length_ * (v.activeElement.width_in_bits_ >> 3)))
            return -1;
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Overloaded array subscript to return the indexed SVElement as reference
    SVElement &operator[](const size_t index) const
    {
        activeElement.mem_ = mem_ + index * activeElement.width_in_bits_ / 8;
        return (activeElement);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for referenced elements, i.e. externally allocated elements
    RVVector(const size_t length_elements, const size_t single_element_width_bits, const size_t start_reg_index,
             uint8_t *mem)
        : SVector(length_elements, start_reg_index)
        , mem_(mem)
        , activeElement(*(new SVElement(single_element_width_bits, mem)))
    {
    }

    virtual ~RVVector(void) { delete &activeElement; }
};

//////////////////////////////////////////////////////////////////////////////////////
/// \class RVVRegField
/// \brief RISC-V Vector Register Field. Interprets a one-dimensional byte array as a riscv-vector extension register
/// field. Representation in SVector depends on configuration (VTYPE, VLEN, ...) passed on construction.
class RVVRegField
{
  protected:
    std::vector<RVVector *> vs_{};             //!< Vector of RVVector's
    std::array<SVRegister, 32> regs_{};        //!< Fixed sized (32) SVRegister s. v0...v31
    const size_t vector_register_length_bits_; //!< VLEN, Vector register length in bits
    const size_t vector_length_;               //!< VL, Vector length in elements
    const size_t single_element_width_bits_;   //!< SEW, single element width in bits
    uint8_t *mem_;                             //!< Main memory.
    const SVMul multiplicity_; //!< LMUL, Vector register multiplicity, i.e. how many vector register make up one vector
  public:
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Constructor for referenced main memory, i.e. externally allocated memory
    RVVRegField(const size_t vector_register_length_bits, //!< VLEN
                const size_t vector_length,               //!< VL
                const size_t single_element_width_bits,   //!< SEW
                const SVMul &multiplicity,                //!< MUL
                uint8_t *mem                              //!< Vector register memory field
                )
        : vector_register_length_bits_(vector_register_length_bits)
        , vector_length_(vector_length)
        , single_element_width_bits_(single_element_width_bits)
        , mem_(mem)
        , multiplicity_(multiplicity)
    {
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Initialization method. Call after constructor before use.
    void init(void)
    {
        size_t group_size_regs =
            (multiplicity_.is_frac()
                 ? 1
                 : multiplicity_.n_ / multiplicity_.d_); //!< How many vector register make up one vector
        size_t n_vector_register_groups =
            (regs_.size() /
             (multiplicity_.is_frac() ? 1 : multiplicity_.n_ / multiplicity_.d_)); //!< How many vector register groups

        uint8_t *tmem = mem_;
        for (auto &it : regs_)
        {
            it.init_ref(vector_register_length_bits_, tmem);
            tmem += vector_register_length_bits_ / 8;
        }
        for (size_t i_group = 0; i_group < n_vector_register_groups; ++i_group)
        {
            vs_.push_back(new RVVector(vector_length_, single_element_width_bits_, i_group * group_size_regs,
                                       regs_[i_group * group_size_regs].mem_));
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Destructor. Deletes generated SVector and SRegister fields
    virtual ~RVVRegField(void)
    {
        for (auto &v : vs_)
        {
            delete v;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Check wether passed register number is aligned with the current vector register field configuration.
    /// \param reg_n Register number
    /// \returns True unless not aligned.
    bool vec_reg_is_aligned(const size_t reg_n)
    {
        if (!multiplicity_.is_frac())
        {
            return ((reg_n % (multiplicity_.n_ / multiplicity_.d_)) ? false : true);
        }
        return (true);
    }

    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get the SVector for a given register number. Aligns to the vector containing the passed register number
    RVVector &get_vec(const size_t reg_n)
    {
        RVVector *ret = nullptr;
        for (size_t i_vec = 0; i_vec < vs_.size(); ++i_vec)
        {
            ret = vs_[i_vec];
            if ((reg_n == vs_[i_vec]->start_reg_index_))
            {
                break;
            }
        }
        return (*ret);
    }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get the SVRegister for a given register number.
    SVRegister &get_vecreg(const size_t reg_n) { return (regs_[reg_n]); }
    //////////////////////////////////////////////////////////////////////////////////////
    /// \brief Get the SVRegister responsible for masking operations
    SVRegister &get_mask_reg(void) { return (regs_[0]); }
};

#endif /* __SOFTVECOR_TYPES_H__ */
