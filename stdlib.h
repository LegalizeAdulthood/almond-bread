#ifndef STDLIB_H
#define STDLIB_H

/* declarations for systems that do not have stdlib.h. */

#ifndef size_t
#include <sys/types.h>
#endif

/* allocate and free dynamic memory */
extern void *malloc(size_t size);
extern void free(void *ptr);

/* cause program termination */
extern void exit(int status);

#endif /* STDLIB_H */
