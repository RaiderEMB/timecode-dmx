package webui2;
use Dancer ':syntax';
use DB;

our $VERSION = '0.1';

before sub {
    if (! session('user') && request->path_info !~ m{^/(login|logout)}) {
        var requested_path => request->path_info;
        request->path_info('/login');
    }
};
 
get '/login' => sub {
    template 'login', { path => vars->{requested_path} };
};
 
post '/login' => sub {
    if (params->{user} eq 'admin' && params->{pass} eq 'admin') {
        session user => "1";
        redirect '/';
    } else {
        redirect '/login?failed=1';
    }
};

get '/logout' => sub {
	session user => 0;
	redirect "/";
};

get '/' => sub {
	template 'index';
};

get '/patch' => sub {
	template 'patch';
};

true;
