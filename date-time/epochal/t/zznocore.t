#!perl
#
# Was a core file created? Such events would probably fail any exit status
# tests, but folks might neglect to check that.

use strict;
use warnings;

use Test::More tests => 1;
ok( !-e 'epochal.core', "made no ore from *.c" );
