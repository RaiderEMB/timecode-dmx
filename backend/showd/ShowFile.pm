#!/usr/bin/perl
=pod
= DMX event client
= (c) Copyright Raider, 2011
= 
= Author: Håkon Nessjøen <haakon.nessjoen@gmail.com>
=cut
package ShowFile;
use IO::Socket;
use Carp;
use strict;

our $fh;
our %transactions;

# Flags
sub NONE { 1; }
sub OVERRIDE { 1; }

sub SHOW_FUNC_EFFECT_START { 1; }
sub SHOW_FUNC_EFFECT_END  { 2; }
sub SHOW_FUNC_BARTRIG_START { 3; }
sub SHOW_FUNC_BARTRIG_END { 4; }
sub SHOW_FUNC_TS_START { 5; }
sub SHOW_FUNC_TS_END { 6; }
sub SHOW_FUNC_RESET { 7; }
sub SHOW_FUNC_FADE { 10; }
sub SHOW_FUNC_BLINK { 11; }
sub SHOW_FUNC_LOCK { 12; }
sub SHOW_FUNC_CONTROL { 13; }

sub init {
	open($fh, ">". shift()) or die "Error opening file: $@";
	print $fh "showdaemon" . chr(0);
}

sub effect_start {
	my ($id) = @_;

	print $fh pack("nCn", 5, SHOW_FUNC_EFFECT_START, $id);
}

sub effect_end {
	my ($id) = @_;

	print $fh pack("nC", 1, SHOW_FUNC_EFFECT_END);
}

sub ts_start {
	my ($timestamp) = @_;

	if ($timestamp !~ /^\d+$/) {
		$timestamp = convert_time($timestamp);
	}

	print $fh pack("nCN", 5, SHOW_FUNC_TS_START, $timestamp);
}

sub lock {
	my ($channel, $val, $secs) = @_;

	print $fh pack("nCnCf", 8, SHOW_FUNC_LOCK, $channel, $val, $secs);
}

sub fade {
	my ($channel, $from, $to, $secs) = @_;

	print $fh pack("nCnCCf", 9, SHOW_FUNC_FADE, $channel, $from, $to, $secs);
}

sub blink {
	my ($channel, $from, $to, $secson, $secsoff, $times) = @_;

	print $fh pack("nCnCCffN", 17, SHOW_FUNC_BLINK, $channel, $from, $to, $secson, $secsoff, $times);
}

sub ts_end {
	print $fh pack("nC", 1, SHOW_FUNC_TS_END);
}

sub convert_time {
	my $ts = shift();
	if ($ts =~ /^(\d+):([0-5]\d):([0-5]\d).(\d\d\d)$/) {
		return $1 * 3600 + $2 * 60 + $3 + $4 / 1000;
	}
	return 0;
}

sub print_time {
        my $ts = shift();
        my $milli = $ts*1000 % 1000;
        my $sec = int($ts % 60);
        my $min = int($ts/60) % 60;
        my $hour = int($ts/3600);
        return sprintf("%02d:%02d:%02d.%03d", $hour, $min, $sec, $milli);
}


1;
