#!/usr/bin/tclsh
# See COPYING file for copyright and license details.
package require Tk

set bin [list getgbook getabook getbnbook]

proc updateStatus {chan} {
	if {![eof $chan]} {
		set a [gets $chan]
		if { $a != "" } { .st configure -text $a }
	} else {
		close $chan
		.dl configure -state normal -text "download"
		.st configure -text ""
	}
}

proc go {} {
	if { [.id get] == "" } { return }
	set cmd "[.bin get [.bin curselection]] [.id get]"
	.dl configure -state disabled -text "downloading"
	update
	set out [open "|$cmd 2>@1" "r"]
	fileevent $out readable [list updateStatus $out]
}

label .lab -text "book id"
entry .id
listbox .bin -listvariable bin -exportselection 0
.bin selection set 0
button .dl -text "download" -command go
label .st -relief sunken -width 20

pack .lab .id .bin .dl .st
bind . <Return> go
