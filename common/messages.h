
/*
 * (C) Copyright 1999 Denis McLaughlin
 */


/* An event is really just a int, an index into the list of events. 
 * Each event corresponds roughly to a message sent from the client
 * to the server.
 */

typedef int event;

/* The actual message IDs.  At some point, these should be changed to
 * large, unusual integers, to assist in protocol debugging.  Later.
 */

/***************************************************************************/

/* The JOIN messages, sent by the client after connection, as well as
 * the server's replies.
 */
#define JOIN 123401
#define JOINDENY 123402
#define JOINACCEPT 123403

/* sent by the server to connected clients when the server is quitting */
#define SERVERQUIT 123404

/* sent by the client to the server when the client is quitting */
#define CLIENTQUIT 123405

/* sent if the server is full when the client tries to connect */
#define DECLINE 123406

/* sent by the server when the client is about to be terminated */
#define KICK 123407

/* the ID messages, request from client, responses from server */
#define ID 123408
#define IDACCEPT 123409
#define IDDENY 123410

/* sent by the client when sending in a chat message, sent by the server
 * when broadcasting the chat message
 */
#define CHAT 123411

/* sent by server to clients after game state change: provides all info
 * needed by client to enter or resume game
 */
#define STATE 123412

/* sent as a request when the creator wants to kick another player */
#define KICKPLAYER 123413
#define KICKDENY 123414

/* sent by a client setting options */
#define OPTIONS 123415
#define OPTIONSDENY 123416

/* sent by the creator to start the game */
#define START 123417
#define STARTDENY 123418

/* sent by the creator to end or reset the game, sent by the server to tell
 * the clients the game is ending */
#define END 123419
#define ENDDENY 123420

/* sent by client as responses to an order offer */
#define ORDER 123421
#define ORDERALONE 123422
#define ORDERPASS 123423
#define ORDERDENY 123424

/* sent by client to indicate dropped card, and the deny message */
#define DROP 123425
#define DROPDENY 123426

/* sent by client as responses to a call offer */
#define CALL 123427
#define CALLALONE 123428
#define CALLPASS 123429
#define CALLDENY 123430

/* sent by client as responses to a defend offer */
#define DEFEND 123431
#define DEFENDPASS 123432
#define DEFENDDENY 123433

/* sent by client as responses to a play offer */
#define PLAY 123434
#define PLAYDENY 123435

/* flag messages sent by server */
#define TRICKOVER 123436
#define HANDOVER 123437
#define GAMEOVER 123438
#define PLAYOFFER 123439
#define DEFENDOFFER 123440
#define CALLOFFER 123441
#define ORDEROFFER 123442
#define DROPOFFER 123443
#define DEAL 123444

/***************************************************************************/
