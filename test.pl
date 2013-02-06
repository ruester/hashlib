#!/usr/bin/env perl

use strict;
use warnings;
use Storable;
use Data::Dumper;
use Time::HiRes qw(gettimeofday);

my %hash = ();

sub generate_random_string
{
	my $length_of_randomstring=shift;# the length of 
			 # the random string to generate

	my @chars=('a'..'z','A'..'Z','0'..'9');
	my $random_string;
	foreach (1..$length_of_randomstring) 
	{
		# rand @chars will generate a random 
		# number between 0 and scalar @chars
		$random_string.=$chars[rand @chars];
	}
	return $random_string;
}

my $str;

#for (my $i = 0; $i < 1000000; $i++) {
    #$str = generate_random_string(32);
    #print $str, "\n";
    #$hash{$str} = 1;
#}

my @arr = ();
my $i;

while (<STDIN>) {
    chomp;
    push(@arr, $_);
}

my $begin = gettimeofday;

foreach my $element (@arr) {
    $hash{$element} = "12345678901234567890123456789012";
}

my $time = gettimeofday-$begin;
print "hash put: ", $time, "\n";

$begin = gettimeofday;

foreach my $element (@arr) {
    $hash{$element};
}

$time = gettimeofday-$begin;
print "hash get: ", $time, "\n";

#print Dumper %hash;

$begin = gettimeofday;

store \%hash, 'store';

$time = gettimeofday-$begin;
print "hash store: ", $time, "\n";
