
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* specific euchres includes */
#include "euchres.h"
#include "global.h"


/* program main loop */

/* The euchre server runs as follows:
 *  - start up
 *  - read config file and command switches
 *  - open listen socket
 *  - accept four clients
 *  - play the game
 */
int main(int argc, char **argv)
{
	debug(GENERAL) fprintf(stderr,"entering main()\n");

	/* reads command switches, config, inits data, opens sockets */
	Initialize(argc,argv);

	/* prints some header and title information */
	PrintHeader();

	/* plays the game */	
	PlayGame();

	Exit();
} 


/* Prints the usage string for this program */
void Usage(void)
{
	debug(GENERAL) fprintf(stderr,"entering Usage()\n");

	fprintf(stderr,"\n");
	fprintf(stderr,"Usage: ");
	fprintf(stderr,
	"euchres [-h] [-d {0|1}] [-v <vl>] [-p <port>] [-F <file>] [-L <file>]\n");
	fprintf(stderr,"\t-h        : prints this usage string\n");
	fprintf(stderr,"\t-d {0|1}  : enables (1) or disables (0) debug\n");
	fprintf(stderr,"\t-v <vl>   : sets the verbosity level\n");
	fprintf(stderr,"\t-p <port> : sets the port to <port>\n");     
	fprintf(stderr,"\t-F <file> : sets the config file to <file>\n");     
	fprintf(stderr,"\t-L <file> : sets the log file to <file>\n");     
	fprintf(stderr,"\n");

	Exit();
}



/* A short exit routine, to centralize closeup and shutdown */
void Exit(void)
{
	int pnum;

	debug(GENERAL) fprintf(stderr,"entering Exit()\n");

	/* shut down the client sockets */
	for (pnum=0; pnum<4; pnum++)
		if (players[pnum].socket != undefined)
		{	sprintf(tbuffer1,"Notifying client %d (%s)",
				pnum,players[pnum].playername);
			myLog(tbuffer1);
			SendQuit(pnum,"Sorry, server is exiting");
			close(players[pnum].socket);
		}

	/* shut down the server socket */
	myLog("Stopping euchre server...");
	close(s);

	myLog("Exiting...");
	if ( logF != NULL) fclose(logF);

	exit(0);
}
