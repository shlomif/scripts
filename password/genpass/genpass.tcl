#!/usr/bin/env expect

package require Tcl 8.5

set code2char {a b c d e f g h i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z 0 1 2 3 4 5 6 7 8 9 + /}
set c2clen [llength $code2char]

proc genpass {length} {
    global code2char c2clen
    set fh [open /dev/urandom r]
    binary scan [read $fh $length] c* codes
    foreach c $codes {
        append pass [lindex $code2char [expr {$c % $c2clen}]]
    }
    close $fh
    if {[string length $pass] < $length} {
        error "insufficient entropy for password??"
    }
    return $pass
}

puts [genpass 8]
