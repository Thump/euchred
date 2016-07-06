############################################################################
#
# Euchre server makefile
# Denis McLaughlin
# February 24, 1999
#
###########################################################################

# runtime default configuration values

# the base directory for all config files
BASE=.

# default configuration file
CONFIG=$(BASE)/euchres.ini

# default log file
LOGFILE=$(BASE)/euchres.log

# default listen port
PORT=1234

# default debug flag (0=off, 1=on)
DF=0

# default verbosity level: requires DF=1 (see debug.h for values)
VL=0

# version number
VERSION=0.9


###########################################################################

# general compile flags 'n stuff

# The compiler to use
CC=gcc

# these are for general use
CFLAGS= -Wall -O2 -DBASE=\"$(BASE)\" -DCONFIG=\"$(CONFIG)\" -DLOGFILE=\"$(LOGFILE)\" -DPORT=$(PORT) -DDF=$(DF) -DVL=$(VL) -DVERSION=\"$(VERSION)\"

LDFLAGS=

# command to generate tag files
#CTAGS=ctags -tvS
CTAGS=ctags 

# tags file
CTAGSFILE=./tags

# The directory that contains the makescripts
MAKESCRIPTS=../makescripts

###########################################################################

BIN=euchres

SRC=client.c \
    cards.c \
    euchres.c \
    game.c \
    init.c \
    log.c \
    message.c \
    pack.c \
    print.c \
    signal.c \
    socket.c \

OBJ=client.o \
	cards.o \
	euchres.o \
    game.o \
    init.o \
    log.o \
    message.o \
    pack.o \
    print.o \
    signal.o \
    socket.o \

AUX=$(CTAGSFILE) $(CTAGSFILE).vim euchres.log

CLEAN=$(OBJ) $(AUX) core
REALCLEAN=$(CLEAN) $(BIN)
DISTCLEAN=$(REALCLEAN) .*.vim .*.swp

#*******************************#

$(BIN): proto.h tags ../common/messages.pl ../common/suits.pl $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS) 

all : $(OBJ) 
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS) 

tags: $(SRC) $(HEADERS)
	rm -f $(CTAGSFILE)
	$(CTAGS) -f $(CTAGSFILE) *.h *.c
	cat $(CTAGSFILE) | $(MAKESCRIPTS)/tags2syntax > $(CTAGSFILE).vim

clean:
	rm -f $(CLEAN)

realclean:
	rm -f $(REALCLEAN)

distclean:
	rm -f $(DISTCLEAN)

proto.h: $(SRC)
	$(MAKESCRIPTS)/mkproto > proto.h

../common/messages.pl: ../common/messages.h
	$(MAKESCRIPTS)/h2pl ../common/messages.h > ../common/messages.pl

../common/suits.pl: ../common/suits.h
	$(MAKESCRIPTS)/h2pl ../common/suits.h > ../common/suits.pl

%.o : %.c 
	$(CC) $(CFLAGS) -c $< -o $@
