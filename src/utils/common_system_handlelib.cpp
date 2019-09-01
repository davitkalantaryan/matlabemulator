//
// file:            common_system.cpp
// created on:      2019 Aug 26
// created by:      Davit Kalantaryan (davit.kalantaryan@desy.de)
//
// Here is functions, that is used to call system routines and binaries
//

#include <common/system/handlelib.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#ifdef _WIN32

#ifndef CINTERFACE
#define CINTERFACE
#define CINTERFACE_here_defined
#endif
#ifndef NOMINMAX
#define NOMINMAX
#define NOMINMAX_here_defined
#endif
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#ifdef CINTERFACE_here_defined
#undef CINTERFACE_here_defined
#undef CINTERFACE
#endif
#ifdef NOMINMAX_here_defined
#undef NOMINMAX_here_defined
#undef NOMINMAX
#endif

#else

#include <dlfcn.h>

#endif

namespace common{ namespace system {


void* LoadDynLib(const char* a_libraryName)
{
#ifdef _WIN32
    HMODULE libModule = LoadLibraryA(a_libraryName);
    return static_cast<void*>(libModule);
#else
    return dlopen(a_libraryName,RTLD_LAZY);
#endif
}


void* GetSymbolAddress(void* a_libraryHandle, const char* a_symbolName)
{
#ifdef _WIN32
    HMODULE libModule = static_cast<HMODULE>(a_libraryHandle);
    return GetProcAddress(libModule,a_symbolName);
#else
    return dlsym(a_libraryHandle,a_symbolName);
#endif
}


void UnloadDynLib(void* a_libraryHandle)
{
#ifdef _WIN32
    HMODULE libModule = static_cast<HMODULE>(a_libraryHandle);
    FreeLibrary(libModule);
#else
    dlclose(a_libraryHandle);
#endif
}

}} // namespace common{ namespace system {
