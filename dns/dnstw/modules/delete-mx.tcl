set argc [llength $argv]
if {$argc < 1 || $argc % 2 == 0} {
    die {Usage: dnstw [-F | -d domain] [-n] delete-mx host [pri host [pri2 host2 ..]]}
}
set host [lindex $argv 0]

audit_hostnames host

if {$argc == 1} {
    append nsupdate \
      "yxrrset $host.$domain MX\n" \
      "del $host.$domain MX\n" \
      "send\n"
} else {
    shift argv
    foreach {priority mxhost} $argv {
        audit_hostnames mxhost
        append nsupdate \
          "yxrrset $host.$domain MX $priority $mxhost\n" \
          "del $host.$domain MX $priority $mxhost\n"
    }
    append nsupdate "send\n"
}
