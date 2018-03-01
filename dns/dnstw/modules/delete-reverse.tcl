if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] delete-reverse ip [ip2 ..]}
}

foreach arg $argv {
    ipparse $arg ipaddr reverse type

    set nsupdate [ string cat $nsupdate \
        "yxrrset $reverse PTR\n" \
        "del $reverse $reverse PTR\n" \
        "send\n" ]
}
