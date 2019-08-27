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

#ifdef SOCKET_ERROR
#undef SOCKET_ERROR
#endif
#define SOCKET_ERROR -1

namespace common{ namespace system {


int RunExecutableStatic(char* a_argv[],struct SExechandle* a_pHandle)
{
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
}


readCode::Type TExecHandle_ReadFromOutOrErr(TExecHandle a_handle,void* a_bufferOut, size_t a_bufferOutSize,void* a_bufferErr, size_t a_bufferErrSize, size_t* a_pReadSize, int a_timeoutMs)
{
    fd_set rfds, efds;
    *a_pReadSize = 0;
    int nTry, maxsd;
    readCode::Type retCode = readCode::RCerror;

    maxsd = a_handle->stderrRead > a_handle->stdoutRead ? a_handle->stderrRead  : a_handle->stdoutRead ;

    FD_ZERO( &rfds );
    FD_SET( a_handle->stdoutRead, &rfds );
    FD_SET( a_handle->stderrRead, &rfds );
    FD_SET( a_handle->stdoutRead, &efds );
    FD_SET( a_handle->stderrRead, &efds );

    struct timeval		aTimeout2;
    struct timeval*		pTimeout;

    if( a_timeoutMs >= 0 ){
        aTimeout2.tv_sec = a_timeoutMs / 1000;
        aTimeout2.tv_usec = (a_timeoutMs%1000)*1000 ;
        pTimeout = &aTimeout2;
    }
    else{
        pTimeout = nullptr;
    }

    nTry = select(++maxsd, &rfds, nullptr, &efds, pTimeout );

    switch(nTry)
    {
    case 0:	/* time out */
        return readCode::RCtimeout;
    case SOCKET_ERROR:
        if( errno == EINTR ){
            /* interrupted by signal */
            return readCode::RCinterrupted;
        }

        return readCode::RCselectError;
    default:
        // we can read
        break;
    }

    if( FD_ISSET( a_handle->stdoutRead, &rfds ) ){
        ssize_t outRead = read(a_handle->stdoutRead,a_bufferOut,a_bufferOutSize);
        if(outRead>0){
            *a_pReadSize = static_cast<size_t>(outRead);
            return readCode::RCstdout;
        }
    }

    if( FD_ISSET( a_handle->stderrRead, &rfds ) ){
        ssize_t errRead = read(a_handle->stdoutRead,a_bufferErr,a_bufferErrSize);
        if(errRead>0){
            *a_pReadSize = static_cast<size_t>(errRead);
            return readCode::RCstderr;
        }
    }

    return retCode;
}


int TExecHandle_WriteToStdIn(TExecHandle a_handle,const void* a_buffer, size_t a_bufferSize)
{
    return static_cast<int>(::write(a_handle->stdinToWriite,a_buffer,a_bufferSize));
}


}} // namespace common{ namespace system {
