/* 
 * gif.h --
 *
 *      GIF(tm) Header File.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef GIF_H
#define GIF_H

#include <tcl.h>
#include <tk.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>

extern void gif_message(char *fmt, ...);

extern void gif_error(char *fmt, ...);

extern int giferror;

extern int gif_configure(int width, int height);

extern int gif_w,gif_h;
extern XImage *gif_image;
extern CARD32 *gif_imagedata;

static inline void gif_putpixel(int x, int y, int c)
{
  raster->image[raster->width*y+x]=c;
  gif_imagedata[y*gif_w+x]=(CARD32)raster->xcolor[c]->pixel;
}

static inline void gif_putline(int x1, int x2, int y, int c)
{
  register int x=y*gif_w,s=x+x2;
  register CARD32 pixel=(CARD32)raster->xcolor[c]->pixel;
  
  memset(raster->image+y*raster->width+x1,c,x2-x1+1);
  for(x+=x1; x<=s; x++)
    gif_imagedata[x]=pixel;
}

static inline int gif_getpixel(int x, int y)
{
  return RasterGetpixel(raster,x,y);
}

extern void gif_putpixel(int x, int y, int c);

extern void gif_putline(int x1, int x2, int y, int c);

extern int gif_getpixel(int x, int y);

extern void gif_setcolormap(unsigned char cmap[3][256]);

extern void ReadGIF (FILE *fd, int imageNumber );

extern int verbose, showComment;

extern void GIFEncode (FILE* fp, int GWidth, int GHeight, int GInterlace, int Background, int BitsPerPixel, int Red[], int Green[], int Blue[], int (*GetPixel)());

extern void RasterLoadgif(Raster *rasterPtr, char *filename);
     
extern void RasterSavegif(Raster *rasterPtr, char *filename);

#endif /* GIF_H */
