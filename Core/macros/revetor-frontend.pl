#!/usr/bin/perl

# The following is defined if we are run through mod_perl.
# Configured in test mode via /perl-run/, e.g.:
#   https://phi1.t2.ucsd.edu/perl-run/revetor-test.pl
#   https://fireworks.cern.ch/perl-run/revetor-test.pl
my $mod_perl_req = shift;

use CGI;

use IO::Socket qw(AF_INET SOCK_STREAM);
use IO::Socket::Timeout;
use File::Basename;

#use Data::Dumper;
#$Data::Dumper::Sortkeys = 1;
#use LWP::UserAgent qw();
#my $ua = LWP::UserAgent->new();

my $q = new CGI;

$EVE_HOST   = "localhost";
$EVE_PORT   =  6666;

@ADMINS = qw( amraktad matevz );

$LFN_RE = '^/*(/store/\S*\.root)$';
$EOS_RE = '^/*(/eos/\S*\.root)$';

$CERN_UPN   = $ENV{'OIDC_CLAIM_cern_upn'};
$CERN_GName = $ENV{'OIDC_CLAIM_given_name'};
$CERN_FName = $ENV{'OIDC_CLAIM_family_name'};

$REDIR_HOST  = $ENV{'SERVER_NAME'};
$LOGFILE_WWW = "/logs/" . $CERN_UPN;
$LOGFILE_PFX = $ENV{'DOCUMENT_ROOT'} . $LOGFILE_WWW;
$CONFIG_WWW = "/config/";

$IS_TEST = $ENV{'SCRIPT_NAME'} =~ m/-test.pl$/;
$BACK_URL="https://fireworks.cern.ch/cmsShowWeb/revetor-yuxiao.pl";

if ($IS_TEST)
{
  $EVE_PORT    =  6669;
  $LOGFILE_WWW = "/logs-test/" . $CERN_UPN;
  $LOGFILE_PFX = $ENV{'DOCUMENT_ROOT'} . $LOGFILE_WWW;
  $CONFIG_WWW = "/config-test/";
}
$AUTO_REDIRECT=1;
$SOURCES = {}; # name -> prefix mapping
$error_str;

if ($REDIR_HOST eq "fireworks.ucsd.edu" or $REDIR_HOST eq "phi1.t2.ucsd.edu")
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
elsif ($REDIR_HOST eq "fireworks.cern.ch" or $REDIR_HOST eq "vocms0102.cern.ch")
{
  $SOURCES->{'EOS'} = {
    'desc'   => "Open CERN EOS LFN (/store/...) or PFN (/eos/...)",
    'prefix' => sub {
      my $f = shift;
      if    ($f =~ m!${LFN_RE}!) { return "/eos/cms" . $1; }
      elsif ($f =~ m!${EOS_RE}!) { return $1; }
      else  { $error_str = "should match '/store/.../file-name.root' or '/eos/.../file-name.root'"; return undef; }
    }
  };
  # $SOURCES->{'EOS_XROOT'} = { xxxxx "root://eoscms.cern.ch/" };
  $SOURCES->{'AAA'} = { # requires cert
    'desc'   => "Open AAA LFN (/store/...) via the regional EU redirector (fallbacks to global / US if the file is not found in the EU)",
    'prefix' => "root://vocms0111.cern.ch:1094/"
  };

  $PORT_MAP_FOO = sub {
    my $resp = shift;
    my ($port_rem) = $resp->{'port'} =~ m/(\d\d)$/;
    #return "https://${REDIR_HOST}/host${port_rem}/$resp->{'dir'}?token=$resp->{'key'}";
    return "https://${REDIR_HOST}/host${port_rem}/$resp->{'dir'}";
  };
}
else
{
  $CONFIG_ERROR = "unconfigured host '${REDIR_HOST}'";
}

$PRINT_URL_ARGS = 0;
$PRINT_ENV      = 0;
$PRINT_TUNNEL_SUGGESTION = 0;

# Sample dir setup the same way on phi1 and on fireworks
$SAMPLE_DIR = "/data2/CMSSW_12_5_relval-samples";

# comment RelValMuMureco.root
@SAMPLES = qw{
  RelValMuMuMiniaod.root
  RelValMuMureco.root
  RelValRecHitReco.root 
  RelValTTBarReco.root
  RelValZTTMiniaod.root
  RelValZEEMiniaod.root
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
<link rel="stylesheet" type="text/css" href="/css/main.css" />
  <title>cmsShowWeb Gateway</title>
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

sub recv_with_timeout
{
  my $sock = shift;
  my $size = shift;
  my $timeout = shift || 5;
  my $err_to  = shift || 100;
  my $buf;
  my $sum_t = 0;

  $sock->read_timeout($timeout);

  my $ret;
  while (not defined ($ret = $sock->recv($buf, $size)))
  {
    $sum_t += $timeout;
    cgi_print "Waiting for server response, ${sum_t}s [ max = ${err_to}s ]";
    flush();
    if ($sum_t >= $err_to) {
      return undef;
    }
  }
  chomp  $buf;
  return $buf;
}

sub connect_to_server
{
  my $request = shift;
  my $verbose = shift;
  my $buf;

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

  $buf = recv_with_timeout($client, 1024, 5, 30);
  
  unless (length($buf)) {
    my $err_str = $!;
    cgi_print "Error receiving server greeting, error: ${err_str}.";
    $client->close();
    return $buf;
  }
  cgi_print "Server greeting: $buf" if $verbose;

  cgi_print "Sending $request" if $verbose;

  # MUST include trailing \n, the server is looking for it!

  my $size = $client->send($request);
  cgi_print "Sent data of length: $size" if $verbose;

  flush();

  $buf = recv_with_timeout($client, 1024, 5, 300);
 
  if (length($buf)) {
    cgi_print "Server response: $buf" if $verbose;
  } else {
    my $err_str = $!;
    cgi_print "Error receiving server response, error: ${err_str}.";
  }

  $client->close();

  return $buf;
}

sub start_session
{
  my $file = shift;
  my $logdirurl = "https://${REDIR_HOST}${LOGFILE_WWW}/";
  my $fwconfig = $q->param('FWconfig');
  my $fwconfigdir = $ENV{'DOCUMENT_ROOT'}.$CONFIG_WWW . ${CERN_UPN};

  $fwconfig  =~ s/^\s+|\s+$//g;

  if ($q->url_param('fwconfig') && $q->url_param('fwconfig') =~ m!^\w[-\w]+\.fwc$!)
	  {
		      $fwconfig = $q->url_param('fwconfig');
	      }


  if ($fwconfig ne "" and $fwconfig !~ m!^http!) {
      $fwconfig = ${fwconfigdir} . "/" . ${fwconfig};
  }
  elsif ($fwconfig =~m /^https\:\/\/${REDIR_HOST}(.*)/ )
  {
    $fwconfig = $ENV{'DOCUMENT_ROOT'} . $1;
  }

  my $fwgeo = $q->param('FWgeo');
 
  if ($q->url_param('fwgeo') )
  {
    $fwgeo = $q->url_param('fwgeo');
  }


  my $buf = connect_to_server(qq{{"action": "load", "file": "$file",
                                  "logdir": "$LOGFILE_PFX", "logdirurl": "$logdirurl",
                                  "fwconfig": "$fwconfig", "fwconfigdir": "$fwconfigdir",
                                  "fwgeo": "$fwgeo",
                                  "user": "$CERN_UPN"}\n}, 1);

  return undef unless length($buf);

  # Expect hash response, as { 'port'=> , 'dir'=> , 'key'=> }
  my $resp = eval $buf;
  unless (defined $resp) {
    cgi_print "Failed parsing of server response:", "    $buf";
    return undef;
  }

  if (defined $resp->{'error'}) {
    cgi_print "Server responded with an error:", "    $resp->{'error'}";
    if (defined $resp->{'log_fname'}) {
      print "More information might be available in the <a href=\"$LOGFILE_WWW/$resp->{'log_fname'}\">log file</a>\n";
    }
    print "<p><a href=$BACK_URL >Back to main gateway page</a>\n";
    return undef;
  }

  my $URL = &$PORT_MAP_FOO($resp);

  # For opening on localhost directly.
  # print "xdg-open $URL\n";
  # exec  "xdg-open $URL";

  if ($AUTO_REDIRECT) {
    print "<META HTTP-EQUIV=refresh CONTENT=\"0;URL=$URL\">\n";
    return;
  }


  print<<"FNORD";
<h2>
Your event display is ready, click link to enter:
</h2>
<p>
<a href="$URL">$URL</a>
<p>
<a href="$LOGFILE_WWW/$resp->{'log_fname'}">Log file</a>
<p>
<a href=$BACK_URL>Back to main page</a>
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

if ($CONFIG_ERROR)
{
    cgi_print "Startup error: $CONFIG_ERROR";
    return;
}

# Usage of INET sockets in cgi-bin under SE requires:
#   /usr/sbin/setsebool -P httpd_can_network_connect 1
# Maybe we should use UNIX sockets.

# cgi_print("File=".$q->param('File'));

my @names = $q->url_param();

if ($q->url_param("stop_redirect")) {
   $AUTO_REDIRECT=0;
}

if ($PRINT_URL_ARGS)
{
  print "<p><pre>\n";
  print "N_param = ", scalar(@names), "\n";

  for my $k (@names)
  {
    print "$k: ", $q->url_param($k), "\n";
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

# Remap URL param load_file to POST param "Action" "Load File EOS" but only
# if POST "Action" is not already set.
if ($q->url_param('load_file') && not $q->param('Action'))
{
  $q->param('Action', "Load File EOS");
  $q->param('File',   $q->url_param('load_file'));
}
if ($q->url_param('load_file_aaa') && not $q->param('Action'))
{
  $q->param('Action', "Load File AAA");
  $q->param('File',   $q->url_param('load_file_aaa'));
}


if ($q->url_param('load_sample') && not $q->param('Action'))
{
  my $path = "Load ".$q->url_param('load_sample');
  $q->param('Action',   $path);
}

if ($q->param('Action') =~ m/^Load/)
{
  my $file;

  if ($q->param('Action') =~ m/^Load File (.*)$/)
  {
    my $srcobj = $SOURCES->{$1};

    my $fn_str = $q->param('File');
    $fn_str =~ s/^\s+//;
    $fn_str =~ s/\s+$//;

    cgi_print "Processing '$fn_str'";

    my @files = split(/\s+/, $fn_str);
    my $fcnt = 0;
    $error_str = undef;
    foreach my $fi (@files)
    {
      ++$fcnt;
      if (not ref($srcobj->{'prefix'}))
      {
        if ($fi =~ m!${LFN_RE}!)
        {
          $fi = $srcobj->{'prefix'} . $1;
          cgi_print "$fcnt: $fi";
        }
        else
        {
          $error_str = "filename '$fi' ($fcnt) should match '/store/.../file-name.root'";
          last;
        }
      }
      elsif (ref($srcobj->{'prefix'}) eq 'CODE')
      {
        my $out = &{$srcobj->{'prefix'}}($fi);
        if ($out) {
          $fi = $out;
        } else {
          $error_str = "filename '$fi' ($fcnt) $error_str";
          last;
        }
      }
      else
      {
        $error_str = "wrong source definition, prefix should be a scalar or code ref, is " . ref($srcobj->{'prefix'});
        last;
      }
    }
    $file = join(" ", @files) unless $error_str;
  }
  elsif ($q->param('Action') =~ m/^Load (.+\.root)/)
  {
    $file = "${SAMPLE_DIR}/$1";
  }
  else
  {
    $error_str = "unmatched Action value '$q->param('Action')'";
  }

  if (defined $file)
  {
    start_session($file);
  }
  else
  {
    cgi_print "Error Load: $error_str";
  }
}
elsif ($q->param('Action') eq 'Show Usage')
{
  my $buf = connect_to_server(qq{{"action": "report_usage"}\n}, 0);
  my $r = eval $buf;
  print "Currently serving $r->{current_sessions} (total $r->{total_sessions} since service start).";
  print "<br><br>\n";# Request and show current session, users, run times ... log links for matchin user
  $r->{'table'} =~ s/$ENV{'DOCUMENT_ROOT'}//g;
  print $r->{'table'};
  print "<p><a href=$BACK_URL>Back to main page</a>\n";
}
else
{
  ## DATA ##
  my $shost = ($REDIR_HOST =~ m/.cern.ch$/)  ? "CERN" : "UC San Diego";
  print"<h2 style=\"color:navy\">cmsShowWeb Gateway @ $shost </h2>";
  cgi_print "Hello ${CERN_GName}, choose your action below.";

  print $q->start_form(), "\n";

  # Default hidden submit button to eat up <Enter> presses from textfields
  print $q->submit('Action', "nothing", undef, hidden, "onclick=\"event.preventDefault();\""), "\n";

  print("<h3> Open Event Display </h3>\n");
  print("Enter file name, use white space to chain multiple files:<br>");
  print $q->textfield('File', '', 150, 32767), "\n";
  print "<table>\n";
  print join("\n", map { "<tr><td>" . $q->submit('Action', "Load File $_") . "</td><td>" . $SOURCES->{$_}{'desc'} . "</td></tr>"} (sort keys %$SOURCES));
  print "\n</table>\n";

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
  print "<br><br>\n";


  ## FWC CONFIGURATION ##
  print("<h3>Configuration (optional)</h3>");
  print("Configration is auto loaded relative to the data file content. <br>
  If you choose to use custom configuration, enter name of fireworks configuration file residing in  <a href=\"${CONFIG_WWW}${CERN_UPN}\">${CONFIG_WWW}${CERN_UPN}</a> or URL<br>\n");
  print $q->textfield('FWconfig', '', 150, 512), "\n";

  ## FWC geomtery ##
  print("<h3>Geometry (optional)</h3>");
  print("EOS path to geometry file. <br>");
  print $q->textfield('FWgeo', '', 150, 512), "\n";

  ## STATUS ##
  print "<br><br>\n";

  ## If logfile dir exists, tell user about it.
  if (-e $LOGFILE_PFX) {
    print "Your recent logs are available here: <a href=\"$LOGFILE_WWW\">$LOGFILE_WWW</a>\n";
  }

  {
    my $buf = connect_to_server(qq{{"action": "status"}\n}, 0);
    my $r = eval $buf;
    print "<br>\n";
    print("<h3>Status</h3>");
    print "Currently serving $r->{current_sessions} (total $r->{total_sessions} since service start).";
  }
  print "<br>\n";
  if (grep(/$CERN_UPN/, @ADMINS))
  {
    print $q->submit('Action', "Show Usage");
  }

  print $q->end_form();

  print "<footer>";
  printf "Mail to ";
  print "<a href=\"mailto:cmstalk+visualization\@cern.ch\">cmstalk+visualization\@cern.ch</a></p> ";
  printf "CMS Talk archive ";
  print "<a href=\"https://cms-talk.web.cern.ch/c/offcomp/visualization/165\">Visualization Discussion</a></p>";
  # printf "Known issues: ";
  # print "<a href=\"https://github.com/alja/FireworksWeb/issues\">github issues</a></p>";
  print "</footer>";
}

cgi_end();
