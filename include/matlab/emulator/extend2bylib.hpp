/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       matlab/emulator/extend2bylib.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 1
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator (method 2)  \n
 *   No library is needed in order to use functionalities described in this header
 *
 */

#ifndef MATLAB_EMULATOR_EXTEND2BYLIB_HPP
#define MATLAB_EMULATOR_EXTEND2BYLIB_HPP

#include <matlab/emulator/extendbylib_common.hpp>

#define MTLAB_EMULATOR_ENTRY        "MatlabEmulatorEntry"
#define MTLAB_EMULATOR_ENTRY_HELP   "g_matlabEmulatorEntryHelp"
// the function should be declared like this
// EXPORT_FROM_LIB2 EXTERN_C2 const char* MatlabEmulatorEntry(const char* a_cpcInputLine)
//
// the help variable should be declared globally like this
// 1. EXPORT_FROM_LIB2 VARIABLE_C2 const char* g_matlabEmulatorEntryHelp = "This is help from MatlabEmulatorEntry";  // or
// 2. EXPORT_FROM_LIB2 VARIABLE_C2 char*       g_matlabEmulatorEntryHelp = "This is help from MatlabEmulatorEntry";  // or
// 3. EXPORT_FROM_LIB2 VARIABLE_C2 const char  g_matlabEmulatorEntryHelp[] = "This is help from MatlabEmulatorEntry";  // or
// 4. EXPORT_FROM_LIB2 VARIABLE_C2 char        g_matlabEmulatorEntryHelp[] = "This is help from MatlabEmulatorEntry";
// The value can be equal to null

BEGIN_C_DECL2


END_C_DECL2


//namespace matlab { namespace emulator {
//}}  // namespace matlab { namespace emulator {
#endif  // #ifndef MATLAB_EMULATOR_EXTEND2BYLIB_HPP
