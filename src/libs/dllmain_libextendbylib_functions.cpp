//
// file:        dllmain_libextendbylib_functions.cpp
// created on:  2019 Sep 02
//
#include <extendbylib_functions_private.hpp>


EXPORT_FROM_LIB2 VARIABLE_C2 matlab::emulator::ApplicationBase* s_pApplicationPointer = nullptr;

matlab::emulator::ApplicationBase::~ApplicationBase()
{
    //
}


BEGIN_C_DECL2


MATEMUL_EDL_EXPORT int MatEmPrintStandard(const char* a_format, ...)
{
    if(s_pApplicationPointer){
        int nReturn;
        va_list args;

        va_start(args, a_format);
        nReturn = s_pApplicationPointer->MatEmPrintStandard(a_format,args);
        va_end(args);
        return nReturn;
    }

    return -1;
}


MATEMUL_EDL_EXPORT int MatEmPrintError(const char* a_format, ...)
{
    if(s_pApplicationPointer){
        int nReturn;
        va_list args;

        va_start(args, a_format);
        nReturn = s_pApplicationPointer->MatEmPrintError(a_format,args);
        va_end(args);
        return nReturn;
    }

    return -1;
}


MATEMUL_EDL_EXPORT int MatEmPrintWarning(const char* a_format, ...)
{
    if(s_pApplicationPointer){
        int nReturn;
        va_list args;

        va_start(args, a_format);
        nReturn = s_pApplicationPointer->MatEmPrintWarning(a_format,args);
        va_end(args);
        return nReturn;
    }

    return -1;
}


MATEMUL_EDL_EXPORT int MatEmPrintColored(TMatEmRGB a_color, const char* a_format, ...)
{
    if(s_pApplicationPointer){
        int nReturn;
        va_list args;

        va_start(args, a_format);
        nReturn = s_pApplicationPointer->MatEmPrintColored(a_color,a_format,args);
        va_end(args);
        return nReturn;
    }

    return -1;
}


MATEMUL_EDL_EXPORT int PutVariableToEmulatorWorkspace(const char* a_variableName, mxArray* a_var)
{
    if(s_pApplicationPointer){
        return s_pApplicationPointer->PutVariableToEmulatorWorkspace(a_variableName,a_var);
    }
    return -1;
}


MATEMUL_EDL_EXPORT int PutVariableToMatlabWorkspace(const char* a_variableName, mxArray* a_var)
{
    if(s_pApplicationPointer){
        return s_pApplicationPointer->PutVariableToMatlabWorkspace(a_variableName,a_var);
    }
    return -1;
}


END_C_DECL2

