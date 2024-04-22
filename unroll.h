/* 
 * unroll.h --
 *
 *	Unroll Iteration Method, this employs a technique of checking for
 *      bailout only every 8 iterations, thus making it possible to save one
 *      fp multiplication per iteration.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef UNROLL_H
#define UNROLL_H

#include "conf.h"
#include <math.h>
#include "fract.h"
#include "ieee754.h"

#if #cpu (i386) || defined(i386) || defined(__i386) || defined(__i386) || defined(__i486)
#define INTEL
#endif

static unsigned char adjust[256]={0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8};
				  
static inline
IterResult UnrollIterate(register double cre, register double cim,
			 register double re, register double im,
			 int start)
{
  static int basin=FALSE;
  register int iter,offset=0,k,n;
  register double ren,imn,sre,sim;
#ifdef INTEL
  register float mag;
  float b=(float)bailout,e=(float)equal;
  register unsigned long bail=*(unsigned long *)&b,magi;
  register unsigned long eq=*(unsigned long *)&e;
#else
  register double mag;
#endif
  union ieee754_double d;
  int exponent;
  register IterResult res;
  
  if(basin)
    {
      if(fabs(re-cre)<=stepx)
	if(re<=-0.75)              
	  {                         
	    if(re*re+im*im+re+re<=-0.9375)
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
	    foo=re*re+im*im; 
	    if(foo*(8.0*foo-3.0)+re<=0.09375)
	      {
		res.re=re; res.im=im;
		res.iter=maxiter;
		return res;
	      }
	  }

      sre=re; sim=im;
      ren=re*re;
      imn=im*im;
      if(start!=0)
	{
	  offset=maxiter-start+7;
	  iter=offset>>3;
	  offset&=7;
	  offset=(8-offset);
	}
      else
	iter=maxiter>>3;
      k=n=8;
      
      do
	{
	  im=im*re;
	  re=ren-imn;
	  im+=im;
	  re+=cre;
	  im+=cim;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;
	  
#ifdef INTEL
	  mag=fabs(sre-re);
	  magi=*(unsigned long *)&mag;
	  if(magi<eq)
	    {
	      mag=fabs(sim-im);
	      magi=*(unsigned long *)&mag;
	      if(magi<eq)
		{
		  res.re=re; res.im=im;
		  res.iter=maxiter;
		  return res;
		}
	    }
#else /* INTEL */
	  if(fabs(sre-re)<equal&&fabs(sim-im)<equal)
	    {
	      res.re=re; res.im=im;
	      res.iter=maxiter;
	      return res;
	    }
#endif /* INTEL */

	  k-=8;
	  if(k<=0)
	    {
	      n<<=1;
	      sre=re;
	      sim=im;
	      k=n;
	    }

	  imn=im*im;
	  ren=re*re;
	  mag=ren+imn;
#ifdef INTEL
	  magi=*(unsigned long *)&mag;
#endif
	}
#ifdef INTEL
      while (magi<bail&&--iter!=0);
#else
      while (mag<bailout&&--iter!=0);
#endif
    }
  else
    {
      ren=re*re;
      imn=im*im;
      if(start!=0)
	{
	  offset=maxiter-start+7;
	  iter=offset>>3;
	  offset&=7;
	  offset=(8-offset);
	}
      else
	iter=maxiter>>3;
      
      do
	{      
	  im=im*re;
	  re=ren-imn;
	  im+=im;
	  re+=cre;
	  im+=cim;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*re;
	  ren=re+im;
	  re=re-im;
	  imn+=imn;
	  re=ren*re;
	  im=imn+cim;
	  re+=cre;

	  imn=im*im;
	  ren=re*re;
	  mag=ren+imn;
#ifdef INTEL
	  magi=*(unsigned long *)&mag;
#endif
	}
#ifdef INTEL
      while (magi<bail&&--iter!=0);
#else
      while (mag<bailout&&--iter!=0);
#endif
    }
  
  if(iter==0)
    {
      basin=TRUE;
      res.re=re; res.im=im;
      res.iter=maxiter;
      return res;
    }
  else
    {
      basin=FALSE;
#ifdef INTEL
      d.d=ren+imn;
#else
      d.d=mag;
#endif
      exponent=(d.ieee.exponent-_IEEE754_DOUBLE_BIAS)>>3;

      res.re=re; res.im=im;
      res.iter=(maxiter+offset-(((iter-1)<<3)+adjust[exponent]));
      return res;
    }
}

#endif /* UNROLL_H */
