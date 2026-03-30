/* Copyright 1997 by Takashi KANAI */
#include <stdio.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <assert.h>

/* Small safety helpers.
 * - XMALLOC: aborts on OOM instead of returning NULL.
 * - XFREE: frees and NULLs a pointer variable (to reduce double-free risk).
 * - GMORPH_ASSERT: prints location and aborts on violated invariants.
 */
static inline void *gmorph_xmalloc(size_t size, const char *file, int line)
{
  void *p = malloc(size);
  if (p == NULL) {
    fprintf(stderr, "GMorph: out of memory (%zu) at %s:%d\n", size, file, line);
    abort();
  }
  return p;
}

#define XMALLOC(sz) gmorph_xmalloc((sz), __FILE__, __LINE__)

#define XFREE(ptr)                           \
  do {                                       \
    if ((ptr) != NULL) {                    \
      free((ptr));                           \
      (ptr) = NULL;                          \
    }                                        \
  } while (0)

#define GMORPH_ASSERT(cond) do {                                                    \
  if (!(cond)) {                                                                      \
    fprintf(stderr, "GMorph: assert failed: %s at %s:%d\n", #cond, __FILE__, __LINE__); \
    abort();                                                                         \
  }                                                                                    \
} while (0)
