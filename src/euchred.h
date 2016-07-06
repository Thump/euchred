
/*
 * (C) Copyright 1999 Denis McLaughlin
 *
 * General include file for the euchre server, specifies a number of
 * extern'd global environment variables, some game data structures,
 * function prototypes.
 */


/****************************************************************************/
/* Global macros and defines */

/* this line implements the groovy debugging routines: see debug.h for the
 * valid values of a
 */
#define debug(a) if (df && (vl&a))

/* this is an easy post-malloc check for out of memory conditions: typical
 * usage would be: foo=(char *)malloc(1024); lomem(foo);
 */
#define lomem(a) if ((a)==NULL) {printf("Eek!  Out of memory at line %d in %s : aborting\n",__LINE__,__FILE__); exit(255);}

/* used for scratch space sizing */
#define SSIZE 10240

/* true, false, and undefined */
#define true (1)
#define false (0)
#define undefined (-1)

/* dorky little max and min routines */
/* #define max(a,b) (a>b ? b : a) */
#define max(a,b) (a>b ? a : b)
#define min(a,b) (a>b ? a : b)

/* The tail */
#define TAIL (short)64222

/* default user name */
#define DEFNAME "<no name>"

/****************************************************************************/
/* Global variables, expected to be accessed and modified from everywhere */

/* base directory, only really used for information purposes */
extern char *base;

/* a short header to print for help screens 'n such */
extern char *header;

/* config file name and file pointer */
extern char *config;
extern FILE *configF;

/* log file name and file pointer */
extern char *logfile;
extern FILE *logF;

/* debug flag */
extern int df;

/* verbosity level  */
extern int vl;

/* port to listen for incoming clients on */
extern int port;

/* listen socket, and c1, c2, c3, c4 sockets */
extern int s;

/* general scratch space */
extern char tbuffer1[];
extern char tbuffer2[];
extern char tbuffer3[];
extern char  dbuffer[];

/* delay before kicking a client who hasn't responded to a message */
extern int maxmsgwait;

/* delay before popping out of select() to do admin stuff */
extern int pollinterval;

/* the protocol version we speak: actually, we speak all protocol versions
 * equal to or less than this
 */
extern int protocol;

/* maximum message length we will accept from a client */
extern int maxmsglen;


/**************************************************************************/
/* global data structures */

typedef int boolean;


/* In the game structure, the arrayed variables are intended to be 
 * indexed by team number: taking the corresponding team number from the
 * the player structure will allow the appropriate value to be retrieved.
 *
 * ingame is initialized to false, and only trued when 4 players have
 * joined, either bot or human.  ingame will only move from true to false
 * when the game is actually complete.  suspend is initialized to false: it
 * cannot be trued until after the game has started (ingame trued),
 * thus ingame=false implies suspend=false.  suspend is trued when
 * the game has started and the number of valid players falls below 4.
 * It offers a mechanism to allow players to leave and join running
 * games.
 *
 * For this single-game server, there is only one global game structure.
 * 
 * If you add an element to the game structure, ensure you add a corresponding
 * line to the PrintGame() routine, and the InitData() routine.
 */

typedef struct
{
	boolean ingame;           /* true when a game has started */
	int gh;                   /* the game handle */
	boolean suspend;          /* true when a game gas been suspended */
	int players;              /* the number of players connected */
 	int bots;                 /* the number of bots connected */
	int score[2];             /* score array */
	int tricks;               /* total number of tricks played in the game */
	int numholecalled[2];     /* number of times hole card ordered by team */
	int numtrumpcalled[2];    /* number of times trump called by team */
	int numdealerscrewed[2];  /* number of times dealer was screwed by team */
 	int numeuchres[2];        /* number of euchres by team */
	int numdefends[2];        /* number of defends by team */
	boolean candefend;        /* whether defend-alones are allowed */
	boolean aloneonorder;     /* whether a partner who orders goes alone */
	boolean screw;            /* whether screw the dealer is effect */
} Game;
extern Game game;


/* This is used to track the state of the player structure: it is unconnected
 * before anyone connects, connected once a client has connected but before
 * she has joined, and joined once the JOIN message is sent.  If a player
 * drops out, the state is returned to disconnected. 
 */

typedef enum {unconnected,connected,joined} pstate;


/* This is the card data structure.  The suit and value are just integers:
 * value is numbered 2 through 14, with 11, 12, 13, and 14 being jack,
 * queen, king and ace, respectively.  The suits are defined in
 * ../common/suits.h: in brief, they are numbered from 0 to 3 in alphabetic
 * order.
 */

typedef struct
{
	int suit;
	int value;
} Card;
extern Card *deck;


/* This is the player structure, instantiated four times in the players[]
 * array.  The team element can be used to index the game structure arrays.
 *
 * The cards array is pointer to an array of numcards cards.  No point
 * in messing around with a linked list.  This list contains the user's
 * current cards: so long as the cards were dealt fairly, and never changed
 * unless the user plays a card, cheating will not be possible.
 * 
 * If you add an element to the player structure, ensure you add a corresponding
 * line to the PrintPlayer() routine, and the InitData() routine.
 *
 */

typedef struct
{
	boolean joinwait;         /* true if server waiting for client to join */
	boolean msgwait;          /* true if server waiting for client msg */
	int msglen;               /* anticipated length of incoming message */
	int msgtime;              /* time server started waiting for message */
	pstate state;             /* the player state, see pstate comment */
	int socket;               /* the client socket */
	int ph;                   /* the player handle */
	Card *cards;              /* a pointer to the cards array */
	int numcards;             /* number of cards in array */
	int team;                 /* team number: 0 or 1, -1 if undefined */
	boolean creator;          /* true if player has god status */
	char *playername;         /* pointer to text string of player's name */
	char *hardware;           /* pointer to text string of player's platform */
	char *OS;                 /* pointer to text string of player's OS */
	char *clientname;         /* pointer to text string of player's client */
	char *comment;            /* pointer to text string of player comment */
	char *ip;                 /* pointer to text string of player's IP */
	int holecalled;           /* number of times player ordered hole */
	int holemade;             /* number of times successful */
	int trumpcalled;          /* number of times player called trump */
	int trumpsmade;           /* number of times successful */
	int handsalonecalled;     /* number of times gone alone */
	int handsalonemade;       /* number of times successful */
	int defendalonescalled;   /* number of defend alones */
	int defendalonesmade;     /* number of times successful */
	int tricksmade;           /* number of tricks made */
	int antitricksmade;       /* number of anti-tricks made */
	int euchres;              /* number of times got euchered */
	int screwed;              /* number of times got screwed */
	int nummsgs;              /* number of messages sent */
	int volmsgs;              /* number of bytes of messages */
	boolean playoffer;        /* flag to track play offer */
	boolean orderoffer;       /* flag to track order offer */
	boolean dropoffer;        /* flag to track drop offer */
	boolean calloffer;        /* flag to track call offer */
	boolean defendoffer;      /* flag to track defend offer */
    boolean dealer;           /* true if player was the dealer for this hand */
    boolean ordered;          /* true if player ordered for this hand */
    boolean alone;            /* true if player went alone for this hand */
    boolean defend;           /* true if player defended alone */
    boolean leader;           /* true if player lead this hand */
    boolean maker;            /* true if player made this hand */
    boolean cardinplay;       /* true if player has a card in play */ 
	Card card;                /* current card in play, if any */
    boolean passed;           /* true if player has passed on hole or call */
} Player;
extern Player players[4];


/* This is the hand structure: tracks the status of each hand */
typedef struct
{
	Card hole;      /* set to current hole card, if any */
	int suit;       /* current called suit */
	int hstate;     /* state: -1: pregame 0: hole 1: call 2: defend 3: play */
	int dealer;     /* set to the team number that dealt */
	int maker;      /* set to the player who made it */
	int tricks[2];  /* tricks array: tricks won by one team or the other */
} Hand;
extern Hand hand;


/****************************************************************************/
/* Some high level includes: nicer to include them at the top, but then
 * some of the structures are being used (in proto.h) before they're
 * defined.
 */
 
#include "debug.h"
#include "proto.h"
#include "../common/messages.h"
#include "../common/suits.h"
