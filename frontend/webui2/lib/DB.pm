package DB;
use base 'Exporter';

@EXPORT = qw(db);

# Created by DBIx::Class::Schema::Loader
# DO NOT MODIFY THE FIRST PART OF THIS FILE

use strict;
use warnings;

use base 'DBIx::Class::Schema';

__PACKAGE__->load_namespaces;


# Created by DBIx::Class::Schema::Loader v0.07010 @ 2011-09-04 13:13:28
# DO NOT MODIFY THIS OR ANYTHING ABOVE! md5sum:NbuS4oTt3hzj89Zqk2gPVw


our $_db = DB->connect( "dbi:mysql:database=rdmx", "root", "warp242",
  {
    quote_char => "`",
    name_sep   => '.'
  }
);
sub db {
	return $_db->resultset(shift);
}

# You can replace this text with custom code or comments, and it will be preserved on regeneration
1;
