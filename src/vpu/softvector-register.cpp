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
/// \file softvector-register.cpp
/// \brief Extended register softvector representation
/// \date 07/03/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "vpu/softvector-types.hpp"

SVRegister &SVRegister::operator=(const SVRegister &rhs)
{
    for (size_t i_byte = 0; i_byte < length_bits_ / 8; ++i_byte)
    {
        (*this)[i_byte] = rhs[i_byte];
    }
    return (*this);
}

SVRegister &SVRegister::operator=(const int64_t rhs)
{
    for (size_t i_byte = 0; i_byte < length_bits_; ++i_byte)
    {
        if (i_byte < 8)
            (*this)[i_byte] = rhs >> 8 * i_byte;
        else
            (*this)[i_byte] = (rhs >= 0) ? 0 : 0xFF;
    }
    return (*this);
}
