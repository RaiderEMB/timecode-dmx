package DB::Result::Patch;

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

use strict;
use warnings;

use base 'DBIx::Class::Core';

__PACKAGE__->load_components("InflateColumn::DateTime");

=head1 NAME

DB::Result::Patch

=cut

__PACKAGE__->table("patch");

=head1 ACCESSORS

=head2 show

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 0

=head2 dmx

  data_type: 'smallint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 fixture

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 1

=head2 tags

  data_type: 'varchar'
  is_nullable: 0
  size: 255

=cut

__PACKAGE__->add_columns(
  "show",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 0,
  },
  "dmx",
  { data_type => "smallint", extra => { unsigned => 1 }, is_nullable => 0 },
  "fixture",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 1,
  },
  "tags",
  { data_type => "varchar", is_nullable => 0, size => 255 },
);
__PACKAGE__->set_primary_key("show", "dmx");

=head1 RELATIONS

=head2 fixture

Type: belongs_to

Related object: L<DB::Result::Fixture>

=cut

__PACKAGE__->belongs_to(
  "fixture",
  "DB::Result::Fixture",
  { id => "fixture" },
  {
    is_deferrable => 1,
    join_type     => "LEFT",
    on_delete     => "CASCADE",
    on_update     => "CASCADE",
  },
);

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
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:DNDWdBDT9gtthEzN51F35Q


# You can replace this text with custom code or comments, and it will be preserved on regeneration
1;
