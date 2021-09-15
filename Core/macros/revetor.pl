#!/usr/bin/perl

# The following is defined if we are run through mod_perl.
# Configured in test mode via /perl-run/, e.g.:
#   https://phi1.t2.ucsd.edu/perl-run/revetor-test.pl
#   https://fireworks.cern.ch/perl-run/revetor-test.pl
my $mod_perl_req = shift;

use CGI;

use IO::Socket qw(AF_INET SOCK_STREAM);
use IO::Socket::Timeout;

my $q = new CGI;

$EVE_HOST   = "localhost";
$EVE_PORT   =  6666;

@ADMINS = qw( amraktad matevz );

$LFN_RE = '^\w*/*(/store/.*\.root)\w*$';
$EOS_RE = '^\w*/*(/eos/.*\.root)\w*$';

$CERN_UPN   = $ENV{'OIDC_CLAIM_cern_upn'};
$CERN_GName = $ENV{'OIDC_CLAIM_given_name'};
$CERN_FName = $ENV{'OIDC_CLAIM_family_name'};

$REDIR_HOST  = $ENV{'SERVER_NAME'};
$LOGFILE_WWW = "/logs/" . $CERN_UPN;
$LOGFILE_PFX = $ENV{'DOCUMENT_ROOT'} . $LOGFILE_WWW;

$IS_TEST = $ENV{'SCRIPT_NAME'} =~ m/-test.pl$/;

if ($IS_TEST)
{
  $EVE_PORT    =  6669;
  $LOGFILE_WWW = "/logs-test/" . $CERN_UPN;
  $LOGFILE_PFX = $ENV{'DOCUMENT_ROOT'} . $LOGFILE_WWW;
}

$SOURCES = {}; # name -> prefix mapping
$error_str;

if ($REDIR_HOST eq "phi1.t2.ucsd.edu")
{
  $SOURCES->{'XCache_UCSD'} = {
    'desc'   => "Open LFN /store/... via XCache at UCSD",
    'prefix' => "root://xcache-00.t2.ucsd.edu/"
  };
  $SOURCES->{'AAA_FNAL'} = { # requires cert
    'desc'   => "Open LFN /store/... via AAA regional US redirector at FNAL",
    'prefix' => "root://cmsxrootd.fnal.gov:1094/"
  };

  $PORT_MAP_FOO = sub {
    my $resp = shift;
    return "https://${REDIR_HOST}:$resp->{'port'}/$resp->{'dir'}?token=$resp->{'key'}";
  };
}
elsif ($REDIR_HOST eq "fireworks.cern.ch")
{
  $SOURCES->{'EOS'} = {
    'desc'   => "Open CERN EOS LFN (/store/...) or PFN (/eos/...)",
    'prefix' => sub {
      my $f = shift;
      if    ($f =~ m!${LFN_RE}!) { return "/eos/cms" . $f; }
      elsif ($f =~ m!${EOS_RE}!) { return $f; }
      else  { $error_str = "File shoud match '/store/.../file-name.root' or '/eos/.../file-name.root'"; return undef; }
    }
  };
  # $SOURCES->{'EOS_XROOT'} = { xxxxx "root://eoscms.cern.ch/" };

  $PORT_MAP_FOO = sub {
    my $resp = shift;
    my ($port_rem) = $resp->{'port'} =~ m/(\d\d)$/;
    return "https://${REDIR_HOST}/host${port_rem}/$resp->{'dir'}?token=$resp->{'key'}";
  };
}

$PRINT_URL_ARGS = 0;
$PRINT_ENV      = 0;
$PRINT_TUNNEL_SUGGESTION = 0;

# Sample dir setup the same way on phi1 and on fireworks
$SAMPLE_DIR = "/data2/relval-samples";

@SAMPLES = qw{
  RelValMuMuMiniaod.root
  RelValMuMureco.root
  RelValRecHitReco.root 
  RelValTTBarReco.root
  RelValZTTMiniaod.root
  RelVallZTTGenSimReco.root      
  RelValZZMiniaod.root
};

# CGI script to connect to an Event Display server.
#
# Reports progress as it goes and at the end outputs a link to
# a newly spawned instance.
# And a reminder that a tunnel needs to be made at this point.
#
# Once things more-or-less work, we can just redirect on success:
#   print $q->redirect('http://$REDIR_HOST:$REDIR_PORT/...');
# ... or do some JS magick, or whatever.

################################################################################

sub cgi_beg
{
  print $q->header('text/html');

  print <<"FNORD";
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
  <title>cmsShowWeb Event-display Gateway</title>
</head>
<body>
FNORD
}

sub cgi_print
{
  print "<p><pre>", join("\n", @_), "</pre>\n";
}

sub cgi_end
{
  print<<"FNORD";

</body>
</html>
FNORD
}

sub cgi_die
{
  print "<p><b> $_[0] </b>\n";
  cgi_end();
  exit(1);
}

sub flush
{
  defined $mod_perl_req ? $mod_perl_req->rflush() : select()->flush();
}

################################################################################
# Connect and redirect
################################################################################

sub connect_to_server
{
  my $request = shift;
  my $verbose = shift;

  cgi_print "Connecting to local cmsShowWeb forker now ..." if $verbose;

  my $client = IO::Socket->new(
      Domain   => AF_INET,
      Type     => SOCK_STREAM,
      proto    => 'tcp',
      PeerHost => $EVE_HOST,
      PeerPort => $EVE_PORT,
      Timeout  => 5
  ) || cgi_die "Can't open socket: $@";

  IO::Socket::Timeout->enable_timeouts_on($client);

  cgi_print "Connected to $EVE_PORT" if $verbose;

  my $buf;
  $client->recv($buf, 1024);
  cgi_print "Server greeting: $buf" if $verbose;

  cgi_print "Sending $request" if $verbose;

  # MUST include trailing \n, the server is looking for it!

  my $size = $client->send($request);
  cgi_print "Sent data of length: $size" if $verbose;

  flush();

  my $tout  = 5;
  my $sum_t = 0;
  $client->read_timeout($tout);
  while (not defined ($ret = $client->recv($buf, 1024)))
  {
    $sum_t += $tout;
    cgi_print "Waiting for server response, ${sum_t}s";
    flush();
  }
  my $err_str = $!;

  if (length($buf)) {
    cgi_print "Server response: $buf" if $verbose;
    chomp $buf;
  } else {
    cgi_print "Error receiving session details, error: ${err_str}.";
  }

  $client->close();

  return $buf;
}

sub start_session
{
  my $file = shift;

  my $buf = connect_to_server(qq{{"action": "load", "file": "$file", "logdir": "$LOGFILE_PFX", "user": "$CERN_UPN"}\n}, 1);

  return undef unless length($buf);

  # Expect hash response, as { 'port'=> , 'dir'=> , 'key'=> }
  my $resp = eval $buf;
  unless (defined $resp) {
    cgi_print "Failed parsing of server response:", "    $buf";
    return undef;
  }

  if (defined $resp->{'error'}) {
    cgi_print "Server responded with error:", "    $resp->{'error'}";
    if (defined $resp->{'log_fname'}) {
      print "More information might be available in the <a href=\"$LOGFILE_WWW/$resp->{'log_fname'}\">log file</a>\n";
    }
    print "<p><a href=$ENV{'SCRIPT_URI'}>Back to main page</a>\n";
    return undef;
  }

  my $URL = &$PORT_MAP_FOO($resp);

  # For opening on localhost directly.
  # print "xdg-open $URL\n";
  # exec  "xdg-open $URL";

  print<<"FNORD";
<h2>
Your event display is ready, click link to enter:
</h2>
<p>
<a href="$URL">$URL</a>
<p>
<a href="$LOGFILE_WWW/$resp->{'log_fname'}">Log file</a>
<p>
<a href="$ENV{'SCRIPT_URI'}">Back to main page</a>
FNORD

  if ($PRINT_TUNNEL_SUGGESTION)
  {
    print<<"FNORD";
<small>
<p>
P.S. You probably need to make a tunnel to port $resp->{'port'} as things stand now.
<p>
ssh -S vocms-ctrl -O forward -L$resp->{'port'}:localhost:$resp->{'port'}  x
</small>
FNORD
  }
}


################################################################################
# Main & Form stuff
################################################################################

cgi_beg();

# Usage of INET sockets in cgi-bin under SE requires:
#   /usr/sbin/setsebool -P httpd_can_network_connect 1
# Maybe we should use UNIX sockets.

# cgi_print("File=".$q->param('File'));

my @names = $q->param();

if ($PRINT_URL_ARGS)
{
  print "<p><pre>\n";
  print "N_param = ", scalar(@names), "\n";

  for my $k (@names)
  {
    print "$k: ", $q->param($k), "\n";
  }
  print "\n", '-' x 80, "\n";
  print "</pre>\n";
}
if ($PRINT_ENV)
{
  print "<p><pre>\n";
  for my $k (sort keys %ENV)
  {
    print "$k: ", $ENV{$k}, "\n";
  }
  print "\n", '-' x 80, "\n";
  print "</pre>\n";
}


if ($q->param('Action') =~ m/^Load/)
{
  my $file;

  if ($q->param('Action') =~ m/^Load File (.*)$/)
  {
    my $srcobj = $SOURCES->{$1};
    if (not ref($srcobj->{'prefix'}))
    {
      if ($q->param('File') =~ m!${LFN_RE}!)
      {
        $file = $srcobj->{'prefix'} . $1;
      }
      else
      {
        cgi_print "Error: File shoud match '/store/....../file-name.root'";
      }
    }
    elsif (ref($srcobj->{'prefix'}) eq 'CODE')
    {
      $file = &{$srcobj->{'prefix'}}($1);
      cgi_print "Error: " . $error_str unless defined $file;
    }
    else
    {
      cgi_print "Error Load: wrong source definition, prefix should be a scalar or code ref, is " . ref($srcobj->{'prefix'});
    }
  }
  elsif ($q->param('Action') =~ m/^Load (.+\.root)/)
  {
    $file = "${SAMPLE_DIR}/$1";
  }
  else
  {
    cgi_print "Error Load: Unmatched Action value '$q->param('Action')'";
  }

  if (defined $file)
  {
    start_session($file);
  }
  else
  {
    cgi_print "Error Load: file name error";
  }
}
elsif ($q->param('Action') eq 'Show Usage')
{
  my $buf = connect_to_server(qq{{"action": "report_usage"}\n}, 0);
  my $r = eval $buf;
  print "Currently serving $r->{current_sessions} (total $r->{total_sessions} since service start).";
  print "<br><br>\n";# Request and show current session, users, run times ... log links for matchin user
  print $r->{'table'};
}
else
{
  cgi_print "Hello ${CERN_GName}, choose your action below.";

  print $q->start_form();

  print $q->textfield('File', '', 150, 512), "\n";
  print "<table>\n";
  print join("\n", map { "<tr><td>" . $q->submit('Action', "Load File $_") . "</td><td>" . $SOURCES->{$_}{'desc'} . "</td></tr>"} (keys %$SOURCES)), "\n</table>\n";

  # Proto for running locate on remote server. Locks up on caches, need objects in
  # %SOURCES with flag allow_xrdfs_locate.
  # for my $k (keys %SOURCES) {
  #  if ($SOURCES{$k} =~ m!root://([\w\d\.-]+(?::\d+)?)/!) {
  #    print "<br>xrdfs $1 locate -m\n";
  #  }
  # }

  print "<br>\n";
  for my $f (@SAMPLES)
  {
    print "<br>\n";
    print $q->submit('Action', "Load $f");
  }

  if (grep(/$CERN_UPN/, @ADMINS))
  {
    print "<br><br>\n";
    print $q->submit('Action', "Show Usage");
  }

  print $q->end_form();

  ## If logfile dir exists, tell user about it.
  if (-e $LOGFILE_PFX) {
    print "<br><br>\n";
    print "Your recent logs might be available here: <a href=\"$LOGFILE_WWW\">$LOGFILE_WWW</a>\n";
  }

  {
    my $buf = connect_to_server(qq{{"action": "status"}\n}, 0);
    my $r = eval $buf;
    print "<br><br>\n";
    print "Currently serving $r->{current_sessions} (total $r->{total_sessions} since service start).";
  }
}

cgi_end();
