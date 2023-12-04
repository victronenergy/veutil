#pragma once

/*
 * This file should be rather straightforward since only gcc / clang is
 * supported, which do have a decent stdlib, e.g.
 *   - for booleans use stdbool.h
 *   - for types use stdint.h
 */

#ifndef VE_DCL
# ifdef __cplusplus
#  define VE_DCL  extern "C"
# else
#  define VE_DCL
# endif
#endif

#ifndef VE_UNUSED
# define VE_UNUSED(a)	(void)(a)
#endif

#ifndef ARRAY_LENGTH
# define ARRAY_LENGTH(a)	(sizeof(a) / sizeof(a[0]))
#endif