package UI::Controller::Patch;
use Moose;
use namespace::autoclean;

BEGIN {extends 'Catalyst::Controller'; }

sub index :Path :Args(0) {
    my ( $self, $c ) = @_;
}



__PACKAGE__->meta->make_immutable;

1;
