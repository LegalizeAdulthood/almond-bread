/* 
 * scan.c --
 *
 *	Scan Image Generation Method, "Single Pass Method", computes every
 *      pixel and plots it.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "conf.h"
#include <tcl.h>
#include <tk.h>
#include "raster.h"
#include "fract.h"
#include "coloring.h"
#include "iterate.h"

void ScanCompute(double minr, double mini, double maxr, double maxi,
		 int xoffset, int yoffset, int maxx, int maxy)
{
  int x,y,c;
  double re,im;

  for(y=yoffset, im=mini; y<=maxy; y++, im+=stepy)
    for(x=xoffset, re=minr; x<=maxx; x++, re+=stepx)
      {
	c=FractColor(FractIterate(re,im,re,im,0));
	RasterPutpixel(raster,x,y,c);
	if(symmetric)
	  RasterPutpixel(raster,x,symy-(y-yoffset),c);

#ifndef MULTITHREADING
	FractCheckEvent();
#endif
	if(abbruch) return;
      }
}
