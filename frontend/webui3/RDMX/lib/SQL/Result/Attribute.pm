package SQL::Result::Attribute;

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

SQL::Result::Attribute

=cut

__PACKAGE__->table("attribute");

=head1 ACCESSORS

=head2 id

  data_type: 'varchar'
  is_nullable: 0
  size: 1

=head2 name

  data_type: 'varchar'
  is_nullable: 0
  size: 32

=cut

__PACKAGE__->add_columns(
  "id",
  { data_type => "varchar", is_nullable => 0, size => 1 },
  "name",
  { data_type => "varchar", is_nullable => 0, size => 32 },
);
__PACKAGE__->set_primary_key("id");
__PACKAGE__->add_unique_constraint("name", ["name"]);

=head1 RELATIONS

=head2 addresses

Type: has_many

Related object: L<SQL::Result::Address>

=cut

__PACKAGE__->has_many(
  "addresses",
  "SQL::Result::Address",
  { "foreign.attr" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);

=head2 effectsteps

Type: has_many

Related object: L<SQL::Result::Effectstep>

=cut

__PACKAGE__->has_many(
  "effectsteps",
  "SQL::Result::Effectstep",
  { "foreign.attribute" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-11 03:09:41
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:V2GKJRx7iOUabVw5D6zzVA


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
