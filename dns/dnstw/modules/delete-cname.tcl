if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] delete-cname cname [cname2 ..]}
}

foreach cname $argv {
    audit_hostnames cname

    set nsupdate [ string cat $nsupdate \
        "yxrrset $cname.$domain CNAME\n" \
        "del $cname.$domain CNAME\n" \
        "send\n" ]
}
