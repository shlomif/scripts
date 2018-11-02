if {[llength $argv] != 2} {
    die {Usage: dnstw [-F | -d domain] [-n] repoint-cname host cname}
}
set host  [lindex $argv 0]
set cname [lindex $argv 1]

if {$host eq $cname} { die "cname must be different than host" }

audit_hostnames host cname

# folks forget to do this at $work when the www CNAME changes; see also
# repoint-domain. but this strays into business logic territory
if {$cname eq "www"} {
    puts stderr "info: be sure to update A/AAAA for the domain if necessary"
}

append nsupdate \
  "yxrrset $cname.$domain CNAME\n" \
  "yxdomain $host.$domain\n" \
  "del $cname.$domain CNAME\n" \
  "add $cname.$domain $TTL CNAME $host.$domain\n" \
  "send\n"
