if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] unmake-record host ..}
}
set host [lindex $argv 0]

allow_underscore_hosts 
audit_hostnames host

shift argv
set record [join $argv " "]
if {[regexp {[^A-Za-z0-9=:?/_ ."-]} $record]} {
    die "invalid data in DNS record"
}

set nsupdate [ string cat $nsupdate \
    "del $host.$domain $TTL $record\n" \
    "send\n" ]
