/*
 * Copyright [2020] [Technical University of Munich]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
//////////////////////////////////////////////////////////////////////////////////////
/// \file softvector.cpp
/// \brief C++ Source for ETISS JIT libary or independent C application.
/// Implements a C interface when compiled to library beforehand
/// \date 06/23/2020
//////////////////////////////////////////////////////////////////////////////////////

#include "softvector.h"
#include "base/base.hpp"
#include "lsu/lsu.hpp"
#include "arithmetic/integer.hpp"
#include "arithmetic/floatingpoint.hpp"
#include "arithmetic/fixedpoint.hpp"
#include "misc/mask.hpp"
#include "misc/permutation.hpp"
#include "misc/reduction.hpp"

extern "C"
{

    int8_t vtype_decode(uint16_t vtype, uint8_t *ta, uint8_t *ma, uint32_t *sew, uint8_t *z_lmul, uint8_t *n_lmul)
    {
        return (VTYPE::decode(vtype, ta, ma, sew, z_lmul, n_lmul));
    }

    uint16_t vtype_encode(uint16_t sew, uint8_t z_lmul, uint8_t n_lmul, uint8_t ta, uint8_t ma)
    {
        return VTYPE::encode(sew, z_lmul, n_lmul, ta, ma);
    }

    uint8_t vtype_extractSEW(uint16_t pVTYPE) { return VTYPE::extractSEW(pVTYPE); }

    uint8_t vtype_extractLMUL(uint16_t pVTYPE) { return VTYPE::extractLMUL(pVTYPE); }

    uint8_t vtype_extractTA(uint16_t pVTYPE) { return VTYPE::extractTA(pVTYPE); }

    uint8_t vtype_extractMA(uint16_t pVTYPE) { return VTYPE::extractMA(pVTYPE); }

    uint16_t vcfg_concatEEW(uint8_t mew, uint8_t width) { return (VTYPE::concatEEW(mew, width)); }

    uint8_t vload_encoded_unitstride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd,
                                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
            return 1;

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_readMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                buff[i] = pM[addr + i];
        };

        VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8, pVd, pMSTART, pVSTART,
                       pVm, 0);

        return (0);
    }

    uint8_t vload_encoded_stride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd,
                                 uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pSTRIDE)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
        {
            return 1;
        }

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_readMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                buff[i] = pM[addr + i];
        };

        VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8, pVd, pMSTART, pVSTART,
                       pVm, pSTRIDE);

        return (0);
    }

    uint8_t vload_segment_unitstride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF,
                                     uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
            return 1;
        if ((pVd + pNF * _z_emul / _n_emul) > 32)
            return 1;
        if (pVSTART >= pVL)
            return (0);

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_readMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                buff[i] = pM[addr + i];
        };

        uint16_t _vstart = pVSTART;
        uint64_t _moffset = pMSTART;

        for (int i = 0; i < pNF; ++i)
        {
            VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8,
                           pVd + (i * _z_emul / _n_emul), _moffset, _vstart, pVm, 0);
            _moffset += (pVL - _vstart) * pEEW / 8;
            _vstart = 0;
        }

        return (0);
    }

    uint8_t vload_segment_stride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF,
                                 uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART,
                                 int16_t pSTRIDE)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
            return 1;
        if ((pVd + pNF * _z_emul / _n_emul) > 32)
            return 1;
        if (pVSTART >= pVL)
            return (0);

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_readMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                buff[i] = pM[addr + i];
        };

        uint16_t _vstart = pVSTART;
        uint64_t _moffset = pMSTART;

        for (int i = 0; i < pNF; ++i)
        {
            _moffset = pMSTART + i * pEEW / 8;
            VLSU::load_eew(f_readMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8,
                           pVd + (i * _z_emul / _n_emul), _moffset, _vstart, pVm, pSTRIDE);
            _vstart = 0;
        }

        return (0);
    }

    uint8_t vstore_encoded_unitstride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd,
                                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
            return 1;

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_writeMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                pM[addr + i] = buff[i];
        };

        VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8, pVd, pMSTART, pVSTART,
                        pVm, 0);

        return (0);
    }

    uint8_t vstore_encoded_stride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pVd,
                                  uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART, int16_t pStride)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * 8) || (_z_emul > _n_emul * 8))
            return 1;

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_writeMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                pM[addr + i] = buff[i];
        };
        VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8, pVd, pMSTART, pVSTART,
                        pVm, pStride);

        return (0);
    }

    uint8_t vstore_segment_unitstride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF,
                                      uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
            return 1;
        if ((pVd + pNF * _z_emul / _n_emul) > 32)
            return 1;
        if (pVSTART >= pVL)
            return (0);

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_writeMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                pM[addr + i] = buff[i];
        };

        uint16_t _vstart = pVSTART;
        uint64_t _moffset = pMSTART;

        for (int i = 0; i < pNF; ++i)
        {
            VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8,
                            pVd + (i * _z_emul / _n_emul), _moffset, _vstart, pVm, 0);
            _moffset += (pVL - _vstart) * pEEW / 8;
            _vstart = 0;
        }

        return (0);
    }

    uint8_t vstore_segment_stride(void *pV, uint8_t *pM, uint16_t pVTYPE, uint8_t pVm, uint16_t pEEW, uint8_t pNF,
                                  uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint64_t pMSTART,
                                  int16_t pStride)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint64_t _z_emul = pEEW * _vt._z_lmul;
        uint64_t _n_emul = _vt._sew * _vt._n_lmul;

        if ((_n_emul > _z_emul * pNF * 8) || (_z_emul * pNF > _n_emul * 8))
            return 1;
        if ((pVd + pNF * _z_emul / _n_emul) > 32)
            return 1;
        if (pVSTART >= pVL)
            return (0);

        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        std::function<void(size_t, uint8_t *, size_t)> f_writeMem = [pM](size_t addr, uint8_t *buff, size_t len) {
            for (size_t i = 0; i < len; ++i)
                pM[addr + i] = buff[i];
        };

        uint16_t _vstart = pVSTART;
        uint64_t _moffset = pMSTART;
        for (int i = 0; i < pNF; ++i)
        {
            _moffset = pMSTART + i * pEEW / 8;
            VLSU::store_eew(f_writeMem, VectorRegField, _z_emul, _n_emul, pEEW / 8, pVL, pVLEN / 8,
                            pVd + (i * _z_emul / _n_emul), _moffset, _vstart, pVm, 0);
            _moffset += (pVL - _vstart) * pEEW / 8;
            _vstart = 0;
        }

        return (0);
    }

    /* 11. Vector Integer Arithmetic Instructions */

    /* 11.1. Vector Single-Width Integer Add and Subtract */
    uint8_t vadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::add_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vadd_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::add_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vadd_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::add_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::sub_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vsub_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::sub_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vrsub_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::rsub_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vrsub_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::rsub_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                            pVSTART, pVm);

        return (0);
    }
    /* End 11.1. */

    /* 11.2. Vector Widening Integer Add/Subtract */
    uint8_t vwaddu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, true, false);

        return (0);
    }

    uint8_t vwadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, true, true);

        return (0);
    }

    uint8_t vwsubu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, false, false);

        return (0);
    }

    uint8_t vwsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, false, true);

        return (0);
    }

    uint8_t vwaddu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, true, false, pXLEN / 8);

        return (0);
    }

    uint8_t vwadd_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, true, true, pXLEN / 8);

        return (0);
    }

    uint8_t vwsubu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, false, false, pXLEN / 8);

        return (0);
    }

    uint8_t vwsub_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, false, true, pXLEN / 8);

        return (0);
    }

    uint8_t vwaddu_w_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, true, false);

        return (0);
    }

    uint8_t vwadd_w_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, true, true);

        return (0);
    }

    uint8_t vwsubu_w_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, false, false);

        return (0);
    }

    uint8_t vwsub_w_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::wop_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, false, true);

        return (0);
    }

    uint8_t vwaddu_w_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, true, false, pXLEN / 8);

        return (0);
    }

    uint8_t vwadd_w_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, true, true, pXLEN / 8);

        return (0);
    }

    uint8_t vwsubu_w_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, false, false, pXLEN / 8);

        return (0);
    }

    uint8_t vwsub_w_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::wop_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, false, true, pXLEN / 8);

        return (0);
    }
    /* End 11.2. */

    /* 11.3. Vector Integer Extension */
    uint8_t vext_vf(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t extension_encoding,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vext_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            extension_encoding, pVSTART, pVm);

        return (0);
    }
    /* End 11.3. */

    /* 11.5. Vector Bitwise Logical Instructions */
    uint8_t vand_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::and_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vand_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::and_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vand_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::and_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vor_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                   uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::or_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                          pVSTART, pVm);

        return (0);
    }

    uint8_t vor_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                   uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::or_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                          pVSTART, pVm);

        return (0);
    }

    uint8_t vor_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                   uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::or_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                          pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vxor_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::xor_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vxor_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::xor_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vxor_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::xor_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }
    /* End 11.5. */

    /* 11.6. Vector Single-Width Shift Instructions */
    uint8_t vsll_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::sll_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vsll_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        VARITH_INT::sll_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vsll_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::sll_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vsrl_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::srl_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vsrl_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::srl_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vsrl_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::srl_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vsra_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::sra_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vsra_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::sra_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vsra_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::sra_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }
    /* End 11.6. */

    /* 11.7. Vector Narrowing Integer Right Shift Instructions */
    uint8_t vnsrl_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vnsrl_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vnsrl_wi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vnsrl_wi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vnsrl_wx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vnsrl_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vnsra_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vnsra_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vnsra_wi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vnsra_wi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vnsra_wx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vnsra_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }
    /* End 11.7. */

    /* 11.8. Vector Integer Compare Instructions */
    uint8_t vmseq_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::mseq_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmseq_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::mseq_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmseq_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::mseq_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmsne_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msne_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmsne_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msne_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmsne_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::msne_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmsltu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msltu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmsltu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::msltu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmslt_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::mslt_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmslt_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::mslt_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmsleu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msleu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmsleu_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msleu_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmsleu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::msleu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmsle_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msle_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmsle_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msle_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmsle_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::msle_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmsgtu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msgtu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmsgtu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::msgtu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmsgtu_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msgtu_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmsgt_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msgt_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmsgt_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::msgt_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmsgt_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::msgt_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                            pVSTART, pVm);

        return (0);
    }
    /* End 11.8. */

    uint8_t vmv_xs(void *pV, void *pR, uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL,
                   uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRd * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRd * 8]);

        VPERM::mv_xs(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVs2, ScalarReg, pXLEN / 8);

        return (0);
    }

    uint8_t vmv_sx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN,
                   uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VPERM::mv_sx(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVd, ScalarReg, pVSTART, pXLEN / 8);

        return (0);
    }

    uint8_t vfmv_fs(void *pV, void *pF, uint16_t pVTYPE, uint8_t pRd, uint8_t pVs2, uint16_t pVLEN, uint16_t pVL,
                    uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pF))[pRd * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pF)[pRd * 8]);

        VPERM::fmv_fs(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVs2, ScalarReg, pXLEN / 8);

        return (0);
    }

    uint8_t vfmv_sf(void *pV, void *pF, uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pF))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pF)[pRs1 * 8]);

        VPERM::fmv_sf(VectorRegField, _vt._sew / 8, pVL, pVLEN / 8, pVd, ScalarReg, pVSTART, pXLEN / 8);

        return (0);
    }

    uint8_t vslideup_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VPERM::slideup_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                          pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vslideup_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VPERM::slideup_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                          pVSTART, pVm);

        return (0);
    }

    uint8_t vslidedown_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                          uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VPERM::slidedown_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vslidedown_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                          uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VPERM::slidedown_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vslide1up_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VPERM::slide1up(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                        pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vfslide1up(void *pV, void *pF, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pF))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pF)[pRs1 * 8]);

        VPERM::fslide1up(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                         pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vslide1down_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                           uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VPERM::slide1down(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                          pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vfslide1down(void *pV, void *pF, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pF))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pF)[pRs1 * 8]);

        VPERM::fslide1down(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    /* 11.10. Vector Single-Width Integer Multiply Instructions */
    uint8_t vmul_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmul_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmul_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmul_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmulh_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmulh_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmulh_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmulh_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmulhu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmulhu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm);

        return (0);
    }

    uint8_t vmulhu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmulhu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmulhsu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmulhsu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm);

        return (0);
    }

    uint8_t vmulhsu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmulhsu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }
    /* End 11.10. */

    /* 11.11. Vector Integer Divide Instructions */
    uint8_t vdiv_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vdiv_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vdiv_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vdiv_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vdivu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vdivu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vdivu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vdivu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vrem_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vrem_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vrem_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vrem_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vremu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vremu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vremu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vremu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }
    /* End 11.11. */

    /* 11.12. */
    uint8_t vwmul_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vwmul_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm, VARITH_INT::VWMUL_TYPE::S_S);

        return (0);
    }

    uint8_t vwmul_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vwmul_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::VWMUL_TYPE::S_S);

        return (0);
    }

    uint8_t vwmulu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vwmul_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm, VARITH_INT::VWMUL_TYPE::U_U);

        return (0);
    }

    uint8_t vwmulu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vwmul_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::VWMUL_TYPE::U_U);

        return (0);
    }

    uint8_t vwmulsu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vwmul_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm, VARITH_INT::VWMUL_TYPE::S_U);

        return (0);
    }

    uint8_t vwmulsu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vwmul_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::VWMUL_TYPE::S_U);

        return (0);
    }
    /* End 11.12. */

    uint8_t vmax_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmax_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmax_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmax_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmaxu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmaxu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmaxu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmaxu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmin_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmin_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART, pVm);

        return (0);
    }

    uint8_t vmin_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                    uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmin_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                            ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vminu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vminu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vminu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vminu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    /* 11.4 Vector Integer Add-with-Carry / Subtract-with-Borrow Instructions */
    uint8_t vadc_vvm(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vadc_vvm(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART);

        return (0);
    }

    uint8_t vadc_vxm(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vadc_vxm(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pXLEN / 8);

        return (0);
    }

    uint8_t vadc_vim(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vadc_vim(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                             pVSTART);

        return (0);
    }

    uint8_t vmadc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmadc_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmadc_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmadc_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmadc_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmadc_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vsbc_vvm(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vsbc_vvm(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART);

        return (0);
    }

    uint8_t vsbc_vxm(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vsbc_vxm(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pXLEN / 8);

        return (0);
    }

    uint8_t vmsbc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmsbc_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmsbc_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmsbc_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }
    /* End 11.4 */

    /* 11.13. Vector Single-Width Integer Multiply-Add Instructions */
    uint8_t vmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmacc_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmacc_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmacc_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vnmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vnmsac_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm);

        return (0);
    }

    uint8_t vnmsac_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vnmsac_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vmadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmadd_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                             pVSTART, pVm);

        return (0);
    }

    uint8_t vmadd_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmadd_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                             ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }

    uint8_t vnmsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vnmsub_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm);

        return (0);
    }

    uint8_t vnmsub_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vnmsub_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8);

        return (0);
    }
    /* End 11.13. */

    /* 11.14. Vector Widening Integer Multiply-Add Instructions  */
    uint8_t vwmaccu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vwmacc_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::VWMACC_TYPE::U_U);

        return (0);
    }

    uint8_t vwmaccu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vwmacc_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::VWMACC_TYPE::U_U);

        return (0);
    }

    uint8_t vwmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vwmacc_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::VWMACC_TYPE::S_S);

        return (0);
    }

    uint8_t vwmacc_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vwmacc_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::VWMACC_TYPE::S_S);

        return (0);
    }

    uint8_t vwmaccsu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vwmacc_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::VWMACC_TYPE::S_U);

        return (0);
    }

    uint8_t vwmaccsu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vwmacc_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::VWMACC_TYPE::S_U);

        return (0);
    }

    uint8_t vwmaccus_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vwmacc_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::VWMACC_TYPE::U_S);

        return (0);
    }
    /* End 11.14. */

    /* 11.15. Vector Integer Merge Instructions */
    uint8_t vmerge_vv(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmerge_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART);

        return (0);
    }

    uint8_t vmerge_vi(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::vmerge_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                              pVSTART);

        return (0);
    }

    uint8_t vmerge_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::vmerge_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pXLEN / 8);

        return (0);
    }
    /* End 11.15. */

    /* 11.16. Vector Integer Move Instructions */
    uint8_t vmv_vv(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::mv_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVSTART);

        return (0);
    }

    uint8_t vmv_vi(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVimm, uint16_t pVSTART, uint16_t pVLEN,
                   uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::mv_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVimm, pVSTART);

        return (0);
    }

    uint8_t vmv_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN,
                   uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_INT::mv_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, ScalarReg,
                          pVSTART, pXLEN / 8);

        return (0);
    }
    /* End 11.16. */

} // extern "C"
