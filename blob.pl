#!/usr/bin/perl

use strict;
use warnings;

my $code = "\x5e\x66\x81\xc6\x38\x00\x48\x31\xc0\xfe\xc0\x48\x31\xff\x66\xff\xc7\x48\x31\xd2\x66\x81\xc2\x05\x00\x0f\x05\x48\xb8\x3c\x00\x00\x00\x00\x00\x00\x00\x48\x31\xff\x66\xff\xc7\x66\xff\xc7\x0f\x05\x48\x65\x6c\x6c\x0a";


open my $fh, '>', 'blob.bin' or die $!;

print $fh $code;

close $fh;
