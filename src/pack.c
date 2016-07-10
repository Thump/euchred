
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 */

/* Data packing routines. */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netinet/in.h>


/* specific euchred includes */
#include "euchred.h"


/* This routine takes a pointer to a buffer and an integer, converts
 * the integer to network byte order, copies it into the buffer, and
 * returns the size.
 */
int PackInt(char *buffer, int data)
{
    int tmp;

    debug(GENERAL) fprintf(stderr,"entering PackInt()\n");

    tmp=htonl(data);
    memcpy(buffer,&tmp,4);
    return(4);
}


/* This routine takes a pointer to a buffer and a boolean value (0, 1 or
 * -1 for false, true and undefined) and packs it into buffer in network
 * byte order, returning the size.
 */
int PackBoolean(char *buffer, boolean data)
{
    int tmp;

    debug(GENERAL) fprintf(stderr,"entering PackBoolean()\n");

    tmp=htonl(data);
    memcpy(buffer,&tmp,4);
    return(4);
}


/* This routine takes a pointer to a buffer and a short, converts
 * the short to network byte order, copies it into the buffer, and
 * returns the size.
 */
int PackShort(char *buffer, short data)
{
    short tmp;

    debug(GENERAL) fprintf(stderr,"entering PackShort()\n");

    tmp=htons(data);
    memcpy(buffer,&tmp,2);
    return(2);
}


/* This routine converts a supplied integer from network byte order,
 * and returns the value.
 */
int UnpackInt(int data)
{
    int tmp;

    debug(GENERAL) fprintf(stderr,"entering UnpackInt()\n");

    tmp=ntohl(data);
    return(tmp);
}


/* This routine converts a supplied short from network byte order,
 * and returns the value.
 */
short UnpackShort(short data)
{
    int tmp;

    debug(GENERAL) fprintf(stderr,"entering UnpackShort()\n");

    tmp=ntohs(data);
    return(tmp);
}


/* This routine converts a supplied boolean from network byte order,
 * and returns the value.
 */
boolean UnpackBoolean(boolean data)
{
    boolean tmp;

    debug(GENERAL) fprintf(stderr,"entering UnpackBoolean()\n");

    tmp=ntohl(data);
    return(tmp);
}


/* This routine takes a pointer to a buffer and a string.  It packs
 * the buffer with an integer representing the length of the string,
 * followed by the string.  It returns the length of the string plus
 * the integer representing the length of the string.  buffer is
 * assumed to be large enough to hold data.
 */
int PackString(char *buffer, char *data)
{
    int len=0,size=0;

    debug(GENERAL) fprintf(stderr,"entering PackString()\n");

    /* if the string is null, we leave len with its preinit value of 0 */
    if (data != NULL)
        len=strlen(data);
    size=PackInt(buffer,len);
    memcpy(buffer+size,data,len);
    return(len+size);
}


/* This routine takes a pointer to a string and returns the string in newly
 * malloc buffer.
 *
 * Technically, yes, this doesn't do anything useful.  This is intended
 * to provide forward support for international multi-byte character
 * strings, which would require network-to-host byte order conversion.
 */
char *UnpackString(char *data)
{
    char *tmp;

    debug(GENERAL) fprintf(stderr,"entering PackString()\n");

    tmp=malloc(strlen(data)*sizeof(char));
    lomem(tmp);
    strcpy(tmp,data);
    return(tmp);
}


/* This routine takes a buffer and player number and returns the buffer
 * with the generic player data packed into, suitable for sending out
 * with a STATE message.  It packs the buffer in place, and returns the
 * number of bytes packed.
 */
int PackPlayer(char *buffer, int pnum)
{
    int size=0;

    debug(GENERAL) fprintf(stderr,"entering PackPlayer()\n");

    /* <pN> : <pstate> <pdata>
     *   <pstate> : {0|1|2} # unconnected, connected, joined
     *   <pdata> : if <pstate> == joined
     *               <ph> <nmstring> <clstring> <hwstring> <osstring>
     *               <cmtstring> <team> <numcards> <creator> <ordered>
     *               <madeit> <alone> <defend> <offer> <card> <passed>
     *      <team> : {-1|0|1} # no team, team 0, or team 1
     *      <creator>|<ordered>|<dealer>|<alone>|<defend>|<lead>|<maker>|
     *      <playoffer>|<orderoffer>|<dropoffer>|<calloffer>|<defendoffer>|
     *      <passed> : <boolean>
     *      <card> : card in play
     */

    /* pstate */
    if (players[pnum].state == unconnected)
        size+=PackInt(buffer+size,0);
    else if (players[pnum].state == connected)
        size+=PackInt(buffer+size,1);
    else if (players[pnum].state == joined)
        size+=PackInt(buffer+size,2);

    /* if the client hasn't joined, we're done */
    if (players[pnum].state != joined)
        return(size);

    /* otherwise pack the rest of this shit */
    size+=PackInt(buffer+size,players[pnum].ph);
    size+=PackString(buffer+size,players[pnum].playername);
    size+=PackString(buffer+size,players[pnum].clientname);
    size+=PackString(buffer+size,players[pnum].hardware);
    size+=PackString(buffer+size,players[pnum].OS);
    size+=PackString(buffer+size,players[pnum].comment);
    size+=PackInt(buffer+size,players[pnum].team);
    size+=PackInt(buffer+size,players[pnum].numcards);
    size+=PackBoolean(buffer+size,players[pnum].creator);
    size+=PackBoolean(buffer+size,players[pnum].ordered);
    size+=PackBoolean(buffer+size,players[pnum].dealer);
    size+=PackBoolean(buffer+size,players[pnum].alone);
    size+=PackBoolean(buffer+size,players[pnum].defend);
    size+=PackBoolean(buffer+size,players[pnum].leader);
    size+=PackBoolean(buffer+size,players[pnum].maker);

    /* and the offers */
    size+=PackBoolean(buffer+size,players[pnum].playoffer);
    size+=PackBoolean(buffer+size,players[pnum].orderoffer);
    size+=PackBoolean(buffer+size,players[pnum].dropoffer);
    size+=PackBoolean(buffer+size,players[pnum].calloffer);
    size+=PackBoolean(buffer+size,players[pnum].defendoffer);

    /* and current card in play */
    size+=PackBoolean(buffer+size,players[pnum].cardinplay);
    if ( players[pnum].cardinplay )
        size+=PackCard(buffer+size,players[pnum].card);
    size+=PackBoolean(buffer+size,players[pnum].passed);

    return(size);
}


/* This routine packs the cards for a specific players.  We take the
 * player whose cards we are packing and a buffer to put them in,
 * and return the size (in bytes) of the result.  buffer is assumed
 * to be large enough to hold the result.
 */
int PackCards(char *buffer, int pnum)
{
    int size=0,i;

    debug(GENERAL) fprintf(stderr,"entering PackCards()\n");

    /* <cards> : <numcards> <card1> .. <cardN>
     *   <cardN> : <value> <suit>
     *     <value> : {2|3|4|5|6|7|8|9|10|11|12|13|14}
     *     <suit> : {0|1|2|3}
     */

    size+=PackInt(buffer+size,players[pnum].numcards);

    for (i=0; i<players[pnum].numcards; i++)
        size+=PackCard(buffer+size,players[pnum].cards[i]);

    return(size);
}


/* This routine takes a buffer and a Card structure, and packs that
 * card into buffer in network independent byte order.  buffer is assumed
 * to be large enough to hold the result.  The number of bytes packed
 * is returned.
 */
int PackCard(char *buffer, Card card)
{
    int size=0;

    debug(GENERAL) fprintf(stderr,"entering PackCard()\n");

    /*   <card> : <value> <suit>
     *     <value> : {2|3|4|5|6|7|8|9|10|11|12|13|14}
     *     <suit> : {0|1|2|3}
     */

    size+=PackInt(buffer+size,card.value);
    size+=PackInt(buffer+size,card.suit);

    return(size);
}
