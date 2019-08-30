//
// file:       common_system_include_private.hpp
//

#ifndef COMMON_SYSTEM_INCLUDE_PRIVATE_HPP
#define COMMON_SYSTEM_INCLUDE_PRIVATE_HPP

#ifdef _WIN32
#else
#include <unistd.h>
#include <pthread.h>
#endif
#include <common/system.hpp>
#include <stdint.h>

//#define MAKE_SOME_TESTS


namespace common{ namespace system {

#ifdef _WIN32
#else

struct SPipeStruct{
    int pipe;
    uint32_t  isClosed : 1;
    uint32_t  isIsInError : 1;
    uint32_t  reserved : 30;
    SPipeStruct(){this->pipe=-1;this->isClosed=this->isIsInError=0;}
};
#endif

struct SExechandle{
#ifdef _WIN32
#else
    pid_t   pid;
    SPipeStruct readPipes[NUMBER_OF_EXE_READ_PIPES], stdinToWriite, dataToChild, localControlPipeWrite, remoteControlPipe[2];
    //readCode::Type retFromThread;
    uint64_t  finished : 1;
    uint64_t  waited : 1;
    uint64_t  shouldWait : 1;
    SExechandle(){this->pid=-1;this->waited=this->finished=0;this->shouldWait=1;}
#endif
};

int RunExecutableStatic(char* a_argv[],struct SExechandle* a_pHandle);
int RunExecutableNoWaitLineStatic(const char* a_argumentsLine,struct SExechandle* a_pHandle);

}}

#endif  // #ifndef COMMON_SYSTEM_INCLUDE_PRIVATE_HPP
