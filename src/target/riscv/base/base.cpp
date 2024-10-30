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
/// \file base.cpp
/// \brief C++ Source for vector base helpers for RISC-V ISS
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "base/base.hpp"

int8_t VTYPE::decode(uint16_t vtype, uint8_t *ta, uint8_t *ma, uint32_t *sew, uint8_t *z_lmul, uint8_t *n_lmul)
{
    *ta = extractTA(vtype);
    *ma = extractMA(vtype);
    *sew = 8 << extractSEW(vtype);

    uint8_t _flmul = extractLMUL(vtype);

    *z_lmul = 1;
    *n_lmul = 1;

    switch (bits_LMUL_t(_flmul))
    {
    case BITS_LMUL::MF8:
        *z_lmul = 8;
        break;
    case BITS_LMUL::MF4:
        *n_lmul = 4;
        break;
    case BITS_LMUL::MF2:
        *n_lmul = 2;
        break;
    case BITS_LMUL::M1:
        *z_lmul = 1;
        break;
    case BITS_LMUL::M2:
        *z_lmul = 2;
        break;
    case BITS_LMUL::M4:
        *z_lmul = 4;
        break;
    case BITS_LMUL::M8:
        *z_lmul = 8;
        break;
    default:
        return (-1);
    }
    return (1);
}

uint16_t VTYPE::encode(uint16_t sew, uint8_t z_lmul, uint8_t n_lmul, uint8_t ta, uint8_t ma)
{
    uint16_t vtype = 0;
    uint8_t lmulF = -1;
    int16_t sewF = -1;

    sew = sew >> 3;
    while (sew)
    {
        ++sewF;
        sew = sew >> 1;
    }

    if (z_lmul >= n_lmul)
    {
        uint8_t lmul = z_lmul / n_lmul;
        while (lmul)
        {
            ++lmulF;
            lmul = lmul >> 1;
        }
    }
    else
    {
        switch (n_lmul / z_lmul)
        {
        case 2:
            lmulF = BITS_LMUL::MF2;
            break;
        case 4:
            lmulF = BITS_LMUL::MF4;
            break;
        case 8:
            lmulF = BITS_LMUL::MF8;
            break;
        default:
            lmulF = BITS_LMUL::RES;
            break;
        }
    }

    vtype = (ma ? 0x80 : 0) | (ta ? 0x40 : 0) | ((sewF) << 3) | (lmulF & 0x7);

    return (vtype);
}

uint8_t VTYPE::extractSEW(uint16_t pVTYPE)
{
    uint8_t x = 0;
    x |= (pVTYPE & MASK::MSKSEW) >> OFFSETS::OFFSEW;
    return (x);
}

uint8_t VTYPE::extractLMUL(uint16_t pVTYPE)
{
    uint8_t x = 0;
    x |= (pVTYPE & MASK::MSKLMUL);
    return (x);
}

uint8_t VTYPE::extractTA(uint16_t pVTYPE)
{
    uint8_t x = 0;
    x = (pVTYPE & MASK::MSKTA) ? 1 : 0;
    return (x);
}

uint8_t VTYPE::extractMA(uint16_t pVTYPE)
{
    uint8_t x = 0;
    x = (pVTYPE & MASK::MSKMA) ? 1 : 0;
    return (x);
}

uint16_t VTYPE::concatEEW(uint8_t mew, uint8_t width)
{
    uint8_t _eewbf = ((mew & 0x1) << 3) | (width & 0x7);
    uint16_t ret = 0;
    switch (_eewbf)
    {
    case EEW_8:
        ret = 8;
        break;
    case EEW_16:
        ret = 16;
        break;
    case EEW_32:
        ret = 32;
        break;
    case EEW_64:
        ret = 64;
        break;
    case EEW_128:
        ret = 128;
        break;
    case EEW_256:
        ret = 256;
        break;
    case EEW_512:
        ret = 512;
        break;
    case EEW_1024:
        ret = 1024;
        break;
    default:
        break;
    }
    return (ret);
}
