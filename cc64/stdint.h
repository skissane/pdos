/* Public domain implementation of stdint.h,
   originally contributed by Simon Kissane */

#ifndef _STDINT_INCLUDED
#define _STDINT_INCLUDED

#define INT8_MIN -128
#define INT16_MIN -32768
#define INT32_MIN -2147483648
#define INT8_MAX 127
#define INT16_MAX 32767
#define INT32_MAX 2147483647
#define UINT8_MAX 0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffff

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;
typedef signed long intmax_t;
typedef unsigned long uintmax_t;
typedef signed long intptr_t;
typedef unsigned long uintptr_t;

#define INTMAX_MIN INT32_MIN
#define INTMAX_MAX INT32_MAX
#define UINTMAX_MAX UINT32_MAX
#define INTPTR_MIN INT32_MIN
#define INTPTR_MAX INT32_MAX
#define UINTPTR_MAX UINT32_MAX

#endif
