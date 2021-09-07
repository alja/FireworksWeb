#!/usr/bin/perl

use CGI;

use IO::Socket qw(AF_INET SOCK_STREAM);

my $q = new CGI;

$EVE_HOST   = "localhost";
$EVE_PORT   =  6666;

$CERN_UPN   = $ENV{'OIDC_CLAIM_cern_upn'};
$CERN_GName = $ENV{'OIDC_CLAIM_given_name'};
$CERN_FName = $ENV{'OIDC_CLAIM_family_name'};

$REDIR_HOST  = $ENV{'SERVER_NAME'};
$LOGFILE_WWW = "/logs/" . $CERN_UPN;
$LOGFILE_PFX = $ENV{'DOCUMENT_ROOT'} . $LOGFILE_WWW;

$IS_TEST = $ENV{'SCRIPT_NAME'} =~ m/-test.pl$/;

if ($REDIR_HOST eq "phi1.t2.ucsd.edu")
{
  $REDIR_HOST = "phi1.t2.ucsd.edu";
  # $XCACHE_PFX = "root://xcache-00";
  $XCACHE_PFX = "root://cmsxrootd.fnal.gov";

  $EVE_PORT =  6669 if $IS_TEST;

  $PORT_MAP_FOO = sub {
    my $resp = shift;
    return "https://${REDIR_HOST}:$resp->{'port'}/$resp->{'dir'}?token=$resp->{'key'}";
  };
}
elsif ($REDIR_HOST eq "fireworks.cern.ch")
{
  # $XCACHE_PFX = "root://eoscms.cern.ch";
  $XCACHE_PFX = "/eos/cms/";

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

# Developmental auto-flush of stdout
$| = 1;

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

  cgi_print "Connected to $EVE_PORT" if $verbose;

  my $buf;
  $client->recv($buf, 1024);
  cgi_print "Server greeting: $buf" if $verbose;

  cgi_print "Sending $request" if $verbose;

  # MUST include trailing \n, the server is looking for it!

  my $size = $client->send($request);
  cgi_print "Sent data of length: $size" if $verbose;

  # $client->shutdown(SHUT_WR);

  $client->recv($buf, 1024);
  cgi_print "Server response: $buf" if $verbose;
  chomp $buf;

  $client->close();

  return $buf;
}

sub start_session
{
  my $file = shift;

  my $buf = connect_to_server(qq{{"action": "load", "file": "$file", "logdir": "$LOGFILE_PFX", "user": "$CERN_UPN"}\n}, 1);

  # Expect hash response, as { 'port'=> , 'dir'=> , 'key'=> }
  my $resp = eval $buf;

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

# cgi_print("LFN=".$q->param('LFN'));

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

  if ($q->param('Action') eq 'Load LFN')
  {
    if ($q->param('LFN') =~ m!^\w*(/+store/.*\.root)w*$!)
    {
      $file = "${XCACHE_PFX}/$1";
    }
    else
    {
      cgi_print "Error: LFN shoud match '/store/....../file-name.root'";
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
    cgi_print "Error Load: undefined file";
  }
}
elsif ($q->param('Action') eq 'Show Usage')
{

}
else
{
  cgi_print "Hello ${CERN_GName}, choose your action below.";

  print $q->start_form();

  print $q->textfield('LFN', '/store/...', 150, 512);
  print "<br>\n";
  print $q->submit('Action', 'Load LFN');

  print "<br>\n";
  for my $f (@SAMPLES)
  {
    print "<br>\n";
    print $q->submit('Action', "Load $f");
  }

  print "<br><br>\n";
  print $q->submit('Action', "Show Usage");

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
    print "Currently serving $r->{current_sessions} (total $r->{total_sessions} since service start)."
  }
}

cgi_end();
