
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
#include <signal.h>

/* specific euchres includes */
#include "euchres.h"


/* This routine sets the signal handlers */
void SetSignal(void)
{
	debug(GENERAL) fprintf(stderr,"entering SetSignal()\n");

	signal(SIGHUP,(void *)TrapHUP);
	signal(SIGINT,(void *)TrapINT);
	signal(SIGPIPE,(void *)SIG_IGN);
}


/* Catches the SIGHUP signal */
void TrapHUP(void)
{
	debug(GENERAL) fprintf(stderr,"entering TrapHUP()\n");

	myLog("Whoops, caught SIGHUP, exiting...");
	Exit();
}


/* Catches the SIGINT signal */
void TrapINT(void)
{
	debug(GENERAL) fprintf(stderr,"entering TrapINT()\n");

	myLog("Whoops, caught SIGINT, exiting...");
	Exit();
}
