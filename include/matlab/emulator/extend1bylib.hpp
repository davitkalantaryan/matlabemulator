/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       matlab/emulator/extend1bylib.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <cesar.garcia@ingeniouskey.com>
 * @date       2019 Sep 1
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator (method 1) \n
 *   No library is needed in order to use functionalities described in this header
 *
 */

#ifndef MATLAB_EMULATOR_EXTEND1BYLIB_HPP
#define MATLAB_EMULATOR_EXTEND1BYLIB_HPP

#include <stddef.h>
#include <matlab/emulator/extendbylib_common.hpp>

#define MTLAB_EMUL_TABLE_NAME   "g_vcMatlabEmulatorTable"
// this variable is global and should be defined like this (one can use macros 'DECLARE_EM_TABLE' below)
// EXPORT_FROM_LIB2 VARIABLE_C2 const TEmTableEntry g_vcMatlabEmulatorTable[] = {{func,"This is help"},{NULL}};
// or
// EXPORT_FROM_LIB2 VARIABLE_C2 TEmTableEntry g_vcMatlabEmulatorTable[] = {{func,"This is help"},{NULL}};
// (NULL termination is important)
// const qualifier is not mandatory. Please do not forgot keywords 'EXPORT_FROM_LIB2' in order to place
// your table in the library lookup table. nd do not forgot keyword 'EXTERN_C2' in order to make sure
// that no decoration will be used for the symbol. For GCC in the case of variable there is no decoration,
// but the same is not true for Microsoft compiler
// the functions used in the table can be hidden (even static)

#define DECLARE_EM_TABLE()    \
EXTERN_C2 const TEmTableEntry g_vcMatlabEmulatorTable[]; \
EXPORT_FROM_LIB2 VARIABLE_C2 const TEmTableEntry g_vcMatlabEmulatorTable[]
// example of usage of the macros 'DECLARE_EM_TABLE'
/*
DECLARE_EM_TABLE() = {
    {TestFunction1,"Func1","This is help1"},
    {TestFunction2,"Func2",nullptr},
    {TestFunction3,nullptr,nullptr},
    {nullptr,nullptr,nullptr}
};
 */

BEGIN_C_DECL2

typedef struct SEmTableEntry{
    FncTypeEmExtension      function;
    const char*             functionName; // can be null
    const char*             helpString; // can be null
}TEmTableEntry;

END_C_DECL2

//namespace matlab { namespace emulator {
//}}  // namespace matlab { namespace emulator {
#endif  // #ifndef MATLAB_EMULATOR_EXTEND1BYLIB_HPP
