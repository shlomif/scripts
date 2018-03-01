if {[llength $argv] != 2} {
    die {Usage: dnstw [-F | -d domain] [-n] create-cname host cname}
}
set host  [lindex $argv 0]
set cname [lindex $argv 1]

if {$host eq $cname} { die "cname must be different than host" }

audit_hostnames host cname

# NOTE the nxdomain might be problematical if there are DNSSEC records
# already there as that's a notable exception to the "no other records
# for CNAME" rule (or if a site breaks with said rule for some reason)
set nsupdate [ string cat $nsupdate \
    "yxdomain $host.$domain\n" \
    "nxdomain $cname.$domain\n" \
    "add $cname.$domain $TTL CNAME $host.$domain\n" \
    "send\n" ]
