AlmondBread

1. Introduction

AlmondBread is yet another program which calculates and displays
images of the Mandelbrot set. It is certainly not as versatile as
e.g. Fractint, but I think it can be fun to play with nonetheless,
particularly because of its nice interface (courtesy of Tcl/Tk), fast
algorithms and flexible colormap handling. Here are some more of
AlmondBread's features:

- Fast algorithms, among them an implementation of Simultaneous Orbit
Iteration. For example, the accompanying image lightning.gif (an
extreme case, admittedly) took more than 2 hours on my little 486DX-33
using Fractint for DOS, less than 4 minutes using AlmondBread under
Linux.
- Save/Load Fractint-compatible GIF files.
- Load Fractint-style colormap files (.MAP files).
- Shared or own colormap using an allocation scheme similar to that of
xv(1).
- Easy-to-use zoombox function.

2. System Requirements

You will need a system that runs X11 (at least Release 5). In
addition, Tcl/Tk (at least version 7.4/4.0) must be installed on your
system. If it is not, you can get it for free from
http://www.smli.com/research/tcl. Also, you must have the jstools
suite of programs installed on your system. If you don't, get it for
free from http://www.shore.net/~js/js-jstools.html.
Compiling AlmondBread requires a C compiler. Instructions on
installing AlmondBread can be found in the file INSTALL.

3. Further Documentation

AlmondBread has on-line help. The files AlmondBread.ps and
AlmondBread.html contain the same text as PostScript and HTML. Both
were generated automatically from the jdoc source, so look sucky.
There is also a little man-page (AlmondBread.1), which is not nearly
as elaborate as the on-line help.

Please check out the AlmondBread homepage at
http://www.cs.tu-berlin.de/~rms/AlmondBread/

4. Copyright

AlmondBread was written by Michael Ganss (rms@cs.tu-berlin.de).
Please contact me if you discover any bugs, have fixes for them or any
other ideas that might aid in the improvement of this software,
including favorable comments as well. AlmondBread is Copyright (c)
1994-1997 Michael R. Ganss. All Rights Reserved. See section
"Copyright" of the on-line help for a full statement of rights and
permissions.

Michael Ganss
(rms@cs.tu-berlin.de)
