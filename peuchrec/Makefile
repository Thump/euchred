
############################################################################
#
# Peuchrec makefile
# Denis McLaughlin
# May 27, 1999
#
###########################################################################

# Dorky Makefile to create perl tags.  I have a perverse urge to put the
# hdb command in here.

# command to generate tag files
CTAGS=../makescripts/perltags

# tags file
CTAGSFILE=./tags

# The directory that contains the makescripts
MAKESCRIPTS=../makescripts

###########################################################################

all : tags ../common/messages.pl

tags : *.pl
	$(CTAGS) > $(CTAGSFILE)

../common/messages.pl: ../common/messages.h
	$(MAKESCRIPTS)/h2pl ../common/messages.h > ../common/messages.pl
