/* 
 * soi.c --
 *
 *	Simultaneous Orbit Iteration Image Generation Method. Computes
 *      rectangular regions by tracking the orbits of only a few key points.
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

/* compute the value of the interpolation polynomial at (x,y) */
#define GET_REAL(x,y) \
interpolate(cim1,midi,cim2,\
	    interpolate(cre1,midr,cre2,zre1,zre5,zre2,x),\
	    interpolate(cre1,midr,cre2,zre6,zre9,zre7,x),\
	    interpolate(cre1,midr,cre2,zre3,zre8,zre4,x),y)
#define GET_IMAG(x,y) \
interpolate(cre1,midr,cre2,\
	    interpolate(cim1,midi,cim2,zim1,zim6,zim3,y),\
	    interpolate(cim1,midi,cim2,zim5,zim9,zim8,y),\
	    interpolate(cim1,midi,cim2,zim2,zim7,zim4,y),x)
     
/* compute the value of the interpolation polynomial at (x,y)
   from saved values before interpolation failed to stay within tolerance */
#define GET_SAVED_REAL(x,y) \
interpolate(cim1,midi,cim2,\
	    interpolate(cre1,midr,cre2,sr1,sr5,sr2,x),\
	    interpolate(cre1,midr,cre2,sr6,sr9,sr7,x),\
	    interpolate(cre1,midr,cre2,sr3,sr8,sr4,x),y)
#define GET_SAVED_IMAG(x,y) \
interpolate(cre1,midr,cre2,\
	    interpolate(cim1,midi,cim2,si1,si6,si3,y),\
	    interpolate(cim1,midi,cim2,si5,si9,si8,y),\
	    interpolate(cim1,midi,cim2,si2,si7,si4,y),x)
     
/* compute the value of te interpolation polynomial at (x,y)
   during scanning. Here, key values do not change, so we can precompute
   coefficients in one direction and simply evaluate the polynomial
   during scanning. */
#define GET_SCAN_REAL(x,y) \
interpolate(cim1,midi,cim2,\
	    EVALUATE(cre1,midr,br10,br11,br12,x),\
	    EVALUATE(cre1,midr,br20,br21,br22,x),\
	    EVALUATE(cre1,midr,br30,br31,br32,x),y)
#define GET_SCAN_IMAG(x,y) \
interpolate(cre1,midr,cre2,\
	    EVALUATE(cim1,midi,bi10,bi11,bi12,y),\
	    EVALUATE(cim1,midi,bi20,bi21,bi22,y),\
	    EVALUATE(cim1,midi,bi30,bi31,bi32,y),x)

/* compute coefficients of Newton polynomial (b0,..,b2) from
   (x0,w0),..,(x2,w2). */
#define INTERPOLATE(x0,x1,x2,w0,w1,w2,b0,b1,b2) \
b0=w0;\
b1=(w1-w0)/(double)(x1-x0);\
b2=((w2-w1)/(double)(x2-x1)-b1)/(x2-x0)

/* evaluate Newton polynomial given by (x0,b0),(x1,b1) at x:=t */     
#define EVALUATE(x0,x1,b0,b1,b2,t) \
((b2*(t-x1)+b1)*(t-x0)+b0)

/* saved yoffset */
static int base;
static double twidth;
int minprogress;
double tolerance;
     
/* Newton Interpolation.
   It computes the value of the interpolation polynomial given by
   (x0,w0)..(x2,w2) at x:=t

   P2(x)=b0+b1*(x-x0)+b2*(x-x0)*(x-x1)
        =b0+(b1+b2*(x-x1))*(x-x0)

   b0=w0
   b1=(w1-w0)/(x1-x0)
   b2=((w2-w1)/(x2-x1)-(w1-w0)/(x1-x0))/(x2-x0)
   */
static inline double interpolate(double x0, double x1, double x2,
				 double w0, double w1, double w2,
				 double t)
{
  register double b0=w0,b1=w1,b2=w2,b;

  b=(b1-b0)/(x1-x0);
  return (double)((((b2-b1)/(x2-x1)-b)/(x2-x0))*(t-x1)+b)*(t-x0)+b0;
}

/* SOI - Perform simultaneous orbit iteration for a given rectangle

   Input: cre1..cim2 : values defining the four corners of the rectangle
          x1..y2     : corresponding pixel values
	  zre1..zim9 : intermediate iterated values of the key points (key values)
	  
	  (cre1,cim1)               (cre2,cim1)
	  (zre1,zim1)  (zre5,zim5)  (zre2,zim2)
	       +------------+------------+
	       |            |            |
	       |            |            |
	  (zre6,zim6)  (zre9,zim9)  (zre7,zim7)
	       |            |            |
	       |            |            |
	       +------------+------------+
	  (zre3,zim3)  (zre8,zim8)  (zre4,zim4)
	  (cre1,cim2)               (cre2,cim2)

	  i       : current number of iterations
	  */
   
void SOI(double cre1, double cre2, double cim1, double cim2,
	 int x1, int x2, int y1, int y2,
	 double zre1, double zim1,
	 double zre2, double zim2,
	 double zre3, double zim3,
	 double zre4, double zim4,
	 double zre5, double zim5,
	 double zre6, double zim6,
	 double zre7, double zim7,
	 double zre8, double zim8,
	 double zre9, double zim9,
	 int i)
{
  /* center of rectangle */
  double midr=(cre1+cre2)/2,midi=(cim1+cim2)/2;
  /* used in scanning */
  int x,y,z,color,savecolor,savex,helpcolor;
  double re,im,restep,imstep,interstep,helpre;
  double zre,zim;
  /* interpolation coefficients */
  double br10,br11,br12,br20,br21,br22,br30,br31,br32;
  double bi10,bi11,bi12,bi20,bi21,bi22,bi30,bi31,bi32;
  /* ratio of interpolated test point to iterated one */
  double l1,l2;
  /* squares of key values */
  double rq1,iq1;
  double rq2,iq2;
  double rq3,iq3;
  double rq4,iq4;
  double rq5,iq5;
  double rq6,iq6;
  double rq7,iq7;
  double rq8,iq8;
  double rq9,iq9;
  /* saved values of key values */
  double sr1,si1,sr2,si2,sr3,si3,sr4,si4;
  double sr5,si5,sr6,si6,sr7,si7,sr8,si8,sr9,si9;
  /* key values for subsequent rectangles */
  double re10,re11,re12,re13,re14,re15,re16,re17,re18,re19,re20,re21;
  double im10,im11,im12,im13,im14,im15,im16,im17,im18,im19,im20,im21;
  double re91,re92,re93,re94,im91,im92,im93,im94;
  /* test points */
  double cr1,cr2;
  double ci1,ci2;
  double tzr1,tzi1,tzr2,tzi2,tzr3,tzi3,tzr4,tzi4;
  double trq1,tiq1,trq2,tiq2,trq3,tiq3,trq4,tiq4;
  /* number of iterations before SOI iteration cycle */
  int before;

#ifndef MULTITHREADING
  FractCheckEvent();
#endif
  if(abbruch) return;
  
  if(i>maxiter)
    {
      RasterFillbox(raster,x1,y1,x2-x1,y2-y1,inside);
      if(symmetric)
	RasterFillbox(raster,x1,symy-(y2-base)+1,x2-x1,y2-y1,inside);
      return;
    }
  
  if(y2-y1<=goscanat)
    {
      /* finish up the image by scanning the rectangle */
    scan:
      INTERPOLATE(cre1,midr,cre2,zre1,zre5,zre2,br10,br11,br12);
      INTERPOLATE(cre1,midr,cre2,zre6,zre9,zre7,br20,br21,br22);
      INTERPOLATE(cre1,midr,cre2,zre3,zre8,zre4,br30,br31,br32);
      
      INTERPOLATE(cim1,midi,cim2,zim1,zim6,zim3,bi10,bi11,bi12);
      INTERPOLATE(cim1,midi,cim2,zim5,zim9,zim8,bi20,bi21,bi22);
      INTERPOLATE(cim1,midi,cim2,zim2,zim7,zim4,bi30,bi31,bi32);
      
      restep=(cre2-cre1)/(x2-x1);
      imstep=(cim2-cim1)/(y2-y1);
      interstep=interleave*restep;
      
      for(y=y1, im=cim1; y<y2; y++, im+=imstep)
	{
#ifndef MULTITHREADING
	  FractCheckEvent();
#endif
	  if(abbruch) return;
	  
	  zre=GET_SCAN_REAL(cre1,im);
	  zim=GET_SCAN_IMAG(cre1,im);
	  savecolor=FractColor(FractIterate(cre1,im,zre,zim,i));
	  
	  savex=x1;
	  
	  for(x=x1+interleave, re=cre1+interstep; x<x2;
	      x+=interleave, re+=interstep)
	    {
	      zre=GET_SCAN_REAL(re,im);
	      zim=GET_SCAN_IMAG(re,im);

	      color=FractColor(FractIterate(re,im,zre,zim,i));
	      if(color==savecolor)
		continue;
	      
	      for (z=x-1, helpre=re-restep; z>x-interleave; z--,helpre-=restep)
		{
		  zre=GET_SCAN_REAL(helpre,im);
		  zim=GET_SCAN_IMAG(helpre,im);
		  helpcolor=FractColor(FractIterate(helpre,im,zre,zim,i));
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
	  
	  for (z=x2-1, helpre=cre2-restep; z>savex; z--,helpre-=restep)
	    {
	      zre=GET_SCAN_REAL(helpre,im);
	      zim=GET_SCAN_IMAG(helpre,im);
	      helpcolor=FractColor(FractIterate(helpre,im,zre,zim,i));
	      
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

  rq1=zre1*zre1; iq1=zim1*zim1;
  rq2=zre2*zre2; iq2=zim2*zim2;
  rq3=zre3*zre3; iq3=zim3*zim3;
  rq4=zre4*zre4; iq4=zim4*zim4;
  rq5=zre5*zre5; iq5=zim5*zim5;
  rq6=zre6*zre6; iq6=zim6*zim6;
  rq7=zre7*zre7; iq7=zim7*zim7;
  rq8=zre8*zre8; iq8=zim8*zim8;
  rq9=zre9*zre9; iq9=zim9*zim9;

  cr1=0.75*cre1+0.25*cre2; cr2=0.25*cre1+0.75*cre2;
  ci1=0.75*cim1+0.25*cim2; ci2=0.25*cim1+0.75*cim2;

  tzr1=GET_REAL(cr1,ci1);
  tzi1=GET_IMAG(cr1,ci1);

  tzr2=GET_REAL(cr2,ci1);
  tzi2=GET_IMAG(cr2,ci1);

  tzr3=GET_REAL(cr1,ci2);
  tzi3=GET_IMAG(cr1,ci2);

  tzr4=GET_REAL(cr2,ci2);
  tzi4=GET_IMAG(cr2,ci2);

  trq1=tzr1*tzr1;
  tiq1=tzi1*tzi1;

  trq2=tzr2*tzr2;
  tiq2=tzi2*tzi2;

  trq3=tzr3*tzr3;
  tiq3=tzi3*tzi3;

  trq4=tzr4*tzr4;
  tiq4=tzi4*tzi4;
  
  before=i;
  
  do
    {
      sr1=zre1; si1=zim1;
      sr2=zre2; si2=zim2;
      sr3=zre3; si3=zim3;
      sr4=zre4; si4=zim4;
      sr5=zre5; si5=zim5;
      sr6=zre6; si6=zim6;
      sr7=zre7; si7=zim7;
      sr8=zre8; si8=zim8;
      sr9=zre9; si9=zim9;

      /* iterate key values */
      zim1=(zim1+zim1)*zre1+cim1;
      zre1=rq1-iq1+cre1;
      rq1=zre1*zre1;
      iq1=zim1*zim1;
      
      zim2=(zim2+zim2)*zre2+cim1;
      zre2=rq2-iq2+cre2;
      rq2=zre2*zre2;
      iq2=zim2*zim2;
      
      zim3=(zim3+zim3)*zre3+cim2;
      zre3=rq3-iq3+cre1;
      rq3=zre3*zre3;
      iq3=zim3*zim3;
      
      zim4=(zim4+zim4)*zre4+cim2;
      zre4=rq4-iq4+cre2;
      rq4=zre4*zre4;
      iq4=zim4*zim4;

      zim5=(zim5+zim5)*zre5+cim1;
      zre5=rq5-iq5+midr;
      rq5=zre5*zre5;
      iq5=zim5*zim5;
      
      zim6=(zim6+zim6)*zre6+midi;
      zre6=rq6-iq6+cre1;
      rq6=zre6*zre6;
      iq6=zim6*zim6;
      
      zim7=(zim7+zim7)*zre7+midi;
      zre7=rq7-iq7+cre2;
      rq7=zre7*zre7;
      iq7=zim7*zim7;
      
      zim8=(zim8+zim8)*zre8+cim2;
      zre8=rq8-iq8+midr;
      rq8=zre8*zre8;
      iq8=zim8*zim8;
      
      zim9=(zim9+zim9)*zre9+midi;
      zre9=rq9-iq9+midr;
      rq9=zre9*zre9;
      iq9=zim9*zim9;

      /* iterate test point */
      tzi1=(tzi1+tzi1)*tzr1+ci1;
      tzr1=trq1-tiq1+cr1;
      trq1=tzr1*tzr1;
      tiq1=tzi1*tzi1;

      tzi2=(tzi2+tzi2)*tzr2+ci1;
      tzr2=trq2-tiq2+cr2;
      trq2=tzr2*tzr2;
      tiq2=tzi2*tzi2;

      tzi3=(tzi3+tzi3)*tzr3+ci2;
      tzr3=trq3-tiq3+cr1;
      trq3=tzr3*tzr3;
      tiq3=tzi3*tzi3;

      tzi4=(tzi4+tzi4)*tzr4+ci2;
      tzr4=trq4-tiq4+cr2;
      trq4=tzr4*tzr4;
      tiq4=tzi4*tzi4;

      i++;

      /* if one of the iterated values bails out, subdivide */
      if((rq1+iq1)>bailout||
	 (rq2+iq2)>bailout||
	 (rq3+iq3)>bailout||
	 (rq4+iq4)>bailout||
	 (rq5+iq5)>bailout||
	 (rq6+iq6)>bailout||
	 (rq7+iq7)>bailout||
	 (rq8+iq8)>bailout||
	 (rq9+iq9)>bailout||
	 (trq1+tiq1)>bailout||
	 (trq2+tiq2)>bailout||
	 (trq3+tiq3)>bailout||
	 (trq4+tiq4)>bailout)
	break;

      /* if maximum number of iterations is reached, the whole rectangle
	 can be assumed part of M. This is of course best case behavior
	 of SOI, we seldomly get there */
      if(i>maxiter)
	{
	  RasterFillbox(raster,x1,y1,x2-x1,y2-y1,inside);
	  if(symmetric)
	    RasterFillbox(raster,x1,symy-(y2-base)+1,x2-x1,y2-y1,inside);
	  return;
	}

      /* now for all test points, check whether they exceed the
	 allowed tolerance. if so, subdivide */
      l1=GET_REAL(cr1,ci1);
      l1=(tzr1==0.0)?
	(l1==0.0)?1.0:1000.0:
	l1/tzr1;
      if(fabs(1.0-l1)>twidth)
	break;
      
      l2=GET_IMAG(cr1,ci1);
      l2=(tzi1==0.0)?
	(l2==0.0)?1.0:1000.0:
	l2/tzi1;
      if(fabs(1.0-l2)>twidth)
	break;
      
      l1=GET_REAL(cr2,ci1);
      l1=(tzr2==0.0)?
	(l1==0.0)?1.0:1000.0:
	l1/tzr2;
      if(fabs(1.0-l1)>twidth)
	break;

      l2=GET_IMAG(cr2,ci1);
      l2=(tzi2==0.0)?
	(l2==0.0)?1.0:1000.0:
	l2/tzi2;
      if(fabs(1.0-l2)>twidth)
	break;

      l1=GET_REAL(cr1,ci2);
      l1=(tzr3==0.0)?
	(l1==0.0)?1.0:1000.0:
	l1/tzr3;
      if(fabs(1.0-l1)>twidth)
	break;
      
      l2=GET_IMAG(cr1,ci2);
      l2=(tzi3==0.0)?
	(l2==0.0)?1.0:1000.0:
	l2/tzi3;
      if(fabs(1.0-l2)>twidth)
	break;
      
      l1=GET_REAL(cr2,ci2);
      l1=(tzr4==0.0)?
	(l1==0.0)?1.0:1000.0:
	l1/tzr4;
      if(fabs(1.0-l1)>twidth)
	break;

      l2=GET_IMAG(cr2,ci2);
      l2=(tzi4==0.0)?
	(l2==0.0)?1.0:1000.0:
	l2/tzi4;
      if(fabs(1.0-l2)>twidth)
	break;
    }
  while(1);

  i--;

  /* this is a little heuristic I tried to improve performance. */
  if(i-before<minprogress)
    {
      zre1=sr1; zim1=si1;
      zre2=sr2; zim2=si2;
      zre3=sr3; zim3=si3;
      zre4=sr4; zim4=si4;
      zre5=sr5; zim5=si5;
      zre6=sr6; zim6=si6;
      zre7=sr7; zim7=si7;
      zre8=sr8; zim8=si8;
      zre9=sr9; zim9=si9;
      goto scan;
    }

  /* compute key values for subsequent rectangles */
  
  re10=interpolate(cre1,midr,cre2,sr1,sr5,sr2,cr1);
  im10=interpolate(cre1,midr,cre2,si1,si5,si2,cr1);

  re11=interpolate(cre1,midr,cre2,sr1,sr5,sr2,cr2);
  im11=interpolate(cre1,midr,cre2,si1,si5,si2,cr2);

  re20=interpolate(cre1,midr,cre2,sr3,sr8,sr4,cr1);
  im20=interpolate(cre1,midr,cre2,si3,si8,si4,cr1);

  re21=interpolate(cre1,midr,cre2,sr3,sr8,sr4,cr2);
  im21=interpolate(cre1,midr,cre2,si3,si8,si4,cr2);

  re15=interpolate(cre1,midr,cre2,sr6,sr9,sr7,cr1);
  im15=interpolate(cre1,midr,cre2,si6,si9,si7,cr1);

  re16=interpolate(cre1,midr,cre2,sr6,sr9,sr7,cr2);
  im16=interpolate(cre1,midr,cre2,si6,si9,si7,cr2);
  
  re12=interpolate(cim1,midi,cim2,sr1,sr6,sr3,ci1);
  im12=interpolate(cim1,midi,cim2,si1,si6,si3,ci1);

  re14=interpolate(cim1,midi,cim2,sr2,sr7,sr4,ci1);
  im14=interpolate(cim1,midi,cim2,si2,si7,si4,ci1);

  re17=interpolate(cim1,midi,cim2,sr1,sr6,sr3,ci2);
  im17=interpolate(cim1,midi,cim2,si1,si6,si3,ci2);

  re19=interpolate(cim1,midi,cim2,sr2,sr7,sr4,ci2);
  im19=interpolate(cim1,midi,cim2,si2,si7,si4,ci2);

  re13=interpolate(cim1,midi,cim2,sr5,sr9,sr8,ci1);
  im13=interpolate(cim1,midi,cim2,si5,si9,si8,ci1);

  re18=interpolate(cim1,midi,cim2,sr5,sr9,sr8,ci2);
  im18=interpolate(cim1,midi,cim2,si5,si9,si8,ci2);

  re91=GET_SAVED_REAL(cr1,ci1);
  re92=GET_SAVED_REAL(cr2,ci1);
  re93=GET_SAVED_REAL(cr1,ci2);
  re94=GET_SAVED_REAL(cr2,ci2);

  im91=GET_SAVED_IMAG(cr1,ci1);
  im92=GET_SAVED_IMAG(cr2,ci1);
  im93=GET_SAVED_IMAG(cr1,ci2);
  im94=GET_SAVED_IMAG(cr2,ci2);
  
  SOI(cre1,midr,cim1,midi,
	  x1,(x1+x2)>>1,y1,(y1+y2)>>1,
	  sr1,si1,
	  sr5,si5,
	  sr6,si6,
	  sr9,si9,
	  re10,im10,
	  re12,im12,
	  re13,im13,
	  re15,im15,
	  re91,im91,
	  i);
  SOI(midr,cre2,cim1,midi,
	  (x1+x2)>>1,x2,y1,(y1+y2)>>1,
	  sr5,si5,
	  sr2,si2,
	  sr9,si9,
	  sr7,si7,
	  re11,im11,
	  re13,im13,
	  re14,im14,
	  re16,im16,
	  re92,im92,
	  i);
  SOI(cre1,midr,midi,cim2,
	  x1,(x1+x2)>>1,(y1+y2)>>1,y2,
	  sr6,si6,
	  sr9,si9,
	  sr3,si3,
	  sr8,si8,
	  re15,im15,
	  re17,im17,
	  re18,im18,
	  re20,im20,
	  re93,im93,
	  i);
  SOI(midr,cre2,midi,cim2,
	  (x1+x2)>>1,x2,(y1+y2)>>1,y2,
	  sr9,si9,
	  sr7,si7,
	  sr8,si8,
	  sr4,si4,
	  re16,im16,
	  re18,im18,
	  re19,im19,
	  re21,im21,
	  re94,im94,
	  i);
}

/* entry function for SOI */
void SOICompute(double minr, double mini, double maxr, double maxi,
		int xoffset, int yoffset, int maxx, int maxy)
{
  base=yoffset;
  twidth=tolerance/raster->width;
  
  SOI(minr,maxr,mini,maxi,
      xoffset,maxx+1,yoffset,maxy+1,
      minr,mini,
      maxr,mini,
      minr,maxi,
      maxr,maxi,
      (maxr+minr)/2,mini,
      minr,(mini+maxi)/2,
      maxr,(mini+maxi)/2,
      (maxr+minr)/2,maxi,
      (minr+maxr)/2,(mini+maxi)/2,
      0);
}
