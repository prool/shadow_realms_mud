#
#	Makefile
#		MGaterD (c) for SR by Xor 2002-2003
#

CC		= gcc
LD		= gcc
RM		= /bin/rm -f
AR		= /usr/bin/ar
RANLIB		= /usr/bin/ranlib
ECHO		= /bin/echo
INCLUDES	= -I . -I ../lib

CFLAGS		= -g -Wall -O -pipe $(INCLUDES) 
LFLAGS		= -g

#
# for Linux
CFLAGS		+= -DLINUX -DMCCP
LIBS		+= -L ../lib -lmnet -lmuddy -lz

CFILES		= mgaterd.c tunneling.c control.c
OFILES		= mgaterd.o tunneling.o control.o

mgaterd		: $(OFILES)
		$(RM) mgaterd
		$(LD) $(LFLAGS) -o mgaterd $(OFILES) $(LIBS)
.c.o:
		$(CC) $(CFLAGS) -c $<
clean:
		$(RM) *.o *.core mgaterd

install:	mgaterd
		@$(ECHO) 'Do it youself! :)'

# DO NOT DELETE

