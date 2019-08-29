//
// file:        common_createnewprocess_unix.cpp
// created on:  2018 Nov 29
// created by:  D. Kalantaryan
//

#ifndef _WIN32

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>

//static const char s_cpcTerminatorString[]={' ','\n','\t','\0'};

int CreateProcessToDevNullAndWait(const char* a_cpcExecute, char* a_argv[])
{
    int status;
    int nPid = fork();

    if(nPid){
        int nWaitResult;
        do {
            nWaitResult = waitpid(nPid, &status, WUNTRACED | WCONTINUED);
            if (nWaitResult == -1) {return -1;}

        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        //return WEXITSTATUS(status);
    }
    else{
        //freopen( "/dev/null", "r", stdin);
        //freopen( "/dev/null", "w", stdout);
        //freopen( "/dev/null", "w", stderr);
        execvp(a_cpcExecute,a_argv);
        _exit(1);
    }

    //return -1;
    return WEXITSTATUS(status);
}

int CreateProcessToDevNullAndWait2(const char* a_cpcExecuteLine)
{
    int nReturn;
    char** ppArgs = static_cast<char**>(calloc(2,sizeof(char*)));

    ppArgs[0] = strdup(a_cpcExecuteLine);
    nReturn = CreateProcessToDevNullAndWait(ppArgs[0],ppArgs);
    free(ppArgs[0]);
    free(ppArgs);

    return nReturn;
}



#endif  // #ifdef _WIN32
