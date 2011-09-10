package RDMX::Controller::Root;
use Moose;
use namespace::autoclean;

BEGIN { extends 'Catalyst::Controller' }

__PACKAGE__->config(namespace => '');

sub index_auto : PathPart('') Chained('/') {
	my ( $self, $c ) = @_;
}

sub view : PathPart Chained('') Args(0) {

sub default :Path {
    my ( $self, $c ) = @_;
    $c->response->body( 'Page not found' );
    $c->response->status(404);
}

sub end : ActionClass('RenderView') {}



__PACKAGE__->meta->make_immutable;

1;
