/* 
 * fract.h --
 *
 *	Fractal Information Header File, this provides all declarations for
 *      calculation-related stuff.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef FRACT_H
#define FRACT_H

#include "conf.h"
#include <tcl.h>
#include <tk.h>

extern int abbruch;
extern int compute;
extern int nochange;
extern int force;

extern int maxiter;
extern double minr,maxr,mini,maxi;
extern double bailout;
extern double stepx,stepy,equal;
extern int symmetric;
extern int maxx,maxy,symy;
extern int inside;

typedef struct IterResult
{
  double re,im;
  int iter;
} IterResult;

extern void (*FractCompute[])(double, double, double, double,
			      int, int, int, int);
extern int method;

extern int iterate;

extern int scheme;

#if (TK_MAJOR_VERSION == 4) && (TK_MINOR_VERSION == 0)
#define Tk_GetNumMainWindows() tk_NumMainWindows
#endif

static inline void FractCheckEvent(void)
{
  while(Tk_DoOneEvent(TK_ALL_EVENTS|TK_DONT_WAIT));
  if(Tk_GetNumMainWindows()<=0||
     !compute)
    abbruch=TRUE;
}

extern void FractMain(int argc, char **argv, Tcl_AppInitProc *appInitProc);
extern void FractMainLoop(void);

extern int complete;
extern double prevminr,prevmaxr,prevmini,prevmaxi;

extern void ScanCompute(double minr, double mini, double maxr, double maxi,
			int xoffset, int yoffset, int maxx, int maxy);

extern int interleave;

extern void InterleaveCompute(double minr, double mini, double maxr,
			      double maxi,
			      int xoffset, int yoffset, int maxx, int maxy);

extern int goscanat;

extern void TesseralCompute(double minr, double mini, double maxr, double maxi,
			    int xoffset, int yoffset, int maxx, int maxy);

extern void BoundaryCompute(double minr, double mini, double maxr, double maxi,
			    int xoffset, int yoffset, int maxx, int maxy);

extern int minprogress;
extern double tolerance;

extern void SOICompute(double minr, double mini, double maxr, double maxi,
		       int xoffset, int yoffset, int maxx, int maxy);

#endif /* FRACT_H */
