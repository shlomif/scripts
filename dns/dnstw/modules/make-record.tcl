if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] make-record host ..}
}
set host [lindex $argv 0]

# cannot use audit_hostnames as that does not permit _ which could be
# required for _VLMCS._tcp or other such records
if {[regexp {[^A-Za-z0-9_ .-]} $host]} {
    die "invalid data in host record"
}

shift argv
if {[regexp {[^A-Za-z0-9_ .-]} $argv]} {
    die "invalid data in DNS record"
}

set nsupdate [ string cat $nsupdate \
    "add $host.$domain $TTL $argv\n" \
    "send\n" ]
