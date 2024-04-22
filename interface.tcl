# 
# interface.tcl --
#
#      TCL code related to user interface management.
#
# Copyright (c) 1994-1997 Michael R. Ganss. All Rights Reserved.
#
# See section "Copyright" of the on-line help for information on usage and
# redistribution of this file, and for a DISCLAIMER OF ALL WARRANTIES.
#

scan $tk_version "%d.%d" major_version minor_version
set tcl_prompt1 "puts -nonewline almond>"
set tcl_precision 17
toplevel .i
tk_focusFollowsMouse
wm resizable .i 0 0
wm title .i "AlmondBread"
wm protocol .i WM_DELETE_WINDOW {Tschuess}
proc Tschuess {} {
    global compute
    set compute 0
    update
    after 500 {destroy .}
}

image create photo icon -file $abdir/almond.ppm
toplevel .icon
wm withdraw .icon
label .icon.l -image icon -borderwidth 0
pack .icon.l
wm deiconify .icon
wm iconwindow . .icon
wm iconname . "Image"

toplevel .icon2
wm withdraw .icon2
label .icon2.l -image icon -borderwidth 0
pack .icon2.l
wm deiconify .icon2
wm iconwindow .i .icon2

frame .i.menu -relief raised -borderwidth 2
menubutton .i.menu.file -text "File" -menu .i.menu.file.m -underline 0
if {($major_version == 4) && ($minor_version < 1)} {
    menu .i.menu.file.m
} else {
    menu .i.menu.file.m -transient 1
}
.i.menu.file.m add command -label "Load..." -command {
    global status compute zb_active
    set compute 0
    if {$zb_active} {
	ToggleZoombox 0 0
    }
    set filename [j:fs -title "Load Image" -prompt "Select image file to load:"]
    if { $filename != "" } {
	set status "Loading $filename..."
	update idletasks
	after 2000 { set status "" }
	.r loadgif $filename
	set status "Done."
	after 2000 { set status "" }
    }
    if {$showcmap} {
	.cmap.r colormap
	ShowColormap .cmap.r
    }
} -underline 0

.i.menu.file.m add command -label "Save..." -command {
    global status compute zb_active
    set compute 0
    if {$zb_active} {
	ToggleZoombox 0 0
    }
    set filename [j:fs -title "Save Image" -prompt "Save as:"]
    if { $filename != "" } {
	set status "Saving $filename..."
	update idletasks
	after 2000 { set status "" }
	.r savegif $filename
	set status "Done."
	after 2000 { set status "" }
    }
} -underline 0

.i.menu.file.m add separator
.i.menu.file.m add command -label "Quit" -command {Tschuess} -underline 0

menubutton .i.menu.colormap -text "Colormap" -menu .i.menu.colormap.m -underline 0
menu .i.menu.colormap.m
.i.menu.colormap.m add checkbutton -label "Show Colormap" -command {
    ToggleShowColormap
} -underline 0
set visual [winfo visual .]
if {$visual != "truecolor" && $visual != "directcolor"} {
    .i.menu.colormap.m add checkbutton -label "Own Colormap" -underline 0 -command {
	global owncmap status zb_active
	if {$zb_active} {
	    ToggleZoombox 0 0
	}
	if {$owncmap == "no"} {
	    set status "Allocating own colormap..."
	    update idletasks
	    set owncmap yes
	} else {
	    set status "Allocating shared colormap..."
	    update idletasks
	    set owncmap no
	}
	update idletasks
	set status "Done."
	after 2000 { set status "" }
    }
}
.i.menu.colormap.m add separator
.i.menu.colormap.m add command -label "Load..." -command {
    global zb_active
    if {$zb_active} {
	ToggleZoombox 0 0
    }
    set filename [j:fs -title "Load Colormap" -prompt "Select colormap file to load:"]
    if { $filename != "" } {
	global status
	set status "Loading colormap $filename..."
	update idletasks
	LoadColormap $filename .r
	set status "Done."
	after 2000 { set status "" }
	if {$showcmap} {
	    .cmap.r colormap
	    ShowColormap .cmap.r
	}
    }
} -underline 0

menubutton .i.menu.drawing -text "Drawing" -menu .i.menu.drawing.m -underline 0
menu .i.menu.drawing.m

.i.menu.drawing.m add radiobutton -variable method -label "Scan" -value 0 -underline 1
.i.menu.drawing.m add radiobutton -variable method -label "Interleave" -value 1 -underline 0
.i.menu.drawing.m add radiobutton -variable method -label "Tesseral" -value 2 -underline 0
.i.menu.drawing.m add radiobutton -variable method -label "Boundary Trace" -value 3 -underline 0
.i.menu.drawing.m add radiobutton -variable method -label "SOI" -value 4 -underline 0
.i.menu.drawing.m add separator

set drawopts 0
set drawprev 0
.i.menu.drawing.m add command -label "Options..." -command {
    global drawopts drawprev
    if {!$drawopts} {
	toplevel .drawopts
	wm title .drawopts "Drawing Method Options"
	wm resizable .drawopts 0 0
	wm protocol .drawopts WM_DELETE_WINDOW {
	    destroy .drawopts
	    set drawopts 0
	}
	wm group .drawopts .i
	
	button .drawopts.done -text "Done" -command {
	    destroy .drawopts
	    set drawopts 0
	}
	pack .drawopts.done -side bottom -anchor center

	DrawPackOptions $drawprev
	set drawopts 1
	update idletasks
	bind .drawopts <Any-Button-3> {
	    destroy .drawopts
	    set drawopts 0
	}
    }
} -underline 0
 
proc DrawTrace {n e o} {
    global drawprev drawopts
    upvar #0 $n method
    if {$method == 0 || $method == 3} {
	if {$drawopts} {
	    destroy .drawopts
	    set drawopts 0
	}
	.i.menu.drawing.m entryconfigure last -state disabled
    } else {
	if {$drawopts} {
	    DrawPackOptions $drawprev
	}
	.i.menu.drawing.m entryconfigure last -state normal
    }
    set drawprev $method
}
trace variable method w DrawTrace

proc DrawPackOptions {prev} {
    global method drawopts
    if {!$drawopts} {
	frame .drawopts.interleave
	label .drawopts.interleave.label -text "Interleave"
	entry .drawopts.interleave.entry -relief sunken -textvariable interleave -width 20
	
	frame .drawopts.goscanat
	label .drawopts.goscanat.label -text "Scan At"
	entry .drawopts.goscanat.entry -relief sunken -textvariable goscanat -width 20
	
	frame .drawopts.minprogress
	label .drawopts.minprogress.label -text "Minimum Progress"
	entry .drawopts.minprogress.entry -relief sunken -textvariable minprogress -width 20

	frame .drawopts.tolerance
	label .drawopts.tolerance.label -text "Tolerance"
	entry .drawopts.tolerance.entry -relief sunken -textvariable tolerance -width 20
    } else {
	switch $prev {
	    0 {}
	    1 { pack forget .drawopts.interleave }
	    2 { pack forget .drawopts.goscanat .drawopts.interleave }
	    3 {}
	    4 { pack forget .drawopts.interleave .drawopts.goscanat .drawopts.minprogress .drawopts.tolerance }
	}
    }
    switch $method {
	0 {}
	1 { 
	    pack .drawopts.interleave -side top -fill x
	    pack .drawopts.interleave.label -side left -anchor w
	    pack .drawopts.interleave.entry -side right -anchor e
	}
	2 {
	    pack .drawopts.interleave -side top -fill x
	    pack .drawopts.interleave.label -side left -anchor w
	    pack .drawopts.interleave.entry -side right -anchor e

	    pack .drawopts.goscanat -side top -fill x
	    pack .drawopts.goscanat.label -side left -anchor w
	    pack .drawopts.goscanat.entry -side right -anchor e
	}
	3 {}
	4 {
	    pack .drawopts.interleave -side top -fill x
	    pack .drawopts.interleave.label -side left -anchor w
	    pack .drawopts.interleave.entry -side right -anchor e

	    pack .drawopts.goscanat -side top -fill x
	    pack .drawopts.goscanat.label -side left -anchor w
	    pack .drawopts.goscanat.entry -side right -anchor e
	    
	    pack .drawopts.minprogress -side top -fill x
	    pack .drawopts.minprogress.label -side left -anchor w
	    pack .drawopts.minprogress.entry -side right -anchor e

	    pack .drawopts.tolerance -side top -fill x
	    pack .drawopts.tolerance.label -side left -anchor w
	    pack .drawopts.tolerance.entry -side right -anchor e
	}
    }
}

menubutton .i.menu.iteration -text "Iteration" -menu .i.menu.iteration.m -underline 0
menu .i.menu.iteration.m

.i.menu.iteration.m add radiobutton -variable iterate -label "Conventional" -value 0 -underline 0
.i.menu.iteration.m add radiobutton -variable iterate -label "Optimized" -value 1 -underline 1
.i.menu.iteration.m add radiobutton -variable iterate -label "Unroll" -value 2 -underline 0
.i.menu.iteration.m add separator

set iteropts 0
set iterprev 0
.i.menu.iteration.m add command -label "Options..." -command {
    global iteropts iterprev
    if {!$iteropts} {
	toplevel .iteropts
	wm title .iteropts "Iteration Method Options"
	wm resizable .iteropts 0 0
	wm protocol .iteropts WM_DELETE_WINDOW {
	    destroy .iteropts
	    set iteropts 0
	}
	wm group .iteropts .i

	button .iteropts.done -text "Done" -command {
	    destroy .iteropts
	    set iteropts 0
	}
	
	frame .iteropts.bailout
	label .iteropts.bailout.label -text "Bailout"
	entry .iteropts.bailout.entry -width 20 -relief sunken -textvariable bailout
	pack .iteropts.bailout -side top -fill x
	pack .iteropts.bailout.label -side left -anchor w
	pack .iteropts.bailout.entry -side right -anchor e

	frame .iteropts.maxiter
	label .iteropts.maxiter.label -text "MaxIter"
	entry .iteropts.maxiter.entry -width 20 -relief sunken -textvariable maxiter
	pack .iteropts.maxiter -side top -fill x
	pack .iteropts.maxiter.label -side left -anchor w
	pack .iteropts.maxiter.entry -side right -anchor e
	pack .iteropts.done -side bottom -anchor center
	
	IterPackOptions $iterprev
	set iteropts 1
	update idletasks
	bind .iteropts <Any-Button-3> {
	    destroy .iteropts
	    set iteropts 0
	}
    }
} -underline 0
 
proc IterTrace {n e o} {
    global iterprev iteropts
    upvar #0 $n iterate
    if {$iteropts} {
	IterPackOptions $iterprev
    }
    set iterprev $iterate
}
trace variable iterate w IterTrace

proc IterPackOptions {prev} {
    global iterate iteropts
    if {!$iteropts} {
    } else {
	switch $prev {
	    0 {}
	    1 {}
	    2 {}
	}
    }
    switch $iterate {
	0 {}
	1 {}
	2 {}
    }
}
    
menubutton .i.menu.scheme -text "Coloring" -menu .i.menu.scheme.m -underline 1
menu .i.menu.scheme.m

.i.menu.scheme.m add radiobutton -variable scheme -label "Normal" -value 0 -underline 0
.i.menu.scheme.m add radiobutton -variable scheme -label "Potential" -value 1 -underline 0
.i.menu.scheme.m add radiobutton -variable scheme -label "Decomposition" -value 2 -underline 0
.i.menu.scheme.m add separator

set schemeopts 0
set schemeprev 0
.i.menu.scheme.m add command -label "Options..." -command {
    global schemeopts schemeprev
    if {!$schemeopts} {
	toplevel .schemeopts
	wm title .schemeopts "Coloring Scheme Options"
	wm resizable .schemeopts 0 0
	wm protocol .schemeopts WM_DELETE_WINDOW {
	    destroy .schemeopts
	    set schemeopts 0
	}
	wm group .schemeopts .i
	
	button .schemeopts.done -text "Done" -command {
	    destroy .schemeopts
	    set schemeopts 0
	}

	frame .schemeopts.inside
	label .schemeopts.inside.label -text "Inside Color"
	entry .schemeopts.inside.entry -width 20 -relief sunken -textvariable inside
	pack .schemeopts.inside -side top -fill x
	pack .schemeopts.inside.label -side left -anchor w
	pack .schemeopts.inside.entry -side right -anchor e
	pack .schemeopts.done -side bottom -anchor center
	
	SchemePackOptions $schemeprev
	set schemeopts 1
	update idletasks
	bind .schemeopts <Any-Button-3> {
	    destroy .schemeopts
	    set schemeopts 0
	}
    }
} -underline 0
 
proc SchemeTrace {n e o} {
    global schemeprev schemeopts
    upvar #0 $n scheme
    if {$schemeopts} {
	SchemePackOptions $schemeprev
    }
    set schemeprev $scheme
}
trace variable scheme w SchemeTrace

proc SchemePackOptions {prev} {
    global scheme schemeopts
    if {!$schemeopts} {
	frame .schemeopts.maxcolor
	label .schemeopts.maxcolor.label -text "MaxColor"
	entry .schemeopts.maxcolor.entry -relief sunken -textvariable maxcolor -width 20
	
	frame .schemeopts.slope
	label .schemeopts.slope.label -text "Slope"
	entry .schemeopts.slope.entry -relief sunken -textvariable slope -width 20
	
	frame .schemeopts.arity
	label .schemeopts.arity.label -text "Arity"
	entry .schemeopts.arity.entry -relief sunken -textvariable arity -width 20
    } else {
	switch $prev {
	    0 {}
	    1 { pack forget .schemeopts.maxcolor .schemeopts.slope }
	    2 { pack forget .schemeopts.arity }
	}
    }
    switch $scheme {
	0 {}
	1 { 
	    pack .schemeopts.maxcolor -side top -fill x
	    pack .schemeopts.maxcolor.label -side left -anchor w
	    pack .schemeopts.maxcolor.entry -side right -anchor e
	    
	    pack .schemeopts.slope -side top -fill x
	    pack .schemeopts.slope.label -side left -anchor w
	    pack .schemeopts.slope.entry -side right -anchor e
	}
	2 {
	    pack .schemeopts.arity -side top -fill x
	    pack .schemeopts.arity.label -side left -anchor w
	    pack .schemeopts.arity.entry -side right -anchor e
	}
    }
}

menubutton .i.menu.help -text "Help" -menu .i.menu.help.m -underline 0
menu .i.menu.help.m

.i.menu.help.m add command -label "Help" -command {
    global helpdir
    set cwd [pwd]
    cd $helpdir
    exec jdoc AlmondBread &
    cd $cwd
} -underline 0
.i.menu.help.m add command -label "About AlmondBread..." -command {about} -underline 0

pack .i.menu -side top -fill x
pack .i.menu.file -side left
pack .i.menu.colormap -side left
pack .i.menu.drawing -side left
pack .i.menu.iteration -side left
pack .i.menu.scheme -side left
pack .i.menu.help -side right

frame .i.bot

frame .i.location
frame .i.location.lr
label .i.location.lr.llabel -text "Left"
entry .i.location.lr.lentry -width 20 -relief sunken -textvariable minr
label .i.location.lr.rlabel -text "Right"
entry .i.location.lr.rentry -width 20 -relief sunken -textvariable maxr
frame .i.location.t
label .i.location.t.label -text "Top"
# this is not really an entry, but for conformity's sake
#label .i.location.t.entry -width 20 -relief sunken -textvariable top -bg bisque2 -anchor w -font -Adobe-Helvetica-Medium-R-Normal--*-120-* -padx 0 -pady 0
label .i.location.t.entry -width 20 -relief sunken -textvariable top -bg gray80
frame .i.location.b
label .i.location.b.label -text "Bottom"
entry .i.location.b.entry -width 20 -relief sunken -textvariable bottom

proc AdjustMini {args} {
    global mini maxi minr maxr top
    set dx [expr ($maxr-$minr)/[Width .r]]
    set mini [expr $maxi-([Height .r]*$dx)]
    trace vdelete mini w Mini2Top
    set top [expr -($mini)]
    trace variable mini w Mini2Top
}

proc MakeSymmetric {} {
    global mini maxi minr maxr
    set dx [expr ($maxr-$minr)/[Width .r]]
    set maxi [expr ([Height .r]*$dx)/2]
}    

trace variable top w Top2Mini
trace variable bottom w Bottom2Maxi
proc Top2Mini {n e o} {
    upvar #0 $n x
    global mini
    set mini [expr -($x)]
}
proc Bottom2Maxi {n e o} {
    upvar #0 $n x
    global maxi
    trace vdelete maxi w Maxi2Bottom
    set maxi [expr -($x)]
    trace variable maxi w Maxi2Bottom
    AdjustMini
}
trace variable maxi w Maxi2Bottom
trace variable mini w Mini2Top
proc Maxi2Bottom {n e o} {
    upvar #0 $n x
    global bottom
    trace vdelete bottom w Bottom2Maxi
    set bottom [expr -($x)]
    trace variable bottom w Bottom2Maxi
    AdjustMini
}
proc Mini2Top {n e o} {
    upvar #0 $n x
    global top
    set top [expr -($x)]
}
set maxi $maxi
set mini $mini
trace variable minr w AdjustMini
trace variable maxr w AdjustMini
set minr $minr
set maxr $maxr
MakeSymmetric

button .i.bot.go -text "Go" -relief groove -borderwidth 4 -foreground green -activeforeground green -font -Adobe-Helvetica-Bold-R-Normal--*-240-* -command {
    global compute
    set compute [expr !$compute]
}
bind Entry <Return> {
    global compute
    if {!$compute} {set compute 1}
}   
set tickcount 0
proc togglecompute {name el op} {
    upvar #0 $name compute
    global status tickcount
    if {$compute} {
	set status "Computing image..."
	.i.bot.go configure -text "Abort" -foreground red -activeforeground red
	.i.bot.time.digits configure -text "00:00:00"
	global seconds
	set seconds 0
	after 1000 { incr tickcount; tick $tickcount }
    } else {
	set status ""
	.i.bot.go configure -text "Go" -foreground green -activeforeground green
    }
}
trace variable compute w togglecompute
set compute $compute

proc tick n {
    global seconds compute tickcount
    if {(!$compute) || ($n != $tickcount)} return
    after 1000 [list tick $n]
    incr seconds
    set s [expr $seconds%60]
    set m [expr ($seconds/60)%60]
    set h [expr $seconds/3600]
    .i.bot.time.digits configure -text [format "%02d:%02d:%02d" $h $m $s]
}

pack .i.location -side top -padx 2m -pady 2m -fill both -expand 1
 
pack .i.location.t -side top -anchor center -fill x
pack .i.location.t.label -side top -anchor n
pack .i.location.t.entry -side bottom -anchor s

pack .i.location.lr -side top -anchor center
pack .i.location.lr.llabel -side left -anchor w
pack .i.location.lr.lentry -side left -anchor w

pack .i.location.lr.rlabel -side right -anchor e
pack .i.location.lr.rentry -side right -anchor e

pack .i.location.b -side bottom -anchor center -fill x
pack .i.location.b.label -side bottom -anchor s
pack .i.location.b.entry -side top -anchor n

frame .i.bot.time -relief groove -borderwidth 4
label .i.bot.time.digits -text "00:00:00" -font -Adobe-Helvetica-Bold-R-Normal--*-240-*
label .i.bot.time.text -text "Calculation Time"

.i.bot configure -height 64
pack .i.bot -side bottom -padx 2m -fill x -expand 1
place .i.bot.go -relx 0.0 -y 0 -relwidth 0.5 -relheight 1.0
place .i.bot.time -relx 0.5 -y 2 -relwidth 0.5 -relheight 1.0 -height -4
pack .i.bot.time.digits -side top -fill x
pack .i.bot.time.text -side bottom -fill x

label .i.status -anchor w -relief flat -padx 2m -pady 2m -textvariable status
pack .i.status -before .i.bot -side bottom -fill x

.i.bot.go invoke
