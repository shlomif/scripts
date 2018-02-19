if {[llength $argv] < 2} {
    die {Usage: dnstw [-F | -d domain] [-n] create-host host ip [ip ..]}
}
set host [lindex $argv 0]

audit_hostnames host

shift argv
foreach arg $argv {
    set type A
    if { [catch {exec -- v4addr -aq $arg} output] } {
        set type AAAA
        if { [catch {exec -- v6addr -aq $arg} output] } {
            die "unable to parse ip address: $arg"
        }
    }
    set ret [split $output "\n"]
    set ipaddr [lindex $ret 0]
    set reverse [lindex $ret 1]
    # distict "send" used here to avoid a vague "NOZONE" error
    set nsupdate [ string cat $nsupdate \
        "add $host.$domain $TTL $type $ipaddr\n" \
        "send\n" \
        "nxrrset $reverse PTR\n" \
        "add $reverse $TTL PTR $host.$domain\n" \
        "send\n" ]
}
