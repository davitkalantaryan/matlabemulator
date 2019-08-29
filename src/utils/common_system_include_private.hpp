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

struct SExechandle{
#ifdef _WIN32
#else
    pid_t   pid;
    int stdinToWriite, stdoutRead, stderrRead, dataToChild, dataFromChild, controlPipe[2];
    readCode::Type retFromThread;
    uint64_t  isWaited : 1;
    uint64_t  shouldWait : 1;
#endif
};

int RunExecutableStatic(char* a_argv[],struct SExechandle* a_pHandle);
int RunExecutableNoWaitLineStatic(const char* a_argumentsLine,struct SExechandle* a_pHandle);

}}

#endif  // #ifndef COMMON_SYSTEM_INCLUDE_PRIVATE_HPP
