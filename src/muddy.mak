CC=cl.exe

CFLAGS=/nologo /ML /W3 /G4e /O2g /D "WIN32" /YX /c
INCLUDES= -I . -I .\COMPAT -I .\COMPAT\regex-win32

LINK32=link.exe

LFLAGS=wsock32.lib kernel32.lib user32.lib gdi32.lib winspool.lib\
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib\
 odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no\
 /machine:I386 /out:"muddy.exe"

COMM_CFILES = ban.c charset.c comm.c comm_act.c\
			  comm_colors.c comm_info.c resolver.c
COMM_OFILES = ban.obj charset.obj comm.obj comm_act.obj\
			  comm_colors.obj comm_info.obj resolver.obj

DB_CFILES = cmd.c db.c db_area.c db_clan.c db_class.c\
			db_skills.c db_lang.c db_race.c\
			gsn.c lang.c msg.c resource.c spellfn.c word.c
DB_OFILES = cmd.obj db.obj db_area.obj db_clan.obj db_class.obj\
			db_skills.obj db_lang.obj db_race.obj gsn.obj\
			lang.obj msg.obj resource.obj spellfn.obj word.obj


OLC_CFILES = olc.c olc_area.c olc_clan.c olc_help.c\
			 olc_lang.c olc_mob.c olc_mpcode.c olc_msg.c\
			 olc_room.c olc_obj.c olc_save.c olc_word.c
OLC_OFILES = olc.obj olc_area.obj olc_clan.obj olc_help.obj\
			 olc_lang.obj olc_mob.obj olc_mpcode.obj\
			 olc_msg.obj olc_room.obj olc_obj.obj olc_save.obj\
			 olc_word.obj


COMPAT_CFILES = compat\winstuff.c compat\strsep.c compat\strcasecmp.c\
				compat\regex-win32\regex_regcomp.c\
				compat\regex-win32\regex_regerror.c\
				compat\regex-win32\regex_regexec.c\
				compat\regex-win32\regex_regfree.c
COMPAT_OFILES = compat\winstuff.obj compat\strsep.obj compat\strcasecmp.obj\
				compat\regex-win32\regex_regcomp.obj\
				compat\regex-win32\regex_regerror.obj\
				compat\regex-win32\regex_regexec.obj\
				compat\regex-win32\regex_regfree.obj

CFILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c auction.c\
		 buffer.c clan.c class.c\
		 effects.c fight.c flag.c handler.c healer.c help.c hometown.c\
		 hunt.c interp.c log.c lookup.c magic.c magic2.c martial_art.c mem.c\
		 mlstring.c mob_cmds.c mob_prog.c namedp.c note.c obj_prog.c quest.c\
		 race.c raffects.c rating.c recycle.c religion.c repair.c\
		 save.c skills.c special.c str.c string_edit.c\
		 tables.c update.c util.c varr.c\
		 $(COMM_CFILES) $(DB_CFILES) $(OLC_CFILES) $(COMPAT_CFILES)

OFILES = act_comm.obj act_info.obj act_move.obj act_obj.obj act_wiz.obj\
	 auction.obj buffer.obj clan.obj class.obj effects.obj fight.obj flag.obj\
	 handler.obj healer.obj help.obj hometown.obj hunt.obj interp.obj log.obj\
	 lookup.obj magic.obj magic2.obj martial_art.obj mem.obj mlstring.obj\
	 mob_cmds.obj mob_prog.obj namedp.obj note.obj obj_prog.obj quest.obj\
	 race.obj raffects.obj rating.obj recycle.obj religion.obj repair.obj\
	 save.obj skills.obj special.obj str.obj string_edit.obj tables.obj\
	 update.obj util.obj varr.obj\
	 $(COMM_OFILES) $(DB_OFILES) $(OLC_OFILES) $(COMPAT_OFILES)


ALL : "muddy.exe"

CLEAN :
		-@erase "muddy.exe"
		-@erase "*.obj"
		-@erase "COMPAT\*.obj"
		-@erase "COMPAT\regex-win32\*.obj"

"muddy.exe" : $(DEF_FILE) $(OFILES)
	$(LINK32) @<<
  $(LFLAGS) $(OFILES)
<<

.c.obj:
	$(CC) $(CFLAGS) $(INCLUDES) /Fo%|pfF.obj $<
