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
namespace readCode{enum Type{RCerror=-1,RCnone=0,RCstdout,RCstderr};}
//typedef int (*TypeReadFn)(char*,size_t);


TExecHandle RunExecutableNoWait(char* argv[]);
readCode::Type TExecHandle_ReadFromOutOrErr(TExecHandle handle,void* bufferOut, size_t bufferOutSize,void* bufferErr, size_t bufferErrSize);
void TExecHandle_WriteToStdIn(const void* buffer, size_t bufferSize);

}} // namespace common{ namespace system {



#endif  // #ifndef COMMON_SYSTEM_HPP
