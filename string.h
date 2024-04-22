#ifndef STRING_H
#define STRING_H

/* declarations for systems that do not have string.h. */

#if !(STDC_HEADERS)

#ifndef size_t
#include <sys/types.h>
#endif

#ifdef HAVE_STRNCMP
extern char * strcpy(char *,const char *);
extern char * strncpy(char *,const char *,size_t);
extern size_t strlen(const char *);
extern size_t strnlen(const char *,size_t);
extern int strcmp(const char *,const char *);
extern int strncmp(const char *,const char *,size_t);
#else
extern char * ab_strcpy(char *,const char *);
extern char * ab_strncpy(char *,const char *,size_t);
extern size_t ab_strlen(const char *);
extern size_t ab_strnlen(const char *,size_t);
extern int ab_strcmp(const char *,const char *);
extern int ab_strncmp(const char *,const char *,size_t);
#define strcpy ab_strcpy
#define strncpy ab_strncpy
#define strlen ab_strlen
#define strnlen ab_strnlen
#define strcmp ab_strcmp
#define strncmp ab_strncmp
#endif

#ifdef HAVE_STRDUP
extern char * strdup(const char *s);
#else
extern char * ab_strdup(const char *s);
#define strdup ab_strdup
#endif

#ifdef HAVE_MEMCPY
extern void * memset(void *,int,size_t);
extern void * memcpy(void *,const void *,size_t);
#else
extern void * ab_memset(void *,int,size_t);
extern void * ab_memcpy(void *,const void *,size_t);
#define memset ab_memset
#define memcpy ab_memcpy
#endif

#else /* STDC_HEADERS */

#ifdef HAVE_STRDUP
extern char * strdup(const char *s);
#else
extern char * ab_strdup(const char *s);
#define strdup ab_strdup
#endif

#endif /* STDC_HEADERS */

#endif /* STRING_H */
