package RDMX::Model::SQL;

use strict;
use base 'Catalyst::Model::DBIC::Schema';

__PACKAGE__->config(
    schema_class => 'SQL',
    
    connect_info => {
        dsn => 'dbi:mysql:database=rdmx;host=casa.exploit.no',
        user => 'root',
        password => 'warp242',
    }
);

=head1 NAME

RDMX::Model::SQL - Catalyst DBIC Schema Model

=head1 SYNOPSIS

See L<RDMX>

=head1 DESCRIPTION

L<Catalyst::Model::DBIC::Schema> Model using schema L<SQL>

=head1 GENERATED BY

Catalyst::Helper::Model::DBIC::Schema - 0.54

=head1 AUTHOR

root

=head1 LICENSE

This library is free software, you can redistribute it and/or modify
it under the same terms as Perl itself.

=cut

1;
