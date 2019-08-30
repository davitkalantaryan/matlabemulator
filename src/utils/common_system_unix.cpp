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
#include <common/matlabemulator_compiler_internal.h>

#ifdef SOCKET_ERROR
#undef SOCKET_ERROR
#endif
#define SOCKET_ERROR -1

#define SIGNAL_FOR_THREAD_STOP  SIGUSR1

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

    a_pHandle->readPipes[DATA_FROM_CHILD_EXE_PIPE].pipe = vnDataFromChild[0];
    a_pHandle->dataToChild.pipe = vnDataToChild[1];

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
    int vnStdin[2]={-1,-1}, vnStdout[2]={-1,-1}, vnStderr[2]={-1,-1}, vnRemoteControlPipe[2]={-1,-1};
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

    if(pipe(vnRemoteControlPipe)){
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
        close(vnRemoteControlPipe[0]);

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

        a_pHandle->stdinToWriite.pipe = vnStdin[1];
        a_pHandle->readPipes[STDOUT_EXE_PIPE].pipe = vnStdout[0];
        a_pHandle->readPipes[STDERR_EXE_PIPE].pipe = vnStderr[0];
        a_pHandle->remoteControlPipe[ME_READ_PIPE].pipe = vnRemoteControlPipe[0];
        a_pHandle->remoteControlPipe[ME_WRITE_PIPE].pipe = vnRemoteControlPipe[1];

        // these 3 lines done bu compiler
        //a_pHandle->controlPipe[0].pipe = -1;
        //a_pHandle->controlPipe[1].pipe = -1;
        //a_pHandle->waited = 0;
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

    if(vnRemoteControlPipe[0]>0){
        close(vnStderr[0]);
    }

    if(vnRemoteControlPipe[1]>0){
        close(vnStderr[1]);
    }

    return -1;
}


static void TExecHandle_WaitFinishStatic2(TExecHandle a_handle)
{
    if(!LIKELY_VALUE2(a_handle->finished,false)){
        char vcBuffer[2];
        ssize_t contrPpeRead ;

        do{
            contrPpeRead = ::read(a_handle->remoteControlPipe[ME_READ_PIPE].pipe,vcBuffer,1);

            if((contrPpeRead<=0)&&(errno!=EINTR)){
                a_handle->finished = 1;
                return;
            }
        }
        while(a_handle->shouldWait );

    }
}


static int TExecHandle_WaitPidStatic2(TExecHandle a_handle)
{
    if(!a_handle->waited){

        if(!LIKELY_VALUE2(a_handle->finished,true)){
            TExecHandle_WaitFinishStatic2(a_handle);
        }

        if(LIKELY_VALUE2(a_handle->finished,true)){
            // application is finished and closed the pipe
            pid_t w;
            int status;

            do {
                w = waitpid(a_handle->pid, &status, WUNTRACED | WCONTINUED);
                if (w == -1) {
                    if(errno == EINTR){
                        printf("waitpid: interrupted! handle->shouldWait = %d\n",static_cast<int>(a_handle->shouldWait));
                        return EINTR;
                    }
                    else{
                        perror("waitpid:");
                        return -1;
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

            a_handle->waited = 1;
            if(a_handle->localControlPipeWrite.pipe>0){
                write(a_handle->localControlPipeWrite.pipe,&status,4);
            }
            return WEXITSTATUS(status);
        }

    }

    return 0;
}


void TExecHandle_WaitAndClearExecutable(TExecHandle a_handle)
{
    TExecHandle_WaitFinishStatic2(a_handle);
    TExecHandle_WaitPidStatic2(a_handle);

    // int stdinToWriite, stdoutRead, stderrRead, dataToChild, dataFromChild, controlPipe[2];
    if(a_handle->stdinToWriite.pipe>0){close(a_handle->stdinToWriite.pipe);}
    if(a_handle->dataToChild.pipe>0){close(a_handle->dataToChild.pipe);}
    if(a_handle->localControlPipeWrite.pipe>0){close(a_handle->localControlPipeWrite.pipe);}
    if(a_handle->remoteControlPipe[ME_READ_PIPE].pipe>0){close(a_handle->remoteControlPipe[ME_READ_PIPE].pipe);}
    if(a_handle->remoteControlPipe[ME_WRITE_PIPE].pipe>0){close(a_handle->remoteControlPipe[ME_WRITE_PIPE].pipe);}
    for(int i(0);i<NUMBER_OF_EXE_READ_PIPES;++i)if(a_handle->readPipes[i].pipe>0){close(a_handle->readPipes[i].pipe);}

    delete a_handle;
}


readCode::Type TExecHandle_ReadFromStandardPipesStatic(TExecHandle a_handle,  void* a_buffers[NUMBER_OF_EXE_READ_PIPES], size_t a_buffersSizes[NUMBER_OF_EXE_READ_PIPES], size_t* a_pReadSize, int a_timeoutMs)
{
    fd_set rfds, efds;
    int nTry, maxsd(0),i;
    SPipeStruct* vcFds = a_handle->readPipes;
    readCode::Type retCode = readCode::RCerror;

    FD_ZERO( &rfds );
    FD_ZERO( &efds );

    for(i=0;i<NUMBER_OF_EXE_READ_PIPES;++i){
        if((!vcFds[i].isClosed) && (!vcFds[i].isIsInError) && (vcFds[i].pipe>=0)){
            FD_SET( vcFds[i].pipe, &rfds );
            FD_SET( vcFds[i].pipe, &efds );

            if(vcFds[i].pipe>maxsd){
                maxsd = vcFds[i].pipe;
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

    for(i=0;i<NUMBER_OF_EXE_READ_PIPES;++i){
        if( (vcFds[i].pipe>=0) && FD_ISSET( vcFds[i].pipe, &rfds ) ){
            ssize_t errRead = read(vcFds[i].pipe,a_buffers[i],a_buffersSizes[i]);
            if(errRead>0){
                *a_pReadSize = static_cast<size_t>(errRead);
                return s_finalRetCodes[i];
            }
            else if(!errRead){
                vcFds[i].isClosed = 1;
            }
            else{
                vcFds[i].isIsInError = 1;
                retCode = readCode::RCreadError2;
            }
        }
    }

    return retCode;
}


int TExecHandle_WriteToStdIn(TExecHandle a_handle,const void* a_buffer, size_t a_bufferSize)
{
    return static_cast<int>(::write(a_handle->stdinToWriite.pipe,a_buffer,a_bufferSize));
}

static void* WaiterThreadFunction(void* a_pHandle)
{
    TExecHandle a_handle = static_cast<TExecHandle>(a_pHandle);
    TExecHandle_WaitPidStatic2(a_handle);

    return nullptr;
}


readCode::Type TExecHandle_WatitForEndAndReadFromOutOrErr(TExecHandle a_handle,void* a_buffers[NUMBER_OF_EXE_READ_PIPES],size_t a_buffersSizes[NUMBER_OF_EXE_READ_PIPES],size_t* a_pReadSize,int a_timeoutMs)
{    
    int nRet;
    pthread_t waiterThread = static_cast<pthread_t>(0);
    readCode::Type retVal;
    struct sigaction oldSigaction, newSigAction;

    sigemptyset(&newSigAction.sa_mask);
    newSigAction.sa_flags = 0;
    newSigAction.sa_restorer = nullptr;
    newSigAction.sa_handler = [](int){};

    sigaction(SIGNAL_FOR_THREAD_STOP,&newSigAction,&oldSigaction);

    if(a_handle->localControlPipeWrite.pipe<0){
        int vcControlPipe[2];
        //if(pipe(a_handle->controlPipe)){
        if(pipe(vcControlPipe)){
            retVal = readCode::RCnoRecource;
            goto returnPoint;
        }

        a_handle->readPipes[CONTROL_RD_EXE_PIPE].pipe = vcControlPipe[0];
        a_handle->localControlPipeWrite.pipe = vcControlPipe[1];
    }
    a_handle->shouldWait=1;
    nRet = pthread_create(&waiterThread,nullptr,&WaiterThreadFunction,a_handle);

    if(nRet){
        retVal = readCode::RCnoRecource;
        goto returnPoint;
    }

    retVal = TExecHandle_ReadFromStandardPipesStatic(a_handle,a_buffers,a_buffersSizes,a_pReadSize,a_timeoutMs);

    a_handle->shouldWait = 0;
    pthread_kill(waiterThread,SIGNAL_FOR_THREAD_STOP);
    ::write(a_handle->remoteControlPipe[ME_WRITE_PIPE].pipe,"o",1);

returnPoint:
    if(waiterThread){
        pthread_join(waiterThread,nullptr);
    }
    sigaction(SIGNAL_FOR_THREAD_STOP,&oldSigaction,nullptr);
    return retVal;
}


/*//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////*/


}} // namespace common{ namespace system {
