# -*- Perl -*-

package UtilityTestBelt;
use 5.24.0;
use warnings;

use parent 'Import::Base';
our @IMPORT_MODULES = (
    feature => [qw( :5.24 )],    # includes strict, say, postderef
    'warnings',
    'File::Spec' => [qw()],      # TODO migrate things to f(x) calls
    'File::Spec::Functions' => [qw(catdir catfile splitdir)],
    'File::Temp'            => [qw(tempdir tempfile)],
    'Test::Cmd',                 # TODO rewrite things to use UnixCmdWrap
    'Test::Most',
    { 'Test::UnixCmdWrap' => 0.02 },
    'Test::UnixExit',
);

1;
