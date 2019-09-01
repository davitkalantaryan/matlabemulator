//
// file:        dllmain_libextend1_test.cpp
// created on:  2019 Sep 01
//
#include <iostream>
#include <matlab/emulator/extendbylib.hpp>

EXPORT_FROM_LIB2 const char* TestFunction1(const char* a_cpcInputLine)
{
    ::std::cout<< "TestFunction1: inputLine=\"" << a_cpcInputLine << "\"" << std::endl;
    return "Work done1";
}

HIDE_SYMBOL2 const char* TestFunction2(const char* a_cpcInputLine)
{
    ::std::cout<< "TestFunction2: inputLine=\"" << a_cpcInputLine << "\"" << std::endl;
    return "Work done2";
}

static const char* TestFunction3(const char* a_cpcInputLine)
{
    ::std::cout<< "TestFunction3: inputLine=\"" << a_cpcInputLine << "\"" << std::endl;
    return "Work done3";
}

static const char* TestFunction4(const char* a_cpcInputLine)
{
    ::std::cout<< "TestFunction3: inputLine=\"" << a_cpcInputLine << "\"" << std::endl;
    return "Work done3";
}

EXTERN_C2 const TEmTableEntry g_vcMatlabEmulatorTable[];
EXPORT_FROM_LIB2 EXTERN_C2 const TEmTableEntry g_vcMatlabEmulatorTable[] = {
    {TestFunction1,"Func1","This is help1"},
    {TestFunction2,"Func2",nullptr},
    {TestFunction3,nullptr,nullptr},
    {TestFunction4,"Func1",nullptr},
    {nullptr,nullptr,nullptr}
};
