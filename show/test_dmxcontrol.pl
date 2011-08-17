#!/usr/bin/perl
use DMXControl;
use strict;
srand(time());

$DMXControl::host = '127.0.0.1';
$DMXControl::port = 9118;

DMXControl::init();
DMXControl::start_transaction();
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,5,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,7,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,8,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,9,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,9,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,9,DMXControl::OVERRIDE);
DMXControl::fade(int(rand()*512),0,255,20,DMXControl::OVERRIDE);
DMXControl::blink(12,0,255,0.1,0.8,4,DMXControl::OVERRIDE);
DMXControl::end_transaction();
