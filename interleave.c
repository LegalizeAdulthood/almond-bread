/* 
 * interleave.c --
 *
 *      Interleave Image Generation Method, computes only every nth pixel and
 *      draws consecutive pixels of same color as a line, a little bit like
 *      Fractint's solid guessing method.
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

int interleave;

void InterleaveCompute(double minr, double mini, double maxr, double maxi,
		       int xoffset, int yoffset, int maxx, int maxy)
{
  int x,y,symax=symy,z,color;
  int savex;
  int nustep=interleave;
  double re,im;
  double nustepx=nustep*stepx;
  double minrplusnu=minr+nustepx;
  double help;
  int savecolor,helpcolor;
  
  for (y = yoffset, im = mini; y <= maxy; y++, im += stepy, symax--)
    {
#ifndef MULTITHREADING
      FractCheckEvent();
#endif
      if(abbruch) return;
      
      savecolor=FractColor(FractIterate(minr,im,minr,im,0));
      
      savex = xoffset;
      for (x = nustep+xoffset, re = minrplusnu; x < maxx;
	   x += nustep, re += nustepx)
	{
	  color=FractColor(FractIterate(re,im,re,im,0));
	  if (color == savecolor)
	    continue;
	  z=x-1;
	  help=re-stepx;
	  helpcolor=FractColor(FractIterate(help,im,help,im,0));
	  while((z > savex) && (helpcolor != savecolor))
	    {
	      RasterPutpixel(raster,z,y,helpcolor);
	      if(symmetric)
		RasterPutpixel(raster,z,symax,helpcolor);
	      z--;
	      help-=stepx;
	      helpcolor=FractColor(FractIterate(help,im,help,im,0));
	    }
	  RasterPutline(raster, savex, y, z, savecolor);
	  if(symmetric)
	    RasterPutline(raster, savex, symax, z, savecolor);
	  savex = x;
	  savecolor = color;
	}
      z=maxx;
      help=re-stepx;
      helpcolor=FractColor(FractIterate(help,im,help,im,0));
      while((z > savex) && (helpcolor != savecolor))
	{
	  RasterPutpixel(raster,z,y,helpcolor);
	  if(symmetric)
	    RasterPutpixel(raster,z,symax,helpcolor);
	  z--;
	  help-=stepx;
	  helpcolor=FractColor(FractIterate(help,im,help,im,0));
	}
      RasterPutline(raster, savex, y, z, savecolor);
      if(symmetric)
	RasterPutline(raster, savex, symax, z, savecolor);
      savex = x;
    }
}
