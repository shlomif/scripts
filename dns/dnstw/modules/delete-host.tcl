if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] delete-host host [ip ..]}
}
set host [lindex $argv 0]

audit_hostnames host

# guard on NS is to avoid accidentally deleting a name server
set nsupdate [ string cat $nsupdate \
    "yxdomain $host.$domain\n" \
    "nxrrset $host.$domain NS\n" \
    "del $host.$domain\n" \
    "send\n" ]

shift argv
foreach arg $argv {
    if { [catch {exec -- v4addr -aq $arg} output] } {
        if { [catch {exec -- v6addr -aq $arg} output] } {
            die "unable to parse ip address: $arg"
        }
    }
    set reverse [lindex [split $output "\n"] 1]
    set nsupdate [ string cat $nsupdate \
        "yxrrset $reverse PTR\n" \
        "del $reverse PTR\n" \
        "send\n" ]
}
