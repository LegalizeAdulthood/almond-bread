/* 
 * raster.c --
 *
 *      This module implements the raster widget. It is basically a stripped
 *      down canvas widget with rudimentary Colormap management. Most of the
 *      code was snitched from the tkSquare.c demo that comes with Tk.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

/*
 * Copyright (c) 1991-1993 The Regents of the University of California.
 * All rights reserved.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 * 
 * IN NO EVENT SHALL THE UNIVERSITY OF CALIFORNIA BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT
 * OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE UNIVERSITY OF
 * CALIFORNIA HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE UNIVERSITY OF CALIFORNIA SPECIFICALLY DISCLAIMS ANY WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#include "conf.h"
#if STDC_HEADERS
#include <stdlib.h>
#include <string.h>
#else
#include "stdlib.h"
#include "string.h"
#endif
#include <math.h>
#include <tcl.h>
#include <tk.h>
#include <X11/Xutil.h>
#include <X11/Xcms.h>
#include <X11/Xmd.h>
#include "raster.h"
#include "gif.h"

/* Information used for argv parsing. */

Tk_ConfigSpec configSpecs[] =
{
  /* default width is 320 pixels */
  {TK_CONFIG_PIXELS, "-width", "width", "Width",
     "320", Tk_Offset (Raster, width), 0, (Tk_CustomOption *) NULL},
  
  /* default height is 200 pixels */
  {TK_CONFIG_PIXELS, "-height", "height", "Height",
     "200", Tk_Offset (Raster, height), 0, (Tk_CustomOption *) NULL},

  /* no own colormap */
  {TK_CONFIG_BOOLEAN, "-owncmap", "owncmap", "Owncmap",
     "no", Tk_Offset (Raster, owncmap), 0, (Tk_CustomOption *) NULL},
  
  {TK_CONFIG_END, (char *) NULL, (char *) NULL, (char *) NULL,
     (char *) NULL, 0, 0}
};
     
Raster *raster=NULL;

/*
 * --------------------------------------------------------------
 * 
 * RasterCmd --
 * 
 * This procedure is invoked to process the "raster" Tcl command.  It creates a
 * new "raster" widget.
 * 
 * Results: A standard Tcl result.
 * 
 * Side effects: A new widget is created and configured.
 * 
 * --------------------------------------------------------------
 */

int
  RasterCmd (clientData, interp, argc, argv)
ClientData clientData;	/* Main window associated with interpreter. */
Tcl_Interp *interp;	/* Current interpreter. */
int argc;			/* Number of arguments. */
char **argv;		/* Argument strings. */
{
  Tk_Window main = (Tk_Window) clientData;
  Raster *rasterPtr;
  Tk_Window tkwin;
  
  if (argc < 2)
    {
      Tcl_AppendResult (interp, "wrong # args:  should be \"",
			argv[0], " pathName ?options?\"", (char *) NULL);
      return TCL_ERROR;
    }
  
  tkwin = Tk_CreateWindowFromPath (interp, main, argv[1], (char *) NULL);
  if (tkwin == NULL)
    return TCL_ERROR;
  Tk_SetClass (tkwin, "Raster");
  
  /* Allocate and initialize the widget record. */
  
  rasterPtr = (Raster *) malloc (sizeof (Raster));
  rasterPtr->tkwin = tkwin;
  rasterPtr->display = Tk_Display (tkwin);
  rasterPtr->interp = interp;
  rasterPtr->width = 0;
  rasterPtr->height = 0;
  rasterPtr->x1 = 1;
  rasterPtr->y1 = 0;
  rasterPtr->x2 = 0;
  rasterPtr->y2 = 0;
  rasterPtr->image=NULL;
  rasterPtr->owncmap=0;
  rasterPtr->colors = 0;
  rasterPtr->depth = 0;
  rasterPtr->exposecount=0;
  rasterPtr->window = None;
  rasterPtr->pmap = None;
  rasterPtr->gc = None;
  rasterPtr->copygc = None;
  rasterPtr->xorgc = None;
  rasterPtr->cmap = None;
  rasterPtr->updatePending = 0;

  Tk_CreateEventHandler (rasterPtr->tkwin, ExposureMask | StructureNotifyMask,
			 RasterEventProc, (ClientData) rasterPtr);
  Tcl_CreateCommand (interp, Tk_PathName (rasterPtr->tkwin), RasterWidgetCmd,
		     (ClientData) rasterPtr, (void (*)()) NULL);
  if (RasterConfigure (interp, rasterPtr, argc - 2, argv + 2, 0) != TCL_OK)
    {
      Tk_DestroyWindow (rasterPtr->tkwin);
      return TCL_ERROR;
    }

  if(raster==NULL)
    raster = rasterPtr;
  interp->result = Tk_PathName (rasterPtr->tkwin);
  return TCL_OK;
}

/*
 * --------------------------------------------------------------
 * 
 * RasterWidgetCmd --
 * 
 * This procedure is invoked to process the Tcl command that corresponds to a
 * widget managed by this module. See the user documentation for details on
 * what it does.
 * 
 * Results: A standard Tcl result.
 * 
 * Side effects: See the user documentation.
 * 
 * --------------------------------------------------------------
 */

int
  RasterWidgetCmd (clientData, interp, argc, argv)
ClientData clientData;	/* Information about raster widget. */
Tcl_Interp *interp;	/* Current interpreter. */
int argc;			/* Number of arguments. */
char **argv;		/* Argument strings. */
{
  Raster *rasterPtr = (Raster *) clientData;
  int result = TCL_OK;
  int length;
  char c;
  
  if (argc < 2)
    {
      Tcl_AppendResult (interp, "wrong # args: should be \"",
			argv[0], " option ?arg arg ...?\"", (char *) NULL);
      return TCL_ERROR;
    }
  Tk_Preserve ((ClientData) rasterPtr);
  c = argv[1][0];
  length = strlen (argv[1]);
  if ((c == 'c') && (strncmp (argv[1], "configure", length) == 0))
    {
      if (argc == 2)
	result = Tk_ConfigureInfo (interp, rasterPtr->tkwin, configSpecs,
				   (char *) rasterPtr, (char *) NULL, 0);
      else if (argc == 3)
	result = Tk_ConfigureInfo (interp, rasterPtr->tkwin, configSpecs,
				   (char *) rasterPtr, argv[2], 0);
      else
	result = RasterConfigure (interp, rasterPtr, argc - 2, argv + 2,
				 TK_CONFIG_ARGV_ONLY);
    }
  else if ((c == 'z') && (strncmp (argv[1], "zoombox", length) == 0))
    {
      int x1, y1, x2, y2;
      
      if (argc != 2 && argc != 6)
	{
	  Tcl_AppendResult (interp, "wrong # args: should be \"",
			    argv[0], " zoombox ?x1 y1 x2 y2?\"", (char *) NULL);
	  goto error;
	}
      if (argc == 6)
	{
	  if ((Tk_GetPixels (interp, rasterPtr->tkwin, argv[2],
			     &x1) != TCL_OK) ||
	      (Tk_GetPixels (interp, rasterPtr->tkwin, argv[3],
			     &y1) != TCL_OK) ||
	      (Tk_GetPixels (interp, rasterPtr->tkwin, argv[4],
			     &x2) != TCL_OK) ||
	      (Tk_GetPixels (interp, rasterPtr->tkwin, argv[5],
			     &y2) != TCL_OK))
	    goto error;
	  if (x1 > x2 || y1 > y2)
	    {
	      Tcl_AppendResult (interp, "parameter out of range", NULL);
	      goto error;
	    }
	  RasterZoombox (rasterPtr, x1, y1, x2, y2);
	}
      sprintf (interp->result, "%d %d %d %d", rasterPtr->x1, rasterPtr->y1,
	       rasterPtr->x2, rasterPtr->y2);
    }
  else if ((c == 'c') && (strncmp (argv[1], "colormap", length) == 0))
    {
      int c, r, g, b;
      char dummy[256];

      if(rasterPtr==raster)
	{
	  if (argc != 3)
	    {
	      Tcl_AppendResult (interp, "wrong # args: should be \"",
				argv[0], " colormap map\"", (char *) NULL);
	      goto error;
	    }
	  if (argc == 3)
	    {
	      char *s;
	      
	      for(c=0; c<256; c++)
		{
		  sprintf(dummy,"%s(%d,red)",argv[2],c);
		  if((s=Tcl_GetVar(interp,dummy,TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG))
		     ==NULL)
		    goto error;
		  if(Tcl_GetInt(interp, s, &r) != TCL_OK)
		    goto error;
		  rasterPtr->origcolor[c].red=(r%65536);
		  sprintf(dummy,"%s(%d,green)",argv[2],c);
		  if((s=Tcl_GetVar(interp,dummy,TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG))
		     ==NULL)
		    goto error;
		  if(Tcl_GetInt(interp, s, &g) != TCL_OK)
		    goto error;
		  rasterPtr->origcolor[c].green=(g%65536);
		  sprintf(dummy,"%s(%d,blue)",argv[2],c);
		  if((s=Tcl_GetVar(interp,dummy,TCL_GLOBAL_ONLY|TCL_LEAVE_ERR_MSG))
		     ==NULL)
		    goto error;
		  if(Tcl_GetInt(interp, s, &b) != TCL_OK)
		    goto error;
		  rasterPtr->origcolor[c].blue=(b%65536);
		}
	      RasterFreeColors(rasterPtr);
	      RasterColormap(rasterPtr);
	      RasterRecolor(rasterPtr);
	    }
	  sprintf (interp->result, "%s", argv[2]);
	}
      else
	{
	  if (argc != 2)
	    {
	      Tcl_AppendResult (interp, "wrong # args: should be \"",
				argv[0], " colormap\"", (char *) NULL);
	      goto error;
	    }
	  if (argc==2)
	    {
	      int c;
	      Tk_Window tkwin=rasterPtr->tkwin;
	      
	      if(!rasterPtr->owncmap)
		{
		  Tk_SetWindowColormap(tkwin,DefaultColormap(rasterPtr->display,Tk_ScreenNumber(tkwin)));
		  XSync(Tk_Display(tkwin),False);
		  rasterPtr->cmap=DefaultColormap(rasterPtr->display,
						  Tk_ScreenNumber(tkwin));
		}
	      else
		{
		  rasterPtr->cmap=raster->cmap;
		  Tk_SetWindowColormap (tkwin, rasterPtr->cmap);
		  XSync(Tk_Display(tkwin),False);
		}
	      
	      for(c=0; c<256; c++)
		{
		  rasterPtr->origcolor[c]=raster->origcolor[c];
		  rasterPtr->xcolor[c]=raster->xcolor[c];
		}
	    }
	} 
    }
  else if ((c == 'l') && (strncmp (argv[1], "loadgif", length) == 0))
    {
      if (argc != 3)
	{
	  Tcl_AppendResult (interp, "wrong # args: should be \"",
			    argv[0], " loadgif filename\"", (char *) NULL);
	  goto error;
	}
      if (argc == 3)
	{
	  RasterLoadgif(rasterPtr,argv[2]);
	  if(giferror)
	    goto error;
	}
      sprintf (interp->result, "%s", argv[2]);
    }
  else if ((c == 's') && (strncmp (argv[1], "savegif", length) == 0))
    {
      if (argc != 3)
	{
	  Tcl_AppendResult (interp, "wrong # args: should be \"",
			    argv[0], " savegif filename\"", (char *) NULL);
	  goto error;
	}
      if (argc == 3)
	{
	  RasterSavegif(rasterPtr,argv[2]);
	  if(giferror)
	    goto error;
	}
      sprintf (interp->result, "%s", argv[2]);
    }
  else if ((c == 'p') && (strncmp (argv[1], "putpixel", length) == 0))
    {
      int c=0, x=0, y=0;
      
      if (argc != 5)
	{
	  Tcl_AppendResult (interp, "wrong # args: should be \"",
			    argv[0], " putpixel x y color\"", (char *) NULL);
	  goto error;
	}
      if (argc == 5)
	{
	  if ((Tcl_GetInt (interp, argv[2], &x) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[3], &y) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[4], &c) != TCL_OK))
	    goto error;
	  if (c < 0 || c >= rasterPtr->colors || x < 0 || x >= rasterPtr->width ||
	      y < 0 || y >= rasterPtr->height)
	    {
	      Tcl_AppendResult (interp, "parameter out of range", NULL);
	      goto error;
	    }
	  RasterPutpixel (rasterPtr, x, y, c);
	}
      sprintf (interp->result, "%d %d %d", x, y, c);
    }
  else if ((c == 'p') && (strncmp (argv[1], "putline", length) == 0))
    {
      int c, x1, x2, y;
      
      if (argc != 6)
	{
	  Tcl_AppendResult (interp, "wrong # args: should be \"",
			    argv[0], " putline x1 y x2 color\"", (char *) NULL);
	  goto error;
	}
      if (argc == 6)
	{
	  if ((Tcl_GetInt (interp, argv[2], &x1) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[3], &y) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[4], &x2) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[5], &c) != TCL_OK))
	    goto error;
	  if (c < 0 || c >= rasterPtr->colors ||
	      x1 < 0 || x1 >= rasterPtr->width ||
	      x2 < 0 || x2 >= rasterPtr->width ||
	      y < 0 || y >= rasterPtr->height)
	    {
	      Tcl_AppendResult (interp, "parameter out of range", NULL);
	      goto error;
	    }
	  RasterPutline (rasterPtr, x1, y, x2, c);
	}
      sprintf (interp->result, "%d %d %d %d", x1, y, x2, c);
    }
  else if ((c == 'f') && (strncmp (argv[1], "fillbox", length) == 0))
    {
      int c, x, y, w, h;
      
      if (argc != 7)
	{
	  Tcl_AppendResult (interp, "wrong # args: should be \"",
			    argv[0], " fillbox x y width height color\"",
			    (char *) NULL);
	  goto error;
	}
      if (argc == 7)
	{
	  if ((Tcl_GetInt (interp, argv[2], &x) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[3], &y) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[4], &w) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[5], &h) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[6], &c) != TCL_OK))
	    goto error;
	  if (c < 0 || c >= rasterPtr->colors ||
	      x < 0 || x >= rasterPtr->width ||
	      y < 0 || y >= rasterPtr->height ||
	      x + w < 0 || x + w > rasterPtr->width ||
	      y + h < 0 || y + h > rasterPtr->height)
	    {
	      Tcl_AppendResult (interp, "parameter out of range", NULL);
	      goto error;
	    }
	  RasterFillbox (rasterPtr, x, y, w, h, c);
	}
      sprintf (interp->result, "%d %d %d %d %d", x, y, w, h, c);
    }
  else if ((c == 'g') && (strncmp (argv[1], "getpixel", length) == 0))
    {
      int c=0, x, y;
      
      if (argc != 4)
	{
	  Tcl_AppendResult (interp, "wrong # args: should be \"",
			    argv[0], " getpixel x y\"",
			    (char *) NULL);
	  goto error;
	}
      if (argc == 4)
	{
	  if ((Tcl_GetInt (interp, argv[2], &x) != TCL_OK) ||
	      (Tcl_GetInt (interp, argv[3], &y) != TCL_OK))
	    goto error;
	  if (x < 0 || x >= rasterPtr->width ||
	      y < 0 || y >= rasterPtr->height)
	    {
	      Tcl_AppendResult (interp, "parameter out of range", NULL);
	      goto error;
	    }
	  c = RasterGetpixel (rasterPtr, x, y);
	}
      sprintf (interp->result, "%d", c);
    }
  else
    {
      Tcl_AppendResult (interp, "bad option \"", argv[1],
			"\":  must be configure, colormap, fillbox, getpixel, loadgif, putline, putpixel, savegif, or zoombox",
			(char *) NULL);
      goto error;
    }
  if (!rasterPtr->updatePending)
    {
      Tk_DoWhenIdle (RasterDisplay, (ClientData) rasterPtr);
      rasterPtr->updatePending = 1;
    }
  Tk_Release ((ClientData) rasterPtr);
  return result;
  
 error:
  Tk_Release ((ClientData) rasterPtr);
  return TCL_ERROR;
  
}

/*
 * ----------------------------------------------------------------------
 * 
 * RasterConfigure --
 * 
 * This procedure is called to process an argv/argc list in conjunction with the
 * Tk option database to configure (or reconfigure) a raster widget.
 * 
 * Results: The return value is a standard Tcl result.  If TCL_ERROR is
 * returned, then interp->result contains an error message.
 * 
 * Side effects: Configuration information gets set for rasterPtr; old resources
 * get free, if there were any.
 * 
 * ----------------------------------------------------------------------
 */

int
  RasterConfigure (interp, rasterPtr, argc, argv, flags)
Tcl_Interp *interp;	/* Used for error reporting. */
Raster *rasterPtr;		/* Information about widget. */
int argc;			/* Number of valid entries in argv. */
char **argv;		/* Arguments. */
int flags;			/* Flags to pass to Tk_ConfigureWidget. */
{
  Tk_Window tkwin = rasterPtr->tkwin;
  XGCValues gcValues;
  int owncmap=rasterPtr->owncmap;
  int width=rasterPtr->width,height=rasterPtr->height;
  
  if (Tk_ConfigureWidget (interp, tkwin, configSpecs,
			  argc, argv, (char *) rasterPtr, flags) != TCL_OK)
    return TCL_ERROR;
  
  /*
   * Set the background for the window and create a graphics context for use
   * during redisplay.
   */

  Tk_SetWindowBackground(tkwin,1);
  
  if (rasterPtr->gc == None)
    rasterPtr->gc = XCreateGC (Tk_Display(tkwin),
			       RootWindow(Tk_Display(tkwin),
					  Tk_ScreenNumber(tkwin)),
			       0, &gcValues);
  
  if (rasterPtr->copygc == None)
    {
      gcValues.function = GXcopy;
      gcValues.graphics_exposures = False;
      rasterPtr->copygc = XCreateGC (Tk_Display(tkwin),
				     RootWindow(Tk_Display(tkwin),
						Tk_ScreenNumber(tkwin)),
				     GCFunction | GCGraphicsExposures,
				     &gcValues);
    }
  
  if (rasterPtr->xorgc == None)
    {
      gcValues.function = GXinvert;
      gcValues.foreground = BlackPixel (Tk_Display (tkwin), 0);
      rasterPtr->xorgc = XCreateGC (Tk_Display(tkwin),
				    RootWindow(Tk_Display(tkwin),
					       Tk_ScreenNumber(tkwin)),
				    GCFunction | GCForeground,
				    &gcValues);
    }

  if(rasterPtr->image==NULL)
    rasterPtr->image=(unsigned char *)ckalloc(rasterPtr->width*
					      rasterPtr->height);
  else
    if(rasterPtr->width!=width||rasterPtr->height!=height)
      /* geometry has changed */
      {
	if(rasterPtr->image!=NULL)
	  ckfree(rasterPtr->image);
	rasterPtr->image=(unsigned char *)ckalloc(rasterPtr->width*
						 rasterPtr->height);
	memset(rasterPtr->image,0,rasterPtr->width*rasterPtr->height);
	if(rasterPtr->pmap!=None)
	  XFreePixmap(rasterPtr->display,rasterPtr->pmap);
	rasterPtr->pmap = XCreatePixmap (Tk_Display (tkwin),
					Tk_WindowId (tkwin),
					rasterPtr->width,
					rasterPtr->height, Tk_Depth (tkwin));
	XSetForeground (Tk_Display (tkwin), rasterPtr->gc, 1);
	XFillRectangle (Tk_Display (tkwin), rasterPtr->pmap, rasterPtr->gc, 0, 0,
			rasterPtr->width, rasterPtr->height);
      }
  
  if(rasterPtr->cmap!=None&&(owncmap!=rasterPtr->owncmap||
			     rasterPtr!=raster))
    /* this is not the first invocation and owncmap has changed
       or it's not the main raster widget */
    {
      int c;
      
      if(rasterPtr==raster)
	RasterFreeColors(rasterPtr);

      if(!rasterPtr->owncmap)
	{
	  Tk_SetWindowColormap(tkwin, DefaultColormap(rasterPtr->display,Tk_ScreenNumber(tkwin)));
	  XSync(Tk_Display(tkwin),False);
	  if(rasterPtr==raster)
	    XFreeColormap(rasterPtr->display,rasterPtr->cmap);
	  rasterPtr->cmap=DefaultColormap(rasterPtr->display,
					  Tk_ScreenNumber(tkwin));
	}
      else
	{
	  if(rasterPtr==raster)
	    rasterPtr->cmap = XCreateColormap (Tk_Display (tkwin),
					       Tk_WindowId (tkwin),
					       Tk_Visual (tkwin),
					       AllocNone);
	  else
	    rasterPtr->cmap=raster->cmap;
	  Tk_SetWindowColormap (tkwin, rasterPtr->cmap);
	  XSync(Tk_Display(tkwin),False);
	}

      if(rasterPtr==raster)
	{
	  RasterColormap(rasterPtr);
	  RasterRecolor(rasterPtr);
	}
      else
	{
	  for(c=0; c<256; c++)
	    {
	      rasterPtr->origcolor[c]=raster->origcolor[c];
	      rasterPtr->xcolor[c]=raster->xcolor[c];
	    }
	}
	  
    }
  
  /*
   * Register the desired geometry for the window.  Then arrange for the
   * window to be redisplayed.
   */
  
  Tk_GeometryRequest (tkwin, rasterPtr->width, rasterPtr->height);
  Tk_SetInternalBorder (tkwin, 0);
  if (!rasterPtr->updatePending)
    {
      Tk_DoWhenIdle (RasterDisplay, (ClientData) rasterPtr);
      rasterPtr->updatePending = 1;
    }
  return TCL_OK;
}

/*
 * --------------------------------------------------------------
 * 
 * RasterEventProc --
 * 
 * This procedure is invoked by the Tk dispatcher for various events on rasters.
 * 
 * Results: None.
 * 
 * Side effects: When the window gets deleted, internal structures get cleaned
 * up.  When it gets exposed, it is redisplayed.
 * 
 * --------------------------------------------------------------
 */

void
  RasterEventProc (clientData, eventPtr)
ClientData clientData;	/* Information about window. */
XEvent *eventPtr;		/* Information about event. */
{
  Raster *rasterPtr = (Raster *) clientData;
  
  if (eventPtr->type == Expose)
    {
      if (!rasterPtr->updatePending)
	{
	  rasterPtr->expose[rasterPtr->exposecount]=eventPtr->xexpose;
	  rasterPtr->exposecount=(rasterPtr->exposecount+1)%256;
	  Tk_DoWhenIdle (RasterDisplay, (ClientData) rasterPtr);
	}
    }
  else if (eventPtr->type == ConfigureNotify)
    {
      if (!rasterPtr->updatePending)
	{
	  Tk_DoWhenIdle (RasterDisplay, (ClientData) rasterPtr);
	  rasterPtr->updatePending = 1;
	}
    }
  else if (eventPtr->type == DestroyNotify)
    {
      Tcl_DeleteCommand (rasterPtr->interp, Tk_PathName (rasterPtr->tkwin));
      rasterPtr->tkwin = NULL;
      if (rasterPtr->updatePending)
	{
	  Tk_CancelIdleCall (RasterDisplay, (ClientData) rasterPtr);
	}
      Tk_EventuallyFree ((ClientData) rasterPtr, (Tk_FreeProc *)RasterDestroy);
    }
}

/*
 * --------------------------------------------------------------
 * 
 * RasterDisplay --
 * 
 * This procedure redraws the contents of a raster window. It is invoked as a
 * do-when-idle handler, so it only runs when there's nothing else for the
 * application to do.
 * 
 * Results: None.
 * 
 * Side effects: Information appears on the screen.
 * 
 * --------------------------------------------------------------
 */

void
  RasterDisplay (clientData)
ClientData clientData;	/* Information about window. */
{
  Raster *rasterPtr = (Raster *) clientData;
  Tk_Window tkwin = rasterPtr->tkwin;
  
  rasterPtr->updatePending = 0;
  if (!Tk_IsMapped (tkwin))
    return;
  
  if (rasterPtr->window == None)
    rasterPtr->window = Tk_WindowId (tkwin);
  
  if (rasterPtr->cmap == None)
    {
      int a,b,c;

      rasterPtr->depth = c = XDefaultDepthOfScreen (Tk_Screen (tkwin));
      rasterPtr->colors = (c>=8?256:(1<<c));
      
      if(!rasterPtr->owncmap)
	{
	  Tk_SetWindowColormap(tkwin, DefaultColormap(rasterPtr->display,Tk_ScreenNumber(tkwin)));
	  XSync(Tk_Display(tkwin),False);
	  rasterPtr->cmap=DefaultColormap(rasterPtr->display,
					  Tk_ScreenNumber(tkwin));
	}
      else
	{
	  if(rasterPtr==raster)
	    rasterPtr->cmap = XCreateColormap (Tk_Display (tkwin),
					       Tk_WindowId (tkwin),
					       Tk_Visual (tkwin),
					       AllocNone);
	  else
	    rasterPtr->cmap=raster->cmap;
	  Tk_SetWindowColormap (tkwin, rasterPtr->cmap);
	  XSync(Tk_Display(tkwin),False);
	}

      if(rasterPtr==raster)
	{
	  for(a=0; a<256; a+=32)
	    {
	      for(b=a, c=240<<8; b<a+16; b++, c-=12<<8)
		{
		  rasterPtr->origcolor[b].red=c;
		  rasterPtr->origcolor[b].green=c;
		  rasterPtr->origcolor[b].blue=c;
		}
	      for(b=a+16; b<a+32; b++, c+=12<<8)
		{
		  rasterPtr->origcolor[b].red=c;
		  rasterPtr->origcolor[b].green=c;
		  rasterPtr->origcolor[b].blue=c;
		}
	    }
	  rasterPtr->origcolor[0].red=0;
	  rasterPtr->origcolor[0].green=0;
	  rasterPtr->origcolor[0].blue=0;
	  
	  /*for(c=0; c<256; c++)
	    {
	      rasterPtr->origcolor[c].red=65535;
	      rasterPtr->origcolor[c].green=c<<8;
	      rasterPtr->origcolor[c].blue=32768;
	    }*/
	  RasterColormap(rasterPtr);
	}
      else
	{
	  for(c=0; c<256; c++)
	    {
	      rasterPtr->origcolor[c]=raster->origcolor[c];
	      rasterPtr->xcolor[c]=raster->xcolor[c];
	    }
	}
    }
  
  if (rasterPtr->pmap == None)
    {
      rasterPtr->pmap = XCreatePixmap (Tk_Display (tkwin), Tk_WindowId (tkwin),
				      rasterPtr->width,
				      rasterPtr->height, Tk_Depth (tkwin));
      XSetForeground (Tk_Display (tkwin), rasterPtr->gc, 1);
      XFillRectangle (Tk_Display (tkwin), rasterPtr->pmap, rasterPtr->gc, 0, 0,
		      rasterPtr->width, rasterPtr->height);
    }

  if(rasterPtr->exposecount!=0)
    {
      int c,x,y,w,h;
      
      c=(--rasterPtr->exposecount);
      x=rasterPtr->expose[c].x;
      y=rasterPtr->expose[c].y;
      w=rasterPtr->expose[c].width;
      h=rasterPtr->expose[c].height;

      XCopyArea (Tk_Display (tkwin), rasterPtr->pmap, Tk_WindowId (tkwin),
		 rasterPtr->copygc, x, y, w, h, x, y);
    }
  else
    XCopyArea (Tk_Display (tkwin), rasterPtr->pmap, Tk_WindowId (tkwin),
	       rasterPtr->copygc, 0, 0, Tk_Width (tkwin), Tk_Height (tkwin),
	       0, 0);
}

/*
 * ----------------------------------------------------------------------
 * 
 * RasterDestroy --
 * 
 * This procedure is invoked by Tk_EventuallyFree or Tk_Release to clean up the
 * internal structure of a raster at a safe time (when no-one is using it
 * anymore).
 * 
 * Results: None.
 * 
 * Side effects: Everything associated with the raster is freed up.
 * 
 * ----------------------------------------------------------------------
 */

void
  RasterDestroy (memPtr)
char *memPtr;	/* Info about raster widget. */
{
  Raster *rasterPtr = (Raster *) memPtr;
  
  Tk_FreeOptions (configSpecs, (char *) rasterPtr, rasterPtr->display, 0);
  if (rasterPtr->copygc != None)
    XFreeGC (rasterPtr->display, rasterPtr->copygc);
  if (rasterPtr->xorgc != None)
    XFreeGC (rasterPtr->display, rasterPtr->xorgc);
  if (rasterPtr->gc != None)
    XFreeGC (rasterPtr->display, rasterPtr->gc);
  if (rasterPtr->pmap != None)
    XFreePixmap (rasterPtr->display, rasterPtr->pmap);
  if (rasterPtr->cmap != None && rasterPtr==raster)
    XFreeColormap (rasterPtr->display, rasterPtr->cmap);
  if(rasterPtr->image!=NULL)
    ckfree(rasterPtr->image);
  
  free ((char *) rasterPtr);
}

void
  RasterZoombox (rasterPtr, x1, y1, x2, y2)
Raster *rasterPtr;
int x1,y1,x2,y2;
{
  Tk_Window tkwin = rasterPtr->tkwin;
  
  if (rasterPtr->x1 < rasterPtr->x2)
    /* delete zoombox only if there is one */
    {
      XDrawRectangle (rasterPtr->display, Tk_WindowId (tkwin),
		      rasterPtr->xorgc, rasterPtr->x1, rasterPtr->y1,
		      rasterPtr->x2 - rasterPtr->x1,
		      rasterPtr->y2 - rasterPtr->y1);
      XDrawRectangle (rasterPtr->display, rasterPtr->pmap,
		      rasterPtr->xorgc, rasterPtr->x1, rasterPtr->y1,
		      rasterPtr->x2 - rasterPtr->x1,
		      rasterPtr->y2 - rasterPtr->y1);
    }
  
  if (x1 != 0 || x2 != 0 || y1 != 0 || y2 != 0)
    {
      XDrawRectangle (rasterPtr->display, Tk_WindowId (rasterPtr->tkwin),
		      rasterPtr->xorgc, x1, y1, x2 - x1, y2 - y1);
      XDrawRectangle (rasterPtr->display, rasterPtr->pmap,
		      rasterPtr->xorgc, x1, y1, x2 - x1, y2 - y1);
    }
  
  rasterPtr->x1 = x1;
  rasterPtr->y1 = y1;
  rasterPtr->x2 = x2;
  rasterPtr->y2 = y2;
}

/* this is needed to remember all colors that were actually
   allocated so we know which colors to free later */
int alloc[256];

static inline double Distance(XColor x, XColor y)
{
  register double r=x.red-y.red,g=x.green-y.green,b=x.blue-y.blue;

  r*=0.3; g*=0.59; b*=0.11;

  return (double)sqrt(r*r+g*g+b*b);
}

void
  RasterRecolor(rasterPtr)
Raster *rasterPtr;
{
  register int x,w=rasterPtr->width,h=rasterPtr->height;
  register XImage *image;
  register CARD32 *imagedata,*p;
  CARD32 cmap[256];
  register unsigned char *i=rasterPtr->image,*pos,*e;
  CARD16 word=1;
  char *wp=(char*)&word;
  int byte_order=(wp[0]>0?LSBFirst:MSBFirst);

  /* When a colormap change has occured, this function is called.
     The whole image has to be repainted using the new pixel values.
     Policy: Allocate a 32 bits XImage, fill in the new values, and send
     the image to the server, thus letting it deal with byteorder and such.
     If you know of a safer and/or quicker way to do this, let me know.
     */
  imagedata=(CARD32 *)malloc((w*h)<<2);
  image=XCreateImage(rasterPtr->display,Tk_Visual(rasterPtr->tkwin),
		     32,ZPixmap,0,
		     (char*)imagedata,w,h,32,w<<2);
  image->byte_order=byte_order;
  
  for(x=0; x<256; x++)
    cmap[x]=(CARD32)rasterPtr->xcolor[x]->pixel;

  for(pos=i, p=imagedata, e=i+w*h; pos<e; pos++, p++)
    *p=cmap[*pos];
  
  image->depth=Tk_Depth(rasterPtr->tkwin);
  XPutImage(rasterPtr->display,rasterPtr->pmap,
	    rasterPtr->gc,image,0,0,0,0,rasterPtr->width,rasterPtr->height);
  
  XDestroyImage(image);
}

void
  RasterColormap(rasterPtr)
Raster *rasterPtr;
{
  int x,c,max;
  XColor color[256],*ret[256],dummy;
  double distance[256],maxdistance,d;
  int position[256];
  /* the following is solely needed for statistics */
  int phase[256],stats[3];
  static char msg[256];
  char dummy1[20],dummy2[20];
  
  memcpy(color,rasterPtr->origcolor,256*sizeof(XColor));

  for(c=0; c<256; c++)
    {
      position[c]=c;
      distance[c]=0;
    }
 
  for(c=1; c<256; c++)
    {
      for(x=c; x<256; x++)
        distance[x]+=Distance(color[x],color[c-1]);
      
      max=c;
      maxdistance=0;
      for(x=c; x<256; x++)
        if(distance[x]>maxdistance)
          {
            maxdistance=distance[x];
            max=x;
          }
      
      dummy=color[c];
      color[c]=color[max];
      color[max]=dummy;

      x=position[c];
      position[c]=position[max];
      position[max]=x;

      d=distance[c];
      distance[c]=distance[max];
      distance[max]=d;
    }

  /* phase 1 and 2 combined using Tk's color allocation facilities */
  for(c=0; c<256; c++)
    {
      if((ret[c]=Tk_GetColorByValue(rasterPtr->tkwin,&color[c]))!=NULL)
	{
	  /* extremely dirty, alloc'ing color again to see
	     if it is a "close" color or not */
	  dummy=color[c];
	  if(XAllocColor(rasterPtr->display,rasterPtr->cmap,&dummy))
	    {
	      XFreeColors(rasterPtr->display,rasterPtr->cmap,
			  &dummy.pixel,1,0);
	      phase[c]=0;
	    }
	  else
	    phase[c]=1;
	}
      else
	phase[c]=2;
    }
  
  /* phase 3 */
  for(c=0; c<256; c++)
    if(phase[c]==2)
      {
	maxdistance=113512;
	max=0;
	for(x=0; x<256; x++)
	  if(phase[x]<2)
	    {
	      if((d=Distance(*ret[x],color[c]))<maxdistance)
		{
		  maxdistance=d;
		  max=x;
		}
	    }

	ret[c]=ret[max];
      }

  /* map allocated colors to rasterPtr->xcolor */
  stats[0]=stats[1]=stats[2]=0;
  for(c=0; c<256; c++)
    {
      rasterPtr->xcolor[position[c]]=ret[c];
      alloc[position[c]]=(phase[c]<2);
      /* set global variable cmap */
      sprintf(dummy1,"%d,red",position[c]);
      sprintf(dummy2,"%d",ret[c]->red);
      Tcl_SetVar2(rasterPtr->interp,"cmap",dummy1,dummy2,TCL_GLOBAL_ONLY);
      sprintf(dummy1,"%d,green",position[c]);
      sprintf(dummy2,"%d",ret[c]->green);
      Tcl_SetVar2(rasterPtr->interp,"cmap",dummy1,dummy2,TCL_GLOBAL_ONLY);
      sprintf(dummy1,"%d,blue",position[c]);
      sprintf(dummy2,"%d",ret[c]->blue);
      Tcl_SetVar2(rasterPtr->interp,"cmap",dummy1,dummy2,TCL_GLOBAL_ONLY);
      stats[phase[c]]++;
    }
  sprintf(msg,"Got %d desired colors, %d close, %d mapped.", stats[0], stats[1], stats[2]);
  Tcl_SetVar(rasterPtr->interp, "status", msg, TCL_GLOBAL_ONLY);
  Tcl_GlobalEval(rasterPtr->interp, "update idletasks");
}

void
  RasterFreeColors(rasterPtr)
Raster *rasterPtr;
{
  int c;
  
  for(c=0; c<256; c++)
    if(alloc[c])
      /* color allocated in phase 1 or 2 */
      Tk_FreeColor(rasterPtr->xcolor[c]);
}
