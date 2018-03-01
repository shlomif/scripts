if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] delete-host host [ip ..]}
}
set host [lindex $argv 0]

audit_hostnames host

# guard on NS is to avoid accidentally deleting a name server; use
# unmake-record or nscat if you really do need to delete a NS (or
# remove this guard)
set nsupdate [ string cat $nsupdate \
    "yxdomain $host.$domain\n" \
    "nxrrset $host.$domain NS\n" \
    "del $host.$domain\n" \
    "send\n" ]

shift argv
foreach arg $argv {
    ipparse $arg ipaddr reverse type

    set nsupdate [ string cat $nsupdate \
        "yxrrset $reverse PTR\n" \
        "del $reverse PTR\n" \
        "send\n" ]
}
