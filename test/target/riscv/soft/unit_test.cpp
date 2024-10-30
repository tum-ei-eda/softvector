/*
 * Copyright [2020] [Technical University of Munich]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "cases.hpp"
#include "softvector.h"
#include "gtest/gtest.h"
#include <stdio.h>
#include "stdint.h"
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <dirent.h>
#include <sys/types.h>

std::string gGoldenDir = "golden/";
std::string gLogDir = "log/";

void read_directory(const std::string &name, std::vector<std::string> &v)
{
    DIR *dirp = opendir(name.c_str());
    struct dirent *dp;
    while ((dp = readdir(dirp)) != NULL)
    {
        v.push_back(dp->d_name);
    }
    closedir(dirp);
}

class Cvload_encoded_unitstrideTest : public ::testing::Test
{
  public:
    std::vector<Cvload_encoded_unitstride *> cases;

    Cvload_encoded_unitstrideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vload_encoded_unitstride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvload_encoded_unitstride(fp));
            }
        }
    }
    virtual ~Cvload_encoded_unitstrideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvstore_encoded_unitstrideTest : public ::testing::Test
{
  public:
    std::vector<Cvstore_encoded_unitstride *> cases;
    Cvstore_encoded_unitstrideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vstore_encoded_unitstride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvstore_encoded_unitstride(fp));
            }
        }
    }
    virtual ~Cvstore_encoded_unitstrideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvload_encoded_strideTest : public ::testing::Test
{
  public:
    std::vector<Cvload_encoded_stride *> cases;

    Cvload_encoded_strideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vload_encoded_stride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvload_encoded_stride(fp));
            }
        }
    }
    virtual ~Cvload_encoded_strideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvstore_encoded_strideTest : public ::testing::Test
{
  public:
    std::vector<Cvstore_encoded_stride *> cases;
    Cvstore_encoded_strideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vstore_encoded_stride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvstore_encoded_stride(fp));
            }
        }
    }
    virtual ~Cvstore_encoded_strideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvload_segment_unitstrideTest : public ::testing::Test
{
  public:
    std::vector<Cvload_segment_unitstride *> cases;
    Cvload_segment_unitstrideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vload_segment_unitstride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvload_segment_unitstride(fp));
            }
        }
    }
    virtual ~Cvload_segment_unitstrideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvstore_segment_unitstrideTest : public ::testing::Test
{
  public:
    std::vector<Cvstore_segment_unitstride *> cases;
    Cvstore_segment_unitstrideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vstore_segment_unitstride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvstore_segment_unitstride(fp));
            }
        }
    }
    virtual ~Cvstore_segment_unitstrideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvload_segment_strideTest : public ::testing::Test
{
  public:
    std::vector<Cvload_segment_stride *> cases;
    Cvload_segment_strideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vload_segment_stride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvload_segment_stride(fp));
            }
        }
    }
    virtual ~Cvload_segment_strideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvstore_segment_strideTest : public ::testing::Test
{
  public:
    std::vector<Cvstore_segment_stride *> cases;
    Cvstore_segment_strideTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vstore_segment_stride") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvstore_segment_stride(fp));
            }
        }
    }
    virtual ~Cvstore_segment_strideTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvadd_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvadd_vv *> cases;
    Cvadd_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vadd_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvadd_vv(fp));
            }
        }
    }
    virtual ~Cvadd_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvadd_viTest : public ::testing::Test
{
  public:
    std::vector<Cvadd_vi *> cases;
    Cvadd_viTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vadd_vi") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvadd_vi(fp));
            }
        }
    }
    virtual ~Cvadd_viTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvadd_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvadd_vx *> cases;
    Cvadd_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vadd_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvadd_vx(fp));
            }
        }
    }
    virtual ~Cvadd_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsub_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvsub_vv *> cases;
    Cvsub_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsub_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsub_vv(fp));
            }
        }
    }
    virtual ~Cvsub_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsub_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvsub_vx *> cases;
    Cvsub_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsub_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsub_vx(fp));
            }
        }
    }
    virtual ~Cvsub_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvand_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvand_vv *> cases;
    Cvand_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vand_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvand_vv(fp));
            }
        }
    }
    virtual ~Cvand_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvand_viTest : public ::testing::Test
{
  public:
    std::vector<Cvand_vi *> cases;
    Cvand_viTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vand_vi") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvand_vi(fp));
            }
        }
    }
    virtual ~Cvand_viTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvand_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvand_vx *> cases;
    Cvand_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vand_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvand_vx(fp));
            }
        }
    }
    virtual ~Cvand_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvor_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvor_vv *> cases;
    Cvor_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vor_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvor_vv(fp));
            }
        }
    }
    virtual ~Cvor_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvor_viTest : public ::testing::Test
{
  public:
    std::vector<Cvor_vi *> cases;
    Cvor_viTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vor_vi") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvor_vi(fp));
            }
        }
    }
    virtual ~Cvor_viTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvor_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvor_vx *> cases;
    Cvor_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vor_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvor_vx(fp));
            }
        }
    }
    virtual ~Cvor_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvxor_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvxor_vv *> cases;
    Cvxor_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vxor_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvxor_vv(fp));
            }
        }
    }
    virtual ~Cvxor_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvxor_viTest : public ::testing::Test
{
  public:
    std::vector<Cvxor_vi *> cases;
    Cvxor_viTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vxor_vi") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvxor_vi(fp));
            }
        }
    }
    virtual ~Cvxor_viTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvxor_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvxor_vx *> cases;
    Cvxor_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vxor_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvxor_vx(fp));
            }
        }
    }
    virtual ~Cvxor_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsll_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvsll_vv *> cases;
    Cvsll_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsll_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsll_vv(fp));
            }
        }
    }
    virtual ~Cvsll_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsll_viTest : public ::testing::Test
{
  public:
    std::vector<Cvsll_vi *> cases;
    Cvsll_viTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsll_vi") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsll_vi(fp));
            }
        }
    }
    virtual ~Cvsll_viTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsll_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvsll_vx *> cases;
    Cvsll_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsll_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsll_vx(fp));
            }
        }
    }
    virtual ~Cvsll_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsrl_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvsrl_vv *> cases;
    Cvsrl_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsrl_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsrl_vv(fp));
            }
        }
    }
    virtual ~Cvsrl_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsrl_viTest : public ::testing::Test
{
  public:
    std::vector<Cvsrl_vi *> cases;
    Cvsrl_viTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsrl_vi") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsrl_vi(fp));
            }
        }
    }
    virtual ~Cvsrl_viTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsrl_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvsrl_vx *> cases;
    Cvsrl_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsrl_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsrl_vx(fp));
            }
        }
    }
    virtual ~Cvsrl_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsra_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvsra_vv *> cases;
    Cvsra_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsra_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsra_vv(fp));
            }
        }
    }
    virtual ~Cvsra_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsra_viTest : public ::testing::Test
{
  public:
    std::vector<Cvsra_vi *> cases;
    Cvsra_viTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsra_vi") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsra_vi(fp));
            }
        }
    }
    virtual ~Cvsra_viTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvsra_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvsra_vx *> cases;
    Cvsra_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vsra_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvsra_vx(fp));
            }
        }
    }
    virtual ~Cvsra_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};
// 12.10 mul
class Cvmul_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvmul_vv *> cases;
    Cvmul_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmul_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmul_vv(fp));
            }
        }
    }
    virtual ~Cvmul_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvmul_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvmul_vx *> cases;
    Cvmul_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmul_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmul_vx(fp));
            }
        }
    }
    virtual ~Cvmul_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvmulh_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvmulh_vv *> cases;
    Cvmulh_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmulh_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmulh_vv(fp));
            }
        }
    }
    virtual ~Cvmulh_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvmulh_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvmulh_vx *> cases;
    Cvmulh_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmulh_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmulh_vx(fp));
            }
        }
    }
    virtual ~Cvmulh_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvmulhu_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvmulhu_vv *> cases;
    Cvmulhu_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmulhu_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmulhu_vv(fp));
            }
        }
    }
    virtual ~Cvmulhu_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvmulhu_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvmulhu_vx *> cases;
    Cvmulhu_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmulhu_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmulhu_vx(fp));
            }
        }
    }
    virtual ~Cvmulhu_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvmulhsu_vvTest : public ::testing::Test
{
  public:
    std::vector<Cvmulhsu_vv *> cases;
    Cvmulhsu_vvTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmulhsu_vv") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmulhsu_vv(fp));
            }
        }
    }
    virtual ~Cvmulhsu_vvTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};

class Cvmulhsu_vxTest : public ::testing::Test
{
  public:
    std::vector<Cvmulhsu_vx *> cases;
    Cvmulhsu_vxTest(void)
    {

        std::vector<std::string> golden_files;
        std::vector<std::string> case_files;

        read_directory(gGoldenDir.c_str(), golden_files);
        for (auto &_f : golden_files)
        {
            if (_f.find("vmulhsu_vx") != std::string::npos)
            {
                std::string fp = gGoldenDir + _f;
                cases.push_back(new Cvmulhsu_vx(fp));
            }
        }
    }
    virtual ~Cvmulhsu_vxTest(void)
    {
        for (auto &_case : cases)
        {
            delete _case;
        }
    }
};
TEST(vtype_decode, HandleBitfieldEncodingZLMULgtNLMUL)
{
    uint16_t x;
    uint32_t sew;
    uint8_t z_lmul;
    uint8_t n_lmul;
    uint8_t ta;
    uint8_t ma;
    uint16_t vtype = 0x52;
    x = vtype_decode(vtype, &ta, &ma, &sew, &z_lmul, &n_lmul);
    EXPECT_EQ(x, 1);
    EXPECT_EQ(ta, 1);
    EXPECT_EQ(ma, 0);
    EXPECT_EQ(sew, 32);
    EXPECT_EQ(z_lmul, 4); // LMUL=4
    EXPECT_EQ(n_lmul, 1);
}

TEST(vtype_encode, HandleBitfieldEncodingZLMULgtNLMUL)
{
    uint16_t x;
    uint16_t sew = 32;
    uint8_t z_lmul = 8;
    uint8_t n_lmul = 2;
    uint8_t ta = 1;
    uint8_t ma = 0;
    x = vtype_encode(sew, z_lmul, n_lmul, ta, ma);
    EXPECT_EQ(x, 0x52);
}

TEST(vtype_encode, HandleBitfieldEncodingZLMULltNLMUL)
{
    uint16_t x;
    uint16_t sew = 64;
    uint8_t z_lmul = 2;
    uint8_t n_lmul = 8;
    uint8_t ta = 0;
    uint8_t ma = 1;
    x = vtype_encode(sew, z_lmul, n_lmul, ta, ma);
    EXPECT_EQ(x, ma << 7 | ta << 6 | (/*sew*/ 0b011 << 3) | /*lmul*/ 0b110);
    z_lmul = 1;
    n_lmul = 2;
    x = vtype_encode(sew, z_lmul, n_lmul, ta, ma);
    EXPECT_EQ(x, ma << 7 | ta << 6 | (/*sew*/ 0b011 << 3) | /*lmul*/ 0b111);
    z_lmul = 1;
    n_lmul = 4;
    x = vtype_encode(sew, z_lmul, n_lmul, ta, ma);
    EXPECT_EQ(x, ma << 7 | ta << 6 | (/*sew*/ 0b011 << 3) | /*lmul*/ 0b110);
    z_lmul = 1;
    n_lmul = 8;
    x = vtype_encode(sew, z_lmul, n_lmul, ta, ma);
    EXPECT_EQ(x, ma << 7 | ta << 6 | (/*sew*/ 0b011 << 3) | /*lmul*/ 0b101);
}

TEST(vtype_extractSEWTest, HandleBitFieldExtract)
{
    uint8_t x;
    x = vtype_extractSEW(0xFFFF);
    EXPECT_EQ(x, 0x7);
    x = vtype_extractSEW(0x0000);
    EXPECT_EQ(x, 0);
}

TEST(vtype_extractLMULTest, HandleBitFieldExtract)
{
    uint8_t x;
    x = vtype_extractLMUL(0xFFFF);
    EXPECT_EQ(x, 0x7);
    x = vtype_extractLMUL(0x0000);
    EXPECT_EQ(x, 0x0);
}

TEST(vtype_extractTATest, HandleBitFieldExtract)
{
    uint8_t x;
    x = vtype_extractTA(0xFFFF);
    EXPECT_EQ(x, 0x1);
    x = vtype_extractTA(0x0000);
    EXPECT_EQ(x, 0x0);
}

TEST(vtype_extractMATest, HandleBitFieldExtract)
{
    uint8_t x;
    x = vtype_extractMA(0xFFFF);
    EXPECT_EQ(x, 0x1);
    x = vtype_extractMA(0x0000);
    EXPECT_EQ(x, 0x0);
}

TEST_F(Cvload_encoded_unitstrideTest, VariousTestCases)
{
    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvstore_encoded_unitstrideTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvload_encoded_strideTest, VariousTestCases)
{
    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvstore_encoded_strideTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvload_segment_unitstrideTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvstore_segment_unitstrideTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvload_segment_strideTest, VariousTestCases)
{
    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvstore_segment_strideTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvadd_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvadd_viTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvadd_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsub_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsub_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvand_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvand_viTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvand_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvor_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvor_viTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvor_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvxor_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvxor_viTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvxor_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsll_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsll_viTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsll_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsrl_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsrl_viTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsrl_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsra_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsra_viTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvsra_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

TEST_F(Cvmul_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}
TEST_F(Cvmul_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}
TEST_F(Cvmulh_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}
TEST_F(Cvmulh_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}
TEST_F(Cvmulhu_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}
TEST_F(Cvmulhu_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}
TEST_F(Cvmulhsu_vvTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}
TEST_F(Cvmulhsu_vxTest, VariousTestCases)
{

    for (auto &_case : cases)
    {
        EXPECT_EQ(_case->compare_return, 0) << "Fail at golden compare " << _case->identity;
    }
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    for (int i = 1; i < argc; ++i)
    {
        if (i != argc - 1)
        {
            if (std::string("--golden") == argv[i])
            {
                std::cout << "Golden directory: " << argv[i + 1] << std::endl;
                gGoldenDir = argv[i + 1];
                gGoldenDir += '/';
            }
            if (std::string("--log") == argv[i])
            {
                std::cout << "Log directory: " << argv[i + 1] << std::endl;
                gLogDir = argv[i + 1];
                gLogDir += '/';
            }
        }
    }
    return RUN_ALL_TESTS();
}
