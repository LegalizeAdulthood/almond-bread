/* 
 * raster.h --
 *
 *	Raster Widget Header File.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef RASTER_H
#define RASTER_H

#include "conf.h"
#if STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else
#include "stdlib.h"
#include "string.h"
#endif
#include <tcl.h>
#include <tk.h>

typedef struct
{
  Tk_Window tkwin;		/* Window that embodies the raster.  NULL
				 * means window has been deleted but widget
				 * record hasn't been cleaned up yet. */
  Display *display;		/* X's token for the window's display. */
  Tcl_Interp *interp;		/* Interpreter associated with widget. */
  
  int width, height;
  int x1, y1, x2, y2;		/* coordinates of the zoombox */
  int depth, colors;
  
  /*
   * Information used when displaying widget:
   */

  unsigned char *image;          
  int owncmap;                  /* true if own colormap is being used */
  XColor origcolor[256];        /* original color values that we try to
				   allocate */
  XColor *xcolor[256];          /* values actually allocated */
  XExposeEvent expose[256];     /* expose events containing exposed rectangles
				 */
  int exposecount;              /* number of expose events to be processed */
  Window window;		/* X Window handle of the widget's window */
  Pixmap pmap;		        /* Pixmap used for double-buffering */
  GC gc;			/* the "normal" gc used for drawing */
  GC copygc;			/* Graphics context for copying from
				 * off-screen pixmap onto screen. */
  GC xorgc;			/* GC used for the zoombox */
  Colormap cmap;
  int updatePending;		/* Non-zero means a call to RasterDisplay has
				 * already been scheduled && it's a whole
				 * screen redisplay 
				 */
}
Raster;

extern Tk_ConfigSpec configSpecs[];

extern int RasterConfigure _ANSI_ARGS_ ((Tcl_Interp * interp,
					Raster * rasterPtr, int argc,
					char **argv,
					int flags));
extern void RasterDestroy _ANSI_ARGS_ ((char *memPtr));
extern void RasterDisplay _ANSI_ARGS_ ((ClientData clientData));
extern void RasterEventProc _ANSI_ARGS_ ((ClientData clientData,
					 XEvent * eventPtr));
extern int RasterWidgetCmd _ANSI_ARGS_ ((ClientData clientData,
					Tcl_Interp *, int argc, char **argv));
extern int RasterCmd _ANSI_ARGS_ ((ClientData clientData, Tcl_Interp * interp,
				  int argc, char **argv));
extern void RasterZoombox _ANSI_ARGS_ ((Raster * rasterPtr,
				       int x1, int y1, int x2, int y2));

static inline void RasterPutpixel _ANSI_ARGS_ ((register Raster * rasterPtr,
					       register int x, register int y,
					       register int c))
{
  register GC gc = rasterPtr->gc;
  register Display *display = rasterPtr->display;
  
  XSetForeground (display, gc, rasterPtr->xcolor[c]->pixel);
  XDrawPoint (display, rasterPtr->window, gc, x, y);
  XDrawPoint (display, rasterPtr->pmap, gc, x, y);
  rasterPtr->image[rasterPtr->width*y+x]=c;
}

static inline void RasterPutline _ANSI_ARGS_ ((register Raster * rasterPtr,
					      register int x1, register int y,
					      register int x2,
					      register int c))
{
  register GC gc = rasterPtr->gc;
  register Display *display = rasterPtr->display;
  
  XSetForeground (display, gc, rasterPtr->xcolor[c]->pixel);
  XDrawLine (display, rasterPtr->window, gc, x1, y, x2, y);
  XDrawLine (display, rasterPtr->pmap, gc, x1, y, x2, y);
  memset(rasterPtr->image+y*rasterPtr->width+x1,c,x2-x1+1);
}

static inline void RasterFillbox _ANSI_ARGS_ ((register Raster * rasterPtr,
					      register int x, register int y,
					      register int w, register int h,
					      register int c))
{
  register int pos,line=rasterPtr->width,end=(y+h)*line;
  register GC gc = rasterPtr->gc;
  register Display *display = rasterPtr->display;
  
  XSetForeground (display, gc, rasterPtr->xcolor[c]->pixel);
  XFillRectangle (display, rasterPtr->window, gc, x, y, w, h);
  XFillRectangle (display, rasterPtr->pmap, gc, x, y, w, h);
  for(pos=y*line+x; pos<end; pos+=line)
    memset(rasterPtr->image+pos,c,w);
}

static inline int RasterGetpixel _ANSI_ARGS_ ((register Raster * rasterPtr,
					      register int x, register int y))
{
  return rasterPtr->image[y*rasterPtr->width+x];
}

extern void RasterColormap _ANSI_ARGS_ ((Raster *rasterPtr));
extern void RasterFreeColors _ANSI_ARGS_ ((Raster *rasterPtr));     
extern void RasterRecolor _ANSI_ARGS_ ((Raster *rasterPtr));
					     
extern Raster *raster;

#endif /* RASTER_H */
