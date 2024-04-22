/* 
 * iterate.h --
 *
 *	Iteration methods header file.
 *
 * Copyright (c) 1996-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef ITERATE_H
#define ITERATE_H

#include "conf.h"
#include "conventional.h"
#include "optimized.h"
#include "unroll.h"

static inline IterResult FractIterate(register double cre, register double cim,
				      register double re, register double im,
				      int start)
{
  switch(iterate)
    {
    case 0:
      return ConventionalIterate(cre,cim,re,im,start);
      break;
    case 1:
      return OptimizedIterate(cre,cim,re,im,start);
      break;
    case 2:
      return UnrollIterate(cre,cim,re,im,start);
      break;
    default:
      return UnrollIterate(cre,cim,re,im,start);
      break;
    }
}

#endif /* ITERATE_H */
