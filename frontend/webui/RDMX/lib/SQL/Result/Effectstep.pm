package SQL::Result::Effectstep;

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

SQL::Result::Effectstep

=cut

__PACKAGE__->table("effectstep");

=head1 ACCESSORS

=head2 id

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_auto_increment: 1
  is_nullable: 0

=head2 effect

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_foreign_key: 1
  is_nullable: 0

=head2 attribute

  data_type: 'varchar'
  is_foreign_key: 1
  is_nullable: 0
  size: 1

=head2 step

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 action

  data_type: 'enum'
  extra: {list => ["blink","set","fade"]}
  is_nullable: 0

=head2 from

  data_type: 'tinyint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 to

  data_type: 'tinyint'
  extra: {unsigned => 1}
  is_nullable: 1

=head2 time

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_nullable: 0

=head2 time2

  data_type: 'mediumint'
  extra: {unsigned => 1}
  is_nullable: 1

=head2 times

  data_type: 'integer'
  extra: {unsigned => 1}
  is_nullable: 1

=head2 preval

  data_type: 'tinyint'
  extra: {unsigned => 1}
  is_nullable: 1

=cut

__PACKAGE__->add_columns(
  "id",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_auto_increment => 1,
    is_nullable => 0,
  },
  "effect",
  {
    data_type => "mediumint",
    extra => { unsigned => 1 },
    is_foreign_key => 1,
    is_nullable => 0,
  },
  "attribute",
  { data_type => "varchar", is_foreign_key => 1, is_nullable => 0, size => 1 },
  "step",
  { data_type => "mediumint", extra => { unsigned => 1 }, is_nullable => 0 },
  "action",
  {
    data_type => "enum",
    extra => { list => ["blink", "set", "fade"] },
    is_nullable => 0,
  },
  "from",
  { data_type => "tinyint", extra => { unsigned => 1 }, is_nullable => 0 },
  "to",
  { data_type => "tinyint", extra => { unsigned => 1 }, is_nullable => 1 },
  "time",
  { data_type => "mediumint", extra => { unsigned => 1 }, is_nullable => 0 },
  "time2",
  { data_type => "mediumint", extra => { unsigned => 1 }, is_nullable => 1 },
  "times",
  { data_type => "integer", extra => { unsigned => 1 }, is_nullable => 1 },
  "preval",
  { data_type => "tinyint", extra => { unsigned => 1 }, is_nullable => 1 },
);
__PACKAGE__->set_primary_key("id");

=head1 RELATIONS

=head2 effect

Type: belongs_to

Related object: L<SQL::Result::Effect>

=cut

__PACKAGE__->belongs_to(
  "effect",
  "SQL::Result::Effect",
  { id => "effect" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);

=head2 attribute

Type: belongs_to

Related object: L<SQL::Result::Attribute>

=cut

__PACKAGE__->belongs_to(
  "attribute",
  "SQL::Result::Attribute",
  { id => "attribute" },
  { is_deferrable => 1, on_delete => "CASCADE", on_update => "CASCADE" },
);


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-15 01:58:38
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:p1ZagwAMIi1278QfWkcTaA


# You can replace this text with custom code or comments, and it will be preserved on regeneration
__PACKAGE__->meta->make_immutable;
1;
