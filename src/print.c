
/*
 * (C) Copyright 1999 Denis McLaughlin
 *
 * Printing routines
 *
 */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

/* specific euchred includes */
#include "euchred.h"


/* Prints a miscellaneous header */
void PrintHeader(void)
{
    debug(GENERAL) fprintf(stderr,"entering PrintHeader()\n");
    fprintf(stderr,header);
    fprintf(stderr,"\n");
}


/* Prints the game configuration, as set by the command line and
 * the configuration file.
 */
void PrintConfig(void)
{
    debug(GENERAL) fprintf(stderr,"entering PrintConfig()\n");

    fprintf(stderr,"Defaults:\n");
    fprintf(stderr,"   base directory: %s\n",base);
    fprintf(stderr,"   configuration file: %s\n",config);
    fprintf(stderr,"   log file: %s\n",logfile);
    fprintf(stderr,"   listen port: %d\n",port);
    fprintf(stderr,"   debug flag: %d\n",df);
    fprintf(stderr,"   verbosity level: %d\n",vl);
    fprintf(stderr,"   max message wait: %d\n",maxmsgwait);
    fprintf(stderr,"   poll interval: %d\n",pollinterval);
    fprintf(stderr,"\n");
}


/* Prints the game data structure */
void PrintGame(void)
{
    debug(GENERAL) fprintf(stderr,"entering PrintGame()\n");

    fprintf(stderr,"In game: %d\n",game.ingame);
    fprintf(stderr,"Game Handle: %d\n",game.gh);
    fprintf(stderr,"Game Suspended: %d\n",game.suspend);
    fprintf(stderr,"Number of players: %d\n",game.players);
    fprintf(stderr,"Number of bots: %d\n",game.bots);
    fprintf(stderr,"Team Score 1 2: %d %d\n",game.score[0], game.score[1]);
    fprintf(stderr,"Number of Tricks: %d\n",game.tricks);
    fprintf(stderr,"Number of Holes Called 1 2: %d %d\n",game.numholecalled[0],
        game.numholecalled[1]);
    fprintf(stderr,"Number of Trump Called 1 2: %d %d\n",game.numtrumpcalled[0],
        game.numtrumpcalled[1]);
    fprintf(stderr,"Number of Dealers Screwed 1 2: %d %d\n",
        game.numdealerscrewed[0],game.numdealerscrewed[1]);
    fprintf(stderr,"Number of Euchres 1 2: %d %d\n",game.numeuchres[0],
        game.numeuchres[1]);
    fprintf(stderr,"Number of Defends 1 2: %d %d\n",game.numdefends[0],
        game.numdefends[1]);
    fprintf(stderr,"Can Defend Alone: %d\n",game.candefend);
    fprintf(stderr,"Must Go Alone on Order: %d\n",game.aloneonorder);
    fprintf(stderr,"Screw The Dealer: %d\n",game.screw);
}


/* Prints the specified player structure */
void PrintPlayer(Player player)
{
    debug(GENERAL) fprintf(stderr,"entering PrintGame()\n");

    fprintf(stderr,"join wait: %d\n",player.joinwait);
    fprintf(stderr,"msg wait: %d\n",player.msgwait);
    fprintf(stderr,"msg len: %d\n",player.msglen);
    fprintf(stderr,"msg time: %d\n",player.msgtime);
    fprintf(stderr,"pstate: "); PrintPstate(player.state); fprintf(stderr,"\n");
    fprintf(stderr,"socket number: %d\n",player.socket);
    fprintf(stderr,"player handle: %d\n",player.ph);
    fprintf(stderr,"cards: ");
    PrintCards(player.cards,player.numcards);
    fprintf(stderr,"number of cards: %d\n",player.numcards);
    fprintf(stderr,"team: %d\n",player.team);
    fprintf(stderr,"game creator: %d\n",player.creator);
    fprintf(stderr,"player name: %s\n",player.playername);
    fprintf(stderr,"hardware: %s\n",player.hardware);
    fprintf(stderr,"OS: %s\n",player.OS);
    fprintf(stderr,"client name: %s\n",player.clientname);
    fprintf(stderr,"player comment: %s\n",player.comment);
    fprintf(stderr,"ip: %s\n",player.ip);
    fprintf(stderr,"number of holes called: %d\n",player.holecalled);
    fprintf(stderr,"Number of holes made: %d\n",player.holemade);
    fprintf(stderr,"Number of trump called: %d\n",player.trumpcalled);
    fprintf(stderr,"Number of trump made: %d\n",player.trumpsmade);
    fprintf(stderr,"Number of alones: %d\n",player.handsalonecalled);
    fprintf(stderr,"Number of alones made: %d\n",player.handsalonemade);
    fprintf(stderr,"Number of defends: %d\n",player.defendalonescalled);
    fprintf(stderr,"Number of defends made: %d\n",player.defendalonesmade);
    fprintf(stderr,"Number of tricks made: %d\n",player.tricksmade);
    fprintf(stderr,"Number of anti-tricks made: %d\n",player.antitricksmade);
    fprintf(stderr,"Number of times euchered: %d\n",player.euchres);
    fprintf(stderr,"Number of times screwed: %d\n",player.screwed);
    fprintf(stderr,"Number of messages: %d\n",player.nummsgs);
    fprintf(stderr,"Volume of messages (bytes): %d\n",player.volmsgs);
    fprintf(stderr,"Play offered: %d\n",player.playoffer);
    fprintf(stderr,"Order offered: %d\n",player.orderoffer);
    fprintf(stderr,"Drop offered: %d\n",player.dropoffer);
    fprintf(stderr,"Call offered: %d\n",player.calloffer);
    fprintf(stderr,"Defend offered: %d\n",player.defendoffer);
    fprintf(stderr,"Dealt: %d\n",player.dealer);
    fprintf(stderr,"Ordered: %d\n",player.ordered);
    fprintf(stderr,"Lead: %d\n",player.leader);
    fprintf(stderr,"Maker: %d\n",player.maker);
    fprintf(stderr,"Defending: %d\n",player.defend);
    fprintf(stderr,"Going Alone: %d\n",player.alone);
    if (player.cardinplay)
    {
        fprintf(stderr,"Card in play: ");
        PrintCard(player.card);
        fprintf(stderr,"\n");
    }
    fprintf(stderr,"Has passed: %d\n",player.passed);
}


/* Prints the pstate enumerated type */
void PrintPstate(pstate state)
{
    debug(GENERAL) fprintf(stderr,"entering PrintPstate()\n");

    if (state==unconnected)
        fprintf(stderr,"unconnected");
    if (state==connected)
        fprintf(stderr,"connected");
    if (state==joined)
        fprintf(stderr,"joined");
}


/* prints the first n ints off the top of a buffer */
void PrintBuffer(char *buffer, int len)
{
    int i,*ptr;

    debug(GENERAL) fprintf(stderr,"entering PrintBuffer()\n");

    /* we set ptr to buffer, so that we can access ptr via ints, without
     * having to cast everything when calling PrintBuffer()
     */
    ptr=(int *)buffer;

    fprintf(stderr,"%d bytes: ",len);
    fprintf(stderr,"%d ints: ",len/4);
    for (i=0; i<len/4; i++)
        fprintf(stderr,"%d ",ntohl(ptr[i]));
    fprintf(stderr,"\n");
}


/* Prints an numcards-element array of cards */
void PrintCards(Card *cards, int numcards)
{
    int i;

    for (i=0; i<numcards; i++)
        PrintCard(cards[i]);
    fprintf(stderr,"\n");
}


/* Prints a single card */
void PrintCard(Card card)
{
    if (card.value < 11) fprintf(stderr,"%d",card.value);
    else if (card.value == 11) fprintf(stderr,"J");
    else if (card.value == 12) fprintf(stderr,"Q");
    else if (card.value == 13) fprintf(stderr,"K");
    else if (card.value == 14) fprintf(stderr,"A");

    if (card.suit == 0) fprintf(stderr,"c");
    if (card.suit == 1) fprintf(stderr,"d");
    if (card.suit == 2) fprintf(stderr,"h");
    if (card.suit == 3) fprintf(stderr,"s");

    fprintf(stderr," ");
}


/* returns a text string with the text name of a card in it */
char *CardText(Card card)
{
    sprintf(tbuffer3,"%s%s",ValueText(card.value),SuitText(card.suit));
    return(tbuffer3);
}


/* returns a text string with the text name of a card in it */
char *SuitText(int suit)
{
    if (suit == 0) sprintf(tbuffer1,"c");
    if (suit == 1) sprintf(tbuffer1,"d");
    if (suit == 2) sprintf(tbuffer1,"h");
    if (suit == 3) sprintf(tbuffer1,"s");

    return(tbuffer1);
}


/* returns a text string with the text name of a card in it */
char *ValueText(int value)
{
    if (value < 11) sprintf(tbuffer2,"%d",value);
    else if (value == 11) sprintf(tbuffer2,"J");
    else if (value == 12) sprintf(tbuffer2,"Q");
    else if (value == 13) sprintf(tbuffer2,"K");
    else if (value == 14) sprintf(tbuffer2,"A");

    return(tbuffer2);
}


/* Prints the hand structure */
void PrintHand(Hand hand)
{
    fprintf(stderr,"Hand state is: ");
    fprintf(stderr,"dealt by %d, ",hand.dealer);

    if (hand.suit > -1)
    {
        fprintf(stderr,"trump suit is ");
        if (hand.suit == 0) fprintf(stderr,"c, ");
        if (hand.suit == 1) fprintf(stderr,"d, ");
        if (hand.suit == 2) fprintf(stderr,"h, ");
        if (hand.suit == 3) fprintf(stderr,"s, ");
        fprintf(stderr,"called by %s, team %d ",
            players[hand.maker].playername,players[hand.maker].team);
    }
    else
        fprintf(stderr,"no trump, ");

    if (hand.hole.suit > -1)
    {
        fprintf(stderr,"hole card is: ");
        PrintCard(hand.hole);
        fprintf(stderr,", ");
    }
    else
        fprintf(stderr,"no hole card, ");

    fprintf(stderr,"tricks are %d to %d, ",hand.tricks[0],hand.tricks[1]);

    fprintf(stderr,"current state is: ");
    if (hand.hstate == 0) fprintf(stderr,"pre-game\n");
    if (hand.hstate == 1) fprintf(stderr,"order offer\n");
    if (hand.hstate == 2) fprintf(stderr,"call offer\n");
    if (hand.hstate == 3) fprintf(stderr,"defend offer\n");
    if (hand.hstate == 4) fprintf(stderr,"in play\n");
}


/* takes a suit and prints the long form of the name */
char *PrintSuit(int suit)
{
    if (suit==0) return("clubs");
    if (suit==1) return("diamonds");
    if (suit==2) return("hearts");
    if (suit==3) return("spades");
    return("Unknown Suit");
}
