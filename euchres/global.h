
/*
 * (C) Copyright 1999 Denis McLaughlin
 *
 * Include file for global variables, used to set default run time
 * values: all of these can be overridden in either the config file
 * or the command line
 */

/* This is the place to define all global variables: this file is only
 * included by euchre.c.  For any variable added to this file, a corresponding
 * extern reference should exist in euchres.h
 */


/* header string */
char *header="Euchre Server\nversion "VERSION" \ncompiled "__DATE__" at "__TIME__" \n \n(C) Copyright 1999 Denis McLaughlin \nmclaughlin.denis@gmail.com\n";

/* base directory name, only really used for informational purposes */
char *base=BASE;

/* config file name and file pointer */
char *config=CONFIG;
FILE *configF;

/* log file name and file pointer */
char *logfile=LOGFILE;
FILE *logF=NULL;

/* default listen port */
int port=PORT;

/* default debug flag */
int df=DF;

/* default verbosity: 0 */
int vl=VL;

/* general text scratch space */
char tbuffer1[SSIZE];
char tbuffer2[SSIZE];
char tbuffer3[SSIZE];

/* general data scratch space */
char dbuffer[SSIZE];

/* the game, player records, and the hand */
Player players[4];
Game game;
Card *deck=NULL;
Hand hand;

/* the listening socket */
int s;

/* this is how long we will wait for a client response generally */
int maxmsgwait=60;

/* how long to remain in select() before coming up for air, in seconds 
 * increasing this number reduces the CPU load, but decreases the accuracy
 * of joinwait and msgwait
 */
int pollinterval=5;

/* The protocol version we understand: we actually speak all versions equal
 * to or less than this.
 */
int protocol=1;

/* maximum length of message we will accept from a client */
int maxmsglen=10240;
