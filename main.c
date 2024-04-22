/* 
 * main.c --
 *
 *	This file contains a generic main program for Tk-based applications.
 *	It can be used as-is for many applications, just by supplying a
 *	different appInitProc procedure for each specific application.
 *	Or, it can be used as a template for creating new main programs
 *	for Tk applications.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#include "conf.h"
#include <stdio.h>
#if STDC_HEADERS
#include <string.h>
#include <stdlib.h>
#else
#include "stdlib.h"
#include "string.h"
#endif
#include <tcl.h>
#include <tk.h>
#include "fract.h"
#include "fractint.h"
#include "trace.h"

#ifdef HAVE_ISATTY
extern int isatty(int desc);
#endif

#if TK_MAJOR_VERSION == 4
#if TK_MINOR_VERSION == 0
#include "main4.0.c"
#elif TK_MINOR_VERSION == 1
#include "main4.1.c"
#elif TK_MINOR_VERSION == 2
#include "main4.2.c"
#else
#include "main4.2.c"
#endif
#else
#include "main4.2.c"
#endif
