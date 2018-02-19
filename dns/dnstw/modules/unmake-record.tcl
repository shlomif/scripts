if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] unmake-record host ..}
}
set host [lindex $argv 0]

if {[regexp {[^A-Za-z0-9_ .-]} $host]} {
    die "invalid data in host record"
}

shift argv
if {[regexp {[^A-Za-z0-9_ .-]} $argv]} {
    die "invalid data in DNS record"
}

set nsupdate [ string cat $nsupdate \
    "del $host.$domain $TTL $argv\n" \
    "send\n" ]
