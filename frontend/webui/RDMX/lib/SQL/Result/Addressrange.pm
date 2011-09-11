package SQL::Result::Addressrange;

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

SQL::Result::Addressrange

=cut

__PACKAGE__->table("addressrange");

=head1 ACCESSORS

=head2 id

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_auto_increment: 1
  is_nullable: 0

=head2 address

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 0

=head2 valfrom

  data_type: 'smallint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 valto

  data_type: 'smallint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 printf

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=head2 formula

  data_type: 'varchar'
  is_nullable: 1
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
  "address",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 0,
  },
  "valfrom",
  { data_type => "smallint", extra => { unsigned => 1 }, is_nullable => 0 },
  "valto",
  { data_type => "smallint", extra => { unsigned => 1 }, is_nullable => 0 },
  "printf",
  { data_type => "varchar", is_nullable => 0, size => 255 },
  "formula",
  { data_type => "varchar", is_nullable => 1, size => 255 },
);
__PACKAGE__->set_primary_key("id");
__PACKAGE__->add_unique_constraint("valfrom", ["valfrom", "valto"]);

=head1 RELATIONS

=head2 address

Type: belongs_to

Related object: L<SQL::Result::Address>

=cut

__PACKAGE__->belongs_to(
  "address",
  "SQL::Result::Address",
  { id => "address" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-10 20:28:06
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:al/MmE8eTqzL5w1oPoN/Ew


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
