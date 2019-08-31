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


#define NUMBER_OF_EXE_READ_PIPES     4 // stdoutRead, stderrRead, dataFromChild, controlPipe[0]
#define STDOUT_EXE_PIPE              0
#define STDERR_EXE_PIPE              1
#define DATA_FROM_CHILD_EXE_PIPE     2
#define CONTROL_RD_EXE_PIPE          3

#define ME_READ_PIPE                 0
#define ME_WRITE_PIPE                1

#define INIT_C_ARRAY01(_value)      {(_value)}
#define INIT_C_ARRAY02(_value)      {(_value),(_value)}
#define INIT_C_ARRAY03(_value)      {(_value),(_value),(_value)}
#define INIT_C_ARRAY04(_value)      {(_value),(_value),(_value),(_value)}
#define INIT_C_ARRAY05(_value)      {(_value),(_value),(_value),(_value),(_value)}

#define INIT_C_ARRAYRD              INIT_C_ARRAY04


namespace common{ namespace system {

typedef struct SExechandle* TExecHandle;
namespace readCode{enum Type{
    RCerror=-10,RCreadError2=-6,RCnoRecource=-5,RCtimeout=-3,RCinterrupted=-2,RCselectError=-1,
    RCnone=0,RCstdout,RCstderr,RCdata=4,RCcontrol=8,RCallPipesInerror=16
};}
//typedef int (*TypeReadFn)(char*,size_t);

static const readCode::Type s_finalRetCodes[NUMBER_OF_EXE_READ_PIPES] = {readCode::RCstdout,readCode::RCstderr,readCode::RCdata,readCode::RCcontrol};


TExecHandle RunExecutableNoWait(char* argv[]);
TExecHandle RunExecutableNoWaitLine(const char* argumentsLine);
readCode::Type TExecHandle_ReadFromStandardPipesStatic(TExecHandle handle,void* buffers[NUMBER_OF_EXE_READ_PIPES],size_t buffersSizes[NUMBER_OF_EXE_READ_PIPES],size_t* pReadSize,int timeoutMs);
int TExecHandle_WriteToStdIn(TExecHandle handle,const void* buffer, size_t bufferSize);
readCode::Type TExecHandle_WatitForEndAndReadFromOutOrErr(TExecHandle handle,void* buffers[NUMBER_OF_EXE_READ_PIPES],size_t buffersSizes[NUMBER_OF_EXE_READ_PIPES],size_t* pReadSize,int timeoutMs);
void TExecHandle_WaitAndClearExecutable(TExecHandle handle);
bool TExecHandle_IsExeFinished(TExecHandle handle);

}} // namespace common{ namespace system {



#endif  // #ifndef COMMON_SYSTEM_HPP
