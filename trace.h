/* 
 * trace.h --
 *
 *	Tcl Variable Traces Header File.
 *
 * Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
 *
 * See section "Copyright" of the on-line help for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef TRACE_H
#define TRACE_H

typedef struct
{
  int *value;
  int min,max;
  int canchange;
}
IntTrace;

typedef struct
{
  double *value;
  double min,max;
  int canchange;
}
DoubleTrace;

char *IntCallback(ClientData data, Tcl_Interp *interp,
		  char *name1, char *name2,
		  int flags);

char *DoubleCallback(ClientData data, Tcl_Interp *interp,
		     char *name1, char *name2,
		     int flags);

void InstallTraces(Tcl_Interp *interp);

#endif /* TRACE_H */
