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

#define NUMBER_OF_STANDARD_READ_PIPES    4 // stdoutRead, stderrRead, dataFromChild, controlPipe[0]

namespace common{ namespace system {

typedef struct SExechandle* TExecHandle;
namespace readCode{enum Type{RCerror=-10,RCpipeClosed=-7,RCreadError=-6,RCnoRecource=-5,RCtimeout=-3,RCinterrupted=-2,RCselectError=-1,RCnone=0,RCstdout,RCstderr,RCdata,RCcontrol,RCexeFinished};}
//typedef int (*TypeReadFn)(char*,size_t);


TExecHandle RunExecutableNoWait(char* argv[]);
TExecHandle RunExecutableNoWaitLine(const char* argumentsLine);
readCode::Type TExecHandle_ReadFromStandardPipesStatic(TExecHandle handle,void* buffers[NUMBER_OF_STANDARD_READ_PIPES],size_t buffersSizes[NUMBER_OF_STANDARD_READ_PIPES],size_t* pReadSize,int timeoutMs);
int TExecHandle_WriteToStdIn(TExecHandle handle,const void* buffer, size_t bufferSize);
readCode::Type TExecHandle_WatitForEndAndReadFromOutOrErr(TExecHandle handle,void* buffers[NUMBER_OF_STANDARD_READ_PIPES],size_t buffersSizes[NUMBER_OF_STANDARD_READ_PIPES],size_t* pReadSize,int timeoutMs);
void TExecHandle_WaitAndClearExecutable(TExecHandle handle);

}} // namespace common{ namespace system {



#endif  // #ifndef COMMON_SYSTEM_HPP
