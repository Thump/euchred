
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>


/* specific euchres includes */
#include "euchres.h"


/* This routine starts listening on a socket, and then calls the
 * GetEvent() routine.  This routine then services the event, including
 * modifying the game state as appropriate, before returning to GetEvent().
 */
void PlayGame(void)
{
	fd_set fd;
	int pnum;

	debug(GENERAL) fprintf(stderr,"entering PlayGame()\n");

	while ( true )
	{
		fd=DoSelect();

		/* something on the listen socket: must be a new client */
		if (FD_ISSET(s,&fd))
			NewClient();

		/* step through and service clients if needed */
		for (pnum=0; pnum<4; pnum++)
			if (players[pnum].state != unconnected)
				if (FD_ISSET(players[pnum].socket,&fd))
					ServiceClient(pnum);

		/* generic maintenance routine:
		 *  - step through currently connected but unjoined clients,
		 *    turfing those over the limit
		 *  - step through clients with outstanding messages, turfing
		 *    those over the limit
		 */
		for (pnum=0; pnum<4; pnum++)
		{	if (players[pnum].msgwait &&
				time(NULL) > players[pnum].msgtime+maxmsgwait )
				KickClient(pnum,"Timeout waiting for data");
		
			if (players[pnum].joinwait &&
				time(NULL) > players[pnum].msgtime+maxmsgwait )
				KickClient(pnum,"Timeout waiting for JOIN");
		}
	}
}


/* This routine chooses a new creator, and then modifies the player
 * structure accordingly.
 */
void PickCreator()
{	int pnum;

	debug(GENERAL) fprintf(stderr,"entering PickCreator()\n");

	for (pnum=0; pnum<4; pnum++)
		if (players[pnum].state == joined)
		{	players[pnum].creator = true;
			return;
		}
}


/* This routine clears the game data structure: this starts everything
 * fresh, resulting in no players playing.
 */
void ClearGame()
{
	debug(GENERAL) fprintf(stderr,"entering ClearGame()\n");

	game.ingame=false;
	game.gh=0;
	game.suspend=false;
	game.players=0;
	game.bots=0;
	game.score[0]=0;
	game.score[1]=0;
	game.tricks=0;
	game.numholecalled[0]=0;
	game.numholecalled[1]=0;
	game.numtrumpcalled[0]=0;
	game.numtrumpcalled[1]=0;
	game.numdealerscrewed[0]=0;
	game.numdealerscrewed[1]=0;
	game.numeuchres[0]=0;
	game.numeuchres[1]=0;
	game.numdefends[0]=0;
	game.numdefends[1]=0;
	game.candefend=true;
	game.aloneonorder=true;
	game.screw=true;
}


/* This routine resets the game data structure, allowing the same
 * group of players to play again
 */
void ResetGame()
{	int i;

	debug(GENERAL) fprintf(stderr,"entering ResetGame()\n");

	game.ingame=false;
	game.suspend=false;
	game.score[0]=0;
	game.score[1]=0;
	game.tricks=0;
	game.numholecalled[0]=0;
	game.numholecalled[1]=0;
	game.numtrumpcalled[0]=0;
	game.numtrumpcalled[1]=0;
	game.numdealerscrewed[0]=0;
	game.numdealerscrewed[1]=0;
	game.numeuchres[0]=0;
	game.numeuchres[1]=0;
	game.numdefends[0]=0;
	game.numdefends[1]=0;
	game.candefend=true;
	game.aloneonorder=true;
	game.screw=true;

	for (i=0; i<4; i++)
		players[i].dealer=false;
}


/* Routine to flush player structures */
void ClearPlayer(int pnum)
{
	debug(GENERAL) fprintf(stderr,"entering ClearPlayer()\n");

	players[pnum].joinwait=false;
	players[pnum].msgwait=false;
	players[pnum].msglen=0;
	players[pnum].msgtime=0;
	players[pnum].state=unconnected;
	players[pnum].socket=undefined;
	players[pnum].ph=-1;
	players[pnum].cards=NULL;
	players[pnum].numcards=0;
	players[pnum].team=undefined;
	players[pnum].creator=false;
	players[pnum].playername=strdup(DEFNAME);
	lomem(players[pnum].playername);
	players[pnum].hardware=NULL;
	players[pnum].OS=NULL;
	players[pnum].clientname=NULL;
	players[pnum].comment=NULL;
	players[pnum].ip=NULL;
	players[pnum].holecalled=0;
	players[pnum].holemade=0;
	players[pnum].trumpcalled=0;
	players[pnum].trumpsmade=0;
	players[pnum].handsalonecalled=0;
	players[pnum].handsalonemade=0;
	players[pnum].defendalonescalled=0;
	players[pnum].defendalonesmade=0;
	players[pnum].tricksmade=0;
	players[pnum].antitricksmade=0;
	players[pnum].euchres=0;
	players[pnum].screwed=0;
	players[pnum].nummsgs=0;
	players[pnum].volmsgs=0;
	players[pnum].playoffer=false;
	players[pnum].orderoffer=false;
	players[pnum].dropoffer=false;
	players[pnum].calloffer=false;
	players[pnum].defendoffer=false;
	players[pnum].dealer=false;
	players[pnum].ordered=false;
	players[pnum].alone=false;
	players[pnum].defend=false;
	players[pnum].leader=false;
	players[pnum].maker=false;
	players[pnum].cardinplay=false;
	players[pnum].passed=false;
}


/* routine to clear hand structure */
void ClearHand()
{	int i;

	debug(GENERAL) fprintf(stderr,"entering ClearHand()\n");

	hand.hole.suit=-1;
	hand.hole.value=-1;
	hand.suit=-1;
	hand.hstate=0;
	hand.dealer=-1;
	if (hand.maker > -1 && hand.maker < 4)
		players[hand.maker].maker=false;
	hand.maker=-1;
	hand.tricks[0]=0;
	hand.tricks[1]=0;
	for (i=0; i<4; i++)
	{	if (players[i].cards != NULL)
			free(players[i].cards);
		players[i].cards=NULL;
		players[i].numcards=0;
		players[i].orderoffer=false;
		players[i].dropoffer=false;
		players[i].calloffer=false;
		players[i].defendoffer=false;
		players[i].playoffer=false;
		players[i].ordered=false;
		players[i].alone=false;
		players[i].defend=false;
		players[i].leader=false;
		players[i].maker=false;
		players[i].cardinplay=false;
		players[i].passed=false;
	}
}


/* This routine shuffles up a new deck, deals it out, then (if no dealer
 * is currently set) chooses a dealer.  Called at the beginning of each
 * hand.
 */
void NewHand()
{	int i;

	debug(GENERAL) fprintf(stderr,"entering NewHand()\n");

	/* choose the dealer */
	PickDealer();

	/* create, shuffle, deal and sort the cards */
	SendDeal();
	if (deck != NULL)
		free(deck);
	deck=NewDeck();
	for (i=0; i<20; i++)
		AddCard(i%4,deck[i]);
	for (i=0; i<4; i++)
		SortCards(i);

	/* set the hand information */
	hand.hole.suit=deck[20].suit;
	hand.hole.value=deck[20].value;
	hand.hstate=1;
}


/* This routine chooses the next dealer: if there already is a dealer, the
 * next dealer is the player number incremented by 1, mod 4.  Otherwise
 * we shuffle the deck of cards and step through them until we find the
 * jack of spades, choosing the index mod 4 player.  Well how did you
 * think I was going to do it?
 */
void PickDealer()
{	int i;
	Card *deck;

	debug(GENERAL) fprintf(stderr,"entering PickDealer()\n");

	for (i=0; i<4; i++)
		if (players[i].dealer)
		{	players[i].dealer=false;
			players[(i+1)%4].dealer=true;
			hand.dealer=(i+1)%4;
			return;
		}

	deck=NewDeck();
	for (i=0; i<24; i++)
		if (deck[i].value==11 && deck[i].suit==3)
		{	players[i%4].dealer=true;
			hand.dealer=i%4;
		}
	free(deck);

}


/* This routine chooses the next leader: it's set to (dealer+1)%4, unless
 * (dealer+3)%4 is going alone, in which case it is set to (dealer+2)%4.
 *
 * Hey, 'dealer' is an anagram of 'leader', betcha dint know that.
 */
void PickLeader()
{	int i;

	debug(GENERAL) fprintf(stderr,"entering PickLeader()\n");

	/* first, just to be sure, we clear any previous leaders */
	for (i=0; i<4; i++)
		players[i].leader=false;

	/* if (dealer+1)%4's partner is not going alone or defending alone,
	 * then set her as the leader
	 */
	if (!players[(hand.dealer+3)%4].alone &&
		!players[(hand.dealer+3)%4].defend)
	{	players[(hand.dealer+1)%4].leader=true;
		return;
	}

	/* in the case where (dealer+1)%4's partner is going alone or defending
	 * alone, if the dealer is not going alone or defending alone, then set
	 * the dealer's partner as the leader
	 */
	if (!players[hand.dealer].alone && !players[hand.dealer].defend)
	{	players[(hand.dealer+2)%4].leader=true;
		return;
	}

	/* otherwise, in the instance where the both the dealer and the player
	 * to the dealer's right are going alone / defending alone, return
	 * the player to the dealer's right as the leader
	 */
	players[(hand.dealer+3)%4].leader=true;
}


/* This is a generic routine, called to determine the next action to occur:
 * it analyses the current game state and sets the next game state.  This
 * is the routine that drives the game forward, from the order offer to
 * call offer to play offer to score update followed by a new hand.
 */
void NextAction()
{
	debug(GENERAL) fprintf(stderr,"entering NextAction()\n");

	/* Each of NextOrder(), NextCall(), NextDefend() and NextPlay()
	 * set the game state according to the next person who should
	 * be offered the order, call, defend or play.  If they can, they
	 * unset the current offer, set the next one, and return true.
	 * Otherwise, they unset the current offer and return false.  A
	 * false return means all offers are complete for that state, and
	 * the next state can be entered.  The specific state to be entered
	 * next is set by the NextOrder()|NextCall()|NextDefend()|NextPlay()
	 * routine.
	 */

	/* if we're offering order */
	if ( hand.hstate == 1)
		if ( NextOrder() )
			return;

	/* if we're offering call */
	if ( hand.hstate == 2 )
		if ( NextCall() )
			return;

	/* if we're offering defend */
	if ( hand.hstate == 3 )
		if ( NextDefend() )
			return;

	/* if we're offering play */
	if ( hand.hstate == 4 )
		if ( NextPlay() )
			return;

	/* If we got here, the hand is over: we check here for game over
	 * states (one of the teams having a score greater than 9).  If the
	 * game isn't over, we clear the current hand, deal the new hand,
	 * and call NextOrder() before returning.
	 */
	if (game.score[0] > 9)
	{	GameOver(0);
		return;
	}
	else if (game.score[1] > 9)
	{	GameOver(1);
		return;
	}

	ClearHand();
	NewHand();
	NextOrder();
}


/* This routine examines the current state in order to determine who,
 * if anyone, should be offered the next order.  If no-one is currently
 * being offered the order, (dealer+1)%4 is offered.  If someone is
 * being offered the order, the next person in line is offered.  In
 * both of these situations, the function returns true.  If there is
 * no-one left to receive the order offer, the function returns false,
 * after it sets the next state by checking to see if anyone ordered.
 * In all cases, the current offered player is unset.
 */
boolean NextOrder()
{	int i,current;

	debug(GENERAL) fprintf(stderr,"entering NextOrder()\n");

	/* find out if we already have an offer, and unset it if we do */
	current=-1;
	for (i=0; i<4; i++)
		if (players[i].orderoffer)
		{	current=i;
			players[i].orderoffer=false;
		}

	/* if we have no current offers, offer to player after dealer */
	if (current == -1)
		for (i=0; i<4; i++)
			if (players[i].dealer)
			{	players[(i+1)%4].orderoffer=true;
				SendOrderOffer((i+1)%4);
				return(true);
			}

	/* bad things: if current is still -1, we don't have a dealer */
	if (current == -1)
	{	myLog("Can't find dealer in NextOrder()");
		return(false);
	}

	/* if the current offer is to the dealer (meaning everyone has been
	 * offered the hole), set the next step by checking whether trump
	 * has been set, and return false to move to the next step
	 */
	if (players[current].dealer)
	{	if (hand.suit == -1)
			hand.hstate+=1;
		else
			hand.hstate+=2;
		return(false);
	}

	players[(current+1)%4].orderoffer=true;
	SendOrderOffer((current+1)%4);
	return(true);
}


/* This routine examines the current state in order to determine who,
 * if anyone, should be offered the next call.  If no-one is currently
 * being offered the call, (dealer+1)%4 is offered.  If someone is
 * being offered the call, the next person in line is offered.  In
 * both of these situations, the function returns true.  If there is
 * no-one left to receive the order offer, the function returns false,
 * after it sets the next state by checking to see if anyone called.
 * In all cases, the current offered player is unset.
 */
boolean NextCall()
{	int i,current;

	debug(GENERAL) fprintf(stderr,"entering NextCall()\n");

	/* find out if we already have an offer, and unset it if we do */
	current=-1;
	for (i=0; i<4; i++)
		if (players[i].calloffer)
		{	current=i;
			players[i].calloffer=false;
		}

	/* if we have no current offers, offer to player after dealer */
	if (current == -1)
		for (i=0; i<4; i++)
			if (players[i].dealer)
			{	players[(i+1)%4].calloffer=true;
				SendCallOffer((i+1)%4);
				return(true);
			}

	/* bad things: if current is still -1, we don't have a dealer */
	if (current == -1)
	{	myLog("Can't find dealer in NextCall()");
		return(false);
	}

	/* if the current offer is to the dealer (meaning everyone has been
	 * offered the hole), set the next step by checking whether trump
	 * has been set, and return false to move to the next step
	 */
	if (players[current].dealer)
	{	if (hand.suit == -1)
		{	myLog("Dealer escaped a screwing: badness");
			hand.suit=2; /* rando state */
		}
		else
			hand.hstate++;
		return(false);
	}

	players[(current+1)%4].calloffer=true;
	SendCallOffer((current+1)%4);
	return(true);
}


/* This routine examines the current state in order to determine who,
 * if anyone, should be offered the next defend.  If no-one is currently
 * being offered the defend, (dealer+1)%4 is offered, so long as that
 * player is on the opposite team, and didn't pass.  If someone is
 * being offered the defend, the next person in line is offered, so long
 * as they are on the opposite team and didn't pass.  In both of these
 * situations, the function returns true.  If there is no-one left to
 * receive the defend offer, the function returns false, after it sets
 * the next state to play.
 */
boolean NextDefend()
{	int i,current;

	debug(GENERAL) fprintf(stderr,"entering NextDefend()\n");

	/* find out if we already have an offer, and unset it if we do */
	current=-1;
	for (i=0; i<4; i++)
		if (players[i].defendoffer)
		{	current=i;
			players[i].defendoffer=false;
		}

	/* If we have no current offers, offer to first player after the
	 * dealer who meets the criteria of being after the caller.  If
	 * we do have an offer outm offer it to the next one in line.
	 */
	if (current == -1)
		current=(hand.maker+1)%4;
	else
		current=(current+1)%4;

	for (i=current; i!=(hand.dealer+1)%4; i=(i+1)%4)
		if (players[i%4].team != players[hand.maker].team)
		{	players[i%4].defendoffer=true;
			SendDefendOffer((i+1)%4);
			return(true);
		}

	/* we only get here if the defend has been offered to everyone,
	 * in which case we move to the next state, beginning play
	 */
	hand.hstate=4;
	players[(hand.dealer+1)%4].leader=true;
	return(false);
}


/* This routine examines the current state in order to determine who,
 * if anyone, should be offered the next play.  If no-one is currently
 * being offered the play, (dealer+1)%4 is offered.  If someone is
 * being offered the play, the next person in line is offered.  In
 * both of these situations, the function returns true.  If there is
 * no-one left to receive the play offer, the function examines the
 * played cards, determines the highest, assign the trick, resets the
 * played cards, and resets the play.
 */
boolean NextPlay()
{	int i,current,maketeam,defendteam;
	int maker1=-1,maker2=-1,defender1=-1,defender2=-1;

	debug(GENERAL) fprintf(stderr,"entering NextPlay()\n");

	/* find out if we already have an offer, and unset it if we do */
	current=-1;
	for (i=0; i<4; i++)
		if (players[i].playoffer)
		{	current=i;
			players[i].playoffer=false;
		}

	/* if we have no current offers, choose a leader and offer it to her */
	if (current == -1)
	{	PickLeader();
		for (i=0; i<4; i++)
			if (players[i].leader)
			{	players[i].playoffer=true;
				SendPlayOffer(i);
				return(true);
			}
	}

	/* if the current offer is to the person prior to the leader, or if
	 * it is to the person prior to the person prior to the leader, and
	 * the person after the leader is either defending or alone, or if it
	 * is to the person after the leader, and both the leader and the person
	 * after the leader are either going alone or defending alone, then
	 * then the trick is over: evaluate it, reset the trick, recompute the
	 * leader, offer to the leader, and return
	 *
	 * Would you like parmesan cheese or ground pepper with that?
	 */
	if (players[(current+1)%4].leader ||

		( players[(current+2)%4].leader &&
		  (players[(current+3)%4].alone || players[(current+3)%4].defend)) ||

		(players[(current+3)%4].leader &&
		  ( (players[current].alone || players[current].defend) &&
		    (players[(current+3)%4].alone || players[(current+3)%4].defend)
		  )
	    )
	   )
	{	EvaluateCards();

		/* Check if the hand is over: if the making team has 5 tricks, if
		 * if the making team has 3 or more tricks and the defending team
		 * has 1, or if the defending team has 3 or more tricks, do the
	 	 * scores and return false.  Otherwise, clear current plays,
		 * and return.  (Next leader was set in EvaluateCards().
		 */

		/* set the team variables */
		maketeam=players[hand.maker].team-1;
		defendteam=maketeam+1; if (defendteam > 1) defendteam=0;

		/* Determine the defenders and makers: yeah, there's a smarter way
		 * to do this, but two tears in a bucket.
		 */
		for (i=0; i<4; i++)
			if (players[i].team == maketeam+1)
				maker1=i;
			else if (players[i].team == defendteam+1)
				defender1=i;
		for (i=3; i>-1; i--)
			if (players[i].team == maketeam+1)
				maker2=i;
			else if (players[i].team == defendteam+1)
				defender2=i;

		/* check the makers got 'em all */
		if (hand.tricks[maketeam] == 5)
			/* by going alone? */
			if (players[hand.maker].alone)
			{	sprintf(tbuffer1,
					"Server: %s went alone and got all 5 tricks: 4 points."
					,players[hand.maker].playername);
				SendChat(tbuffer1);
				SendState();
				SendHandOver();
				game.score[maketeam]+=4;
				return(false);
			}
			else /* nope */
			{	sprintf(tbuffer1,
					"Server: %s and %s took all 5 tricks: 2 points.",
					players[maker1].playername,players[maker2].playername);
				SendChat(tbuffer1);
				SendState();
				SendHandOver();
				game.score[maketeam]+=2;
				return(false);
			}

		/* check if the defenders stopped them  */
		if (hand.tricks[maketeam]>2 && hand.tricks[defendteam]>0)
		{	sprintf(tbuffer1,
				"Server: %s and %s were stopped at %d tricks: 1 point.",
				players[maker1].playername,players[maker2].playername,
				hand.tricks[maketeam]);
			SendState();
			SendChat(tbuffer1);
			SendHandOver();
			game.score[maketeam]++;
				return(false);
		}

		/* did the defenders euchre them? */
		if (hand.tricks[defendteam] > 2)
		{	/* by defending? */
			if (players[defender1].defend)
			{	sprintf(tbuffer1,
					"Server: %s defended and got the euchre: 4 points."
					,players[defender1].playername);
				SendChat(tbuffer1);
				SendState();
				SendHandOver();
				game.score[defendteam]+=4;
				return(false);
			} /* the other guy? */
			else if (players[defender2].defend)
			{	sprintf(tbuffer1,
					"Server: %s defended and got the euchre: 4 points."
					,players[defender2].playername);
				SendChat(tbuffer1);
				SendState();
				SendHandOver();
				game.score[defendteam]+=4;
				return(false);
			}
			else /* nope */
			{	sprintf(tbuffer1,"Server: %s and %s got euchered: 2 points.",
				players[maker1].playername,players[maker2].playername);
				SendChat(tbuffer1);
				SendState();
				SendHandOver();
				game.score[defendteam]+=2;
				return(false);
			}
		}

		/* Okay, if we got here, the hand isn't over: clear the current
		 * plays and return.  We don't worry about setting the leader
		 * and offering the play: EvaluateCards() did this already.
		 */
		for (i=0; i<4; i++)
		{	players[i].cardinplay=false;
			players[i].card.suit=-1;
			players[i].card.value=-1;
		}
		return(true);
	}

	/* the default fall through: there is already an offer made, and
	 * there is a valid next player, so just offer to the next in line
	 */
	if ( !players[(current+3)%4].alone && !players[(current+3)%4].defend)
	{	players[(current+1)%4].playoffer=true;
		return(true);
	}

	if ( !players[current].alone && !players[current].defend)
	{	players[(current+2)%4].playoffer=true;
		return(true);
	}

	if ((players[current].alone || players[current].defend) &&
		(players[(current+3)%4].alone || players[(current+3)%4].defend)
	   )
	{	players[(current+3)%4].playoffer=true;
		return(true);
	}

	myLog("Bad fall through in NextPlay(): Help me Tito!");
	return(true);
}


/* This routine returns true if the passed card makes a valid play
 * for that player, given the other cards already played.
 */
boolean ValidPlay(int pnum, Card card)
{	int i,leader,leadsuit;

	debug(GENERAL) fprintf(stderr,"entering ValidPlay()\n");

	leader=-1;
	for (i=0; i<4; i++)
		if (players[i].state == joined)
			if (players[i].leader)
				leader=i;

	if (leader==-1)
	{	myLog("Can't find leader in ValidPlay()");
		Exit();
	}

	if ( ! players[leader].cardinplay )
		return(true);
	else
		leadsuit=SuitOf(players[leader].card);

	/* One of three cases must apply: either the lead suit and played suit
	 * match, in which case the card is valid.  Or the player does not have
	 * the lead suit, in which case any card is valid.  Otherwise the player
	 * didn't follow when she should have, ergo the card is not valid.
	 */

	if (SuitOf(card)==leadsuit)
		return(true);

	if (!HasSuit(pnum,leadsuit))
		return(true);

	return(false);
}


/* This routine evaluates the current cards in play, determines who won
 * the trick, modifies the players, hand, and game structure, and then
 * returns
 */
void EvaluateCards()
{	int i,numcards,leadsuit,trumpsuit,winner,leader=-1;
	Card cards[4],winningcard;

	/* first we extract the cards in play, also storing the lead suit */
	leadsuit=-1;
	numcards=0;
	for (i=0; i<4; i++)
	{	if (players[i].cardinplay)
		{	if (players[i].leader)
			{	leader=i;
				leadsuit=SuitOf(players[i].card);
			}
			cards[numcards]=players[i].card;
			numcards++;
		}
	}
	trumpsuit=hand.suit;

	if (leadsuit==-1)
	{	myLog("Shitfuck, can't find leadsuit in EvaluateCards()");
		leadsuit=2;
	}

	/* now we step through the cards played, settling on the top card */
	winningcard=cards[0];
	for (i=1; i<numcards; i++)
		winningcard=HiCard(winningcard,cards[i],leadsuit,trumpsuit);

	/* Knowing the winning card, we determine who played it */
	winner=-1;
	for (i=0; i<4; i++)
		if (players[i].cardinplay &&
			players[i].card.suit == winningcard.suit &&
			players[i].card.value == winningcard.value)
			winner=i;
	if (winner == -1)
	{	myLog("Shitfuck, can't find winning card");
		Exit();
	}

	/* we tell people about it, before setting the next state */
	sprintf(tbuffer1,"Server: %s won the trick with %s",
		players[winner].playername,CardText(winningcard));
	SendChat(tbuffer1);
	SendState();
	SendTrickOver();

	/* increment the trick count and set new leader, and indicate they
	 * should play
	 */
	hand.tricks[players[winner].team-1]++;
	players[leader].leader=false;
	players[winner].leader=true;
	players[winner].playoffer=true;
	SendPlayOffer(winner);
}


/* This routine takes two cards, the lead suit and the trump suit, and
 * returns whichever of the two cards is the highest.  The algorithm is
 * a bit belabored, but hopefully the more easily understood for that.
 * We simplify the algorithm by first checking for the right and left:
 * if either are present, we return those.  This saves munging about with
 * value changes later, since we can then compare card.value directly.
 */
Card HiCard(Card card1, Card card2, int leadsuit, int trumpsuit)
{
	/* return the right if we have it */
	if (card1.suit == trumpsuit && card1.value==11)
		return(card1);
	if (card2.suit == trumpsuit && card2.value==11)
		return(card2);

	/* return the left if we have it */
	if (SuitOf(card1) == trumpsuit && card1.value==11)
		return(card1);
	if (SuitOf(card2) == trumpsuit && card2.value==11)
		return(card2);


	/* If both cards are leadsuit, return the card with the higher value. */
	if ( card1.suit == leadsuit && card2.suit == leadsuit )
		if ( card1.value > card2.value )
			return(card1);
		else
			return(card2);
 
	/* If both cards are trump, reuturn the card with the higher value. */
	if ( card1.suit == trumpsuit && card2.suit == trumpsuit )
		if ( card1.value > card2.value )
			return(card1);
		else
			return(card2);

	/* If one card is trump and the other not, return the trump.  */
	if ( card1.suit == trumpsuit && card2.suit != trumpsuit )
		return(card1);
	if ( card1.suit != trumpsuit && card2.suit == trumpsuit )
		return(card2);

	/* If one card is leadsuit and the other not, return the one that is. */
	if ( card1.suit == leadsuit && card2.suit != leadsuit )
		return(card1);
	if ( card1.suit != leadsuit && card2.suit == leadsuit )
		return(card2);

	/* If neither card is trumpsuit, and neither card is leadsuit, return
	 * the card with the higher value.  Technically this favours card2,
	 * since if the two are equal card2 wins.  But eventually it will be
	 * compared against a leadsuit (since every hand has a leadsuit), and
	 * will lose.
	 */
	if ( card1.value > card2.value )
		return(card1);
	else
		return(card2);
}


/* Game over routine */
void GameOver(int winningteam)
{	int i,winner1=-1,winner2=-1,losingteam;

	/* Yeahyeahyeah, whatever, you think you code so hot, where's
	 * -your- euchre server?
	 */
	for (i=0; i<4; i++)
		if (players[i].team == winningteam+1)
			winner1=i;
	for (i=3; i>-1; i--)
		if (players[i].team == winningteam+1)
			winner2=i;

	losingteam=winningteam+1;
	if (losingteam>2) losingteam=1;

	/* tell em all */
	sprintf(tbuffer1,
		"Server: %s and %s have won with a score of %d to %d.",
		players[winner1].playername,players[winner2].playername,
		game.score[winningteam],game.score[losingteam]);
	SendChat(tbuffer1);
	SendChat("Server: Game over.");
	SendGameOver();

	/* then reset everything for the next game */
	ClearHand();
	ResetGame();
}
