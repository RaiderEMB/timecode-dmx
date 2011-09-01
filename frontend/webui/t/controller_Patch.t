use strict;
use warnings;
use Test::More;

BEGIN { use_ok 'Catalyst::Test', 'UI' }
BEGIN { use_ok 'UI::Controller::Patch' }

ok( request('/patch')->is_success, 'Request should succeed' );
done_testing();
