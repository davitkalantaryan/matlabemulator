/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       matlab/emulator/extendbyexe_functions.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 1
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator
 *
 */

#ifndef MATLAB_EMULATOR_EXTENDBYEXE_FUNCTIONS_HPP
#define MATLAB_EMULATOR_EXTENDBYEXE_FUNCTIONS_HPP

#include <stdint.h>
#include <common/matlabemulator_compiler_internal.h>
#include <matrix.h>

#ifdef MATEMUL_EE_CREATING_DYN_LIB
#define MATEMUL_EE_EXPORT  EXPORT_FROM_LIB2
#elif defined(MATEMUL_EE_USING_SOURCES_OR_STAT_LIB)
#define MATEMUL_EE_EXPORT
#else
#define MATEMUL_EE_EXPORT  IMPORT_FROM_LIB2
#endif


BEGIN_C_DECL2


MATEMUL_EE_EXPORT int PutVariableToEmulatorWorkspaceFromBin(const char* variableName, mxArray* var);
MATEMUL_EE_EXPORT int PutVariableToMatlabWorkspaceFromBin(const char* variableName, mxArray* var);


END_C_DECL2

//namespace matlab { namespace emulator {
//}}  // namespace matlab { namespace emulator {
#endif  // #ifndef MATLAB_EMULATOR_EXTENDBYEXE_FUNCTIONS_HPP
