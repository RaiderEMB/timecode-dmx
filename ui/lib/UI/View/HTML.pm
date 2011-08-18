package UI::View::HTML;

use strict;
use warnings;

use base 'Catalyst::View::TT';

__PACKAGE__->config(
    TEMPLATE_EXTENSION => '.tt',
    render_die => 1,
);

=head1 NAME

UI::View::HTML - TT View for UI

=head1 DESCRIPTION

TT View for UI.

=head1 SEE ALSO

L<UI>

=head1 AUTHOR

Raider EMB,,,

=head1 LICENSE

This library is free software. You can redistribute it and/or modify
it under the same terms as Perl itself.

=cut

1;
