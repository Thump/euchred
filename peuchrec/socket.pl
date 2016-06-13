
# PeuchreC : Perl Euchre Client
#
# (C) Copyright 1999 Denis McLaughlin

use Socket;


###########################################################################
# Routine to open and connect a socket to a remote server.
# This routine takes no arguments: it assumes that $port and $server (both
# global variables) are defined prior to entry.  The socket is passed back
# through $S, a global file descriptor.

sub Connect
{
	AddChat("Client: Attempting to connect to $server:$port.");

	$proto = getprotobyname('tcp');
	socket(S, PF_INET, SOCK_STREAM, $proto) ||
		Exit("Client: Error in socket(): $!");
	$sin = sockaddr_in($port,inet_aton($server));

	setsockopt(S,SOL_SOCKET,SO_REUSEADDR,1) ||
		Exit("Client: Error setting SO_REUSEADDR: $!\n");

	if (connect(S,$sin))
	{	$old=select(S); $|=1; select($old);
		$gstatus="connected";
		$S=S;
	}
	else
	{	AddChat("Client: Error in connect(): $!");
	}
}


###########################################################################
# Short routine to clean data structures following a server disconnect

sub Disconnect
{
	#AddChat("Client: Disconnecting from $server:$port.");
	AddChat("Client: Disconnecting from server.");

	CloseServer();

	ClearPlayer($foe1) if ("$foe1" ne "");
	ClearPlayer($foe2) if ("$foe2" ne "");
	ClearPlayer($partner) if ("$partner" ne "");
	ClearPlayer($me) if ("$me" ne "");

	ClearGame();

	$gstatus="unconnected";
}


###########################################################################
# Waits for an incoming message: returns when there is data to be read
# on STDIN or $S: returns $rout, the bit vector

sub Select
{
	local($rin,$rout);

	if ($S)
	{	undef $rin;
		vec($rin,fileno(STDIN),1)=1;
		vec($rin,fileno($S),1)=1;
		#AddChat("Setting STDIN and S: rin is ".unpack("B*",$rin)."\n");
	}
	else
	{	undef $rin;
		vec($rin,fileno(STDIN),1)=1;
		#AddChat("Setting STDIN: rin is ".unpack("B*",$rin)."\n");
	}

	raw();
	($nfound,$timeleft)=select($rout=$rin, undef, undef, undef);
	Exit("Client: select() error: $!\n") if ( $nfound < 0 );

	return($rout);
}


###########################################################################
# Shuts down the server socket politely

sub CloseServer
{
	if ($S)
	{	SendQuit("Sorry, it's been fun");
		close($S);
		$S=undef;
	}
}


###########################################################################
# Routine which takes a data buffer and sends it to the server socket

sub SendMessage
{
	local($data)=$_[0];

	print $S $data;
}


###########################################################################
# Short routine to create file descriptor vectors (read: file descriptor
# sets) for the select() routine.

sub fdset
{
	local(@fhlist) = split(' ',$_[0]);
	local($bits)=0;
	for (@fhlist)
	{
		vec($bits,fileno($_),1) = 1;
	}
	$bits;
}


###########################################################################
# Short routine to check if a certain file descriptor is set

sub fdisset
{
	local(@fhlist) = split(' ',$_[0]);
	local($bits);
	for (@fhlist)
	{
		vec($bits,fileno($_),1) = 1;
	}
	$bits;
}


###########################################################################
# Short routine to read a four byte integer off the server socket

sub ReadInt
{	local($tmp);

	$len=sysread($S,$tmp,4);

	# If the server sends us a message and then severs our link, sysread()
	# can sometimes return a system error (ENOLINK) prior to delivering the
	# data.  To work around this, we reread if we get a system error.
	$len=sysread($S,$tmp,4) if (!defined($len));

	return("") if ($len == 0);
    $tmp=unpack("N",$tmp);

	return($tmp);
}


###########################################################################
# Short routine to read a two byte short off the server socket

sub ReadShort
{	local($tmp);

	$len=sysread($S,$tmp,2);
	return("") if ($len == 0);
    $tmp=unpack("n",$tmp);

	return($tmp);
}


###########################################################################
# Short routine to read a string off the server socket

sub ReadString
{	local($tmp);
	local($len);

	sysread($S,$len,4);
    $len=unpack("N",$len);

	sysread($S,$tmp,$len);

	return($tmp);
}


###########################################################################
# Short routine to read a four byte integer, pretend it's a boolean and
# return it

sub ReadBoolean
{	local($tmp);

	$len=sysread($S,$tmp,4);
	return("") if ($len == 0);
    $tmp=unpack("N",$tmp);

	return($tmp);
}


###########################################################################
# Short routine to read a card from the socket: a card consists of two
# four-byte integers, the first representing the value (2 through 14),
# and the other representing the suit (0 through 3, in alphabetic order

sub ReadCard
{	local($tmp,$value,$suit);

	$value=ReadInt();
	$suit=ReadInt();

	$tmp="";
	$tmp = "$value" if ($value < 11);
	$tmp = "J" if ($value == 11);
	$tmp = "Q" if ($value == 12);
	$tmp = "K" if ($value == 13);
	$tmp = "A" if ($value == 14);

	$tmp .= "c" if ($suit == 0);
	$tmp .= "d" if ($suit == 1);
	$tmp .= "h" if ($suit == 2);
	$tmp .= "s" if ($suit == 3);
	
	return($tmp);
}


###########################################################################
# Short routine to read a suit from the socket: a suit consists of a
# four-byte integer, representing the suit (0 through 3, in alphabetic
# order

sub ReadSuit
{	local($tmp,$suit);

	$suit=ReadInt();

	$tmp .= "c" if ($suit == 0);
	$tmp .= "d" if ($suit == 1);
	$tmp .= "h" if ($suit == 2);
	$tmp .= "s" if ($suit == 3);
	
	return($tmp);
}


###########################################################################
# A short routine used to eat all pending data on server socket: only intended
# for debugging use

sub ClearS
{
	sysread($S,$tmp,10000);
}


###########################################################################
1;
