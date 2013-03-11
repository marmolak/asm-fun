#!/usr/bin/perl
use strict;
use warnings;

open my $fh, '<', 'blob.bin' or die "can't opeb blob.bin! $!\n";

binmode $fh;

while ( read ($fh, my $buffer, 4096) ) {
	# damm i like perl and functional programming! :D
	print join '', map { sprintf ("\\x%.2x", ord $_); } split ('', $buffer);
}

close $fh;
