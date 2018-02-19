if {[llength $argv] != 2} {
    die {Usage: dnstw [-F | -d domain] [-n] create-reverse host ip}
}
set host [lindex $argv 0]
set ip [lindex $argv 1]

audit_hostnames host

if { [catch {exec -- v4addr -aq $ip} output] } {
    if { [catch {exec -- v6addr -aq $ip} output] } {
        die "unable to parse ip address: $ip"
    }
}
set reverse [lindex [split $output "\n"] 1]

set nsupdate [ string cat $nsupdate \
    "nxrrset $reverse PTR\n" \
    "add $reverse $TTL PTR $host.$domain\n" \
    "send\n" ]
