/* generalasm.c
 * This file contains routines to replace general.asm.
 *
 * This file Copyright 1991 Ken Shirriff.  It may be used according to the
 * fractint license conditions, blah blah blah.
 */
#include "conf.h"
#include <stdio.h>
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
#include "raster.h"
#include "gif.h"
#include "fractint.h"

static void getChar(unsigned char *, unsigned char **, int),
  getInt(short *, unsigned char **, int),
  getLong(long *, unsigned char **, int),
  getFloat(float *, unsigned char **, int),
  getDouble(double *, unsigned char **, int);

void
  decode_fractal_info(info,dir)
struct fractal_info *info;
int dir;
{
  unsigned char *buf;
  unsigned char *bufPtr;
  int i;
  
  if (dir==1) {
    buf = (unsigned char *)malloc(FRACTAL_INFO_SIZE);
    bufPtr = buf;
    memcpy((char *)buf,(char *)info,FRACTAL_INFO_SIZE);
  }  else {
    buf = (unsigned char *)malloc(sizeof(struct fractal_info));
    bufPtr = buf;
    memcpy((char *)buf,(char *)info,sizeof(struct fractal_info));
  }
  
  if (dir==1) {
    strncpy(info->info_id,(char *)bufPtr,8);
  } else {
    strncpy((char *)bufPtr,info->info_id,8);
  }
  bufPtr += 8;
  getInt(&info->iterations,&bufPtr,dir);
  getInt(&info->fractal_type,&bufPtr,dir);
  getDouble(&info->xmin,&bufPtr,dir);
  getDouble(&info->xmax,&bufPtr,dir);
  getDouble(&info->ymin,&bufPtr,dir);
  getDouble(&info->ymax,&bufPtr,dir);
  getDouble(&info->creal,&bufPtr,dir);
  getDouble(&info->cimag,&bufPtr,dir);
  getInt(&info->videomodeax,&bufPtr,dir);
  getInt(&info->videomodebx,&bufPtr,dir);
  getInt(&info->videomodecx,&bufPtr,dir);
  getInt(&info->videomodedx,&bufPtr,dir);
  getInt(&info->dotmode,&bufPtr,dir);
  getInt(&info->xdots,&bufPtr,dir);
  getInt(&info->ydots,&bufPtr,dir);
  getInt(&info->colors,&bufPtr,dir);
  getInt(&info->version,&bufPtr,dir);
  getFloat(&info->parm3,&bufPtr,dir);
  getFloat(&info->parm4,&bufPtr,dir);
  getFloat(&info->potential[0],&bufPtr,dir);
  getFloat(&info->potential[1],&bufPtr,dir);
  getFloat(&info->potential[2],&bufPtr,dir);
  getInt(&info->rseed,&bufPtr,dir);
  getInt(&info->rflag,&bufPtr,dir);
  getInt(&info->biomorph,&bufPtr,dir);
  getInt(&info->inside,&bufPtr,dir);
  getInt(&info->logmap,&bufPtr,dir);
  getFloat(&info->invert[0],&bufPtr,dir);
  getFloat(&info->invert[1],&bufPtr,dir);
  getFloat(&info->invert[2],&bufPtr,dir);
  getInt(&info->decomp[0],&bufPtr,dir);
  getInt(&info->decomp[1],&bufPtr,dir);
  getInt(&info->symmetry,&bufPtr,dir);
  for (i=0;i<16;i++) {
    getInt(&info->init3d[i],&bufPtr,dir);
  }
  getInt(&info->previewfactor,&bufPtr,dir);
  getInt(&info->xtrans,&bufPtr,dir);
  getInt(&info->ytrans,&bufPtr,dir);
  getInt(&info->red_crop_left,&bufPtr,dir);
  getInt(&info->red_crop_right,&bufPtr,dir);
  getInt(&info->blue_crop_left,&bufPtr,dir);
  getInt(&info->blue_crop_right,&bufPtr,dir);
  getInt(&info->red_bright,&bufPtr,dir);
  getInt(&info->blue_bright,&bufPtr,dir);
  getInt(&info->xadjust,&bufPtr,dir);
  getInt(&info->eyeseparation,&bufPtr,dir);
  getInt(&info->glassestype,&bufPtr,dir);
  getInt(&info->outside,&bufPtr,dir);
  getDouble(&info->x3rd,&bufPtr,dir);
  getDouble(&info->y3rd,&bufPtr,dir);
  getChar((unsigned char *)&info->stdcalcmode,&bufPtr,dir);
  getChar((unsigned char *)&info->useinitorbit,&bufPtr,dir);
  getInt(&info->calc_status,&bufPtr,dir);
  getLong(&info->tot_extend_len,&bufPtr,dir);
  getInt(&info->distest,&bufPtr,dir);
  getInt(&info->floatflag,&bufPtr,dir);
  getInt(&info->bailout,&bufPtr,dir);
  getLong(&info->calctime,&bufPtr,dir);
  for (i=0;i<4;i++) {
    getChar(&info->trigndx[i],&bufPtr,dir);
  }
  getInt(&info->finattract,&bufPtr,dir);
  getDouble(&info->initorbit[0],&bufPtr,dir);
  getDouble(&info->initorbit[1],&bufPtr,dir);
  getInt(&info->periodicity,&bufPtr,dir);
  getInt(&info->pot16bit,&bufPtr,dir);
  getFloat(&info->faspectratio,&bufPtr,dir);
  getInt(&info->system,&bufPtr,dir);
  getInt(&info->release,&bufPtr,dir);
  getInt(&info->flag3d,&bufPtr,dir);
  getInt(&info->transparent[0],&bufPtr,dir);
  getInt(&info->transparent[1],&bufPtr,dir);
  getInt(&info->ambient,&bufPtr,dir);
  getInt(&info->haze,&bufPtr,dir);
  getInt(&info->randomize,&bufPtr,dir);
  getInt(&info->rotate_lo,&bufPtr,dir);
  getInt(&info->rotate_hi,&bufPtr,dir);
  getInt(&info->distestwidth,&bufPtr,dir);
  getDouble(&info->dparm3,&bufPtr,dir);
  getDouble(&info->dparm4,&bufPtr,dir);
  getInt(&info->fillcolor,&bufPtr,dir);
  getDouble(&info->mxmaxfp,&bufPtr,dir);
  getDouble(&info->mxminfp,&bufPtr,dir);
  getDouble(&info->mymaxfp,&bufPtr,dir);
  getDouble(&info->myminfp,&bufPtr,dir);
  getInt(&info->zdots,&bufPtr,dir);
  getFloat(&info->originfp,&bufPtr,dir);
  getFloat(&info->depthfp,&bufPtr,dir);
  getFloat(&info->heightfp,&bufPtr,dir);
  getFloat(&info->widthfp,&bufPtr,dir);
  getFloat(&info->distfp,&bufPtr,dir);
  getFloat(&info->eyesfp,&bufPtr,dir);
  getInt(&info->orbittype,&bufPtr,dir);
  getInt(&info->juli3Dmode,&bufPtr,dir);
  getInt(&info->maxfn,&bufPtr,dir);
  getInt(&info->inversejulia,&bufPtr,dir);
  getDouble(&info->dparm5,&bufPtr,dir);
  getDouble(&info->dparm6,&bufPtr,dir);
  getDouble(&info->dparm7,&bufPtr,dir);
  getDouble(&info->dparm8,&bufPtr,dir);
  getDouble(&info->dparm9,&bufPtr,dir);
  getDouble(&info->dparm10,&bufPtr,dir);
  for (i=0;i<50;i++) {
    getInt(&info->future[i],&bufPtr,dir);
  }
  if (bufPtr-buf != FRACTAL_INFO_SIZE) {
    gif_message("components = %d bytes, FRACTAL_INFO_SIZE = %d",
		bufPtr-buf, FRACTAL_INFO_SIZE);
  }
  if (dir==0) {
    memcpy((char *)info,(char *)buf,FRACTAL_INFO_SIZE);
  }
  
  free(buf);
}


static void
  getChar(dst,src,dir)
unsigned char *dst;
unsigned char **src;
int dir;
{
  if (dir==1) {
    *dst = **src;
  } else {
    **src = *dst;
  }
  (*src)++;
}


static void
  getInt(dst,src,dir)
short *dst;
unsigned char **src;
int dir;
{
  if (dir==1) {
    *dst = (*src)[0] + ((((char *)(*src))[1])<<8);
  } else {
    (*src)[0] = (*dst)&0xff;
    (*src)[1] = ((*dst)&0xff00)>>8;
  }
  (*src) += 2; 
}


static void
  getLong(dst,src,dir)
long *dst;
unsigned char **src;
int dir;
{
  if (dir==1) {
    *dst = ((unsigned long)((*src)[0])) +
      (((unsigned long)((*src)[1]))<<8) +
	(((unsigned long)((*src)[2]))<<16) +
	  (((long)(((char *)(*src))[3]))<<24);
  } else {
    (*src)[0] = (*dst)&0xff;
    (*src)[1] = ((*dst)&0xff00)>>8;
    (*src)[2] = ((*dst)&0xff0000)>>8;
    (*src)[3] = ((*dst)&0xff000000)>>8;
  }
  (*src) += 4; 
}

#define P4 16.
#define P7 128.
#define P8 256.
#define P12 4096.
#define P15 32768.
#define P20 1048576.
#define P23 8388608.
#define P28 268435456.
#define P36 68719476736.
#define P44 17592186044416.
#define P52 4503599627370496.



static void
  getDouble(dst,src,dir)
double *dst;
unsigned char **src;
int dir;
{
  int e;
  double f;
  int i;
  if (dir==1) {
    for (i=0;i<8;i++) {
      if ((*src)[i] != 0) break;
    }
    if (i==8) {
      *dst = 0;
    } else {
      e = (((*src)[7]&0x7f)<<4) + (((*src)[6]&0xf0)>>4) - 1023;
      f = 1 + ((*src)[6]&0x0f)/P4 + (*src)[5]/P12 + (*src)[4]/P20 +
	(*src)[3]/P28 + (*src)[2]/P36 + (*src)[1]/P44 + (*src)[0]/P52;
      f *= pow(2.,(double)e);
      if ((*src)[7]&0x80) {
	f = -f;
      }
      *dst = f;
    }
  } else {
    if (*dst==0) {
      memset((char *)(*src),0,8);
    } else {
      int s=0;
      f = *dst;
      if (f<0) {
	s = 0x80;
	f = -f;
      }
      e = log(f)/log(2.);
      f = f/pow(2.,(double)e) - 1;
      if (f<0) {
	e--;
	f = (f+1)*2-1;
      } else if (f>=1) {
	e++;
	f = (f+1)/2-1;
      }
      e += 1023;
      (*src)[7] = s | ((e&0x7f0)>>4);
      f *= P4;
      (*src)[6] = ((e&0x0f)<<4) | (((int)f)&0x0f);
      f = (f-(int)f)*P8;
      (*src)[5] = (((int)f)&0xff);
      f = (f-(int)f)*P8;
      (*src)[4] = (((int)f)&0xff);
      f = (f-(int)f)*P8;
      (*src)[3] = (((int)f)&0xff);
      f = (f-(int)f)*P8;
      (*src)[2] = (((int)f)&0xff);
      f = (f-(int)f)*P8;
      (*src)[1] = (((int)f)&0xff);
      f = (f-(int)f)*P8;
      (*src)[0] = (((int)f)&0xff);
    }
  }
  *src += 8; 
}


static void
  getFloat(dst,src,dir)
float *dst;
unsigned char **src;
int dir;
{
  int e;
  double f;
  int i;
  if (dir==1) {
    for (i=0;i<4;i++) {
      if ((*src)[i] != 0) break;
    }
    if (i==4) {
      *dst = 0;
    } else {
      e = ( (((*src)[3]&0x7f)<<1)| (((*src)[2]&0x80)>>7) ) - 127 ;
      f = 1 + ((*src)[2]&0x7f)/P7 + (*src)[1]/P15 + (*src)[0]/P23;
      f *= pow(2.,(double)e);
      if ((*src)[3]&0x80) {
	f = -f;
      }
      *dst = f;
    }
  } else {
    if (*dst==0) {
      memset((char *)(*src),0,4);
    } else {
      int s=0;
      f = *dst;
      if (f<0) {
	s = 0x80;
	f = -f;
      }
      e = log(f)/log(2.);
      f = f/pow(2.,(double)e) - 1;
      if (f<0) {
	e--;
	f = (f+1)*2-1;
      } else if (f>=1) {
	e++;
	f = (f+1)/2-1;
      }
      e += 127;
      (*src)[3] = s | ((e&0xfe)>>1);
      f *= P7;
      (*src)[2] = ((e&0x01)<<7) | (((int)f)&0x7f);
      f = (f-(int)f)*P8;
      (*src)[1] = (((int)f)&0xff);
      f = (f-(int)f)*P8;
      (*src)[0] = (((int)f)&0xff);
    }
  }
  *src += 4; 
}
