/* 
 * conventional.h --
 *
 *      Conventional Iteration Method, aka Level Set Method in Science of
 *      Fractal Images, just iterates z->z^2+c until maxiter is reached or
 *      |z| exceeds bailout.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef CONVENTIONAL_H
#define CONVENTIONAL_H

#include "conf.h"
#include "fract.h"

static inline
IterResult ConventionalIterate(register double cre, register double cim,
			 register double re, register double im,
			 int start)
{
  register int i=start;
  register double rq=re*re,iq=im*im;
  register IterResult res;
  
  do
    {   
      im=(im+im)*re+cim;
      re=rq-iq+cre;
      rq=re*re;
      iq=im*im;
    }
  while (i++<=maxiter && rq+iq<bailout);

  res.iter=i;
  res.re=re;
  res.im=im;

  return res;
}

#endif /* CONVENTIONAL_H */
