/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       matlab/emulator/extendbylib.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <cesar.garcia@ingeniouskey.com>
 * @date       2019 Sep 1
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator
 *
 */

#ifndef MATLAB_EMULATOR_EXTENDBYLIB_HPP
#define MATLAB_EMULATOR_EXTENDBYLIB_HPP

#include <common/matlabemulator_compiler_internal.h>

#define MTLAB_EMUL_TABLE_NAME   "g_vcMatlabEmulatorTable"
// this variable should be defined like this (NULL termination is important)
// EXPORT_FROM_LIB2 EXTERN_C2 const TEmTableEntry g_vcMatlabEmulatorTable[] = {{func,"This is help"},{NULL}};
// or
// EXPORT_FROM_LIB2 EXTERN_C2 TEmTableEntry g_vcMatlabEmulatorTable[] = {{func,"This is help"},{NULL}};
// const qualifier is not mandatory. Please do not forgot keywords 'EXPORT_FROM_LIB2' in order to place
// your table in the library lookup table. nd do not forgot keyword 'EXTERN_C2' in order to make sure
// that no decoration will be used for the symbol. For GCC in the case of variable there is no decoration,
// but the same is not true for Microsoft compiler
// the functions used in the table can be hidden (even static)

BEGIN_C_DECL2

typedef const char* (*FncTypeEmTableEntry)(const char*);

typedef struct SEmTableEntry{
    FncTypeEmTableEntry     function;
    const char*             functionName; // can be null
    const char*             helpString; // can be null
}TEmTableEntry;


END_C_DECL2



namespace matlab { namespace emulator {

//

}}  // namespace matlab { namespace emulator {


#endif  // #ifndef MATLAB_EMULATOR_EXTENDBYLIB_HPP
