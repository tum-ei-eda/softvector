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
#ifndef __RVV_HL_TESTPAR_H__
#define __RVV_HL_TESTPAR_H__

#include <sstream>
#include <iomanip>
#include <stdlib.h>
#include <fstream>
#include <iostream>

class CaseParameter
{
  public:
    typedef enum FRACT
    {
        Z,
        N,
        NONE
    } FRACT_t;

    typedef enum DATT
    {
        INT8,
        INT16,
        INT32,
        INT64,
        STRING,
        UINT8,
        UINT16,
        UINT32,
        UINT64,
        BOOLEAN,
        FLOAT,
        FRACT,
        VECTOR,
        MEMORY
    } datt_t;

    std::string name;
    datt_t basetype;
    FRACT_t _fracttype;

    CaseParameter(std::string name, datt_t basetype, FRACT_t fractype = FRACT::NONE)
        : name(name), basetype(basetype), _fracttype(fractype)
    {
    }

    virtual ~CaseParameter(void){};

    virtual void getVal(void *out) = 0;
    virtual int initbyFile(const char *fp_goldenFile) = 0;
};

template <typename base_t = int>
class IntegerParameter final : public CaseParameter
{
  public:
    base_t &mVal;

    IntegerParameter(std::string name, base_t &valueref, datt_t basetype = DATT::INT32)
        : CaseParameter(name, basetype), mVal(valueref)
    {
    }

    int initbyFile(const char *fp_goldenFile)
    {
        std::ifstream fh_golden_output;
        fh_golden_output.open(fp_goldenFile, std::ios::in);
        std::string line;
        bool inputparse = false, outputparse = false;

        while (std::getline(fh_golden_output, line))
        {
            if (line.find("#Input:") != std::string::npos)
            {
                inputparse = true;
                outputparse = true;
            }
            else if (line.find("#Output:") != std::string::npos)
            {
                inputparse = false;
                outputparse = true;
            }
            if (inputparse)
            {
                std::istringstream iss(line);
                size_t _pos = iss.str().find(name + ":");
                if (_pos != std::string::npos)
                {
                    std::string strVal = iss.str().substr(_pos + name.size() + 1);
                    mVal = std::stoi(strVal);
                }
            }
        }
        fh_golden_output.close();

        return 1;
    }

    void getVal(void *out) { *(reinterpret_cast<base_t *>(out)) = mVal; }
};

template <typename base_t = int>
class FractParameter final : public CaseParameter
{

  public:
    base_t &mVal;

    FractParameter(std::string name, base_t &valueref, FRACT_t fracttype, datt_t basetype = DATT::UINT32)
        : CaseParameter(name, basetype, fracttype), mVal(valueref)
    {
    }

    int initbyFile(const char *fp_goldenFile)
    {
        std::ifstream fh_golden_output;
        fh_golden_output.open(fp_goldenFile, std::ios::in);

        std::string line;

        bool inputparse = false, outputparse = false;

        while (std::getline(fh_golden_output, line))
        {
            if (line.find("#Input:") != std::string::npos)
            {
                inputparse = true;
                outputparse = true;
            }
            else if (line.find("#Output:") != std::string::npos)
            {
                inputparse = false;
                outputparse = true;
            }
            if (inputparse)
            {
                std::istringstream iss(line);
                size_t _pos = iss.str().find(name + ":");
                if (_pos != std::string::npos)
                {
                    std::string valstring = iss.str().substr(_pos + name.size() + 1);
                    size_t _fracpos = valstring.find('/');

                    if (_fracpos != std::string::npos)
                    {
                        if (_fracttype == FRACT::Z)
                        {
                            mVal = std::stoi(valstring.substr(0, _fracpos));
                        }
                        else
                        {
                            mVal = std::stoi(valstring.substr(_fracpos + 1));
                        }
                    }
                    else
                    {
                        if (_fracttype == FRACT::Z)
                        {
                            mVal = std::stoi(valstring);
                        }
                        else
                        {
                            mVal = 1;
                        }
                    }
                }
            }
        }
        fh_golden_output.close();

        return 1;
    }

    void getVal(void *out) { *(reinterpret_cast<base_t *>(out)) = mVal; }
};

class VectorParameter final : public CaseParameter
{
  public:
    uint8_t **mVal;
    size_t mSize;

    VectorParameter(uint8_t **valueref) : CaseParameter("Vector", DATT::VECTOR), mVal(valueref), mSize() {}

    int initbyFile(const char *fp_goldenFile)
    {
        std::ifstream fh_golden_output;
        fh_golden_output.open(fp_goldenFile, std::ios::in);

        std::string line;
        std::vector<uint8_t> mem;

        bool inputparse = false, outputparse = false;

        while (std::getline(fh_golden_output, line))
        {
            if (line.find("#Input:") != std::string::npos)
            {
                inputparse = true;
                outputparse = true;
            }
            else if (line.find("#Output:") != std::string::npos)
            {
                inputparse = false;
                outputparse = true;
            }
            if (inputparse)
            {
                std::vector<uint8_t> tmpmem;
                size_t _pos = std::string::npos;
                if (line[0] == 'V')
                {
                    _pos = 0;
                };
                if (_pos != std::string::npos)
                {
                    size_t _contentstart = line.find("[", _pos);
                    size_t _contentend = line.find("]", _pos);

                    if (_contentstart != std::string::npos && _contentend != std::string::npos)
                    {
                        std::string contentstr = line.substr(_contentstart + 1, _contentend - (_contentstart + 1));
                        size_t bytepos = std::string::npos;

                        size_t bytecnt = 0;
                        do
                        {
                            bytepos = contentstr.find("|");
                            std::string valstr = contentstr.substr(0, 2);
                            if (valstr != "--")
                            {
                                tmpmem.push_back(uint8_t(std::stoi(valstr, 0, 16)));
                            }
                            if (contentstr.size() > 3)
                                contentstr = contentstr.substr(3);
                        } while (bytepos != std::string::npos);
                    }
                }
                if (tmpmem.size() > 0)
                {
                    for (size_t i = tmpmem.size(); i > 0; --i)
                    {
                        mem.push_back(tmpmem[i - 1]);
                    }
                }
            }
        }
        fh_golden_output.close();

        if (mem.size() > 0)
        {
            *mVal = new uint8_t[mem.size()];
            mSize = mem.size();
            for (size_t i = 0; i < mem.size(); ++i)
                (*mVal)[i] = mem[i];
        }
        else
        {
            return -1;
        }

        return 1;
    }

    void getVal(void *out)
    {
        //*(reinterpret_cast<base_t*>(out)) = mVal;
    }
};

class MemoryParameter final : public CaseParameter
{
  public:
    uint8_t **mVal;
    size_t mSize;

    MemoryParameter(uint8_t **valueref) : CaseParameter("Memory", DATT::MEMORY), mVal(valueref), mSize() {}

    int initbyFile(const char *fp_goldenFile)
    {
        std::ifstream fh_golden_output;
        fh_golden_output.open(fp_goldenFile, std::ios::in);
        std::string line;
        std::vector<uint8_t> mem;
        bool inputparse = false, outputparse = false;

        while (std::getline(fh_golden_output, line))
        {
            if (line.find("#Input:") != std::string::npos)
            {
                inputparse = true;
                outputparse = true;
            }
            else if (line.find("#Output:") != std::string::npos)
            {
                inputparse = false;
                outputparse = true;
            }
            if (inputparse)
            {
                std::vector<uint8_t> tmpmem;
                // std::istringstream iss(line);
                size_t _pos = line.find("M+");
                if (_pos != std::string::npos)
                {
                    size_t _contentstart = line.find("[", _pos);
                    size_t _contentend = line.find("]", _pos);

                    std::string contentstr = line.substr(_contentstart + 1, _contentend - (_contentstart + 1));
                    size_t bytepos = std::string::npos;

                    size_t bytecnt = 0;
                    do
                    {
                        bytepos = contentstr.find("|");
                        std::string valstr = contentstr.substr(0, 2);
                        if (valstr != "--")
                        {
                            tmpmem.push_back(uint8_t(std::stoi(valstr, 0, 16)));
                        }
                        if (contentstr.size() > 3)
                            contentstr = contentstr.substr(3);
                    } while (bytepos != std::string::npos);
                }
                if (tmpmem.size() > 0)
                {
                    for (size_t i = tmpmem.size(); i > 0; --i)
                    {
                        mem.push_back(tmpmem[i - 1]);
                    }
                }
            }
        }
        fh_golden_output.close();

        if (mem.size() > 0)
        {
            *mVal = new uint8_t[mem.size()];
            mSize = mem.size();
            for (size_t i = 0; i < mem.size(); ++i)
                (*mVal)[i] = mem[i];
        }
        else
        {
            return -1;
        }

        return 1;
    }

    void getVal(void *out)
    {
        //*(reinterpret_cast<base_t*>(out)) = mVal;
    }
};

#endif /* __RVV_HL_TESTPAR_H__ */
