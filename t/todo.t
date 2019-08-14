#!perl
use lib qw(lib/perl5);
use UtilityTestBelt;

my $cmd = Test::UnixCmdWrap->new;

# either of these is supported though VISUAL has priority so both must
# be gone for the tests
delete @ENV{ my @editor = qw(EDITOR VISUAL) };

# check if HOME has been corrupted (which the m4 inserts into the code)
# by comparing it against the password database. the macro definition is
# within a divert(-1) block to help prevent shennanigans should HOME
# contain something that is not only a home directory
my $home = ( getpwuid $> )[7];

foreach (@editor) {
    $cmd->run( stdout => qr(^$home/todo$), env => { $_ => 'echo' } );
}

$cmd->run( env => { VISUAL => 'false' }, status => 1 );

done_testing(9);
