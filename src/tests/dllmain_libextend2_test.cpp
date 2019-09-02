//
// file:        dllmain_libextend2_test.cpp
// created on:  2019 Sep 02
//
#include <iostream>
#include <matlab/emulator/extend2bylib.hpp>

EXPORT_FROM_LIB2 EXTERN_C2 const char* MatlabEmulatorEntry(const char* a_cpcInputLine)
{
    ::std::cout<< "MatlabEmulatorEntry: inputLine=\"" << a_cpcInputLine << "\"" << std::endl;
    return "Work done from MatlabEmulatorEntry";
}

EXTERN_C2 const char* g_matlabEmulatorEntryHelp;
EXPORT_FROM_LIB2 VARIABLE_C2 const char* g_matlabEmulatorEntryHelp = "This is help from MatlabEmulatorEntry";
