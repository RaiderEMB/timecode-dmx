package RDMX::Controller::Tester;
use Moose;
use namespace::autoclean;
use lib "../../../backend/showd";
use DMXControl;
use Data::Dumper;
use Time::HiRes qw( usleep );

BEGIN {extends 'Catalyst::Controller'; }

$DMXControl::host = '127.0.0.1';

sub effect_test   : Chained('/effect_id') : PathPart('test') : Args(0) {
	my ($self, $c) = @_;

	my @lengths;
	my @stepdata;

	my @steps = $c->model("SQL::Effectstep")->search({ effect => $c->stash->{effect}->id }, { order_by => "step" });

	my $step = 1;
	foreach my $effstep (@steps) {
		my $steptime;
		$steptime =  ($effstep->time + $effstep->time2) * $effstep->times if ($effstep->action eq 'blink');
		$steptime = $effstep->time + $effstep->time2 if ($effstep->action ne 'blink');

		$lengths[$effstep->step] = 0 if not defined $lengths[$effstep->step];

		if ($steptime > $lengths[$effstep->step]) {
			$lengths[$effstep->step] = $steptime;
		}

		$stepdata[$effstep->step] = [] if not defined $stepdata[$effstep->step];
		push @{$stepdata[$effstep->step]}, $effstep;
	}

	for my $ff (1..$#stepdata) {
		$c->log->error("Step:$ff\n");
		for my $step (@{$stepdata[$ff]}) {
			$c->log->error("Step:$ff:eff:".$step->action.":".$step->attribute->name."\n");
		}
	}

	my @fixtures = $c->model("SQL::Patch")->search({ show => $c->stash->{timeline}->id });

	DMXControl::init("192.168.0.109");
	for my $step (1..$#stepdata) {
		DMXControl::start_transaction();
			my $i = 0;
		for my $eff (@{$stepdata[$step]}) {
			foreach my $fix (@fixtures) {
			$_ .= 'FIX:'.$fix->fixture->id.':Trying:'.$eff->attribute->name.':::';
				my $fixi = $c->model("SQL::Address")->search({ fixture => $fix->fixture->id, attr => $eff->attribute->id });
				if (my $fixinfo = $fixi->first) {
					if ($eff->action eq 'set') {
						DMXControl::lock($fix->dmx + $fixinfo->offset - 1, $eff->from, ($eff->time + 100) / 1000, DMXControl::OVERRIDE);
					}
					elsif ($eff->action eq 'fade') {
						DMXControl::fade($fix->dmx + $fixinfo->offset - 1, $eff->from, $eff->to, ($eff->time + 100) / 1000, DMXControl::OVERRIDE);
					}
					elsif ($eff->action eq 'blink') {
						DMXControl::blink($fix->dmx + $fixinfo->offset - 1, $eff->from, $eff->to, ($eff->time + 100) / 1000, ($eff->time2 + 100) / 1000, $eff->times, DMXControl::OVERRIDE);
					}
					$i++;
					$_ .= 'STEP:'.$step.':'.$eff->action.':'.$eff->attribute->name.'=>'.$eff->from.':EFF:'.$fix->dmx.':'.$eff->time.':' .$fixinfo->offset.'=='.($fix->dmx + $fixinfo->offset).' ';
				} else {
					$_ .= "FIX:".$fix->fixture->id.":Missing \n";
				}
				$_ .= '<br>';
			}
			$_ .= '<br>';
		}
		DMXControl::end_transaction();
		usleep($lengths[$step]*1000);
	}
	$c->response->body("{<pre>EFID:".$c->stash->{effect}->id.":".$#steps."...".Dumper(\@lengths).$_."}");
	return 1;
}





__PACKAGE__->meta->make_immutable;

1;
