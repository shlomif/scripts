# -*- Perl -*-

package UtilityTestBelt;
use 5.16.0;
use warnings;

use parent 'Import::Base';
our @IMPORT_MODULES = (
    feature => [qw( :5.16 )],    # includes 'strict', 'say'
    'warnings',
    'File::Spec' => [qw()],
    'File::Temp' => [qw(tempdir tempfile)],
    'Test::Cmd',
    'Test::Most',
    'Test::UnixExit',
);

1;
