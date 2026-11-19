
#if !defined _JAVA_TYPES_H_
#error "Never include <java_types_md.h> directly; use <java_types.h> instead."
#endif

#ifndef _JAVA_TYPES_MD_H_
#define _JAVA_TYPES_MD_H_

#include <javacall_defs.h>

/** Byte parameter type. */
typedef signed char jbyte;

/** Char parameter type. */
typedef javacall_utf16 jchar;

/** Integer parameter type. */
typedef int jint;

/** 64-bit parameter type. */
typedef javacall_int64 jlong;

/** Platform-specific 64-bit integer type specifier. */
#define PCSL_LLD "%I64d"

#endif /* !_JAVA_TYPES_MD_H_ */
