package SQL::Result::Effect;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

use strict;
use warnings;

use Moose;
use MooseX::NonMoose;
use namespace::autoclean;
extends 'DBIx::Class::Core';

__PACKAGE__->load_components("InflateColumn::DateTime");

=head1 NAME

SQL::Result::Effect

=cut

__PACKAGE__->table("effect");

=head1 ACCESSORS

=head2 id

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_auto_increment: 1
  is_nullable: 0

=head2 name

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=cut

__PACKAGE__->add_columns(
  "id",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_auto_increment => 1,
    is_nullable => 0,
  },
  "name",
  { data_type => "varchar", is_nullable => 0, size => 255 },
);
__PACKAGE__->set_primary_key("id");

=head1 RELATIONS

=head2 effectsteps

Type: has_many

Related object: L<SQL::Result::Effectstep>

=cut

__PACKAGE__->has_many(
  "effectsteps",
  "SQL::Result::Effectstep",
  { "foreign.effect" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);

=head2 tageffects

Type: has_many

Related object: L<SQL::Result::Tageffect>

=cut

__PACKAGE__->has_many(
  "tageffects",
  "SQL::Result::Tageffect",
  { "foreign.effect" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-16 01:24:32
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:IKKS9dXxAXPhTZW+Na/1HQ


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
