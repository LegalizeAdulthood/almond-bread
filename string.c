/*
 *  linux/lib/string.c
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 */

#include "conf.h"
#if STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else
#include "stdlib.h"
#include "string.h"
#endif

#ifndef HAVE_STRNCMP
char * ab_strcpy(char * dest,const char *src)
{
  char *tmp = dest;
  
  while ((*dest++ = *src++) != '\0')
    /* nothing */;
  return tmp;
}

char * ab_strncpy(char * dest,const char *src,size_t count)
{
  char *tmp = dest;
  
  while (count-- && (*dest++ = *src++) != '\0')
    /* nothing */;
  
  return tmp;
}

int ab_strcmp(const char * cs,const char * ct)
{
  register signed char res;
  
  while (1) {
    if ((res = *cs - *ct++) != 0 || !*cs++)
      break;
  }

  return (int)res;
}

int ab_strncmp(const char * cs,const char * ct,size_t count)
{
  register signed char res = 0;
  
  while (count) {
    if ((res = *cs - *ct++) != 0 || !*cs++)
      break;
    count--;
  }
  
  return (int)res;
}

size_t ab_strlen(const char * s)
{
  const char *sc;
  
  for (sc = s; *sc != '\0'; ++sc)
    /* nothing */;
  return (size_t)(sc - s);
}

size_t ab_strnlen(const char * s, size_t count)
{
  const char *sc;
  
  for (sc = s; *sc != '\0' && count--; ++sc)
    /* nothing */;
  return (size_t)(sc - s);
}

#endif /* HAVE_STRNCMP */

#ifndef HAVE_MEMCPY
void * ab_memset(void * s,int c,size_t count)
{
  char *xs = (char *) s;
  
  while (count--)
    *xs++ = (char)c;
  
  return (void *)s;
}

void * ab_memcpy(void * dest,const void *src,size_t count)
{
  char *tmp = (char *) dest, *s = (char *) src;
  
  while (count--)
    *tmp++ = *s++;
  
  return (void *)dest;
}
#endif /* HAVE_MEMCPY */

#ifndef HAVE_STRDUP
char * ab_strdup(const char *s)
{
  char *p;

  p=(char *)malloc(strlen(s)+1);
  
  if(p!=NULL)
    strcpy(p,s);

  return p;
}
#endif /* HAVE_STRDUP */
