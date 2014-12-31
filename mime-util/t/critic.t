#!/usr/bin/env perl
#
# Test with Perl::Critic.

use strict;
use warnings;
use Cwd qw(getcwd);
use Test::Perl::Critic qw(all_critic_ok);

my $cwd = getcwd;
if ( $cwd =~ '/t$' ) {
  chdir '..' or die "error: could not chdir: $!\n";
}

all_critic_ok('.');
