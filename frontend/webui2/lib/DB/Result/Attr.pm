package DB::Result::Attr;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components("InflateColumn::DateTime");

=head1 NAME

DB::Result::Attr

=cut

__PACKAGE__->table("attr");

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

Related object: L<DB::Result::Address>

=cut

__PACKAGE__->has_many(
  "addresses",
  "DB::Result::Address",
  { "foreign.attr" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-04 13:13:28
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:WlB+OmLI3LXDilc8NkNHgQ


# You can replace this text with custom code or comments, and it will be preserved on regeneration
1;
