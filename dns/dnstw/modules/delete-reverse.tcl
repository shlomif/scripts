if {[llength $argv] != 1} {
    die {Usage: dnstw [-F | -d domain] [-n] delete-reverse ip}
}
set ip [lindex $argv 0]

if { [catch {exec -- v4addr -aq $ip} output] } {
    if { [catch {exec -- v6addr -aq $ip} output] } {
        die "unable to parse ip address: $ip"
    }
}
set reverse [lindex [split $output "\n"] 1]

set nsupdate [ string cat $nsupdate \
    "yxrrset $reverse PTR\n" \
    "del $reverse $reverse PTR\n" \
    "send\n" ]
