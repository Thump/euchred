
/*
 * Euchre Server
 *
 * (C) Copyright 1999 Denis McLaughlin
 *
 */


/* some generic includes */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

/* specific euchres includes */
#include "euchres.h"


/* This routine opens the log file, setting logF to the file is the open
 * succeeds.  If the open does not succeed, logF is set to stderr.
 * Either way, calling the myLog() routine will validly output data.
 * 
 * Originally logF was going to be set to stderr if the open failed:
 * this complicated the conditional logging a bit.  So the only
 * valid way to use logF is to let myLog() do it for you.  If you have
 * a message that needs to see the light of day, fprintf() it to
 * stderr.
 */
void OpenLog(void)
{
	debug(GENERAL) fprintf(stderr,"entering OpenLog()\n");

	/* attempt to open (char *)log */
	logF = fopen(logfile,"w");
	
	if ( logF == NULL)
	{
		sprintf(tbuffer1,"Error opening log file %s: %s",
			logfile,sys_errlist[errno]);
		myLog(tbuffer1);
	}
	else
	{
		sprintf(tbuffer1,"Log file %s opened",logfile);
		myLog(tbuffer1);
	}
}


/* This routine logs messages: if the log file was not successfully opened,
 * the message is sent to stderr.  If the log file was successfully opened,
 * and the debug flag is not set, messages are sent to the log file.  If
 * the log was successfully opened, and the debug flag is set, messages
 * sent to both the log file and stderr.
 * 
 * It is expected that the message text passed to myLog() will not have a
 * new line appended, to allow myLog() to have flexibility in post-processing
 * the results.
 */
void myLog(char *msgtext)
{
	time_t t;
	struct tm *now;
	char timetext[1024];
	
	debug(GENERAL) fprintf(stderr,"entering myLog()\n");

	t=time(NULL);
	now=localtime(&t);
	sprintf(timetext,"%d-%02d-%02d %02d:%02d:%02d",now->tm_year+1900,
		now->tm_mon+1,now->tm_mday, now->tm_hour,now->tm_min,now->tm_sec);

	/* not valid log file : message to stderr */
	if ( logF==NULL)
		fprintf(stderr,"%s : %s\n",timetext,msgtext);
	/* valid log file, not debug : message to the log file */
	else if ( logF!=NULL && df==0 )
		fprintf(logF,"%s : %s\n",timetext,msgtext);
	/* valid log file, debug : message to stderr and log file */
	else if ( logF!=NULL && df==1)
	{
		fprintf(logF,"%s : %s\n",timetext,msgtext);
		fprintf(stderr,"%s : %s\n",timetext,msgtext);
		fflush(logF);
	}
}
