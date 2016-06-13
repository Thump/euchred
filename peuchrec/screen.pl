
# PeuchreC : Perl Euchre Client
#
# (C) Copyright 1999 Denis McLaughlin


###########################################################################
# Routine to display current screen

sub UpdateScreen
{	local($pad,$gamestatus);

	$cname="Perl Euchre Client";
	$cnamel=length $cname;

	$srvprt="Server: $server  Port: $port";
	$srvprtl=length $srvprt;

	$statname="Status: $gstatus  Name: $name";
	$statnamel=length $statname;

	$gamestatus=GameStatus();
	$gamestatusl=length $gamestatus;

	$p0=PlayerName($me);
	$p1=PlayerName($foe1);
	$p2=PlayerName($partner);
	$p3=PlayerName($foe2);

	$p0l=length $p0; $p1l=length $p1; $p2l=length $p2; $p3l=length $p3;

	$p0string=PlayerString($me);
	$p1string=PlayerString($foe1);
	$p2string=PlayerString($partner);
	$p3string=PlayerString($foe2);

	$p0ls=length $p0string; $p1ls=length $p1string;
	$p2ls=length $p2string; $p3ls=length $p3string;

	# we don't set card values if $trickover so that we can see them
	if ( $trickover != 1 )
	{	$p0card=""; $p1card=""; $p2card=""; $p3card="";
		$p0card=Card($me);
		$p1card=Card($foe1);
		$p2card=Card($partner);
		$p3card=Card($foe2);
	}

	$p0lc=hilength($p0card); $p1lc=hilength($p1card);
	$p2lc=hilength($p2card); $p3lc=hilength($p3card);

	$options="";
	$options=OptionsString() if ( $gstatus eq "joined" );
	$optionsl=length $options;

	$trumpstring="";
	$trumpstring=TrumpString() if ( $game{ingame} );
	$trumpstringl=length $trumpstring;

	$choices=Choices();
	$choicesl=hilength($choices);

	# Display the score text if the game has started
	$score="";
	$score= "Score:  Us $game{score}{$players{$me}{team}}  Them $game{score}{$players{$foe1}{team}}" if ( $game{ingame});
	$scorel=length $score;

	$tricks="";
	$tricks= "Tricks: Us $game{tricks}{$players{$me}{team}}  Them $game{tricks}{$players{$foe1}{team}}" if ( $game{ingame});
	$tricksl=length $score;

	# set the help menu based on game state
    $help="q:Quit n:Name";
	$help.=" s:Server p:Port c:Connect" if ($gstatus eq "unconnected");
	$help.=" d:Disconnect" if ($gstatus ne "unconnected");
	$help.=" m:Msg" if ($gstatus eq "joined");
	$help.=" k:Kick" if ($players{$me}{creator});
	$help.=" o:Options" if ($players{$me}{creator} && !$game{ingame});
	$help.=" s:Start" if ($players{$me}{creator} && !$game{ingame}
		&& $game{numplayers}>3);
	$help.=" e:End" if ($players{$me}{creator});
	$pad = " "x(($msgcolumns-length($help))/2);
	$help = "$pad$help$pad\n";
	

	print "[H[J";

	# impending hirsuteness
	print "$cname"," "x($msgcolumns-($cnamel+$srvprtl)),"$srvprt\n";
	#print "$score     Status: $gstatus  Name: $name\n";
	print " "x($msgcolumns-$statnamel),"$statname\n";
	print "-"x$msgcolumns,"\n";
	print "$score"," "x($msgcolumns-($scorel+$optionsl)),"$options\n";
	print "$tricks"," "x($msgcolumns/2-($tricksl+int($p2l/2))),$p2," "x($msgcolumns-($tricksl+(int($msgcolumns/2)-($tricksl+int($p2l/2)))+$p2l+$trumpstringl)),"$trumpstring\n";
	print " "x(($msgcolumns-$p2ls)/2),$p2string,"\n";
	print "\n";
	print " "x(($msgcolumns-$p2lc)/2),$p2card,"\n";
	print "    ",$p1," "x((($msgcolumns/2)-5)-(4+$p1l+$p1lc)),$p1card," "x((($msgcolumns/2)+5)-(4+$p1l+((($msgcolumns/2)-5)-(4+$p1l+$p1lc))+$p1lc)),$p3card," "x($msgcolumns-(8+$p1l+$p1lc+$p3lc+$p3l+((($msgcolumns/2)-5)-(4+$p1l+$p1lc))+((($msgcolumns/2)+5)-(4+$p1l+((($msgcolumns/2)-5)-(4+$p1l+$p1lc))+$p1lc)))),$p3,"\n";
	print "    ",$p1string," "x((($msgcolumns/2)-1)-(4+$p1ls)),$p0card," "x($msgcolumns-(8+$p1ls+((($msgcolumns/2)-1)-(4+$p1ls))+$p0lc+$p3ls)),$p3string,"\n";
	#print "    ",$p1string," "x((($msgcolumns/2)-1)-(4+$p1ls)),$p0card," "x($msgcolumns-(8+$p1ls+((($msgcolumns/2)-1)-(4+$p1ls))+$p0lc+$p3ls)),$p3string,"\n";
	print "\n";
	print " "x(($msgcolumns-$p0l)/2),$p0,"\n";
	print " "x(($msgcolumns-$p0ls)/2),$p0string,"\n";
	print "$gamestatus\n";
	print "-"x$msgcolumns,"\n";
	print " "x(($msgcolumns-$choicesl)/2),$choices,"\n";
	print "-"x$msgcolumns,"\n";
    print "$help";
	print "-"x$msgcolumns,"\n";
};


###########################################################################
# A routine to append a message to the log: $lead is an internal use,
# to allow subsequent lines to be indented

sub AddChat
{
	local($msg)=$_[0];
	local($lead)=$_[1];
	local(@words,$word,$line)=(undef,undef,undef);

	# convert tabs to single spaces: sorry, but variable length characters
	# are just too awkward: note this is a client side limit, not a server
	# limit
	$msg =~ s/\t/ /g;

	# we remove all new lines, then add one at the end
	$msg =~ s/([^ \n])\n([^ \n])/\1 \2/g;
	$msg =~ s/\n//g;
	$msg .= "\n";

	# the simple case
	if ((length($msg)+length($lead)) < 79)
	{
		push @messages,"$lead$msg";
		UpdateScreen();
		ShowChat();
		return;
	}

	# okay, not so simple: split into words
	@words=split / /,$msg;

	# whoa, shit, no spaces at all! cut it in two, add the first, recurse
	if ($#words==0)
	{
		$word=substr $msg,0,78-length($lead);
		push @messages,"$lead$word\n";

		$word=substr $msg,78-length($lead);
		AddChat($word,"   ");
		return;
	}

	# pop words off the end of @words until @words is short enough
	while ((length("@words")+length($lead)) > 79)
	{
		$word=pop @words;
		$line="$word $line";
	}

	# then add @words (now short enough) and call recursively on $line
	push @messages,"$lead@words\n";
	AddChat("$line","   ");
	UpdateScreen();
	ShowChat();
}


###########################################################################
# Prints the appropriate set of messages from the end of @messages, taking
# into account the current value of $msgpos
#
# Done in three cases: $#messages < $msglines -> just print everything

sub ShowChat
{
	local($end,$line);

	$end=$#messages-$msgpos;
	$start=$end-($msglines-1); $start=0 if ($start < 0);
	
	for $i ($start..$end-1)
	{
		print $messages[$i];
	}

	if ($msgpos > 0)
	{	$line="^"x78;
	}
	else
	{	$line=$messages[$end];
	}

	$line =~ s/\n//;
	print $line;
	print "\r";
}


###########################################################################
# Used to show a prompt for further information.  The text prompt is
# expected to have been passed in as $_[0].  To keep things looking
# pretty, we null out any newlines in $_[0].  Because we use sysread()
# on STDIN elsewhere, we need to use a munged up little routine to
# read a line from STDIN here.
#
# GetString() returns with a single line of text, terminated by a
# newline.

sub GetString
{
	local($prompt)=$_[0];
	$prompt =~ s/\n//g;

	print "\n$prompt";

	#normal();
	raw();
	$string="";
	sysread STDIN,$char,1;
	while ($char ne "\n" && $char ne "\r")
	{	if (ord($char) == 127 || ord($char) == 8)
		{	$string=~s/.$//;
			UpdateScreen();
		}

		$string.=$char if (ord($char) => 32 && ord($char) < 127);
		UpdateScreen();
		print "\n$prompt$string";
		sysread STDIN,$char,1;
	}
	noecho();

	$string =~ s/\n//g;
	$string =~ s/$/\n/;

	return $string;
}


###########################################################################
# Used to set a new server

sub GetServer
{
	UpdateScreen();
	$newserver=GetString("Enter server name: ");
	$newserver =~ s/\n//g;

	if ($newserver =~ /^$/)
	{	AddChat("Client: Bad server specification: must enter name.\n");
		return;
	}

	$server=$newserver;
	AddChat("Client: New server is $newserver.\n");
	return;
}


###########################################################################
# Used to set a new port number

sub GetPort
{
	UpdateScreen();
	$newport=GetString("Enter port number: ");
	$newport =~ s/\n//g;

	if ($newport =~ /[^0-9]+/)
	{	AddChat("Client: Bad port specification: only numbers allowed.\n");
		return;
	}

	if ($newport !~ /[0-9]+/)
	{	AddChat("Client: Bad port specification: numbers required.\n");
		return;
	}

	$port=$newport;
	AddChat("Client: New port is $newport.\n");
	return;
}


###########################################################################
# Used to set a new name

sub GetName
{
	UpdateScreen();
	$newname=GetString("Enter name: ");
	$newname =~ s/\n//g;

	if ($newname =~ /^$/)
	{	AddChat("Client: Bad name: must enter name.\n");
		return;
	}

	$name=$newname;
	AddChat("Client: New name is $newname.\n");
	return;
}


###########################################################################
# Used to enter a message

sub GetChat
{
	$chat="";

	if ($gstatus ne "joined")
	{	AddChat("Client: Can't send message until connected.");
		return;
	}

	UpdateScreen();
	$chat=GetString("Enter message: ");
	$chat =~ s/\n//g;

	return;
}


###########################################################################
# Sets the values in a players screen string

sub PlayerString
{	local($pnum) = $_[0];
	local($tmp) = "";


	$tmp .= "creator," if ($players{$pnum}{creator});
	$tmp .= "dealer," if ($players{$pnum}{dealer});
	$tmp .= "maker," if ($players{$pnum}{maker});
	$tmp .= "leader," if ($players{$pnum}{leader});
	$tmp .= "order," if ($players{$pnum}{order});
	$tmp .= "alone," if ($players{$pnum}{alone});
	$tmp .= "defending," if ($players{$pnum}{defend});
	$tmp .= "play?," if ($players{$pnum}{playoffer});
	$tmp .= "order?," if ($players{$pnum}{orderoffer});
	$tmp .= "drop?," if ($players{$pnum}{dropoffer});
	$tmp .= "call?," if ($players{$pnum}{calloffer});
	$tmp .= "defend?," if ($players{$pnum}{defendoffer});

	$tmp =~ s/,$//;
	$tmp = "($tmp)" if ($tmp ne "");

	return($tmp);
}


###########################################################################
#  Sets the options string

sub OptionsString
{	local($options)="Options: ";

	$options.="a" if (  $game{aloneonorder} );
	$options.="-" if ( !$game{aloneonorder} );
	$options.="d" if (  $game{candefend} );
	$options.="-" if ( !$game{candefend} );
	$options.="s" if (  $game{screw} );
	$options.="-" if ( !$game{screw} );

	return($options);
}


###########################################################################
#  Sets the trump string

sub TrumpString
{	local($trumpstring)="";

	$trumpstring = "Clubs" if ( $game{trump} eq "c" );
	$trumpstring = "Diamonds" if ( $game{trump} eq "d" );
	$trumpstring = "Hearts" if ( $game{trump} eq "h" );
	$trumpstring = "Spades" if ( $game{trump} eq "s" );
	#$trumpstring = "<not set>" if ( $trumpstring eq "" );
	$trumpstring = "<not set>" if ( !$game{trumpset} );

	return("Trump: $trumpstring");
}


###########################################################################
# Prints a player structure 

sub PrintPlayer
{	local($pnum)=$_[0];

	AddChat("state is $players{$pnum}{state}");
	AddChat("ph is $players{$pnum}{ph}");
    AddChat("name is $players{$pnum}{name}");
    AddChat("clientname is $players{$pnum}{clientname}");
    AddChat("hw is $players{$pnum}{hardware}");
    AddChat("os is $players{$pnum}{os}");
    AddChat("comment is $players{$pnum}{comment}");
    AddChat("team is $players{$pnum}{team}");
    AddChat("numcards is $players{$pnum}{numcards}");
    AddChat("creator? $players{$pnum}{creator}");
    AddChat("ordered? $players{$pnum}{ordered}");
    AddChat("dealer? $players{$pnum}{dealer}");
    AddChat("alone? $players{$pnum}{alone}");
    AddChat("defending? $players{$pnum}{defend}");
    AddChat("leading? $players{$pnum}{leader}");
    AddChat("play? $players{$pnum}{playoffer}");
    AddChat("order? $players{$pnum}{orderoffer}");
    AddChat("call? $players{$pnum}{calloffer}");
    AddChat("defend? $players{$pnum}{defendoffer}");
    AddChat("cardinplay? $players{$pnum}{cardinplay}");
    AddChat("card $players{$pnum}{card}");
}


###########################################################################
# sets the value for a players name intelligently

sub PlayerName
{	local($pnum)=$_[0];
	local($tmp);

	$tmp = "$players{$pnum}{name} ($players{$pnum}{numcards})"
		if ($players{$pnum}{state} eq "joined");

	$tmp =~ s/ *\(\) *//;

	return($tmp);
}


###########################################################################
# sets the value for the game status line

sub GameStatus
{
	return("Waiting to connect...")
		if ( $gstatus eq "unconnected");

	return("Waiting for players...")
		if ( ! $game{ingame} && $game{numplayers} < 4);

	return("Waiting for creator to start...")
		if ( ! $game{ingame} && $game{numplayers} == 4);

	return("Offer to order trump...")
		if ( $game{hstate} == 1);

	return("Offer to call trump...")
		if ( $game{hstate} == 2);

	return("Offer to defend alone...")
		if ( $game{hstate} == 3);

	return("Playing...")
		if ( $game{hstate} == 4);

	return("Unknown game state");
}


###########################################################################
# Used to specify a player, probably to kick them off

sub GetPlayer
{	local($targetph)="";
	local($player)="";
	local($pnum);

	UpdateScreen();
	$player=GetString("Player to kick: ");
	$player =~ s/\n//g;

	for ($pnum=0; $pnum<4; $pnum++)
	{	if ($players{$pnum}{state} eq "joined")
		{	if ("$players{$pnum}{name}" eq "$player")
			{	$targetph=$players{$pnum}{ph};
			}
		}
	}

	if ($targetph eq "")
	{	AddChat("Client: Can't find player $player");
		return("");
	}

	return($targetph);
}


###########################################################################
# Gets options, returning them in an array

sub GetOptions
{	local($candefend,$aloneonorder,$screw);

	UpdateScreen();
	$aloneonorder=GetAnswer("Do players go alone after ordering their partner?",
		$game{aloneonorder});

	UpdateScreen();
	$candefend=GetAnswer("Can players defend alone?",$game{candefend});

	UpdateScreen();
	$screw=GetAnswer("Is screw the dealer in effect?",$game{screw});

	return($candefend,$aloneonorder,$screw);
}


###########################################################################
# Used to show a yes/no prompt.  The current value of the variable is passed
# as the second argument, and is used to compose the default, either [Y/n]
# or [y/N].  The only way to change the default value is to press the letter
# (case insensitive) corresponding to the opposite: all other keys will
# keep the current value.

sub GetAnswer
{
	local($prompt)=$_[0];
	local($curval)=$_[1];
	$prompt =~ s/\n//g;

	$prompt .= " [Y/n] " if ($curval);
	$prompt .= " [y/N] " if (!$curval);

	print "'n' to change the value, anything else to leave it unchanged\n"
		if ($curval);
	print "'y' to change the value, anything else to leave it unchanged\n"
		if (!$curval);
	print "\n$prompt";

	sysread STDIN,$char,1;
	$char =~ tr /A-Z/a-z/;

	return(0) if ($char eq "n" && $curval);
	return(1) if ($char eq "y" && !$curval);
	return(1) if ($curval);
	return(0) if (!$curval);
}


###########################################################################
# This routine computes the whether anything should be displayed for that
# players card-in-play string

sub Card
{	local($pnum)=$_[0];

	if ($game{hstate}==1)
	{	if ($players{$pnum}{dealer})
		{	return($game{hole});
		}
		else
		{	return("");
		}
	}

	if ($game{hstate}==4)
	{	return($players{$pnum}{card})
			if ( $players{$pnum}{cardinplay} == 1 && $trickover == 0);
	}
}


###########################################################################
# A routine to return a string of highlighted text

sub hi
{	return "$hion$_[0]$hioff";
}


###########################################################################
# A routine that returns the current choice string

sub Choices
{	local($cards);

	return "" if ( !$game{ingame});
	$cards=$players{$me}{cards};

	# continue, after trick
	return("Cards: ".$cards."    ".
		HiChoices(" ","Continue"))
		if ( $trickover == 1 );

	# order the hole card up if your partner is the dealer and screw is on
	return("Cards: ".$cards."    ".
		HiChoices(" : ","Pass","Order alone"))
		if ($players{$me}{orderoffer} && $players{$partner}{dealer} &&
			$game{aloneonorder} );

	# order the hole card up
	return("Cards: ".$cards."    ".
		HiChoices(" : ","Pass","Order","Order alone"))
		if ( $players{$me}{orderoffer} && !$players{$me}{dealer});

	# pick up the hole card
	return("Cards: ".$cards."    ".
		HiChoices(" : ","Pass","Pick up","Pick up alone"))
		if ( $players{$me}{orderoffer} && $players{$me}{dealer});

	# having been ordered or having picked up, drop a card
	return("Drop Card: ".
		HiChoices(" ",split ' ',$cards))
		if ( $players{$me}{dropoffer});

	# call a trump if you're not the dealer
	return("Cards: ".$cards."    ".
		HiChoices(" : ","Pass","Call","Call alone"))
		if ( $players{$me}{calloffer} && !$game{callstate}
			&& !$players{$me}{dealer} );

	# call a trump if you're a dealer and screw the dealer is set
	return("Cards: ".$cards."    ".
		HiChoices(" : ","Call","Call alone"))
		if ( $players{$me}{calloffer} && !$game{callstate}
			&& $players{$me}{dealer} && $game{screw} );

	# call a trump if you're a dealer and screw the dealer is not set
	return("Cards: ".$cards."    ".
		HiChoices(" : ","Fold hand","Call","Call alone"))
		if ( $players{$me}{calloffer} && !$game{callstate}
			&& $players{$me}{dealer} && !$game{screw} );

	# choose a suit to call
	return("Cards: ".$cards."    ".
		HiChoices(" : ",CallSuits()))
		if ( $players{$me}{calloffer} && $game{callstate});

	# defend or not defend
	return("Cards: ".$cards."    ".
		HiChoices(" : ","Pass","Defend alone"))
		if ( $players{$me}{defendoffer});

	# choose a card to play
	return("Play Card: ".
		HiChoices(" ",split ' ',$cards))
		if ( $players{$me}{playoffer});

	return ("Cards: $cards") if ( $game{ingame});
}


###########################################################################
# This routine returns a list of choices, appropriately highlighted.

sub HiChoices
{	local($i,$chc);
	local($result)="";
	local($separator)=shift(@_);

	$i=0;
	$choice=$#_ if ( $choice > $#_);

	for $chc (@_)
	{	if ($i == $choice)
		{	$result .= hi($chc);
			$choicetext=$chc;
		}
		else
		{	$result .= $chc;
		}

		$result .= $separator;
		$i++;
	}

	$result =~ s/[ :]*$//;
	return($result);
}


###########################################################################
# This routine returns the list of valid suits that can be called

sub CallSuits
{	local($suit,@result);

	for $suit ("c","d","h","s")
	{	push @result,SuitToText($suit) if ( $suit ne SuitOf($game{hole}) );
	}
	push @result,"Cancel";

	return(@result);
}


###########################################################################
# Changes the suit string to a name

sub SuitToText
{	return("Clubs") if ($_[0] eq "c");
	return("Diamonds") if ($_[0] eq "d");
	return("Hearts") if ($_[0] eq "h");
	return("Spades") if ($_[0] eq "s");
	return("Unknown Suit");
}


###########################################################################
# Changes the suit name to a suit type

sub TextToType
{	return(0) if ($_[0] eq "Clubs");
	return(1) if ($_[0] eq "Diamonds");
	return(2) if ($_[0] eq "Hearts");
	return(3) if ($_[0] eq "Spades");
	return(-1);
}


###########################################################################
# takes a card of the form "6h" and returns "h"

sub SuitOf
{	local($card)=$_[0];

	$card =~ s/^.*(.)$/\1/g;
	return($card);
}


###########################################################################
# takes a card of the form "6h" and returns the value and suit integers,
# appropriate to be sent in a network message

sub CardToType
{	local($card)=$_[0];

	($value=$card) =~ s/^(.*).$/\1/g;
	$value=11 if ($value eq "J");
	$value=12 if ($value eq "Q");
	$value=13 if ($value eq "K");
	$value=14 if ($value eq "A");

	($suit=$card) =~ s/^.*(.)$/\1/g;
	$suit=0 if ($suit eq "c");
	$suit=1 if ($suit eq "d");
	$suit=2 if ($suit eq "h");
	$suit=3 if ($suit eq "s");

	return($value,$suit);
}


###########################################################################
# This routine dynamically resizes the screen

sub Winch
{	
	$msglines=6;
	$msglines=$ENV{LINES}-19
		if ( defined($ENV{LINES}) );

	$msgcolumns=79;
	$msgcolumns=$ENV{COLUMNS}-1
		if ( defined($ENV{COLUMNS}) );
}


###########################################################################
# This routine returns the length of a string, discounting any hilighting
# characters

sub hilength
{	local($string)=$_[0];

	$string =~ s/\[K//;
	$string =~ s/\[7m//;
	$string =~ s/\[m//;
	$string =~ s/\[27m//;

	return(length($string));
}


###########################################################################
1;
