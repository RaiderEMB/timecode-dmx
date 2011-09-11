package SQL::Result::Fixture;

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

SQL::Result::Fixture

=cut

__PACKAGE__->table("fixture");

=head1 ACCESSORS

=head2 id

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_auto_increment: 1
  is_nullable: 0

=head2 brand

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=head2 model

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=head2 mode

  data_type: 'varchar'
  is_nullable: 1
  size: 128

=head2 dmxsize

  data_type: 'smallint'
  is_nullable: 0

=head2 pandeg

  data_type: 'smallint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 tiltdeg

  data_type: 'smallint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 fixtype

  data_type: 'varchar'
  is_nullable: 0
  size: 40

=head2 fixshort

  data_type: 'varchar'
  is_nullable: 0
  size: 11

=cut

__PACKAGE__->add_columns(
  "id",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_auto_increment => 1,
    is_nullable => 0,
  },
  "brand",
  { data_type => "varchar", is_nullable => 0, size => 255 },
  "model",
  { data_type => "varchar", is_nullable => 0, size => 255 },
  "mode",
  { data_type => "varchar", is_nullable => 1, size => 128 },
  "dmxsize",
  { data_type => "smallint", is_nullable => 0 },
  "pandeg",
  { data_type => "smallint", extra => { unsigned => 1 }, is_nullable => 0 },
  "tiltdeg",
  { data_type => "smallint", extra => { unsigned => 1 }, is_nullable => 0 },
  "fixtype",
  { data_type => "varchar", is_nullable => 0, size => 40 },
  "fixshort",
  { data_type => "varchar", is_nullable => 0, size => 11 },
);
__PACKAGE__->set_primary_key("id");
__PACKAGE__->add_unique_constraint("brand", ["brand", "model", "mode", "dmxsize"]);

=head1 RELATIONS

=head2 addresses

Type: has_many

Related object: L<SQL::Result::Address>

=cut

__PACKAGE__->has_many(
  "addresses",
  "SQL::Result::Address",
  { "foreign.fixture" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);

=head2 patches

Type: has_many

Related object: L<SQL::Result::Patch>

=cut

__PACKAGE__->has_many(
  "patches",
  "SQL::Result::Patch",
  { "foreign.fixture" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-10 20:28:06
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:ewsRpOZIe+ngZaaWf11Ixw


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
