package RDMX::Controller::Root;
use lib "../../../../backend/showd";
use Moose;
use namespace::autoclean;
use Data::Dumper;
use ShowFile;

BEGIN { extends 'Catalyst::Controller' }

__PACKAGE__->config(namespace => '');

sub auto : Local {
	my ( $self, $c ) = @_;
	$c->log->error("auto");

	$c->stash->{current_view} = "TT";

	$c->stash->{api} = { api_version => 1, };

	push @{ $c->stash->{page_title} }, "RDMX";

	push @{ $c->stash->{static_script} }, "/static/js/jquery-1.6.2.min.js";
	push @{ $c->stash->{static_script} }, "/static/js/jquery-ui-1.8.16.custom.min.js";
	push @{ $c->stash->{static_script} }, "/static/fancybox/jquery.fancybox-1.3.4.pack.js";   
	push @{ $c->stash->{static_style} }, "http://fonts.googleapis.com/css?family=Antic";
	push @{ $c->stash->{static_style} }, "/static/css/ui-lightness/jquery-ui-1.8.16.custom.css";
	push @{ $c->stash->{static_style} }, "/static/fancybox/jquery.fancybox-1.3.4.css";   


	if ( -f $c->path_to("root") . "/static/auto.js" ) {
		push @{ $c->stash->{static_script} }, "/static/auto.js";
	}

	if ( -f $c->path_to("root") . "/static/" . $c->action . ".js" ) {
		push @{ $c->stash->{static_script} }, "/static/" . $c->action . ".js";
	}

	if ( -f $c->path_to("root") . "/static/auto.css" ) {
		push @{ $c->stash->{static_style} }, "/static/auto.css";
	}

	if ( -f $c->path_to("root") . "/static/" . $c->action . ".css" ) {
		push @{ $c->stash->{static_style} }, "/static/" . $c->action . ".css";
	}

	if ( defined $c->request->param("api") && $c->request->param("api") eq 'json' ) {
		$c->stash->{current_view} = "JSON";
	}
	
	return 1;
}

sub index : Path : Args(0) {
	my ( $self, $c ) = @_;
	@{$c->stash->{timelines}} = $c->model("SQL::Timeline")->all;
	return 1;
}

sub timeline_entry : PathPart('') Chained('/') CaptureArgs(1) {
	my ( $self, $c, $timeline_id ) = @_;
	if ($c->stash->{timeline} = $c->model("SQL::Timeline")->find( $timeline_id )) {

	}
	else {
		$c->go("/default");
	}

	return 1;
}
sub timeline : Chained('timeline_entry') : PathPart('') : Args(0) {
	my ( $self, $c ) = @_;
	return 1;
}

sub generate : Chained("timeline_entry") : PathPart("generate") : Args(0) {
	my ( $self, $c ) = @_;
	$c->stash->{current_view} = "JSON";

	my $timeline = $c->stash->{timeline};

	my %timecode;
	my %effect;
	my %fixture;
	my %tag_fix;
	my %tag_eff;
	my %tag_fix_eff;
	my %patch_fix;

	my @effects = $c->model("SQL::Effectstep")->all;
	foreach my $row (@effects) {
		%{$effect{$row->effect->id}[$row->step]{$row->attribute->id}} = $row->get_columns;
	}

	my @tags = $c->model("SQL::Tageffect")->search({ timeline => $timeline->id });
	$_ = "tags: " . scalar(@tags);
	foreach my $tag (@tags) {
		my @patches = $c->model("SQL::Tagpatch")->search({ tag => $tag->tag });
		$_ .= " patches: " . scalar(@patches);
		foreach my $patch (@patches) {
			my $patchdata = $c->model("SQL::Patch")->search({ show => $timeline->id, dmx => $patch->patchaddr })->first;
			$_ .= " patchdata: " . $patchdata->fixture->id;
			my @fixturedata = $c->model("SQL::Address")->search({ fixture => $patchdata->fixture->id });
			$_ .= " fixturedata: " . scalar(@fixturedata);
			foreach my $fix (@fixturedata) {
				%{$fixture{$fix->fixture->id}{$fix->attr->id}} = $fix->get_columns;
			}
			$tag_fix{$tag->tag}{$patchdata->fixture->id} = 1;
			$patch_fix{$patch->patchaddr} = $patchdata->fixture->id;
		}
		$tag_eff{$tag->tag} = $tag->effect->id;
	}

	ShowFile::init("/tmp/datafile.showd");
	my $eff_id = 1;
	foreach my $tag (keys %tag_eff) {
		foreach my $fix (keys %{$tag_fix{$tag}}) {
			# generate effect per fixture:
			ShowFile::effect_start($eff_id);
			my $stepi=0;
			foreach my $step (@{$effect{$tag_eff{$tag}}}) {
				foreach my $row (keys %$step) {
					my $effect = $effect{$tag_eff{$tag}}[$stepi]{$row};
					if (defined $fixture{$fix}{$row}) {
						my $fixture = $fixture{$fix}{$row};
						if ($effect->{action} eq 'set') {
							ShowFile::lock($fixture->{offset}-1, $effect->{from}, $effect->{'time'} / 1000, $stepi);
						}
						elsif ($effect->{action} eq 'blink') {
							ShowFile::blink($fixture->{offset}-1, $effect->{from}, $effect->{to}, $effect->{'time'} / 1000, $effect->{time2} / 1000, $effect->{times}, $stepi);
						}
						elsif ($effect->{action} eq 'fade') {
							ShowFile::fade($fixture->{offset}-1, $effect->{from}, $effect->{to}, $effect->{'time'} / 1000, $stepi);
						}
					}
				}
				$stepi++;
			}
			ShowFile::effect_end();
			$tag_fix_eff{$tag.'-'.$fix.'-'.$tag_eff{$tag}} = $eff_id;
			$eff_id++;
		}
	}

	foreach my $row ($c->stash->{timeline}->timecodes) {
		ShowFile::ts_start($row->position);
		@tags = $row->timecodetags;
		foreach my $tag (@tags) {
			my $tageff = $c->model("SQL::Tageffect")->search({ timeline => $timeline->id, tag => $tag->tag })->first;
			next if not defined $tageff;

			$tageff = $tageff->effect->id;
			my @tagpatch = $c->model("SQL::Tagpatch")->search({ timeline => $timeline->id, tag => $tag->tag });

			foreach my $patch (@tagpatch) {
				my $fix = $patch_fix{$patch->patchaddr};
				ShowFile::effect($patch->patchaddr, $tag_fix_eff{$tag->tag.'-'.$fix.'-'.$tag_eff{$tag->tag}});
				$_ .= $tag->tag.'-'.$fix.'-'.$tag_eff{$tag->tag}.' = ' .$tag_fix_eff{$tag->tag.'-'.$fix.'-'.$tag_eff{$tag->tag}} .' ';
			}		
		}
		ShowFile::ts_end();
	}
	ShowFile::close();
	$c->response->body("{success:true}");
	return 1;
}

sub patch_entry : Chained("timeline_entry") : PathPart("patch") : CaptureArgs(0) {
	my ( $self, $c ) = @_;
	@{ $c->stash->{patches} } = $c->model("SQL::Patch")->search({ show => $c->stash->{timeline}->id });
 
	return 1;
}

sub patch_add : Chained('patch_entry') : PathPart('add') : Args(0) {
	my ( $self, $c ) = @_;
	return 1;
}


sub patch : Chained('patch_entry') : PathPart('') : Args(0) {
	my ( $self, $c ) = @_;
	return 1;
}

sub edit_entry : Chained("timeline_entry") : PathPart("edit") : CaptureArgs(0) {
	my ( $self, $c ) = @_;
	return 1;
}

sub edit : Chained('edit_entry') : PathPart('') : Args(0) {
	my ( $self, $c ) = @_;

	my $tcs = $c->stash->{timeline}->timecodes;
	while (my $tc = $tcs->next) {
		push @{ $c->stash->{timecodes} }, $tc;
	}

}

sub edit_ajax : Chained("edit_entry") : PathPart("ajax") : Args(0) {
	my ( $self, $c ) = @_;
	if (defined $c->request->param("update")) {
		if (my $timecode = $c->model("SQL::Timecode")->find( $c->request->param("update") )) {
			if (defined $c->request->param("comment")) {
				$timecode->comment( $c->request->param("comment") );
				$timecode->update();
			}
		}
	}
	$c->response->body("{}");
	return 0;
}



sub effect_entry : Chained("timeline_entry") : PathPart("effect") : CaptureArgs(0) {
	my ( $self, $c ) = @_;
	return 1;
}

sub effect : Chained('effect_entry') : PathPart('') : Args(0) {
	my ( $self, $c ) = @_;
	if (defined $c->request->param("neweff")) {
		my $neweff = $c->model("SQL::Effect")->create({
			name => $c->request->param("neweff")
		});
		$neweff->update();
		$c->response->redirect("/".$c->stash->{timeline}->id."/effect/".$neweff->id());
	}
	@{ $c->stash->{effects} } = $c->model("SQL::Effect")->all();

}

sub effect_ajax : Chained("effect_entry") : PathPart("ajax") : Args(0) {
	my ( $self, $c ) = @_;
#	if (defined $c->request->param("update")) {
#		if (my $timecode = $c->model("SQL::Timecode")->find( $c->request->param("update") )) {
#			if (defined $c->request->param("comment")) {
#				$timecode->comment( $c->request->param("comment") );
#				$timecode->update();
#			}
#		}
#	}
	$c->response->body("{}");
	return 0;
}


sub effect_id     : Chained('effect_entry') : PathPart('') : CaptureArgs(1) {
	my ( $self, $c, $id ) = @_;
	if (defined $id) {
		if (my $effect = $c->model("SQL::Effect")->find($id)) {
			$c->stash->{effect} = $effect;
			return 1;
		} 
	}
	$c->response->status(404);
	return 0
}
sub effect_show   : Chained('effect_id') : PathPart('') : Args(0) {
	my ( $self, $c ) = @_;
	if (defined $c->request->param("del")) {
		$c->model("SQL::Effectstep")->find($c->request->param("del"))->delete();
		$c->response->redirect("/".$c->stash->{timeline}->id."/effect/". $c->stash->{effect}->id);
	}
	if (defined $c->request->param("new") && defined $c->request->param("action")) {
		my $es = $c->model("SQL::Effectstep")->create({
			effect => $c->stash->{effect}->id,
			attribute => $c->request->param("attribute"),
			'step' => $c->request->param("step"),
			'action' => $c->request->param("action"),
			'from' => $c->request->param("from"),
			'to' => $c->request->param("to"),
			'time' => $c->request->param("time"),
			'time2' => $c->request->param("time2"),
			'times' => $c->request->param("times"),
		});
		$es->update();
		$c->response->redirect("/".$c->stash->{timeline}->id."/effect/". $c->stash->{effect}->id);
	}


	@{ $c->stash->{'steps'} } = $c->model("SQL::Effectstep")->search({ effect => $c->stash->{effect}->id }, { order_by => "step" });    	

}

sub default :Path {
    my ( $self, $c ) = @_;
    $c->response->body( 'Page not found' );
    $c->response->status(404);
    return 1;
}

sub end : ActionClass('RenderView') {

}


__PACKAGE__->meta->make_immutable;

1;
