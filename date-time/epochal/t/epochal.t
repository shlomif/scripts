#!perl
use lib qw(../../lib/perl5);
use UtilityTestBelt;
use POSIX qw(strftime);

my $cmd = Test::UnixCmdWrap->new;

$ENV{TZ} = 'UTC';

my $utc_year = strftime('%Y', gmtime);

$cmd->run(
    args => q{-f '%b %d %H:%M:%S' -Y 2016 t/messages},
    stdout =>
      [ '1481827952 host Dec 15 18:52:32', '1481827952 host', '1481849429' ],
);
$cmd->run(
    args   => q{-f '%b %d %H:%M:%S' -Y 2016 -g t/messages},
    stdout => [ '1481827952 host 1481827952', '1481827952 host', '1481849429' ],
);
$cmd->run(
    args   => q{-f '%H:%M:%S' -o '%H:%M' -},
    env    => { TZ => 'US/Pacific' },
    stdin  => '18:40:22',
    stdout => ['18:40'],
);
$cmd->run(
    args   => q{-f '%b %d %H:%M:%S' -o '%Y' -s -y t/messages},
    stdout => [ $utc_year, $utc_year, $utc_year ],
);
$cmd->run(args => '-h', status => 64, stderr => qr/Usage/);

# and these following tests because wacky Kerberos timestamps put the
# year after the timezone (and, formerly, segfaults on OpenBSD)

my $dates = <<'EOD';
Expiration date: Tue Jan 20 22:03:50 UTC 2015
EOD

my $dates_epoch      = 'Expiration date: 1421791430';
my $dates_only_epoch = '1421791430';

$cmd->run(
    args   => q{-f '%a %b %d %H:%M:%S %Z %Y'},
    stdin  => $dates,
    stdout => [$dates_epoch]
);

$cmd->run(
    args   => q{-s -f '%a %b %d %H:%M:%S %Z %Y'},
    stdin  => $dates,
    stdout => [$dates_only_epoch]
);

done_testing(21);
