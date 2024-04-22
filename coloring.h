/* 
 * coloring.h --
 *
 *	Coloring Schemes, these map from the final iteration value to a color.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef COLORING_H
#define COLORING_H

#include "conf.h"
#include <math.h>
#include <tcl.h>
#include <tk.h>
#include "fract.h"
#include "raster.h"

static inline int NormalColor(IterResult res)
{
  if(res.iter>=maxiter)
    return inside;
  else
    return (res.iter%raster->colors);
}

extern double slope,maxcolor;

static inline int PotentialColor(IterResult res)
{
  register double pot;
  register int colors=raster->colors;
  
  if(res.iter>=maxiter)
    return inside;

  if((pot=ldexp(1.0,res.iter+2))!=HUGE)
    pot=log(res.re*res.re+res.im*res.im)/pot;
  else
    pot=0.0;
  
  pot=maxcolor-slope*sqrt(pot)-1.0;
  
  if(pot<1.0)
    return 1;
  
  if(pot>=colors)
    return colors-1;
  else
    return (int)pot;
}

extern double arity;

static inline int DecompositionColor(IterResult res)
{
  if(res.iter>=maxiter)
    return inside;
  else
    return ((int)((atan2(res.re,res.im)/(M_PI+M_PI)+0.75)*arity)%raster->colors);
}

static inline int FractColor(IterResult res)
{
  switch(scheme)
    {
    case 0:
      return NormalColor(res);
      break;
    case 1:
      return PotentialColor(res);
      break;
    case 2:
      return DecompositionColor(res);
      break;
    default:
      return 0;
    }
}

#endif /* COLORING_H */
