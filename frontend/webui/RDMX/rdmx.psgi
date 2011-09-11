use strict;
use warnings;

use RDMX;

my $app = RDMX->apply_default_middlewares(RDMX->psgi_app);
$app;

