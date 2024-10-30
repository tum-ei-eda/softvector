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
#ifndef __RVV_HL_TESTCASES_H__
#define __RVV_HL_TESTCASES_H__

#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>

#include "softvector.h"
#include "base/base.hpp"
#include "parameters.hpp"

#include "gtest/gtest.h"

extern std::string gLogDir;

class Case
{
  public:
    std::string identity;
    std::string type;
    std::string brief;
    std::string details;
    const std::string filepath_goldenOutput;

    std::vector<CaseParameter *> mPars;

    int run_return;
    int compare_return;

    Case(std::string &path_to_golden_file)
        : run_return(-1), compare_return(-1), filepath_goldenOutput(path_to_golden_file)
    {

        std::string fname = path_to_golden_file.substr(path_to_golden_file.rfind("/") + 1);
        identity = fname.substr(fname.find("Case-"), fname.rfind(".") - fname.find("Case-"));
        type = fname.substr(0, fname.find("Case-"));

        std::ifstream fh_golden_output;
        fh_golden_output.open(path_to_golden_file, std::ios::in);

        std::string line;
        std::vector<uint8_t> mem;

        bool inputparse = false, outputparse = false;

        while (std::getline(fh_golden_output, line))
        {
            if (line.find("#Brief:") != std::string::npos)
            {
                brief = line.substr(line.find(":") + 1);
            }
            else if (line.find("#Details:") != std::string::npos)
            {
                details = line.substr(line.find(":") + 1);
            }
        }
        fh_golden_output.close();
    }

    Case(const char *ident, const char *type, const char *path_to_goldenOutput, const char *brief = "")
        : identity(ident)
        , type(type)
        , filepath_goldenOutput(((std::string(path_to_goldenOutput) + type) + ident) + ".txt")
        , brief(brief)
        , run_return(-1)
        , compare_return(-1)
    {
    }

    virtual ~Case(void) {}

    virtual std::string rep_VECTORS(uint8_t *V, bool hex = true) = 0;
    virtual std::string rep_inVECTORS(bool hex = true) = 0;
    virtual std::string rep_outVECTORS(bool hex = true) = 0;
    virtual std::string rep_MEMORY(uint8_t *M, size_t size, int nlb = 32, bool hex = true) = 0;
    virtual std::string rep_inMEMORY(int nlb = 32, bool hex = true) = 0;
    virtual std::string rep_outMEMORY(int nlb = 32, bool hex = true) = 0;
    virtual std::string rep_ISET(void) = 0;
    virtual bool run(void) { return (0); };

    virtual int initParameters(void)
    {
        int ret = 1;
        for (auto &it : mPars)
        {
            if (it->initbyFile(filepath_goldenOutput.c_str()) <= 0)
            {
                ret = -1;
            }
        }
        return (ret);
    }

    virtual CaseParameter *getParameter(std::string name)
    {
        CaseParameter *ret = nullptr;
        for (auto &it : mPars)
        {
            if (it->name == name)
            {
                ret = it;
            }
        }
        return (ret);
    }

    virtual int compare_outputs(int dut_return)
    {
        std::string fp_case_output;
        std::ofstream fh_case_output;

        std::string exe = "mkdir -p " + gLogDir;

        system(exe.c_str());
        fp_case_output = gLogDir;
        fp_case_output += type;
        fp_case_output += identity + ".txt";

        fh_case_output.open(fp_case_output, std::ios::out);
        fh_case_output << "############################################################################################"
                          "##############"
                       << std::endl
                       << "#File: " << filepath_goldenOutput.substr(filepath_goldenOutput.rfind("/") + 1) << std::endl
                       << "#Brief:" << brief << std::endl
                       << "#Details:" << details << std::endl
                       << "############################################################################################"
                          "##############"
                       << std::endl
                       << std::endl
                       << "#Input:" << std::endl
                       << std::endl
                       << rep_ISET() << std::endl
                       << rep_inMEMORY() << std::endl
                       << rep_inVECTORS() << std::endl
                       << "#Output:" << std::endl
                       << std::endl
                       << rep_outMEMORY() << std::endl
                       << rep_outVECTORS() << std::endl
                       << "EXCEPTION:" << bool(dut_return);
        fh_case_output.close();

        if (!dut_return)
        {
            std::string diff = std::string("diff -BaiEZw ") + filepath_goldenOutput + std::string(" ") +
                               fp_case_output + std::string("\n");

            int diffret = system(diff.c_str());
            return (diffret);
        }
        else
        {
            std::ifstream fh_golden_output;
            fh_golden_output.open(filepath_goldenOutput, std::ios::in);
            std::stringstream x;
            char y;
            while (fh_golden_output >> y)
            {
                x << y;
            }
            fh_golden_output.close();

            if (x.str().find("EXCEPTION") == std::string::npos)
            {
                std::cout << "Golden output either missing or not specifying an EXCEPTION value" << std::endl;
                return (128);
            }
            else
            {
                if (x.str().find("EXCEPTION:1") != std::string::npos)
                {
                    return (0);
                }
                return (1);
            }
        }
    }
};

class VCase : public Case
{
  public:
    uint8_t _xlen{};
    uint16_t _sew{};
    uint8_t _lmul{};
    uint8_t _Zlmul{};
    uint8_t _Nlmul{};
    uint16_t _eew{};
    uint8_t _vd{};
    uint8_t _vstart{};
    uint16_t _vlen{};
    uint16_t _vl{};
    uint64_t _mstart{};
    uint8_t _vm{};

    uint8_t *_V{};
    uint8_t *_Vin{};
    uint8_t *_M{};
    uint8_t *_Min{};

    int initdone{};

    std::string rep_ISET(void)
    {
        std::stringstream ss;
        for (int k = 0; k < mPars.size(); ++k)
        {
            auto &it = mPars[k];
            if (it->basetype != CaseParameter::DATT::MEMORY and it->basetype != CaseParameter::DATT::VECTOR)
            {
                std::stringstream valss;
                if (it->basetype == CaseParameter::DATT::INT8)
                {
                    int8_t i = 0;
                    it->getVal(&i);
                    valss << int(i);
                }
                if (it->basetype == CaseParameter::DATT::INT16)
                {
                    int16_t i = 0;
                    it->getVal(&i);
                    valss << int(i);
                }
                if (it->basetype == CaseParameter::DATT::INT32)
                {
                    int32_t i = 0;
                    it->getVal(&i);
                    valss << int(i);
                }
                if (it->basetype == CaseParameter::DATT::INT64)
                {
                    int64_t i = 0;
                    it->getVal(&i);
                    valss << int(i);
                }
                else if (it->basetype == CaseParameter::DATT::UINT8)
                {
                    uint8_t u = 0;
                    it->getVal(&u);
                    valss << int(u);
                }
                else if (it->basetype == CaseParameter::DATT::UINT16)
                {
                    uint16_t u = 0;
                    it->getVal(&u);
                    valss << int(u);
                }
                else if (it->basetype == CaseParameter::DATT::UINT32)
                {
                    uint32_t u = 0;
                    it->getVal(&u);
                    valss << int(u);
                }
                else if (it->basetype == CaseParameter::DATT::UINT64)
                {
                    uint64_t u = 0;
                    it->getVal(&u);
                    valss << int(u);
                }
                else
                {
                }

                switch (it->_fracttype)
                {
                case CaseParameter::FRACT::Z:
                    ss << it->name << ":" << valss.str();
                    break;
                case CaseParameter::FRACT::N:
                    if (valss.str() != "1")
                    {
                        ss << "/" << valss.str();
                    }
                    ss << std::endl;
                    break;
                default:
                    ss << it->name << ":" << valss.str() << std::endl;
                    break;
                }
            }
        }
        return (ss.str());
    }

    std::string rep_VECTORS(uint8_t *V, bool hex = true)
    {
        std::stringstream ss;

        for (size_t x = 0; x < 32; ++x)
        {
            ss << "V" << x << "\t[";
            for (int i = _vlen / 8 - 1; i >= 0; --i)
            {
                if (!hex)
                {
                    for (int bit = 7; bit >= 0; --bit)
                    {
                        ss << (V[x * _vlen / 8 + i] & (1 << bit) ? 1 : 0);
                    }
                }
                else
                {
                    ss << std::setfill('0') << std::setw(sizeof(uint8_t) * 2) << std::hex << int(V[x * _vlen / 8 + i])
                       << std::dec;
                }
                if (i > 0)
                    ss << "|";
            }
            ss << "]" << std::endl;
        }
        return ss.str();
    }

    std::string rep_inVECTORS(bool hex = true) { return (rep_VECTORS(_Vin, hex)); }

    std::string rep_outVECTORS(bool hex = true) { return (rep_VECTORS(_V, hex)); }

    std::string rep_inMEMORY(int nlb = 32, bool hex = true)
    {
        if (_Min)
            return (rep_MEMORY(_Min, reinterpret_cast<MemoryParameter *>(getParameter("Memory"))->mSize, nlb, hex));
        else
            return ("");
    }

    std::string rep_outMEMORY(int nlb = 32, bool hex = true)
    {
        if (_M)
            return (rep_MEMORY(_M, reinterpret_cast<MemoryParameter *>(getParameter("Memory"))->mSize, nlb, hex));
        return ("");
    }

    std::string rep_MEMORY(uint8_t *M, size_t size, int nlb = 32, bool hex = true)
    {
        std::stringstream ss;
        auto nchunks = (size) / nlb + 1; // (_eew/8)/nlb +1;
        auto nibble = (size) % nlb;      // (_eew/8)%nlb;
        if (!nibble)
        {
            nibble = nlb;
            --nchunks;
        }
        for (size_t x = 0; x < nchunks; ++x)
        {
            ss << "M+" << x * nlb << "\t[";
            for (int i = nlb - 1; i >= 0; --i)
            {
                std::string payload = "";
                if (x == (nchunks - 1) && i >= nibble)
                {
                    payload = "--";
                }
                else
                {
                    if (!hex)
                    {
                        for (int bit = 7; bit >= 0; --bit)
                        {
                            payload += (M[x * nlb + i] & (1 << bit) ? "1" : "0");
                        }
                    }
                    else
                    {
                        std::stringstream _ss;
                        _ss << std::setfill('0') << std::setw(sizeof(uint8_t) * 2) << std::hex << int(M[x * nlb + i])
                            << std::dec;
                        payload = _ss.str();
                    }
                }
                ss << payload;
                if (i > 0)
                    ss << "|";
            }
            ss << "]" << std::endl;
        }
        return ss.str();
    }

    void init(void)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("XLEN", _xlen, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint16_t>("SEW", _sew, CaseParameter::DATT::UINT16));
        mPars.push_back(
            new FractParameter<uint8_t>("LMUL", _Zlmul, CaseParameter::FRACT::Z, CaseParameter::DATT::UINT8));
        mPars.push_back(
            new FractParameter<uint8_t>("LMUL", _Nlmul, CaseParameter::FRACT::N, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VD", _vd, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VSTART", _vstart, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint16_t>("VLEN", _vlen, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint16_t>("VL", _vl, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint8_t>("VM", _vm, CaseParameter::DATT::UINT8));

        mPars.push_back(new VectorParameter(&_V));
        mPars.push_back(new VectorParameter(&_Vin));

        if (initParameters() >= 0)
        {
            std::cout << "\033[0;36m"
                      << "[ sub-test ] "
                      << "\033[0m" << identity << " " << type << " - " << brief << std::endl;
            initdone = 1;
        }
        else
        {
            std::cout << "\033[0;31m"
                      << "[ error    ]"
                      << "\033[0m"
                         "\tFailed to initialize Test"
                      << std::endl;
            initdone = -1;
        }
    }

    VCase(std::string &path_to_golden_file) : Case(path_to_golden_file) {}

    virtual ~VCase(void)
    {
        if (_V)
            delete[](_V);
        if (_M)
            delete[](_M);
        if (_Vin)
            delete[](_Vin);
        if (_Min)
            delete[](_Min);
        for (auto &it : mPars)
        {
            delete (it);
        }
    }
};

class Cvadd_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvadd_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {

        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvadd_vv(void) {}

    bool run(void)
    {

        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vadd_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvadd_vi final : public VCase
{
  public:
    uint8_t _vs2{};
    int8_t _vimm{};

    Cvadd_vi(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int8_t>("VIMM", _vimm, CaseParameter::DATT::INT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvadd_vi(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vadd_vi(_V, vtype, _vm, _vd, _vs2, _vimm, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvadd_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvadd_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvadd_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vadd_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvsub_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvsub_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsub_vv(void) {}

    bool run(void)
    {

        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vsub_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvsub_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvsub_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsub_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);

        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vsub_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvand_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvand_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvand_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vand_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvand_vi final : public VCase
{
  public:
    uint8_t _vs2{};
    int8_t _vimm{};

    Cvand_vi(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int8_t>("VIMM", _vimm, CaseParameter::DATT::INT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvand_vi(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vand_vi(_V, vtype, _vm, _vd, _vs2, _vimm, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvand_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvand_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvand_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vand_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvor_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvor_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvor_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vor_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvor_vi final : public VCase
{
  public:
    uint8_t _vs2{};
    int8_t _vimm{};

    Cvor_vi(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int8_t>("VIMM", _vimm, CaseParameter::DATT::INT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvor_vi(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vor_vi(_V, vtype, _vm, _vd, _vs2, _vimm, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvor_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvor_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvor_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vor_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvxor_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvxor_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvxor_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vxor_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvxor_vi final : public VCase
{
  public:
    uint8_t _vs2{};
    int8_t _vimm{};

    Cvxor_vi(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int8_t>("VIMM", _vimm, CaseParameter::DATT::INT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvxor_vi(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vxor_vi(_V, vtype, _vm, _vd, _vs2, _vimm, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvxor_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvxor_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvxor_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vxor_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvsll_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvsll_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsll_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vsll_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvsll_vi final : public VCase
{
  public:
    uint8_t _vs2{};
    int8_t _vimm{};

    Cvsll_vi(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int8_t>("VIMM", _vimm, CaseParameter::DATT::INT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsll_vi(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vsll_vi(_V, vtype, _vm, _vd, _vs2, _vimm, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvsll_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvsll_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsll_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vsll_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvsrl_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvsrl_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsrl_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vsrl_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvsrl_vi final : public VCase
{
  public:
    uint8_t _vs2{};
    int8_t _vimm{};

    Cvsrl_vi(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int8_t>("VIMM", _vimm, CaseParameter::DATT::INT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsrl_vi(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vsrl_vi(_V, vtype, _vm, _vd, _vs2, _vimm, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvsrl_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvsrl_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsrl_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vsrl_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvsra_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvsra_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsra_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vsra_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvsra_vi final : public VCase
{
  public:
    uint8_t _vs2{};
    int8_t _vimm{};

    Cvsra_vi(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int8_t>("VIMM", _vimm, CaseParameter::DATT::INT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsra_vi(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vsra_vi(_V, vtype, _vm, _vd, _vs2, _vimm, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvsra_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvsra_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvsra_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vsra_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvload_encoded_unitstride final : public VCase
{
  public:
    Cvload_encoded_unitstride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvload_encoded_unitstride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vload_encoded_unitstride(_V, _M, vtype, _vm, _eew, _vd, _vstart, _vlen, _vl, _mstart);

        return (ret);
    }
};

class Cvstore_encoded_unitstride final : public VCase
{
  public:
    Cvstore_encoded_unitstride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvstore_encoded_unitstride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vstore_encoded_unitstride(_V, _M, vtype, _vm, _eew, _vd, _vstart, _vlen, _vl, _mstart);

        return (ret);
    }
};

class Cvload_encoded_stride final : public VCase
{
  public:
    int16_t _vstride{};

    Cvload_encoded_stride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new IntegerParameter<int16_t>("VSTRIDE", _vstride, CaseParameter::DATT::INT16));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvload_encoded_stride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vload_encoded_stride(_V, _M, vtype, _vm, _eew, _vd, _vstart, _vlen, _vl, _mstart, _vstride);

        return (ret);
    }
};

class Cvstore_encoded_stride final : public VCase
{
  public:
    int16_t _vstride{};

    Cvstore_encoded_stride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new IntegerParameter<int16_t>("VSTRIDE", _vstride, CaseParameter::DATT::INT16));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvstore_encoded_stride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vstore_encoded_stride(_V, _M, vtype, _vm, _eew, _vd, _vstart, _vlen, _vl, _mstart, _vstride);

        return (ret);
    }
};

class Cvload_segment_unitstride final : public VCase
{
  public:
    uint8_t _vnf{};

    Cvload_segment_unitstride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new IntegerParameter<uint8_t>("VNF", _vnf, CaseParameter::DATT::UINT8));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvload_segment_unitstride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vload_segment_unitstride(_V, _M, vtype, _vm, _eew, _vnf, _vd, _vstart, _vlen, _vl, _mstart);

        return (ret);
    }
};

class Cvstore_segment_unitstride final : public VCase
{
  public:
    uint8_t _vnf{};

    Cvstore_segment_unitstride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new IntegerParameter<uint8_t>("VNF", _vnf, CaseParameter::DATT::UINT8));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvstore_segment_unitstride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vstore_segment_unitstride(_V, _M, vtype, _vm, _eew, _vnf, _vd, _vstart, _vlen, _vl, _mstart);

        return (ret);
    }
};

class Cvload_segment_stride final : public VCase
{
  public:
    uint8_t _vnf{};
    int16_t _vstride{};

    Cvload_segment_stride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint8_t>("VNF", _vnf, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int16_t>("VSTRIDE", _vstride, CaseParameter::DATT::INT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvload_segment_stride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vload_segment_stride(_V, _M, vtype, _vm, _eew, _vnf, _vd, _vstart, _vlen, _vl, _mstart, _vstride);

        return (ret);
    }
};

class Cvstore_segment_stride final : public VCase
{
  public:
    uint8_t _vnf{};
    int16_t _vstride{};

    Cvstore_segment_stride(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint16_t>("EEW", _eew, CaseParameter::DATT::UINT16));
        mPars.push_back(new IntegerParameter<uint8_t>("VNF", _vnf, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int16_t>("VSTRIDE", _vstride, CaseParameter::DATT::INT16));
        mPars.push_back(new IntegerParameter<uint64_t>("MSTART", _mstart, CaseParameter::DATT::UINT64));
        mPars.push_back(new MemoryParameter(&_M));
        mPars.push_back(new MemoryParameter(&_Min));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvstore_segment_stride(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vstore_segment_stride(_V, _M, vtype, _vm, _eew, _vnf, _vd, _vstart, _vlen, _vl, _mstart, _vstride);

        return (ret);
    }
};
// 12.10
class Cvmul_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvmul_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmul_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vmul_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};
class Cvmul_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvmul_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmul_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vmul_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};
class Cvmulh_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvmulh_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmulh_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vmulh_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};
class Cvmulh_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvmulh_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmulh_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vmulh_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};
class Cvmulhu_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvmulhu_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmulhu_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vmulhu_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};
class Cvmulhu_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvmulhu_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmulhu_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vmulhu_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

class Cvmulhsu_vv final : public VCase
{
  public:
    uint8_t _vs1{};
    uint8_t _vs2{};

    Cvmulhsu_vv(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<uint8_t>("VS1", _vs1, CaseParameter::DATT::UINT8));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmulhsu_vv(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        auto ret = vmulhsu_vv(_V, vtype, _vm, _vd, _vs1, _vs2, _vstart, _vlen, _vl);

        return (ret);
    }
};

class Cvmulhsu_vx final : public VCase
{
  public:
    uint8_t _vs2{};
    int64_t _x{};

    Cvmulhsu_vx(std::string &path_to_golden_file) : VCase(path_to_golden_file)
    {
        mPars.push_back(new IntegerParameter<uint8_t>("VS2", _vs2, CaseParameter::DATT::UINT8));
        mPars.push_back(new IntegerParameter<int64_t>("X", _x, CaseParameter::DATT::INT64));
        init();
        if (initdone > 0)
        {
            run_return = run();
            compare_return = compare_outputs(run_return);
        }
    }

    virtual ~Cvmulhsu_vx(void) {}

    bool run(void)
    {
        uint16_t vtype = VTYPE::encode(_sew, _Zlmul, _Nlmul, 0, 0);
        rep_ISET();

        uint8_t *r = new uint8_t[_xlen / 8];
        memset(r, 0, _xlen / 8);
        for (int i = 0; i < _xlen / 8; ++i)
            r[i] = ((uint8_t *)(&_x))[i];

        auto ret = vmulhsu_vx(_V, r, vtype, _vm, _vd, _vs2, 0, _vstart, _vlen, _vl, _xlen / 8);

        delete[] r;

        return (ret);
    }
};

#endif /* __RVV_HL_TESTCASES_H__ */
