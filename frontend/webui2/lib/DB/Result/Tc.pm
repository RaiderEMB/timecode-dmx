package DB::Result::Tc;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components("InflateColumn::DateTime");

=head1 NAME

DB::Result::Tc

=cut

__PACKAGE__->table("tc");

=head1 ACCESSORS

=head2 id

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_auto_increment: 1
  is_nullable: 0

=head2 show

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 0

=head2 position

  data_type: 'bigint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 tags

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=head2 comment

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
  "show",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 0,
  },
  "position",
  { data_type => "bigint", extra => { unsigned => 1 }, is_nullable => 0 },
  "tags",
  { data_type => "varchar", is_nullable => 0, size => 255 },
  "comment",
  { data_type => "varchar", is_nullable => 0, size => 255 },
);
__PACKAGE__->set_primary_key("id");
__PACKAGE__->add_unique_constraint("show", ["show", "position"]);

=head1 RELATIONS

=head2 show

Type: belongs_to

Related object: L<DB::Result::Show>

=cut

__PACKAGE__->belongs_to(
  "show",
  "DB::Result::Show",
  { id => "show" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-04 13:13:28
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:/3PuZw3mle/HD2R1vZDYIA


# You can replace this text with custom code or comments, and it will be preserved on regeneration
1;
