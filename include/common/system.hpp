//
// file:            common/system.hpp
// created on:      2019 Aug 26
// created by:      Davit Kalantaryan (davit.kalantaryan@desy.de)
//
// This header is for functions, that is used to call system routines and binaries
//

#ifndef COMMON_SYSTEM_HPP
#define COMMON_SYSTEM_HPP

#include <stddef.h>

namespace common{ namespace system {

typedef struct SExechandle* TExecHandle;
namespace readCode{enum Type{RCerror=-4,RCtimeout=-3,RCinterrupted=-2,RCselectError=-1,RCnone=0,RCstdout,RCstderr};}
//typedef int (*TypeReadFn)(char*,size_t);


TExecHandle RunExecutableNoWait(char* argv[]);
readCode::Type TExecHandle_ReadFromOutOrErr(TExecHandle handle,void* bufferOut, size_t bufferOutSize,void* bufferErr, size_t bufferErrSize,size_t* a_pReadSize,int timeoutMs);
int TExecHandle_WriteToStdIn(TExecHandle handle,const void* buffer, size_t bufferSize);

}} // namespace common{ namespace system {



#endif  // #ifndef COMMON_SYSTEM_HPP
