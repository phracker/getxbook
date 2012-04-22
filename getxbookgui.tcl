#!/usr/bin/tclsh
# See COPYING file for copyright and license details.
package require Tk

set bins {{getgbook "Google Book ID" "Google\nBook Preview" \
           "http*://books.google.com/*" {[&?]id=([^&]*)}} \
          {getabook "ISBN 10" "Amazon Look\nInside The Book" \
           "http*://*amazon*/*" {/([0-9]{10}}} \
          {getbnbook "ISBN 13" "Barnes & Noble\nBook Viewer" \
           "http*://www.barnesandnoble.com/*" ""}}
set binselected 0
set dling 0
set manual 0

set env(PATH) "[file dirname $::argv0]:$env(PATH)"
set iconpath "[file dirname $::argv0]/icons"

proc updateStatus {chan} {
	global dling
	if {![eof $chan]} {
		set a [gets $chan]
		if { $a != "" } { .st configure -text $a }
	} else {
		if { ! [catch {close $chan}] } {
			.st configure -text "[.input.id get] Done"
		}
		.dl configure -state normal -text "Download"
		set dling 0
	}
}

proc go {} {
	global dling binselected bins
	if { [.input.id get] == "" } { return }
	set cmd "[lindex [lindex $bins $binselected] 0] [.input.id get]"
	set dling 1
	.dl configure -state disabled -text "Downloading"
	.st configure -text ""
	set out [open "|$cmd 2>@1" "r"]
	fileevent $out readable [list updateStatus $out]
}

proc parseurl {url} {
	global bins
	for {set i 0} {$i < [llength $bins]} {incr i} {
		set b [lindex $bins $i]
		if { [string match [lindex $b 3] "$url"] } {
			selbin $i
			set binregex [lindex $b 4]
			if {"$binregex" != "" && [regexp "$binregex" $url m sub]} {
				.input.id delete 0 end
				.input.id insert 0 "$sub"
			}
		}
	}
}

proc watchsel {} {
	global dling manual

	if { !$dling && !$manual && \
	     ! [catch {set sel "[clipboard get -type UTF8_STRING]"}] } {
		parseurl "$sel"
	}

	after 500 watchsel
}

proc selbin {sel} {
	global bins binselected

	.binfr.$binselected configure -relief flat
	set binselected $sel
	.binfr.$binselected configure -relief solid

	.input.lab configure -text [lindex [lindex $bins $sel] 1]
}

frame .input
label .input.lab
entry .input.id -width 14

frame .binfr
for {set i 0} {$i < [llength $bins]} {incr i} {
	set b [lindex $bins $i]
	set binname [lindex $b 0]
	if { [catch {image create photo im$i -file "$iconpath/$binname.gif"}] } {
		image create photo im$i
	}
	button .binfr.$i -text [lindex $b 2] -image im$i \
	       -command "selbin $i" -compound top -relief flat
	pack .binfr.$i -side left
	bind .binfr.$i <Key> {set manual 1}
	bind .binfr.$i <Button> {set manual 1}
}
.binfr.$binselected invoke

button .dl -text "Download" -command go
label .st

pack .input.lab -side left
pack .input.id

pack .binfr .input .dl .st
bind . <Return> go

bind .input.id <Key> {set manual 1}
watchsel
