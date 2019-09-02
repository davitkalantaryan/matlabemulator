//
// file:        dllmain_libextend1_test.cpp
// created on:  2019 Sep 01
//
#include <iostream>
#include <matlab/emulator/extend1bylib.hpp>
#include <matlab/emulator/extendbylib_print_functions.hpp>
#include <matlab/emulator/extendbylib_other_functions.hpp>

#define VERSION_STRING "version: 3. "

EXPORT_FROM_LIB2 const char* TestFunction1(const char* a_cpcInputLine)
{
    MatEmPrintColored({255,100,0},VERSION_STRING "TestFunction1: inputLine=\"%s\"\n",a_cpcInputLine);
    return "Work done1";
}

HIDE_SYMBOL2 const char* TestFunction2(const char* a_cpcInputLine)
{
    mxArray* pValue = mxCreateDoubleMatrix(1,1,mxREAL);
    *mxGetPr(pValue) = 11.;
    PutVariableToEmulatorWorkspace(a_cpcInputLine,pValue);
    return "Work done2";
}

static const char* TestFunction3(const char* a_cpcInputLine)
{
    mxArray* pValue = mxCreateString(a_cpcInputLine);
    PutVariableToMatlabWorkspace(a_cpcInputLine,pValue);
    MatEmPrintStandard(VERSION_STRING "variable '%s' is in the matlab workspace\n",a_cpcInputLine);
    return "Work done3";
}

static const char* TestFunction4(const char* a_cpcInputLine)
{
    ::std::cout<< "TestFunction3: inputLine=\"" << a_cpcInputLine << "\"" << std::endl;
    return "Work done3";
}

DECLARE_EM_TABLE() = {
    {TestFunction1,"Func1","This is help1"},
    {TestFunction2,"Func2",nullptr},
    {TestFunction3,nullptr,nullptr},
    {TestFunction4,"Func1",nullptr},
    {nullptr,nullptr,nullptr}
};
