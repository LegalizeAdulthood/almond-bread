/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, David Koblas.                                     | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */

#include "conf.h"
#include <stdio.h>
#if STDC_HEADERS
#include <string.h>
#else
#include "stdlib.h"
#include "string.h"
#endif
#include <tcl.h>
#include <tk.h>
#include "raster.h"
#include "gif.h"
/* Fractint specific */
#include "fractint.h"

#define	MAXCOLORMAPSIZE		256

#define	TRUE	1
#define	FALSE	0

#define CM_RED		0
#define CM_GREEN	1
#define CM_BLUE		2

#define	MAX_LWZ_BITS		12

#define INTERLACE		0x40
#define LOCALCOLORMAP	        0x80
#define BitSet(byte, bit)	(((byte) & (bit)) == (bit))

#define	ReadOK(file,buffer,len)	(fread(buffer, len, 1, file) != 0)

#define LM_to_uint(a,b)			(((b)<<8)|(a))

struct {
  unsigned int	Width;
  unsigned int	Height;
  unsigned char	ColorMap[3][MAXCOLORMAPSIZE];
  unsigned int	BitPixel;
  unsigned int	ColorResolution;
  unsigned int	Background;
  unsigned int	AspectRatio;
} GifScreen;

struct {
  int	transparent;
  int	delayTime;
  int	inputFlag;
  int	disposal;
} Gif89 = { -1, -1, -1, 0 };

int	verbose=FALSE;
int	showComment=FALSE;

static int ReadColorMap (FILE *fd, int number,
			 unsigned char buffer[3][MAXCOLORMAPSIZE]);
static int DoExtension (FILE *fd, int label);
static int GetDataBlock (FILE *fd, unsigned char  *buf);
static int GetCode (FILE *fd, int code_size, int flag);
static int LWZReadByte (FILE *fd, int flag, int input_code_size);
static void ReadImage (FILE *fd, int len, int height,
		       unsigned char cmap[3][MAXCOLORMAPSIZE],
		       int interlace, int ignore);

void
  ReadGIF(fd, imageNumber)
FILE	*fd;
int	imageNumber;
{
  unsigned char	buf[16];
  unsigned char	c;
  unsigned char	localColorMap[3][MAXCOLORMAPSIZE];
  int		useGlobalColormap;
  int		bitPixel;
  int		imageCount = 0;
  char		version[4];
  
  if (! ReadOK(fd,buf,6))
    {
      gif_error("error reading magic number" );
      return;
    }
  
  if (strncmp((char *)buf,"GIF",3) != 0)
    {
      gif_error("not a GIF file" );
      return;
    }
  
  strncpy(version, (char *)buf + 3, 3);
  version[3] = '\0';
  
  if ((strcmp(version, "87a") != 0) && (strcmp(version, "89a") != 0))
    {
      gif_error("bad version number, not '87a' or '89a'" );
      return;
    }
  
  if (! ReadOK(fd,buf,7))
    {
      gif_error("failed to read screen descriptor" );
      return;
    }
  
  GifScreen.Width           = LM_to_uint(buf[0],buf[1]);
  GifScreen.Height          = LM_to_uint(buf[2],buf[3]);
  GifScreen.BitPixel        = 2<<(buf[4]&0x07);
  GifScreen.ColorResolution = (((buf[4]&0x70)>>3)+1);
  GifScreen.Background      = buf[5];
  GifScreen.AspectRatio     = buf[6];
  
  if (BitSet(buf[4], LOCALCOLORMAP)) {	/* Global Colormap */
    if (ReadColorMap(fd,GifScreen.BitPixel,GifScreen.ColorMap))
      {
	gif_error("error reading global colormap" );
	return;
      }
  }
  
  if (GifScreen.AspectRatio != 0 && GifScreen.AspectRatio != 49) {
    float	r;
    r = ( (float) GifScreen.AspectRatio + 15.0 ) / 64.0;
    gif_message("warning - non-square pixels; to fix do a 'pnmscale -%cscale %g'",
	       r < 1.0 ? 'x' : 'y',
	       r < 1.0 ? 1.0 / r : r );
  }
  
  for (;;) {
    if (! ReadOK(fd,&c,1))
      {
	gif_error("EOF / read error on image data" );
	return;
      }
    
    if (c == ';') {		/* GIF terminator */
      if (imageCount < imageNumber)
	{
	  gif_error("only %d image%s found in file",
		    imageCount, imageCount>1?"s":"" );
	  return;
	}
      return;
    }
    
    if (c == '!') { 	/* Extension */
      if (! ReadOK(fd,&c,1))
	{
	  gif_error("OF / read error on extension function code");
	  return;
	}
      DoExtension(fd, c);
      continue;
    }
    
    if (c != ',') {		/* Not a valid start character */
      gif_message("bogus character 0x%02x, ignoring", (int) c ); 
      continue;
    }
    
    ++imageCount;
    
    if (! ReadOK(fd,buf,9))
      {
	gif_error("couldn't read left/top/width/height");
	return;
      }
    
    useGlobalColormap = ! BitSet(buf[8], LOCALCOLORMAP);
    
    bitPixel = 1<<((buf[8]&0x07)+1);
    
    if (! useGlobalColormap) {
      if (ReadColorMap(fd, bitPixel, localColorMap))
	{
	  gif_error("error reading local colormap" );
	  return;
	}
      ReadImage(fd, LM_to_uint(buf[4],buf[5]),
		LM_to_uint(buf[6],buf[7]), localColorMap,
		BitSet(buf[8], INTERLACE), imageCount != imageNumber);
    } else {
      ReadImage(fd, LM_to_uint(buf[4],buf[5]),
		LM_to_uint(buf[6],buf[7]), GifScreen.ColorMap,
		BitSet(buf[8], INTERLACE), imageCount != imageNumber);
    }
    
  }
}

static int
  ReadColorMap(fd,number,buffer)
FILE		*fd;
int		number;
unsigned char	buffer[3][MAXCOLORMAPSIZE];
{
  int		i;
  unsigned char	rgb[3];
  
  for (i = 0; i < number; ++i) {
    if (! ReadOK(fd, rgb, sizeof(rgb)))
      {
	gif_error("bad colormap" );
	return TRUE;
      }
    buffer[CM_RED][i] = rgb[0] ;
    buffer[CM_GREEN][i] = rgb[1] ;
    buffer[CM_BLUE][i] = rgb[2] ;
  }
  
  gif_setcolormap(buffer);
  
  return FALSE;
}

static int
  DoExtension(fd, label)
FILE	*fd;
int	label;
{
  static char	buf[256];
  char		*str;
  /* Fractint specific */
  static unsigned char fibuf[1024]; /* this should be enough */
  int pos=0;
  
  switch (label) {
  case 0x01:		/* Plain Text Extension */
    str = "Plain Text Extension";
#ifdef notdef
    if (GetDataBlock(fd, (unsigned char*) buf) == 0)
      ;
    
    lpos   = LM_to_uint(buf[0], buf[1]);
    tpos   = LM_to_uint(buf[2], buf[3]);
    width  = LM_to_uint(buf[4], buf[5]);
    height = LM_to_uint(buf[6], buf[7]);
    cellw  = buf[8];
    cellh  = buf[9];
    foreground = buf[10];
    background = buf[11];
    
    while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
      
      gif_putpixel(xpos,ypos,v);
      
      ++index;
    }
    
    return FALSE;
#else
    break;
#endif
  case 0xff:		/* Application Extension */
    str = "Application Extension";

    /* Fractint specific code */

    GetDataBlock(fd, (unsigned char *)buf);
    if(strncmp(buf,"fractint",8))
      break;
    if(buf[8]!='0'||buf[9]!='0'||buf[10]!='1')
      break;
    gif_message("got Fractint application extension");
    while((GetDataBlock(fd, fibuf+pos))!=0)
      pos+=255;
    fi=(struct fractal_info *)fibuf;
    
    return FALSE;
  case 0xfe:		/* Comment Extension */
    str = "Comment Extension";
    while (GetDataBlock(fd, (unsigned char*) buf) != 0) {
      if (showComment)
	gif_message("gif comment: %s", buf );
    }
    return FALSE;
  case 0xf9:		/* Graphic Control Extension */
    str = "Graphic Control Extension";
    (void) GetDataBlock(fd, (unsigned char*) buf);
    Gif89.disposal    = (buf[0] >> 2) & 0x7;
    Gif89.inputFlag   = (buf[0] >> 1) & 0x1;
    Gif89.delayTime   = LM_to_uint(buf[1],buf[2]);
    if ((buf[0] & 0x1) != 0)
      Gif89.transparent = buf[3];
    
    while (GetDataBlock(fd, (unsigned char*) buf) != 0)
      ;
    return FALSE;
  default:
    str = buf;
    sprintf(buf, "UNKNOWN (0x%02x)", label);
    break;
  }
  
  gif_message("got a '%s' extension - please report this to koblas@mips.com",
	      str );
  
  while (GetDataBlock(fd, (unsigned char*) buf) != 0)
    ;
  
  return FALSE;
}

int	ZeroDataBlock = FALSE;

static int
  GetDataBlock(fd, buf)
FILE		*fd;
unsigned char 	*buf;
{
  unsigned char	count;
  
  if (! ReadOK(fd,&count,1)) {
    gif_message("error in getting DataBlock size" );
    return -1;
  }
  
  ZeroDataBlock = count == 0;
  
  if ((count != 0) && (! ReadOK(fd, buf, count))) {
    gif_message("error in reading DataBlock" );
    return -1;
  }
  
  return count;
}

static int
  GetCode(fd, code_size, flag)
FILE	*fd;
int	code_size;
int	flag;
{
  static unsigned char	buf[280];
  static int		curbit, lastbit, done, last_byte;
  int			i, j, ret;
  unsigned char		count;
  
  if (flag) {
    curbit = 0;
    lastbit = 0;
    done = FALSE;
    return 0;
  }
  
  if ( (curbit+code_size) >= lastbit) {
    if (done) {
      if (curbit >= lastbit)
	gif_error("ran off the end of my bits" );
      return -1;
    }
    buf[0] = buf[last_byte-2];
    buf[1] = buf[last_byte-1];
    
    if ((count = GetDataBlock(fd, &buf[2])) == 0)
      done = TRUE;
    
    last_byte = 2 + count;
    curbit = (curbit - lastbit) + 16;
    lastbit = (2+count)*8 ;
  }
  
  ret = 0;
  for (i = curbit, j = 0; j < code_size; ++i, ++j)
    ret |= ((buf[ i / 8 ] & (1 << (i % 8))) != 0) << j;
  
  curbit += code_size;
  
  return ret;
}

static int
  LWZReadByte(fd, flag, input_code_size)
FILE	*fd;
int	flag;
int	input_code_size;
{
  static int	fresh = FALSE;
  int		code, incode;
  static int	code_size, set_code_size;
  static int	max_code, max_code_size;
  static int	firstcode, oldcode;
  static int	clear_code, end_code;
  static int	table[2][(1<< MAX_LWZ_BITS)];
  static int	stack[(1<<(MAX_LWZ_BITS))*2], *sp;
  register int	i;
  
  if (flag) {
    set_code_size = input_code_size;
    code_size = set_code_size+1;
    clear_code = 1 << set_code_size ;
    end_code = clear_code + 1;
    max_code_size = 2*clear_code;
    max_code = clear_code+2;
    
    GetCode(fd, 0, TRUE);
    
    fresh = TRUE;
    
    for (i = 0; i < clear_code; ++i) {
      table[0][i] = 0;
      table[1][i] = i;
    }
    for (; i < (1<<MAX_LWZ_BITS); ++i)
      table[0][i] = table[1][0] = 0;
    
    sp = stack;
    
    return 0;
  } else if (fresh) {
    fresh = FALSE;
    do {
      firstcode = oldcode =
	GetCode(fd, code_size, FALSE);
    } while (firstcode == clear_code);
    return firstcode;
  }
  
  if (sp > stack)
    return *--sp;
  
  while ((code = GetCode(fd, code_size, FALSE)) >= 0) {
    if (code == clear_code) {
      for (i = 0; i < clear_code; ++i) {
	table[0][i] = 0;
	table[1][i] = i;
      }
      for (; i < (1<<MAX_LWZ_BITS); ++i)
	table[0][i] = table[1][i] = 0;
      code_size = set_code_size+1;
      max_code_size = 2*clear_code;
      max_code = clear_code+2;
      sp = stack;
      firstcode = oldcode =
	GetCode(fd, code_size, FALSE);
      return firstcode;
    } else if (code == end_code) {
      int		count;
      unsigned char	buf[260];
      
      if (ZeroDataBlock)
	return -2;
      
      while ((count = GetDataBlock(fd, buf)) > 0)
	;
      
      if (count != 0)
	gif_message("missing EOD in data stream (common occurance)");
      return -2;
    }
    
    incode = code;
    
    if (code >= max_code) {
      *sp++ = firstcode;
      code = oldcode;
    }
    
    while (code >= clear_code) {
      *sp++ = table[1][code];
      if (code == table[0][code])
	{
	  gif_error("circular table entry BIG ERROR");
	  return -2;
	}
      code = table[0][code];
    }
    
    *sp++ = firstcode = table[1][code];
    
    if ((code = max_code) <(1<<MAX_LWZ_BITS)) {
      table[0][code] = oldcode;
      table[1][code] = firstcode;
      ++max_code;
      if ((max_code >= max_code_size) &&
	  (max_code_size < (1<<MAX_LWZ_BITS))) {
	max_code_size *= 2;
	++code_size;
      }
    }
    
    oldcode = incode;
    
    if (sp > stack)
      return *--sp;
  }
  return code;
}

static void
  ReadImage(fd, len, height, cmap, interlace, ignore)
FILE	*fd;
int	len, height;
unsigned char	cmap[3][MAXCOLORMAPSIZE];
int	interlace, ignore;
{
  unsigned char	c;	
  int		v,oldv=0,oldx=0;
  int		xpos = 0, ypos = 0, pass = 0;
  
  /*
   **  Initialize the Compression routines
   */
  if (! ReadOK(fd,&c,1))
    {
      gif_error("EOF / read error on image data" );
      return;
    }
  
  if (LWZReadByte(fd, TRUE, c) < 0)
    {
      gif_error("error reading image" );
      return;
    }
  
  /*
   **  If this is an "uninteresting picture" ignore it.
   */
  if (ignore) {
    if (verbose)
      gif_message("skipping image..." );
    
    while (LWZReadByte(fd, FALSE, c) >= 0)
      ;
    return;
  }
  
  if(!gif_configure(len,height))
    return;
  
  if (verbose)
    gif_message("reading %d by %d%s GIF image",
	       len, height, interlace ? " interlaced" : "" );
  
  while ((v = LWZReadByte(fd,FALSE,c)) >= 0 ) {

    if(!xpos)
      oldv=v;
    else if(v!=oldv)
      {
	if(oldx+1<xpos)
	  gif_putline(oldx,xpos-1,ypos,oldv);
	else
	  gif_putpixel(oldx,ypos,oldv);
	oldv=v;
	oldx=xpos;
      }
    
    ++xpos;
    if (xpos == len) {

      /* Do some events */
      /* No, don't do any, because some events interfere w/screen construction,
	 e.g. bringing up the zoombox */
      /* while(Tk_DoOneEvent(TK_ALL_EVENTS|TK_DONT_WAIT)); */
      
      if(oldx+1<xpos)
	gif_putline(oldx,xpos-1,ypos,oldv);
      else
	gif_putpixel(oldx,ypos,oldv);
      oldx=0;
      
      xpos = 0;
      if (interlace) {
	switch (pass) {
	case 0:
	case 1:
	  ypos += 8; break;
	case 2:
	  ypos += 4; break;
	case 3:
	  ypos += 2; break;
	}
	
	if (ypos >= height) {
	  ++pass;
	  switch (pass) {
	  case 1:
	    ypos = 4; break;
	  case 2:
	    ypos = 2; break;
	  case 3:
	    ypos = 1; break;
	  default:
	    goto fini;
	  }
	}
      } else {
	++ypos;
      }
    }
    if (ypos >= height)
      break;
  }
  
 fini:
  if (LWZReadByte(fd,FALSE,c)>=0)
    gif_message("too much input data, ignoring extra...");

  /*
  if (verbose)
    gif_message("writing output");
  */
}
