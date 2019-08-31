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
#define LIKELY_VALUE2(_x,_val)  (_x)
#elif defined(__GNUC__)
#define LIKELY2(_x)             __builtin_expect(!!(_x), 1)
#define UNLIKELY2(_x)           __builtin_expect(!!(_x), 0)
#define LIKELY_VALUE2(_x,_val)  __builtin_expect((_x), (_val))
#endif

#endif  // #ifndef COMMON_MATLABEMULATOR_COMPILER_INTERNAL_H
