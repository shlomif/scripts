#!/usr/bin/env expect

package require Tcl 8.5

puts 1..1

set fh [open "| ./stardate" r]
set want [clock format [clock seconds] -format %Q]
set got  [read -nonewline $fh]
close $fh

if {$want eq $got} {
    puts "ok 1 - stardate from C same as for expect"
} else {
    puts "not ok 1 - stardate $want ne $got"
}
