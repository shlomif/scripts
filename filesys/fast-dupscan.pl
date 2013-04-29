#!/usr/local/bin/perl -w
# Craig Reyenga

use strict;
use warnings;
#use Cwd;
#use BSD::stat;
use Digest::MD5;
#use File::Basename;
use File::Find;
#use File::Spec;

#my @files;
my %sizes;

my $debug=1;

my $dir=shift(@ARGV);
my $dig=Digest::MD5->new;

find(
    sub {
	if (!-l $File::Find::name && -f _) {
	    my $size=-s $File::Find::name;
	    if (defined($sizes{$size})) {
		push(@{$sizes{$size}}, $File::Find::name);
	    } else {
	        $sizes{$size}=[$File::Find::name];
	    }
	}
    },$dir);

for my $size (sort { $a <=> $b } keys %sizes) {
    my @aos=@{$sizes{$size}};
    if ($size == 0) {
	print "##The following are empty:\n";
	for my $i (@aos) {
	    print "#$i\n";
	}
	next;
    }
    my $hm=@aos;
    if ($hm >= 2) {
	my %pot;
        for my $i (@aos) {
            unless (open FILE, $i) {
	        warn "##Can't open $i: $!\n";
	        next;
	    }
	    $dig->addfile(*FILE);
	    my $g = $dig->hexdigest;
	    close FILE;
	    
	    if (defined($pot{$g})) {
		push(@{$pot{$g}}, $i);
	    } else {
		$pot{$g}=[$i];
	    }
        }
	for my $p (sort keys %pot) {
	    my @aop=@{$pot{$p}};
	    my $hmp=@aop;
	    if ($hmp >= 2) { 
	        print "##Duplicate: s=$size md5=$p\n";
		print "#" . shift(@aop) . "\n";
	        print join("\n", @aop);
		print "\n";
	    }
	}
	    
    }
    
}


__END__
 } elsif (defined($sums{$g})&&($g ne 'd41d8cd98f00b204e9800998ecf8427e')) {
  print "Identical files found:\n $f\n $sums{$g}\n";
 } else {
  $sums{$g} = $f;
 }
} ## end for


