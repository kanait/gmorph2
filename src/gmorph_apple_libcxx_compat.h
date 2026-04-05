/*
 * libc++ __locale_dir/support/bsd_like.h calls ::strtof_l, ::snprintf_l, ::isdigit_l, …
 * On some Xcode + Homebrew Qt setups, system headers included via -include still do not
 * expose those names in the global namespace before <locale>. Explicit C declarations fix it.
 *
 * Symbols are implemented in libSystem (libc); we only declare. Duplicate declarations
 * after <stdlib.h> are OK if signatures match.
 *
 * Apple-only; skipped when __cplusplus is undefined (.c with -include).
 */
#if defined(__APPLE__) && defined(__cplusplus)

#undef __STRICT_ANSI__
#undef _ANSI_SOURCE
#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _DARWIN_C_SOURCE
#define _DARWIN_C_SOURCE 1
#endif

#include <stddef.h>

extern "C" {
#include <Xlocale.h>

float strtof_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t);
double strtod_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t);
long double strtold_l(const char *__restrict __nptr, char **__restrict __endptr, locale_t);
long long strtoll_l(const char *__restrict __nptr, char **__restrict __endptr, int __base,
                     locale_t);
unsigned long long strtoull_l(const char *__restrict __nptr, char **__restrict __endptr,
                               int __base, locale_t);

int snprintf_l(char *__restrict __str, size_t __size, locale_t __loc,
               const char *__restrict __format, ...);
int asprintf_l(char **__restrict __ret, locale_t __loc, const char *__restrict __format, ...);
int sscanf_l(const char *__restrict __str, locale_t __loc, const char *__restrict __format, ...);

int isdigit_l(int __c, locale_t __loc);
int isxdigit_l(int __c, locale_t __loc);
}

#endif
