//
// file:            common_system.cpp
// created on:      2019 Aug 26
// created by:      Davit Kalantaryan (davit.kalantaryan@desy.de)
//
// Here is functions, that is used to call system routines and binaries
//

#include <common/system.hpp>

#ifdef _WIN32
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>

namespace common{ namespace system {

struct SExechandle{
#ifdef _WIN32
#else
    pid_t   pid;
    int stdinToWriite, stdoutRead, stderrRead;
#endif
};

static int RunExecutableStatic(char* a_argv[],struct SExechandle* a_pHandle)
{
#ifdef _WIN32

#else   // #ifdef _WIN32

#if 0
    int stderrCopy;
    if(!m_vErrorPipes[0]){
        if(pipe(m_vErrorPipes)){
            return;
        }
    }
    stderrCopy = dup(STDERR_FILENO);
    dup2(m_vErrorPipes[1],STDERR_FILENO);
    m_pEngine = engOpen(MATLAB_START_COMMAND);
    //m_pEngine = engOpenSingleUse(MATLAB_START_COMMAND,nullptr,nullptr);
    dup2(stderrCopy,STDERR_FILENO);
    close(stderrCopy);
#endif
    int vnStdin[2], vnStdout[2], vnStderr[2];
    int stdinCopy,stdoutCopy,stderrCopy;

    if(pipe(vnStdin)){
        return -1;
    }

    if(pipe(vnStdout)){
        return -1;
    }

    if(pipe(vnStderr)){
        return -1;
    }

    stdinCopy = dup(STDIN_FILENO);
    stdoutCopy = dup(STDOUT_FILENO);
    stderrCopy = dup(STDERR_FILENO);

    dup2(vnStdin[0],STDIN_FILENO);
    dup2(vnStdout[1],STDOUT_FILENO);
    dup2(vnStderr[1],STDERR_FILENO);

    a_pHandle->pid = fork();

    if(!a_pHandle->pid){
        // we are a child application
        execv(a_argv[0],a_argv);
        // if we are here,
        // this means we have
        // a problem to run execv
        fprintf(stderr,"Unable to run executable: %s\n",a_argv[0]);  // this will go to our error pipe
        perror("\n");
        exit(1);
    }

    dup2(stdinCopy,STDIN_FILENO);
    dup2(stdoutCopy,STDOUT_FILENO);
    dup2(stderrCopy,STDERR_FILENO);

    close(stdinCopy);
    close(stdoutCopy);
    close(stderrCopy);

    close(vnStdin[0]);
    close(vnStdout[1]);
    close(vnStderr[1]);

    a_pHandle->stdinToWriite = vnStdin[1];
    a_pHandle->stdoutRead = vnStdout[0];
    a_pHandle->stdoutRead = vnStderr[0];

    return 0;
#endif  // #ifdef _WIN32
}


TExecHandle RunExecutableNoWait(char* a_argv[])
{
    struct SExechandle* pHandle = new struct SExechandle;

    if(RunExecutableStatic(a_argv,pHandle)){
        delete pHandle;
        return nullptr;
    }

    return pHandle;
}


readCode::Type TExecHandle_ReadFromOutOrErr(TExecHandle a_handle,void* a_bufferOut, size_t a_bufferOutSize,void* a_bufferErr, size_t a_bufferErrSize)
{
    readCode::Type retCode = readCode::RCnone;

    return retCode;
}
void TExecHandle_WriteToStdIn(const void* buffer, size_t bufferSize)


}} // namespace common{ namespace system {
