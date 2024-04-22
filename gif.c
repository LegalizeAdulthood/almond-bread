/* 
 * gif.c --
 *
 *	Load and Save GIF(tm) files, this is not the de-/encoder though.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "conf.h"
#include <stdio.h>
#if STDC_HEADERS
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#else
#include "stdlib.h"
#include "string.h"
#endif
#ifdef HAVE_STRERROR
extern int errno;
#endif
#include <tcl.h>
#include <tk.h>
#include "raster.h"
#include "gif.h"
#include "fractint.h"

#define TRUE 1
#define FALSE 0

int giferror;
static char gif_err[256];

void gif_error(char *fmt, ...)
{
#ifdef HAVE_VPRINTF
  va_list args;
  
  va_start(args, fmt);
  vsprintf(gif_err, fmt, args);
  va_end(args);
#endif
  giferror=TRUE;
}

void gif_message(char *fmt, ...)
{
#ifdef HAVE_VPRINTF
  va_list args;
  static char msg[256];
  
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  Tcl_VarEval(raster->interp, "set status \"", msg, "\"", NULL);
  Tcl_GlobalEval(raster->interp, "update idletasks ; after 4000 { set status \"\" }");
  va_end(args);
#endif
}

int gif_w,gif_h;
XImage *gif_image;
CARD32 *gif_imagedata;

int gif_configure(int width, int height)
{
  static char a0[]="-width",a1[10],a2[]="-height",a3[10];
  static char *argv[]={a0,a1,a2,a3};
  Tk_Window topwin=raster->tkwin;
  CARD16 word=1;
  char *wp=(char*)&word;
  int byte_order=(wp[0]>0?LSBFirst:MSBFirst);

  sprintf(a1,"%d",width);
  sprintf(a3,"%d",height);
  if(RasterConfigure(raster->interp, raster, 4, argv,
		     TK_CONFIG_ARGV_ONLY)!=TCL_OK)
    return FALSE;

  /* the following had to be done, because no geometry
     manager would resize the toplevel window after it
     had been resized manually */
  while(!Tk_IsTopLevel(topwin))
    topwin=Tk_Parent(topwin);
  Tk_ResizeWindow(topwin,width,height);
  while(Tk_DoOneEvent(TK_ALL_EVENTS|TK_DONT_WAIT));

  gif_w=width; gif_h=height;
  gif_imagedata=(CARD32 *)malloc((width*height)<<2);
  gif_image=XCreateImage(raster->display,Tk_Visual(raster->tkwin),32,ZPixmap,0,
		     (char*)gif_imagedata,width,height,32,width<<2);
  gif_image->byte_order=byte_order;
  
  return TRUE;
}

void gif_setcolormap(unsigned char cmap[3][256])
{
  int c;
  
  for(c=0; c<256; c++)
    {
      raster->origcolor[c].red=cmap[0][c]<<8;
      raster->origcolor[c].green=cmap[1][c]<<8;
      raster->origcolor[c].blue=cmap[2][c]<<8;
    }
  RasterFreeColors(raster);
  RasterColormap(raster);
}

void
  RasterLoadgif(rasterPtr, filename)
Raster *rasterPtr;
char *filename;
{
  FILE *in;
  
  verbose = TRUE;
  showComment = TRUE;
  giferror=FALSE;
  
  if((in=fopen(filename,"r"))==NULL)
    {
      giferror=TRUE;
#ifdef HAVE_STRERROR
      sprintf(rasterPtr->interp->result,"%s: %s",filename,strerror(errno));
#else
      sprintf(rasterPtr->interp->result,"%s: error reading file",filename);
#endif
      return;
    }

  fi=NULL;
  ReadGIF(in, 1);

  gif_image->depth=Tk_Depth(rasterPtr->tkwin);
  XPutImage(rasterPtr->display,rasterPtr->pmap,
	    rasterPtr->gc,gif_image,0,0,0,0,rasterPtr->width,rasterPtr->height);
  
  if (!rasterPtr->updatePending)
    {
      Tk_DoWhenIdle (RasterDisplay, (ClientData) rasterPtr);
      rasterPtr->updatePending = 1;
    }
  XDestroyImage(gif_image);

  if(fi==NULL)
    {
      loaded=GIF;
      Tcl_GlobalEval(raster->interp,"AdjustMini");
    }
  else
    {
      GetInfo(fi);
      /* put fi back in Fractint format, so it can be saved */
      decode_fractal_info(fi,0);
    }

  if(giferror)
    Tcl_SetResult(rasterPtr->interp,gif_err,TCL_STATIC);
  
  fclose(in);
}

static void
  RasterGetColormap(rasterPtr,cmap)
Raster *rasterPtr;
int *cmap;
{
  int c;
  
  for(c=0; c<256; c++)
    {
      cmap[c]=rasterPtr->origcolor[c].red>>8;
      cmap[c+256]=rasterPtr->origcolor[c].green>>8;
      cmap[c+512]=rasterPtr->origcolor[c].blue>>8;
    }
}
  
void
  RasterSavegif(rasterPtr,filename)
Raster *rasterPtr;
char *filename;
{
  FILE *out;
  static int colormap[3][256];

  giferror=FALSE;

  if((out=fopen(filename,"w"))==NULL)
    {
      giferror=TRUE;
#ifdef HAVE_STRERROR
      sprintf(rasterPtr->interp->result,"%s: %s",filename,strerror(errno));
#else
      sprintf(rasterPtr->interp->result,"%s: error writing file",filename);
#endif
      return;
    }

  RasterGetColormap(rasterPtr,colormap);

  /*
  if(loaded!=GIF)
    SetInfo(fi);
    */
    
  GIFEncode(out,rasterPtr->width,rasterPtr->height,0,0,8,
	    colormap[0],colormap[1],colormap[2],gif_getpixel);

  fclose(out);
}
