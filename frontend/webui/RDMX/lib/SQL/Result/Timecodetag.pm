package SQL::Result::Timecodetag;

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

SQL::Result::Timecodetag

=cut

__PACKAGE__->table("timecodetag");

=head1 ACCESSORS

=head2 timecode

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 0

=head2 tag

  data_type: 'varchar'
  is_nullable: 0
  size: 32

=cut

__PACKAGE__->add_columns(
  "timecode",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 0,
  },
  "tag",
  { data_type => "varchar", is_nullable => 0, size => 32 },
);
__PACKAGE__->set_primary_key("timecode", "tag");

=head1 RELATIONS

=head2 timecode

Type: belongs_to

Related object: L<SQL::Result::Timecode>

=cut

__PACKAGE__->belongs_to(
  "timecode",
  "SQL::Result::Timecode",
  { id => "timecode" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-11 03:42:21
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:FdALQhSeEId1vWLAZ6bl3w


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
