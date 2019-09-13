#
#	Shadow Realms	(c) 1999
#

BIN_NAME	= mudsr

CC		= gcc
LD		= gcc
#INCLUDES	= -I . -I /usr/include/rpcsvc -I lib
INCLUDES	= -I . -I lib

CFLAGS		= -g -Wall -O -pipe $(INCLUDES)
LFLAGS		= -g
LIBS		= -L lib -lmuddy -lmnet
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

# for on MCCP support uncomment next lines.
#	Require 'zlib'. Find it:
#		URL: http://www.cdrom.com/pub/infozip/zlib/
#		"zlib*.rpm, zlib-devel*.rpm" for Linux Red Hat
CFLAGS	+= -DMCCP
LIBS	+= -lz

# for build static (not shared linked) muddy uncomment next line
#LFLAGS	+= -static

#
#	for FreeBSD
# CFLAGS	+= -DBSD44
# LIBS	+= -lcrypt

#
#	for Linux with threading
#	NOT WORK !
# CFLAGS	+= -DLINUX -DUSE_THREADS
# LIBS		+= -lpthread

#
#	for Linux
CFLAGS	+= -DLINUX
LIBS	+=

#
#	for AIX
#CFLAGS  = -DSUNOS -O2 -pipe
#CFILES  = strsep.c snprintf.c
#OFILES  = strsep.o snprintf.o
#INCLUDES        = -I .


#
#	for SunOS 5 (Solaris)
#CFLAGS	+= -DSUNOS
#LIBS	+= -lsocket -lnsl
#CFILES	+= strsep.c snprintf.c
#OFILES	+= strsep.o snprintf.o

#
#	for Unixware 2.x
#CFLAGS	+= -DSVR4 -DOLD_RAND
#LIBS	+= -lsocket -lnsl -lcrypt
#CFILES	+= strsep.c
#OFILES	+= strsep.o

#
#	Generally Makefile should not be edited below this line
#
COMM_CFILES = access.c comm_act.c\
	      comm_colors.c comm_control.c comm.c
COMM_OFILES = access.o comm_act.o\
	      comm_colors.o comm_control.o comm.o

DB_CFILES = db.c db_area.c db_class.c db_hometown.c\
	    db_lang.c db_race.c db_skills.c db_socials.c\
	    db_system.c db_religion.c db_damages.c\
	    cmd.c gsn.c hometown.c lang.c msg.c spellfn.c\
	    socials.c 
DB_OFILES = db.o db_area.o db_class.o db_hometown.o\
	    db_lang.o db_race.o db_skills.o db_socials.o\
	    db_system.o db_religion.o db_damages.o\
	    cmd.o gsn.o hometown.o lang.o msg.o spellfn.o\
	    socials.o

OLC_CFILES = olc.c olc_area.c olc_help.c\
	     olc_lang.c olc_mob.c olc_mpcode.c olc_msg.c\
	     olc_room.c olc_rule.c olc_obj.c olc_save.c\
	     olc_social.c olc_religion.c olc_skill.c
OLC_OFILES = olc.o olc_area.o olc_help.o\
	     olc_lang.o olc_mob.o olc_mpcode.o olc_msg.o\
	     olc_room.o olc_rule.o olc_obj.o olc_save.o\
	     olc_social.o olc_religion.o olc_skill.o

CFILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c auction.c\
         buffer.c class.c\
	 effects.c fight.c handler.c healer.c help.c\
	 hunt.c interp.c lookup.c magic.c magic2.c martial_art.c mem.c\
	 mlstring.c mob_cmds.c mob_prog.c namedp.c note.c obj_prog.c quest.c\
	 race.c raffects.c rating.c recycle.c religion.c repair.c\
	 save.c skills.c special.c string_edit.c\
	 tables.c update.c pchar.c rel_magic.c\
	 $(DB_CFILES) $(OLC_CFILES) $(COMM_CFILES)

OFILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o auction.o\
         buffer.o class.o\
	 effects.o fight.o handler.o healer.o help.o\
	 hunt.o interp.o lookup.o magic.o magic2.o martial_art.o mem.o\
	 mlstring.o mob_cmds.o mob_prog.o namedp.o note.o obj_prog.o quest.o\
	 race.o raffects.o rating.o recycle.o religion.o repair.o\
	 save.o skills.o special.o string_edit.o\
	 tables.o update.o pchar.o rel_magic.o\
	 $(DB_OFILES) $(OLC_OFILES) $(COMM_OFILES)

all	: MSG_COMPILE ${BIN_NAME}
mudsr	: muddy
muddy	: $(OFILES)
	rm -f ${BIN_NAME}
	@echo "$(LD) $(LFLAGS) -o ${BIN_NAME} <ofiles> $(LIBS)"
	@$(LD) $(LFLAGS) -o ${BIN_NAME} $(OFILES) $(LIBS)

MSG_COMPILE:
	@echo -e "Compile object file:\n   $(CC) $(CFLAGS) -c <file>"
.c.o: 
	@echo -e "Compile $<"
	@$(CC) $(CFLAGS) -c $<
depend:
	mkdep $(CFLAGS) $(CFILES)

clean:
	rm -f *.o *.core ${BIN_NAME}

ctags:
	ctags -dtw $(CFILES) *.h

gtags:
	gtags -co .

install: muddy
	install -c -m 770 ${BIN_NAME} $(INSTALL_PATH)/bin

# DO NOT DELETE
