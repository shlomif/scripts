if {[llength $argv] != 2} {
    die {Usage: dnstw [-F | -d domain] [-n] repoint-cname host cname}
}
set host  [lindex $argv 0]
set cname [lindex $argv 1]

if {$host eq $cname} { die "cname must be different than host" }

audit_hostnames host cname

set nsupdate [ string cat $nsupdate \
    "yxrrset $cname.$domain CNAME\n" \
    "yxdomain $host.$domain\n" \
    "del $cname.$domain CNAME\n" \
    "add $cname.$domain $TTL CNAME $host.$domain\n" \
    "send\n" ]
