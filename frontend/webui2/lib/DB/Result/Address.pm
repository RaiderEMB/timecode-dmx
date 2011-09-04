package DB::Result::Address;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components("InflateColumn::DateTime");

=head1 NAME

DB::Result::Address

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

Related object: L<DB::Result::Attr>

=cut

__PACKAGE__->belongs_to(
  "attr",
  "DB::Result::Attr",
  { id => "attr" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);

=head2 fixture

Type: belongs_to

Related object: L<DB::Result::Fixture>

=cut

__PACKAGE__->belongs_to(
  "fixture",
  "DB::Result::Fixture",
  { id => "fixture" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);

=head2 addressranges

Type: has_many

Related object: L<DB::Result::Addressrange>

=cut

__PACKAGE__->has_many(
  "addressranges",
  "DB::Result::Addressrange",
  { "foreign.address" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-04 13:13:28
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:j+HMGPOxYtu9IKuAQzU27w


# You can replace this text with custom code or comments, and it will be preserved on regeneration
1;
