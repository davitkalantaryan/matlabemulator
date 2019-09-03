/*
 *  Copyright (C) DESY For details refers to LICENSE.md
 *
 *  Written by Davit Kalantaryan <davit.kalantaryan@desy.de>
 *
 */

/**
 *
 * @file       matlab/emulator/extendbylib_print_functions.hpp
 * @copyright  Copyright (C) DESY For details refers to LICENSE.md
 * @brief      Header for providing API to comunicate with extensions
 * @author     Davit Kalantaryan <davit.kalantaryan@desy.de>
 * @date       2019 Sep 1
 * @details
 *   Include file declars symbols necessary for extending MATLAB emulator
 *
 */

#ifndef MATLAB_EMULATOR_EXTENDBYLIB_PRINT_FUNCTIONS_HPP
#define MATLAB_EMULATOR_EXTENDBYLIB_PRINT_FUNCTIONS_HPP

#include <stdint.h>
#include <matlab/emulator/extendbylib_common.hpp>


BEGIN_C_DECL2

typedef struct SMatEmRGB{
    uint8_t rd,gr,bl;
    //uint8_t reserved;
}TMatEmRGB;


MATEMUL_EDL_EXPORT int MatEmPrintStandard(const char* format, ...);
MATEMUL_EDL_EXPORT int MatEmPrintError(const char* format, ...);
MATEMUL_EDL_EXPORT int MatEmPrintWarning(const char* format, ...);
MATEMUL_EDL_EXPORT int MatEmPrintColored(TMatEmRGB color, const char* format, ...);


END_C_DECL2

//namespace matlab { namespace emulator {
//}}  // namespace matlab { namespace emulator {
#endif  // #ifndef MATLAB_EMULATOR_EXTENDBYLIB_PRINT_FUNCTIONS_HPP
