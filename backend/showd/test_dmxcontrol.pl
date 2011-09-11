#!/usr/bin/perl
use DMXControl;
use Time::HiRes qw{ usleep };
use strict;
srand(time());

$DMXControl::host = '127.0.0.1';
$DMXControl::port = 9118;

DMXControl::init();
print "mainloop\n";
	DMXControl::start_transaction();
	#DMXControl::fade(1,1,15,10,DMXControl::OVERRIDE);
	#DMXControl::lock(1,15,8,DMXControl::OVERRIDE);
	#DMXControl::lock(2,30,10,DMXControl::OVERRIDE);
	DMXControl::blink(2,5,90,0.5,0.5,10,DMXControl::OVERRIDE);
	DMXControl::blink(1,0,15,0.5,0.5,10,DMXControl::OVERRIDE);
	DMXControl::lock(3,90,10,DMXControl::OVERRIDE);
	DMXControl::fade(4,0,255,10,DMXControl::OVERRIDE);
	DMXControl::fade(5,0,155,10,DMXControl::OVERRIDE);
	DMXControl::end_transaction();
sleep(8);
	DMXControl::start_transaction();
	DMXControl::lock(2,0,2,DMXControl::OVERRIDE);
	DMXControl::end_transaction();
exit;
while (1) {
	print "Control\n";
	DMXControl::control(DMXControl::OVERRIDE);
	usleep(500000);
}
	exit;
