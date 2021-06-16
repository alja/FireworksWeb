#!/usr/bin/perl

use CGI;

use IO::Socket qw(AF_INET SOCK_STREAM);

my $q = new CGI;

$EVE_HOST   = "localhost";
$EVE_PORT   =  6666; # 6666 for REve, 7777 for fwShow
$REDIR_HOST = "phi1.t2.ucsd.edu";
# $XCACHE_PFX = "root://xcache-00"; # 
$XCACHE_PFX = "root://cmsxrootd.fnal.gov"; # 

$PRINT_TUNNEL_SUGGESTION = 0;

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
  my $file = shift;

  cgi_print "Connecting to local cmsShowWeb forker now ...";

  my $client = IO::Socket->new(
      Domain   => AF_INET,
      Type     => SOCK_STREAM,
      proto    => 'tcp',
      PeerHost => $EVE_HOST,
      PeerPort => $EVE_PORT,
      Timeout  => 5
  ) || cgi_die "Can't open socket: $@";

  cgi_print "Connected to $EVE_PORT";

  my $buf;
  $client->recv($buf, 1024);
  cgi_print "Server greeting: $buf";


  cgi_print "Sending $file";

  # MUST include trailing \n, the server is looking for it!

  my $size = $client->send("$file\n");
  cgi_print "Sent data of length: $size";

  # $client->shutdown(SHUT_WR);

  $client->recv($buf, 1024);
  cgi_print "Server response: $buf";
  chomp $buf;

  $client->close();

  # Expect hash response, as { 'port'=> , 'dir'=> , 'key'=> }
  $resp = eval $buf;

  my $URL = "https://${REDIR_HOST}:$resp->{'port'}/$resp->{'dir'}?token=$resp->{'key'}";

  # For opening on localhost directly.
  # print "xdg-open $URL\n";
  # exec  "xdg-open $URL";

  print<<"FNORD";
<h2>
Your event display is ready, click link to enter:
</h2>
<p>
<a href="$URL">$URL</a>
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

my $user = $ENV{'OIDC_CLAIM_given_name'};

# cgi_print("LFN=".$q->param('LFN'));
my @names = $q->param();

print "<p><pre>\n";
print "N_param = ", scalar(@names), "\n";

for my $k (@names)
{
  print "$k: ", $q->param($k), "\n";
}

print "\n\n", '-' x 80, "\n\n";
print "</pre>\n";

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
  $file = "/data2/relval-samples/$1";
}
elsif (defined $q->param('Action'))
{
  cgi_print "Error: Unmatched Action value '$q->param('Action')'";
}

if (defined $file)
{
  connect_to_server($file);
}
else
{
  cgi_print "Hello ${user}, choose your action now ...";

  print $q->start_form();

  print $q->textfield('LFN', '/store/...', 150, 512);
  print "<BR>\n";
  print $q->submit('Action','Load LFN');

  print "<BR>\n";
  for my $f (@SAMPLES)
  {
    print "<BR>\n";
    print $q->submit('Action', "Load $f");
  }
  print $q->end_form();
}

cgi_end();
