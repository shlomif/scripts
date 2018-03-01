if {[llength $argv] < 1} {
    die {Usage: dnstw [-F | -d domain] [-n] make-record host ..}
}
set host [lindex $argv 0]

allow_underscore_hosts 
audit_hostnames host

shift argv
set record [join $argv " "]
# this may even need to be the "print" character class depending on what
# a site wants to put into e.g. TXT records (though allowing $ or [] may
# allow unexpected TCL variable or proc call activity)
if {[regexp {[^A-Za-z0-9=:?/_ ."-]} $record]} {
    die "invalid data in DNS record"
}

set nsupdate [ string cat $nsupdate \
    "add $host.$domain $TTL $record\n" \
    "send\n" ]
