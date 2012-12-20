#ifndef _ARCH_TYPES_H_
#define _ARCH_TYPES_H_

#include <stddef.h>

typedef signed char s8;
typedef unsigned char u8;

typedef signed short s16;
typedef unsigned short u16;

typedef signed int s32;
typedef unsigned int u32;

typedef signed long long s64;
typedef unsigned long long u64;

#ifndef _SIZE_T_DEFINED_
typedef unsigned long size_t;
#endif
typedef long          ssize_t;
typedef long long     off_t;

#endif
