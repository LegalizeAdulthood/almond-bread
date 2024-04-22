# 
# color.tcl --
#
#      TCL code related to colormap management.
#
# Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
#
# See section "Copyright" of the on-line help for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

proc Width {r} {
    set c [$r configure -width]
    scan $c "-width width Width %d %d" dw w
    return $w
}

proc Height {r} {
    set c [$r configure -height]
    scan $c "-height height Height %d %d" dh h
    return $h
}

set showcmap 0
proc ToggleShowColormap {} {
    global showcmap owncmap
    set showcmap [expr !$showcmap]
    if {$showcmap} {
	toplevel .cmap
	wm title .cmap "Colormap"
	wm minsize .cmap 1 1
	wm grid .cmap 10 10 16 16
	wm protocol .cmap WM_DELETE_WINDOW { .i.menu.colormap.m invoke 1 }
	wm group .cmap .i
	.cmap configure -bg black
	raster .cmap.r -width 160 -height 160 -owncmap $owncmap
	pack .cmap.r
	update idletasks
	ShowColormap .cmap.r
	bind .cmap <Configure> {
	    if {[Resize .cmap.r %w %h]} {
		wm grid .cmap [expr %w/16] [expr %h/16] 16 16
		.cmap.r colormap
		ShowColormap .cmap.r
	    }
	}
	bind .cmap.r <Any-Button-3> { .i.menu.colormap.m invoke 1 }
    } else {
	destroy .cmap
    }
}
    
proc ShowColormap {r} {
    set c 0
    set w [Width $r]
    set h [Height $r]
    set bw [expr $w/16]
    set bh [expr $h/16]
    for { set y 0 } { $y < $h && $c < 256 } { incr y $bh } {
	for { set x 0 } { $x < $w } { incr x $bw; incr c 1 } {
	    $r fillbox $x $y $bw $bh $c
	}
    }
}

proc LoadColormap { filename frame } {
    global cmap
    set fd [open $filename r]
    for {set c 0} {$c<256} {incr c} {
	if {[eof $fd]} {
	    set cmap($c,red) 0
	    set cmap($c,green) 0
	    set cmap($c,blue) 0
	} else {
	    gets $fd l
	    set r 0
	    set g 0
	    set b 0
	    scan $l "%d %d %d" r g b
	    set cmap($c,red) [expr $r << 8]
	    set cmap($c,green) [expr $g << 8]
	    set cmap($c,blue) [expr $b << 8]
	}
    }
    close $fd
    $frame colormap cmap
}

set owncmap no
trace variable owncmap w SetOwncmap
proc SetOwncmap {name element op} {
    upvar #0 $name x
    global showcmap owncmap
    .r configure -owncmap $x
    if {$showcmap} {
	.cmap.r configure -owncmap $x
	ShowColormap .cmap.r
    }
}
