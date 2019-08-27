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


int RunExecutableStatic(char* a_argv[],struct SExechandle* a_pHandle)
{
    return 0;
}


readCode::Type TExecHandle_ReadFromOutOrErr(TExecHandle a_handle,void* a_bufferOut, size_t a_bufferOutSize,void* a_bufferErr, size_t a_bufferErrSize, size_t* a_pReadSize, int a_timeoutMs)
{
    readCode::Type retCode = readCode::RCerror;

    return retCode;
}


int TExecHandle_WriteToStdIn(TExecHandle a_handle,const void* a_buffer, size_t a_bufferSize)
{
    return 0;
}


}} // namespace common{ namespace system {
