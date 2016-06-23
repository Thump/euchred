############################################################################
#
# Euchre makefile
# Denis McLaughlin
# August 21, 1999
#
###########################################################################

SERVER=euchres/euchres

CLIENT=peuchrec/peuchrec

###########################################################################

all : $(SERVER) $(CLIENT)

$(SERVER): euchres/*.h euchres/*.c common/*.h
	( cd euchres ; make )

$(CLIENT): common/*.pl
	( cd peuchrec ; make )

clean:
	( cd euchres ; make clean )

distclean:
	( cd euchres ; make distclean )
	rm -f euchres.log
