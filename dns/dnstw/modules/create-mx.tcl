set argc [llength $argv]
if {$argc < 2 || $argc > 3} {
    die {Usage: dnstw [-F | -d domain] [-n] create-mx host [ mx | priority mx ]}
}
set host [lindex $argv 0]
if {$argc == 2} {
    set priority $default_mx_priority
    set mxhost [lindex $argv 1]
} else {
    set priority [lindex $argv 1]
    positive_int_or priority $default_mx_priority
    set mxhost [lindex $argv 2]
}

audit_hostnames host mxhost

set nsupdate [ string cat $nsupdate \
    "yxdomain $host.$domain\n" \
    "yxdomain $mxhost.$domain\n" \
    "add $host.$domain $TTL MX $priority $mxhost.$domain\n" \
    "send\n" ]
