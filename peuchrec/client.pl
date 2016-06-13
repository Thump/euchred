
# PeuchreC : Perl Euchre Client
#
# (C) Copyright 1999 Denis McLaughlin


###########################################################################
# Reads in and processes information from STDIN

sub ServiceSTDIN
{
	sysread STDIN,$cmd,1;

	# arrow key state machine first, to simplify state exit
	if ($cmd eq "")
	{	$arrowstate=1;
		return;
	}
	if ( ($cmd eq "[" || $cmd eq "O") && $arrowstate==1)
	{	$arrowstate=2;
		return;
	}
	if ($arrowstate==2)
	{	UpArrow() if ($cmd eq "a" || $cmd eq "A");
		DownArrow() if ($cmd eq "b" || $cmd eq "B");
		RightArrow() if ($cmd eq "c" || $cmd eq "C");
		LeftArrow() if ($cmd eq "d" || $cmd eq "D");

		$arrowstate=0;
		return;
	}

	$arrowstate=0;
	Exit() if ($cmd eq "q" || ord($cmd) == 4);

	if ($cmd eq "c")
	{	if ($gstatus eq "unconnected")
		{	Connect();
			if ($gstatus eq "connected")
			{	AddChat("Client: Connected to server, sending join request.");
				SendJoin();
			}
		}
		return;
	}

	# allows user to reset name
	if ($cmd eq "n")
	{	GetName();
		SendID() if ($gstatus eq "joined");
		return;
	}

	# allows user to reset target port
	if ($cmd eq "p")
	{	GetPort() if ($gstatus eq "unconnected");
		return;
	}

	# message get and send
	if ($cmd eq "m")
	{	if ($gstatus eq "joined")
		{	GetChat();
			SendChat($chat) if ($chat ne "");
		}
		return;
	}

	# scroll to the bottom
	if ($cmd eq " ")
	{	$msgpos=0;
		return;
	}

	# test routine, not for normal use
	if ($cmd eq "t")
	{	test();
		return;
	}

	# fast, ungraceful exit
	if ($cmd eq "Q")
	{	exit();
	}

	# disconnect, no exit
	if ($cmd eq "d")
	{	Disconnect() if ($gstatus ne "unconnected");
		return;
	}

	# refresh screen
	if (ord($cmd) == 12)
	{	UpdateScreen();
		ShowChat();
		return;
	}

	# kick players
	if ($cmd eq "k")
	{	if ($players{$me}{creator})
		{	$targetph=GetPlayer();
			SendKick($targetph) if ($targetph ne "");
		}
		return;
	}

	# set and send options
	if ($cmd eq "o")
	{	SendOptions(GetOptions()) if ($players{$me}{creator} && !$game{ingame});
		return;
	}

	# allows user to reset target server, and also used to start the game:
	# the functions are orthogonal enough to allow overloading
	if ($cmd eq "s")
	{	GetServer() if ($gstatus eq "unconnected");
		SendStart() if ($players{$me}{creator} && !$game{ingame}
        	&& $game{numplayers}>3);
		return;
	}

	# client (creator) wishes to end the game
	if ($cmd eq "e")
	{	SendEnd() if ($players{$me}{creator});
		return;
	}

	# send selection
	if ($cmd eq "" || ord($cmd) == 10)
	{	SendChoice();

		# we track $oldchoice so we can restore choice if denied 
		$oldchoice=$choice;
		$choice=0;

		return;
	}

	CmdError($cmd);
	$cmd="";
}


###########################################################################
# Reads in and processes information from the server

sub ServiceS
{
	$len=ReadInt(); # we disregard this: a robust client wouldn't
	$cmd=ReadInt();

	if ($cmd eq "")
	{	ServerDie();
		return;
	}

	if ($cmd == $JOINACCEPT)
	{	JoinAccept();
		return;
	}

	if ($cmd == $JOINDENY)
	{	JoinDeny();
		return;
	}

	if ($cmd == $SERVERQUIT)
	{	ServerQuit();
		return;
	}

	if ($cmd == $DECLINE)
	{	ServerDecline();
		return;
	}

	if ($cmd == $KICK)
	{	Kick();
		return;
	}

	if ($cmd == $CHAT)
	{	Chat();
		return;
	}

	if ($cmd == $STATE)
	{	State();
		return;
	}

	if ($cmd == $KICKDENY)
	{	KickDeny();
		return;
	}

	if ($cmd == $OPTIONSDENY)
	{	OptionsDeny();
		return;
	}

	if ($cmd == $STARTDENY)
	{	StartDeny();
		return;
	}

	if ($cmd == $ENDDENY)
	{	EndDeny();
		return;
	}

	if ($cmd == $ORDERDENY)
	{	OrderDeny();
		return;
	}

	if ($cmd == $DROPDENY)
	{	DropDeny();
		return;
	}

	if ($cmd == $CALLDENY)
	{	CallDeny();
		return;
	}

	if ($cmd == $PLAYDENY)
	{	PlayDeny();
		return;
	}

	if ($cmd == $TRICKOVER)
	{	TrickOver();
		return;
	}

	if ($cmd == $HANDOVER)
	{	HandOver();
		return;
	}

	if ($cmd == $GAMEOVER)
	{	GameOver();
		return;
	}

	if ($cmd == $PLAYOFFER)
	{	PlayOffer();
		return;
	}

	if ($cmd == $DEFENDOFFER)
	{	DefendOffer();
		return;
	}

	if ($cmd == $CALLOFFER)
	{	CallOffer();
		return;
	}

	if ($cmd == $ORDEROFFER)
	{	OrderOffer();
		return;
	}

	if ($cmd == $DROPOFFER)
	{	DropOffer();
		return;
	}

	if ($cmd == $DEAL)
	{	Deal();
		return;
	}

	ServerError($cmd);
}


###########################################################################
# Responds with an error message to an unknown command

sub CmdError
{
	AddChat("Client: Unknown keyboard command: $cmd, ord ".ord($cmd)."\n");
}


###########################################################################
# Responds with an error message to an unknown server message

sub ServerError
{
	AddChat("Client: Unknown server message: $cmd.\n");
	Disconnect();
}


###########################################################################
# The up arrow command: message buffer scroll back

sub UpArrow
{
	$msgpos++;
	$msgpos=($#messages-$msglines)+1 if ($msgpos > ($#messages-$msglines)+1);
}


###########################################################################
# The down arrow command: message buffer scroll forward

sub DownArrow
{
	$msgpos--;
	$msgpos=0 if ($msgpos < 0);
}


###########################################################################
# The left arrow command: choice move left

sub LeftArrow
{
	$choice--;
	$choice=0 if ($choice < 0);
}


###########################################################################
# The right arrow command: choice move right

sub RightArrow
{
	$choice++;
}


###########################################################################
# Processes a join accept: get the length, the game handle and the
# the player handle

sub JoinAccept
{
	# <JOINACCEPT> <gh> <ph> <team> <tail>
	$gh=ReadInt();
	$ph=ReadInt();
	$team=ReadInt();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in join accept.") if ($junk != $tail);

	$gstatus="joined";
	AddChat("Client: We're team $team!");
	SendID();
}


###########################################################################
# Processes a join deny: get the length, and then the reason

sub JoinDeny
{
	# <JOINDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();

	AddChat("Client: Hmmm, missing tail in join deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us join, saying: \"$reason\"\n");
	Disconnect();
}


###########################################################################
# Processes a server quit: shut down the client and exit

sub ServerQuit
{
	# <SERVERQUIT> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();

	AddChat("Client: Hmmm, missing tail in server quit.") if ($junk != $tail);

	AddChat("Client: Server is exiting, saying: \"$reason\"\n");
	Disconnect();
}


###########################################################################
# Processes a server decline: shut down the client and exit

sub ServerDecline
{
	# <DECLINE> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();

	AddChat("Client: Hmmm, missing tail in decline.") if ($junk != $tail);

	AddChat("Client: Server is declining us, saying: \"$reason\"\n");
	Disconnect();
}


###########################################################################
# Processes a server kick: read it, display the message, disconnect

sub Kick
{
	# <KICK> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();

	AddChat("Client: Hmmm, missing tail in kick.") if ($junk != $tail);

	AddChat("Client: Server is kicking us off, saying: \"$reason\"\n");
	Disconnect();
}


###########################################################################
# Processes a server chat message: read it and display the message

sub Chat
{
	local($chat);

	# <CHAT> <string> <tail>
	$chat=ReadString();
	$junk=ReadShort();

	AddChat("Client: Hmmm, missing tail in chat.") if ($junk != $tail);

	AddChat("$chat");
}


###########################################################################
# Processes a state message

sub State
{	local($chat,$pnum);

	# <STATE> <playersdata> <gamedata> <cards> <tail> 

	for ($i=0; $i<4; $i++)
	{	ReadPlayer($i);
	}
	SetTeams();

	# <gamedata> : <ingame> <suspend> <holein> <hole> <trumpset> <trump>
	#              <tricks> <score> <options>
	#   <ingame> : <boolean>
	#   <hstate> : <0|1|2|3|4> # pregame, hole, trump, defend, play
	#   <suspend> : <boolean>
	#   <holein> : <boolean> # true if hole card
	#   <hole> : <card> # only packed if <holein> true
	#     <card> : <value> <suit>
	#   <trumpset> : <boolean> # true if trump set
	#   <trump> : <suit> # only packed if <trumpset> true
	#   <tricks> : <tricks0> <tricks1>
	#     <tricks0> : # trickse for team 0
	#     <tricks1> : # trickse for team 1
	#   <score> : <team0> <team1>
	#     <team0> : # score of team 0
	#     <team1> : # score of team 1
	#   <options> : <alone> <defend> <aloneonorder> <screw>
	#     <alone>|<defend>|<aloneonorder>|<screw> : <boolean>

	$game{ingame}=ReadBoolean();
	$game{hstate}=ReadInt();
	$game{suspend}=ReadBoolean();
	$game{holein}=ReadBoolean();
	$game{hole}=ReadCard() if ($game{holein});
	$game{trumpset}=ReadBoolean();
	$game{trump}=ReadSuit() if ($game{trumpset});
	$game{tricks}{1}=ReadInt();
	$game{tricks}{2}=ReadInt();
	$game{score}{1}=ReadInt();
	$game{score}{2}=ReadInt();
	$game{candefend}=ReadBoolean();
	$game{aloneonorder}=ReadBoolean();
	$game{screw}=ReadBoolean();

	$players{$me}{cards}="";
	$players{$me}{numcards}=ReadInt();
	for ($i=0; $i<$players{$me}{numcards}; $i++)
	{	$players{$me}{cards} .= ReadCard()." ";
	}

	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in chat.") if ($junk != $tail);
}


###########################################################################
# Reads a player state structure off the socket

sub ReadPlayer
{	local($pnum)=$_[0];

	# <pN> : <pstate> <pdata>
	#   <pstate> : {0|1|2} # unconnected, connected, joined
	#   <pdata> : if <pstate> == joined
	#               <ph> <nmstring> <clstring> <hwstring> <osstring>
	#               <cmtstring> <team> <numcards> <creator> <ordered>
	#               <madeit> <alone> <defend> <offer>
	#      <team> : {-1|0|1} # no team, team 0, or team 1
    #      <creator>|<ordered>|<dealer>|<alone>|<defend>|<lead>|<maker>
    #      <playoffer>|<orderoffer>|<dropoffer>|<calloffer>|<defendoffer>
	#             : <boolean>

	$junk=ReadInt();
	$players{$pnum}{state}="unconnected" if ($junk == 0);
	$players{$pnum}{state}="connected" if ($junk == 1);
	$players{$pnum}{state}="joined" if ($junk == 2);

	if ( $players{$pnum}{state} ne "joined" )
	{	ClearPlayer($pnum,$players{$pnum}{state});
		return;
	}

	$players{$pnum}{ph}=ReadInt();
	$players{$pnum}{name}=ReadString();
	$players{$pnum}{clientname}=ReadString();
	$players{$pnum}{hardware}=ReadString();
	$players{$pnum}{os}=ReadString();
	$players{$pnum}{comment}=ReadString();
	$players{$pnum}{team}=ReadInt();
	$players{$pnum}{numcards}=ReadInt();
	$players{$pnum}{creator}=ReadBoolean();
	$players{$pnum}{ordered}=ReadBoolean();
	$players{$pnum}{dealer}=ReadBoolean();
	$players{$pnum}{alone}=ReadBoolean();
	$players{$pnum}{defend}=ReadBoolean();
	$players{$pnum}{leader}=ReadBoolean();
	$players{$pnum}{maker}=ReadBoolean();
	$players{$pnum}{playoffer}=ReadBoolean();
	$players{$pnum}{orderoffer}=ReadBoolean();
	$players{$pnum}{dropoffer}=ReadBoolean();
	$players{$pnum}{calloffer}=ReadBoolean();
	$players{$pnum}{defendoffer}=ReadBoolean();
	$players{$pnum}{cardinplay}=ReadBoolean();
	$players{$pnum}{card}=ReadCard() if ($players{$pnum}{cardinplay});
	$players{$pnum}{passed}=ReadBoolean();
}


###########################################################################
# Processes a kick deny: get the length, and then the reason

sub KickDeny
{
	# <JOINDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in kick deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us kick users, saying: \"$reason\"\n");
}


###########################################################################
# Processes an options deny: get the length, and then the reason

sub OptionsDeny
{
	# <OPTIONSDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in options deny.")
		if ($junk != $tail);

	AddChat("Client: The server won't let us change options, saying: \"$reason\"\n");
}


###########################################################################
# Processes a start deny: get the length, and then the reason

sub StartDeny
{
	# <STARTDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in start deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us start the game, saying: \"$reason\"\n");
}


###########################################################################
# Processes an end deny: get the length, and then the reason

sub EndDeny
{
	# <ENDDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in end deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us end the game, saying: \"$reason\"\n");
}


###########################################################################
# Processes an order deny: get the length, and then the reason

sub OrderDeny
{
	# <ORDERDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in order deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us order, saying: \"$reason\"\n");
	$choice=$oldchoice;
}


###########################################################################
# Processes a drop deny: get the length, and then the reason

sub DropDeny
{
	# <DROPDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in order deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us drop, saying: \"$reason\"\n");
	$choice=$oldchoice;
}


###########################################################################
# Processes a call deny: get the length, and then the reason

sub CallDeny
{
	# <CALLDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in call deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us pass, saying: \"$reason\"\n");
	$choice=$oldchoice;
}


###########################################################################
# Processes a play deny: get the length, and then the reason

sub PlayDeny
{
	# <PLAYDENY> <string> <tail>
	$reason=ReadString();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in play deny.") if ($junk != $tail);

	AddChat("Client: The server won't let us play, saying: \"$reason\"\n");
	$choice=$oldchoice;
}


###########################################################################
# Processes a trick over: pause for a second

sub TrickOver
{
	# <TRICKOVER> <tail>
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in TrickOver().") if ($junk != $tail);

	$trickover=1;
}


###########################################################################
# Processes a hand over: the peuchrec client doesn't do anything, other
# clients may want some kind of hand-completion animation

sub HandOver
{
	# <HANDOVER> <tail>
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in HandOver().") if ($junk != $tail);
}


###########################################################################
# Processes a game over: don't do anything

sub GameOver
{
	# <GAMEOVER> <tail>
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in GameOver().") if ($junk != $tail);

	$trickover=0;
}


###########################################################################
# Processes a play offer: for this client, we don't do anything.  Spiffier
# clients may want to do some kind of animation for this

sub PlayOffer
{	local($player);
	# <PLAYOFFER> <pnum> <tail>
	$player=ReadInt();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in PlayOffer().") if ($junk != $tail);
	AddChat("Server: It's $players{$player}{name}'s turn to play.");
}


###########################################################################
# Processes a defend offer: for this client, we don't do anything.  Spiffier
# clients may want to do some kind of animation for this

sub DefendOffer
{	local($player);
	# <DEFENDOFFER> <pnum> <tail>
	$player=ReadInt();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in DefendOffer().") if ($junk != $tail);
	AddChat("Server: $players{$player}{name} is offered the option to defend.");
}


###########################################################################
# Processes a order offer: for this client, we don't do anything.  Spiffier
# clients may want to do some kind of animation for this

sub OrderOffer
{	local($player);
	# <ORDEROFFER> <pnum> <tail>
	$player=ReadInt();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in OrderOffer().") if ($junk != $tail);
	AddChat("Server: $players{$player}{name} is offered the option to order.");
}


###########################################################################
# Processes a call offer: for this client, we don't do anything.  Spiffier
# clients may want to do some kind of animation for this

sub CallOffer
{	local($player);

	# <CALLOFFER> <pnum> <tail>
	$player=ReadInt();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in CallOffer().") if ($junk != $tail);
	if ( $players{$player}{dealer} && $game{screw} )
	{	AddChat("Server: Screw the dealer is in effect: $players{$player}{name} must choose trump.");
	}
	else
	{	AddChat("Server: $players{$player}{name} is offered the option to call.");
	}
}


###########################################################################
# Processes a drop offer: for this client, we don't do anything.  Spiffier
# clients may want to do some kind of animation for this

sub DropOffer
{	local($player);

	# <DROPOFFER> <pnum> <tail>
	$player=ReadInt();
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in CallOffer().") if ($junk != $tail);
	AddChat("Server: $players{$player}{name} is choosing which card to drop.");
}


###########################################################################
# Processes a deal: for this client, we don't do anything.  Spiffier
# clients may want to do some kind of animation for this

sub Deal
{	local($player);

	# <DEAL> <tail>
	$junk=ReadShort();
	AddChat("Client: Hmmm, missing tail in CallOffer().") if ($junk != $tail);
	AddChat("Server: A new hand is being dealt.");
}


###########################################################################
1;
