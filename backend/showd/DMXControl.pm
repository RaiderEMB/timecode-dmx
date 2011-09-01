#!/usr/bin/perl
=pod
= DMX event client
= (c) Copyright Raider, 2011
= 
= Author: Håkon Nessjøen <haakon.nessjoen@gmail.com>
=cut
package DMXControl;
use IO::Socket;
use Carp;
use strict;

our $sock;
our $host = '127.0.0.1';
our $port = 9118;
our %transactions;

# Flags
sub NONE { 1; }
sub OVERRIDE { 1; }

sub init {
	$sock = IO::Socket::INET->new(
    		Proto    => 'udp',
   		PeerPort => $port,
		PeerAddr => $host,
	) or carp "Could not create socket: $!\n";
	srand(time());
}

sub fade {
	my ($channel, $fromval, $toval, $seconds, $flags) = @_;

	$sock->send(pack("nCCnCCf", int(rand() * 65564), 1, $flags, $channel, $fromval, $toval, $seconds));
}

sub blink {
	my ($channel, $lowval, $highval, $secdown, $secup, $times, $flags) = @_;

	$sock->send(pack("nCCnCCffN", int(rand() * 65564), 2, $flags, $channel, $lowval, $highval, $secdown, $secup, $times));
}

sub control {
	my ($flags) = @_;

	$sock->send(pack("nCC", int(rand() * 65564), 3, $flags));
}

sub lock {
	my ($channel, $value, $forsecs, $flags) = @_;

	$sock->send(pack("nCCnCf", int(rand() * 65564), 4, $flags, $channel, $value, $forsecs));
}

sub max {
	my ($channel, $maxval, $forsecs, $flags) = @_;

	$sock->send(pack("nCCnCf", int(rand() * 65564), 6, $flags, $channel, $maxval, $forsecs));
}

sub min {
	my ($channel, $minval, $forsecs, $flags) = @_;

	$sock->send(pack("nCCnCf", int(rand() * 65564), 7, $flags, $channel, $minval, $forsecs));
}

sub add {
	my ($channel, $addval, $forsecs, $flags) = @_;

	$sock->send(pack("nCCnCf", int(rand() * 65564), 8, $flags, $channel, $addval, $forsecs));
}

sub sub {
	my ($channel, $subval, $forsecs, $flags) = @_;

	$sock->send(pack("nCCnCf", int(rand() * 65564), 9, $flags, $channel, $subval, $forsecs));
}

sub cancel {
	my ($operation_id, $flags) = @_;

	$sock->send(pack("nCCN", int(rand() * 65564), 10, $flags, $operation_id));
}

sub cancel_trans {
	my ($transaction_id, $flags) = @_;

	$sock->send(pack("nCCN", int(rand() * 65564), 10, $flags, $transaction_id));
}

sub scalemax {
	my ($channel, $maxval, $forsecs, $flags) = @_;

	$sock->send(pack("nCCnCf", int(rand() * 65564), 12, $flags, $channel, $maxval, $forsecs));
}

sub scalemin {
	my ($channel, $minval, $forsecs, $flags) = @_;

	$sock->send(pack("nCCnCf", int(rand() * 65564), 13, $flags, $channel, $minval, $forsecs));
}

sub start_transaction {
	my ($flags) = @_;

	$sock->send(pack("nCC", int(rand() * 65564), 14, $flags || 0));
}

sub end_transaction {
	my ($flags) = @_;

	$sock->send(pack("nCC", int(rand() * 65564), 15, $flags || 0));
}

1;
