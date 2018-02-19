if {[llength $argv] != 1} {
    die {Usage: dnstw [-F | -d domain] [-n] delete-cname cname}
}
set cname [lindex $argv 0]

audit_hostnames cname

set nsupdate [ string cat $nsupdate \
    "yxrrset $cname.$domain CNAME\n" \
    "del $cname.$domain CNAME\n" \
    "send\n" ]
