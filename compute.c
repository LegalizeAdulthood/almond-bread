/* 
 * compute.c --
 *
 *	Compute Image, this module implements the main loop and dispatches
 *      the actual calculation calls to the chosen computation methods.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "conf.h"
#include <stdio.h>
#include <math.h>
#include <tcl.h>
#include <tk.h>
#include "fract.h"
#include "raster.h"
#include "fractint.h"
#include "thread.h"

int abbruch;
int compute;
int nochange;
int force;

int maxiter;
double minr,maxr,mini,maxi;
double bailout;
double stepx,stepy,equal;
static double sympart;
int symmetric;
static int sympixels;
int maxx,maxy,symy;
int inside;

void (*FractCompute[])(double, double, double, double,
		       int, int, int, int)=
{
  ScanCompute,
  InterleaveCompute,
  TesseralCompute,
  BoundaryCompute,
  SOICompute
};

int method;

int iterate;

int scheme;
double slope,maxcolor; /* for PotentialColor */
double arity; /* for DecompositionColor */

int symmetry[]=
{
  TRUE,TRUE,FALSE
};

int complete=FALSE;

typedef struct ComputeArgs
{
  double minr,mini,maxr,maxi;
  int xoffset,yoffset,maxx,maxy;
  Condition_t cond;
} ComputeArgs;

static void *Compute(void *arg)
{
  ComputeArgs *args=(ComputeArgs *)arg;
  double sx,sy,minr,mini,maxr;
  int xoffset,yoffset,maxx,maxy;
  
  while(1)
    {
      WaitCondition(&(args->cond),START_THREAD);
      
      sx=stepx; sy=stepy;
      minr=args->minr;
      mini=args->mini;
      maxr=args->maxr;
      maxi=args->maxi;
      xoffset=args->xoffset;
      yoffset=args->yoffset;
      maxx=args->maxx;
      maxy=args->maxy;

      stepx=(maxr-minr)/(maxx-xoffset+1);
      stepy=(maxi-mini)/(maxy-yoffset+1);
  
      if(abbruch)
	{
	  SignalCondition(&(args->cond),STOP_THREAD);
#ifdef MULTITHREADING
	  continue;
#else
	  break;
#endif
	}
  
      RasterFillbox(raster,xoffset,yoffset,
		    (maxx-xoffset)+1,(maxy-yoffset)+1,1);
  
      complete=FALSE;

      if(maxi<=0||mini>=0||!symmetry[scheme])
	{
	  symmetric=FALSE;
	  FractCompute[method](minr,mini,maxr,maxi,
			       xoffset,yoffset,maxx,maxy);
	}
      else
	{
	  sympart=((-mini)<=maxi?-mini:-maxi);
	  sympixels=(int)(fabs(sympart-mini)/stepy);
	  if(sympixels>maxy) sympixels=maxy;
	  if(sympart>0)
	    {
	      symmetric=TRUE;
	      symy=sympixels;
	      FractCompute[method](minr,mini,maxr,(mini+sympart)/2,
				   xoffset,yoffset,maxx,sympixels>>1);
	      symmetric=FALSE;
	      if(sympixels+1<=maxy)
		FractCompute[method](minr,sympart,maxr,maxi,
				     xoffset,sympixels+1,maxx,maxy);
	    }
	  else
	    {
	      symmetric=TRUE;
	      symy=maxy;
	      FractCompute[method](minr,sympart,maxr,(sympart+maxi)/2,
				   xoffset,sympixels,maxx,(sympixels+maxy)>>1);
	      symmetric=FALSE;
	      if(sympixels-1>=0)
		FractCompute[method](minr,mini,maxr,sympart,
				     xoffset,yoffset,maxx,sympixels);
	    }
	}
  
      stepx=sx;
      stepy=sy;

      SignalCondition(&(args->cond),STOP_THREAD);
#ifndef MULTITHREADING
      break;
#endif
    }

  return (void *)NULL;
}

double prevminr=0.0,prevmaxr=0.0,prevmini=0.0,prevmaxi=0.0;

void FractMainLoop(void)
{
  double saveminr,savemaxr,savemini,savemaxi;
#ifdef MULTITHREADING
  Thread_t compute_thread=DefaultThread();
#endif
  ComputeArgs args;

#ifdef MULTITHREADING
  /* create computation thread */
  InitCondition(&args.cond);
  CreateThread(compute_thread,Compute,&args);
#endif
  
  while(1)
    {
      while(!compute)
	{
	  Tk_DoOneEvent(TK_ALL_EVENTS);
	  if(Tk_GetNumMainWindows()<=0) return;
	}

      abbruch=FALSE;
      
      /* we need these, because [min|max][r|i] might be overwritten
	 during computation */
      saveminr=minr; savemaxr=maxr;
      savemini=mini; savemaxi=maxi;
      
      maxx=raster->width-1;
      maxy=raster->height-1;
      stepx=(maxr-minr)/(maxx+1);
      stepy=(maxi-mini)/(maxy+1);
      equal=(stepx>stepy?stepy:stepx);

      args.minr=minr; args.maxr=maxr;
      args.mini=mini; args.maxi=maxi;
      args.xoffset=0; args.yoffset=0;
      args.maxx=maxx; args.maxy=maxy;

#ifdef MULTITHREADING
      SignalCondition(&args.cond,START_THREAD); /* start computation thread */

      while(args.cond.value==START_THREAD)
	{
	  FractCheckEvent();
	  if(Tk_GetNumMainWindows()<=0) return;
	}
#else /* !MULTITHREADING */
      Compute(&args);
#endif /* MULTITHREADING */      
      
      if(!abbruch)
	{
	  complete=TRUE;
	  prevminr=saveminr;
	  prevmaxr=savemaxr;
	  prevmini=savemini;
	  prevmaxi=savemaxi;
	}
      loaded=OWN;
      nochange=TRUE;
      Tcl_SetVar(raster->interp,"force","0",TCL_GLOBAL_ONLY);
      Tcl_SetVar(raster->interp,"compute","0",TCL_GLOBAL_ONLY);
      SetInfo(fi);
    }
}
