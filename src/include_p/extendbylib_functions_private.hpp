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

#ifndef MATLAB_EMULATOR_EXTENDBYLIB_FUNCTIONS_PRIVATE_HPP
#define MATLAB_EMULATOR_EXTENDBYLIB_FUNCTIONS_PRIVATE_HPP

#include <stdint.h>
#include <matlab/emulator/extendbylib_print_functions.hpp>
#include <matlab/emulator/extendbylib_other_functions.hpp>
#include <stdarg.h>

namespace matlab { namespace emulator {

class ApplicationBase
{
public:
    virtual ~ApplicationBase();

    virtual int MatEmPrintStandard(const char* format, va_list argList)=0;
    virtual int MatEmPrintError(const char* format, va_list argList)=0;
    virtual int MatEmPrintWarning(const char* format, va_list argList)=0;
    virtual int MatEmPrintColored(TMatEmRGB color, const char* format, va_list argList)=0;
    virtual int PutVariableToEmulatorWorkspace(const char* variableName, mxArray* var)=0;
    virtual int PutVariableToMatlabWorkspace(const char* variableName, mxArray* var)=0;
};

}}  // namespace matlab { namespace emulator {


BEGIN_C_DECL2



//int MatEmPrintStandard(const char* format, ...);
//int MatEmPrintError(const char* format, ...);
//int MatEmPrintError(const char* format, ...);
//int MatEmPrintWarning(const char* format, ...);
//int MatEmPrintColored(TMatEmRGB color, const char* format, ...);


END_C_DECL2

EXTERN_C2 matlab::emulator::ApplicationBase* s_pApplicationPointer;


#endif  // #ifndef MATLAB_EMULATOR_EXTENDBYLIB_FUNCTIONS_PRIVATE_HPP
