package webui2;
use Dancer ':syntax';
use DB;

our $VERSION = '0.1';

get '/' => sub {
    print db("Fixture")."\n";
    template 'index';
};

true;
