/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       entry_test_mex_root_crash.cpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 3
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator (method 1) \n
 *   No library is needed in order to use functionalities described in this header
 *
 */

#include <mex.h>

//void mexFunction (int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[]) __attribute__ ((unused));
//extern "C" int mexPrintf(const char	*/* printf style format */,.../* any additional arguments */)  __attribute__((weak));

void mexFunction(int a_nNumOuts, mxArray *a_Outputs[],int a_nNumInps, const mxArray*a_Inputs[])
{
    mexPrintf("version: 5. ni=%d, inps=%p, no=%d, outs=%p\n",a_nNumInps,a_Inputs,a_nNumOuts,a_Outputs);

}
