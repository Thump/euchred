
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


/* specific euchres includes */
#include "euchres.h"

/* This routine returns a new shuffled deck of cards, that is, a 24 element
 * array of Card
 */
Card *NewDeck()
{	int suit,value,count,index,target;
	Card *deck,card;

	debug(GENERAL) fprintf(stderr,"entering NewDeck()\n");

	/* first we create an unshuffled deck */
	index=0;
	deck=(Card *)malloc(sizeof(Card)*24);
	for (suit=0; suit<4; suit++)
		for (value=9; value<15; value++)
		{	deck[index].suit=suit;
			deck[index].value=value;
			index++;
		}

	/* then we shuffle: we make a pass through the deck, replacing each
	 * card by another from a random location (possibly equal to itself);
	 * we do this 7 times, for obvious reasons
	 */
	for (count=0; count<7; count++)
		for (index=0; index<24; index++)
		{	target=(int) (24.0*rand()/(RAND_MAX+1.0));
			card.suit=deck[target].suit;
			card.value=deck[target].value;
			deck[target].suit=deck[index].suit;
			deck[target].value=deck[index].value;
			deck[index].suit=card.suit;
			deck[index].value=card.value;
		}

	return(deck);
}


/* This routine takes a card and a player and adds that card to the players
 * hand
 */
void AddCard(int pnum, Card card)
{ 
	debug(GENERAL) fprintf(stderr,"entering AddCard()\n");

	if (players[pnum].numcards > 4)
	{	sprintf(tbuffer1,"Tried to add a sixth card to %s (%d)",
			players[pnum].playername,players[pnum].numcards);
		myLog(tbuffer1);
		return;
	}

	players[pnum].cards=(Card *)realloc(players[pnum].cards,
		sizeof(Card)*(players[pnum].numcards+1));
	lomem(players[pnum].cards);

	players[pnum].cards[players[pnum].numcards].suit=card.suit;
	players[pnum].cards[players[pnum].numcards].value=card.value;
	players[pnum].numcards++;
}


/* This routine takes a card and a player and removes that card from that
 * players hand
 */
void RemoveCard(int pnum, Card card)
{	int index,num;
	Card *cards;

	debug(GENERAL) fprintf(stderr,"entering RemoveCard()\n");

	if ( !HasCard(pnum,card) )
	{	sprintf(tbuffer1,"Tried to remove a nonexistent card from %s",
			players[pnum].playername);
		myLog(tbuffer1);
		return;
	}

	cards=players[pnum].cards;
	num=players[pnum].numcards;
	players[pnum].cards=NULL;
	players[pnum].numcards=0;

	for (index=0; index<num; index++)
		if (card.suit != cards[index].suit || card.value != cards[index].value)
			AddCard(pnum,cards[index]);

	free(cards);
}


/* This routine returns true if player pnum has card card in her hand,
 * false otherwise
 */
boolean HasCard(int pnum, Card card)
{	int index;

	debug(GENERAL) fprintf(stderr,"entering HasCard()\n");

	for (index=0; index<players[pnum].numcards; index++)
		if (players[pnum].cards[index].suit == card.suit &&
			players[pnum].cards[index].value == card.value)
			return(true);

	return(false);
}


/* This routine returns true if the given player has a card of the same
 * suit as suit.
 */
boolean HasSuit(int pnum, int suit)
{	int i;

	for (i=0; i<players[pnum].numcards; i++)
		if (SuitOf(players[pnum].cards[i]) == suit)
			return(true);

	return(false);
}


/* This routine takes a card and trump and returns the suit of the 
 * card
 */
int SuitOf(Card card)
{
	if (card.value == 11 && (card.suit+hand.suit)==3)
		return(hand.suit);
	else
		return(card.suit);
}


/* This routine compares two cards, return -1, 0, 1 in the case where
 * card1 is less than, equal to, or greater than card2.  Sort order is:
 * suits in alphabetic order, values hi to lo.
 */
int CardCompare(Card *card1, Card *card2)
{
	if ( hand.suit != -1)
	{	/* order the left with trump */
		if (SuitOf(*card1) < SuitOf(*card2))
			return(-1);
		if (SuitOf(*card1) > SuitOf(*card2))
			return(1);

		/* always hi sort the left and right */
		if ( card1->suit == hand.suit && card1->value == 11)
			return(-1);
		if ( card2->suit == hand.suit && card2->value == 11)
			return(1);
		if ( SuitOf(*card1) == hand.suit && card1->value == 11)
			return(-1);
		if ( SuitOf(*card2) == hand.suit && card2->value == 11)
			return(1);

		/* reverse sort the rest */
		if (card1->value > card2->value)
			return(-1);
		if (card1->value == card2->value)
			return(0);
		if (card1->value < card2->value)
			return(1);
	}
	else
	{	/* Sort suits alphabetically */
		if (card1->suit < card2->suit)
			return(-1);
		if (card1->suit > card2->suit)
			return(1);

		/* Sort card value in reverse order */		
		if (card1->value > card2->value)
			return(-1);
		if (card1->value == card2->value)
			return(0);
		if (card1->value < card2->value)
			return(1);
	}

	return(0);
}


/* This routine sorts a player's cards */
void SortCards()
{	int pnum;

	for (pnum=0; pnum<4; pnum++)
		qsort(players[pnum].cards,players[pnum].numcards,
		sizeof(Card),(void *)(*CardCompare));
}
