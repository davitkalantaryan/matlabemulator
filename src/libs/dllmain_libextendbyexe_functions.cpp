//
// file:        dllmain_libextendbylib_functions.cpp
// created on:  2019 Sep 02
//
#include <matlab/emulator/extendbyexe_functions.hpp>
#include <stdio.h>


BEGIN_C_DECL2

MATEMUL_EE_EXPORT int PutVariableToEmulatorWorkspaceFromBin(const char* a_variableName, mxArray* a_var)
{
    printf("inps: {%s,%p}. Function 'PutVariableToEmulatorWorkspaceFromBin' is not implemented yet.\n",a_variableName,static_cast<void*>(a_var));
    return 0;
}


MATEMUL_EE_EXPORT int PutVariableToMatlabWorkspaceFromBin(const char* a_variableName, mxArray* a_var)
{
    printf("inps: {%s,%p}. Function 'PutVariableToMatlabWorkspaceFromBin' is not implemented yet.\n",a_variableName,static_cast<void*>(a_var));
    return 0;
}


END_C_DECL2

