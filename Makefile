############################################################################
#
# Euchre makefile
# Denis McLaughlin
# August 21, 1999
#
###########################################################################

SERVER=src/euchred

###########################################################################

all : $(SERVER)

$(SERVER): src/*.h src/*.c common/*.h
	( cd src ; make )

clean:
	( cd src ; make clean )

distclean:
	( cd src ; make distclean )
	rm -f euchred.log
