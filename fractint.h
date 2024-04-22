/* 
 * fractint.h --
 *
 *	fractal_info structure.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef FRACTINT_H
#define FRACTINT_H

#define BYTE unsigned char
#define FRACTAL_INFO_SIZE 502

struct fractal_info	    /*  for saving data in GIF file     */
{
    char  info_id[8];	    /* Unique identifier for info block */
    short iterations;
    short fractal_type;	    /* 0=Mandelbrot 1=Julia 2= ... */
    double xmin;
    double xmax;
    double ymin;
    double ymax;
    double creal;
    double cimag;
    short videomodeax;
    short videomodebx;
    short videomodecx;
    short videomodedx;
    short dotmode;
    short xdots;
    short ydots;
    short colors;
    short version;	    /* used to be 'future[0]' */
    float parm3;
    float parm4;
    float potential[3];
    short rseed;
    short rflag;
    short biomorph;
    short inside;
    short logmap;
    float invert[3];
    short decomp[2];
    short symmetry;
			/* version 2 stuff */
    short init3d[16];
    short previewfactor;
    short xtrans;
    short ytrans;
    short red_crop_left;
    short red_crop_right;
    short blue_crop_left;
    short blue_crop_right;
    short red_bright;
    short blue_bright;
    short xadjust;
    short eyeseparation;
    short glassestype;
			/* version 3 stuff, release 13 */
    short outside;
			/* version 4 stuff, release 14 */
    double x3rd;	  /* 3rd corner */
    double y3rd;
    char stdcalcmode;	  /* 1/2/g/b */
    char useinitorbit;	  /* init Mandelbrot orbit flag */
    short calc_status;	  /* resumable, finished, etc */
    long tot_extend_len;  /* total length of extension blocks in .gif file */
    short distest;
    short floatflag;
    short bailout;
    long calctime;
    BYTE trigndx[4];      /* which trig functions selected */
    short finattract;
    double initorbit[2];  /* init Mandelbrot orbit values */
    short periodicity;	  /* periodicity checking */
			/* version 5 stuff, release 15 */
    short pot16bit;	  /* save 16 bit continuous potential info */
    float faspectratio;   /* finalaspectratio, y/x */
    short system; 	  /* 0 for dos, 1 for windows */
    short release;	  /* release number, with 2 decimals implied */
    short flag3d; 	  /* stored only for now, for future use */
    short transparent[2];
    short ambient;
    short haze;
    short randomize;
			/* version 6 stuff, release 15.x */
    short rotate_lo;
    short rotate_hi;
    short distestwidth;
			/* version 7 stuff, release 16 */
    double dparm3;
    double dparm4;
			/* version 8 stuff, release 17 */
    short fillcolor;
			/* version 9 stuff, release 18 */
    double mxmaxfp;
    double mxminfp;
    double mymaxfp;
    double myminfp;
    short zdots;
    float originfp;
    float depthfp;
    float heightfp;
    float widthfp;
    float distfp;
    float eyesfp;
    short orbittype;
    short juli3Dmode;
    short maxfn;
    short inversejulia;
    double dparm5;
    double dparm6;
    double dparm7;
    double dparm8;
    double dparm9;
    double dparm10;
    short future[50];	  /* for stuff we haven't thought of yet */
};

extern struct fractal_info *fi;

#define OWN 0
#define MANDEL 1
#define OTHERFRACTAL 2
#define GIF 3
extern int loaded;

extern void decode_fractal_info(struct fractal_info *info, int dir);

extern void SetDefaultInfo(void);

extern void GetInfo(struct fractal_info *fi);

extern void SetInfo(struct fractal_info *fi);

#endif /* FRACTINT_H */
