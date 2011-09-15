package SQL::Result::Tagpatch;

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

SQL::Result::Tagpatch

=cut

__PACKAGE__->table("tagpatch");

=head1 ACCESSORS

=head2 tag

  data_type: 'varchar'
  is_nullable: 0
  size: 32

=head2 patchaddr

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_nullable: 0

=cut

__PACKAGE__->add_columns(
  "tag",
  { data_type => "varchar", is_nullable => 0, size => 32 },
  "patchaddr",
  { data_type => "mediumint", extra => { unsigned => 1 }, is_nullable => 0 },
);
__PACKAGE__->set_primary_key("tag", "patchaddr");


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-15 01:58:38
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:z2onmeTS0lMPs3Y/ek9/bA


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
