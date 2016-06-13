
# PeuchreC : Perl Euchre Client
#
# (C) Copyright 1999 Denis McLaughlin


###########################################################################
# Top level config routine, calls the others

sub Config
{
	# configure some local variables before game and argument parsing
	ConfigVars();

	# parse and interpret out command line
	ParseArgs();

	# set the screen up
	UpdateScreen();

	# set signal catchers
	ConfigSignals();

	# and unbuffer stdout, so last lines show up
	$|=1;
}


###########################################################################
# I wrote my own arg processor, because Perl's Getopt and getopts
# are pieces of shit: why don't they propagate $errs?

sub ParseArgs
{
	local($numarg)=$#ARGV;

	for ($i=0; $i<=$numarg; $i++)
	{	Usage() if ($ARGV[$i] =~ /^-h/);

		if ($ARGV[$i] =~ /^-s/)
		{
			($server=$ARGV[$i]) =~ s/^..//e;

			if ($server eq "")
			{	if ($i == $numarg || $ARGV[$i+1] =~ /^-/)
				{	print "error: -s requires an argument\n";
					Usage();
				}
				else
				{	$server=$ARGV[++$i];
				}
			}
			next;
		}

		if ($ARGV[$i] =~ /^-p/)
		{
			($port=$ARGV[$i]) =~ s/^..//e;

			if ($port eq "")
			{	if ($i == $numarg || $ARGV[$i+1] =~ /^-/)
				{	print "error: -p requires an argument\n";
					Usage();
				}
				else
				{	$port=$ARGV[++$i];
				}
			}
			next;
		}

		print "error: unknown argument $ARGV[$i]\n";
		Usage();
	}
}


###########################################################################
# The usage string

sub Usage
{
	print "
 Usage: peuchrec [-h] [-s <server>] [-p <port]
	-h <port>   : prints this usage string
	-s <server> : sets the game server to connect to
	-p <port>   : sets the port to connect to the server on\n\n";

	exit(-1);
}

###########################################################################
# Sets signal catchers

sub ConfigSignals
{
	$SIG{INT}="SignalCatcher";
	$SIG{HUP}="SignalCatcher";
	$SIG{PIPE}="IGNORE";
}


###########################################################################
# This is in a BEGIN subroutine, so this stuff gets evaled very first.
# It sets stuff up for the tty, hopefully in a reasonably platform
# independent way.
#
# Once this has been loaded, raw() sets raw mode (with echo off), normal()
# sets normal (with echo on), and noecho() sets normal with echo off.

BEGIN
{
	use POSIX qw(:termios_h);
	my ($term,$normal,$echo,$noecho,$cooked,$fd_stdin);

	$fd_stdin=fileno(STDIN);

	$term=POSIX::Termios->new();
	$term->getattr($fd_stdin);

	$echo=ECHO|ECHOK;
	$cooked=ICANON;
	$normal=$term->getlflag()&$echo;
	$raw=$normal&~$echo&~$cooked;
	$noecho=$normal&~$echo;

	sub raw
	{
		$term->setlflag($raw);
		$term->setcc(VTIME,1);
		$term->setattr($fd_stdin,TCSANOW);
	}

	sub normal
	{
		$term->setlflag($normal);
		$term->setcc(VTIME,0);
		$term->setattr($fd_stdin,TCSANOW);
	}

	sub noecho
	{
		$term->setlflag($noecho);
		$term->setcc(VTIME,0);
		$term->setattr($fd_stdin,TCSANOW);
	}
}


###########################################################################
# The configuration section for various global variables.

sub ConfigVars
{
	# names array
	@names=("Thump","Kablooie","Suspence","Minx","WhupAss","Mumbles","Usul","Chris","Grant","Laura","Brad","Leslie","Bonnie","NSDev","Sigbur","Neal","Mary","Aiden","Dave","Thorn","Thistle","Rob","Ruvinder","Ciaran","Neema","Terry","Tina","Louise","Nina","Maeve","Cormac","Nikos");

	# this is the socket connected to the server
	$S=undef;

	# default server and port
	$server="127.0.0.1";  # default server to connect to
	$port=1234;          # default port to connect to on the server

	# protocol version
	$protocol=1;

	# game and player handles
	$gh=0;
	$ph=0;

	# local player name
	#$name="Player";
	$name=Choose(@names);

	# overall game status
	$gstatus="unconnected";

	# @messages is the messages array, displayed on the screen
	# $msgpos is an integer number: lines up to ($#messages-$msgpos)
	# are displayed.  Thus $msgpos=0 means display the most recent
	# messages, $msgpos=1 displays all but the most recent message,
	$msgpos=0;
	Winch();
	if ( $ENV{TERM} eq "linux" )
	{	$hion="]K[7m";
		$hioff="[27m";
	}
	if ( $ENV{TERM} eq "xterm" )
	{	$hion="[7m";
		$hioff="[m";
	}

	# Arrow keys are multi-character keys.  Ick.  For the Linux console,
	# they consist of ^[[A, ^[[B, ^[[C, ^[[D for up, down, right, and left,
	# respectively.  $state starts at 0, moves to 1 on ^[, 2 on [,
	# and then the commands kick on A, B, C, or D.  Reset to 0 on anything
	# else, or on successful completion.
	$arrowstate=0;

	# sets the client, OS, hardware, and comment string
	$os=`uname -rs`; $os =~ s/\n//;
	$hw=`uname -m`; $hw=~ s/\n//;
	$client="PeuchreC 0.9"; $client=~ s/\n//;
	$comment="Going alone and kicking ass.";

	# tail
    $tail=64222;

	# clear player records: we don't set the state attribute, since we
	# also want to use ClearPlayer() when state is 
	ClearPlayer(0,"unconnected");
	ClearPlayer(1,"unconnected");
	ClearPlayer(2,"unconnected");
	ClearPlayer(3,"unconnected");

	ClearGame();

	# me, partner, opponent 1, and opponent 2 pointers
	$me=""; $partner=""; $foe1=""; $foe2="";
	$numplayers=0;

	# to hold which team I'm on
	$team="";

	# keyboard magic, don't touch
	$raw=1;

	# tracks the current choice: numeric value in positions from the left
	$choice=0;
	$choicetext="";
	$trickover=0;
}


###########################################################################
# clears a player record

sub ClearPlayer
{	local($pnum)=$_[0];
	local($state)=$_[1];

	$players{$pnum}{state}=$state;
	$players{$pnum}{ph}="";
	$players{$pnum}{name}="";
	$players{$pnum}{clientname}="";
	$players{$pnum}{hardware}="";
	$players{$pnum}{os}="";
	$players{$pnum}{comment}="";
	$players{$pnum}{team}="";
	$players{$pnum}{numcards}=0;
	$players{$pnum}{creator}=0;
	$players{$pnum}{ordered}=0;
	$players{$pnum}{dealer}=0;
	$players{$pnum}{alone}=0;
	$players{$pnum}{defend}=0;
	$players{$pnum}{leader}=0;
	$players{$pnum}{playoffer}=0;
	$players{$pnum}{orderoffer}=0;
	$players{$pnum}{dropoffer}=0;
	$players{$pnum}{calloffer}=0;
	$players{$pnum}{defendoffer}=0;
	$players{$pnum}{cardinplay}=0;
	$players{$pnum}{card}="";

	$foe1="" if ("$foe1" eq "$pnum");
	$foe2="" if ("$foe2" eq "$pnum");
	$partner="" if ("$partner" eq "$pnum");
	$me="" if ("$me" eq "$pnum");
}


###########################################################################
# Clears the game structure

sub ClearGame
{	$game{ingame}=0;
	$game{suspend}=0;
	$game{holein}=0;
	$game{hole}=-1;
	$game{trumpset}=-1;
	$game{trump}=-1;
	$game{tricks}{1}=0;
	$game{tricks}{2}=0;
	$game{score}{1}=0;
	$game{score}{2}=0;
	$game{candefend}=1;
	$game{aloneonorder}=1;
	$game{screw}=1;
	$game{numplayers}=0;
	$game{callstate}=0;
	$trickover=0;
}


###########################################################################
# Randomly returns one element of the array it is passed

sub Choose
{	return($_[rand($#_)]);
}


###########################################################################
1;
