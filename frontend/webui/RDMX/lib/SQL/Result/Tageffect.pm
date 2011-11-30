package SQL::Result::Tageffect;

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

SQL::Result::Tageffect

=cut

__PACKAGE__->table("tageffect");

=head1 ACCESSORS

=head2 timeline

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 0

=head2 tag

  data_type: 'varchar'
  is_nullable: 0
  size: 32

=head2 effect

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 1

=cut

__PACKAGE__->add_columns(
  "timeline",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 0,
  },
  "tag",
  { data_type => "varchar", is_nullable => 0, size => 32 },
  "effect",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 1,
  },
);
__PACKAGE__->set_primary_key("timeline", "tag");

=head1 RELATIONS

=head2 timeline

Type: belongs_to

Related object: L<SQL::Result::Timeline>

=cut

__PACKAGE__->belongs_to(
  "timeline",
  "SQL::Result::Timeline",
  { id => "timeline" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);

=head2 effect

Type: belongs_to

Related object: L<SQL::Result::Effect>

=cut

__PACKAGE__->belongs_to(
  "effect",
  "SQL::Result::Effect",
  { id => "effect" },
  {
    is_deferrable => 1,
    join_type     => "LEFT",
    on_delete     => "CASCADE",
    on_update     => "CASCADE",
  },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-17 15:15:24
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:pzsKRSW7PCXXQTTwxaQOsw


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
