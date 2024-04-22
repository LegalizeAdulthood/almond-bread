# 
# about.tcl --
#
#      TCL code implementing the about box.
#
# Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
#
# See section "Copyright" of the on-line help for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

proc about {} {
    global about
    if {$about} {return}
    set about 1
    toplevel .about
    wm resizable .about 0 0
    wm title .about "About"
    wm group .about .i
    frame .about.f -borderwidth 4m
    message .about.f.header -justify center -aspect 10000 -foreground red -text "AlmondBread" 
    j:configure_font .about.f.header {
	-adobe-helvetica-bold-r-normal--*-180-*
	-*-lucidabright-bold-r-normal--*-180-*
	-*-times-bold-r-normal--*-180-*
	default
    }
    message .about.f.text -justify center -aspect 10000 -text "by Michael R. Ganss, rms@cs.tu-berlin.de\n\nA fractal generation program for the X Window System\nVersion 0.22\n\nCopyright \251 1994-1997 Michael Ganss. All rights reserved.\n"
    frame .about.f.okframe -relief sunken -borderwidth 1
    button .about.f.okframe.button -relief sunken -padx 4m -pady 1m -text "OK" -command {
	destroy .about
	set about 0
    }
    pack .about.f -fill x
    pack .about.f.header -fill x
    pack .about.f.text -fill x
    pack [j:rule .about.f] -fill x
    pack [j:filler .about.f] -fill x
    pack .about.f.okframe
    pack .about.f.okframe.button -padx 1m -pady 1m
    j:default_button .about.f.okframe.button .about
    focus .about.f.okframe.button
}
set about 0