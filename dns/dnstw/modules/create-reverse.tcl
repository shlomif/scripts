if {[llength $argv] % 2 != 0} {
    die {Usage: dnstw [-F | -d domain] [-n] create-reverse host ip [host2 ip2 [..]]}
}

foreach {host ip} $argv {
    audit_hostnames host

    ipparse $ip ipaddr reverse type

    set nsupdate [ string cat $nsupdate \
        "nxrrset $reverse PTR\n" \
        "add $reverse $TTL PTR $host.$domain\n" \
        "send\n" ]
}
