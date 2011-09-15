package SQL::Result::Timecode;

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

SQL::Result::Timecode

=cut

__PACKAGE__->table("timecode");

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
  "comment",
  { data_type => "varchar", is_nullable => 0, size => 255 },
);
__PACKAGE__->set_primary_key("id");
__PACKAGE__->add_unique_constraint("show", ["show", "position"]);

=head1 RELATIONS

=head2 show

Type: belongs_to

Related object: L<SQL::Result::Timeline>

=cut

__PACKAGE__->belongs_to(
  "show",
  "SQL::Result::Timeline",
  { id => "show" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);

=head2 timecodetags

Type: has_many

Related object: L<SQL::Result::Timecodetag>

=cut

__PACKAGE__->has_many(
  "timecodetags",
  "SQL::Result::Timecodetag",
  { "foreign.timecode" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-11 03:55:11
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:Qi18nGqvDlqhrNvKGC6qzw


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
