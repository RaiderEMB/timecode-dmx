use strict;
use warnings;
use Test::More;


use Catalyst::Test 'RDMX';
use RDMX::Controller::Tester;

ok( request('/tester')->is_success, 'Request should succeed' );
done_testing();
