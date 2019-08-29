//
// file:            common_system.cpp
// created on:      2019 Aug 26
// created by:      Davit Kalantaryan (davit.kalantaryan@desy.de)
//
// Here is functions, that is used to call system routines and binaries
//

#include <common/system.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "common_system_include_private.hpp"

namespace common{ namespace system {

TExecHandle RunExecutableNoWait(char* a_argv[])
{
    struct SExechandle* pHandle = new struct SExechandle;

    if(RunExecutableStatic(a_argv,pHandle)){
        delete pHandle;
        return nullptr;
    }

    return pHandle;
}



TExecHandle RunExecutableNoWaitLine(const char* a_argumentsLine)
{
    struct SExechandle* pHandle = new struct SExechandle;

    if(RunExecutableNoWaitLineStatic(a_argumentsLine,pHandle)){
        delete pHandle;
        return nullptr;
    }

    return pHandle;
}


}} // namespace common{ namespace system {
