#!/usr/bin/perl
use DMXControl;
use Time::HiRes qw{ usleep };
use strict;
srand(time());

$DMXControl::host = '127.0.0.1';
$DMXControl::port = 9118;

DMXControl::init();
while (1) {
print "mainloop\n";
for (1..44) {
	DMXControl::start_transaction();
	for my $ch (1..511) {
		DMXControl::blink($ch,0,255,0.1,0.1,4,DMXControl::OVERRIDE);
	}
	DMXControl::end_transaction();
	usleep(23000);
}
}
