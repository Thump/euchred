
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 */

/* Socket manipulating routines */


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
#include <sys/ioctl.h>


/* specific euchres includes */
#include "euchres.h"


/* This routine prepares, opens, binds and listens on the server socket */
void OpenSocket(void)
{
	struct sockaddr_in saddr;
	int optval;

    debug(GENERAL) fprintf(stderr,"entering OpenSocket()\n");

	memset(&saddr,0,sizeof(saddr));
	saddr.sin_family=AF_INET;
	saddr.sin_addr.s_addr=htonl(INADDR_ANY);
	saddr.sin_port=htons(port);

	if ( (s=socket(AF_INET,SOCK_STREAM,0)) == -1)
	{
		sprintf(tbuffer1,"Error creating socket: %s",sys_errlist[errno]);
		log(tbuffer1);
		Exit();
	}

	optval=1;
	if ( setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(int *)&optval,sizeof(optval))<0)
	{
		sprintf(tbuffer1,"Error setting SO_REUSEADDR: %s",sys_errlist[errno]);
		log(tbuffer1);
		Exit();
	}

	if (bind(s,&saddr,sizeof(saddr)) != 0)
	{
		sprintf(tbuffer1,"Error binding socket: %s",sys_errlist[errno]);
		log(tbuffer1);
		Exit();
	}

	listen(s,5);
}


/* This routine does a select against the listener socket as well as
 * all current client sockets.  The set of all ready to read sockets
 * is returned.
 */
fd_set DoSelect(void)
{
	fd_set rfd;
	int i,maxfd;
	struct timeval t;

	debug(GENERAL) fprintf(stderr,"entering DoSelect()\n");

	/* this sets the filedescriptor set for the listen socket and
	 * all existing clients */
	FD_ZERO(&rfd);
	FD_SET(s,&rfd);
	maxfd=s;
	for (i=0; i<4; i++)
		if (players[i].state != unconnected)
		{	FD_SET(players[i].socket,&rfd);
			maxfd=max(players[i].socket,maxfd);
		}
	maxfd++;

	/* set out timeout value */
	t.tv_sec=pollinterval;
	t.tv_usec=0;

	i=select(maxfd,&rfd,NULL,NULL,&t);

	return(rfd);
}


/* This routine takes a pointer to an integer: the contents of that integer
 * are modified to hold a socket accepted from the server listen socket,
 * and a pointer to the text of the socket's IP address is returned. 
 */
char *AcceptSocket(int *client)
{
	int size;
	struct sockaddr_in saddr;
	char *ip;

	debug(GENERAL) fprintf(stderr,"entering AcceptSocket()\n");

    size=sizeof(saddr);
    *client=accept(s,&saddr,&size);

	ip=strdup(inet_ntoa(saddr.sin_addr));
	lomem(ip);

	return(ip);
}


/* maps a socket number to a client: -1 means no known client */
int Socket2Client(int sock)
{
	int i;

	debug(GENERAL) fprintf(stderr,"entering Socket2Client()\n");

	for (i=0; i<game.players; i++)
		if (players[i].socket == sock)
			return(i);

	return(-1);
}


/* This takes a pointer to a socket, a pointer to a buffer, and the size
 * of the buffer, and sends the contents of the buffer to the socket.
 */
void SendMsg(int sock, char *buffer, int size)
{
	int written=0;

	debug(GENERAL) fprintf(stderr,"entering SendMsg()\n");

	written=write(sock,buffer,size);

	/* Theoretically we should be catching EPIPE here, to avoid problems
	 * writing to a disconnected client.  We don't care, since we catch
	 * those with the select() loop.
	 */

	debug(SEND) fprintf(stderr,"sent %d of %d bytes to client %d\n",
		written,size,Socket2Client(sock));
}


/* This routine takes care of reading data from a socket: it takes a socket
 * number, a pointer to a buffer, and a size, and reads size bytes of data
 * from the socket into buffer.  Buffer is assumed to have sufficient space
 * to store the data.   The routine returns the return code of read(),
 * that is, the number of bytes read.
 */
int ReadMsg(int sock, char *buffer, int size)
{
	debug(GENERAL) fprintf(stderr,"entering ReadMsg()\n");

	return(read(sock,buffer,size));
}


/* This routine takes a socket and a pointer to a string: if there is
 * sufficient data pending on the socket to satisfy the string, the
 * data is read and stored in string and the routine returns true.
 * Otherwise the routine returns false without modifying string.
 */
char *ReadString(int pnum)
{
	char *tmp;
	int len;

	debug(GENERAL) fprintf(stderr,"entering ReadString()\n");

	if ( ! ReadInt(pnum,&len) )
	{	sprintf(tbuffer1,"Short string header from %d (%s)",
			pnum,players[pnum].playername);
		log(tbuffer1);
		KickClient(pnum,"Short message");
		return(NULL);
	}

	if (len > maxmsglen)
	{	sprintf(tbuffer1,"Oversized string (%d) from %d (%s)",
			len,pnum,players[pnum].playername);
		log(tbuffer1);
		KickClient(pnum,"Oversized message");
		return(NULL);
	}

	if ( ! BytesReady(players[pnum].socket,len) )
	{	sprintf(tbuffer1,"Short string from %d (%s)",
			pnum,players[pnum].playername);
		log(tbuffer1);
		KickClient(pnum,"Short message");
		return(NULL);
	}

	tmp=(char *)malloc(len+1);
	lomem(tmp);
	memset(tmp,0,len+1);

	ReadMsg(players[pnum].socket,(char *)tmp,len);

	return(tmp);
}


/* This routine takes a socket and a pointer to an integer: if there is
 * sufficient data pending on the socket to satisfy the integer, the
 * data is read and stored in data and the routine returns true.
 * Otherwise the routine returns false without modifying data.
 */
boolean ReadInt(int pnum, int *data)
{
	int len;

	debug(GENERAL) fprintf(stderr,"entering ReadInt()\n");

	if ( ! IntReady(players[pnum].socket) )
	{	sprintf(tbuffer1,"Short integer from %d (%s)",
			pnum,players[pnum].playername);
		log(tbuffer1);
		KickClient(pnum,"Short message");
		return false;
	}

	ReadMsg(players[pnum].socket,(char *)&len,4);
	len=UnpackInt(len);

	*data=len;
	return(true);
}


/* This routine takes a socket and a pointer to a short: if there is
 * sufficient data pending on the socket to satisfy the short, the
 * data is read and stored in data and the routine returns true.
 * Otherwise the routine returns false without modifying data.
 */
boolean ReadShort(int pnum, short *data)
{
	short len;

	debug(GENERAL) fprintf(stderr,"entering ReadShort()\n");

	if ( ! ShortReady(players[pnum].socket) )
	{	sprintf(tbuffer1,"Missing tail from %d (%s)",
			pnum,players[pnum].playername);
		log(tbuffer1);
		KickClient(pnum,"Missing tail");
		return false;
	}

	ReadMsg(players[pnum].socket,(char *)&len,2);
	len=UnpackShort(len);

	*data=len;
	return(true);
}


/* This routine takes a socket and a pointer to a boolean: if there is
 * sufficient data pending on the socket to satisfy the boolean, the
 * data is read and stored in data and the routine returns true.
 * Otherwise the routine returns false without modifying data.
 */
boolean ReadBoolean(int pnum, boolean *data)
{
	boolean len;

	debug(GENERAL) fprintf(stderr,"entering ReadBoolean()\n");

	if ( ! BooleanReady(players[pnum].socket) )
	{	sprintf(tbuffer1,"Short boolean from %d (%s)",
			pnum,players[pnum].playername);
		log(tbuffer1);
		KickClient(pnum,"Short message");
		return false;
	}

	ReadMsg(players[pnum].socket,(char *)&len,4);
	len=UnpackBoolean(len);

	*data=len;
	return(true);
}


/* This routine checks whether there is enough data pending on socket sock
 * to make up a 4 byte integer: if there is, it returns true, otherwise
 * false
 */
boolean IntReady(int sock)
{
	debug(GENERAL) fprintf(stderr,"entering IntReady()\n");

	return(BytesReady(sock,4));
}


/* This routine checks whether there is enough data pending on socket sock
 * to make up a 2 byte short: if there is, it returns true, otherwise
 * false
 */
boolean ShortReady(int sock)
{
	debug(GENERAL) fprintf(stderr,"entering ShortReady()\n");

	return(BytesReady(sock,2));
}


/* This routine checks whether there is enough data pending on socket sock
 * to make up a 4 byte boolean: if there is, it returns true, otherwise
 * false
 */
boolean BooleanReady(int sock)
{
	debug(GENERAL) fprintf(stderr,"entering BooleanReady()\n");

	return(BytesReady(sock,4));
}


/* This routine takes a socket and an integer n, and checks whether there
 * checks whether there are n bytes of data pending on the socket.  True
 * if there is, false otherwise.
 */
boolean BytesReady(int sock, int n)
{
	int available;

	debug(GENERAL) fprintf(stderr,"entering BytesReady()\n");

	ioctl(sock,FIONREAD,&available);

	if (available > n-1)
		return true;
	else
		return false;
}
