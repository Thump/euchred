# PeuchreC : Perl Euchre Client
#
# (C) Copyright 1999 Denis McLaughlin


###########################################################################
# Sends a join message to the server

sub SendJoin
{	local($data);
	if ($gstatus eq "unconnected")
	{
		AddChat("error: can't join until connected\n");
		return;
	}

	# <msg> : <msglen> <JOIN> <protocol> <string> <tail>
	$strlen=length($name);
	$msglen=4+4+4+$strlen+2;
	$data=pack "N N N N a$strlen n",$msglen,$JOIN,$protocol,$strlen,$name,$tail;

	SendMessage($data);
}


###########################################################################
# Sends a quit message to the server

sub SendQuit
{	local($reason)=$_[0];
	local($data);

	# <msg> : <msglen> <CLIENTQUIT> <gameh> <playerh> <string> <tail>
	$strlen=length($reason);
	$msglen=4+4+4+4+$strlen+2;
	$data=pack "N N N N N a$strlen n",	
		$msglen,$CLIENTQUIT,$gh,$ph,$strlen,$reason,$tail;

	SendMessage($data);
}


###########################################################################
# Sends the identity packet to the server

sub SendID
{	local($data);

	# <msg> : <msglen> <ID> <gameh> <playerh> <data> <tail>
	# <data> : <nmstring> <clstring> <hwstring> <osstring> <cmtstring>
	# <*string> : <stringlen> <string>
	$namelen=length($name);
	$clientlen=length($client);
	$hwlen=length($hw);
	$oslen=length($os);
	$cmtlen=length($comment);
	$msglen=4+4+4+4+$namelen+4+$clientlen+4+$hwlen+4+$oslen+4+$commentlen+2;
	$data=pack
		"N N N N N a$namelen N a$clientlen N a$hwlen N a$oslen N a$cmtlen n",
		$msglen,$ID,$gh,$ph,$namelen,$name,$clientlen,
		$client,$hwlen,$hw,$oslen,$os,$cmtlen,$comment,$tail;

	SendMessage($data);
}


###########################################################################
# Sends a chat message to the server

sub SendChat
{	local($chat)=$_[0];

	# <msg> : <msglen> <CHAT> <gameh> <playerh> <string> <tail>
	$chatlen=length($chat);
	$msglen=4+4+4+4+$chatlen+2;
	$data=pack "N N N N N a$chatlen n",
		$msglen,$CHAT,$gh,$ph,$chatlen,$chat,$tail;

	SendMessage($data);
}


###########################################################################
# Sends a kick request

sub SendKick
{	local($targetph)=$_[0];

	# <msg> : <msglen> <KICKPLAYER> <gh> <ph> <targetph> <tail>
	$msglen=4+4+4+4+2;
	$data=pack "N N N N N n",
		$msglen,$KICKPLAYER,$gh,$ph,$targetph,$tail;

	SendMessage($data);
}


###########################################################################
# Sends options to the server

sub SendOptions
{	local($candefend)=$_[0];
	local($aloneonorder)=$_[1];
	local($screw)=$_[2];

	# <msg> : <msglen> <OPTIONS> <gh> <ph> <candefend>
	#         <aloneonorder> <screw> <tail>
	$msglen=4+4+4+4+4+4+2;
	$data=pack "N N N N N N N n",
		$msglen,$OPTIONS,$gh,$ph,$candefend,$aloneonorder,
		$screw,$tail;

	SendMessage($data);
}


###########################################################################
# Sends the message to start the game

sub SendStart
{	
	# <msg> : <msglen> <START> <gh> <ph> <tail>
	$msglen=4+4+4+2;
	$data=pack "N N N N n",$msglen,$START,$gh,$ph,$tail;

	SendMessage($data);
}


###########################################################################
# Sends the message to end the game

sub SendEnd
{	
	# <msg> : <msglen> <END> <gh> <ph> <tail>
	$msglen=4+4+4+2;
	$data=pack "N N N N n",$msglen,$END,$gh,$ph,$tail;

	SendMessage($data);
}


###########################################################################
# Sends the current choice if we're in a game, and we have a choice
# offered to us

sub SendChoice
{	return if (!$game{ingame});

	if ($trickover == 1)
	{	$trickover=0; return; }

	if ($players{$me}{orderoffer})
	{	SendOrder(); return; }

	if ($players{$me}{calloffer} && $game{callstate} eq "1")
	{	SendCall(); return; }

	if ($players{$me}{calloffer} && $game{callstate} eq "0")
	{	SetCall(); return; }

	if ($players{$me}{dropoffer})
	{	SendDrop(); return; }

	if ($players{$me}{defendoffer})
	{	SendDefend(); return; }

	if ($players{$me}{playoffer})
	{	SendPlay(); return; }

	return;
}


###########################################################################
# Sends the current order choice

sub SendOrder
{
	# <msg> : <msglen> <ORDER|ORDERALONE|ORDERPASS> <gh> <ph> <tail>
	$msglen=4+4+4+2;
	$data=pack "N N N N n",$msglen,$ORDER,$gh,$ph,$tail
		if ($choicetext eq "Order" || $choicetext eq "Pick up");
	$data=pack "N N N N n",$msglen,$ORDERALONE,$gh,$ph,$tail
		if ($choicetext eq "Order alone" || $choicetext eq "Pick up alone");
	$data=pack "N N N N n",$msglen,$ORDERPASS,$gh,$ph,$tail
		if ($choicetext eq "Pass");

	SendMessage($data);
}


###########################################################################
# Send drop card 

sub SendDrop
{	local($value,$suit);

	($value,$suit)=CardToType($choicetext);

	# <msg> : <msglen> <DROP> <gh> <ph> <value> <suit> <tail>
	$msglen=4+4+4+4+4+2;
	$data=pack "N N N N N N n",$msglen,$DROP,$gh,$ph,$value,$suit,$tail;

	SendMessage($data);
}


###########################################################################
# Sets the current call choice: this routine is called when choosing to
# call, call alone, or call pass, followed by the real SendCall when the
# player chooses which suit to send

sub SetCall
{
	$game{callstate}=1;

	if ($choicetext eq "Pass" || $choicetext eq "Fold hand")
	{	SendCallPass();
		return;
	}

	$callmsg=$CALL if ($choicetext eq "Call");
	$callmsg=$CALLALONE if ($choicetext eq "Call alone");

	$game{callstate}=0 if ($choicetext eq "Cancel");
}


###########################################################################
# Sends the current call choice

sub SendCall
{	local($suit);

	# <msg> : <msglen> <CALL|CALLALONE> <gh> <ph> <suit> <tail>
	$msglen=4+4+4+4+2;
	$suit=TextToType($choicetext);
	$game{callstate}=0;

	$data=pack "N N N N N n",$msglen,$callmsg,$gh,$ph,$suit,$tail;

	SendMessage($data);
}


###########################################################################
# Sends a call pass

sub SendCallPass
{	# <msg> : <msglen> <CALLPASS> <gh> <ph> <tail>
	$msglen=4+4+4+2;
	$game{callstate}=0;

	$data=pack "N N N N n",$msglen,$CALLPASS,$gh,$ph,$tail;

	SendMessage($data);
}


###########################################################################
# Sends defend 

sub SendDefend
{
	# <msg> : <msglen> <DEFEND> <gh> <ph> <tail>
	$msglen=4+4+4+2;
	$data=pack "N N N N n",$msglen,$DEFEND,$gh,$ph,$tail
		if ($choicetext eq "Defend alone");
	$data=pack "N N N N n",$msglen,$DEFENDPASS,$gh,$ph,$tail
		if ($choicetext eq "Pass");

	SendMessage($data);
}


###########################################################################
# Send card 

sub SendPlay
{	local($value,$suit);

	($value,$suit)=CardToType($choicetext);

	# <msg> : <msglen> <PLAY> <gh> <ph> <value> <suit> <tail>
	$msglen=4+4+4+4+4+2;
	$data=pack "N N N N N N n",$msglen,$PLAY,$gh,$ph,$value,$suit,$tail;

	SendMessage($data);
}


###########################################################################
1;
