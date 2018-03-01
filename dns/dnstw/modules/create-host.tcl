if {[llength $argv] < 2} {
    die {Usage: dnstw [-F | -d domain] [-n] create-host host ip [ip ..]}
}
set host [lindex $argv 0]

audit_hostnames host

shift argv
foreach arg $argv {
    ipparse $arg ipaddr reverse type

    # distict "send" used here to avoid a vague "NOZONE" error
    set nsupdate [ string cat $nsupdate \
        "add $host.$domain $TTL $type $ipaddr\n" \
        "send\n" \
        "nxrrset $reverse PTR\n" \
        "add $reverse $TTL PTR $host.$domain\n" \
        "send\n" ]
}
