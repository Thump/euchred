
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <errno.h>
#include <time.h>

/* specific euchres includes */
#include "euchres.h"


/* This is a wrapper function: it calls ReadSwitch() and ReadConfig(),
 * which configures the game, and then Initialize(), which initializes
 * the data structures.  Then it calls the socket routine, to open up
 * the listening socket.
 */
void Initialize(int argc, char **argv)
{
	debug(GENERAL) fprintf(stderr,"entering Initialize()\n");

	/* We read the switches first, then read the config file, then we
	 * re-read the switches.  This lets us use -F to reset the config file,
	 * but still allows switches to over-ride the config file.  A bit
	 * inelegant, but better than messy option locking.
	 */
	ReadSwitch(argc,argv);
	ReadConfig();
	ReadSwitch(argc,argv);
	
	/* Open the log file */
	OpenLog();

	log("Starting euchre server");

	/* initialize game and player structures */
	InitData();

	/* create listening socket */
	OpenSocket();

	/* set signals */
	SetSignal();

	/* initialize our random number generator */
	srand(time(NULL));
}


/* this routine steps through the commandline arguments we were passed
 * setting variables as appropriate.
 */
void ReadSwitch(int argc, char **argv)
{
	int opt;

	debug(GENERAL) fprintf(stderr,"entering ReadSwitch()\n");

	/* We step through the command line switches */
	while ((opt=getopt(argc,argv,"h?d:v:p:F:L:")) != EOF)
	{
		if (opt == 'F')
		{
			config=strdup(optarg);
			lomem(config);
		}
		
		if (opt == 'd')
			df=atoi(optarg);
		
		if (opt == 'v')
			vl=atoi(optarg);

		if (opt == 'p')
			port=atoi(optarg);
		
		if (opt == 'L')
			lomem(logfile=strdup(optarg));

		if (opt == '?')
			Usage();

		if (opt == 'h')
			Usage();
	}
	/* reset optind to 1, so we can reprocess the switches later */
	optind=1;
}


/* This routine reads the configuration file (defined on the command
 * line specified at compile time) and sets appropriate values.
 */
void ReadConfig(void)
{
	int result1=0,result2=0;
	int linecount=0;

	debug(GENERAL) fprintf(stderr,"entering ReadConfig()\n");

	/* attempt to open config file, error and return if we can't */
	configF=fopen(config,"r");
	if (configF==NULL)
	{
		sprintf(tbuffer1,"Error opening config %s: %s",
			config,sys_errlist[errno]);
		log(tbuffer1);
		return;
	}


	/* This while loop steps through the lines in the config file,
	 * parsing those it can figure out, ignoring anything containing
	 * only blanks, or text preceded by a #, and declaring (and then
	 * ignoring) crap.
	 * 
	 * Because of our line limit on fgets, we can only read lines of SSIZE
	 * or less.  At time of writing, set to 10240, it shouldn't be too
	 * much of an issue.
	 */
	while (fgets(tbuffer1,SSIZE,configF) != NULL)
	{
		/* increment a counter for our lines */
		linecount++;


		/* if its of valid form, try and read it, ignore unknown names */
		result1=sscanf(tbuffer1," %[^#= \t\n] = %[^#\n]",tbuffer2,tbuffer3);
		if (result1 == 2)
		{
			if (!strcmp("logfile",tbuffer2))
				lomem(logfile=strdup(tbuffer3));
			
			if (!strcmp("port",tbuffer2))
				port=atoi(tbuffer3);
			
			if (!strcmp("debug",tbuffer2))
				df=atoi(tbuffer3);
			
			if (!strcmp("verbosity",tbuffer2))
				vl=atoi(tbuffer3);
			
			if (!strcmp("maxmsgwait",tbuffer2))
				maxmsgwait=atoi(tbuffer3);

			continue;
		}


		/* see if it matches an ignorable line */
		result1=sscanf(tbuffer1," %[#]",tbuffer2);
		result2=sscanf(tbuffer1," %[\n\r]",tbuffer2);
		if (result1==1 || result2<0)
			continue;

		
		sprintf(tbuffer1,"unknown syntax error in %s at line %d: ignored"
			,config,linecount);
		log(tbuffer1);
	}

	/* close our config file */
	fclose(configF);
}


/* This routine initializes the game data structure. */
void InitData(void)
{	int pnum;

	debug(GENERAL) fprintf(stderr,"entering InitData()\n");

	ClearGame();
	for (pnum=0; pnum<4; pnum++)
		ClearPlayer(pnum);
	ClearHand();
}
