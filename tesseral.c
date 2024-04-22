/* 
 * tesseral.c --
 *
 *      Tesseral Image Generation Method. Continually subdivides image into
 *      smaller squares, filling in those of same interior color.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "conf.h"
#include <math.h>
#include <tcl.h>
#include <tk.h>
#include "raster.h"
#include "fract.h"
#include "coloring.h"
#include "iterate.h"

#define NUTTIN 1
#define BOTTOM 0
#define RIGHT 2

int goscanat;

void
  TesseralCompute(double minr,double mini,double maxr,double maxi,
		  int xoffset, int yoffset, int maxx, int maxy)
{
  register double basx,basy,HalfR,HalfI,interstep,helpre;
  register int x,y,z,color;
  register int HalfX,HalfY,Compare,savex,savecolor,helpcolor;
  register char Eeerk;
  static char uno=TRUE;
  static char opcode=BOTTOM,Xdivide=TRUE;
  static int base;

#ifndef MULTITHREADING
  FractCheckEvent();
#endif
  if(abbruch) return;
  
  if(uno)
    {
      uno=FALSE;
      base=yoffset;
      for(x=0, basx=minr; x<=maxx; x++, basx+=stepx)
	{
#ifndef MULTITHREADING
	  FractCheckEvent();
#endif
	  if(abbruch) return;

	  RasterPutpixel(raster,x,yoffset,
			 FractColor(FractIterate(basx,mini,basx,mini,0)));
	  if(symmetric)
	    {
	      RasterPutpixel(raster,x,symy,
			     FractColor(FractIterate(basx,
						     maxi+maxi-mini,
						     basx,maxi+maxi-mini,0)));
	    }
	  else
	    {
	      RasterPutpixel(raster,x,maxy,
			     FractColor(FractIterate(basx,maxi,basx,maxi,0)));
	    }
	}
      for(y=yoffset+1, basy=mini+stepy; y<=(symmetric?symy:maxy)-1; y++, basy+=stepy)
	{
#ifndef MULTITHREADING
	  FractCheckEvent();
#endif
	  if(abbruch) return;

	  RasterPutpixel(raster,0,y,
			 FractColor(FractIterate(minr,basy,minr,basy,0)));
	  RasterPutpixel(raster,maxx,y,
			 FractColor(FractIterate(maxr,basy,maxr,basy,0)));
	}
      TesseralCompute(minr,mini,maxr,maxi,xoffset,yoffset,maxx,maxy);
      uno=TRUE;
      Xdivide=TRUE;
      opcode=BOTTOM;
      return;
    }
  
  if(maxx-xoffset<=goscanat||maxy-yoffset<=goscanat)
    {
      if(opcode>=NUTTIN) maxy--;
      if(opcode<=NUTTIN) maxx--;
      interstep=stepx*interleave;
      for(y=yoffset+1, basy=mini+stepy; y<=maxy; y++, basy+=stepy)
	{
#ifndef MULTITHREADING
	  FractCheckEvent();
#endif
	  if(abbruch) return;

	  savecolor=FractColor(FractIterate(minr+stepx,
					    basy,minr+stepx,basy,0));
	  savex=xoffset+1;
	  
	  for(x=xoffset+interleave+1, basx=minr+stepx+interstep; x<=maxx;
	      x+=interleave, basx+=interstep)
	    {
	      if((color=FractColor(FractIterate(basx,basy,basx,basy,0)))
		 ==savecolor)
		continue;
	      
	      for (z=x-1, helpre=basx-stepx; z>x-interleave;
		   z--,helpre-=stepx)
		{
		  helpcolor=FractColor(FractIterate(helpre,
						    basy,helpre,basy,0));
		  if(helpcolor==savecolor)
		    break;
		  
		  RasterPutpixel(raster,z,y,helpcolor);
		  if(symmetric)
		    RasterPutpixel(raster,z,symy-(y-base),helpcolor);
		}
	      
	      RasterPutline(raster,savex,y,z,savecolor);
	      if(symmetric)
		RasterPutline(raster,savex,symy-(y-base),z,savecolor);

	      savex = x;
	      savecolor = color;
	    }

	  for (z=maxx, helpre=maxr; z>savex;
	       z--,helpre-=stepx)
	    {
	      helpcolor=FractColor(FractIterate(helpre,basy,helpre,basy,0));
	      
	      if(helpcolor==savecolor)
		break;
	      
	      RasterPutpixel(raster,z,y,helpcolor);
	      if(symmetric)
		RasterPutpixel(raster,z,symy-(y-base),helpcolor);
	    }
	  
	  RasterPutline(raster,savex,y,z,savecolor);
	  if(symmetric)
	    RasterPutline(raster,savex,symy-(y-base),z,savecolor);
	}
      return;
    }
    
  HalfX=0; HalfY=0;
  Compare=FractColor(FractIterate((minr+maxr)/2,
				  (mini+maxi)/2,
				  (minr+maxr)/2,(mini+maxi)/2,0));
  Eeerk=FALSE;
  
  if(opcode==NUTTIN)
    {
      if(!Xdivide) goto TWO;
    ONE:
      for(x=xoffset; x<=maxx; x++)
	if(RasterGetpixel(raster,x,yoffset)!=Compare||RasterGetpixel(raster,x,maxy)!=Compare)
	  { Eeerk=TRUE; HalfX=x-1; goto SORRY; }
      if(!Xdivide) goto BLIT;
    TWO:
      for(y=yoffset+1; y<=maxy-1; y++)
	if(RasterGetpixel(raster,xoffset,y)!=Compare||RasterGetpixel(raster,maxx,y)!=Compare)
	  { Eeerk=TRUE; HalfY=y-1; goto SORRY; }
      if(!Xdivide) goto ONE;
    }
  else if(opcode==RIGHT)
    {
      for(y=yoffset+1, basy=mini+stepy; y<=maxy-1; y++,basy+=stepy)   
	{
#ifndef MULTITHREADING
	  FractCheckEvent();
#endif
	  if(abbruch) return;

	  color=FractColor(FractIterate(maxr,basy,maxr,basy,0));
	  RasterPutpixel(raster,maxx,y,color);
	  if(symmetric)
	    RasterPutpixel(raster,maxx,symy-(y-base),color);
	  if(!Eeerk&&(color!=Compare||RasterGetpixel(raster,xoffset,y)!=Compare))
	    { Eeerk=TRUE; HalfY=y-1; }
	}
      if(Eeerk) goto SORRY;
      for(x=xoffset; x<=maxx; x++)
	if(RasterGetpixel(raster,x,yoffset)!=Compare||RasterGetpixel(raster,x,maxy)!=Compare)
	  { Eeerk=TRUE; HalfX=x-1; goto SORRY; }
    }
  else 
    {
      for(x=xoffset, basx=minr; x<=maxx; x++,basx+=stepx)   
	{
#ifndef MULTITHREADING
	  FractCheckEvent();
#endif
	  if(abbruch) return;

	  color=FractColor(FractIterate(basx,maxi,basx,maxi,0));
	  RasterPutpixel(raster,x,maxy,color);
	  if(symmetric)
	    RasterPutpixel(raster,x,symy-(maxy-base),color);
	  if(!Eeerk&&(color!=Compare||RasterGetpixel(raster,x,yoffset)!=Compare))
	    { Eeerk=TRUE; HalfX=x-1; }
	}
      if(Eeerk) goto SORRY;
      for(y=yoffset+1; y<=maxy-1; y++)
	if(RasterGetpixel(raster,xoffset,y)!=Compare||RasterGetpixel(raster,maxx,y)!=Compare)
	  { Eeerk=TRUE; HalfY=y-1; goto SORRY; }  
    }
 BLIT:
  RasterFillbox(raster,xoffset+1,yoffset+1,maxx-(xoffset+1),maxy-(yoffset+1),Compare);
  if(symmetric)
    RasterFillbox(raster,xoffset+1,symy-(maxy-base)+1,maxx-(xoffset+1),maxy-(yoffset+1),Compare);
  return;
 SORRY:
  if(Xdivide)
    {
      if(!HalfX||HalfX-xoffset<=goscanat||maxx-HalfX<=goscanat)
	HalfX=(maxx+xoffset)/2;
      HalfR=(HalfX-xoffset)*stepx+minr;
      Xdivide=(HalfX-xoffset>maxy-yoffset);
      opcode=RIGHT;
      TesseralCompute(minr,mini,HalfR,maxi,xoffset,yoffset,HalfX,maxy);
      Xdivide=(maxx-HalfX>maxy-yoffset);
      opcode=NUTTIN;
      TesseralCompute(HalfR,mini,maxr,maxi,HalfX,yoffset,maxx,maxy);
      return;
    }
  else
    {
      /*
      char GoneSymmetric;
      if(!symmetric) GoneSymmetric=symmetric=fabs(maxi+mini)<=stepy;
      else GoneSymmetric=FALSE;
      */
      if(!HalfY||HalfY-yoffset<=goscanat||maxy-HalfY<=goscanat)
	HalfY=(maxy+yoffset)/2;
      HalfI=(HalfY-yoffset)*stepy+mini;
      Xdivide=(maxx-xoffset>HalfY-yoffset);
      opcode=BOTTOM;
      TesseralCompute(minr,mini,maxr,HalfI,xoffset,yoffset,maxx,HalfY);
      /*if(GoneSymmetric) { symmetric=FALSE; return; }*/
      Xdivide=(maxx-xoffset>maxy-HalfY);
      opcode=NUTTIN;
      TesseralCompute(minr,HalfI,maxr,maxi,xoffset,HalfY,maxx,maxy);
      return;
    }
}
