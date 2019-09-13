/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: db.c,v 1.34 2003/07/02 08:55:16 xor Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/

/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>

#if	defined (WIN32)
#	include <compat/compat.h>
#else
#	include <dirent.h>
#endif

#include "merc.h"
#include "rating.h"
#include "socials.h"
#include "update.h"
#include "db.h"
#include "lang.h"
#include "olc.h"
#include "comm.h"

#ifdef SUNOS
#	include "compat.h"
#	define d_namlen d_reclen
#endif

#ifdef SVR4
#	define d_namlen d_reclen
#endif

extern	int	_filbuf		(FILE *);

#if !defined(OLD_RAND)

#if defined(BSD44) || defined(LINUX)
#	include <unistd.h>
#elif defined(WIN32)
#	define random()		rand()
#	define srandom( x ) srand( x )
	int getpid();
	time_t time(time_t *tloc);
#endif

#endif

/* externals for counting purposes */
//extern  DESCRIPTOR_DATA *descriptor_free;

/*
 * Globals.
 */
flag32_t		mud_options;

SHOP_DATA *		shop_first;
SHOP_DATA *		shop_last;

CHAR_DATA *		char_list;
CHAR_DATA *		char_list_lastpc;

OBJ_DATA *		object_list;
TIME_INFO_DATA		time_info;
WEATHER_DATA		weather_info;

ROOM_INDEX_DATA	*	top_affected_room;
int			reboot_counter;

/*
 * Locals.
 */
MOB_INDEX_DATA *	mob_index_hash		[MAX_KEY_HASH];
OBJ_INDEX_DATA *	obj_index_hash		[MAX_KEY_HASH];
ROOM_INDEX_DATA *	room_index_hash		[MAX_KEY_HASH];

//int			line_number;

AREA_DATA *		area_first;
AREA_DATA *		area_last;

int			top_affect;
int			top_area;
int			top_ed;
int			top_exit;
int			top_help;
int			top_mob_index;
int			top_obj_index;
int			top_reset;
int			top_room;
int			top_shop;
int                     top_vnum_room;		/* OLC */
int                     top_vnum_mob;		/* OLC */
int                     top_vnum_obj;		/* OLC */
int			top_mprog_index;	/* OLC */
int 			mob_count;
int			mob_free_count;
int			obj_count;
int			obj_free_count;
int			newmobs;
int			newobjs;

int	nAllocBuf;
int	sAllocBuf;

/*
 * Semi-locals.
 */
//bool			fBootDb;
//char			filename[PATH_MAX];

/*
 * Local booting procedures.
*/
void    init_mm         (void);
 
void	fix_exits	(void);
void    check_mob_progs	(void);

void	reset_area	(AREA_DATA * pArea);
void 	load_limited_objects();

void crush_mud(void)
{
	debug_printf(1, "crush[00]");
	shutdown_prepare("No memory.");
	exit(1);
}

void save_finger_char(CHAR_DATA *ch);
//bool write_to_descriptor(int desc, char *txt, uint length);
void shutdown_prepare(char *message)
{
	DESCRIPTOR_DATA_MUDDY *d /*,*d_next*/;
	FILE *fp;
	
	log_printf("shutdown_prepare: %s", message);
	dunlink(TMP_PATH, PID_FILE);
	
	if (!(fp = dfopen(TMP_PATH, SHUTDOWN_FILE, "w")))
	{
		log_printf("shutdown_prepare: cann't create shutdown file: %s", strerror(errno));
	} else {
		fclose(fp);
	}

	for (d = descriptor_list_muddy; d; d = d->next)
	{
//		d_next = d->next;
		if (d->character)
		{
			if (IS_IMMORTAL(d->character))
				write_to_buffer_muddy(d, message, 0);
				//write_to_descriptor_muddy(d, message, 0);
						//write_to_descriptor_muddy(d," Extra shutdown Shadow Realms.\n\r",0);
			save_finger_char(d->character);
			save_char_obj(d->character, TRUE);
		}
		write_to_buffer_muddy(d," Extra shutdown Shadow Realms.\n\r",0);
		close_descriptor_muddy(d);
	}
}

void save_finger_char(CHAR_DATA *ch);
void reboot_mud(void)
{
	extern bool merc_down;
	DESCRIPTOR_DATA_MUDDY *d /*,*d_next*/;

	log("Rebooting Muddy.");
	for (d = descriptor_list_muddy; d; d = d->next)
	{
	//	d_next = d->next;
		write_to_buffer_muddy(d,"Muddy is going down for rebooting NOW!\n\r",0);
		if (d->character)
		{
			save_finger_char(d->character);
			save_char_obj(d->character, TRUE);
		}
		close_descriptor_muddy(d);
	}
	merc_down = TRUE;    
}

int dbfuncmp(const void *p1, const void *p2)
{
	return str_cmp(*(char**)p1, *(char**)p2);
}

void dbdata_init(DBDATA *dbdata)
{
	dbdata->tab_sz = 0;
	while(dbdata->fun_tab[dbdata->tab_sz].name)
		dbdata->tab_sz++;
	qsort(dbdata->fun_tab, dbdata->tab_sz,
	      sizeof(*dbdata->fun_tab), dbfuncmp);
}

DBFUN *dbfun_lookup(DBDATA *dbdata, const char *name)
{
	return bsearch(&name, dbdata->fun_tab, dbdata->tab_sz,
		       sizeof(*dbdata->fun_tab), dbfuncmp);
}

void db_set_arg(DBDATA *dbdata, const char *name, void *arg)
{
	DBFUN *fun;

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);

	if ((fun = dbfun_lookup(dbdata, name)) == NULL)
		return;

	fun->arg = arg;
}

/*
 * db_parse_file - parses file using dbdata
 * dbdata->tab_sz should be properly intialized
 */
void db_parse_file(DBDATA *dbdata, const char *path, const char *file)
{
	char buf[PATH_MAX];
	int linenum;
	FILE *fp;

	strnzcpy(buf, sizeof(buf), filename);
	linenum = line_number;
	line_number = 0;
	snprintf(filename, sizeof(filename), "%s%c%s",
		 path, PATH_SEPARATOR, file);

	if ((fp = fopen(filename, "r")) == NULL) {
		db_error("db_parse_file", strerror(errno));
		strnzcpy(filename, sizeof(filename), buf);
		line_number = linenum;
		return;
	}

	for (; ;) {
		DBFUN *fn;
		char *word;

		if (fread_letter(fp) != '#') {
			db_error("db_parse_file", "'#' not found");
			break;
		}

		word = fread_word(fp);
		if (word[0] == '$')
			break;

		fn = dbfun_lookup(dbdata, word);
		if (fn) 
			fn->fun(dbdata, fp, fn->arg);
		else {
			db_error("db_parse_file", "bad section name");
			break;
		}
	}
	fclose(fp);

	strnzcpy(filename, sizeof(filename), buf);
	line_number = linenum;
}

void db_load_file(DBDATA *dbdata, const char *path, const char *file)
{
	if (!dbdata->tab_sz)
		dbdata_init(dbdata);
	if (dbdata->dbinit)
		dbdata->dbinit(dbdata);
	db_parse_file(dbdata, path, file);
}

void db_load_list(DBDATA *dbdata, const char *path, const char *file)
{
	FILE *fp;

	if ((fp = dfopen(path, file, "r")) == NULL)
		exit(1);

	if (!dbdata->tab_sz)
		dbdata_init(dbdata);
	for (; ;) {
		char *name = fread_word(fp);
		if (name[0] == '$')
			break;

		if (dbdata->dbinit)
			dbdata->dbinit(dbdata);
		db_parse_file(dbdata, path, name);
	}
	fclose(fp);
}

void init_sspool(void);
void boot_db_system(void)
{
	fBootDb = TRUE;
	db_load_file(&db_system, ETC_PATH, SYSTEM_CONF);
}

/*
 * Big mama top level function.
 */
void boot_db(void)
{
	long lhour, lday, lmonth;
	FILE *file_time;

#ifdef __FreeBSD__
	extern char* malloc_options;
	malloc_options = "X";
#endif
	/*
	 * Init random number generator.
	 */
	init_mm();

	/*
	 * Set time and weather.
	 */

	if ((file_time = dfopen(ETC_PATH, TIME_FILE, "r"))){
		fscanf(file_time, "%ld", &lhour);
		fclose(file_time);
		if (lhour < 0)
			lhour = 0;
	}
	else
		lhour   = 0;
//		lhour	= (current_time - 650336715) / (PULSE_TICK / PULSE_PER_SCD);

	time_info.pulse = 0;				// !!!
	time_info.hour	= lhour  % TIME_HOUR_OF_DAY;
	lday		= lhour  / TIME_HOUR_OF_DAY;
	time_info.day	= lday   % TIME_DAY_OF_MONTH;
	lmonth		= lday   / TIME_DAY_OF_MONTH;
	time_info.month	= lmonth % TIME_MONTH_OF_YEAR;
	time_info.year	= lmonth / TIME_MONTH_OF_YEAR;

	     if (time_info.hour <  5) weather_info.sunlight = SUN_DARK;
	else if (time_info.hour <  6) weather_info.sunlight = SUN_RISE;
	else if (time_info.hour < 19) weather_info.sunlight = SUN_LIGHT;
	else if (time_info.hour < 20) weather_info.sunlight = SUN_SET;
	else                          weather_info.sunlight = SUN_DARK;

	weather_info.change_t	= 0;
	weather_info.change_w	= 0;
	weather_info.waterfull	= 50;

/*	if (time_info.month >= TIME_MONTH_OF_YEAR * 0.4 && time_info.month <= TIME_MONTH_OF_YEAR * 0.7)
		weather_info.mmhg += number_range(1, 50);
	else
		weather_info.mmhg += number_range(1, 80);
*/
	if (time_info.month >= TIME_MONTH_OF_YEAR * 0.875 || time_info.month <= TIME_MONTH_OF_YEAR * 0.125)
		weather_info.temperature	= -50;
	else if (time_info.month >= TIME_MONTH_OF_YEAR * 0.375 && time_info.month <= TIME_MONTH_OF_YEAR * 0.625)
		weather_info.temperature	= 210;
	else
		weather_info.temperature	= 80;
	
	weather_info.sky = SKY_CLOUDY;

/*	     if (weather_info.mmhg <=  980) weather_info.sky = SKY_LIGHTNING;
	else if (weather_info.mmhg <= 1000) weather_info.sky = SKY_RAINING;
	else if (weather_info.mmhg <= 1020) weather_info.sky = SKY_CLOUDY;
	else                                weather_info.sky = SKY_CLOUDLESS;
*/

	/* room_affect_data */
	top_affected_room = NULL;
		
	/* reboot counter */
	reboot_counter = 1440;	/* 24 hours */
	
	init_sspool();

	db_load_list(&db_langs, LANG_PATH, LANG_LIST);
	db_load_file(&db_damages, LANG_PATH, DAMAGES_FILE);
	load_msgdb();
	db_load_file(&db_socials, ETC_PATH, SOCIALS_CONF);
	db_load_file(&db_skills, ETC_PATH, SKILLS_CONF);
	namedp_check(gsn_table);
	namedp_check(spellfn_table);
	db_load_list(&db_races, RACES_PATH, RACE_LIST);
	db_load_list(&db_classes, CLASSES_PATH, CLASS_LIST);
//	db_load_list(&db_clans, CLANS_PATH, CLAN_LIST);
	db_load_list(&db_areas, AREA_PATH, AREA_LIST);
	db_load_file(&db_hometowns, ETC_PATH, HOMETOWNS_CONF);
	db_load_file(&db_religion, ETC_PATH, RELIGION_CONF);
	fBootDb = FALSE;

	/*
	 * Fix up exits.
	 * Declare db booting over.
	 * Reset all areas once.
	 * Load up the songs, notes and ban files.
	 */
	fix_exits();
	check_mob_progs();
	load_limited_objects();

//	convert_objects();           /* ROM OLC */
	area_update();
	load_notes();
	load_bans();
}

/*
 * Sets vnum range for area using OLC protection features.
 */
void vnum_check(AREA_DATA *area, int vnum)
{
	if (area->min_vnum == 0 || area->max_vnum == 0) {
		log_printf("%s: min_vnum or max_vnum not assigned",
			   area->file_name);
#if 0
		area->min_vnum = area->max_vnum = vnum;
#endif
	}

	if (vnum < area->min_vnum || vnum > area->max_vnum) {
		log_printf("%s: %d not in area vnum range",
			   area->file_name, vnum);
#if 0
		if (vnum < area->min_vnum)
			area->min_vnum = vnum;
		else
			area->max_vnum = vnum;
#endif
	}
}

/*
 * Adds a reset to a room.  OLC
 * Similar to add_reset in olc.c
 */
void new_reset(ROOM_INDEX_DATA *pR, RESET_DATA *pReset)
{
	RESET_DATA *pr;
 
	if (!pR)
		return;
 
	pr = pR->reset_last;
 
	if (!pr) {
		 pR->reset_first = pReset;
		 pR->reset_last  = pReset;
	}
	else {
		 pR->reset_last->next = pReset;
		 pR->reset_last       = pReset;
		 pR->reset_last->next = NULL;
	}

	top_reset++;
}

/*
 *  Check mobprogs
 */
void check_mob_progs(void)
{
    MOB_INDEX_DATA *mob;
    MPTRIG        *mptrig;
    int iHash;

    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
    {
	for (mob = mob_index_hash[iHash]; mob; mob = mob->next)
	{
	    for(mptrig = mob->mptrig_list; mptrig; mptrig = mptrig->next)
	    {
		if (mpcode_lookup(mptrig->vnum) == NULL) {
		    db_error("check_mob_progs", "code vnum %d not found.",
			     mptrig->vnum);
		}
	    }
	}
    }
}
 
void fix_exits_room(ROOM_INDEX_DATA *room)
{
	int door;

	for (door = 0; door < MAX_DIR; door++) {
		EXIT_DATA *pexit;

		if ((pexit = room->exit[door]) == NULL)
			continue;

		pexit->to_room.r = get_room_index(pexit->to_room.vnum);
	}
}

/*
 * Translate all room exits from virtual to real.
 * Has to be done after all rooms are read in.
 */
void fix_exits(void)
{
	ROOM_INDEX_DATA *room;
	int iHash;

	for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
		for (room = room_index_hash[iHash]; room; room = room->next)
			fix_exits_room(room);
}

void print_resetmsg(AREA_DATA *pArea)
{
	DESCRIPTOR_DATA_MUDDY *d;
	bool is_empty = mlstr_null(pArea->resetmsg);
	
	for (d = descriptor_list_muddy; d; d = d->next)
	{
		CHAR_DATA *ch;

		if (d->connected != CON_PLAYING)
			continue;

		ch = d->character;
		if (IS_NPC(ch) || !IS_AWAKE(ch)
		|| ch->in_room == NULL
		|| ch->in_room->area != pArea
		|| IS_SET(ch->comm, COMM_CUT_MESSAGE))
			continue;

		if (is_empty)
			act_puts("You hear some squeaking sounds...",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts(mlstr_cval(pArea->resetmsg, ch),
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
	}
}

/*
 * Repopulate areas periodically.
 */
void area_update(void)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea != NULL; pArea = pArea->next) {
		if (++pArea->age < 3)
			continue;

		/*
		 * Check age and reset.
		 * Note: Mud School resets every 3 minutes (not 15).
		 */
		if ((!pArea->empty && (pArea->nplayer == 0 || pArea->age >= 15))
		||  pArea->age >= 31) {
			ROOM_INDEX_DATA *pRoomIndex;

			/*
			 * the rain devastates tracks on the ground
			 */
			if (weather_info.sky == SKY_RAINING || weather_info.sky == SKY_SNOWING) {
				int i;
				DESCRIPTOR_DATA_MUDDY *d;
				CHAR_DATA *ch;

				for (d = descriptor_list_muddy; d; d = d->next)
				{
					if (d->connected != CON_PLAYING)
						continue;

					ch = (d->original != NULL) ?
						d->original : d->character;
					if (ch->in_room->area == pArea
					&&  get_skill(ch, gsn_track) > 50
					&& !IS_SET(ch->in_room->room_flags,
								ROOM_INDOORS))
					{
						if (weather_info.sky == SKY_RAINING)
							act_puts("Rain devastates the tracks on the ground.",
								 ch, NULL, NULL, TO_CHAR, POS_DEAD);
						else
							act_puts("Snow devastates the tracks on the ground.",
								ch, NULL, NULL, TO_CHAR, POS_DEAD);
					}
				}

				for (i = pArea->min_vnum; i < pArea->max_vnum;
									i++) {
					pRoomIndex = get_room_index(i);
					if (pRoomIndex == NULL
					||  IS_SET(pRoomIndex->room_flags,
								ROOM_INDOORS))
						continue;
					room_record("erased", pRoomIndex, -1);  
					if (number_percent() < 50)
						room_record("erased",
							    pRoomIndex, -1);
				}
			}

			reset_area(pArea);
			wiznet("$t has just been reset.",
				NULL, pArea->name, WIZ_RESETS, 0, 0);

			print_resetmsg(pArea);

			pArea->age = number_range(0, 3);
			pRoomIndex = get_room_index(200);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(210);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(220);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(230);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			pRoomIndex = get_room_index(ROOM_VNUM_SCHOOL);
			if (pRoomIndex != NULL && pArea == pRoomIndex->area)
				pArea->age = 15 - 2;
			else if (pArea->nplayer == 0) 
				pArea->empty = TRUE;
		}
	}
}

/*
 * OLC
 * Reset one room.  Called by reset_area and olc.
 */
void reset_room(ROOM_INDEX_DATA *pRoom)
{
    RESET_DATA  *pReset;
    CHAR_DATA   *pMob;
    CHAR_DATA	*mob;
    OBJ_DATA    *pObj;
    CHAR_DATA   *LastMob = NULL;
    OBJ_DATA    *LastObj = NULL;
    int iExit;
    int level = 0;
    bool last;
    int vrel;

    if (!pRoom)
        return;

    pMob        = NULL;
    last        = FALSE;	// only for mobs !!!
    
    for (iExit = 0;  iExit < MAX_DIR;  iExit++)
    {
        EXIT_DATA *pExit;
        if ((pExit = pRoom->exit[iExit]))  
        {
            pExit->exit_info = pExit->rs_flags;
            if ((pExit->to_room.r != NULL)
              && ((pExit = pExit->to_room.r->exit[rev_dir[iExit]])))
            {
                /* nail the other side */
                pExit->exit_info = pExit->rs_flags;
            }
        }
    }

    for (pReset = pRoom->reset_first; pReset != NULL; pReset = pReset->next)
    {
        MOB_INDEX_DATA  *pMobIndex;
        OBJ_INDEX_DATA  *pObjIndex;
        OBJ_INDEX_DATA  *pObjToIndex;
        ROOM_INDEX_DATA *pRoomIndex;
	int count,limit=0;
	//int cln;
	//clan_t* clan=NULL;
        EXIT_DATA *pExit;
        int max;

        switch (pReset->command)
        {
        default:
                bug("Reset_room: bad command %c.", pReset->command);
                break;

        case 'M':
            if (!(pMobIndex = get_mob_index(pReset->arg1)))
            {
                bug("Reset_room: 'M': bad vnum %d.", pReset->arg1);
                continue;
            }

	    if ((pRoomIndex = get_room_index(pReset->arg3)) == NULL)
	    {
		bug("Reset_area: 'R': bad vnum %d.", pReset->arg3);
		continue;
	    }
            
/* */
	    count = 0;
	    for (mob = pRoomIndex->people; mob != NULL; mob = mob->next_in_room)
		if (mob->pIndexData == pMobIndex)
		{
		    count++;
		    if (count >= pReset->arg4 || pMobIndex->count >= pReset->arg2)
		    {
		    	if (JUST_KILLED(mob)) {
		    		LastMob = NULL;
		    		last = FALSE;
		    	} else {
		    		last = TRUE;
		    		LastMob = mob;
		    	}
		    	break;		// goto next reset
		    }

		}
	    if (count >= pReset->arg4 || pMobIndex->count >= pReset->arg2)
            {
                break;
            }

		pMob = create_mob(pMobIndex);
		pMob->zone = pRoom->area;
		char_to_room(pMob, pRoom);
		if (JUST_KILLED(pMob))
			LastMob = NULL;
		else
            		LastMob = pMob;

		level  = URANGE(0, pMob->level - 2, LEVEL_HERO - 1);
		last = TRUE;
		/*  Check religion god  */
		
		if ((vrel = rlook_god(pMobIndex->vnum)) >= 0)
		{
			RELIGION_DATA *rel = RELIGION(vrel);
			rel->god = pMob;
			rel->godroom = pRoom->vnum;
			SET_BIT(pMobIndex->act, ACT_GOD);
			log_printf("Religion[%d]: %s[%d] mob up [%d r]",
				vrel, mlstr_mval(pMob->short_descr),
				pMobIndex->vnum, pRoom->vnum);
		}
		break;

        case 'O':
        case 'B':
        case 'H':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
                log_printf("reset_room: '%c' 1 : bad vnum %d",
                	pReset->command, pReset->arg1);
                log_printf("reset_room: %c %d %d %d",pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
                continue;
            }

            if (!(pRoomIndex = get_room_index(pReset->arg3)))
            {
                log_printf("reset_room: '%c' 2 : bad vnum %d.",
                	pReset->command, pReset->arg3);
                log_printf("reset_room: %c %d %d %d", pReset->arg1, pReset->arg2, pReset->arg3, pReset->arg4);
                continue;
            }

            if ((pRoom->area->nplayer > 0 && number_percent() > 10)
              || count_obj_list(pObjIndex, pRoom->contents) > 0)
	    {
		break;
	    }
		    if ((pObjIndex->limit != -1) &&
		         (pObjIndex->count >= pObjIndex->limit))
		      {
		        break;
		      }

            pObj = create_obj(pObjIndex, 0);
            /* pObj->cost = 0; */
            /* don't use obj_to_room(pObj, pRoom) because it do decay */
            pObj->next_content	= pRoom->contents;
            pRoom->contents	= pObj;
            pObj->in_room	= pRoom;
            pObj->carried_by	= NULL;
            pObj->in_obj	= NULL;
            REMOVE_BIT(pObj->wear_flags, ITEM_DECOMPOSE);

            if (pReset->command == 'B')
            	SET_BIT(pObj->hidden_flags, OHIDE_BURYED);
            else if (pReset->command == 'H')
            	SET_BIT(pObj->hidden_flags, OHIDE_HIDDEN);

	    break;

        case 'P':
        case 'U':
        case 'N':
        case 'A':
		/* Near command	(hide Under, On, bury under(A))
		 * (P)ut command
		 * arg1 - vnum of obj to put
		 * arg2 - max obj
		 * arg3 - vnum of obj to put into
		 * arg4 - min obj
		 */

            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
                log_printf("BUG: Reset_room: '%c': bad vnum %d.",
                	pReset->command, pReset->arg1);
                break;
            }
	    
            if (!(pObjToIndex = get_obj_index(pReset->arg3)))
            {
                log_printf("BUG: Reset_room: '%c': bad vnum %d.",
                	pReset->command, pReset->arg3);
                break;
            }

            /*if (pReset->arg2 > 50) // old format
                limit = 50;
            elseif (pReset->arg2 == -1) // no limit
                limit = 999;
            else*/
                limit = pReset->arg2;

            if (   /* pRoom->area->nplayer > 0 ||*/
                 (LastObj = get_obj_type_in_room(pObjToIndex, pRoom)) == NULL
              || (LastObj->in_room == NULL && (!last || pReset->command != 'P')))
		{
			break;
		}
	    
	    count = count_obj_list(pObjIndex, pReset->command == 'P' ?
			    	LastObj->contains : LastObj->in_room->contents);
	    if (limit == -1)
	    {
		if ((count > pReset->arg4	&& number_percent() < 50)
		|| (count > pReset->arg4 * 2	&& number_percent() < 70)
		|| (count > pReset->arg4 * 5	&& number_percent() < 90)
		|| (count > pReset->arg4 * 20	&& number_percent() < 99))
			break;
	    } else if  (
	    (pObjIndex->count >= limit
		&& (number_range(0, pObjIndex->count - limit + 1) != 0
			&& number_percent() > 1))	// 1% reset always present!
	    || (count > pReset->arg4 && number_range(0,  UMAX(0, count - pReset->arg4)))
	    || count >= pReset->arg2
            )
	    {
		break;
	    }
				                /* lastObj->level  -  ROM */

		if ((pObjIndex->limit != -1) &&
		(pObjIndex->count >= pObjIndex->limit)) {
			break;
		}

	    pObj = create_obj(pObjIndex, 0);

	    /*
	    if (IS_SET(pObjIndex->extra_flags, ITEM_CLAN)) {
	    	clan = NULL;
		for (cln = 0; cln < clans.nused; cln++) 
			if(pObjIndex->vnum == clan_lookup(cln)->obj_vnum)
				clan = clan_lookup(cln);
		if (clan != NULL && clan->obj_ptr == NULL) {
			clan->obj_ptr = pObj;
			clan->altar_ptr = LastObj;
		}
	    }
	    */

	    if (pReset->command == 'P') {
	    	obj_to_obj(pObj, LastObj);
		    	/* fix object lock state! */
		LastObj->value[1] = LastObj->pIndexData->value[1];
	    } else if (pReset->command == 'U') {
	    	obj_under_obj(pObj, LastObj);
	    	if (number_bits(1))
	    		SET_BIT(pObj->hidden_flags, OHIDE_HIDDEN_NEAR);
	    	else
	    		SET_BIT(pObj->hidden_flags, OHIDE_HIDDEN);
	    } else if (pReset->command == 'N') {
		obj_on_obj(pObj, LastObj);
	    } else {	// pReset->command == 'A'
	    	obj_under_obj(pObj, LastObj);
	    	SET_BIT(pObj->hidden_flags, OHIDE_BURYED);
	    }
	    REMOVE_BIT(pObj->wear_flags, ITEM_DECOMPOSE);
            break;

        case 'G':
        case 'E':
            if (!(pObjIndex = get_obj_index(pReset->arg1)))
            {
                bug("Reset_room: 'E' or 'G': bad vnum %d.", pReset->arg1);
                break;
            }

            if (!last)
                break;

            if (!LastMob)
            {
                bug("Reset_room: 'E' or 'G': null mob for vnum %d.",
                    pReset->arg1);
                last = FALSE;
                break;
            }
            
		max = 0;
		/* Check limits... */
		if ((pObjIndex->limit != -1) &&
		(pObjIndex->count >= pObjIndex->limit)) {
			break;
		}

	    if (pReset->command == 'E')
	    {
	    	if (get_eq_char(LastMob, pReset->arg3) != NULL)
	    		break;
	    } else
            {
            	count = count_obj_list(pObjIndex, LastMob->carrying);
            	if (pReset->arg2 == 0) {
            		if (count)
            			break;
            	} else {
	        	if (count >= pReset->arg2)
	        	    	break;
			/* Limits see above... */
			max = UMIN(15, pReset->arg2) - count;
			if ((count > 0 || pReset->arg2 > 1)
			&& number_percent() < ((max * 75) / pReset->arg2))
				break;
        	}
            } 
            
	    pObj = create_obj(pObjIndex, 0);
	    if (LastMob->pIndexData->pShop && pReset->command == 'G') {  /* Shop-keeper? */
	    	SET_BIT(pObj->hidden_flags, OHIDE_INVENTORY);
	    	if (pReset->arg2 == 0)
	    		SET_BIT(pObj->hidden_flags, OHIDE_INFINITY_SELL);
	    }
	    obj_to_char(pObj, LastMob);
	    if (pReset->command == 'E')
	    	equip_char(LastMob, pObj, pReset->arg3);
            break;

        case 'D':
		    if ((pRoomIndex = get_room_index(pReset->arg1)) == NULL)
		    {
			bug("Reset_area: 'D': bad vnum %d.", pReset->arg1);
			break;
		    }

		    if ((pExit = pRoomIndex->exit[pReset->arg2]) == NULL)
			break;

		    switch (pReset->arg3)
		    {
		    case 0:
			REMOVE_BIT(pExit->exit_info,
				EX_CLOSED | EX_LOCKED | EX_BURYED);
			break;

		    case 1:
			SET_BIT(  pExit->exit_info, EX_CLOSED);
			REMOVE_BIT(pExit->exit_info, EX_LOCKED | EX_BURYED);
			break;

		    case 2:
			SET_BIT(  pExit->exit_info, EX_CLOSED | EX_LOCKED);
			REMOVE_BIT(pExit->exit_info, EX_BURYED);
			break;
		    case 3:
			REMOVE_BIT(pExit->exit_info, EX_CLOSED | EX_LOCKED);
			SET_BIT(pExit->exit_info, EX_BURYED);
			break;
		    case 4:
			SET_BIT(  pExit->exit_info, EX_CLOSED | EX_BURYED);
			REMOVE_BIT(pExit->exit_info, EX_LOCKED);
			break;
		    case 5:
			SET_BIT(  pExit->exit_info,
				EX_CLOSED | EX_LOCKED | EX_BURYED);
			break;
		    }

            break;

/*        case 'R':
            if (!(pRoomIndex = get_room_index(pReset->arg1)))
            {
                bug("Reset_room: 'R': bad vnum %d.", pReset->arg1);
                break;
            }

            {

                for (d0 = 0; d0 < pReset->arg2 - 1; d0++)
                {
                    d1                   = number_range(d0, pReset->arg2-1);
                    pExit                = pRoomIndex->exit[d0];
                    pRoomIndex->exit[d0] = pRoomIndex->exit[d1];
                    pRoomIndex->exit[d1] = pExit;
                }
            }
            break;
*/
        }
    }
    
    if (IS_SET(pRoom->room_flags, ROOM_HASH_EXITS))	// by Xor for ^SR^
    {
    	EXIT_DATA *pExit;
    	char	hash_exits[MAX_DIR];
    	int	quantity_exits = 0;
    	int	d0, d1;
    	int	i;
   
    	for (iExit = 0;  iExit < MAX_DIR;  iExit++)
    		if (pRoom->exit[iExit]) {
    			quantity_exits++;
    			hash_exits[iExit] = 1;
    		}
    	while(quantity_exits > 1)
    	{
    		d0 = d1 = 0;
    		i = number_range(0, --quantity_exits);
    		for (iExit = 0;  iExit < MAX_DIR;  iExit++)
    			if (pRoom->exit[iExit] && hash_exits[iExit] && !(i--))
    			{
    				d0 = iExit;
    				hash_exits[iExit] = 0;
    				break;
    			}
    		
    		i = number_range(0, --quantity_exits);
    		for (iExit = 0;  iExit < MAX_DIR;  iExit++)
    			if (pRoom->exit[iExit] && hash_exits[iExit] && !(i--))
    			{
    				d1 = iExit;
    				hash_exits[iExit] = 0;
    				break;
    			}
    		if (d0 == d1)
    		{
    			bug("reset_room: error in algoritm HASH_EXITS", 0);
    		} else {
	    		pExit		= pRoom->exit[d0];
    			pRoom->exit[d0]	= pRoom->exit[d1];
    			pRoom->exit[d1]	= pExit;
    		}
    	}
    }
}

/*
 * OLC
 * Reset one area.
 */
void reset_area(AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoom;
	int vnum;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++)
        	if ((pRoom = get_room_index(vnum)))
			reset_room(pRoom);
}

static void obj_of_callback(int lang, const char **p, void *arg)
{
	mlstring *owner = (mlstring*) arg;
	const char *q;

	if (IS_NULLSTR(*p))
		return;

	q = str_printf(*p, word_form(mlstr_val(owner, lang), 1,
				     lang, RULES_CASE));
	free_string(*p);
	*p = q;
}

/*
 * Create an instance of a mobile.
 */
CHAR_DATA *create_mob(MOB_INDEX_DATA *pMobIndex)
{
	CHAR_DATA *mob;
	int i;
	AFFECT_DATA af;

	if (pMobIndex == NULL) {
		bug("Create_mobile: NULL pMobIndex.", 0);
		exit(1);
	}

	mob = new_char();

	mob->pIndexData		= pMobIndex;

	mob->name		= str_qdup(pMobIndex->name);
	mob->short_descr	= mlstr_dup(pMobIndex->short_descr);
	mob->long_descr		= mlstr_dup(pMobIndex->long_descr);
	mob->description	= mlstr_dup(pMobIndex->description);
	mob->spec_fun		= pMobIndex->spec_fun;
	mob->class		= 0;

	if (pMobIndex->wealth) {
		long wealth;

		wealth = number_range(pMobIndex->wealth/2,
				      3 * pMobIndex->wealth/2);
		mob->gold = number_range(wealth/200,wealth/100);
		mob->silver = wealth - (mob->gold * 100);
	} 

//	mob->plr_flags		= ACT_NPC;
	mob->comm		= COMM_NOSHOUT | COMM_NOMUSIC | COMM_NOCHAT;
	mob->affected_by	= pMobIndex->affected_by;
	mob->alignment		= pMobIndex->alignment;
	mob->level		= pMobIndex->level;
	mob->imm_flags		= pMobIndex->imm_flags;
	mob->res_flags		= pMobIndex->res_flags;
	mob->vuln_flags		= pMobIndex->vuln_flags;
	mob->start_pos		= pMobIndex->start_pos;
	mob->position		= mob->start_pos;
	mob->default_pos	= pMobIndex->default_pos;
	mob->race		= pMobIndex->race;
	mob->form		= pMobIndex->form;
	mob->parts		= pMobIndex->parts;
	mob->size		= pMobIndex->size;
	//mob->clan		= pMobIndex->clan;
//	mob->material		= str_qdup(pMobIndex->material);
	mob->slang		= can_speak(mob, pMobIndex->slang) ?
			pMobIndex->slang : get_default_speak(mob);
	
	for (i = 0; i < QUANTITY_MAGIC + 1; i ++)
		mob->saving_throws[i] = pMobIndex->saving_throws[i];

	mob->dam_type		= pMobIndex->dam_type;
	if (mob->dam_type == 0)
		switch(number_range(1,3)) {
		case (1): mob->dam_type = 3;        break;  /* slash */
		case (2): mob->dam_type = 7;        break;  /* pound */
		case (3): mob->dam_type = 11;       break;  /* pierce */
		}

	mob->sex		= pMobIndex->sex;
	if (mob->sex == SEX_EITHER) { /* random sex */
		MOB_INDEX_DATA *fmob;
		mob->sex = number_range(SEX_MALE, SEX_FEMALE);
		if (mob->sex == SEX_FEMALE
		&&  (fmob = get_mob_index(pMobIndex->fvnum))) {
			mob->name	= str_qdup(fmob->name);
			mob->short_descr= mlstr_dup(fmob->short_descr);
			mob->long_descr	= mlstr_dup(fmob->long_descr);
			mob->description= mlstr_dup(fmob->description);
		}
	}

	for (i = 0; i < MAX_STATS; i ++)
		mob->perm_stat[i] = UMIN(25, 8 + mob->level/4);

	mob->perm_stat[STAT_STR] += mob->size - SIZE_MEDIUM;
	mob->perm_stat[STAT_CON] += (mob->size - SIZE_MEDIUM) / 2;

	mob->hitroll		= (mob->level / 2) + pMobIndex->hitroll;
	mob->damroll		= pMobIndex->damage[DICE_BONUS];
	mob->max_hit		= dice(pMobIndex->hit[DICE_NUMBER],
				       pMobIndex->hit[DICE_TYPE])
				  + pMobIndex->hit[DICE_BONUS];
	mob->hit		= mob->max_hit;
	mob->max_mana		= dice(pMobIndex->mana[DICE_NUMBER],
				       pMobIndex->mana[DICE_TYPE])
				  + pMobIndex->mana[DICE_BONUS];
	mob->max_move		= 100 + mob->level;
	mob->move		= mob->max_move;
	mob->mana		= mob->max_mana;
	mob->damage[DICE_NUMBER]= pMobIndex->damage[DICE_NUMBER];
	mob->damage[DICE_TYPE]	= pMobIndex->damage[DICE_TYPE];
	for (i = 0; i < 4; i++)
		mob->armor[i]	= pMobIndex->ac[i]; 

	if (IS_SET(pMobIndex->act, ACT_RIDEABLE)) {
		mob->max_move	+= 100 + mob->level * 10;
		mob->move	= mob->max_move / 2;
	}
	if (IS_SET(pMobIndex->act, ACT_UNDEAD)) {
		mob->max_mana += mob->level * 10;
	}
	if (IS_SET(pMobIndex->act, ACT_WARRIOR)) {
		mob->perm_stat[STAT_STR] += 3;
		mob->perm_stat[STAT_INT] -= 1;
		mob->perm_stat[STAT_CON] += 2;
	}
		
	if (IS_SET(pMobIndex->act, ACT_THIEF)) {
		mob->perm_stat[STAT_DEX] += 3;
		mob->perm_stat[STAT_INT] += 1;
		mob->perm_stat[STAT_WIS] -= 1;
	}
		
	if (IS_SET(pMobIndex->act, ACT_CLERIC)) {
		mob->perm_stat[STAT_WIS] += 3;
		mob->perm_stat[STAT_DEX] -= 1;
		mob->perm_stat[STAT_STR] += 1;
		mob->max_mana += mob->level * 10;
	}
		
	if (IS_SET(pMobIndex->act, ACT_MAGE)) {
		mob->perm_stat[STAT_INT] += 3;
		mob->perm_stat[STAT_STR] -= 1;
		mob->perm_stat[STAT_DEX] += 1;
		mob->max_mana += mob->level * 15;
	}
		
	if (IS_SET(pMobIndex->off_flags, OFF_FAST))
		mob->perm_stat[STAT_DEX] += 2;
		    
	/* let's get some spell action */
	if (IS_AFFECTED(mob,AFF_SANCTUARY)) {
		af.where	= TO_AFFECTS;
		af.type		= sn_lookup("sanctuary");
		af.level	= mob->level;
		af.duration	= -1;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= AFF_SANCTUARY;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob, AFF_HASTE)) {
		af.where	= TO_AFFECTS;
		af.type		= sn_lookup("haste");
		af.level	= mob->level;
	  	af.duration	= -1;
		af.location	= APPLY_DEX;
		af.modifier	= 1 + (mob->level >= 18) + (mob->level >= 25) + 
				  (mob->level >= 32);
		af.bitvector	= AFF_HASTE;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob,AFF_PROTECT_EVIL)) {
		af.where	= TO_AFFECTS;
		af.type		= sn_lookup("protection evil");
		af.level	= mob->level;
		af.duration	= -1;
		af.location	= APPLY_SAVES;
		af.modifier	= 1;
		af.bitvector	= AFF_PROTECT_EVIL;
		affect_to_char(mob, &af);
	}

	if (IS_AFFECTED(mob,AFF_PROTECT_GOOD)) {
		af.where	= TO_AFFECTS;
		af.type		= sn_lookup("protection good");
		af.level	= mob->level;
		af.duration	= -1;
		af.location	= APPLY_SAVES;
		af.modifier	= 1;
		af.bitvector	= AFF_PROTECT_GOOD;
		affect_to_char(mob, &af);
	}  

	/* link the mob to the world list */
	if (char_list_lastpc) {
		mob->next = char_list_lastpc->next;
		char_list_lastpc->next = mob;
	}
	else {
		mob->next = char_list;
		char_list = mob;
	}

	pMobIndex->count++;
	return mob;
}

CHAR_DATA *create_mob_of(MOB_INDEX_DATA *pMobIndex, mlstring *owner)
{
	CHAR_DATA *mob = create_mob(pMobIndex);

	mlstr_for_each(&mob->short_descr, owner, obj_of_callback);
	mlstr_for_each(&mob->long_descr, owner, obj_of_callback);
	mlstr_for_each(&mob->description, owner, obj_of_callback);

	return mob;
}

/* duplicate a mobile exactly -- except inventory */
void clone_mob(CHAR_DATA *parent, CHAR_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;

	if (parent == NULL || clone == NULL || !IS_NPC(parent))
		return;
	
	/* start fixing values */ 
	clone->name 		= str_qdup(parent->name);
	clone->short_descr	= mlstr_dup(parent->short_descr);
	clone->long_descr	= mlstr_dup(parent->long_descr);
	clone->description	= mlstr_dup(parent->description);
	clone->sex		= parent->sex;
	clone->class		= parent->class;
	clone->race		= parent->race;
	clone->level		= parent->level;
	clone->timer		= parent->timer;
	clone->wait		= parent->wait;
	clone->hit		= parent->hit;
	clone->max_hit		= parent->max_hit;
	clone->mana		= parent->mana;
	clone->max_mana		= parent->max_mana;
	clone->move		= parent->move;
	clone->max_move		= parent->max_move;
	clone->gold		= parent->gold;
	clone->silver		= parent->silver;
	clone->comm		= parent->comm;
	clone->imm_flags	= parent->imm_flags;
	clone->res_flags	= parent->res_flags;
	clone->vuln_flags	= parent->vuln_flags;
	clone->affected_by	= parent->affected_by;
	clone->position		= parent->position;
	clone->saving_throws[0]	= parent->saving_throws[0];
	clone->alignment	= parent->alignment;
	clone->hitroll		= parent->hitroll;
	clone->damroll		= parent->damroll;
	clone->form		= parent->form;
	clone->parts		= parent->parts;
	clone->size		= parent->size;
//	clone->material		= str_qdup(parent->material);
	clone->dam_type		= parent->dam_type;
	clone->start_pos	= parent->start_pos;
	clone->default_pos	= parent->default_pos;
	clone->spec_fun		= parent->spec_fun;
	clone->hunting		= NULL;
	//clone->clan		= parent->clan;
	clone->slang		= parent->slang;
	
	for (i = 0; i < QUANTITY_MAGIC + 1; i ++)
		clone->saving_throws[i] = parent->saving_throws[i];;

	for (i = 0; i < 4; i++)
		clone->armor[i]	= parent->armor[i];

	for (i = 0; i < MAX_STATS; i++)
	{
		clone->perm_stat[i]	= parent->perm_stat[i];
		clone->mod_stat[i]	= parent->mod_stat[i];
	}

	for (i = 0; i < 3; i++)
		clone->damage[i]	= parent->damage[i];

	/* now add the affects */
	for (paf = parent->affected; paf != NULL; paf = paf->next)
		affect_to_char(clone,paf);

}

/*
 * Create an instance of an object.
 */
OBJ_DATA *create_obj(OBJ_INDEX_DATA *pObjIndex, int flags)
{
	OBJ_DATA *obj;
	int i;

	if (pObjIndex == NULL) {
		bug("Create_object: NULL pObjIndex.", 0);
		exit(1);
	}

	obj = new_obj();

	obj->pIndexData	= pObjIndex;
	if (IS_SET(pObjIndex->extra_flags, ITEM_FUZZY_LEVEL))
		obj->level = UMAX(0, pObjIndex->level + number_range(0, 10) - 5);
	else
	 	obj->level = pObjIndex->level;
	obj->wear_loc	= -1;

	obj->name		= str_qdup(pObjIndex->name);
	obj->short_descr	= mlstr_dup(pObjIndex->short_descr);
	obj->description	= mlstr_dup(pObjIndex->description);
//	obj->material		= str_qdup(pObjIndex->material);
	obj->extra_flags	= (pObjIndex->extra_flags & 0xffffffff);
	obj->wear_flags		= pObjIndex->wear_flags;
	obj->value[0]		= pObjIndex->value[0];
	obj->value[1]		= pObjIndex->value[1];
	obj->value[2]		= pObjIndex->value[2];
	obj->value[3]		= pObjIndex->value[3];
	obj->value[4]		= pObjIndex->value[4];
	obj->weight		= pObjIndex->weight;
	obj->owner		= NULL;
	obj->condition		= pObjIndex->condition;
	obj->cost		= pObjIndex->cost;

	/*
	 * Mess with object properties.
	 */
	switch (pObjIndex->item_type) {
	case ITEM_LIGHT:
		if (obj->value[2] == 999)
			obj->value[2] = -1;
		break;

	case ITEM_JUKEBOX:
		for (i = 0; i < 5; i++)
		   obj->value[i] = -1;
		break;
	}

/*			see ch->pcdata->add_spell_level
 *	for (paf = pObjIndex->affected; paf != NULL; paf = paf->next) 
 *		if (paf->location == APPLY_SPELL_AFFECT)
 *			affect_to_obj(obj,paf);
 */
	
	obj->next	= object_list;
	object_list	= obj;
	if (!IS_SET(flags, CO_F_NOCOUNT))
		pObjIndex->count++;
	return obj;
}

OBJ_DATA *create_obj_of(OBJ_INDEX_DATA *pObjIndex, mlstring *owner)
{
	OBJ_DATA *obj = create_obj(pObjIndex, 0);

	mlstr_for_each(&obj->short_descr, owner, obj_of_callback);
	mlstr_for_each(&obj->description, owner, obj_of_callback);

	return obj;
}

/* duplicate an object exactly -- except contents */
void clone_obj(OBJ_DATA *parent, OBJ_DATA *clone)
{
	int i;
	AFFECT_DATA *paf;
	ED_DATA *ed,*ed2;

	if (parent == NULL || clone == NULL)
		return;

	/* start fixing the object */
	clone->name 		= str_qdup(parent->name);
	clone->short_descr	= mlstr_dup(parent->short_descr);
	clone->description	= mlstr_dup(parent->description);
	clone->extra_flags	= parent->extra_flags;
	clone->wear_flags	= parent->wear_flags;
	clone->weight		= parent->weight;
	clone->cost		= parent->cost;
	clone->level		= parent->level;
	clone->condition	= parent->condition;
//	clone->material		= str_qdup(parent->material);
	clone->timer		= parent->timer;
	clone->owner		= mlstr_dup(parent->owner);
	clone->hidden_flags	= parent->hidden_flags;

	for (i = 0;  i < 5; i ++)
		clone->value[i]	= parent->value[i];

	for (paf = parent->affected; paf != NULL; paf = paf->next) 
		affect_to_obj(clone,paf);

	/* extended desc */
	for (ed = parent->ed; ed != NULL; ed = ed->next) {
		ed2			= ed_new();
		ed2->keyword		= str_qdup(ed->keyword);
		ed2->slang		= ed->slang;
		ed2->description	= mlstr_dup(ed->description);
		ed2->next		= clone->ed;
		clone->ed		= ed2;
	}

}

/*
 * Get an extra description from a list.
 */
ED_DATA *ed_lookup(const char *name, ED_DATA *ed)
{
	for (; ed != NULL; ed = ed->next) {
		if (is_name(name, ed->keyword))
			return ed;
	}
	return NULL;
}

/*
 * Translates mob virtual number to its mob index struct.
 * Hash table lookup.
 */
MOB_INDEX_DATA *get_mob_index(int vnum)
{
	MOB_INDEX_DATA *pMobIndex;

	if (vnum <= 0)
		return NULL;

	for (pMobIndex = mob_index_hash[vnum % MAX_KEY_HASH];
	     pMobIndex; pMobIndex = pMobIndex->next)
		if (pMobIndex->vnum == vnum)
			return pMobIndex;

	if (fBootDb)
		db_error("get_mob_index", "bad vnum %d.", vnum);
		
	return NULL;
}

/*
 * Translates mob virtual number to its obj index struct.
 * Hash table lookup.
 */
OBJ_INDEX_DATA *get_obj_index(int vnum)
{
	OBJ_INDEX_DATA *pObjIndex;

	if (vnum <= 0)
		return NULL;

	for (pObjIndex = obj_index_hash[vnum % MAX_KEY_HASH];
	     pObjIndex; pObjIndex = pObjIndex->next)
		if (pObjIndex->vnum == vnum)
			return pObjIndex;

	if (fBootDb)
		db_error("get_obj_index", "bad vnum %d.", vnum);
		
	return NULL;
}

/*
 * Translates mob virtual number to its room index struct.
 * Hash table lookup.
 */
ROOM_INDEX_DATA *get_room_index(int vnum)
{
	ROOM_INDEX_DATA *pRoomIndex;

	if (vnum <= 0)
		return NULL;

	for (pRoomIndex = room_index_hash[vnum % MAX_KEY_HASH];
	     pRoomIndex; pRoomIndex = pRoomIndex->next)
		if (pRoomIndex->vnum == vnum)
			return pRoomIndex;

	if (fBootDb)
		db_error("get_room_index", "bad vnum %d.", vnum);

	return NULL;
}

void *alloc_mem(int sMem)
{
	void *tmp;
	
	tmp = calloc(1, sMem);
	if (tmp)
		return tmp;
	log_printf("Cann't alloc memory (alloc_mem) for DB muddy.");
	crush_mud();
	return NULL;
}

void free_mem(void *p, int sMem)
{
	free(p);
}

void *alloc_perm(int sMem)
{
	void *tmp;
	
	tmp = calloc(1, sMem);
	if (tmp)
		return tmp;
	log_printf("Cann't alloc memory (alloc_perm) for DB muddy.");
	crush_mud();
	return NULL;
}

void do_areas(CHAR_DATA *ch, const char *argument)
{
	AREA_DATA *pArea1;
	AREA_DATA *pArea2;
	int iArea;
	int iAreaHalf;
	BUFFER *output;

	if (argument[0] != '\0') {
		char_puts("No argument is used with this command.\n",ch);
		return;
	}

	iAreaHalf = (top_area + 1) / 2;
	pArea1    = area_first;
	pArea2    = area_first;
	for (iArea = 0; iArea < iAreaHalf; iArea++)
		pArea2 = pArea2->next;

	output = buf_new(-1);
	buf_add(output, "Current areas of Shadow Realms: \n");
	for (iArea = 0; iArea < iAreaHalf; iArea++) {
		buf_printf(output,"{{%2d %3d} {B%-20.20s{x %8.8s ",
			pArea1->min_level,pArea1->max_level,
			pArea1->name,
			pArea1->credits);

		if (pArea2 != NULL) 
			buf_printf(output,"{{%2d %3d} {B%-20.20s{x %8.8s",
				pArea2->min_level,pArea2->max_level,
				pArea2->name,
				pArea2->credits);
		buf_add(output, "\n");

		pArea1 = pArea1->next;
		if (pArea2 != NULL)
			pArea2 = pArea2->next;
	}

	buf_add(output,"\n");	
	page_to_char(buf_string(output), ch);	
	buf_free(output);
}

void do_memory(CHAR_DATA *ch, const char *argument)
{
	extern int mlstr_count;
	extern int mlstr_real_count;
	extern int str_count;
	extern int str_real_count;

	char_printf(ch, "Affects  : %5d [%d]\n", top_affect, sizeof(AFFECT_DATA));
	char_printf(ch, "Areas    : %5d [%d]\n", top_area, sizeof(AREA_DATA));
	char_printf(ch, "ExDes    : %5d [%d]\n", top_ed, sizeof(ED_DATA));
	char_printf(ch, "Exits    : %5d [%d]\n", top_exit, sizeof(EXIT_DATA));
	char_printf(ch, "Helps    : %5d [%d]\n", top_help, sizeof(HELP_DATA));
	char_printf(ch, "Socials  : %5d [%d]\n", socials.nused, sizeof(social_t));
	char_printf(ch, "Mob idx  : %d (%d old, max vnum %d) [%d]\n",
		    top_mob_index, top_mob_index - newmobs, top_vnum_mob,
		    sizeof(MOB_INDEX_DATA));
	char_printf(ch, "Mobs     : %d (%d free) [%d]\n",
		    mob_count, mob_free_count, sizeof(CHAR_DATA));
	char_printf(ch, "Obj idx  : %d (%d old, max vnum %d) [%d]\n",
		    top_obj_index, top_obj_index - newobjs, top_vnum_obj,
		    sizeof(OBJ_INDEX_DATA));
	char_printf(ch, "Objs     : %d (%d free) [%d]\n",
		    obj_count, obj_free_count, sizeof(OBJ_DATA));
	char_printf(ch, "Resets   : %d [%d]\n", top_reset, sizeof(RESET_DATA));
	char_printf(ch, "Rooms    : %d (max vnum %d) [%d]\n",
		    top_room, top_vnum_room, sizeof(ROOM_INDEX_DATA));
	char_printf(ch, "Shops    : %d [%d]\n", top_shop, sizeof(SHOP_DATA));
	char_printf(ch, "Buffers  : %d (%d bytes) [~20]\n",
					nAllocBuf, sAllocBuf /*,
					sizeof(struct buf_data) */);
	char_printf(ch, "strings  : %d (%d allocated) [%d]\n",
			str_count, str_real_count,
			/* sizeof(str) */
			sizeof(int) + sizeof(void *) * 2);
	char_printf(ch, "mlstrings: %d (%d allocated) [~12]\n",
			mlstr_count, mlstr_real_count /*,
			sizeof(struct mlstring)  */);
	char_printf(ch, "PCDATA = %d; RELIGION_DATA = %d; DESCRIPTOR_DATA = %d\n",
		sizeof(PC_DATA), sizeof(RELIGION_DATA),
		sizeof(DESCRIPTOR_DATA_MUDDY));
}

void do_dump(CHAR_DATA *ch, const char *argument)
{
	int count,count2,num_pcs,aff_count;
	CHAR_DATA *fch;
	MOB_INDEX_DATA *pMobIndex;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *pObjIndex;
	ROOM_INDEX_DATA *room;
	EXIT_DATA *exit;
	PC_DATA *pc;
	DESCRIPTOR_DATA_MUDDY *d;
	AFFECT_DATA *af;
	FILE *fp;
	int vnum,nMatch = 0;

	if ((fp = dfopen(TMP_PATH, "mem.dmp", "w")) == NULL)
		return;

	/* report use of data structures */
	
	num_pcs = 0;
	aff_count = 0;

	/* mobile prototypes */
	fprintf(fp,"MobProt	%4d (%8d bytes)\n",
		top_mob_index, top_mob_index * (sizeof(*pMobIndex))); 

	/* mobs */
	count = 0;
	for (fch = char_list; fch != NULL; fch = fch->next)
	{
		count++;
		if (fch->pcdata != NULL)
		    num_pcs++;
		for (af = fch->affected; af != NULL; af = af->next)
		    aff_count++;
	}

	fprintf(fp,"Mobs	%4d (%8d bytes)\n",
		count, count * (sizeof(*fch)));

	fprintf(fp,"Pcdata	%4d (%8d bytes)\n",
		num_pcs, num_pcs * (sizeof(*pc)));

	/* descriptors */
	count = 0; count2 = 0;

	for (d = descriptor_list_muddy; d; d = d->next)
		count++;
//	for (d= descriptor_free; d != NULL; d = d->next)
//		count2++;

	fprintf(fp, "Descs	%4d (%8d bytes), ??? %2d free (%d bytes) ???\n",
		count, count * (sizeof(*d)), count2, count2 * (sizeof(*d)));

	/* object prototypes */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    for (af = pObjIndex->affected; af != NULL; af = af->next)
			aff_count++;
		    nMatch++;
		}

	fprintf(fp,"ObjProt	%4d (%8d bytes)\n",
		top_obj_index, top_obj_index * (sizeof(*pObjIndex)));

	/* objects */
	count = 0;
	for (obj = object_list; obj != NULL; obj = obj->next) {
		count++;
		for (af = obj->affected; af != NULL; af = af->next)
		    aff_count++;
	}

	fprintf(fp,"Objs	%4d (%8d bytes)\n",
		count, count * (sizeof(*obj)));

	/* affects */
	fprintf(fp,"Affects	%4d (%8d bytes)\n",
		aff_count, aff_count * (sizeof(*af)));

	/* rooms */
	fprintf(fp,"Rooms	%4d (%8d bytes)\n",
		top_room, top_room * (sizeof(*room)));

	 /* exits */
	fprintf(fp,"Exits	%4d (%8d bytes)\n",
		top_exit, top_exit * (sizeof(*exit)));

	fclose(fp);

	/* start printing out mobile data */
	if ((fp = dfopen(TMP_PATH, "mob.dmp", "w")) == NULL)
		return;

	fprintf(fp,"\nMobile Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d killed     %s\n",
			pMobIndex->vnum,pMobIndex->count,
			pMobIndex->killed,mlstr_mval(pMobIndex->short_descr));
		}
	fclose(fp);

	/* start printing out object data */
	if ((fp = dfopen(TMP_PATH, "obj.dmp", "w")) == NULL)
		return;

	fprintf(fp,"\nObject Analysis\n");
	fprintf(fp,  "---------------\n");
	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL)
		{
		    nMatch++;
		    fprintf(fp,"#%-4d %3d active %3d reset      %s\n",
			pObjIndex->vnum,pObjIndex->count,
			pObjIndex->reset_num,
			mlstr_mval(pObjIndex->short_descr));
		}

	/* close file */
	fclose(fp);
}

/*
 * Stick a little fuzz on a number.
 */
int number_fuzzy(int number)
{
	switch (number_bits(2))
	{
	case 0:  number -= 1; break;
	case 3:  number += 1; break;
	}

	return UMAX(1, number);
}

/*
 * Generate a random number.
 */
int number_range(int from, int to)
{
	int power;
	int number;

	if (from == 0 && to == 0)
		return 0;

	if ((to = to - from + 1) <= 1)
		return from;

	for (power = 2; power < to; power <<= 1)
		;

	while ((number = number_mm() & (power -1)) >= to)
		;

	return from + number;
}

/*
 * Generate a percentile roll.
 */
int number_percent(void)
{
	int percent;

	while ((percent = number_mm() & (128-1)) > 99)
		;

	return 1 + percent;
}

/*
 * Generate a random door.
 */
int number_door(void)
{
	int door;

	while ((door = number_mm() & (8-1)) > 5)
		;

	return door;
}

int number_bits(int width)
{
	return number_mm() & ((1 << width) - 1);
}

/*
 * I've gotten too many bad reports on OS-supplied random number generators.
 * This is the Mitchell-Moore algorithm from Knuth Volume II.
 * Best to leave the constants alone unless you've read Knuth.
 * -- Furey
 */

/* I noticed streaking with this random number generator, so I switched
	back to the system srandom call.  If this doesn't work for you, 
	define OLD_RAND to use the old system -- Alander */

#if defined (OLD_RAND)
static  int     rgiState[2+55];
#endif
 
void init_mm()
{
#if defined (OLD_RAND)
	int *piState;
	int iState;
 
	piState     = &rgiState[2];
 
	piState[-2] = 55 - 55;
	piState[-1] = 55 - 24;
 
	piState[0]  = ((int) current_time) & ((1 << 30) - 1);
	piState[1]  = 1;
	for (iState = 2; iState < 55; iState++)
	{
		piState[iState] = (piState[iState-1] + piState[iState-2])
		                & ((1 << 30) - 1);
	}
#else
	srandom(time(NULL)^getpid());
#endif
	return;
}
 
long number_mm(void)
{
#if defined (OLD_RAND)
	int *piState;
	int iState1;
	int iState2;
	int iRand;
 
	piState             = &rgiState[2];
	iState1             = piState[-2];
	iState2             = piState[-1];
	iRand               = (piState[iState1] + piState[iState2])
		                & ((1 << 30) - 1);
	piState[iState1]    = iRand;
	if (++iState1 == 55)
		iState1 = 0;
	if (++iState2 == 55)
		iState2 = 0;
	piState[-2]         = iState1;
	piState[-1]         = iState2;
	return iRand >> 6;
#else
	return random() >> 6;
#endif
}

/*
 * Roll some dice.
 */
int dice(int number, int size)
{
	int idice;
	int sum;

	switch (size)
	{
	case 0: return 0;
	case 1: return number;
	}

	for (idice = 0, sum = 0; idice < number; idice++)
		sum += number_range(1, size);

	return sum;
}

/*
 * Simple linear interpolation.
 */
int interpolate(int level, int value_00, int value_32)
{
	return value_00 + level * (value_32 - value_00) / 32;
}

/*
 * Append a string to a file.
 */
void append_file(CHAR_DATA *ch, const char *file, const char *str)
{
	FILE *fp;

	if (IS_NPC(ch) || str[0] == '\0')
		return;

	if ((fp = dfopen(TMP_PATH, file, "a")) == NULL)
		char_puts("Could not open the file!\n", ch);
	else {
		fprintf(fp, "[%5d] %s: %s\n",
		    ch->in_room ? ch->in_room->vnum : 0, ch->name, str);
		fclose(fp);
	}
}

/*
 * This function is here to aid in debugging.
 * If the last expression in a function is another function call,
 *   gcc likes to generate a JMP instead of a CALL.
 * This is called "tail chaining."
 * It hoses the debugger call stack for that call.
 * So I make this the last call in certain critical functions,
 *   where I really need the call stack to be right for debugging!
 *
 * If you don't understand this, then LEAVE IT ALONE.
 * Don't remove any calls to tail_chain anywhere.
 *
 * -- Furey
 */
void tail_chain(void)
{
	return;
}

FILE *deb_file = NULL;
extern int debug_level;		//comm.c

void init_debug_log(void)
{
	if (debug_level > 0)
	{
		deb_file = dfopen(TMP_PATH, DEBUG_LOG, "w");
		if (!deb_file)
		{
			log("Can't init debug file :(");
			debug_level = 0;
		} else {
			fclose(deb_file);
		}
	}
}

void done_debug_log(void)
{
//	if (deb_file)
//		fclose(deb_file);
}

void debug_printf(int level, const char *str)
{
	if (debug_level
	&& level <= debug_level)
	{
		deb_file = dfopen(TMP_PATH, DEBUG_LOG, "w");
		// fseek(deb_file, 0, SEEK_SET);
		fprintf(deb_file, "%s\n", str);
		// fflush(deb_file);
		fclose(deb_file); 
	}
	return;
}

/*
 * Add the objects in players not logged on to object count 
 */

void load_limited_objects()
{
	struct dirent *dp;
	DIR *dirp;
	FILE *pfile;

//	DESCRIPTOR_DATA_MUDDY *desc;
	CHAR_DATA *ch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	struct stat stat_pfile;
	char name_file[MAX_STRING_LENGTH];
	time_t time_update, time_char, local_time;
	int dt, chance;
	bool remove_limits = TRUE;
	bool save_char;
	
	if ((dirp = opendir(PLAYER_PATH)) == NULL) {
		bug("Load_limited_objects: unable to open player directory.",
		    0);
		exit(1);
	}
	
	log_printf("Search limits & create PC_rating :");
	snprintf(name_file, sizeof(name_file), "%s/%s", TMP_PATH, UPDATE_CHAR_FILE);
	local_time = time(NULL);
	if (!stat(name_file, &stat_pfile) && (dt = difftime(local_time, stat_pfile.st_mtime)) < (60 * 60 * 24)){
		time_update = stat_pfile.st_mtime;
		log_printf("Limits modify today (%i hour)", dt / 3600);
		remove_limits = FALSE;
	} else {
		FILE *fp;
		dunlink(TMP_PATH, UPDATE_CHAR_FILE);
		fp = dfopen(TMP_PATH, UPDATE_CHAR_FILE, "w");
		if (!fp) {
			log_printf("Error for create %s: %s.",
				UPDATE_CHAR_FILE, strerror(errno));
			exit(1);
		}
		fclose(fp);
		log_printf("%s has created(modifed): %s", UPDATE_CHAR_FILE, strtime(time(NULL)));
		time_update = local_time;
	}
//	desc = new_descriptor_muddy();
	for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {

#if defined (LINUX) || defined (WIN32)
		if (strlen(dp->d_name) < 3
		|| dp->d_name[0] == '.')
			continue;
#else
/*		if (dp->d_namlen < 3 || dp->d_type != DT_REG)
			continue;

*/
		if (strlen(dp->d_name) < 3
		|| dp->d_name[0] == '.')
			continue;
#endif
		if ((pfile = dfopen(PLAYER_PATH, dp->d_name, "r")) == NULL)
			continue;
		fclose(pfile);

		snprintf(name_file, sizeof(name_file), "%s/%s", PLAYER_PATH, dp->d_name);
		if (!stat(name_file, &stat_pfile)){
			time_char = stat_pfile.st_mtime;
		} else {
			log_printf("Don't open %s pfile !", name_file);
			continue;
		}
		
		if (time_char && time_update){
			dt = UMAX(0, difftime(time_update, time_char) / (60 * 60));
		} else
			dt = 0;
		chance = 1;
		chance += (dt * 10) / 24;

		//log_printf("loaded %s", dp->d_name);
		//load_char_obj(desc, dp->d_name);

		ch = load_char_obj(NULL, dp->d_name);
		save_char = FALSE;
		//ch = desc->character;
		//ch->desc = NULL;
		
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			
			obj->pIndexData->count++; // <--- This is very IMPORTANT !!!
			if (obj->pIndexData
			 && obj->pIndexData->limit != -1
			 && ((number_percent() < chance && remove_limits)
			    || obj->pIndexData->count > obj->pIndexData->limit)
			   ) {
				extract_obj(obj, 0);
				save_char = TRUE;
				continue;
			}
		}
		
		if ((ch->pcdata->perm_hit > ch->level * 31 + 40
		|| ch->pcdata->perm_mana > ch->level * 51 + 150
		|| ch->pcdata->perm_move > ch->level * 15 + 100)
		&& !IS_IMMORTAL(ch))
			log_printf("Warning! %s may be cheater !!!", ch->name);
/*
		{
			AFFECT_DATA	*paf, *paf_next;
			OBJ_DATA	*obj;
			OBJ_DATA	*obj_next;
			
			for (obj = ch->carrying; obj != NULL; obj = obj_next) {
				obj_next = obj->next_content;
				extract_obj(obj, 0);
			}

			save_char = TRUE;
			
			ch->affected_by = 0;
			ch->imm_flags = 0;
			ch->res_flags = 0;
			ch->vuln_flags = 0;
			race_change_check(ch, race_lookup(0),
				race_lookup(ch->pcdata->race));
			
			for (paf = ch->affected; paf; paf = paf_next) {
				paf_next = paf->next;
				affect_remove(ch, paf);
			}
			while ((paf = ch->affected)) {
				ch->affected	= paf->next;
				aff_free(paf);
			}
			reset_char(ch);
		}
*/

		//save_char = TRUE;		// for convert pfiles;
		if (save_char){
			log_printf("%s modified (%d%% chance).", ch->name, chance);
			save_char_obj(ch, FALSE);
		}
		
		rating_add(ch->name, ch->pcdata->pc_killed);
		
		free_char(ch);
	}
	closedir(dirp);
//	free_descriptor_muddy(desc);
}

//#define NBUF 5
//#define NBITS 52

#if 0
/*****************************************************************************
 Name:	        convert_objects
 Purpose:	Converts all old format objects to new format
 Called by:	boot_db (db.c).
 ****************************************************************************/
void convert_objects(void)
{
	int i;
	if (newobjs == top_obj_index)
		return; /* all objects in new format */

	for (i = 0; i < MAX_KEY_HASH; i++) {
		OBJ_INDEX_DATA *pObj;

		for (pObj = obj_index_hash[i]; pObj; pObj = pObj->next)
 			if (IS_SET(pObj->extra_flags, ITEM_OLDSTYLE))
				convert_object(pObj);
	}

}

/*****************************************************************************
 Name:		convert_object
 Purpose:	Converts an ITEM_OLDSTYLE obj to new format
 Called by:	convert_objects (db2.c).
 Note:          Dug out of create_obj (db.c)
 Author:        Hugin
 ****************************************************************************/

void convert_object(OBJ_INDEX_DATA *pObjIndex)
{
    int level;
    int number, type;  /* for dice-conversion */

    level = pObjIndex->level;

    pObjIndex->cost     = 10*level;

    switch (pObjIndex->item_type) {
        default:
            bug("Obj_convert: vnum %d bad type.", pObjIndex->item_type);
            break;

        case ITEM_LIGHT:
        case ITEM_TREASURE:
        case ITEM_FURNITURE:
        case ITEM_TRASH:
        case ITEM_CONTAINER:
        case ITEM_DRINK_CON:
        case ITEM_KEY:
        case ITEM_FOOD:
        case ITEM_BOAT:
        case ITEM_CORPSE_NPC:
        case ITEM_CORPSE_PC:
        case ITEM_FOUNTAIN:
        case ITEM_MAP:
        case ITEM_CLOTHING:
        case ITEM_SCROLL:
        case ITEM_SCALP_CORPSE_NPC:
        case ITEM_SCALP_CORPSE_PC:
	    break;

        case ITEM_WAND:
        case ITEM_STAFF:
            pObjIndex->value[2] = pObjIndex->value[1];
	    break;

        case ITEM_WEAPON:

	    /*
	     * The conversion below is based on the values generated
	     * in one_hit() (fight.c).  Since I don't want a lvl 50 
	     * weapon to do 15d3 damage, the min value will be below
	     * the one in one_hit, and to make up for it, I've made 
	     * the max value higher.
	     * (I don't want 15d2 because this will hardly ever roll
	     * 15 or 30, it will only roll damage close to 23.
	     * I can't do 4d8+11, because one_hit there is no dice-
	     * bounus value to set...)
	     *
	     * The conversion below gives:

	     level:   dice      min      max      mean
	       1:     1d8      1(2)    8(7)     5(5)
	       2:     2d5      2(3)   10(8)     6(6)
	       3:     2d5      2(3)   10(8)     6(6)
	       5:     2d6      2(3)   12(10)     7(7)
	      10:     4d5      4(5)   20(14)    12(10)
	      20:     5d5      5(7)   25(21)    15(14)
	      30:     5d7      5(10)   35(29)    20(20)
	      50:     5d11     5(15)   55(44)    30(30)

	     */

	    number = UMIN(level/4 + 1, 5);
	    type   = (level + 7)/number;

            pObjIndex->value[1] = number;
            pObjIndex->value[2] = type;
	    break;

        case ITEM_ARMOR:
            pObjIndex->value[0] = level / 5 + 3;
            pObjIndex->value[1] = pObjIndex->value[0];
            pObjIndex->value[2] = pObjIndex->value[0];
	    break;

        case ITEM_POTION:
        case ITEM_PILL:
            break;

        case ITEM_MONEY:
	    pObjIndex->value[0] = pObjIndex->cost;
	    break;
    }

    REMOVE_BIT(pObjIndex->extra_flags, ITEM_OLDSTYLE);
    touch_vnum(pObjIndex->vnum);
    ++newobjs;
}
#endif

void *fread_namedp(namedp_t *table, FILE *fp)
{
	char *name = fread_word(fp);
	namedp_t *np = namedp_lookup(table, name);

	if (np == NULL)
		db_error("fread_namedp", "%s: unknown named pointer", name);

	np->touched = TRUE;
	return np->p;
}

/*
int fread_clan(FILE *fp)
{
	int cln;
	const char *name;

	name = fread_string(fp);
	cln = cln_lookup(name);
	if (cln < 0) {
		db_error("fread_clan", "%s: unknown clan", name);
		cln = 0;
	}
	free_string(name);
	return cln;
}
*/

