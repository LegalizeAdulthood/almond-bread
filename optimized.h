/* 
 * optimized.h --
 *
 *	Optimized Iteration Method, this uses periodicity and cardioid
 *      checking.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef OPTIMIZED_H
#define OPTIMIZED_H

#include "conf.h"
#include <math.h>
#include "fract.h"

static inline
IterResult OptimizedIterate(register double cre, register double cim,
			    register double re, register double im,
			    int start)
{
  static int basin=FALSE;
  register int i=start;
  register int check,every;
  register double rq=re*re,iq=im*im;
  register double sre,sim;
  register IterResult res;
  
  if(basin)
    {
      if(fabs(re-cre)<=stepx)
	if(re<=-0.75)              
	  {                         
	    if(rq+iq+re+re<=-0.9375)
	      {
		res.re=re; res.im=im;
		res.iter=maxiter;
		return res;
	      }
	  }
	else if(im<=-0.6495)
	  {
	    register double foo,bar;
	    foo=re+0.1226; bar=im+0.7449;
	    if(foo*foo+bar*bar<=0.009097525)
	      {
		res.re=re; res.im=im;
		res.iter=maxiter;
		return res;
	      }
	  }
	else
	  {
	    register double foo;
	    foo=rq+iq; 
	    if(foo*(8.0*foo-3.0)+re<=0.09375)
	      {
		res.re=re; res.im=im;
		res.iter=maxiter;
		return res;
	      }
	  }
      
      check=0; every=(maxiter>>3);
      sre=re; sim=im;
      
      do
	{   
	  im=(im+im)*re+cim;
	  re=rq-iq+cre;
	  rq=re*re;
	  iq=im*im;
	  if(sre-re<equal && re-sre<equal && sim-im<equal && im-sim<equal)
	    {
	      res.re=re; res.im=im;
	      res.iter=maxiter;
	      return res;
	    }
	  if(check++>=every) { check=0; sre=re; sim=im; }
	}
      while (i++<=maxiter && rq+iq<bailout);
      
      basin=(i>=maxiter);
      res.re=re;
      res.im=im;
      res.iter=i;
      
      return res;
    }
  
  do
    {   
      im=(im+im)*re+cim;
      re=rq-iq+cre;
      rq=re*re;
      iq=im*im;
    }
  while (i++<=maxiter && rq+iq<bailout);
  
  basin=(i>=maxiter);
  res.re=re;
  res.im=im;
  res.iter=i;
  
  return res;
}

#endif /* OPTIMIZED_H */
