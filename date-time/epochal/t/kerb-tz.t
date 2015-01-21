#!perl
#
# Those wacky Kerberos timestamps, the ones that cause segfaults on OpenBSD
# that hopefully will be fixed in OpenBSD 5.7, or you can patch strptime.c of
# libc fame.

use strict;
use warnings;

use Test::Cmd;
use Test::More tests => 4;

$ENV{TZ} = 'UTC';

my $dates = <<'EOD';
Expiration date: Tue Jan 20 22:03:50 UTC 2015
EOD

my $dates_epoch = <<'EODE';
Expiration date: 1421791430
EODE

my $dates_only_epoch = <<'EODE';
1421791430
EODE

my $test = Test::Cmd->new(
  prog    => q{epochal -f '%a %b %d %H:%M:%S %Z %Y'},
  workdir => '',
);

$test->run( stdin => $dates );

is( $test->stdout, $dates_epoch, 'dates converted to epoch' );
is( $? >> 8,       0,            'exit status ok' );

$test = Test::Cmd->new(
  prog    => q{epochal -s -f '%a %b %d %H:%M:%S %Z %Y'},
  workdir => '',
  stdin   => $dates
);

$test->run( stdin => $dates );

is( $test->stdout, $dates_only_epoch, 'dates reduced to epoch' );
is( $? >> 8, 0, 'exit status ok' );
