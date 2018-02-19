# for testing via MacPorts installed named
set nsupdate_cmd /opt/local/bin/nsupdate
# NOTE "-t 0" per nsupdate(1) says it should "disable the timeout" but
# instead with bind 9.12 a "dns_request_createvia3: out of range"
# message appears before nsupdate fails
set nsupdate_args {-k /opt/local/etc/nsuk.key -t 60}

set domain example.net.
set server 127.0.0.1

set default_mx_priority 10

set TTL_MIN 60
set TTL_MAX 86400

########################################################################
#
# utility routines

proc die {{msg ""}} { if {$msg ne ""} { warn $msg }; exit 1 }

proc shift {list} {
    upvar 1 $list ll
    set ll [lassign $ll res]
    return $res
}

proc warn {msg} { puts stderr $msg }

########################################################################
#
# business logic

proc audit_hostnames {args} {
    global accept_fqdn
    if {$accept_fqdn} {
        reject_invalid_fqhost {*}$args
    } else {
        reject_invalid_subhost {*}$args
    }
}

proc positive_int_or {name default} {
    upvar 1 $name value
    if {$value eq ""} {
        set value $default
    } else {
        if {![string is integer $value] || $value < 0} {
            die "$name must be a positive integer"
        }
    }
}

# [RFC 1035] section 2.3.4 and [RFC 1123] section 2.1 for hosts (which
# cannot have _ unlike SRV records). i18n data must already be in
# punycode form [RFC 5891]

# fully qualified name such as "foo.bar.example.net."
# expects to be called via audit_hostnames
proc reject_invalid_fqhost {args} {
    foreach name $args {
        upvar 2 $name value
        if {[string length $value] > 253} {
            die "$name cannot be longer than 253 characters"
        }
        if {[string index $value end] ne "."} {
            die "$name must end with a ."
        }
        # NOTE the trailing . is removed here for both the label split
        # and for when building $nsupdate
        set value [string range $value 0 end-1]
        foreach label [split $value "."] {
            if {[string length $label] == 0} {
                die "$name labels must have some length"
            }
            reject_invalid_host_label $name $label
        }
    }
}

# sub-host fragment such as "zot" or "foo.bar" under the $domain
# (is there an official term for these?)
# expects to be called via audit_hostnames
proc reject_invalid_subhost {args} {
    global domain
    foreach name $args {
        upvar 2 $name value
        if {[string length $value] + 1 + [string length $domain] > 253} {
            die "$name cannot be longer than 253 characters"
        }
        if {[string index $value end] eq "."} {
            die "$name must not end with a ."
        }
        foreach label [split $value "."] {
            if {[string length $label] == 0} {
                die "$name labels must have some length"
            }
            reject_invalid_host_label $name $label
        }
    }
}

# a hostname label such as "bar" of "foo.bar.example.net"
proc reject_invalid_host_label {name value} {
    if {[string length $value] > 63} {
        die "$name label cannot be longer than 63 characters"
    }
    if {![regexp -nocase {^([a-z0-9]+|[a-z0-9][a-z0-9-]+[a-z0-9])$} $value]} {
        die "$name label may only contain /a-z0-9/i or with hyphen in middle"
    }
}
