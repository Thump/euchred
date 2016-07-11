
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 */

/* This file stores routines which manipulate the players structure */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>


/* specific euchred includes */
#include "euchred.h"


/* Service inbound data request on client socket */
void ServiceClient(int pnum)
{
    int request;

    debug(GENERAL) fprintf(stderr,"entering ServiceClient()\n");

    /* this handles fragmented messages */
    if ( ! MsgReady(pnum) )
        return;

    /* right, we've got all the data: read and parse the message ID */
    ReadMsg(players[pnum].socket,(char *)&request,4);
    request=UnpackInt(request);

    switch ( request )
    {
        case JOIN:
            JoinClient(pnum);
            break;

        case CLIENTQUIT:
            ClientQuit(pnum);
            break;

        case ID:
            ClientID(pnum);
            break;

        case CHAT:
            ClientChat(pnum);
            break;

        case KICKPLAYER:
            ClientKick(pnum);
            break;

        case OPTIONS:
            ClientOptions(pnum);
            break;

        case START:
            ClientStart(pnum);
            break;

        case END:
            ClientEnd(pnum);
            break;

        case ORDER:
            ClientOrder(pnum);
            break;

        case ORDERALONE:
            ClientOrderAlone(pnum);
            break;

        case ORDERPASS:
            ClientOrderPass(pnum);
            break;

        case DROP:
            ClientDrop(pnum);
            break;

        case CALL:
            ClientCall(pnum);
            break;

        case CALLALONE:
            ClientCallAlone(pnum);
            break;

        case CALLPASS:
            ClientCallPass(pnum);
            break;

        case DEFEND:
            ClientDefend(pnum);
            break;

        case DEFENDPASS:
            ClientDefendPass(pnum);
            break;

        case PLAY:
            ClientPlay(pnum);
            break;

        default:
            ClientGarbage(pnum);
            break;
    }
}


/* This routine handles fragmented messages: if there are no pending
 * messages for the client, we read the first byte from the socket.
 * If there is no data available, the client has disconnected, so we
 * dump them. If we can successfully read the first integer, we
 * interpret that as the expected length of the total message.  If there
 * are sufficient bytes to make up the message, we return true.  If not,
 * we set the msgpending flag in the player structure with the value of
 * the expected message length, stamp the incoming message time, and
 * return false.
 *
 * If, on entering MsgReady(), we see the message pending flag is set,
 * we read our expected length from the player structure.  We then
 * check if there are that many bytes available.  If there are enough,
 * return true, otherwise false.  We don't check for time overruns
 * here, we do that in a generic maintenance routine in game.c.
 */
boolean MsgReady(int pnum)
{
    int msglen,len,available;

    debug(GENERAL) fprintf(stderr,"entering MsgReady()\n");

    /* if we're not already waiting for a message */
    if ( ! players[pnum].msgwait )
    {
        len=ReadMsg(players[pnum].socket,(char *)&msglen,4);

        /* this deals with disconnected clients */
        if (len==0)
        {
            ClientDisconnect(pnum);
            return(false);
        }

        msglen=UnpackInt(msglen);

        /* drop clients with too long messages */
        if (msglen > maxmsglen)
        {
            sprintf(tbuffer3,"Message packet from %d (%s) too long: %d.",
                pnum,players[pnum].playername,msglen);
            KickClient(pnum,tbuffer3);
            return(false);
        }

        /* check if we have enough data pending on the socket */
        ioctl(players[pnum].socket,FIONREAD,&available);
        if (available < msglen)
        {
            sprintf(tbuffer1,
                "Short message from %d (%s): expecting %d, got %d",
                pnum,players[pnum].playername,msglen,available);
            myLog(tbuffer1);
            players[pnum].msgwait=true;
            players[pnum].msgtime=time(NULL);
            players[pnum].msglen=msglen;
            return(false);
        }
        else
        { return(true); }
    }
    else
    {
        msglen=players[pnum].msglen;

        /* check if we have enough data pending on the socket */
        ioctl(players[pnum].socket,FIONREAD,&available);
        if (available < msglen)
            return(false);
        else
        {
            players[pnum].msgwait=false;
            players[pnum].msgtime=0;
            players[pnum].msglen=0;
            return(true);
        }
    }
}


/* This routine checks if we have room for a new client: it calls
 * DeclineClient() if not, or calls the AddClient routine if so.
 *
 * It assumes the server socket is ready to have an accept() performed
 * on it.
 */
void NewClient()
{
    debug(GENERAL) fprintf(stderr,"entering NewClient()\n");

    if (game.players + game.bots < 4)
        AddClient();
    else
        DeclineClient();
}


/* This routine adds a new client, but effects no communication.  If this
 * is the first client to connect, the client is accorded game creator
 * status.
 */
void AddClient()
{
    int i,client;
    char *ip;

    debug(GENERAL) fprintf(stderr,"entering AddClient()\n");

    ip=AcceptSocket(&client);

    for (i=0; i<4; i++)
    {
        if (players[i].socket == undefined)
            break;
    }

    /* This catches the fall through case, when there are no free player
     * slots.  This should not happen, unless clients being added are not
     * being properly tracked in game.players.
     */
    if (i == 4)
    {
        myLog("Bad AddClient, run your might and tell Denis");
        close(client);
        return;
    }

    game.players++;
    players[i].socket=client;
    players[i].ip=ip;
    sprintf(tbuffer1,"Player %d",i);
    players[i].playername=strdup(tbuffer1);
    lomem(players[i].playername);
    players[i].state=connected;
    players[i].ph=i;
    players[i].joinwait=true;
    players[i].msgtime=time(NULL);

    if (game.players == 1)
    {
        players[i].creator=true;
        sprintf(tbuffer1,"Client %d (%s) connected, assigned creator",i,
            players[i].ip);
    }
    else
    {
        players[i].creator=false;
        sprintf(tbuffer1,"Client %d (%s) added",i,players[i].ip);
    }

    myLog(tbuffer1);
}


/* This routine kicks a user for a specific reason: a KICK packet is
 * sent to the client, a message is logged, and the client is removed.
 */
void KickClient(int pnum, char *reason)
{
    char *name;

    debug(GENERAL) fprintf(stderr,"entering KickClient()\n");

    /* copy the name */
    name=strdup(players[pnum].playername);
    lomem(name);

    /* log our action */
    sprintf(tbuffer1,"Client %d (%s) kicked: %s",pnum,name,reason);
    myLog(tbuffer1);

    /* send the message to the client */
    SendKick(pnum,reason);
    RemoveClient(pnum);

    /* announce it */
    sprintf(tbuffer2,"Server: %s has been kicked: %s.",name,reason);
    SendChat(tbuffer2);

    SendState();
    free(name);
}


/* This routine declines a new client by accepting the connection,
 * sending a decline message, and then closing the connection.  We don't
 * need to RemoveClient() since this is done before the client was added.
 */
void DeclineClient()
{
    int client;
    char *ip;

    debug(GENERAL) fprintf(stderr,"entering DeclineClient()\n");

    ip=AcceptSocket(&client);
    SendDecline(client,"Sorry big guy, game's full...\n");

    sprintf(tbuffer1,"Client from %s declined: game is full",ip);
    myLog(tbuffer1);

    free(ip);
    close(client);
}


/* This is the routine that joins a client.  It assumes that any given server
 * will always maintain backwards compatibility with previous protocol versions
 */
void JoinClient(int pnum)
{
    int cprotocol;
    char *tmp;

    debug(GENERAL) fprintf(stderr,"entering JoinClient()\n");

    sprintf(tbuffer1,"Servicing %d (%s): JOIN",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <JOIN> <protocol> <string> <tail> */
    if ( ! ReadInt(pnum,&cprotocol) ) return;
    if ( (tmp=ReadString(pnum)) == NULL ) return;
    if ( ! TailCheck(pnum)) return;

    /* parse and use the data */
    if (players[pnum].state==joined)
    {
        sprintf(tbuffer1,"Duplicate JOIN: client %d, protocol %d: ignoring",
            pnum,cprotocol);
        myLog(tbuffer1);
        return;
    }

    if (cprotocol > protocol)
    {
        sprintf(tbuffer1,"Sorry, unsupported protocol version: %d\n",
            cprotocol);
        SendJoinDeny(pnum,tbuffer1);
        sprintf(tbuffer1,"Client %d joined with unsupported protocol: %d",
            pnum,cprotocol);
        myLog(tbuffer1);
        RemoveClient(pnum);
        return;
    }

    /* set the player's name and status */
    if ( strlen(tmp) > 0)
    {
        if (players[pnum].playername != NULL)
            free(players[pnum].playername);
        players[pnum].playername=tmp;
        players[pnum].state=joined;
        players[pnum].joinwait=false;
        players[pnum].msgtime=0;

        /* choose and set team */
        if (pnum==0 || pnum==2)
            players[pnum].team=1;
        else
            players[pnum].team=2;
    }

    /* and log some information */
    sprintf(tbuffer1,"Client %d (%s) joined (protocol version %d)",
        pnum,players[pnum].playername,cprotocol);
    myLog(tbuffer1);

    /* and send the join accept as well as a chat message to all players */
    SendJoinAccept(pnum);
    sprintf(tbuffer1,"Server: %s has joined the game.",
        players[pnum].playername);
    SendChat(tbuffer1);

    /* and finally, send out state messages */
    SendState();
}


/* Routine to remove client from data structures */
void RemoveClient(int pnum)
{
    debug(GENERAL) fprintf(stderr,"entering RemoveClient()\n");

    sprintf(tbuffer1,"Client %d (%s) removed",pnum,players[pnum].playername);

    /* disconnect player and undo all the player settings: we don't
     * use ClearPlayer() here because there are parts of the player
     * structure we want to keep around
     */
    close(players[pnum].socket);
    players[pnum].socket=undefined;
    free(players[pnum].ip);
    players[pnum].ip=NULL;
    if ( players[pnum].playername != NULL)
        free(players[pnum].playername);
    players[pnum].playername=strdup(DEFNAME);
    lomem(players[pnum].playername);
    players[pnum].state=unconnected;
    players[pnum].joinwait=false;
    players[pnum].msgwait=false;
    players[pnum].msglen=0;
    players[pnum].msgtime=0;
    players[pnum].ph=-1;
    game.players--;

    if (players[pnum].creator)
    {
        players[pnum].creator=false;
        PickCreator();
    }

    myLog(tbuffer1);

    /* if there are no players left, reset the game and clear the players */
    if (game.players == 0)
    {
        ClearHand();
        ClearGame();
        ClearPlayer(0);
        ClearPlayer(1);
        ClearPlayer(2);
        ClearPlayer(3);
        myLog("No players left: resetting game");
    }
}


/* This routine deals with disconnected clients */
void ClientDisconnect(int pnum)
{
    char *name;

    debug(GENERAL) fprintf(stderr,"entering ClientDisconnect()\n");

    sprintf(tbuffer1,"client %d unexpectedly disconnected",pnum);
    myLog(tbuffer1);
    name=strdup(players[pnum].playername);
    lomem(name);

    RemoveClient(pnum);

    sprintf(tbuffer2,"Server: %s has unexpectedly disconnected.",name);
    SendChat(tbuffer2);
    free(name);

    SendState();
}


/* This routine processes the orderly quit of the client */
void ClientQuit(int pnum)
{
    int gh,ph;
    char *reason;

    debug(GENERAL) fprintf(stderr,"entering ClientQuit()\n");

    sprintf(tbuffer1,"Servicing %d (%s): QUIT",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <CLIENTQUIT> <gh> <ph> <string> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( (reason=ReadString(pnum)) == NULL ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    sprintf(tbuffer1,"client %d has quit, saying: \"%s\"",pnum,reason);
    myLog(tbuffer1);

    sprintf(tbuffer2,"Server: %s has quit, saying: \"%s\".",
        players[pnum].playername,reason);
    SendChat(tbuffer2);

    RemoveClient(pnum);
    SendState();
    free(reason);
}


/* This routine processes a client ID message */
void ClientID(int pnum)
{
    int gh,ph;
    char *name,*client,*hw,*os,*cmt;

    debug(GENERAL) fprintf(stderr,"entering ID()\n");

    sprintf(tbuffer1,"Servicing %d (%s): ID",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <ID> <gameh> <playerh> <data> <tail>
     *   <data> : <nmstring> <clstring> <hwstring> <osstring> <cmtstring>
     */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( (name=ReadString(pnum)) == NULL ) return;
    if ( (client=ReadString(pnum)) == NULL ) return;
    if ( (hw=ReadString(pnum)) == NULL ) return;
    if ( (os=ReadString(pnum)) == NULL ) return;
    if ( (cmt=ReadString(pnum)) == NULL ) return;
    if ( ! TailCheck(pnum)) return;


    /* check the player handle matches */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* process player name */
    if (players[pnum].playername != NULL)
    {
        if ( strcmp(players[pnum].playername,name) )
        {
            sprintf(tbuffer2,"Server: %s has changed name to %s.",
                players[pnum].playername,name);
            SendChat(tbuffer2);
        }
        free(players[pnum].playername);
    }
    players[pnum].playername=name;

    /* process client name */
    if (players[pnum].clientname != NULL)
        free(players[pnum].clientname);
    players[pnum].clientname=client;

    /* process hardware name */
    if (players[pnum].hardware != NULL)
        free(players[pnum].hardware);
    players[pnum].hardware=hw;

    /* process OS name */
    if (players[pnum].OS != NULL)
        free(players[pnum].OS);
    players[pnum].OS=os;

    /* process comment */
    if (players[pnum].comment != NULL)
        free(players[pnum].comment);
    players[pnum].comment=cmt;


    sprintf(tbuffer1,"Received ID from client %d: %s",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* send state message with updated info to players */
    SendState();
}


/* This routine processes a client chat message */
void ClientChat(int pnum)
{
    int gh,ph;
    char *chat;

    debug(GENERAL) fprintf(stderr,"entering ClientChat()\n");

    sprintf(tbuffer1,"Servicing %d (%s): CHAT",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <CHAT> <gameh> <playerh> <string> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( (chat=ReadString(pnum)) == NULL ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    sprintf(tbuffer1,"received chat from client %d: \"%s\"",pnum,chat);
    myLog(tbuffer1);

    sprintf(tbuffer1,"%s: %s",players[pnum].playername,chat);
    SendChat(tbuffer1);
    free(chat);
}


/* This routine is called after processing each client request: it expects
 * to be able to read the EDFOOFOO short waiting on client pnum's socket.
 * If it can, then it returns TRUE, otherwise FALSE.
 */
boolean TailCheck(int pnum)
{
    short tmp;

    debug(GENERAL) fprintf(stderr,"entering TailCheck()\n");

    if ( ! ReadShort(pnum,&tmp) ) return(false);

    if (tmp != TAIL)
    {
        printf(tbuffer1,"Bad tail from %d (%s)",
            pnum,players[pnum].playername);
        myLog(tbuffer1);
        KickClient(pnum,"Bad tail");
        return(false);
    }

    return(true);
}


/* This routine processes a client that has sent in an unparseable message:
 * boot to the head
 */
void ClientGarbage(int pnum)
{
    debug(GENERAL) fprintf(stderr,"entering ClientGarbage()\n");

    sprintf(tbuffer1,"Received garbage from %d (%s)",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    KickClient(pnum,"Garbage in, garbage out, okay?");
}


/* This routine processes a client kick request: it checks that the
 * client has creator priviledges, and that the target player exists
 * and is not equal to the player
 */
void ClientKick(int pnum)
{
    int gh,ph,targetph;

    debug(GENERAL) fprintf(stderr,"entering ClientKick()\n");

    sprintf(tbuffer1,"Servicing %d (%s): CLIENTKICK",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <CLIENTKICK> <gh> <ph> <targetph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! ReadInt(pnum,&targetph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that they're not kicking themselves */
    if (ph == targetph)
    {
        SendKickDeny(pnum,"Can't kick yourself.");
        sprintf(tbuffer1,"Player %s tried to self-kick",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that they have creator status */
    if ( !players[pnum].creator )
    {
        SendKickDeny(pnum,"Must be creator to kick users.");
        sprintf(tbuffer1,"Player %s tried to kick without being creator",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Okay, looks cool, les' do it */
    sprintf(tbuffer3,"Player %s kicked by request of %s",
        players[targetph].playername,players[pnum].playername);
    KickClient(targetph,"creator request");
    myLog(tbuffer3);
}


/* This routine processes the options message.  Only creators should
 * send this message.
 */
void ClientOptions(int pnum)
{
    int gh,ph;
    boolean candefend,aloneonorder,screw;

    debug(GENERAL) fprintf(stderr,"entering ClientOptions()\n");

    sprintf(tbuffer1,"Servicing %d (%s): OPTIONS",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <msglen> <OPTIONS> <gh> <ph> <candefend>
     *         <aloneonorder> <screw> <tail>
     */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! ReadBoolean(pnum,&candefend) ) return;
    if ( ! ReadBoolean(pnum,&aloneonorder) ) return;
    if ( ! ReadBoolean(pnum,&screw) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that they have creator status */
    if ( !players[pnum].creator )
    {
        SendOptionsDeny(pnum,"Must be creator to set options.");
        sprintf(tbuffer1,"Player %s tried to set options without being creator",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the game hasn't started */
    if ( game.ingame )
    {
        SendOptionsDeny(pnum,"Can't change options after game start.");
        sprintf(tbuffer1,"Player %s tried to set options after game start",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Okay, looks cool, les' do it */
    game.candefend=candefend;
    game.aloneonorder=aloneonorder;
    game.screw=screw;

    SendState();
    sprintf(tbuffer3,"Options set by %s",players[pnum].playername);
    myLog(tbuffer3);
}


/* This routine processes a client request to start the game */
void ClientStart(int pnum)
{
    int gh,ph;

    debug(GENERAL) fprintf(stderr,"entering ClientStart()\n");

    sprintf(tbuffer1,"Servicing %d (%s): START",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <START> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that they have creator status */
    if ( !players[pnum].creator )
    {
        SendStartDeny(pnum,"Must be creator to start game.");
        sprintf(tbuffer1,"Player %s tried to start game without being creator",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the game hasn't already started */
    if ( game.ingame )
    {
        SendStartDeny(pnum,"Game already started.");
        sprintf(tbuffer1,"Player %s tried to restart game",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that there are four players ready */
    if ( game.players < 4 )
    {
        SendStartDeny(pnum,"Need four players to start game.");
        sprintf(tbuffer1,"Player %s tried to prestart game",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* tell it */
    sprintf(tbuffer1,"Server: %s has started the game.",
        players[pnum].playername);
    SendChat(tbuffer1);

    /* then do it */
    game.ingame=true;
    NewHand();
    SendState();
    SendDeal();
    NextAction();
}


/* This routine processes a client request to end the game */
void ClientEnd(int pnum)
{
    int gh,ph,i;

    debug(GENERAL) fprintf(stderr,"entering ClientEnd()\n");

    sprintf(tbuffer1,"Servicing %d (%s): END",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <END> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that they have creator status */
    if ( !players[pnum].creator )
    {
        SendEndDeny(pnum,"Must be creator to end game.");
        sprintf(tbuffer1,"Player %s tried to end game without being creator",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* then do it */
    sprintf(tbuffer3,"%s has ended the game",players[pnum].playername);
    for (i=0; i<4; i++)
        if (players[i].state==joined)
        {
            SendQuit(i,tbuffer3);
            RemoveClient(i);
        }

    ClearHand();
    ClearGame();
}


/* This routine processes a client request to order up trump */
void ClientOrder(int pnum)
{
    int gh,ph;

    debug(GENERAL) fprintf(stderr,"entering ClientOrder()\n");

    sprintf(tbuffer1,"Servicing %d (%s): ORDER",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <ORDER> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendOrderDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-order",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].orderoffer )
    {
        SendOrderDeny(pnum,"Order not offered.");
        sprintf(tbuffer1,"Player %s tried to order without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* go alone if the dealer is on the same team */
    if (game.aloneonorder &&
        players[pnum].team==players[hand.dealer].team && !players[pnum].dealer)
        players[pnum].alone=true;

    /* set trump, change to defendoffer state, tell folks, NextAction() */
    hand.suit=hand.hole.suit;
    SortCards();
    players[pnum].orderoffer=false;
    players[pnum].maker=true;
    hand.maker=pnum;

    if (!players[pnum].dealer &&
        players[pnum].team == players[hand.dealer].team &&
        game.aloneonorder )
    {
        if ( game.candefend )
            hand.hstate=3;
        else
            hand.hstate=4;
        NextAction();
    }
    else
    {
        players[hand.dealer].dropoffer=true;
        SendDropOffer(hand.dealer);
    }

    if ( players[pnum].dealer )
    {
        sprintf(tbuffer1,"Server: %s picked up %s.",
            players[pnum].playername,PrintSuit(hand.suit));
    }
    else
    {
        sprintf(tbuffer1,"Server: %s ordered %s up on %s.",
            players[pnum].playername,players[hand.dealer].playername,
            PrintSuit(hand.suit));
    }
    SendChat(tbuffer1);

    SendState();
}


/* This routine processes a client request to order up trump alone */
void ClientOrderAlone(int pnum)
{
    int gh,ph;

    debug(GENERAL) fprintf(stderr,"entering ClientOrderAlone()\n");

    sprintf(tbuffer1,"Servicing %d (%s): ORDERALONE",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <ORDERALONE> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendOrderDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-order",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].orderoffer )
    {
        SendOrderDeny(pnum,"Order not offered.");
        sprintf(tbuffer1,"Player %s tried to order without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* set trump, change to defendoffer state, tell folks, NextAction() */
    hand.suit=hand.hole.suit;
    SortCards();
    players[pnum].orderoffer=false;
    players[pnum].alone=true;
    players[pnum].maker=true;
    hand.maker=pnum;

    if (!players[pnum].dealer && players[pnum].team==players[hand.dealer].team)
    {
        if ( game.candefend )
            hand.hstate=3;
        else
            hand.hstate=4;
        NextAction();
    }
    else
    {
        players[hand.dealer].dropoffer=true;
        SendDropOffer(hand.dealer);
    }

    if ( players[pnum].dealer )
    {
        sprintf(tbuffer1,"Server: %s picked up %s, and is going alone.",
            players[pnum].playername,PrintSuit(hand.suit));
    }
    else
    {
        sprintf(tbuffer1,"Server: %s ordered %s up on %s, and is going alone.",
            players[pnum].playername,players[hand.dealer].playername,
            PrintSuit(hand.suit));
    }
    SendChat(tbuffer1);

    SendState();
}


/* This routine processes a client request to pass on ordering up trump */
void ClientOrderPass(int pnum)
{
    int gh,ph;

    debug(GENERAL) fprintf(stderr,"entering ClientOrderPass()\n");

    sprintf(tbuffer1,"Servicing %d (%s): ORDERPASS",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <ORDERPASS> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendOrderDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-order",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].orderoffer )
    {
        SendOrderDeny(pnum,"Order not offered.");
        sprintf(tbuffer1,"Player %s tried to order without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* tell folks, NextAction() */
    sprintf(tbuffer1,"Server: %s has passed on %s.",
        players[pnum].playername,PrintSuit(hand.hole.suit));
    SendChat(tbuffer1);

    NextAction();
    SendState();
}


/* This routine processes a client card drop request */
void ClientDrop(int pnum)
{
    int gh,ph;
    Card card;

    debug(GENERAL) fprintf(stderr,"entering ClientDrop()\n");

    sprintf(tbuffer1,"Servicing %d (%s): DROP",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <DROP> <gh> <ph> <card.value> <card.suit> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! ReadInt(pnum,&card.value) ) return;
    if ( ! ReadInt(pnum,&card.suit) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendDropDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-drop",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].dropoffer )
    {
        SendDropDeny(pnum,"Drop not offered.");
        sprintf(tbuffer1,"Player %s tried to drop without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that they have the card they dropped */
    if ( ! HasCard(pnum,card) )
    {
        SendDropDeny(pnum,"Can't drop nonexistent card.");
        sprintf(tbuffer1,"Player %s tried to drop nonexistent card",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* set trump, change to defendoffer state, tell folks, NextAction() */
    players[pnum].dropoffer=false;
    if ( game.candefend )
        hand.hstate=3;
    else
        hand.hstate=4;
    RemoveCard(pnum,card);
    AddCard(pnum,hand.hole);
    SortCards();

    NextAction();
    SendState();
}


/* This routine processes a client request to call trump */
void ClientCall(int pnum)
{
    int gh,ph,suit;

    debug(GENERAL) fprintf(stderr,"entering ClientCall()\n");

    sprintf(tbuffer1,"Servicing %d (%s): CALL",pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <ORDER> <gh> <ph> <card.suit> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! ReadInt(pnum,&suit) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendCallDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-call",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].calloffer )
    {
        SendCallDeny(pnum,"Call not offered.");
        sprintf(tbuffer1,"Player %s tried to call without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the suit ordered wasn't turned down */
    if ( suit == hand.hole.suit )
    {
        SendCallDeny(pnum,"Can't call declined suit.");
        sprintf(tbuffer1,"Player %s tried to call declined suit",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* set trump, change to defendoffer state, tell folks, NextAction() */
    hand.suit=suit;
    SortCards();
    players[pnum].calloffer=false;
    players[pnum].maker=true;
    hand.maker=pnum;
    if ( game.candefend )
        hand.hstate=3;
    else
        hand.hstate=4;

    sprintf(tbuffer1,"Server: %s called %s.",
        players[pnum].playername,PrintSuit(hand.suit));
    SendChat(tbuffer1);

    NextAction();
    SendState();
}


/* This routine processes a client request to call trump alone */
void ClientCallAlone(int pnum)
{
    int gh,ph,suit;

    debug(GENERAL) fprintf(stderr,"entering ClientCallAlone()\n");

    sprintf(tbuffer1,"Servicing %d (%s): CALLALONE",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <CALLALONE> <gh> <ph> <suit> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! ReadInt(pnum,&suit) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendCallDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-call",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].calloffer )
    {
        SendCallDeny(pnum,"Call not offered.");
        sprintf(tbuffer1,"Player %s tried to call without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* set trump, change to defendoffer state, tell folks, NextAction() */
    hand.suit=suit;
    SortCards();
    players[pnum].calloffer=false;
    players[pnum].alone=true;
    players[pnum].maker=true;
    hand.maker=pnum;
    if ( game.candefend )
        hand.hstate=3;
    else
        hand.hstate=4;

    sprintf(tbuffer1,
        "Server: %s has called %s, and is going alone.",
        players[pnum].playername,PrintSuit(hand.suit));
    SendChat(tbuffer1);

    NextAction();
    SendState();
}


/* This routine processes a client request to pass on calling trump */
void ClientCallPass(int pnum)
{
    int gh,ph;

    debug(GENERAL) fprintf(stderr,"entering ClientCallPass()\n");

    sprintf(tbuffer1,"Servicing %d (%s): CALLPASS",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <CALLPASS> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendCallDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-call",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].calloffer )
    {
        SendCallDeny(pnum,"Call not offered.");
        sprintf(tbuffer1,"Player %s tried to pass on call without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Deal with hand folding or screw slipping */
    if ( players[pnum].dealer )
    {
        if ( game.screw )
        {
            SendCallDeny(pnum,"Screw the dealer in effect: must choose suit.");
            sprintf(tbuffer1,"Dealer %s tried to pass against screw the dealer",
                players[pnum].playername);
            myLog(tbuffer1);
            return;
        }
        else
        {
            sprintf(tbuffer1,"Server: %s has folded the hand.",
                players[pnum].playername);
            SendChat(tbuffer1);
            sprintf(tbuffer1,"%s has folded the hand",
                players[pnum].playername);
            myLog(tbuffer1);
            ClearHand();
            NewHand();
            SendState();
            SendDeal();
            NextAction();
            return;
        }
    }

    /* tell folks, NextAction() */
    sprintf(tbuffer1,"Server: %s has passed.",
        players[pnum].playername);
    SendChat(tbuffer1);

    NextAction();
    SendState();
}


/* This routine processes a client request to defend alone */
void ClientDefend(int pnum)
{
    int gh,ph;

    debug(GENERAL) fprintf(stderr,"entering ClientDefend()\n");

    sprintf(tbuffer1,"Servicing %d (%s): DEFEND",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <DEFEND> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendDefendDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-defend",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].defendoffer )
    {
        SendDefendDeny(pnum,"Defend not offered.");
        sprintf(tbuffer1,"Player %s tried to defend without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* set trump, change to defendoffer state, tell folks, NextAction() */
    hand.hstate=4;
    players[pnum].defendoffer=false;
    players[pnum].defend=true;

    sprintf(tbuffer1,
        "Server: %s is defending alone.",players[pnum].playername);
    SendChat(tbuffer1);

    NextAction();
    SendState();
}


/* This routine processes a client request to pass on ordering up trump */
void ClientDefendPass(int pnum)
{
    int gh,ph;

    debug(GENERAL) fprintf(stderr,"entering ClientDefendPass()\n");

    sprintf(tbuffer1,"Servicing %d (%s): DEFENDPASS",
        pnum,players[pnum].playername);
    myLog(tbuffer1);

    /* <msg> : <DEFENDPASS> <gh> <ph> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! TailCheck(pnum)) return;

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendDefendDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-defend",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].defendoffer )
    {
        SendDefendDeny(pnum,"Defend not offered.");
        sprintf(tbuffer1,"Player %s tried to pass on defending without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* tell folks, NextAction() */
    sprintf(tbuffer1,"Server: %s is not defending alone.",
        players[pnum].playername);
    SendChat(tbuffer1);

    NextAction();
    SendState();
}


/* This routine processes a client request to play a card */
void ClientPlay(int pnum)
{
    int gh,ph;
    Card card;

    debug(GENERAL) fprintf(stderr,"entering ClientPlay()\n");

    /* <msg> : <PLAY> <gh> <ph> <value> <suit> <tail> */
    if ( ! ReadInt(pnum,&gh) ) return;
    if ( ! ReadInt(pnum,&ph) ) return;
    if ( ! ReadInt(pnum,&(card.value)) ) return;
    if ( ! ReadInt(pnum,&(card.suit)) ) return;
    if ( ! TailCheck(pnum)) return;

    sprintf(tbuffer1,"Servicing %d (%s): PLAY %s",
        pnum,players[pnum].playername,CardName(card));
    myLog(tbuffer1);

    /* Check that the player handle makes sense: use pnum if different */
    if (ph != pnum)
    {
        sprintf(tbuffer1,"mismatch player handle: from %d, claims %d",
            pnum,ph);
        myLog(tbuffer1);
    }

    /* Check that the game has already started */
    if ( ! game.ingame )
    {
        SendDefendDeny(pnum,"Game not yet started.");
        sprintf(tbuffer1,"Player %s tried to pre-defend",
            players[pnum].playername);
        myLog(tbuffer1);
        return;
    }

    /* Check that the user has an offer outstanding */
    if ( ! players[pnum].playoffer )
    {
        sprintf(tbuffer1,"Player %s tried to play without offer",
            players[pnum].playername);
        myLog(tbuffer1);
        SendPlayDeny(pnum,"Play not offered.");
        return;
    }

    /* check that the player actually has that card */
    if ( ! HasCard(pnum,card))
    {
        sprintf(tbuffer1,"Player %s tried to play unowned card",
            players[pnum].playername);
        myLog(tbuffer1);
        SendPlayDeny(pnum,"You don't possess that card.");

        // we resend the play offer, to trigger the client to try again
        SendPlayOffer(pnum);
        return;
    }

    /* check that player is playing a valid card */
    if ( ! ValidPlay(pnum,card))
    {
        sprintf(tbuffer1,"Player %s tried to play invalid card",
            players[pnum].playername);
        myLog(tbuffer1);
        SendPlayDeny(pnum,"Must follow suit.");

        // we resend the play offer, to trigger the client to try again
        SendPlayOffer(pnum);
        return;
    }

    /* tell folks, NextAction() */
    sprintf(tbuffer1,"Server: %s plays %s.",
        players[pnum].playername,CardText(card));
    SendChat(tbuffer1);
    players[pnum].cardinplay=true;
    players[pnum].card=card;
    RemoveCard(pnum,card);

    NextAction();
    SendState();
}
