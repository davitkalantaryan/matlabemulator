//
// file:            common/system/handlelib.hpp
// created on:      2019 Sep 1
// created by:      Davit Kalantaryan (davit.kalantaryan@desy.de)
//
// This header is for functions, that is used to call system routines and binaries
//

#ifndef COMMON_SYSTEM_HANDLELIB_HPP
#define COMMON_SYSTEM_HANDLELIB_HPP

#include <stddef.h>


namespace common{ namespace system {

void* LoadDynLib(const char* libraryName);
void* GetSymbolAddress(void* libraryHandle, const char* symbolName);
void  UnloadDynLib(void* libraryHandle);

}} // namespace common{ namespace system {



#endif  // #ifndef COMMON_SYSTEM_HANDLELIB_HPP
