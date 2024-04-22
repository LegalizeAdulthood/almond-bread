/* 
 * trace.c --
 *
 *	Install Traces on Tcl variables, this installs all traces and provides
 *      the callbacks.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "conf.h"
#if STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#ifndef HAVE_STRDUP
#include "string.h"
#endif
#include <float.h>
#else
#include "stdlib.h"
#include "string.h"
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifndef DBL_MAX
#define DBL_MAX 1.7976931348623157E+308
#endif
#ifndef INT_MAX
#define INT_MAX 2147483647
#endif

#include <tcl.h>
#include <tk.h>
#include "fract.h"
#include "raster.h"
#include "coloring.h"
#include "trace.h"

static IntTrace
  Compute,
  Maxiter,
  Iterate,
  Method,
  Scheme,
  Inside,
  Colors,
  Interleave,
  Goscanat,
  Minprogress,
  Force;

static DoubleTrace
  Minr,
  Maxr,
  Mini,
  Maxi,
  Bailout,
  Slope,
  Maxcolor,
  Arity,
  Tolerance;

static char dummystring[255];

char *IntCallback(ClientData data, Tcl_Interp *interp,
		  char *name1, char *name2,
		  int flags)
{
  static char *error=NULL;
  char *value;
  int ivalue;
  int err;
  IntTrace *var=(IntTrace *)data;

  /* make sure this gets called first */
  Tcl_UntraceVar2(interp,name1,name2,flags,IntCallback,data);
  Tcl_TraceVar2(interp,name1,name2,flags,IntCallback,data);
  
  if(error!=NULL)
    {
      free(error);
      error=NULL;
    }

  value=Tcl_GetVar2(interp, name1, name2, flags&TCL_GLOBAL_ONLY);
  if(value!=NULL)
    {
      if((err=Tcl_GetInt(interp,value,&ivalue))!=TCL_OK||
	 ivalue<var->min||
	 ivalue>var->max)
	{
	  sprintf(dummystring,"%d",*(var->value));
	  Tcl_SetVar2(interp,name1,name2,dummystring,flags&TCL_GLOBAL_ONLY);
	  if(ivalue<var->min||ivalue>var->max)
	    return "value out of range";
	  else
	    return error=strdup(interp->result);
	}
      else
	*(var->value)=ivalue;
    }

  if((var->value)!=(&compute))
    abbruch=TRUE;
  
  return NULL;
}

char *DoubleCallback(ClientData data, Tcl_Interp *interp,
		     char *name1, char *name2,
		     int flags)
{
  static char *error=NULL;
  char *value;
  double dvalue;
  int err;
  DoubleTrace *var=(DoubleTrace *)data;

  /* make sure this gets called first */
  Tcl_UntraceVar2(interp,name1,name2,flags,DoubleCallback,data);
  Tcl_TraceVar2(interp,name1,name2,flags,DoubleCallback,data);

  if(error!=NULL)
    {
      free(error);
      error=NULL;
    }
  
  value=Tcl_GetVar2(interp, name1, name2, flags&TCL_GLOBAL_ONLY);
  if(value!=NULL)
    {
      if((err=Tcl_GetDouble(interp,value,&dvalue))!=TCL_OK||
	 dvalue<var->min||
	 dvalue>var->max)
	{
	  sprintf(dummystring,"%.*g",16,*(var->value));
	  Tcl_SetVar2(interp,name1,name2,dummystring,flags&TCL_GLOBAL_ONLY);
	  if(dvalue<var->min||dvalue>var->max)
	    return "value out of range";
	  else
	    return error=strdup(interp->result);
	}
      else
	*(var->value)=dvalue;
    }

  abbruch=TRUE;
    
  return NULL;
}

void InstallTraces(Tcl_Interp *interp)
{
  compute=0;
  Compute.value=(&compute);
  Compute.min=0;
  Compute.max=1;
  Compute.canchange=TRUE;
  Tcl_TraceVar(interp,"compute",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Compute);
  sprintf(dummystring,"%d",compute);
  Tcl_SetVar(interp,"compute",dummystring,TCL_GLOBAL_ONLY);
  
  maxiter=152;
  Maxiter.value=(&maxiter);
  Maxiter.min=1;
  Maxiter.max=INT_MAX;
  Tcl_TraceVar(interp,"maxiter",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Maxiter);
  sprintf(dummystring,"%d",maxiter);
  Tcl_SetVar(interp,"maxiter",dummystring,TCL_GLOBAL_ONLY);
      
  iterate=2;
  Iterate.value=(&iterate);
  Iterate.min=0;
  Iterate.max=2;
  Iterate.canchange=TRUE;
  Tcl_TraceVar(interp,"iterate",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Iterate);
  sprintf(dummystring,"%d",iterate);
  Tcl_SetVar(interp,"iterate",dummystring,TCL_GLOBAL_ONLY);
      
  method=1;
  Method.value=(&method);
  Method.min=0;
  Method.max=4;
  Method.canchange=TRUE;
  Tcl_TraceVar(interp,"method",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Method);
  sprintf(dummystring,"%d",method);
  Tcl_SetVar(interp,"method",dummystring,TCL_GLOBAL_ONLY);

  scheme=0;
  Scheme.value=(&scheme);
  Scheme.min=0;
  Scheme.max=2;
  Tcl_TraceVar(interp,"scheme",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Scheme);
  sprintf(dummystring,"%d",scheme);
  Tcl_SetVar(interp,"scheme",dummystring,TCL_GLOBAL_ONLY);
  
  inside=1;
  Inside.value=(&inside);
  Inside.min=0;
  Inside.max=raster->colors;
  Tcl_TraceVar(interp,"inside",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Inside);
  sprintf(dummystring,"%d",inside);
  Tcl_SetVar(interp,"inside",dummystring,TCL_GLOBAL_ONLY);

  Colors.value=(&(raster->colors));
  Colors.min=raster->colors;
  Colors.max=raster->colors;
  Tcl_TraceVar(interp,"colors",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Colors);
  sprintf(dummystring,"%d",raster->colors);
  Tcl_SetVar(interp,"colors",dummystring,TCL_GLOBAL_ONLY);

  interleave=4;
  Interleave.value=(&interleave);
  Interleave.min=1;
  Interleave.max=INT_MAX;
  Tcl_TraceVar(interp,"interleave",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Interleave);
  sprintf(dummystring,"%d",interleave);
  Tcl_SetVar(interp,"interleave",dummystring,TCL_GLOBAL_ONLY);

  goscanat=6;
  Goscanat.value=(&goscanat);
  Goscanat.min=1;
  Goscanat.max=INT_MAX;
  Tcl_TraceVar(interp,"goscanat",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Goscanat);
  sprintf(dummystring,"%d",goscanat);
  Tcl_SetVar(interp,"goscanat",dummystring,TCL_GLOBAL_ONLY);

  minprogress=10;
  Minprogress.value=(&minprogress);
  Minprogress.min=1;
  Minprogress.max=INT_MAX;
  Tcl_TraceVar(interp,"minprogress",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Minprogress);
  sprintf(dummystring,"%d",minprogress);
  Tcl_SetVar(interp,"minprogress",dummystring,TCL_GLOBAL_ONLY);

  force=FALSE;
  Force.value=(&force);
  Force.min=0;
  Force.max=1;
  Force.canchange=TRUE;
  Tcl_TraceVar(interp,"force",TCL_TRACE_WRITES,IntCallback,
	       (ClientData)&Force);
  sprintf(dummystring,"%d",force);
  Tcl_SetVar(interp,"force",dummystring,TCL_GLOBAL_ONLY);
  
  minr=(-2.5);
  Minr.value=(&minr);
  Minr.min=(-DBL_MAX);
  Minr.max=DBL_MAX;
  Minr.canchange=TRUE;
  Tcl_TraceVar(interp,"minr",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Minr);
  sprintf(dummystring,"%.*g",16,minr);
  Tcl_SetVar(interp,"minr",dummystring,TCL_GLOBAL_ONLY);

  maxr=1.5;
  Maxr.value=(&maxr);
  Maxr.min=(-DBL_MAX);
  Maxr.max=DBL_MAX;
  Maxr.canchange=TRUE;
  Tcl_TraceVar(interp,"maxr",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Maxr);
  sprintf(dummystring,"%.*g",16,maxr);
  Tcl_SetVar(interp,"maxr",dummystring,TCL_GLOBAL_ONLY);

  mini=(-1.25);
  Mini.value=(&mini);
  Mini.min=(-DBL_MAX);
  Mini.max=DBL_MAX;
  Mini.canchange=TRUE;
  Tcl_TraceVar(interp,"mini",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Mini);
  sprintf(dummystring,"%.*g",16,mini);
  Tcl_SetVar(interp,"mini",dummystring,TCL_GLOBAL_ONLY);

  maxi=1.25;
  Maxi.value=(&maxi);
  Maxi.min=(-DBL_MAX);
  Maxi.max=DBL_MAX;
  Maxi.canchange=TRUE;
  Tcl_TraceVar(interp,"maxi",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Maxi);
  sprintf(dummystring,"%.*g",16,maxi);
  Tcl_SetVar(interp,"maxi",dummystring,TCL_GLOBAL_ONLY);

  bailout=16.0;
  Bailout.value=(&bailout);
  Bailout.min=0.0;
  Bailout.max=DBL_MAX;
  Tcl_TraceVar(interp,"bailout",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Bailout);
  sprintf(dummystring,"%.*g",16,bailout);
  Tcl_SetVar(interp,"bailout",dummystring,TCL_GLOBAL_ONLY);

  slope=250.0;
  Slope.value=(&slope);
  Slope.min=(-DBL_MAX);
  Slope.max=DBL_MAX;
  Tcl_TraceVar(interp,"slope",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Slope);
  sprintf(dummystring,"%.*g",16,slope);
  Tcl_SetVar(interp,"slope",dummystring,TCL_GLOBAL_ONLY);

  maxcolor=raster->colors-1;
  Maxcolor.value=(&maxcolor);
  Maxcolor.min=1.0;
  Maxcolor.max=DBL_MAX;
  Tcl_TraceVar(interp,"maxcolor",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Maxcolor);
  sprintf(dummystring,"%.*g",16,maxcolor);
  Tcl_SetVar(interp,"maxcolor",dummystring,TCL_GLOBAL_ONLY);

  arity=2.0;
  Arity.value=(&arity);
  Arity.min=(-DBL_MAX);
  Arity.max=DBL_MAX;
  Tcl_TraceVar(interp,"arity",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Arity);
  sprintf(dummystring,"%.*g",16,arity);
  Tcl_SetVar(interp,"arity",dummystring,TCL_GLOBAL_ONLY);

  tolerance=0.1;
  Tolerance.value=(&tolerance);
  Tolerance.min=0.0;
  Tolerance.max=DBL_MAX;
  Tcl_TraceVar(interp,"tolerance",TCL_TRACE_WRITES,DoubleCallback,
	       (ClientData)&Tolerance);
  sprintf(dummystring,"%.*g",16,tolerance);
  Tcl_SetVar(interp,"tolerance",dummystring,TCL_GLOBAL_ONLY);
}
