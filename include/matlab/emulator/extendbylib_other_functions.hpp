/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       matlab/emulator/extendbylib_other_functions.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <cesar.garcia@ingeniouskey.com>
 * @date       2019 Sep 1
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator
 *
 */

#ifndef MATLAB_EMULATOR_EXTENDBYLIB_OTHER_FUNCTIONS_HPP
#define MATLAB_EMULATOR_EXTENDBYLIB_OTHER_FUNCTIONS_HPP

#include <stdint.h>
#include <matlab/emulator/extendbylib_common.hpp>
#include <matrix.h>


BEGIN_C_DECL2


MATEMUL_EDL_EXPORT int PutVariableToEmulatorWorkspace(const char* variableName, mxArray* var);
MATEMUL_EDL_EXPORT int PutVariableToMatlabWorkspace(const char* variableName, mxArray* var);


END_C_DECL2

//namespace matlab { namespace emulator {
//}}  // namespace matlab { namespace emulator {
#endif  // #ifndef MATLAB_EMULATOR_EXTENDBYLIB_OTHER_FUNCTIONS_HPP
