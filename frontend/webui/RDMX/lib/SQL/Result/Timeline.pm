package SQL::Result::Timeline;

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

SQL::Result::Timeline

=cut

__PACKAGE__->table("timeline");

=head1 ACCESSORS

=head2 id

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_auto_increment: 1
  is_nullable: 0

=head2 name

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
  "name",
  { data_type => "varchar", is_nullable => 0, size => 255 },
);
__PACKAGE__->set_primary_key("id");

=head1 RELATIONS

=head2 patches

Type: has_many

Related object: L<SQL::Result::Patch>

=cut

__PACKAGE__->has_many(
  "patches",
  "SQL::Result::Patch",
  { "foreign.show" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);

=head2 tageffects

Type: has_many

Related object: L<SQL::Result::Tageffect>

=cut

__PACKAGE__->has_many(
  "tageffects",
  "SQL::Result::Tageffect",
  { "foreign.timeline" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);

=head2 timecodes

Type: has_many

Related object: L<SQL::Result::Timecode>

=cut

__PACKAGE__->has_many(
  "timecodes",
  "SQL::Result::Timecode",
  { "foreign.show" => "self.id" },
  { cascade_copy => 0, cascade_delete => 0 },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-16 01:24:32
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:yN3EuK6SbQMktU3M3rjPUw


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
