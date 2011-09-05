package webui2;
use Dancer ':syntax';
use DB;

our $VERSION = '0.1';

before sub {
    if (! session('user') && request->path_info !~ m{^/(login|logout)}) {
        var requested_path => request->path_info;
        request->path_info('/login');
    }
    
    header 'Cache-Control' => 'no-cache';

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
	session->destory;
	redirect "/";
};

get '/' => sub {
	template 'index';
};

#### SHOW CHAIN #######

get '/show' => sub {
    var shows => db("Show")->all();
    template 'show_list';
};

get '/show/*' => sub {
    my ($show_id) = splat;
    var theshow => db("Show")->find($show_id); 
    template 'show_show';
};

get '/show/*/**' => sub {
    my ($show_id) = splat;
    var theshow => db("Show")->find($show_id);
    pass;
};

prefix '/show/*';

get '/patch' => sub {
    my ($patch) = splat;
    template 'show_patch_list';
};

get '/patch/*' => sub {
    my ($patch) = splat;
    template 'show_patch_show';
};

get '/timeline' => sub {

    my @tclines = db("Tc")->search({ "show" => vars->{'theshow'}->id });
    var tclines => \@tclines;
    my %hasj = ();
    foreach my $line (@tclines) {
        my @split = split /\s*,\s*/, $line->tags();
	for my $word (@split) {
	    $hasj{lc($word)}=1;
	}
    }
    my @keys = sort keys %hasj;
    var tags => \@keys;

    template 'show_timeline';
};




true;
