#!/usr/bin/perl
#
# $Id$
#
#	check the output of testlongstr
#
$c = 1;
$| = 1;
$l = 0;
foreach $i (split(/ /,<>)) {
    $l += length($i)+1;
    $i =~ s/@.*//;
    die "mismatch at $i\n" if $i != $c++;
    print "$i..." if $i % 100 == 0;
}

print " -> result test Ok, $l bytes in length\n";
