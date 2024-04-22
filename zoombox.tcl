# 
# zoombox.tcl --
#
#      TCL code related to zoombox management.
#
# Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
#
# See section "Copyright" of the on-line help for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

bind .r <Any-Button-2> {ToggleZoombox %x %y}
bind .r <Any-B1-Motion> {if {$zb_active} {ResizeZoombox %x %y}}
bind .r <Any-Motion> {if {$zb_active} {MoveZoombox %x %y}}
bind .r <Enter> {
    if {$zb_active} {
	UpdatePosition %x %y
    }
}
bind .r <Any-Return> {if {$zb_active} {ZoomIn}}
bind .r <Any-Double-1> {if {$zb_active} {ZoomIn}}
bind .r <Any-Control-Return> {if {$zb_active} {ZoomOut}}
bind .r <Any-Double-Button-3> {if {$zb_active} {ZoomOut}}

proc UpdatePosition {x y} {
    global mousex mousey
    set mousex $x
    set mousey $y
}

set zb_active 0
proc ToggleZoombox {x y} {
    global mousex mousey zb_active compute
    set zb_active [expr !$zb_active]
    if {$zb_active} {
	set compute 0
	.i.bot.go configure -state disabled
	.r zoombox 0 0 [expr [Width .r] -1] [expr [Height .r] -1]
	focus .r
    } else {
	.i.bot.go configure -state normal
	.r zoombox 0 0 0 0
    }
    set mousex $x
    set mousey $y
}

proc ResizeZoombox {x y} {
    global mousex mousey zb_active
    set x1 [lindex [.r zoombox] 0]
    set x2 [lindex [.r zoombox] 2]
    set y1 [lindex [.r zoombox] 1]
    set y2 [lindex [.r zoombox] 3]
    set w [expr $x2-$x1+1]
    set h [expr $y2-$y1+1]
    set stepx [expr [Width .r]/20]
    set stepy [expr [Height .r]/20]
    if {$y > $mousey} {
	# make zoombox bigger
	if {$h<[Height .r]} {
	    set halfw [expr $w/2+$stepx]
	    set x1 [expr $x-$halfw]
	    set x2 [expr $x+$halfw]
	    set halfh [expr $h/2+$stepy]
	    set y1 [expr $y-$halfh]
	    set y2 [expr $y+$halfh]
	    .r zoombox $x1 $y1 $x2 $y2
	} else {
	    MoveZoombox $x $y
	}
    } else {
	# make zoombox smaller
	if {$h>[expr $stepy*3]} {
	    set halfw [expr $w/2-$stepx]
	    set x1 [expr $x-$halfw]
	    set x2 [expr $x+$halfw]
	    set halfh [expr $h/2-$stepy]
	    set y1 [expr $y-$halfh]
	    set y2 [expr $y+$halfh]
	    .r zoombox $x1 $y1 $x2 $y2
	} else {
	    MoveZoombox $x $y
	}
    }
    set mousex $x
    set mousey $y
}

proc MoveZoombox {x y} {
    global mousex mousey zb_active
    set x1 [lindex [.r zoombox] 0]
    set x2 [lindex [.r zoombox] 2]
    set y1 [lindex [.r zoombox] 1]
    set y2 [lindex [.r zoombox] 3]
    set halfw [expr ($x2-$x1+1)/2]
    set halfh [expr ($y2-$y1+1)/2]
    set x1 [expr $x-$halfw]
    set x2 [expr $x+$halfw]
    set y1 [expr $y-$halfh]
    set y2 [expr $y+$halfh]
    .r zoombox $x1 $y1 $x2 $y2
    set mousex $x
    set mousey $y
}

proc ZoomIn {} {
    global zb_active minr mini maxr maxi compute
    set x1 [lindex [.r zoombox] 0]
    set x2 [lindex [.r zoombox] 2]
    set y1 [lindex [.r zoombox] 1]
    set y2 [lindex [.r zoombox] 3]
    set stepx [expr ($maxr-$minr)/[Width .r]]
    set stepy [expr ($maxi-$mini)/[Height .r]]
    set minr [expr $x1*$stepx+$minr]
    set mini [expr $y1*$stepy+$mini]
    set maxr [expr ($x2-[Width .r]+1)*$stepx+$maxr]
    set maxi [expr ($y2-[Height .r]+1)*$stepy+$maxi]
    if {$zb_active} {
	ToggleZoombox 0 0
    }
    .i.bot.go configure -state normal
    set compute 1
}

proc ZoomOut {} {
    global zb_active minr mini maxr maxi compute
    set x1 [lindex [.r zoombox] 0]
    set x2 [lindex [.r zoombox] 2]
    set y1 [lindex [.r zoombox] 1]
    set y2 [lindex [.r zoombox] 3]
    set stepx [expr ($maxr-$minr)/($x2-$x1)]
    set stepy [expr ($maxi-$mini)/($y2-$y1)]
    set minr [expr $minr-$x1*$stepx]
    set maxr [expr $maxr+([Width .r]+1-$x2)*$stepx]
    set maxi [expr $maxi+([Height .r]+1-$y2)*$stepy]
    AdjustMini
    if {$zb_active} {
	ToggleZoombox 0 0
    }
    .i.bot.go configure -state normal
    set compute 1
}

proc Resize {win w h} {
    if { ($w != [Width $win]) || ($h != [Height $win]) } {
	$win configure -width $w -height $h
	return 1
    }
    return 0
}
    
wm minsize . 1 1
bind . <Configure> {
    global zb_active
    if {$zb_active} {
	ToggleZoombox 0 0
    }
    if {[Resize .r %w %h]} {
	global compute
	set compute 0
	AdjustMini
	update idletasks
	after 1000 {.i.bot.go invoke}
    }
}
