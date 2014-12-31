#!/usr/bin/env perl
#
# Stop operation of a long running process after a certain time period:
#
#   timeout [-q] -- duration command [command args ...]
#
# Run perldoc(1) on this file for additional documentation.

use strict;
use warnings;

use File::Basename qw/basename/;
use Getopt::Long qw/GetOptions/;
my $has_timehires;
eval { require Time::HiRes };
unless ($@) {
  require Time::HiRes;
  $has_timehires = 1;
}

my $MYNAME = basename($0);

# how to convert short human durations into seconds
my %factor = (
  'w' => 604800,
  'd' => 86400,
  'h' => 3600,
  'm' => 60,
  's' => 1,
);

GetOptions(
  'l'         => \my $Flag_Lineout,
  'help|h|?'  => \&help,
  'quiet|q'   => \my $Flag_Quiet,
  'verbose|v' => \my $Flag_Verbose
) or help();

my $duration = shift;
help() if !defined $duration or !@ARGV;

my $timeout = duration2seconds($duration);
$duration = seconds2duration($timeout);

my $t0 = [ Time::HiRes::gettimeofday() ]
  if $has_timehires and $Flag_Verbose;

my $exit_status = 0;

my $pid = fork();
if ( $pid != 0 ) {    # parent
  eval {
    local $SIG{ALRM} = sub { die "alarm\n" };
    alarm $timeout;

    wait();
    $exit_status = ($? & 127 > 0) ? 128 + $? : $? >> 8;

    alarm 0;

    warn "$MYNAME: program ran for ",
      sprintf( "%.1f", Time::HiRes::tv_interval($t0) ), " seconds\n"
      if $has_timehires and $Flag_Verbose;
  };
  if ($@) {
    die "$MYNAME: error: $@" unless $@ eq "alarm\n";

    warn "$MYNAME: duration $duration exceeded: killing pid $pid\n"
      unless $Flag_Quiet;
    $exit_status = 2;

    for my $signal (qw/TERM INT HUP KILL/) {
      last if kill $signal, $pid;
      sleep 1;
      warn "$MYNAME: kill -$signal $pid failed...\n";
    }
    # for cleaner shell display
    print "\n" if $Flag_Lineout;
  }
} else {    # child
  exec @ARGV or die "$MYNAME: could not exec '@ARGV': $!\n";
}

exit $exit_status;

# takes duration such as "2m3s" and returns number of seconds.
sub duration2seconds {
  my $tmpdur = shift;
  my $timeout;

  if ( $tmpdur =~ m/^\d+$/ ) {
    $timeout = $tmpdur;

  } elsif ( $tmpdur =~ m/^[wdhms\d\s]+$/ ) {

    # match "2m 5s" style input and convert to seconds
    while ( $tmpdur =~ m/(\d+)\s*([wdhms])/g ) {
      $timeout += $1 * $factor{$2};
    }
  } else {
    die "$MYNAME: error: unknown characters in duration\n";
  }

  unless ( defined $timeout and $timeout =~ m/^\d+$/ ) {
    die "$MYNAME: error: unable to parse duration\n";
  }

  return $timeout;
}

# takes seconds and returns a shorthand duration format.
sub seconds2duration {
  my $tmpsec = shift;

  unless ( defined $tmpsec and $tmpsec =~ m/^\d+$/ ) {
    die "$MYNAME: error: argument not an integer";
  }

  my $seconds = $tmpsec % 60;
  $tmpsec = ( $tmpsec - $seconds ) / 60;
  my $minutes = $tmpsec % 60;
  $tmpsec = ( $tmpsec - $minutes ) / 60;

  #  my $hours = $tmpsec;
  my $hours = $tmpsec % 24;
  $tmpsec = ( $tmpsec - $hours ) / 24;
  my $days  = $tmpsec % 7;
  my $weeks = ( $tmpsec - $days ) / 7;

  # TODO better way to do this?
  my $temp = ($weeks) ? "${weeks}w" : q{};
  $temp .= ($days)    ? "${days}d"    : q{};
  $temp .= ($hours)   ? "${hours}h"   : q{};
  $temp .= ($minutes) ? "${minutes}m" : q{};
  $temp .= ($seconds) ? "${seconds}s" : q{};
  return $temp;
}

# a generic help blarb
sub help {
  warn <<"HELP";
Usage: $0 [options] -- duration command [command args ..]

Stops operation of the specified command after the specified duration.
The duration is either seconds, or a shorthand format of "2m3s" for
123 seconds.

  -h  Emit this blarb and exit
  -q  Quiet, emit somewhat fewer messages
  -v  Verbose, prints program run time unless timeout is hit

Run perldoc(1) on this script for additional documentation.

HELP
  exit 64;
}

__END__

=head1 NAME

timeout - stop operation of long running programs

=head1 SYNOPSIS

Break out of sleep program after five seconds:

  $ timeout -- 5s sleep 60

=head1 DESCRIPTION

=head2 Overview

This script allows programs to be stopped after a specified period of
time. Practical uses for this script include escape from buggy programs
that stall from Makefile, where a SIGINT to stop the program will also
stop make (because the SIGINT goes to the foreground process group, as I
much later learned).

The exit status will be 0 if the command completes before the timeout,
or >0 if anything goes awry or the command is killed.

=head2 Normal Usage

  $ timeout [options] -- duration command [command args ..]

The duration can either be a number (raw seconds), or a shorthand
format of the form "2m3s" for 120 seconds.  The following factors are
recognized:

  w - weeks
  d - days
  h - hours
  m - minutes
  s - seconds

Multiple factors will be added together, allowing easy addition of
time values to existing timeouts:

  $ timeout -- 3s3s sleep 60

Would only allow the sleep to run for six seconds.

Options:

=over 4

=item B<--help> | B<-h> | B<-?>

Prints a brief usage note about the script and then exits.

=item B<--quiet> | B<-q>

Quiet mode. Prints fewer status messages.

=item B<--verbose> | B<-v>

Verbose mode. Currently prints program run time unless the timeout is
reached. And only if L<Time::HiRes> is available.

=back

=head1 BUGS

=head2 Reporting Bugs

Newer versions of this script may be available from:

http://github.com/thrig/sial.org-scripts/tree/master

If the bug is in the latest version, send a report to the author.
Patches that fix problems or add new features are welcome.

=head2 Known Issues

No known bugs.

=head1 TODO

Currently, a hard upper time limit must be specified. In theory, one
could watch the output from the program and stop the program if it
remains idle for some period of time. (But that gets into the tricky
realm of "busy but doing work" vs. "hung but still looks like it is
doing something" depending on how poorly written the program is and what
is failing.) Fork this program and use an C<IPC::*> module if
interaction with I/O to or from the command run is necessary.

=head1 SEE ALSO

L<Capture::Tiny>, L<IPC::System::Simple>, L<perlipc>

=head1 AUTHOR

thrig - Jeremy Mates (cpan:JMATES) C<< <jmates at cpan.org> >>

=head1 COPYRIGHT

Copyright (c) 2015 Jeremy Mates

     Permission to use, copy, modify, and/or distribute this software for
     any purpose with or without fee is hereby granted, provided that the
     above copyright notice and this permission notice appear in all
     copies.

     THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
     WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
     WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
     AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
     DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA
     OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
     TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
     PERFORMANCE OF THIS SOFTWARE.

=cut
