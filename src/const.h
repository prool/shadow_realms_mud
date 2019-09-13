/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*-
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: const.h,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#ifndef _CONST_H_
#define _CONST_H_

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */

#if defined (WIN32)
#	include <stdlib.h>
#	define PATH_MAX	_MAX_PATH
#	define PATH_SEPARATOR '\\'
#else
#	define PATH_SEPARATOR '/'
#endif

#define TMP_PATH	"tmp"
#define PLAYER_PATH	"player"
#define GODS_PATH	"gods"
#define NOTES_PATH	"notes"
#define MSGDB_PATH	"msgdb"
#define ETC_PATH	"etc"
#define RACES_PATH	"races"
#define CLASSES_PATH	"classes"
#define CLANS_PATH	"clans"
#define AREA_PATH	"area"
#define LANG_PATH	"lang"
#define LOG_PATH	"log"

#define TMP_FILE	"romtmp"
#define UPDATE_CHAR_FILE "limit_update.tmp"
#define	TIME_FILE	"time_sr.mud"
#define PID_FILE	"muddy.pid"

#if defined (WIN32)
#	define NULL_FILE	"NUL"	/* To reserve one stream */
#	define PLISTS_PATH	CLANS_PATH"\\plists"
#else
#	define NULL_FILE	"/dev/null"	/* To reserve one stream */
#	define PLISTS_PATH	CLANS_PATH"/plists"
#endif

#define HOMETOWNS_CONF	"hometowns.conf"/* hometowns */
#define SKILLS_CONF	"skills.conf"	/* skills */
#define SOCIALS_CONF	"socials.conf"	/* socials */
#define SYSTEM_CONF	"system.conf"	/* system configuration */
#define LANG_CONF	"lang.conf"	/* lang definitions */
#define RELIGION_CONF	"religion.conf"	/* religion definition */
#define MSG_FILE	"msgdb.txt"	/* msg db */
#define DAMAGES_FILE	"damages.txt"	/* file with multi-text damages */

#define AREA_LIST	"area.lst"	/* list of areas */
#define CLAN_LIST	"clan.lst"	/* list of clans */
#define CLASS_LIST	"class.lst"	/* list of classes */
#define LANG_LIST	"lang.lst"	/* list of languages */
#define RACE_LIST	"race.lst"

#define BUG_FILE	"bugs.txt"	/* For 'bug' and bug()*/
#define TYPO_FILE	"typos.txt"	/* For 'typo'*/
#define NOTE_FILE	"notes.not"	/* For 'notes'*/
#define IDEA_FILE	"ideas.not"
#define PENALTY_FILE	"penal.not"
#define NEWS_FILE	"news.not"
#define STORY_FILE	"story.not"
#define CHANGES_FILE	"chang.not"
#define SHUTDOWN_FILE	"shutdown"	/* For 'shutdown'*/
#define BAN_FILE	"ban.txt"
#define MAXON_FILE	"maxon.txt"
#define AREASTAT_FILE	"areastat.txt"
#define IMMLOG_FILE	"immortals.log"
#define DEBUG_LOG	"debug.log"
#define DENY_FILE	"deny.list"

/* align numbers */
enum {
	ANUM_GOOD,
	ANUM_NEUTRAL,
	ANUM_EVIL,

	MAX_ANUM
};

enum {
	STAT_STR,
	STAT_INT,
	STAT_WIS,
	STAT_DEX,
	STAT_CON,
	STAT_CHA,

	MAX_STAT			/* number of char stats */
};

#define MAX_STATS	MAX_STAT	/* ROM compatibility */

#define TIME_HOUR_OF_DAY	24
#define TIME_DAY_OF_MONTH	48
#define TIME_MONTH_OF_YEAR	12

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		1024
#define MAX_STRING_HASH		16384

//#define MAX_STRING_LENGTH	8192
//#define MAX_INPUT_LENGTH	1024

#define MAX_PROMPT_LENGTH	60
#define MAX_TITLE_LENGTH	45
#define MAX_CHAR_NAME		12
#define PAGELEN 		22
#define SCROLL_MIN		16
#define SCROLL_MAX		192

#define MAX_LIST_OBJ	500 /* use in act_info.c -> show_list_to_char */
/*
 * Game parameters.
 */
//#define MAX_RELIGION		18
#define MIN_PK_LEVEL		10	/* min PK level */
#define MAX_NEWBIES		120	/* number of newbies allowed */
#define MAX_OLDIES		999	/* number of newbies allowed */
#define MAX_TRADE		5	/* number of trade types for shops */
#define MAX_COND		6	/* number of char condition stats */
#define MAX_DIR			6	/* number of exits */
#define MAX_ALIAS		50	/* number of aliases char can have */
#define RATING_TABLE_SIZE	20	/* rating table size */
#define MAX_LEVEL		101

#define LEVEL_HERO		(MAX_LEVEL - 11)
#define LEVEL_LEGEND		(MAX_LEVEL - 10)
#define LEVEL_IMMORTAL		(MAX_LEVEL - 9)
#define LEVEL_NEWBIE		5

#define HIGH			MAX_LEVEL
#define IMPLEMENTOR		(MAX_LEVEL - 1)
#define CREATOR 		(MAX_LEVEL - 2)
#define SUPREME 		(MAX_LEVEL - 3)
#define DEITY			(MAX_LEVEL - 4)
#define GOD			(MAX_LEVEL - 5)
#define IMMORTAL		(MAX_LEVEL - 6)
#define DEMI			(MAX_LEVEL - 7)
#define ANGEL			(MAX_LEVEL - 8)
#define AVATAR			(MAX_LEVEL - 9)
#define HERO			LEVEL_HERO
#define LEGEND			LEVEL_LEGEND

#define ML	MAX_LEVEL
#define L1 	(MAX_LEVEL - 1)	/* implementor */
#define L2	(MAX_LEVEL - 2) /* creator */
#define L3	(MAX_LEVEL - 3)	/* supreme being */
#define L4	(MAX_LEVEL - 4)	/* deity */
#define L5 	(MAX_LEVEL - 5)	/* god */
#define L6	(MAX_LEVEL - 6)	/* immortal */
#define L7	(MAX_LEVEL - 7)	/* demigod */
#define L8	(MAX_LEVEL - 8)	/* angel */
#define L9	(MAX_LEVEL - 9)	/* avatar */
#define IM	LEVEL_IMMORTAL 	/* angel */
#define HE	LEVEL_HERO	/* hero */

#undef ANATOLIA_MACHINE

#if defined(ANATOLIA_MACHINE)
#	define PULSE_PER_SCD		6  /* 6 for comm.c */
#	define PULSE_PER_SECOND		4  /* for update.c */
#	define PULSE_VIOLENCE		(2 *  PULSE_PER_SECOND)
#else
#	define PULSE_PER_SCD		4
#	define PULSE_PER_SECOND		4
#	define PULSE_VIOLENCE		(3 * PULSE_PER_SECOND)
#endif

#define PULSE_MOBILE		(4 * PULSE_PER_SECOND)
#define PULSE_WATER_FLOAT	(4 * PULSE_PER_SECOND)
#define PULSE_MUSIC		(6 * PULSE_PER_SECOND)
#define PULSE_TRACK		(12 * PULSE_PER_SECOND)
#define PULSE_TICK		(50 * PULSE_PER_SECOND) /* 36 saniye */

/* room_affect_update (not room_update) */
#define PULSE_RAFFECT		(3 * PULSE_MOBILE)
#define PULSE_AREA		(110 * PULSE_PER_SECOND) /* 97 saniye */
#define PULSE_AUCTION		(20 * PULSE_PER_SECOND) /* 20 seconds */

#define FIGHT_DELAY_TIME	(20 * PULSE_PER_SECOND)
#define GHOST_DELAY_TIME	600	/* In real seconds */
#define GHOST_DELAY_PLEVEL	5
#define MISSING_TARGET_DELAY	10

/*
 * Well known mob virtual numbers.
 * Defined in #MOBILES.
 */
#define MOB_VNUM_KITTEN			3090
#define MOB_VNUM_NEWB_GUILDMASTER	3719

#define MOB_VNUM_SHADOW 		10
#define MOB_VNUM_SPECIAL_GUARD		11
#define MOB_VNUM_BEAR			12
#define MOB_VNUM_DEMON			13
#define MOB_VNUM_NIGHTWALKER		14
#define MOB_VNUM_STALKER		15
#define MOB_VNUM_SQUIRE 		16
#define MOB_VNUM_MIRROR_IMAGE		17
#define MOB_VNUM_UNDEAD 		18
#define MOB_VNUM_LION			19
#define MOB_VNUM_WOLF			20
#define MOB_VNUM_SUM_SHADOW		26

#define MOB_VNUM_LESSER_GOLEM		21
#define MOB_VNUM_STONE_GOLEM		22
#define MOB_VNUM_IRON_GOLEM		23
#define MOB_VNUM_ADAMANTITE_GOLEM	24

#define MOB_VNUM_HUNTER 		25

#define MOB_VNUM_PATROLMAN		2106
#define GROUP_VNUM_TROLLS		2100
#define GROUP_VNUM_OGRES		2101

/*
 * Well known object virtual numbers.
 * Defined in #OBJECTS.
 */
#define OBJ_VNUM_SILVER_ONE		1
#define OBJ_VNUM_GOLD_ONE		2
#define OBJ_VNUM_GOLD_SOME		3
#define OBJ_VNUM_SILVER_SOME		4
#define OBJ_VNUM_COINS			5

#define OBJ_VNUM_CROSS			8
#define OBJ_VNUM_CORPSE_NPC		10
#define OBJ_VNUM_CORPSE_PC		11
#define OBJ_VNUM_SEVERED_HEAD		12
#define OBJ_VNUM_TORN_HEART		13
#define OBJ_VNUM_SLICED_ARM		14
#define OBJ_VNUM_SLICED_LEG		15
#define OBJ_VNUM_GUTS			16
#define OBJ_VNUM_BRAINS			17
#define OBJ_VNUM_SCALP_CORPSE_NPC       18
#define OBJ_VNUM_SCALP_CORPSE_PC        19
#define OBJ_VNUM_SCALP                  46

#define OBJ_VNUM_MUSHROOM		20
#define OBJ_VNUM_LIGHT_BALL		21
#define OBJ_VNUM_SPRING 		22
#define OBJ_VNUM_DISC			23
#define OBJ_VNUM_EMPTY_TICKET		24
#define OBJ_VNUM_PORTAL 		25
#define OBJ_VNUM_DUMMY			30
#define OBJ_VNUM_ROSE			1001

#define OBJ_VNUM_SCHOOL_VEST		3703
#define OBJ_VNUM_SCHOOL_SHIELD		3704
#define OBJ_VNUM_SCHOOL_BANNER		3716

#define OBJ_VNUM_MAP			3162
#define OBJ_VNUM_NMAP1			3385
#define OBJ_VNUM_NMAP2			3386

#define OBJ_VNUM_WHISTLE		2116

#define OBJ_VNUM_POTION_VIAL		42
#define OBJ_VNUM_STEAK			27
#define OBJ_VNUM_RANGER_STAFF		28
#define OBJ_VNUM_WOODEN_ARROW		6
#define OBJ_VNUM_GREEN_ARROW		34
#define OBJ_VNUM_RED_ARROW		35
#define OBJ_VNUM_WHITE_ARROW		36
#define OBJ_VNUM_BLUE_ARROW		37
#define OBJ_VNUM_RANGER_BOW		7

#define OBJ_VNUM_DEPUTY_BADGE		70
#define OBJ_VNUM_RULER_BADGE		70
#define OBJ_VNUM_RULER_SHIELD1		71
#define OBJ_VNUM_RULER_SHIELD2		72
#define OBJ_VNUM_RULER_SHIELD3		73
#define OBJ_VNUM_RULER_SHIELD4		74

#define OBJ_VNUM_CHAOS_BLADE		97

#define OBJ_VNUM_DRAGONDAGGER		80
#define OBJ_VNUM_DRAGONMACE		81
#define OBJ_VNUM_PLATE			82
#define OBJ_VNUM_DRAGONSWORD		83
#define OBJ_VNUM_DRAGONLANCE		99

#define OBJ_VNUM_BATTLE_PONCHO		26

#define OBJ_VNUM_POTION_SILVER		43
#define OBJ_VNUM_POTION_GOLDEN		44
#define OBJ_VNUM_POTION_SWIRLING	45
#define OBJ_VNUM_KATANA_SWORD		98

#define OBJ_VNUM_EYED_SWORD		88
#define OBJ_VNUM_FIRE_SHIELD		92
#define OBJ_VNUM_MAGIC_JAR		93
#define OBJ_VNUM_HAMMER 		6522

#define OBJ_VNUM_CHUNK_IRON		6521

/*
 * Well known room virtual numbers.
 * Defined in #ROOMS.
 */

#define ROOM_VNUM_VOID		1
#define ROOM_VNUM_LIMBO 	2
#define ROOM_VNUM_REMORT	3
#define ROOM_VNUM_CHAT		1200
#define ROOM_VNUM_TEMPLE	3001
#define ROOM_VNUM_SCHOOL	3700

#endif

