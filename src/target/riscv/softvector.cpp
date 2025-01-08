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

        // VARITH_INT::add_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                    pVSTART, pVm);
        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::add, /* signed_vs2 = */ true, /* signed_vs1 = */ true);

        return (0);
    }

    uint8_t vadd_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::int_op_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                              pVSTART, pVm, VARITH_INT::add, /* signed_vs2 = */ true, /* signed_imm = */ true);

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

        VARITH_INT::int_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::add, /* signed_vs2 = */ true,
                              /* signed_scalar = */ true);

        return (0);
    }

    uint8_t vsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::sub, /* signed_vs2 = */ true, /* signed_vs1 = */ true);

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

        VARITH_INT::int_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::sub, /* signed_vs2 = */ true,
                              /* signed_scalar = */ true);

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

        VARITH_INT::int_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::rsub, /* signed_vs2 = */ true,
                              /* signed_scalar = */ true);

        return (0);
    }

    uint8_t vrsub_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::int_op_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                              pVSTART, pVm, VARITH_INT::rsub, /* signed_vs2 = */ true, /* signed_imm = */ true);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::logical_and, /* signed_vs2 = */ true, /* signed_vs1 = */ true);

        return (0);
    }

    uint8_t vand_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::int_op_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                              pVSTART, pVm, VARITH_INT::logical_and, /* signed_vs2 = */ true, /* signed_imm = */ true);

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

        VARITH_INT::int_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::logical_and, /* signed_vs2 = */ true,
                              /* signed_scalar = */ true);

        return (0);
    }

    uint8_t vor_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                   uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::logical_or, /* signed_vs2 = */ true, /* signed_vs1 = */ true);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::logical_xor, /* signed_vs2 = */ true, /* signed_vs1 = */ true);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::sll, /* signed_vs2 = */ false, /* signed_vs1 = */ false);
        // VARITH_INT::sll_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                    pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::srl, /* signed_vs2 = */ false, /* signed_vs1 = */ false);
        // VARITH_INT::srl_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                    pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::sra, /* signed_vs2 = */ true, /* signed_vs1 = */ false);
        // VARITH_INT::sra_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                    pVSTART, pVm);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::seq, /* signed_vs2 = */ true,
                                      /* signed_scalar = */ true);
        // VARITH_INT::mseq_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                     ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::sne, /* signed_vs2 = */ true,
                                      /* signed_scalar = */ true);
        // VARITH_INT::msne_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                     ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::sltu, /* signed_vs2 = */ false,
                                      /* signed_scalar = */ false);
        // VARITH_INT::msltu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                      ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::slt, /* signed_vs2 = */ true,
                                      /* signed_scalar = */ true);
        // VARITH_INT::mslt_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                     ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_compare_op_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVimm, pVSTART, pVm, VARITH_INT::sleu, /* signed_vs2 = */ false);
        // VARITH_INT::msleu_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        // pVimm,
        //                      pVSTART, pVm);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::sleu, /* signed_vs2 = */ false,
                                      /* signed_scalar = */ false);
        // VARITH_INT::msleu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                      ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::sle, /* signed_vs2 = */ true,
                                      /* signed_scalar = */ true);
        // VARITH_INT::msle_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                     ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::sgtu, /* signed_vs2 = */ false,
                                      /* signed_scalar = */ false);
        // VARITH_INT::msgtu_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                      ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_compare_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::sgt, /* signed_vs2 = */ true,
                                      /* signed_scalar = */ true);
        // VARITH_INT::msgt_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                     ScalarReg, pVSTART, pVm, pXLEN / 8);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::mul, /* signed_vs2 = */ true, /* signed_vs1 = */ true);
        // VARITH_INT::vmul_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                     pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::mulh, /* signed_vs2 = */ true, /* signed_vs1 = */ true);
        // VARITH_INT::vmulh_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                      pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::mulhu, /* signed_vs2 = */ false, /* signed_vs1 = */ false);
        // VARITH_INT::vmulhu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1,
        // pVs2,
        //                       pVSTART, pVm);

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
        // TODO: new version not passing test
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        // VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1,
        // pVs2,
        //                       pVSTART, pVm, VARITH_INT::mulh, /* signed_vs2 = */ true, /* signed_vs1 = */ false);
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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::div, /* signed_vs2 = */ true, /* signed_vs1 = */ true);
        // VARITH_INT::vdiv_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                     pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::divu, /* signed_vs2 = */ false, /* signed_vs1 = */ false);
        // VARITH_INT::vdivu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                      pVSTART, pVm);

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

        VARITH_INT::int_op_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, VARITH_INT::rem, /* signed_vs2 = */ true,
                              /* signed_scalar = */ true);

        return (0);
    }

    uint8_t vrem_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::rem, /* signed_vs2 = */ true, /* signed_vs1 = */ true);

        // VARITH_INT::vrem_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                     pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::remu, /* signed_vs2 = */ false, /* signed_vs1 = */ false);
        // VARITH_INT::vremu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                      pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::max, /* signed_vs2 = */ true, /* signed_vs1 = */ true);
        // VARITH_INT::vmax_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                     pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::maxu, /* signed_vs2 = */ false, /* signed_vs1 = */ false);
        // VARITH_INT::vmaxu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                      pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::min, /* signed_vs2 = */ true, /* signed_vs1 = */ true);
        // VARITH_INT::vmin_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                     pVSTART, pVm);

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

        VARITH_INT::int_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, VARITH_INT::minu, /* signed_vs2 = */ false, /* signed_vs1 = */ false);
        // VARITH_INT::vminu_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
        //                      pVSTART, pVm);

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
    /* End 11. */

    /* 12. Vector Fixed-Point Arithmetic Instructions */
    /* 12.1. Vector Single-Width Saturating Add and Subtract */
    uint8_t vsaddu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vsadd_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs1, pVs2, pVSTART, pVm, false);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vsaddu_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vsadd_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs2, pVimm, pVSTART, pVm, false);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vsaddu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
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

        auto ret = VARITH_FIXP::vsadd_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs2, ScalarReg, pVSTART, pVm, false, pXLEN / 8);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vsadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vsadd_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs1, pVs2, pVSTART, pVm, true);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vsadd_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vsadd_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs2, pVimm, pVSTART, pVm, true);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vsadd_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
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

        auto ret = VARITH_FIXP::vsadd_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs2, ScalarReg, pVSTART, pVm, true, pXLEN / 8);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vssubu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vssub_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs1, pVs2, pVSTART, pVm, false);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vssubu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
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

        auto ret = VARITH_FIXP::vssub_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs2, ScalarReg, pVSTART, pVm, false, pXLEN / 8);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vssub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vssub_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs1, pVs2, pVSTART, pVm, true);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vssub_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
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

        auto ret = VARITH_FIXP::vssub_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs2, ScalarReg, pVSTART, pVm, true, pXLEN / 8);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }
    /* End 12.1. */

    /* 12.2. Vector Single-Width Averaging Add and Subtract */
    /* TODO: Check for illegal rounding mode values */
    uint8_t vaaddu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FIXP::vaadd_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, false, pRm);

        return (0);
    }

    uint8_t vaaddu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = false };

        VARITH_FIXP::fixp_op_vx(VectorRegField, v_instr_info, pVd, pVs2, ScalarReg, pXLEN, pRm, VARITH_FIXP::aaddu);

        return (0);
    }

    uint8_t vaadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FIXP::vaadd_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, true, pRm);

        return (0);
    }

    uint8_t vaadd_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = true };

        VARITH_FIXP::fixp_op_vx(VectorRegField, v_instr_info, pVd, pVs2, ScalarReg, pXLEN, pRm, VARITH_FIXP::aadd);

        return (0);
    }

    uint8_t vasubu_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = false };

        VARITH_FIXP::fixp_op_vv(VectorRegField, v_instr_info, pVd, pVs1, pVs2, pRm, VARITH_FIXP::asubu);

        return (0);
    }

    uint8_t vasubu_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = false };

        VARITH_FIXP::fixp_op_vx(VectorRegField, v_instr_info, pVd, pVs2, ScalarReg, pXLEN, pRm, VARITH_FIXP::asubu);

        return (0);
    }

    uint8_t vasub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FIXP::vasub_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, true, pRm);

        return (0);
    }

    uint8_t vasub_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = true };

        VARITH_FIXP::fixp_op_vx(VectorRegField, v_instr_info, pVd, pVs2, ScalarReg, pXLEN, pRm, VARITH_FIXP::asub);

        return (0);
    }
    /* End 12.2. */

    /* 12.3. Vector Single-Width Fractional Multiply with Rounding and Saturation */
    uint8_t vsmul_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vsmul_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs1, pVs2, pVSTART, pVm, pRm);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vsmul_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        auto ret = VARITH_FIXP::vsmul_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                         pVs2, ScalarReg, pVSTART, pVm, pXLEN / 8, pRm);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }
    /* End 12.3. */

    /* 12.4. Vector Single-Width Scaling Shift Instructions */
    uint8_t vssrl_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FIXP::vssrl_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, pRm);

        return 0;
    }

    uint8_t vssrl_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FIXP::vssrl_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                              pVSTART, pVm, pRm);

        return 0;
    }

    uint8_t vssrl_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FIXP::vssrl_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, pRm);

        return 0;
    }

    uint8_t vssra_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FIXP::vssra_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                              pVSTART, pVm, pRm);

        return 0;
    }

    uint8_t vssra_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FIXP::vssra_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                              pVSTART, pVm, pRm);

        return 0;
    }

    uint8_t vssra_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FIXP::vssra_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                              ScalarReg, pVSTART, pVm, pXLEN / 8, pRm);

        return 0;
    }
    /* End 12.4. */

    /* 12.5. Vector Narrowing Fixed-Point Clip Instructions */
    uint8_t vnclipu_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = false };

        auto ret = VARITH_FIXP::fixp_op_vv(VectorRegField, v_instr_info, pVd, pVs1, pVs2, pRm, VARITH_FIXP::clipu,
                                           /* narrowing = */ true);

        // VARITH_FIXP::vnclipu_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1,
        // pVs2,
        //                         pVSTART, pVm, pRm);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vnclipu_wi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = false };

        VARITH_FIXP::fixedpoint_info_t fixedpoint_info{ .rounding_mode = pRm, .narrowing_op = true };

        auto ret = VARITH_FIXP::fixp_op_vi(VectorRegField, v_instr_info, fixedpoint_info, pVd, pVs2, pVimm,
                                           VARITH_FIXP::clipu);

        // VARITH_FIXP::vnclipu_wi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN
        // / 8, pVd, pVs2,
        //                         pVimm, pVSTART, pVm, pRm);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vnclipu_wx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = false };

        auto ret = VARITH_FIXP::fixp_op_vx(VectorRegField, v_instr_info, pVd, pVs2, ScalarReg, pXLEN, pRm,
                                           VARITH_FIXP::clipu, true);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;

        // VARITH_FIXP::vnclipu_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
        //                         ScalarReg, pVSTART, pVm, pXLEN / 8, pRm);
    }

    uint8_t vnclip_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        auto ret = VARITH_FIXP::vnclip_wv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                          pVs1, pVs2, pVSTART, pVm, pRm);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;

        return 0;
    }

    uint8_t vnclip_wi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        v_instr_info_t v_instr_info{ .emul_num = _vt._z_lmul,
                                     .emul_denom = _vt._n_lmul,
                                     .sew = _vt._sew,
                                     .vector_length = pVL,
                                     .vector_register_length = pVLEN,
                                     .masked = !pVm,
                                     .start_element = pVSTART,
                                     .signed_op = false };

        VARITH_FIXP::fixedpoint_info_t fixedpoint_info{ .rounding_mode = pRm, .narrowing_op = true };

        auto ret =
            VARITH_FIXP::fixp_op_vi(VectorRegField, v_instr_info, fixedpoint_info, pVd, pVs2, pVimm, VARITH_FIXP::clip);

        // auto ret = VARITH_FIXP::vnclip_wi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8,
        // pVd,
        //                                   pVs2, pVimm, pVSTART, pVm, pRm);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }

    uint8_t vnclip_wx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        auto ret = VARITH_FIXP::vnclip_wx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                          pVs2, ScalarReg, pVSTART, pVm, pXLEN / 8, pRm);

        return ret == VILL::VPU_RETURN::NO_EXCEPT_FP_SAT ? 1 : 0;
    }
    /* End 12.5. */
    /* End 12. */

    /* 13. Vector Floating-Point Instructions */
    /* 13.1. Vector Floating-Point Exception Flags */
    /* End 13.1. */
    /* 13.2. Vector Single-Width Floating-Point Add/Subtract Instructions */

    uint8_t vfadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfadd, pRm);

        return 0;
    }

    uint8_t vfadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfadd, pRm);

        return 0;
    }

    uint8_t vfsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfsub, pRm);

        return 0;
    }

    uint8_t vfsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfsub, pRm);

        return 0;
    }

    uint8_t vfrsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfrsub, pRm);

        return 0;
    }
    /* End 13.2. */
    /* 13.3. Vector Widening Floating-Point Add/Subtract Instructions */
    uint8_t vfwadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwadd, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwadd, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwsub, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwsub, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwadd_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwadd_w, pRm, /*wide_dest=*/true, /*wide_vs2=*/true);

        return 0;
    }

    uint8_t vfwadd_wf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwadd_w, pRm, /*wide_dest=*/true,
                               /*wide_vs2=*/true);

        return 0;
    }

    uint8_t vfwsub_wv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwsub_w, pRm, /*wide_dest=*/true, /*wide_vs2=*/true);

        return 0;
    }

    uint8_t vfwsub_wf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwsub_w, pRm, /*wide_dest=*/true,
                               /*wide_vs2=*/true);

        return 0;
    }
    /* End 13.3. */
    /* 13.4. Vector Single-Width Floating-Point Multiply/Divide Instructions */
    uint8_t vfmul_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfmul, pRm);

        return 0;
    }

    uint8_t vfmul_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfmul, pRm);

        return 0;
    }

    uint8_t vfdiv_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfdiv, pRm);

        return 0;
    }

    uint8_t vfdiv_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfdiv, pRm);

        return 0;
    }
    /* End 13.4. */
    /* 13.5. Vector Widening Floating-Point Multiply */
    uint8_t vfwmul_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwmul, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwmul_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwmul, pRm, /*wide_dest=*/true);

        return 0;
    }
    /* End 13.5. */
    /* 13.6. Vector Single-Width Floating-Point Fused Multiply-Add Instructions */
    uint8_t vfmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfmacc, pRm);

        return 0;
    }

    uint8_t vfmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfmacc, pRm);

        return 0;
    }

    uint8_t vfnmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfnmacc, pRm);

        return 0;
    }

    uint8_t vfnmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfnmacc, pRm);

        return 0;
    }

    uint8_t vfmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfmsac, pRm);

        return 0;
    }

    uint8_t vfmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfmsac, pRm);

        return 0;
    }

    uint8_t vfnmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfnmsac, pRm);

        return 0;
    }

    uint8_t vfnmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfnmsac, pRm);

        return 0;
    }

    uint8_t vfmadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfmadd, pRm);

        return 0;
    }

    uint8_t vfmadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfmadd, pRm);

        return 0;
    }

    uint8_t vfnmadd_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfnmadd, pRm);

        return 0;
    }

    uint8_t vfnmadd_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfnmadd, pRm);

        return 0;
    }

    uint8_t vfmsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfmsub, pRm);

        return 0;
    }

    uint8_t vfmsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfmsub, pRm);

        return 0;
    }

    uint8_t vfnmsub_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfnmsub, pRm);

        return 0;
    }

    uint8_t vfnmsub_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfnmsub, pRm);

        return 0;
    }
    /* End 13.6. */
    /* 13.7. Vector Widening Floating-Point Fused Multiply-Add Instructions */
    uint8_t vfwmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwmacc, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwmacc, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwnmacc_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwnmacc, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwnmacc_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwnmacc, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwmsac, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwmsac, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwnmsac_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfwnmsac, pRm, /*wide_dest=*/true);

        return 0;
    }

    uint8_t vfwnmsac_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfwnmsac, pRm, /*wide_dest=*/true);

        return 0;
    }
    /* End 13.7. */

    /* 13.8. Vector Floating-Point Square-Root Instruction */
    uint8_t vfsqrt_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_unary(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                  pVSTART, pVm, vfsqrt, pRm);

        return 0;
    }
    /* End 13.8. */

    /* 13.9. Vector Floating-Point Reciprocal Square-Root Estimate Instruction */
    uint8_t vfrsqrt7_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                       uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_unary(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                  pVSTART, pVm, vfrsqrt7, pRm);

        return 0;
    }
    /* End 13.9. */

    /* 13.10. Vector Floating-Point Reciprocal Estimate Instruction */
    uint8_t vfrec7_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_unary(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                  pVSTART, pVm, vfrec7, pRm);

        return 0;
    }
    /* End 13.10. */

    /* 13.11. Vector Floating-Point MIN/MAX Instructions */
    uint8_t vfmin_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfmin, pRm);

        return 0;
    }

    uint8_t vfmin_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfmin, pRm);

        return 0;
    }

    uint8_t vfmax_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfmax, pRm);

        return 0;
    }

    uint8_t vfmax_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfmax, pRm);

        return 0;
    }
    /* End 13.11. */

    /* 13.12. Vector Floating-Point Sign-Injection Instructions */
    uint8_t vfsgnj_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfsgnj, pRm);

        return 0;
    }

    uint8_t vfsgnj_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                      uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfsgnj, pRm);

        return 0;
    }

    uint8_t vfsgnjn_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfsgnjn, pRm);

        return 0;
    }

    uint8_t vfsgnjn_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfsgnjn, pRm);

        return 0;
    }

    uint8_t vfsgnjx_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, vfsgnjx, pRm);

        return 0;
    }

    uint8_t vfsgnjx_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART, pVm, vfsgnjx, pRm);

        return 0;
    }
    /* End 13.12. */

    /* 13.13. Vector Floating-Point Compare Instructions */
    uint8_t vmfeq_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1,
                                      pVs2, pVSTART, pVm, vmfeq, pRm);

        return 0;
    }

    uint8_t vmfeq_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pFLEN / 8, pVSTART, pVm, vmfeq, pRm);

        return 0;
    }

    uint8_t vmfne_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1,
                                      pVs2, pVSTART, pVm, vmfne, pRm);

        return 0;
    }

    uint8_t vmfne_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pFLEN / 8, pVSTART, pVm, vmfne, pRm);

        return 0;
    }

    uint8_t vmflt_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1,
                                      pVs2, pVSTART, pVm, vmflt, pRm);

        return 0;
    }

    uint8_t vmflt_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pFLEN / 8, pVSTART, pVm, vmflt, pRm);

        return 0;
    }

    uint8_t vmfle_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_vv_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1,
                                      pVs2, pVSTART, pVm, vmfle, pRm);

        return 0;
    }

    uint8_t vmfle_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pFLEN / 8, pVSTART, pVm, vmfle, pRm);

        return 0;
    }

    uint8_t vmfgt_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pFLEN / 8, pVSTART, pVm, vmfgt, pRm);

        return 0;
    }

    uint8_t vmfge_vf(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
                     uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_op_vf_to_reg(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      ScalarReg, pFLEN / 8, pVSTART, pVm, vmfge, pRm);

        return 0;
    }
    /* End 13.13. */

    /* 13.14. Vector Floating-Point Classify Instruction */
    uint8_t vfclass_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_op_unary(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                  pVSTART, pVm, vfclass, pRm);

        return 0;
    }
    /* End 13.14. */

    /* 13.15. Vector Floating-Point Merge Instruction */
    uint8_t vfmerge_vfm(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t pRs1, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pFLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_merge(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                               ScalarReg, pFLEN / 8, pVSTART);

        return 0;
    }
    /* End 13.15. */

    /* 13.16. Vector Floating-Point Move Instruction */
    uint8_t vfmv_v_f(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVd, uint8_t pRs1, uint16_t pVSTART, uint16_t pVLEN,
                     uint16_t pVL, uint8_t pFLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pFLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRs1 * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRs1 * 8]);

        VARITH_FLOAT::vf_move(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, ScalarReg,
                              pFLEN / 8, pVSTART);

        return 0;
    }
    /* End 13.16. */

    /* 13.17. Single-Width Floating-Point/Integer Type-Convert Instructions */
    uint8_t vfcvt_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                 pVSTART, pVm, convert_x_f, pRm, /* signed_x = */ false,
                                 /* vs2_is_int = */ false);

        return 0;
    }

    uint8_t vfcvt_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                 pVSTART, pVm, convert_x_f, pRm, /* signed_x = */ true,
                                 /* vs2_is_int = */ false);

        return 0;
    }

    uint8_t vfcvt_rtz_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                 pVSTART, pVm, convert_x_f, pRm, /* signed_x = */ false,
                                 /* vs2_is_int = */ false, /* rtz = */ true);

        return 0;
    }

    uint8_t vfcvt_rtz_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                            uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                 pVSTART, pVm, convert_x_f, pRm, /* signed_x = */ true,
                                 /* vs2_is_int = */ false, /* rtz = */ true);

        return 0;
    }

    uint8_t vfcvt_f_xu_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                 pVSTART, pVm, convert_f_x, pRm, /* signed_x = */ false,
                                 /* vs2_is_int = */ true);

        return 0;
    }

    uint8_t vfcvt_f_x_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                        uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                 pVSTART, pVm, convert_f_x, pRm, /* signed_x = */ true,
                                 /* vs2_is_int = */ true);

        return 0;
    }
    /* End 13.17. */

    /* 13.18. Widening Floating-Point/Integer Type-Convert Instructions */
    uint8_t vfwcvt_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_wide(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVSTART, pVm, convert_widening_x_f, pRm, /* signed_x = */ false,
                                      /* vs2_is_int = */ false);

        return 0;
    }

    uint8_t vfwcvt_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_wide(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVSTART, pVm, convert_widening_x_f, pRm, /* signed_x = */ true,
                                      /* vs2_is_int = */ false);

        return 0;
    }

    uint8_t vfwcvt_rtz_xu_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                              uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_wide(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVSTART, pVm, convert_widening_x_f, pRm, /* signed_x = */ false,
                                      /* vs2_is_int = */ false, /* rtz = */ true);

        return 0;
    }

    uint8_t vfwcvt_rtz_x_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_wide(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVSTART, pVm, convert_widening_x_f, pRm, /* signed_x = */ true,
                                      /* vs2_is_int = */ false, /* rtz = */ true);

        return 0;
    }

    uint8_t vfwcvt_f_xu_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_wide(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVSTART, pVm, convert_widening_f_x, pRm, /* signed_x = */ false,
                                      /* vs2_is_int = */ true);

        return 0;
    }

    uint8_t vfwcvt_f_x_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_wide(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVSTART, pVm, convert_widening_f_x, pRm, /* signed_x = */ true,
                                      /* vs2_is_int = */ true);

        return 0;
    }

    uint8_t vfwcvt_f_f_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_wide(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2,
                                      pVSTART, pVm, convert_widening_f_f, pRm, /* signed_x = */ false,
                                      /* vs2_is_int = */ false);

        return 0;
    }
    /* End 13.18. */

    /* 13.19. Narrowing Floating-Point/Integer Type-Convert Instructions */
    uint8_t vfncvt_xu_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_x_f, pRm, /* signed_x = */ false,
                                        /* vs2_is_int = */ false);

        return 0;
    }

    uint8_t vfncvt_x_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_x_f, pRm, /* signed_x = */ true,
                                        /* vs2_is_int = */ false);

        return 0;
    }

    uint8_t vfncvt_rtz_xu_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                              uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_x_f, pRm, /* signed_x = */ false,
                                        /* vs2_is_int = */ false, /* rtz = */ true);

        return 0;
    }

    uint8_t vfncvt_rtz_x_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_x_f, pRm, /* signed_x = */ true,
                                        /* vs2_is_int = */ false, /* rtz = */ true);

        return 0;
    }

    uint8_t vfncvt_f_xu_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                          uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_f_x, pRm, /* signed_x = */ false,
                                        /* vs2_is_int = */ true);

        return 0;
    }

    uint8_t vfncvt_f_x_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_f_x, pRm, /* signed_x = */ true,
                                        /* vs2_is_int = */ true);

        return 0;
    }

    uint8_t vfncvt_f_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_f_f, pRm, /* signed_x = */ false,
                                        /* vs2_is_int = */ false);

        return 0;
    }

    uint8_t vfncvt_rod_f_f_w(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART,
                             uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VARITH_FLOAT::vf_convert_narrow(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd,
                                        pVs2, pVSTART, pVm, convert_narrowing_f_f, pRm, /* signed_x = */ false,
                                        /* vs2_is_int = */ false, /* rtz = */ false, /* rod = */ false);

        return 0;
    }
    /* End 13.19. */
    /* End 13. */

    /* 14. Vector Reduction Operations */
    /* 14.1. Vector Single-Width Integer Reduction Instructions */
    uint8_t vredsum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ false, /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vredmaxu_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_max, /* is_signed = */ false, /* wide_dest = */ false, /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vredmax_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_max, /* is_signed = */ true, /* wide_dest = */ false, /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vredminu_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_min, /* is_signed = */ false, /* wide_dest = */ false, /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vredmin_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_min, /* is_signed = */ true, /* wide_dest = */ false, /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vredand_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_logical_and, /* is_signed = */ false, /* wide_dest = */ false,
                       /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vredor_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_logical_or, /* is_signed = */ false, /* wide_dest = */ false,
                       /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vredxor_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                       uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_logical_xor, /* is_signed = */ false, /* wide_dest = */ false,
                       /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    /* End 14.1. */
    /* 14.2. Vector Widening Integer Reduction Instructions */
    uint8_t vwredsumu_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ false, /* wide_dest = */ true, /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }

    uint8_t vwredsum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ true, /* is_float_instr = */ false,
                       /* rounding_mode = */ 0);

        return 0;
    }
    /* End 14.2. */
    /* 14.3. Vector Single-Width Floating-Point Reduction Instructions */
    uint8_t vfredosum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ false, /* is_float_instr = */ true,
                       /* rounding_mode = */ pRm);

        return 0;
    }

    uint8_t vfredusum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                         uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ false, /* is_float_instr = */ true,
                       /* rounding_mode = */ pRm);

        return 0;
    }

    uint8_t vfredmax_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ false, /* is_float_instr = */ true,
                       /* rounding_mode = */ pRm);

        return 0;
    }

    uint8_t vfredmin_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ false, /* is_float_instr = */ true,
                       /* rounding_mode = */ pRm);

        return 0;
    }
    /* End 14.3. */
    /* 14.4. Vector Widening Floating-Point Reduction Instructions */
    uint8_t vfwredosum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                          uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ true, /* is_float_instr = */ true,
                       /* rounding_mode = */ pRm);

        return 0;
    }

    uint8_t vfwredusum_vs(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                          uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL, uint8_t pRm)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VREDUC::red_op(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2, pVSTART,
                       pVm, red_sum, /* is_signed = */ true, /* wide_dest = */ true, /* is_float_instr = */ true,
                       /* rounding_mode = */ pRm);

        return 0;
    }

    /* 15. Vector Mask Instructions */
    /* 15.1. Vector Mask-Register Logical Instructions */
    uint8_t vmand_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_and);

        return 0;
    }

    uint8_t vmnand_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_nand);

        return 0;
    }

    uint8_t vmandn_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_andn);

        return 0;
    }

    uint8_t vmxor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_xor);

        return 0;
    }

    uint8_t vmor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_or);

        return 0;
    }

    uint8_t vmnor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_nor);

        return 0;
    }

    uint8_t vmorn_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_orn);

        return 0;
    }

    uint8_t vmxnor_mm(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                      uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_logical(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                               pVSTART, pVm, VMASK::logical_xnor);

        return 0;
    }
    /* End 15.1. */
    /* 15.2. Vector count population in mask vcpop.m */
    uint8_t vcpop_m(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pRd, uint8_t pVs2, uint16_t pVSTART,
                    uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRd * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRd * 8]);

        VMASK::mask_op_to_scalar(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVs2,
                                 ScalarReg, pVSTART, pVm, pXLEN / 8, /* is_vcpop = */ true);

        return (0);
    }
    /* End 15.2. */
    /* 15.3. vfirst find-first-set mask bit */
    uint8_t vfirst_m(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pRd, uint8_t pVs2, uint16_t pVSTART,
                     uint16_t pVLEN, uint16_t pVL, uint8_t pXLEN)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *ScalarReg;
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);
        if (pXLEN <= 32)
            ScalarReg = &((static_cast<uint8_t *>(pR))[pRd * 4]);
        else
            ScalarReg = &(static_cast<uint8_t *>(pR)[pRd * 8]);

        VMASK::mask_op_to_scalar(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVs2,
                                 ScalarReg, pVSTART, pVm, pXLEN / 8, /* is_vcpop = */ false);

        return (0);
    }
    /* End 15.3. */
    /* 15.4. vmsbf.m set-before-first mask bit */
    uint8_t vmsbf_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_sxf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVSTART,
                           pVm, /* including_first = */ false, /* only_first = */ false);

        return 0;
    }
    /* End 15.4. */
    /* 15.5. vmsif.m set-including-first mask bit */
    uint8_t vmsif_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_sxf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVSTART,
                           pVm, /* including_first = */ true, /* only_first = */ false);

        return 0;
    }
    /* End 15.5. */
    /* 15.6. vmsof.m set-only-first mask bit */
    uint8_t vmsof_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_op_sxf(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVSTART,
                           pVm, /* including_first = */ false, /* only_first = */ true);

        return 0;
    }
    /* End 15.6. */
    /* 15.8. Vector Iota Instruction */
    uint8_t viota_m(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint16_t pVSTART, uint16_t pVLEN,
                    uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_viota(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVSTART,
                          pVm);

        return 0;
    }
    /* End 15.8. */
    /* 15.9. Vector Element Index Instruction */
    uint8_t vid_v(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VMASK::mask_vid(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVSTART, pVm);

        return 0;
    }
    /* End 15.9. */
    /* End 15. */
    /* 16. Vector Permutation Instructions */
    /* 16.1. Integer Scalar Move Instructions */
    /* End 16.1. */
    /* 16.2. Floating-Point Scalar Move Instructions */
    /* End 16.2. */
    /* 16.3. Vector Slide Instructions */
    /* End 16.3. */
    /* 16.4. Vector Register Gather Instructions */
    uint8_t vrgather_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VPERM::vrgather_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, /* ei16 = */ false);

        return (0);
    }

    uint8_t vrgatherei16_vv(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs1, uint8_t pVs2,
                            uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VPERM::vrgather_vv(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                           pVSTART, pVm, /* ei16 = */ true);

        return (0);
    }

    uint8_t vrgather_vi(void *pV, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pVimm,
                        uint16_t pVSTART, uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VPERM::vrgather_vi(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, pVimm,
                           pVSTART, pVm);

        return (0);
    }

    uint8_t vrgather_vx(void *pV, void *pR, uint16_t pVTYPE, uint8_t pVm, uint8_t pVd, uint8_t pVs2, uint8_t pRs1,
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

        VPERM::vrgather_vx(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, ScalarReg,
                           pVSTART, pVm, pXLEN / 8);

        return (0);
    }
    /* End 16.4. */
    /* 16.5. Vector Compress Instruction */
    uint8_t vcompress_vm(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs1, uint8_t pVs2, uint16_t pVSTART,
                         uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VPERM::vcompress_vm(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs1, pVs2,
                            pVSTART);

        return (0);
    }
    /* End 16.5. */
    /* 16.6. Whole Vector Register Move */
    uint8_t vmvr_v(void *pV, uint16_t pVTYPE, uint8_t pVd, uint8_t pVs2, uint8_t simm5, uint16_t pVSTART,
                   uint16_t pVLEN, uint16_t pVL)
    {
        VTYPE::VTYPE _vt(pVTYPE);
        uint8_t *VectorRegField;

        VectorRegField = static_cast<uint8_t *>(pV);

        VPERM::vmvr_v(VectorRegField, _vt._z_lmul, _vt._n_lmul, _vt._sew / 8, pVL, pVLEN / 8, pVd, pVs2, simm5,
                      pVSTART);

        return 0;
    }
    /* End 16.6. */
    /* End 16. */

} // extern "C"
