#!/usr/bin/env perl
#
# Test routines for SMIME operations in mime-util script.

use strict;
use warnings;
use Fatal qw(open);
use Test::More tests => 3;

if ( -d 't' ) {
  chdir 't' or die "error: could not chdir: dir=t, errno=$!\n";
  #@INC = '../lib';
}

my @command = qw(../mime-util unpack-smime);

# data we expect back out of MIME bundles
open my $fh, '<', 'message.txt';
my $message_txt = do { local $/; <$fh> };
close $fh;

# unpack-smime should handle decrypted or signed MIME data
is( test_unmime( 'message.sign', \@command ),
  $message_txt, 'command: ' . join( ' ', @command, 'message.sign' ) );
is( test_unmime( 'message.decrypt', \@command ),
  $message_txt, 'command: ' . join( ' ', @command, 'message.decrypt' ) );

diag("ignore warning about no MIME data from message.txt");
# also need defined behavior for raw file through unmime process
is( test_unmime( 'message.txt', \@command ),
  $message_txt, 'command: ' . join( ' ', @command, 'message.txt' ) );

exit 0;


# Run command with filename and compare output to expected result
sub test_unmime {
  my $filename = shift;
  my $cmd_ref  = shift;

  # TODO use list format instead (requires exec?)
  my $command = join ' ', @$cmd_ref, $filename;

  open my $fh, '-|', $command or return "error: $!\n";
  my $result_txt = do { local $/; <$fh> };
  close $fh;

  return $result_txt;
}
