#!/usr/bin/env perl
use 5.16.0;
use warnings;

my @code2char =
  qw(a b c d e f g h i j k l m n o p q r s t u v w x y z A B C D E F G H I J K L M N O P Q R S T U V W X Y Z 0 1 2 3 4 5 6 7 8 9 + /);

sub genpass {
    my $length = shift;
    open my $fh, '<', '/dev/urandom' or die "open /dev/urandom failed: $!\n";
    my $buf;
    sysread $fh, $buf, $length;
    my $pass = '';
    for my $c ( unpack "C*", $buf ) {
        $pass .= $code2char[ $c % @code2char ];
    }
    die "insufficient entropy for password??" if length $pass != $length;
    return $pass;
}

say genpass(8)
