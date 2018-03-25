if {[llength $argv] < 1 || $accept_fqdn} {
    die {Usage: dnstw [-d domain] [-n] repoint-domain ip [ip ...]}
}

reject_invalid_domain

# NOTE if only A records modified the AAAA (if any) will be left
# unmolested. another way would be to instead wipe out all records for
# the domain and set what the user supplies...
set additions ""
set seentypes [dict create]

foreach arg $argv {
    ipparse $arg ipaddr reverse type
    dict set seentypes $type 1
    set additions [ string cat $additions "add $domain $TTL $type $ipaddr\n" ]
}

dict for {ttt unused} $seentypes {
    set nsupdate [ string cat $nsupdate "del $domain $ttt\n" ]
}

set nsupdate [ string cat $nsupdate $additions "send\n" ]
