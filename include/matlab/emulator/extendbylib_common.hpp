/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       matlab/emulator/extendbylib_common.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <cesar.garcia@ingeniouskey.com>
 * @date       2019 Sep 1
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator
 *
 */

#ifndef MATLAB_EMULATOR_EXTENDBYLIB_COMMON_HPP
#define MATLAB_EMULATOR_EXTENDBYLIB_COMMON_HPP

#include <common/matlabemulator_compiler_internal.h>

#ifdef MATEMUL_EDL_CREATING_DYN_LIB
#define MATEMUL_EDL_EXPORT  EXPORT_FROM_LIB2
#elif defined(MATEMUL_USING_SOURCES_OR_STAT_LIB)
#define MATEMUL_EDL_EXPORT
#else
#define MATEMUL_EDL_EXPORT  IMPORT_FROM_LIB2
#endif


BEGIN_C_DECL2

typedef const char* (*FncTypeEmExtension)(const char*);

END_C_DECL2

//namespace matlab { namespace emulator {
//}}  // namespace matlab { namespace emulator {
#endif  // #ifndef MATLAB_EMULATOR_EXTENDBYLIB_COMMON_HPP
