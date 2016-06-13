
# PeuchreC : Perl Euchre Client
#
# (C) Copyright 1999 Denis McLaughlin


# Right, let's do this puppy
sub PlayGame
{
	while (1)
	{
		UpdateScreen();
		ShowChat();

		$rout=Select();

		if (vec($rout,fileno(STDIN),1))
		{	ServiceSTDIN();
		}

		if ($S && vec($rout,fileno($S),1))
		{	ServiceS();
		}
	}
}


###########################################################################
# Deals with an unexpected server death

sub ServerDie
{
	AddChat("Client: Uh-oh, the server connection has been lost!\n");

	Disconnect();
}


###########################################################################
# This is the SetTeams() routine: it sets the teams, relying on the server
# always assigning teams in alternating order in the player structure.  We
# step through the player structure until we find ourself, then assign $foe1,
# $partner, and $foe2 sequentially after it.

sub SetTeams
{	local($pnum);

	$game{numplayers}=0;
	$me=""; $foe1=""; $partner=""; $foe2="";

	for ($pnum=0; $pnum<4; $pnum++)
	{
		if ($players{$pnum}{state} eq "joined")
		{	if ($players{$pnum}{ph} == $ph)
			{	$me=$pnum;
			}
			$game{numplayers}++;
		}
	}

	$foe1=($me+1)%4 if ($players{($me+1)%4}{state} eq "joined");
	$partner=($me+2)%4 if ($players{($me+2)%4}{state} eq "joined");
	$foe2=($me+3)%4 if ($players{($me+3)%4}{state} eq "joined");
}


###########################################################################
1;
