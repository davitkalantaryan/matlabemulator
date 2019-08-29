//
// file:            common_system.cpp
// created on:      2019 Aug 26
// created by:      Davit Kalantaryan (davit.kalantaryan@desy.de)
//
// Here is functions, that is used to call system routines and binaries
//

#include <string.h> // we hope that strcspn is rejecting case
#include <common/system.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "common_system_include_private.hpp"
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <alloca.h>
#include <string>

#ifdef SOCKET_ERROR
#undef SOCKET_ERROR
#endif
#define SOCKET_ERROR -1

namespace common{ namespace system {

// TExecHandle RunExecutableNoWaitLine(char* argumentsLine)

static const char* s_sccpAcceptReject = " \t\n\r";

#define MAX_ARGS_NUMBER         500
#define MAX_ARGS_NUMBER_ROUNDED 500

int RunExecutableNoWaitLineStatic(const char* a_argumentsLine,struct SExechandle* a_pHandle)
{
    ::std::string pipeString;
    const char* pcParsePointer = a_argumentsLine;
    size_t unRemArgStrLen(strlen(a_argumentsLine));
    size_t unStrStrResult;
    char** ppcArgv = static_cast<char**>(calloc(MAX_ARGS_NUMBER_ROUNDED,sizeof(char*)));
    int vnDataFromChild[2]={-1,-1},vnDataToChild[2]={-1,-1};
    int ind(0);

    if(!ppcArgv){
        return -1;
    }

    if(pipe(vnDataFromChild)){
        goto errorReturnPoint;
    }

    if(pipe(vnDataToChild)){
        goto errorReturnPoint;
    }

    unStrStrResult = strspn(pcParsePointer,s_sccpAcceptReject);

    for( ; (ind<MAX_ARGS_NUMBER) && (unStrStrResult<unRemArgStrLen);++ind){
        pcParsePointer += unStrStrResult;
        unRemArgStrLen -= unStrStrResult;
        unStrStrResult = strcspn(pcParsePointer,s_sccpAcceptReject);
        ppcArgv[ind]=strndup(pcParsePointer,unStrStrResult);
        pcParsePointer += unStrStrResult;
        unRemArgStrLen -= unStrStrResult;
        unStrStrResult = strspn(pcParsePointer,s_sccpAcceptReject);
    }

    if(!ind){
        goto errorReturnPoint;
    }

    ppcArgv[ind++] = nullptr;

    pipeString = ::std::to_string(vnDataFromChild[1]);
    ppcArgv[ind++] = strdup(pipeString.c_str());

    pipeString = ::std::to_string(vnDataToChild[0]);
    ppcArgv[ind++] = strdup(pipeString.c_str());

    ppcArgv[ind++] = nullptr;

    if(RunExecutableStatic(ppcArgv,a_pHandle)){
        goto errorReturnPoint;
    }

    a_pHandle->dataFromChild = vnDataFromChild[0];
    a_pHandle->dataToChild = vnDataToChild[1];

    close(vnDataFromChild[1]);
    close(vnDataToChild[0]);

    for(ind=0;ind<MAX_ARGS_NUMBER_ROUNDED;++ind){
        free(ppcArgv[ind]);
    }
    free(ppcArgv);

    return 0;

errorReturnPoint:

    if(vnDataFromChild[0]>0){
        close(vnDataFromChild[0]);
    }

    if(vnDataFromChild[1]>0){
        close(vnDataFromChild[1]);
    }

    if(vnDataToChild[0]>0){
        close(vnDataFromChild[0]);
    }

    if(vnDataToChild[1]>0){
        close(vnDataFromChild[1]);
    }

    if(ppcArgv){
        for(ind=0;ind<MAX_ARGS_NUMBER_ROUNDED;++ind){
            free(ppcArgv[ind]);
        }
        free(ppcArgv);
    }

    return -1;
}

int RunExecutableStatic(char* a_argv[],struct SExechandle* a_pHandle)
{
    int vnStdin[2]={-1,-1}, vnStdout[2]={-1,-1}, vnStderr[2]={-1,-1};
    int stdinCopy,stdoutCopy,stderrCopy;

    if(pipe(vnStdin)){
        return -1;
    }

    if(pipe(vnStdout)){
        goto errorReturnPoint;
    }

    if(pipe(vnStderr)){
        goto errorReturnPoint;
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

#ifndef TRY_TO_SKIP_COPY_ON_CHANGE
        close(vnStdin[1]);
        close(vnStdout[0]);
        close(vnStderr[0]);

        //fprintf(stderr,"Calling execv argv[0]=%s\n",a_argv[0]);
#endif
        execvp(a_argv[0],a_argv);
        // if we are here,
        // this means we have
        // a problem to run execv

        //dup2(stdinCopy,STDIN_FILENO);
        //dup2(stdoutCopy,STDOUT_FILENO);
        //dup2(stderrCopy,STDERR_FILENO);

        fprintf(stderr,"Unable to run executable: %s\n",a_argv[0]);  // this will go to our error pipe
        perror("\n");

        close(vnStdin[0]);
        close(vnStdout[1]);
        close(vnStderr[1]);

        _Exit(1);
    }
    else{
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
        a_pHandle->stderrRead = vnStderr[0];
        a_pHandle->controlPipe[0] = -1;
        a_pHandle->controlPipe[1] = -1;

        a_pHandle->isWaited = 0;
    }

    return 0;

errorReturnPoint:
    if(vnStdin[0]>0){
        close(vnStdin[0]);
    }

    if(vnStdin[1]>0){
        close(vnStdin[1]);
    }

    if(vnStdout[0]>0){
        close(vnStdout[0]);
    }

    if(vnStdout[1]>0){
        close(vnStdout[1]);
    }

    if(vnStderr[0]>0){
        close(vnStderr[0]);
    }

    if(vnStderr[1]>0){
        close(vnStderr[1]);
    }

    return -1;
}


void TExecHandle_WaitAndClearExecutable(TExecHandle a_handle)
{
    if(!a_handle->isWaited){
        pid_t w;
        int status;

        do {
            w = waitpid(a_handle->pid, &status, WUNTRACED | WCONTINUED);
            if (w == -1) {
                if(errno == EINTR){
                    printf("waitpid 2: interrupted! handle->shouldWait = %d\n",static_cast<int>(a_handle->shouldWait));
                    break;
                }
                else{
                    perror("waitpid 2:");
                    a_handle->retFromThread = readCode::RCerror;
                    if(a_handle->controlPipe[1]>0){
                        write(a_handle->controlPipe[1],"1234",4);
                    }
                    break;
                }
            }

            else if (WIFEXITED(status)) {
                printf("exited, status=%d\n", WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("killed by signal %d\n", WTERMSIG(status));
            } else if (WIFSTOPPED(status)) {
                printf("stopped by signal %d\n", WSTOPSIG(status));
            } else if (WIFCONTINUED(status)) {
                printf("continued\n");
            }
        }
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }  // if(!a_handle->isWaited){

#ifndef MAKE_SOME_TESTS
    // int stdinToWriite, stdoutRead, stderrRead, dataToChild, dataFromChild, controlPipe[2];
    close(a_handle->stdinToWriite);
    close(a_handle->stdoutRead);
    close(a_handle->stderrRead);
    close(a_handle->dataToChild);
    close(a_handle->dataFromChild);

    if(a_handle->controlPipe[0]>0){
        close(a_handle->controlPipe[0]);
    }

    if(a_handle->controlPipe[1]>0){
        close(a_handle->controlPipe[1]);
    }
#endif

    delete a_handle;
}


readCode::Type TExecHandle_ReadFromStandardPipesStatic(TExecHandle a_handle,  void* a_buffers[NUMBER_OF_STANDARD_READ_PIPES], size_t a_buffersSizes[NUMBER_OF_STANDARD_READ_PIPES], size_t* a_pReadSize, int a_timeoutMs)
{
    fd_set rfds, efds;
    int nTry, maxsd(0),i;
    int vcFds[NUMBER_OF_STANDARD_READ_PIPES] = {a_handle->stdoutRead,a_handle->stderrRead,a_handle->dataFromChild,a_handle->controlPipe[0]};
    readCode::Type finalRetCodes[NUMBER_OF_STANDARD_READ_PIPES] = {readCode::RCstdout,readCode::RCstderr,readCode::RCdata,readCode::RCcontrol};
    readCode::Type retCode = readCode::RCerror;

    FD_ZERO( &rfds );
    FD_ZERO( &efds );

    for(i=0;i<NUMBER_OF_STANDARD_READ_PIPES;++i){
        if(vcFds[i]>=0){
            FD_SET( vcFds[i], &rfds );
            FD_SET( vcFds[i], &efds );

            if(vcFds[i]>maxsd){
                maxsd = vcFds[i];
            }
        }
    }  // for(i=0;i<NUMBER_OF_STANDARD_READ_PIPES;++i){

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

    for(i=0;i<NUMBER_OF_STANDARD_READ_PIPES;++i){
        if( (vcFds[i]>=0) && FD_ISSET( vcFds[i], &rfds ) ){
            ssize_t errRead = read(vcFds[i],a_buffers[i],a_buffersSizes[i]);
            if(errRead>0){
                *a_pReadSize = static_cast<size_t>(errRead);
                return finalRetCodes[i];
            }
            else if(!errRead){
                retCode = readCode::RCpipeClosed;
            }
            else{
                retCode = readCode::RCreadError;
            }
        }
    }

    return retCode;
}


int TExecHandle_WriteToStdIn(TExecHandle a_handle,const void* a_buffer, size_t a_bufferSize)
{
    return static_cast<int>(::write(a_handle->stdinToWriite,a_buffer,a_bufferSize));
}

#ifndef MAKE_SOME_TESTS
static void* WaiterThreadFunction(void* a_pHandle)
{
    TExecHandle a_handle = static_cast<TExecHandle>(a_pHandle);
    pid_t w;
    int status;

    do {
        w = waitpid(a_handle->pid, &status, WUNTRACED | WCONTINUED);
        if (w == -1) {
            if(errno == EINTR){
                printf("waitpid 1: interrupted! handle->shouldWait = %d\n",static_cast<int>(a_handle->shouldWait));
                fflush(stdout);
                a_handle->retFromThread = readCode::RCinterrupted;
                return reinterpret_cast<void*>(EINTR);
            }
            else{
                perror("waitpid 1:");
                a_handle->retFromThread = readCode::RCerror;
                if(a_handle->controlPipe[1]>0){
                    write(a_handle->controlPipe[1],"1234",4);
                }
                a_handle->retFromThread = readCode::RCerror;
                return reinterpret_cast<void*>(EXIT_FAILURE);
            }

        }

        else if (WIFEXITED(status)) {
            printf("exited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("killed by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("stopped by signal %d\n", WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            printf("continued\n");
        }
    }
    while (!WIFEXITED(status) && !WIFSIGNALED(status) && a_handle->shouldWait);

    a_handle->isWaited = 1;
    a_handle->retFromThread = readCode::RCexeFinished;
    if(a_handle->controlPipe[1]>0){
        write(a_handle->controlPipe[1],"1234",4);
    }
    return nullptr;
}
#endif


readCode::Type TExecHandle_WatitForEndAndReadFromOutOrErr(TExecHandle a_handle,void* a_buffers[NUMBER_OF_STANDARD_READ_PIPES],size_t a_buffersSizes[NUMBER_OF_STANDARD_READ_PIPES],size_t* a_pReadSize,int a_timeoutMs)
{    
#ifndef MAKE_SOME_TESTS
    int nRet;
    pthread_t waiterThread = static_cast<pthread_t>(0);
    readCode::Type retVal;
    struct sigaction oldSigaction, newSigAction;

    a_handle->retFromThread = readCode::RCnone;

    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;
    newSigAction.sa_restorer = nullptr;
    newSigAction.sa_handler = [](int){};

    sigaction(SIGINT,&newSigAction,&oldSigaction);

    if(a_handle->controlPipe[0]<0){
        if(pipe(a_handle->controlPipe)){
            retVal = readCode::RCnoRecource;
            goto returnPoint;
        }
    }
    a_handle->shouldWait=1;
    nRet = pthread_create(&waiterThread,nullptr,&WaiterThreadFunction,a_handle);

    if(nRet){
        retVal = readCode::RCnoRecource;
        goto returnPoint;
    }

    retVal = TExecHandle_ReadFromStandardPipesStatic(a_handle,a_buffers,a_buffersSizes,a_pReadSize,a_timeoutMs);

    if(retVal==readCode::RCinterrupted){
        if(a_handle->retFromThread != readCode::RCnone){
            retVal =  a_handle->retFromThread;
            goto returnPoint;
        }
    }

    a_handle->shouldWait = 0;
    pthread_kill(waiterThread,SIGINT);

returnPoint:
    if(waiterThread){
        pthread_join(waiterThread,nullptr);
    }
    sigaction(SIGINT,&oldSigaction,nullptr);
    return retVal;
#else
    pid_t w;
    int status;
    struct sigaction newSigAction;

    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;
    newSigAction.sa_restorer = nullptr;
    newSigAction.sa_handler = SIG_DFL;
    sigaction(SIGCHLD,&newSigAction,nullptr);

    do {
        w = waitpid(a_handle->pid, &status, WUNTRACED | WCONTINUED);
        if (w == -1) {
            perror("waitpid 1:");
            a_handle->retFromThread = readCode::RCerror;
            if(a_handle->controlPipe[1]>0){
                write(a_handle->controlPipe[1],"1234",4);
            }
            return readCode::RCerror;

        }

        else if (WIFEXITED(status)) {
            printf("exited, status=%d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("killed by signal %d\n", WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            printf("stopped by signal %d\n", WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            printf("continued\n");
        }
    }
    while (!WIFEXITED(status) && !WIFSIGNALED(status) && a_handle->shouldWait);

    return readCode::RCexeFinished;
#endif
}


/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


}} // namespace common{ namespace system {
