package SQL::Result::Address;

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

SQL::Result::Address

=cut

__PACKAGE__->table("address");

=head1 ACCESSORS

=head2 id

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_auto_increment: 1
  is_nullable: 0

=head2 fixture

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 0

=head2 offset

  data_type: 'smallint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 type

  data_type: 'varchar'
  is_nullable: 0
  size: 1

=head2 attr

  data_type: 'varchar'
  is_foreign_key: 1
  is_nullable: 0
  size: 1

=head2 bank

  data_type: 'mediumint'
  is_nullable: 0

=head2 name

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=head2 onvalue

  data_type: 'smallint'
  is_nullable: 0

=cut

__PACKAGE__->add_columns(
  "id",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_auto_increment => 1,
    is_nullable => 0,
  },
  "fixture",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 0,
  },
  "offset",
  { data_type => "smallint", extra => { unsigned => 1 }, is_nullable => 0 },
  "type",
  { data_type => "varchar", is_nullable => 0, size => 1 },
  "attr",
  { data_type => "varchar", is_foreign_key => 1, is_nullable => 0, size => 1 },
  "bank",
  { data_type => "mediumint", is_nullable => 0 },
  "name",
  { data_type => "varchar", is_nullable => 0, size => 255 },
  "onvalue",
  { data_type => "smallint", is_nullable => 0 },
);
__PACKAGE__->set_primary_key("id");
__PACKAGE__->add_unique_constraint("unik", ["fixture", "offset"]);

=head1 RELATIONS

=head2 attr

Type: belongs_to

Related object: L<SQL::Result::Attribute>

=cut

__PACKAGE__->belongs_to(
  "attr",
  "SQL::Result::Attribute",
  { id => "attr" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);

=head2 fixture

Type: belongs_to

Related object: L<SQL::Result::Fixture>

=cut

__PACKAGE__->belongs_to(
  "fixture",
  "SQL::Result::Fixture",
  { id => "fixture" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);

=head2 addressranges

Type: has_many

Related object: L<SQL::Result::Addressrange>

=cut

__PACKAGE__->has_many(
  "addressranges",
  "SQL::Result::Addressrange",
  { "foreign.address" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-10 20:28:06
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:FLkcz4wezp8Esfli/aKm+w


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
