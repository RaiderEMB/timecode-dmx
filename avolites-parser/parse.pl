#!/usr/bin/perl -w

use strict;
use Data::Dumper;

die unless defined $ARGV[0];


my $syntax = {
	"ATTR" => {
		'end' => "END",
		'callback' => \&dummy,
	},
	"PFTABLE" => {
		'end' => "PFEND",
		'callback' => \&dummy,
	},
	"TEXT" => {
		'end' => "END",
		'callback' => \&cb_text,
	},
	"DMX" => {
		'end' => "END",
		'callback' => \&cb_dmx,
	},
	"RANGE" => {
		'end' => "REND",
		'callback' => \&dummy,
	},
	"MACRO" => {
		'end' => "MEND",
		'callback' => \&dummy,
	},
	"DEVICEADDRESS" => {
		'end' => "DAEND",
		'callback' => \&cb_deviceaddress,
	},
	"TYPE" => {
		'callback' => \&cb_type,
	},
	"MIRROR" => {
		'callback' => \&cb_mirror,
	},
	"NAME" => {
		'callback' => \&dummy,
	},
	"MINDMX" => {
		'callback' => \&cb_eq,
	},
	"MAXDMX" => {
		'callback' => \&cb_eq,
	},
	"DEVICE" => {
		'callback' => \&cb_device,
	},
	"DIVBY" => {
		'callback' => \&cb_eq,
	},
		
};


open DATA, "<", $ARGV[0] or die "Usage $0 <fixture file> (could not open)";

my $inEscape = 0;
my $inFunction = undef;
my $tempFillerArgs = "";
my $tempFillerData = "";
my $raw_line = 0;
my $first = 1;

our $this = {};

RAW: while (my $raw = <DATA>) {
	$raw_line++;
	chomp $raw;

	# I FUCKING HATE YOU, WINDOWS
	$raw =~ s/\r//gs;

	if ($first) {
		if ($raw =~ /^(<\?xml|\/\/ )/) {
			syntax_error("Not supporting XML (Diamond files)\n");
			exit;
		}
		$first = 0;
	}	
	if (not defined $inFunction) { # Not in function

		# Skip empty lines, and those beginning with semicolon	
		next RAW if ($raw =~ /^(\s*;|\s*$)/); 

		# Remove eventual leading spaces
		$raw =~ s/^\s+//;
		$raw =~ s/\s+$//;

		my $pos = 1;
		if ($raw =~ /^\s*\/\//) {
			syntax_error("Not supporting VIZ files\n");
			exit;
		}

		if ($raw =~ /^([A-Z]+)\s*(.*)$/) {
			my $cmd = $1;
			my $args = $2;
			if (defined $syntax->{uc($cmd)}) {
				$inFunction = uc($cmd);
				$tempFillerArgs = $args;			
				if (not defined $syntax->{$inFunction}->{end}) {
					chomp $tempFillerArgs;
					handle_syntax($inFunction, $tempFillerArgs);
					$inFunction = undef;
				}

				next RAW;

			}
			$pos++;
		}

		if ($raw =~ /^TODO,/) {
			next RAW;
		}
	
		syntax_error("Syntax error in line $raw_line");

	} else { # In function
		if (defined $syntax->{$inFunction}->{end}) {	
			my $end = $syntax->{$inFunction}->{end};
			if ($raw =~ /^$end\s*$/i) {
				chomp $tempFillerArgs;
				handle_syntax($inFunction, $tempFillerArgs, $tempFillerData);
				$tempFillerArgs = "";
				$tempFillerData = "";
				$inFunction     = undef;
			} else {
				$tempFillerData .= $raw."\n";
			}

		}
		else {
			syntax_error("Syntax error in line $raw_line");
		}
	}
}

if (defined $inFunction) {
	syntax_error("Parser done too early. Forgot a closing tag?");
}

sub syntax_error {
	my ($text) = @_;
	print STDERR '[ERROR]: '.$text."\n";
}

sub handle_syntax {
	my ($cmd, $param, $data) = @_;
	if (defined $syntax->{$cmd}->{'callback'}) {
		$param =~ s/^\s+//;
		$param =~ s/\s+$//;

		if (defined $data) {
			&{ $syntax->{$cmd}->{'callback'} }( $cmd, $param, $data );
		} else {
			&{ $syntax->{$cmd}->{'callback'} }( $cmd, $param );
		}
	} else {
		print "SKIPPING: $cmd: $param\n";
	}
}

sub dummy {
	my ($cmd, $param, $data) = @_;
	print STDERR "Use of valid but unimplemented command '$cmd'\n";
}


# Status: Implementation OK   Testing: 50/50
sub cb_device {
	my ($cmd, $param) = @_;
	if ($param =~ s/^(\w+)//) {
		print "'".$param."'\n";
		$this->{'legend'}->{'name'} = $1;
	}
	if ($param =~ s/  (\d+) DMX Channels?//si) {
		$this->{'legend'}->{'channels'} = $1;
	}
	
	if ($param =~ s/,?\s*(8|16|32) ?bit\s*//si) {
		$this->{'legend'}->{'bits'} = $1;
	}
	if ($param =~ s/\s*mode=\s*(.+?)\s*$//i) {
		$this->{'legend'}->{'mode'} = $1;
	}
	return 1;
}

close DATA;


print Dumper $this;




# Status: Implementation NADA   Testing: 50/50
sub cb_deviceaddress {
	my ($cmd, $param, $data) = @_;
	$this->{legend}->{'label'} = [];
	for my $line (split /\n/, $data) {
		chomp $line;

		$line =~ s/^\s*//sg;
		$line =~ s/\s*$//sg;
		if (length $line) {
			if ($line =~ /^"([^"]+)"(.*)/) {
				my $line = $1;
				my $args = $2;
				my @args = ();
				if (defined $args) {
					@args = split /,/, $args;
				}
				push @{ $this->{legend}->{'label'} }, [$line,@args];
			}
		}
	}
}


sub cb_text {
	my ($cmd, $param, $data) = @_;
	$this->{legend}->{'text'} = [];
	for my $line (split /\n/, $data) {
		chomp $line;
		push @{ $this->{legend}->{text} }, $line;
	}
}
	
sub cb_type {
	my ($cmd, $param) = @_;
	if ($param =~ /\s*\d+\s+(\d+)\s+\w\s*/) {
		$this->{patch}->{size} = $1;
	}
}	

sub cb_mirror {
	my ($cmd, $param) = @_;
	if ($param =~ /\s*(\d+)\s+(\d+)\s+(\d+)\s*/) {

		if    ($1 == 0) { $this->{geo}->{type} = "static"; }
		elsif ($1 == 1) { $this->{geo}->{type} = "scanner"; }
		elsif ($1 == 2) { $this->{geo}->{type} = "head"; }
		else {
			syntax_error("Unknown mirror property: $1");
		}

		$this->{geo}->{pan_deg} = $2;
		$this->{geo}->{tilt_deg} = $3;
	}
}

sub cb_eq {
	my ($cmd, $param) = @_;
	if ($param =~ /\s*=\s*(\S+)\s*/) {
		$this->{constraint}->{lc($cmd)} = $1;
	}

}

sub cb_dmx {

	my ($cmd, $param, $data) = @_;

	for my $line (split /\n/, $data) {
		$line =~ s/^\s*//sg;
		$line =~ s/\s*$//sg;
	
		if ($line =~ m{
			^ 
			(\d+) \s+    #  1: "Channels bank number"
			(\d+) \s+    #  2: Upper faders in bank?!
			(\w+) \s+    #  3: Channel type LTP HTP 16bit fadable HTP LTP.. etc
			(\d+) \s+    #  4: DMX Offset
			(\d+) \s+    #  5: Output level 0%-100%
			(\d+)        #  6: Curve number ?! usually 1
			(I|N) \s+    #  7: Curve inverted or normal
			([A-Z0]) \s+ #  8: Attribute type 
			(1) \s+      #  9: Checksum channel (always 1)
			"([^"]+)"\s+ # 10: Attribute name
			(\d+) \s+    # 11: The ON value of this channel 0..255
			(\d+) \s+    # 12: Highlight level of this channel 0..255
			(\d+)        # 13: Lowlight level of this channel 0..255
			$

		}simx) {
			$this->{patch}->{channels}->{$4} = {
				bank => $1,
				bank_ul_faders => $2,
				type => $3,
				offset => $4,
				level_pc => $5,
				curve_dir => $7,
				attr => $8,
				name => $10,
				val_on => $11,
				val_hi => $12,
				val_lo => $13,
			};
		}		
	}


}
