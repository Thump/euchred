
############################################################################
#
# Euchre makefile
# Denis McLaughlin
# May 27, 1999
#
###########################################################################

# The directory that contains the makescripts
MAKESCRIPTS=../makescripts

###########################################################################

all : messages.pl suits.pl

messages.pl: messages.h
	$(MAKESCRIPTS)/h2pl messages.h > messages.pl

suits.pl: suits.h
	$(MAKESCRIPTS)/h2pl suits.h > suits.pl
