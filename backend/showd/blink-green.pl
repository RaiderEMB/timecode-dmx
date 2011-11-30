#!/usr/bin/perl
use DMXControl;
use Time::HiRes qw{ usleep };
use strict;
srand(time());

$DMXControl::host = '192.168.0.109';
$DMXControl::port = 9118;

DMXControl::init();
print "mainloop\n";

	#DMXControl::fade(1,1,15,10,DMXControl::OVERRIDE);
	#DMXControl::lock(1,15,8,DMXControl::OVERRIDE);
	#DMXControl::lock(2,30,10,DMXControl::OVERRIDE);
#	DMXControl::blink(2,5,90,0.5,0.5,10,DMXControl::OVERRIDE);
#	DMXControl::blink(1,0,15,0.5,0.5,10,DMXControl::OVERRIDE);
#	DMXControl::lock(3,90,10,DMXControl::OVERRIDE);
#	DMXControl::fade(4,0,255,10,DMXControl::OVERRIDE);
#	DMXControl::fade(5,0,155,10,DMXControl::OVERRIDE);
#sleep(8);
#	DMXControl::lock(2,0,2,DMXControl::OVERRIDE);
#while (1) {
	print "Control\n";


DMXControl::start_transaction();
DMXControl::lock(500,255,1,DMXControl::OVERRIDE);
DMXControl::lock(504,255,1,DMXControl::OVERRIDE);
DMXControl::lock(501,255,1);
DMXControl::lock(501,255,1);
DMXControl::lock(505,255,1);
#DMXControl::lock(502,128,1,DMXControl::OVERRIDE);
#DMXControl::lock(503,128,1,DMXControl::OVERRIDE);
DMXControl::end_transaction();

while(1) {
	usleep(2000);
}
#DMXControl::start_transaction();


#for (1..512) {
#	print "Chan: $_\n";
#}
#
#DMXControl::end_transaction();
exit;
