#!/usr/bin/env perl
# More portable (and shorter) version.
rand $. < 1 && ( $line = $_ ) while readline;
print $line;
