//
// file:       common_system_include_private.hpp
//

#ifndef COMMON_SYSTEM_INCLUDE_PRIVATE_HPP
#define COMMON_SYSTEM_INCLUDE_PRIVATE_HPP

#ifdef _WIN32
#else
#include <unistd.h>
#endif

namespace common{ namespace system {

struct SExechandle{
#ifdef _WIN32
#else
    pid_t   pid;
    int stdinToWriite, stdoutRead, stderrRead;
#endif
};

int RunExecutableStatic(char* a_argv[],struct SExechandle* a_pHandle);

}}

#endif  // #ifndef COMMON_SYSTEM_INCLUDE_PRIVATE_HPP
