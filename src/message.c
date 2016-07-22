
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 */

/* This file contains routines used to prepare and send messages over
 * the player sockets.
 */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>


/* specific euchred includes */
#include "euchred.h"


/* A short routine to send a decline message to a client who connects
 * after the game is full.  The Send*() routines are typically passed
 * the player number: this one is an exception, due to the fact that
 * we're declining a player who has no player structure.
 */
void SendDecline(int sock, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendDecline()\n");

    /* <msglen> <DECLINE> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,DECLINE);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(sock,dbuffer,size+spacer);
}


/* This routine takes a player index and a text string (the reason)
 * and sends a KICK message to the specified client.
 */
void SendKick(int p, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendKick()\n");

    /* <msglen> <KICK> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,KICK);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[p].socket,dbuffer,size+spacer);
}


/* This routine sends a JOINDENY message, followed by a reason */
void SendJoinDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendJoinDeny()\n");

    /* <msglen> <JOINDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,JOINDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* Sends a join acknowledgement.  With the single game server, the game
 * handle is irrelevant and the player handle is just the player number.
 */
void SendJoinAccept(int pnum)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendJoinAccept()\n");

    /* <msglen> <JOINACCEPT> <gh> <ph> <team> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,JOINACCEPT);
    size+=PackInt(dbuffer+spacer+size,game.gh);
    size+=PackInt(dbuffer+spacer+size,players[pnum].ph);
    size+=PackInt(dbuffer+spacer+size,players[pnum].team);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* Sends a quit message, along with a reason.  */
void SendQuit(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendQuit()\n");

    /* <msglen> <SERVERQUIT> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,SERVERQUIT);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* Broadcasts a chat message to all connected clients */
void SendChat(char *chat)
{
    int size=0,i,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendChat()\n");

    /* <msglen> <CHAT> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,CHAT);
    size+=PackString(dbuffer+spacer+size,chat);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        if (players[i].state != unconnected)
            SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* Sends the state of the current game.  The intention is that this will
 * provide enough information for a virgin client to join the game.  This
 * routine will send the state information to all player.
 */
void SendState()
{
    int size=0,i,tsize=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendState()\n");

    /* <msg> : <msglen> <STATE> <playersdata> <gamedata> <cards> <tail> */
    spacer=sizeof(size);
    size=PackInt(dbuffer+spacer,STATE);

    /* pack all the players in */
    for (i=0; i<4; i++)
        size+=PackPlayer(dbuffer+spacer+size,i);

    /* and then the game data
     * <gamedata> : <ingame> <suspend> <holein> <hole> <trumpset> <trump>
     *              <tricks> <score> <options>
     *   <ingame> : <boolean>
     *   <hstate> : <0|1|2|3|4> # pregame, hole, trump, defend, play
     *   <suspend> : <boolean>
     *   <holein> : <boolean> # true if hole card
     *   <hole> : <card> # only packed if <holein> true
     *     <card> : <value> <suit>
     *   <trumpset> : <boolean> # true if trump set
     *   <trump> : <suit> # only packed if <trumpset> true
     *   <tricks> : <tricks0> <tricks1>
     *     <tricks0> : # tricks for team 0
     *     <tricks1> : # tricks for team 1
     *   <score> : <team0> <team1>
     *     <team0> : # score of team 0
     *     <team1> : # score of team 1
     *   <options> : <alone> <defend> <aloneonorder> <screw>
     *     <alone>|<defend>|<aloneonorder>|<screw> : <boolean>
     */

    /* status */
    size+=PackBoolean(dbuffer+spacer+size,game.ingame);
    size+=PackInt(dbuffer+spacer+size,hand.hstate);
    size+=PackBoolean(dbuffer+spacer+size,game.suspend);

    /* hole and trump, if set */
    if (hand.hole.suit > -1)
    {
        size+=PackBoolean(dbuffer+spacer+size,true);
        size+=PackCard(dbuffer+spacer+size,hand.hole);
    }
    else
        size+=PackBoolean(dbuffer+spacer+size,false);

    if (hand.suit > -1)
    {
        size+=PackBoolean(dbuffer+spacer+size,true);
        size+=PackInt(dbuffer+spacer+size,hand.suit);
    }
    else
        size+=PackBoolean(dbuffer+spacer+size,false);

    /* tricks and score */
    size+=PackInt(dbuffer+spacer+size,hand.tricks[0]);
    size+=PackInt(dbuffer+spacer+size,hand.tricks[1]);
    size+=PackInt(dbuffer+spacer+size,game.score[0]);
    size+=PackInt(dbuffer+spacer+size,game.score[1]);

    /* options */
    size+=PackBoolean(dbuffer+spacer+size,game.candefend);
    size+=PackBoolean(dbuffer+spacer+size,game.aloneonorder);
    size+=PackBoolean(dbuffer+spacer+size,game.screw);

    /* together with the cards for each player */
    for (i=0; i<4; i++)
        if (players[i].state == joined)
        {
            tsize=size;
            tsize+=PackCards(dbuffer+spacer+tsize,i);
            tsize+=PackShort(dbuffer+spacer+tsize,TAIL);
            PackInt(dbuffer,tsize);
            SendMsg(players[i].socket,dbuffer,tsize+spacer);
        }
}


/* This routine sends a KICKDENY message, followed by a reason */
void SendKickDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendKickDeny()\n");

    /* <msglen> <KICKDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,KICKDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends an OPTIONSDENY message, followed by a reason */
void SendOptionsDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendOptionsDeny()\n");

    /* <msglen> <OPTIONSDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,OPTIONSDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends a STARTDENY message, followed by a reason */
void SendStartDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendStartDeny()\n");

    /* <msglen> <STARTDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,STARTDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends an ENDDENY message, followed by a reason */
void SendEndDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendEndDeny()\n");

    /* <msglen> <ENDDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,ENDDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends an ORDERDENY message, followed by a reason */
void SendOrderDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendOrderDeny()\n");

    /* <msglen> <ORDERDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,ORDERDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends a DROPDENY message, followed by a reason */
void SendDropDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendDropDeny()\n");

    /* <msglen> <DROPDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,DROPDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends a CALLDENY message, followed by a reason */
void SendCallDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendCallDeny()\n");

    /* <msglen> <CALLDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,CALLDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends a DEFENDDENY message, followed by a reason */
void SendDefendDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendDefendDeny()\n");

    /* <msglen> <DEFENDDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,DEFENDDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends a PLAYDENY message, followed by a reason */
void SendPlayDeny(int pnum, char *reason)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendPlayDeny()\n");

    /* <msglen> <PLAYDENY> <string> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,PLAYDENY);
    size+=PackString(dbuffer+spacer+size,reason);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends a TRICKOVER message to all players */
void SendTrickOver()
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendTrickOver()\n");

    /* <msglen> <TRICKOVER> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,TRICKOVER);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* This routine sends a HANDOVER message to all players */
void SendHandOver()
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendHandOver()\n");

    /* <msglen> <HANDOVER> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,HANDOVER);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* This routine sends a GAMEOVER message to all players */
void SendGameOver()
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendGameOver()\n");

    /* <msglen> <GAMEOVER> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,GAMEOVER);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* This routine sends a PLAYOFFER message to all players */
void SendPlayOffer(int pnum)
{
    int size=0,spacer;

    debug(GENERAL) fprintf(stderr,"entering SendPlayOffer()\n");

    /* <msglen> <PLAYOFFER> <pnum> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,PLAYOFFER);
    size+=PackInt(dbuffer+spacer+size,pnum);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    //for (int i=0; i<4; i++)
    //    SendMsg(players[i].socket,dbuffer,size+spacer);
    SendMsg(players[pnum].socket,dbuffer,size+spacer);
}


/* This routine sends a DEFENDOFFER message to all players */
void SendDefendOffer(int pnum)
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendDefendOffer()\n");

    /* <msglen> <DEFENDOFFER> <pnum> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,DEFENDOFFER);
    size+=PackInt(dbuffer+spacer+size,pnum);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* This routine sends a CALLOFFER message to all players */
void SendCallOffer(int pnum)
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendCallOffer()\n");

    /* <msglen> <CALLOFFER> <pnum> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,CALLOFFER);
    size+=PackInt(dbuffer+spacer+size,pnum);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* This routine sends a ORDEROFFER message to all players */
void SendOrderOffer(int pnum)
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendOrderOffer()\n");

    /* <msglen> <ORDEROFFER> <pnum> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,ORDEROFFER);
    size+=PackInt(dbuffer+spacer+size,pnum);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* This routine sends a DROPOFFER message to all players */
void SendDropOffer(int pnum)
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendDropOffer()\n");

    /* <msglen> <DROPOFFER> <pnum> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,DROPOFFER);
    size+=PackInt(dbuffer+spacer+size,pnum);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}


/* This routine sends a DEAL message to all players */
void SendDeal()
{
    int size=0,spacer,i;

    debug(GENERAL) fprintf(stderr,"entering SendDeal()\n");

    /* <msglen> <DEAL> <tail> */
    spacer=sizeof(size);
    size =PackInt(dbuffer+spacer,DEAL);
    size+=PackShort(dbuffer+spacer+size,TAIL);
    PackInt(dbuffer,size);

    for (i=0; i<4; i++)
        SendMsg(players[i].socket,dbuffer,size+spacer);
}
