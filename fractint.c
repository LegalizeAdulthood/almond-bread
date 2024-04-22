/* 
 * fractint.c --
 *
 *	Fractint Compatibility Routines, these deal with reading and writing
 *      Fractint's fractal_info application extension data structure.
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
#include <stdlib.h>
#include <string.h>
#include <float.h>
#else
#include "stdlib.h"
#include "string.h"
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifndef FLT_MIN
 #define FLT_MIN 1.175494351E-38F
#endif
#include <math.h>
#include <tcl.h>
#include <tk.h>
#include "raster.h"
#include "fract.h"
#include "coloring.h"
#include "fractint.h"

struct fractal_info default_fi,own_fi;
struct fractal_info *fi=(&own_fi);

int loaded=OWN;

void SetDefaultInfo(void)
{
  memset(&default_fi,0,sizeof(struct fractal_info));
  strcpy(default_fi.info_id,"Fractal");
  default_fi.fractal_type=4;                /* mandelfp */
  default_fi.videomodeax=0;
  default_fi.videomodebx=0;
  default_fi.videomodecx=0;
  default_fi.videomodedx=0;
  default_fi.dotmode=(-1);
  default_fi.colors=256;
  default_fi.version=9;
  default_fi.biomorph=(-1);
  default_fi.init3d[1]=60;
  default_fi.init3d[2]=30;
  default_fi.init3d[4]=90;
  default_fi.init3d[5]=90;
  default_fi.init3d[6]=30;
  default_fi.init3d[12]=1;
  default_fi.init3d[13]=(-1);
  default_fi.init3d[14]=1;
  default_fi.symmetry=999;
  default_fi.previewfactor=20;
  default_fi.red_crop_left=4;
  default_fi.blue_crop_right=4;
  default_fi.red_bright=80;
  default_fi.blue_bright=100;
  default_fi.outside=(-1);
  default_fi.stdcalcmode='g';
  default_fi.calc_status=4;
  default_fi.tot_extend_len=FRACTAL_INFO_SIZE+(FRACTAL_INFO_SIZE+254)/255+15;
  default_fi.trigndx[1]=6;
  default_fi.trigndx[2]=2;
  default_fi.trigndx[3]=3;
  default_fi.periodicity=1;
  default_fi.faspectratio=0.75;
  default_fi.release=1820;
  default_fi.ambient=5120;
  default_fi.rotate_lo=1;
  default_fi.rotate_hi=256;
  default_fi.floatflag=1;
  default_fi.distestwidth=71;
  default_fi.fillcolor=(-1);
}

void GetInfo(struct fractal_info *fi)
{
  char dummy[256];
  
  decode_fractal_info(fi,1);
  
  if(fi->fractal_type!=0&&fi->fractal_type!=4)
    {
      loaded=OTHERFRACTAL;
      Tcl_GlobalEval(raster->interp,"AdjustMini");
      return;
    }

  loaded=MANDEL;
  
  sprintf(dummy,"%d",fi->iterations);
  Tcl_SetVar(raster->interp,"maxiter",dummy,TCL_GLOBAL_ONLY);

  if(fi->inside>=0)
    {
      sprintf(dummy,"%d",fi->inside);
      Tcl_SetVar(raster->interp,"inside",dummy,TCL_GLOBAL_ONLY);
    }

  sprintf(dummy,"%d",fi->colors);
  Tcl_SetVar(raster->interp,"colors",dummy,TCL_GLOBAL_ONLY);
  
  sprintf(dummy,"%d",fi->bailout==0?4:fi->bailout);
  Tcl_SetVar(raster->interp,"bailout",dummy,TCL_GLOBAL_ONLY);
  
  if(fi->decomp[0])
    {
      Tcl_SetVar(raster->interp,"scheme","2",TCL_GLOBAL_ONLY);
      sprintf(dummy,"%d",fi->decomp[0]);
      Tcl_SetVar(raster->interp,"arity",dummy,TCL_GLOBAL_ONLY);
    }
  else
    if(fabs(fi->potential[0])>FLT_MIN)
      {
	Tcl_SetVar(raster->interp,"scheme","1",TCL_GLOBAL_ONLY);
	sprintf(dummy,"%.*g",16,fi->potential[1]);
	Tcl_SetVar(raster->interp,"slope",dummy,TCL_GLOBAL_ONLY);
	sprintf(dummy,"%.*g",16,fi->potential[0]);
	Tcl_SetVar(raster->interp,"maxcolor",dummy,TCL_GLOBAL_ONLY);
	if(fi->potential[2]!=0)
	  {
	    sprintf(dummy,"%.*g",16,fi->potential[2]);
	    Tcl_SetVar(raster->interp,"bailout",dummy,TCL_GLOBAL_ONLY);
	  }
      }
    else
      Tcl_SetVar(raster->interp,"scheme","0",TCL_GLOBAL_ONLY);

  sprintf(dummy,"%.*g",16,fi->xmin);
  Tcl_SetVar(raster->interp,"minr",dummy,TCL_GLOBAL_ONLY);
  
  sprintf(dummy,"%.*g",16,fi->xmax);
  Tcl_SetVar(raster->interp,"maxr",dummy,TCL_GLOBAL_ONLY);

  sprintf(dummy,"%.*g",16,-fi->ymin);
  Tcl_SetVar(raster->interp,"maxi",dummy,TCL_GLOBAL_ONLY);

  sprintf(dummy,"%.*g",16,-fi->ymax);
  Tcl_SetVar(raster->interp,"mini",dummy,TCL_GLOBAL_ONLY);

  sprintf(dummy,"%3ld:%02ld:%02ld", fi->calctime/360000,
	  (fi->calctime%360000)/6000, (fi->calctime%6000)/100);
  Tcl_VarEval(raster->interp,".i.bot.time.digits configure -text ",
	      dummy,NULL);
  
  nochange=TRUE;
  Tcl_SetVar(raster->interp,"force","0",TCL_GLOBAL_ONLY);

  complete=(fi->calc_status==4);

  prevminr=minr;
  prevmaxr=maxr;
  prevmini=mini;
  prevmaxi=maxi;
}

void SetInfo(struct fractal_info *fi)
{
  int secs;
  
  if(loaded==OWN)
    {
      fi=(&own_fi);
      *fi=default_fi;
    }
      
  if(loaded!=GIF)
    {
      fi->iterations=maxiter;
      fi->xmin=fi->x3rd=minr;
      fi->xmax=maxr;
      fi->ymin=fi->y3rd=(-maxi);
      fi->ymax=(-mini);
      fi->xdots=maxx+1;
      fi->ydots=maxy+1;
      fi->colors=raster->colors;
      if(scheme==1)
	{
	  fi->potential[0]=maxcolor;
	  fi->potential[1]=slope;
	  fi->potential[2]=bailout;
	}
      else
	{
	  fi->potential[0]=0.0;
	  fi->potential[1]=0.0;
	  fi->potential[2]=0.0;
	}
      fi->inside=inside;
      if(scheme==2)
	fi->decomp[0]=arity;
      else
	fi->decomp[0]=0;
      fi->stdcalcmode='g';
      fi->bailout=(short)bailout;
      Tcl_GetInt(raster->interp,Tcl_GetVar(raster->interp,"seconds",TCL_GLOBAL_ONLY),
		 &secs);
      fi->calctime=(long)secs*100;
    }

  decode_fractal_info(fi,0);
}
