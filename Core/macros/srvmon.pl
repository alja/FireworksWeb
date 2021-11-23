#!/usr/bin/perl

use IO::Socket qw(AF_INET SOCK_STREAM);
use IO::Socket::Timeout;

$EMAIL = 'mtadel@ucsd.edu, amraktadel@ucsd.edu';

$SERVER_GREETING = "Hello, this is Revetor! What do you want?";

$EVE_HOST   = "localhost";
$EVE_PORT   =  6666;

$IS_TEST = $0 =~ m/-test.pl$/;

if ($IS_TEST)
{
  $EVE_PORT    =  6669;
}

sub cgi_print
{
  print join("\n", @_), "\n";
}

sub cgi_die
{
  my $err = shift;
  my $desc = shift;

  cgi_print "Error detected.", "Short error: $err", "Desription: $desc", "Sending mail to $EMAIL";

  # XXX restart, how ???

  # Send mail
  my $host = `hostname`;

  open(MAIL, "|/usr/sbin/sendmail -t");
  print MAIL <<"FNORD";
Subject: Service error
To: $EMAIL
From: srvmon\@$host

host:     $host
program:  $0
EVE_HOST: $EVE_HOST
EVE_PORT: $EVE_PORT

Short error: $err
Desription:  $desc

Have a nice day!
FNORD
  close(MAIL);

  exit(0);
}

sub flush
{
  select()->flush();
}


################################################################################
# Connect
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
  ) || cgi_die "SOCKET_NEW", "Can't open socket: $@";

  IO::Socket::Timeout->enable_timeouts_on($client);

  cgi_print "Connected to $EVE_PORT" if $verbose;

  $buf = recv_with_timeout($client, 1024, 5, 20);

  unless (length($buf)) {
    my $err_str = $!;
    cgi_die("SERVER_CONNECT", "greeting buffer not defined -- $err_str");
  }
  cgi_print "Server greeting: $buf" if $verbose;
  if ($buf ne $SERVER_GREETING) {
    cgi_die("SERVER_GREETING", "Expected '$SERVER_GREETING', got '$buf'");
  }

  cgi_print "Sending $request" if $verbose;

  my $size = $client->send($request);
  cgi_print "Sent data of length: $size" if $verbose;
  flush();

  $buf = recv_with_timeout($client, 1024, 5, 100);
  unless (length($buf)) {
    my $err_str = $!;
    cgi_die("SERVER_RESPONSE", "Error receiving server response, error: ${err_str}.");
  }
  cgi_print "Server response: $buf" if $verbose;

  $client->close();

  return $buf;
}


################################################################################
# main
################################################################################

my $buf = connect_to_server(qq{{"action": "status"}\n}, 1);
my $r = eval $buf;

cgi_print "Currently serving $r->{current_sessions} (total $r->{total_sessions} since service start).";

# XXX Error condition on too many connections?

exit(0);
