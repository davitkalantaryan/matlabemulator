//
// file:            common/matlabemulator_compiler_internal.h
// created on:      2019 Aug 30
// created by:      Davit Kalantaryan (davit.kalantaryan@desy.de)
//
// This header is for ...
//

#ifndef COMMON_MATLABEMULATOR_COMPILER_INTERNAL_H
#define COMMON_MATLABEMULATOR_COMPILER_INTERNAL_H

#ifdef _MSC_VER

#define CPP_VARIABLE_C2         extern "C"

#define LIKELY_VALUE2(_x,_val)  (_x)

#define EXPORT_FROM_LIB2        __declspec(dllexport)
#define IMPORT_FROM_LIB2        __declspec(dllimport)
#define HIDE_SYMBOL2

#elif defined(__GNUC__)

#define CPP_VARIABLE_C2

#define LIKELY2(_x)             __builtin_expect(!!(_x), 1)
#define UNLIKELY2(_x)           __builtin_expect(!!(_x), 0)
#define LIKELY_VALUE2(_x,_val)  __builtin_expect((_x), (_val))

#define EXPORT_FROM_LIB2
#define IMPORT_FROM_LIB2
#define HIDE_SYMBOL2            __attribute__((visibility("hidden")))

#endif  // #ifdef _MSC_VER

#ifdef __cplusplus

#define VARIABLE_C2     CPP_VARIABLE_C2
#define EXTERN_C2       extern "C"
#define BEGIN_C_DECL2   extern "C"{
#define END_C_DECL2     }

#if 1 // later on should be checked whether we have cpp11+
#define NEWNULLPTR2     nullptr
#else
#define NEWNULLPTR2     NULL
#endif

#else

#define VARIABLE_C2
#define EXTERN_C2      extern
#define BEGIN_C_DECL2
#define END_C_DECL2

#define NEWNULLPTR2     NULL

#endif  // #ifdef __cplusplus

#endif  // #ifndef COMMON_MATLABEMULATOR_COMPILER_INTERNAL_H
