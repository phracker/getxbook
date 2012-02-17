#!/usr/bin/tclsh
# See COPYING file for copyright and license details.
package require Tk

set bin [list getgbook getabook getbnbook]
set binopts [list "book id" "isbn 10" "isbn 13"]

set env(PATH) "[file dirname $::argv0]:$env(PATH)"

proc updateStatus {chan} {
	if {![eof $chan]} {
		set a [gets $chan]
		if { $a != "" } { .st configure -text $a }
	} else {
		if { ! [catch {close $chan}] } {
			.st configure -text "[.top.id get] done"
		}
		.dl configure -state normal -text "download"
		.st configure -text ""
	}
}

proc showopts {} {
	global binopts
	.top.lab configure -text [lindex $binopts [.bin curselection]]
}

proc go {} {
	if { [.top.id get] == "" } { return }
	set cmd "[.bin get [.bin curselection]] [.top.id get]"
	.dl configure -state disabled -text "downloading"
	.st configure -text ""
	set out [open "|$cmd 2>@1" "r"]
	fileevent $out readable [list updateStatus $out]
}

frame .top
label .top.lab
entry .top.id -width 14
listbox .bin -listvariable bin -exportselection 0  -height [llength $bin]
bind .bin <<ListboxSelect>> {showopts}
.bin selection set 0
button .dl -text "download" -command go
label .st
showopts

pack .top .bin .dl .st
pack .top.lab -side left
pack .top.id
bind . <Return> go
