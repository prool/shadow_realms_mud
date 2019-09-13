#
#	muddy libs	(c) 2003
#		(for SR (c) 1999)
#
CC		= gcc
LD		= gcc
RM		= /bin/rm
AR		= /usr/bin/ar
RANLIB		= /usr/bin/ranlib
#INCLUDES	= -I . -I /usr/include/rpcsvc
INCLUDES	= -I .

CFLAGS		= -g -Wall -O -pipe $(INCLUDES)
LFLAGS		= -g -L.
M_LIBS		=
MNET_LIBS	= -lmuddy
INSTALL_PATH	= ..

# If you want to crypt passwords, use -DCRYPT
# example: CFLAGS  += -DBSD44 -DCRYPT
# This option work only in *nix (Unix system).

# If you want to use THREADS, use -DUSE_THREADS in CFLAGS
# and -lpthread in LIBS.
# THREADS give muddy scipting language (Perl) for mob(obj,room, etc...) progs
# and another advance....
# This option work only in *nix (Unix system).
#	Warning!   This feature not work yet!!!
# example: CFLAGS += -DUSE_THREADS
#          LIBS   += -lpthread

# for ON MCCP support uncomment next lines
CFLAGS += -DMCCP
MNET_LIBS += -lz

#
# for FreeBSD
# CFLAGS	+= -DBSD44
# M_LIBS	+= -lcrypt

#
# for Linux with threading
# CFLAGS	+= -DLINUX -DUSE_THREADS
# M_LIBS		+= -lpthread

#
# for Linux
CFLAGS		+= -DLINUX
M_LIBS		+=
MNET_LIBS	+=

#
# for AIX
#CFLAGS  = -DSUNOS -O2 -pipe
#M_CFILES  = strsep.c snprintf.c
#M_OFILES  = strsep.o snprintf.o
#INCLUDES        = -I .


#
# for SunOS 5 (Solaris)
#CFLAGS	+= -DSUNOS
#M_LIBS	+= -lsocket -lnsl
#M_CFILES	+= strsep.c snprintf.c
#M_OFILES	+= strsep.o snprintf.o

#
# for Unixware 2.x
#CFLAGS	+= -DSVR4 -DOLD_RAND
#M_LIBS	+= -lsocket -lnsl -lcrypt
#M_CFILES	+= strsep.c
#M_OFILES	+= strsep.o

#
# Generally Makefile should not be edited below this line
#

M_CFILES	+= varr.c util.c str.c log.c flag.c

M_OFILES	+= varr.o util.o str.o log.o flag.o

MNET_CFILES	+= charset.c mnet.c clients.c tunnels.c resolver.c mccp.c

MNET_OFILES	+= charset.o mnet.o clients.o tunnels.o resolver.o mccp.o

all:	libs
libs:	libmuddy.a libmnet.a
solibs:	libmuddy.so libmnet.so

libmuddy.so:	$(M_OFILES)
	$(RM) -f $@
	$(LD) $(LFLAGS) -shared -o $@ -Wl,-soname=`basename $@` \
	$(M_OFILES) $(M_LIBS)
libmnet.so:	$(MNET_OFILES)
	$(RM) -f $@
	$(LD) $(LFLAGS) -shared -o $@ -Wl,-soname=`basename $@` \
	$(MNET_OFILES) $(MNET_LIBS)

libmuddy.a:	$(M_OFILES)
	$(RM) -f $@
	$(AR) rv $@ $(M_OFILES)
	$(RANLIB) $@
libmnet.a:	$(MNET_OFILES)
	$(RM) -f $@
	$(AR) rv $@ $(MNET_OFILES)
	$(RANLIB) $@

#

.c.o: 
	$(CC) $(CFLAGS) -c $<

depend:
	mkdep $(CFLAGS) $(CFILES)

clean:
	$(RM) -f *.o *.a *.core *.so

ctags:
	ctags -dtw $(CFILES) *.h

gtags:
	gtags -co .

# DO NOT DELETE

