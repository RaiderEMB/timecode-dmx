#!/usr/bin/perl -w

use strict;
use Data::Dumper;
use Raider::Avolites::Personality;


my $ref = Raider::Avolites::Personality::Parse("rods5k6g.r20");

print Dumper $ref;


