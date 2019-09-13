/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: interp.c,v 1.39 2009/08/12 14:14:10 mudsr Exp $
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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <stdarg.h>

#include "merc.h"
#include "interp.h"
#include "olc.h"
#include "cmd.h"
#include "socials.h"

#undef IMMORTALS_LOGS

void interpret_social(social_t *soc, CHAR_DATA *ch, const char *argument);
//char    *get_multi_command      args((DESCRIPTOR_DATA_MUDDY *d, char *argument, bool addincomm));
char *get_multi_command( DESCRIPTOR_DATA_MUDDY *d, char *argument, bool addincomm);

/*
 * Command logging types.
 */
#define LOG_NORMAL	0
#define LOG_ALWAYS	1
#define LOG_NEVER	2

/*
 * Log-all switch.
 */
bool				fLogAll		= FALSE;

#ifdef IMMORTALS_LOGS
/*
 * immortals log file
 */
FILE				*imm_log;
#endif

/*
 * Command table.
 */
cmd_t real_cmd_table[] =
{
    /*
     * Mob command interpreter (placed here for faster scan...)
     */
    { "mob",		do_mob,		POS_DEAD,	 0,  LOG_NEVER, CMD_NOORDER },

    /*
     * Common movement commands.
     */
    { "north",		do_north,	POS_STANDING,    0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN | CMD_GHOST},
    { "east",		do_east,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN | CMD_GHOST},
    { "south",		do_south,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN | CMD_GHOST},
    { "west",		do_west,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN | CMD_GHOST},
    { "up",		do_up,		POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN | CMD_GHOST},
    { "down",		do_down,	POS_STANDING,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_HIDDEN | CMD_GHOST},

	/*
	 * Often command.
	 */
    { "-",		do_repeat,	POS_SLEEPING,	 0,  LOG_NEVER, CMD_CHARMED_OK | CMD_GHOST},
    { "reply",		do_reply,	POS_SLEEPING,	 0,  LOG_NEVER, CMD_CHARMED_OK | CMD_GHOST},
    { "bash",		do_bash,	POS_FIGHTING,    0,  LOG_NORMAL},

    /*
     * Common other commands.
     * Placed here so one and two letter abbreviations work.
     */
    { "at",             do_at,          POS_DEAD,       L6,  LOG_NORMAL, CMD_KEEP_HIDE  },
    { "cast",		do_cast,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_NOLOG_MOB 	},
    { "auction",        do_auction,     POS_SLEEPING,    0,  LOG_NORMAL	},
    { "buy",		do_buy,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "chat",           do_chat,        POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "dual",		do_second_wield,POS_RESTING,	 0,  LOG_NORMAL },
    { "exits",		do_exits,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "estimate",	do_estimate,	POS_RESTING,	 0,  LOG_NORMAL },
    { "get",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "open",		do_open,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "flee",		do_flee,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "goto",           do_goto,        POS_DEAD,       IM,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "glist",          do_glist,       POS_DEAD,        0,  LOG_NEVER, CMD_GHOST},
    { "group",          do_group,       POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "hit",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "inventory",	do_inventory,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "kill",		do_kill,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "look",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "order",		do_order,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "practice",       do_practice,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "research",	do_research,	POS_RESTING,	 0,  LOG_NORMAL, CMD_NOORDER },
    { "rest",		do_rest,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "repair",		do_repair,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "label",		do_label,	POS_RESTING,	 0,  LOG_NORMAL, CMD_NOORDER},
    { "second",		do_second_wield,POS_RESTING,	 0,  LOG_NORMAL	},
    { "sit",		do_sit,		POS_SLEEPING,    0,  LOG_NORMAL	},
    { "smithing",	do_smithing,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "sockets",        do_sockets,	POS_DEAD,       L4,  LOG_NORMAL, CMD_KEEP_HIDE},
    { "stand",		do_stand,	POS_SLEEPING,	 0,  LOG_NORMAL	, CMD_GHOST},
    { "tell",		do_tell,	POS_SLEEPING,	 0,  LOG_NEVER, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "unlock",         do_unlock,      POS_RESTING,     0,  LOG_NORMAL	},
    { "wear",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "wield",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "wizhelp",	do_wizhelp,	POS_DEAD,	IM,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "repeat",		do_repeat,	POS_SLEEPING,	 0,  LOG_NEVER, CMD_CHARMED_OK | CMD_GHOST},
    /*
     * Informational commands.
     */
    { "affects",	do_affects,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "afk",		do_afk,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "areas",		do_areas,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "balance",	do_balance,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "bug",		do_bug,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_FROZEN_OK | CMD_GHOST},
    { "changes",	do_changes,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_FROZEN_OK | CMD_GHOST},
    { "commands",	do_commands,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "compare",	do_compare,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "compress",	do_compress,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST | CMD_FROZEN_OK | CMD_NOORDER},
    { "consider",	do_consider,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "concentrate",	do_concentrate,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "count",		do_count,	POS_SLEEPING,	HE,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "craft",		do_craft,	POS_STANDING,	 0,  LOG_NORMAL },
    { "credits",	do_credits,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "date",		do_date,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "deposit",	do_deposit,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "equipment",	do_equipment,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "escape",		do_escape,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "examine",	do_examine,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_GHOST}, 
    { "help",		do_help,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "ideas",		do_idea,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
    { "news",		do_news,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
    { "story",		do_story,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
    { "raffects",	do_raffects,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "rating",		do_rating,	POS_DEAD,	HE,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "read",		do_look,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "report",		do_report,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK |  CMD_GHOST},
    { "rules",		do_rules,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "scan",           do_scan,        POS_RESTING,     0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "oscore",		do_oscore,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "skills",		do_skills,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "speak",		do_speak,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "socials",	do_socials,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "spells",		do_spells,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "time",		do_time,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "typo",		do_typo,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "weather",	do_weather,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "who",		do_who,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "whois",		do_whois,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "withdraw",	do_withdraw,	POS_STANDING,    0,  LOG_NORMAL },
    { "wizlist",	do_wizlist,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "worth",		do_worth,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "ownlist",	do_ownlist,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},

    { "train",		do_train,	POS_RESTING,	 0,  LOG_NORMAL	},
    /*
     * Communication commands.
     */
    { "bearcall",       do_bear_call,   POS_FIGHTING,    0,  LOG_NORMAL	},
//    { "clan",		do_clan,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
//    { "=",		do_clan,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
//    { "clanlist",	do_clanlist,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK },
//    { "clanrecall",	do_crecall,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "emote",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "pmote",		do_pmote,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
    { ",",		do_emote,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "gtell",		do_gtell,	POS_DEAD,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { ";",		do_gtell,	POS_DEAD,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_CHARMED_OK },
    { "notes",		do_note,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
//    { "petitio",	do_petitio,	POS_DEAD,	 0,  LOG_NORMAL, CMD_CHARMED_OK },
//    { "petition",	do_petition,	POS_DEAD,	 0,  LOG_NORMAL, CMD_NOORDER },
    { "pose",		do_pose,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_GHOST},
//    { "promote",	do_promote,	POS_DEAD,	 0,  LOG_NORMAL	},
    { "pray",           do_pray,        POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
    { "say",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "whisper",	do_whisper,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "replay",		do_replay,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "'",		do_say,		POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "shout",		do_shout,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { ".",		do_shout,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "music",		do_music,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},
    { "gossip",		do_gossip,	POS_DEAD,	 3,  LOG_NORMAL, CMD_CHARMED_OK | CMD_GHOST},
    { "motd",		do_motd,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "racetable",	do_racetable,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "classtable",	do_classtable,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "wake",		do_wake,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE},
    { "warcry",		do_warcry,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "unread",		do_unread,	POS_SLEEPING,    0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
    { "yell",		do_yell,	POS_RESTING,	 0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_NOLOG_MOB | CMD_GHOST},

    { "wanted",		do_wanted,	POS_RESTING,	 0,  LOG_ALWAYS, CMD_NOORDER},
    { "judge",		do_judge,	POS_RESTING,	 0,  LOG_NORMAL, CMD_NOORDER | CMD_CHARMED_OK},

    /*
     * Configuration commands.
     */
    { "alia",		do_alia,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "alias",		do_alias,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_GHOST},
    { "attacker",       do_attacker,    POS_STANDING,    0,  LOG_NORMAL, CMD_NOORDER },
    { "clear",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "cls",		do_clear,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autolist",	do_autolist,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autoassist",	do_autoassist,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "autoexit",	do_autoexit,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autogold",	do_autogold,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autolook",	do_autolook,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autoloot",	do_autoloot,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autosac",	do_autosac,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autosplit",	do_autosplit,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "autoscalp",      do_autoscalp,   POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "description",	do_description,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "delet",		do_delet,	POS_DEAD,	 0,  LOG_ALWAYS, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
    { "delete",		do_delete,	POS_DEAD,	 0,  LOG_ALWAYS, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_FROZEN_OK | CMD_GHOST},
    { "identify",	do_identify,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "nofollow",	do_nofollow,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "mentalblock",	do_mentalblock,	POS_DEAD,        0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_NOORDER },
    { "config",		do_config,	POS_DEAD,	 0,  LOG_NEVER, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_GHOST},
    { "outfit",		do_outfit,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "tick",		do_tick,	POS_DEAD,	ML,  LOG_ALWAYS, CMD_KEEP_HIDE },
    { "quest",          do_quest,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_NOORDER },
    { "qui",		do_qui,		POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_FROZEN_OK | CMD_GHOST},
    { "quit",		do_quit,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_NOORDER | CMD_FROZEN_OK | CMD_GHOST},
    { "reincarnation",	do_reincarnation, POS_DEAD,	 0,  LOG_NORMAL, CMD_NOORDER | CMD_GHOST },
    { "religion",	do_religion,	POS_DEAD,	 0,  LOG_NORMAL, CMD_NOORDER | CMD_GHOST },
    { "quiet",		do_quiet,	POS_SLEEPING, 	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_GHOST},
    { "title",		do_title,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_GHOST},
    { "unalias",	do_unalias,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_GHOST},
    { "wimpy",		do_wimpy,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_NOORDER | CMD_GHOST},


    /*
     * Miscellaneous commands.
     */
    { "endure",         do_endure,      POS_STANDING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "enter", 		do_enter, 	POS_STANDING,	 0,  LOG_NORMAL, CMD_GHOST },
    { "follow",		do_follow,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "gain",		do_gain,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "go",		do_enter,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN | CMD_GHOST},
    { "fade",		do_fade,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "herbs",          do_herbs,       POS_STANDING,    0,  LOG_NORMAL	},
    { "aid",		do_aid,		POS_STANDING,    0,  LOG_NORMAL },
    { "hara",           do_hara,        POS_STANDING,    0,  LOG_NORMAL	},

    { "hide",		do_hide,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "hometown",	do_hometown,	POS_STANDING,	 0,  LOG_NORMAL },
    { "homepoint",	do_homepoint,	POS_STANDING,	 0,  LOG_NORMAL },
    { "human",          do_human,       POS_STANDING,    0,  LOG_NORMAL	},
    { "hunt",           do_hunt,        POS_STANDING,    0,  LOG_NORMAL	},
    { "leave", 		do_enter, 	POS_STANDING,	 0,  LOG_NORMAL	| CMD_GHOST},
    { "rent",		do_rent,	POS_DEAD,	 0,  LOG_NORMAL, CMD_HIDDEN | CMD_GHOST},
    { "save",		do_save,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "sleep",		do_sleep,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST},
    { "sneak",		do_sneak,	POS_STANDING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "split",		do_split,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "steal",		do_steal,	POS_STANDING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },

    { "visible",	do_visible,	POS_SLEEPING,	 0,  LOG_NORMAL	},
    { "where",		do_where,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_GHOST },
//    { "clanitem",	do_clanitem,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE|CMD_CHARMED_OK },

    /*
     * Object manipulation commands.
     */
    { "brandish",	do_brandish,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "butcher",        do_butcher,     POS_STANDING,    0,  LOG_NORMAL	},
    { "scalp",		do_scalp,       POS_STANDING,	 0,  LOG_NORMAL },
    { "crucify",	do_crucify,	POS_STANDING,	 0,  LOG_NORMAL },
    { "close",		do_close,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "detect",         do_detect_hidden,POS_RESTING,    0,  LOG_NORMAL	},
    { "drink",		do_drink,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "drop",		do_drop,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "eat",		do_eat,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "enchant",	do_enchant, 	POS_RESTING,     0,  LOG_NORMAL	},
    { "envenom",	do_envenom,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "fill",		do_fill,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "fly",		do_fly,		POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "give",		do_give,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "heal",		do_heal,	POS_RESTING,	 0,  LOG_NORMAL	}, 
    { "hold",		do_wear,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "layhands",	do_layhands,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "list",		do_list,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "lock",		do_lock,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "lore",           do_lore,        POS_RESTING,     0,  LOG_NORMAL	},

    { "pick",		do_pick,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "pour",		do_pour,	POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "put",		do_put,		POS_RESTING,	 0,  LOG_NORMAL	},
    { "quaff",		do_quaff,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "recite",		do_recite,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "remove",		do_remove,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "request",        do_request,     POS_STANDING,    0,  LOG_NORMAL	},
    { "sell",		do_sell,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "take",		do_get,		POS_RESTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "sacrifice",	do_sacrifice,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "junk",           do_sacrifice,   POS_RESTING,     0,  LOG_NORMAL, CMD_HIDDEN },
    { "trophy",         do_trophy,      POS_STANDING,    0,  LOG_NORMAL	},
    { "value",		do_value,	POS_RESTING,	 0,  LOG_NORMAL	},
    { "show",		do_show,	POS_RESTING,	0,  LOG_NORMAL },
    { "zap",		do_zap,		POS_RESTING,	0,  LOG_NORMAL	},
    { "search",		do_search,	POS_RESTING,	0,	LOG_NORMAL,	CMD_CHARMED_OK },
    { "dig",		do_dig,		POS_STANDING,	0,	LOG_NORMAL,	CMD_CHARMED_OK },
    { "bury",		do_bury,	POS_STANDING,	0,	LOG_NORMAL,	CMD_CHARMED_OK },
    { "recall",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_NOORDER},
    { "/",		do_recall,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_HIDDEN | CMD_NOORDER | CMD_GHOST},
    { "remort",		do_remort,	POS_RESTING,	 0,  LOG_NORMAL, CMD_NOORDER },

    /*
     * Combat commands.
     */
    { "ambush",         do_ambush,      POS_STANDING,    0,  LOG_NORMAL },
    { "assassinate",    do_assassinate, POS_STANDING,    0,  LOG_NORMAL },
    { "backstab",	do_backstab,	POS_STANDING,	 0,  LOG_NORMAL	},

    { "bashdoor",	do_bash_door,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "bs",		do_backstab,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "bite",		do_vbite,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "blindness",	do_blindness_dust,POS_FIGHTING,	 0,  LOG_ALWAYS	},
    { "touch",		do_vtouch,	POS_STANDING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "berserk",	do_berserk,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "bloodthirst",	do_bloodthirst,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "blackjack",	do_blackjack,	POS_STANDING,	 0,  LOG_NORMAL },
    { "caltrops",       do_caltrops,    POS_FIGHTING,    0,  LOG_NORMAL	},
    { "explode",	do_explode, 	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "camouflage",     do_camouflage,  POS_STANDING,    0,  LOG_NORMAL	},
    { "circle",         do_circle,      POS_FIGHTING,    0,  LOG_NORMAL	},
    { "cleave",         do_cleave,      POS_STANDING,    0,  LOG_NORMAL },

    { "dirt",		do_dirt,	POS_FIGHTING,	 0,  LOG_NORMAL },
    { "disarm",		do_disarm,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "dishonor",	do_dishonor,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "dismount",       do_dismount,    POS_STANDING,    0,  LOG_NORMAL	},

    { "guard",          do_guard,       POS_STANDING,    0,  LOG_NORMAL	},

    { "kick",		do_kick,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "lioncall",       do_lion_call,   POS_FIGHTING,    0,  LOG_NORMAL	},
    { "make",           do_make,        POS_STANDING,    0,  LOG_NORMAL	},
    { "mount",          do_mount,       POS_STANDING,    0,  LOG_NORMAL	},
    { "saddle",		do_saddle,	POS_STANDING,	 0,  LOG_NORMAL },
    { "teth",		do_teth,	POS_STANDING,	 0,  LOG_NORMAL },
    { "unteth",		do_unteth,	POS_STANDING,	 0,  LOG_NORMAL },
    { "stable",		do_stable,	POS_STANDING,	 0,  LOG_NORMAL },
    { "murde",		do_murde,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_HIDDEN },
    { "murder",		do_murder,	POS_FIGHTING,	 0,  LOG_ALWAYS },
    { "nerve",          do_nerve,       POS_FIGHTING,    0,  LOG_NORMAL	},
    { "poison",		do_poison_smoke,POS_FIGHTING,	 0,  LOG_ALWAYS	},
    { "rescue",		do_rescue,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "resistance",     do_resistance,  POS_FIGHTING,    0,  LOG_NORMAL	},
    { "truesight",      do_truesight,   POS_FIGHTING,    0,  LOG_NORMAL	},
    { "thumbling",	do_thumbling,	POS_FIGHTING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "forest",		do_forest,	POS_FIGHTING,	 0,  LOG_NORMAL },
    { "shield",		do_shield,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "strangle",       do_strangle,    POS_STANDING,    0,  LOG_NORMAL },
    { "surrender",	do_surrender,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "tame",           do_tame,        POS_FIGHTING,    0,  LOG_NORMAL	},
    { "throw",          do_throw,       POS_FIGHTING,    0,  LOG_NORMAL	},
    { "tiger",		do_tiger,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "trip",		do_trip,	POS_FIGHTING,    0,  LOG_NORMAL },
    { "target",		do_target,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "vampire",	do_vampire,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "vanish",		do_vanish,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "weapon",		do_weapon,	POS_FIGHTING,	 0,  LOG_NORMAL	},
    { "blink",		do_blink,	POS_FIGHTING,    0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "knife", 		do_knife,	POS_STANDING,	 0,  LOG_NORMAL },
//    { "mark",		do_mark,	POS_STANDING,	 0,  LOG_NORMAL	},


    /*
     * Immortal commands.
     */
    { "advance",	do_advance,	POS_DEAD,	ML,  LOG_ALWAYS	}, 
    { "set",		do_set,		POS_DEAD,	L1,  LOG_ALWAYS	},
    { "add",		do_add,		POS_DEAD,	L3,  LOG_ALWAYS },
    { "rp",		do_rp,		POS_DEAD,	IM,  LOG_ALWAYS },
    { "dump",		do_dump,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "rename",		do_rename,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "violate",	do_violate,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "track",          do_track,       POS_STANDING,    0,  LOG_NORMAL	},

    { "allow",		do_allow,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "ban",		do_ban,		POS_DEAD,	L2,  LOG_ALWAYS	},
    { "deny",		do_deny,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "delchar",	do_delchar,	POS_DEAD,	L1,  LOG_ALWAYS },
    { "dename",		do_denyname,	POS_DEAD,	L1,  LOG_ALWAYS },
    { "disconnect",	do_disconnect,	POS_DEAD,	L3,  LOG_ALWAYS	},
    { "freeze",		do_freeze,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "permban",	do_permban,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "protect",	do_protect,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "reboo",		do_reboo,	POS_DEAD,	L1,  LOG_NEVER, CMD_HIDDEN	},
    { "reboot",		do_reboot,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "smite",		do_smite,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "limited",	do_limited,	POS_DEAD,	L2,  LOG_NEVER	},
    { "popularity",	do_popularity,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "shutdow",	do_shutdow,	POS_DEAD,	L1,  LOG_NEVER, CMD_HIDDEN	},
    { "shutdown",	do_shutdown,	POS_DEAD,	L1,  LOG_ALWAYS	},
    { "wizlock",	do_wizlock,	POS_DEAD,	L2,  LOG_ALWAYS	},
    { "affrooms",	do_affrooms,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "force",		do_force,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "load",		do_load,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "newlock",	do_newlock,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "noaffect",	do_noaffect,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "nochannels",	do_nochannels,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "notitle",	do_notitle,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "nochat",         do_nochat,      POS_DEAD,       L6,  LOG_ALWAYS, CMD_KEEP_HIDE },
    { "noemote",	do_noemote,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "noshout",	do_noshout,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "notell",		do_notell,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "pecho",		do_pecho,	POS_DEAD,	L4,  LOG_ALWAYS	}, 
    { "purge",		do_purge,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "restore",	do_restore,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "sla",		do_sla,		POS_DEAD,	L3,  LOG_NEVER	},
    { "slay",		do_slay,	POS_DEAD,	L3,  LOG_ALWAYS	},
    { "nonote",		do_nonote,	POS_DEAD,	L5,  LOG_ALWAYS },
    { "teleport",	do_transfer,    POS_DEAD,	L5,  LOG_ALWAYS	},	
    { "transfer",	do_transfer,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "poofin",		do_bamfin,	POS_DEAD,	L9,  LOG_NORMAL	},
    { "poofout",	do_bamfout,	POS_DEAD,	L9,  LOG_NORMAL	},
    { "gecho",		do_echo,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { ">",		do_echo,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "holylight",	do_holylight,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "incognito",	do_incognito,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "invis",		do_invis,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "log",		do_log,		POS_DEAD,	L1,  LOG_ALWAYS	},
    { "memory",		do_memory,	POS_DEAD,	IM,  LOG_NEVER	},
    { "mwhere",		do_mwhere,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "owhere",		do_owhere,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "peace",		do_peace,	POS_DEAD,	L5,  LOG_NORMAL	},
    { "penalty",	do_penalty,	POS_DEAD,	L7,  LOG_NORMAL	},
    { "penalties",	do_penalty,	POS_DEAD,	L7,  LOG_NORMAL	},
    { "echo",		do_recho,	POS_DEAD,	L6,  LOG_ALWAYS	},
    { "return",         do_return,      POS_DEAD,       0,  LOG_NORMAL, CMD_CHARMED_OK | CMD_HIDDEN },
    { "snoop",		do_snoop,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "stat",		do_stat,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "string",		do_string,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "switch",		do_switch,	POS_DEAD,	L6,  LOG_ALWAYS	},
    { "wizinvis",	do_invis,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "vnum",		do_vnum,	POS_DEAD,	L4,  LOG_NORMAL	},
    { "zecho",		do_zecho,	POS_DEAD,	L4,  LOG_ALWAYS	},
    { "clone",		do_clone,	POS_DEAD,	L5,  LOG_ALWAYS	},
    { "wiznet",		do_wiznet,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "check",		do_check,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "immtalk",	do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "imotd",          do_imotd,       POS_DEAD,       IM,  LOG_NORMAL	},
    { ":",		do_immtalk,	POS_DEAD,	IM,  LOG_NORMAL	},
//	{ "smote",		do_smote,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "prefi",		do_prefi,	POS_DEAD,	IM,  LOG_NORMAL, CMD_HIDDEN },
    { "prefix",		do_prefix,	POS_DEAD,	IM,  LOG_NORMAL	},
    { "objlist",	do_objlist,	POS_DEAD,	ML,  LOG_NORMAL	},
    { "world",		do_world,	POS_DEAD,	L4,	LOG_NORMAL },

    { "settraps",	do_settraps,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "slook",		do_slook,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "mschool",	do_mschool,	POS_SLEEPING,	 0,  LOG_NORMAL, CMD_KEEP_HIDE },
    { "learn",		do_learn,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "teach",		do_teach,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "camp",		do_camp,  	POS_STANDING,    0,  LOG_NORMAL	},
    { "tail",		do_tail,	POS_FIGHTING,    0,  LOG_NORMAL	},
    { "push",		do_push,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "drag",		do_drag,	POS_STANDING,	 0,  LOG_NORMAL	},
    { "demand",         do_demand,      POS_STANDING,    0,  LOG_NORMAL	},
    { "bandage",        do_bandage,     POS_FIGHTING,    0,  LOG_NORMAL	},
    { "shoot",          do_shoot,       POS_FIGHTING,    0,  LOG_NORMAL	},
    { "charge",		do_charge,	POS_STANDING,	 0,  LOG_NORMAL },

    { "find",		do_find,	POS_DEAD,	ML,  LOG_ALWAYS	},
    { "score",		do_score,	POS_DEAD,	 0,  LOG_NORMAL, CMD_KEEP_HIDE | CMD_CHARMED_OK | CMD_GHOST},
    { "katana",		do_katana, 	POS_STANDING,    0,  LOG_NORMAL	},
    { "control",       	do_control,	POS_STANDING,    0,  LOG_NORMAL	},
    { "ititle",		do_ititle,	POS_DEAD,	L8,  LOG_NORMAL },
    { "sense",          do_sense,       POS_RESTING,     0,  LOG_NORMAL	},

    { "mpstat",		do_mpstat,	POS_DEAD,	IM,  LOG_NEVER	},


    { "msgstat",	do_msgstat,	POS_DEAD,	IM,  LOG_NEVER	},
    { "strstat",	do_strstat,	POS_DEAD,	IM,  LOG_NEVER	},

    { "grant",		do_grant,	POS_DEAD,	ML,	LOG_ALWAYS },
    { "disable",	do_disable,	POS_DEAD,	ML,	LOG_ALWAYS },
    { "enable",		do_enable,	POS_DEAD,	ML,	LOG_ALWAYS },

    /*
     * OLC
     */
    { "edit",		do_edit,	POS_DEAD,   L2,  LOG_ALWAYS	},
    { "create",		do_create,	POS_DEAD,   L2,  LOG_ALWAYS	},
    { "asave",          do_asave,	POS_DEAD,   L2,  LOG_ALWAYS	},
    { "alist",		do_alist,	POS_DEAD,   IM,  LOG_NEVER	},
    { "ashow",		do_ashow,	POS_DEAD,   IM,  LOG_NEVER	},
    { "resets",		do_resets,	POS_DEAD,   L2,  LOG_NORMAL	},
    /*
     *	Other commands
     */
    { "version",	do_version,	POS_DEAD,    0,  LOG_NEVER, CMD_GHOST},
    /*
     * End of list.
     */
     { NULL }
};

cmd_t *cmd_table = real_cmd_table;

void interpret(CHAR_DATA *ch, const char *argument)
{
	interpret_raw(ch, argument, FALSE);
}

/*
 * The main entry point for executing commands.
 * Can be recursively called from 'at', 'order', 'force'.
 */
void interpret_raw(CHAR_DATA *ch, const char *argument, bool is_order)
{
	char command[MAX_INPUT_LENGTH];
	const char *logline;
	cmd_t *cmd;
	bool found;
	social_t *soc = NULL;
	int min_pos;
	flag64_t cmd_flags;

	/*
	 * Strip leading spaces.
	 */
	while (isspace(*argument))
		argument++;

	if (argument[0] == '\0')
		return;

	logline = argument;

	/*
	 * Grab the command word.
	 * Special parsing so ' can be a command,
	 * also no spaces needed after punctuation.
	 */

#ifdef IMMORTALS_LOGS
 	if (IS_IMMORTAL(ch)) {
		if ((imm_log = dfopen(GODS_PATH, IMMLOG_FILE, "a+"))) {
			fprintf(imm_log, "%s [%s] %s\n",
				strtime(time(NULL)), ch->name, logline);
			fprintf(imm_log, buf);
			fclose(imm_log);
		}
	}
#endif

	if (!isalpha(argument[0]) && !isdigit(argument[0])) {
		command[0] = argument[0];
		command[1] = '\0';
		argument++;
		while (isspace(*argument))
			argument++;
	} else
		argument = one_argument(argument, command, sizeof(command));

	debug_printf(7, "interpret_raw[0]");
	/*
	 * Look for command in command table.
	 */
	found = FALSE;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (str_prefix(command, cmd->name))
			continue;

		/*
		 * Implement freeze command.
		 */
		if (!IS_NPC(ch)
		&&  IS_SET(ch->pcdata->plr_flags, PLR_FREEZE)
		&&  !IS_SET(cmd->flags, CMD_FROZEN_OK)) {
			char_puts("You're totally frozen!\n", ch);
			return;
		}

		if (IS_SET(cmd->flags, CMD_DISABLED)) {
			char_puts("Sorry, this command is temporarily disabled.\n", ch);
			return;
		}

		if (cmd->level >= LEVEL_IMMORTAL) {
			if (IS_NPC(ch))
				return;

			if (ch->level < HIGH
			&&  !is_name(cmd->name, ch->pcdata->granted))
				continue;
		}
		else if (cmd->level > ch->level)
			return;

		if (IS_GHOST(ch)
		&& !IS_SET(cmd->flags, CMD_GHOST)
		&& !IS_IMMORTAL(ch))
		{
			char_puts("How can you do it if you are spirit now ?!\n", ch);
			return;
		}

		if (is_order) {
			if (IS_SET(cmd->flags, CMD_NOORDER)
			||  cmd->level >= LEVEL_IMMORTAL)
				return;
		}
		else {
			if (IS_AFFECTED(ch, AFF_CHARM)
			&&  !IS_SET(cmd->flags, CMD_CHARMED_OK)
			&&  cmd->level < LEVEL_IMMORTAL 
			&&  !IS_IMMORTAL(ch)) {
				char_puts("First ask your beloved master!\n",
					  ch);
				return;
			}
		}

		if (IS_AFFECTED(ch, AFF_STUN) 
		&&  !(cmd->flags & CMD_KEEP_HIDE)
		&& number_percent() > get_curr_stat(ch, STAT_CON)) {
			char_puts("You are STUNNED to do that.\n", ch);
			return;
		}

		found = TRUE;
		break;
	}
	
	debug_printf(7, "interpret_raw[1]");

	if (!found) {
		/*
		 * Look for command in socials table.
		 */
		if ((soc = social_lookup(command, str_prefix)) == NULL) {
			char_puts("Huh?\n", ch);
			return;
		}

		if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
			char_puts("You are anti-social!\n", ch);
			return;
		}

		min_pos = soc->min_pos;
		cmd_flags = 0;
	}
	else {
		min_pos = cmd->position;
		cmd_flags = cmd->flags;
	}

	/*
	 * Log and snoop.
	 */
	if (cmd->log == LOG_NEVER
	|| ((IS_SET(cmd->flags, CMD_NOLOG_MOB) || soc) && IS_NPC(ch))
	|| !found)
		logline = str_empty;

	if (((!IS_NPC(ch) && IS_SET(ch->pcdata->plr_flags, PLR_LOG))
	||   fLogAll
	||   cmd->log == LOG_ALWAYS) && logline[0] != '\0' 
	&&   logline[0] != '\n') {
		log_printf("Log %s: %s", ch->name, logline);
		wiznet("Log $N: $t", ch, logline, WIZ_SECURE, 0, ch->level);
	}

	if (ch->desc && ch->desc->snoop_by) {
		write_to_buffer_muddy(ch->desc->snoop_by, "# ", 2);
		write_to_buffer_muddy(ch->desc->snoop_by, logline, 0);
		write_to_buffer_muddy(ch->desc->snoop_by, "\n\r", 2);
	}


	if (!IS_NPC(ch)) {
		/* Come out of hiding for most commands */
		if (IS_AFFECTED(ch, AFF_HIDE | AFF_FADE)
		&&  !IS_SET(cmd_flags, CMD_KEEP_HIDE)) {
			REMOVE_BIT(ch->affected_by, AFF_HIDE | AFF_FADE);
			act_puts("You step out of shadows.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n steps out of shadows.",
			    ch, NULL, NULL, TO_ROOM);
		}

		if (IS_AFFECTED(ch, AFF_IMP_INVIS)
		&&  min_pos == POS_FIGHTING) {
			affect_bit_strip(ch, TO_AFFECTS, AFF_IMP_INVIS);
			act_puts("You fade into existence.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n fades into existence.",
			    ch, NULL, NULL, TO_ROOM);
		}
	}

	/*
	 * Character not in position for command?
	 */
	if (ch->position < min_pos) {
		switch(ch->position) {
			case POS_DEAD:
				act("Lie still; You are DEAD.\n", ch, NULL, NULL, TO_CHAR);
				break;

			case POS_MORTAL:
			case POS_INCAP:
				act_puts("You are hurt far too bad "
					  "for that.\n", ch, NULL, NULL, TO_CHAR, POS_DEAD);
				break;

			case POS_STUNNED:
				act_puts("You are too stunned to do that.\n",
					  ch, NULL, NULL, TO_CHAR, POS_DEAD);
				break;

			case POS_SLEEPING:
				act_puts("In your dreams, or what?\n",
					ch, NULL, NULL, TO_CHAR, POS_SLEEPING);
				break;

			case POS_RESTING:
				act("Nah... You feel too relaxed...\n",
					  ch, NULL, NULL, TO_CHAR);
				break;

			case POS_SITTING:
				act("Better stand up first.\n", ch, NULL, NULL, TO_CHAR);
				break;

			case POS_FIGHTING:
				act("No way! You are still fighting!\n",
					  ch, NULL, NULL, TO_CHAR);
				break;

		}
		return;
	}

	if (soc) {
		interpret_social(soc, ch, argument);
		return;
	}
	
	debug_printf(7, "interpret_raw[call]");

	/*
	 * Dispatch the command.
	 */
	cmd->do_fun(ch, argument);
	
	debug_printf(7, "interpret_raw[done]");

	tail_chain();
}

void interpret_social(social_t *soc, CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	ROOM_INDEX_DATA *victim_room;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		act_puts3_mlstr(soc->noarg_char, ch, NULL, NULL, NULL, TO_CHAR, POS_RESTING);
		act_puts3_mlstr(soc->noarg_room,
		    ch, NULL, NULL, NULL, TO_ROOM | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL
	||  (IS_NPC(victim) && victim->in_room != ch->in_room)) {
		act_puts3_mlstr(soc->notfound_char, ch, NULL, NULL, NULL, TO_CHAR, POS_RESTING);
		return;
	}

	if (victim == ch) {
		act_puts3_mlstr(soc->self_char, ch, NULL, victim, NULL, TO_CHAR, POS_RESTING);
		act_puts3_mlstr(soc->self_room,
		    ch, NULL, victim, NULL, TO_ROOM | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
		return;
	}

	victim_room = victim->in_room;
	victim->in_room = ch->in_room;

	act_puts3_mlstr(soc->found_char, ch, NULL, victim, NULL, TO_CHAR, POS_RESTING);
	act_puts3_mlstr(soc->found_vict,
	    ch, NULL, victim, NULL, TO_VICT | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
	act_puts3_mlstr(soc->found_notvict,
	    ch, NULL, victim, NULL, TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);

	victim->in_room = victim_room;

	if (!IS_NPC(ch) && IS_NPC(victim) 
	&&  !IS_AFFECTED(victim, AFF_CHARM)
	&&  IS_AWAKE(victim) && !victim->desc) {
		switch (number_bits(4)) {
			case 0:

			case 1: case 2: case 3: case 4:
			case 5: case 6: case 7: case 8:
				act_puts3_mlstr(soc->found_char,
				    victim, NULL, ch, NULL, TO_CHAR, POS_RESTING);
				act_puts3_mlstr(soc->found_vict,
				    victim, NULL, ch, NULL, TO_VICT | ACT_TOBUF, POS_RESTING);
				act_puts3_mlstr(soc->found_notvict,
				    victim, NULL, ch, NULL,
				    TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT, POS_RESTING);
				break;

			case 9: case 10: case 11: case 12:
				act("$n slaps $N.", victim, NULL, ch, 
				    TO_NOTVICT | ACT_TOBUF | ACT_NOTWIT);
				act("You slap $N.", victim, NULL, ch, TO_CHAR);
				act("$n slaps you.", victim, NULL, ch, 
				    TO_VICT | ACT_TOBUF);
				break;
		}
	}
}

/*
 * Contributed by Alander.
 */
void do_commands(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	int col;
 
	col = 0;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (cmd->level < LEVEL_HERO
		&&  cmd->level <= ch->level 
		&&  !IS_SET(cmd->flags, CMD_HIDDEN)) {
			char_printf(ch, "%-12s", cmd->name);
			if (++col % 6 == 0)
				char_puts("\n", ch);
		}
	}
 
	if (col % 6 != 0)
		char_puts("\n", ch);
}

void do_wizhelp(CHAR_DATA *ch, const char *argument)
{
	cmd_t *cmd;
	int col;
 
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	col = 0;
	for (cmd = cmd_table; cmd->name; cmd++) {
		if (cmd->level < LEVEL_IMMORTAL)
			continue;

		if (ch->level < IMPLEMENTOR
		&&  !is_name(cmd->name, ch->pcdata->granted))
			continue;

		char_printf(ch, "%-12s", cmd->name);
		if (++col % 6 == 0)
			char_puts("\n", ch);
	}
 
	if (col % 6 != 0)
		char_puts("\n", ch);
}

/*********** alias.c **************/

/* does aliasing and other fun stuff */

void substitute_alias(DESCRIPTOR_DATA_MUDDY *d, const char *argument)
{                                                                                      
    CHAR_DATA *ch;                                                                     
	char	buf[MAX_STRING_LENGTH],
		prefix[MAX_INPUT_LENGTH],
		name[MAX_INPUT_LENGTH];
	const char *point[9];
	int alias = 0;
	const char *str;
	int i, counter, counter2, tmp;

//	ch = d->original ? d->original : d->character;	
	ch = d->character;

	/* check for prefix */
	if (IS_PC(ch) && ch->pcdata->prefix[0] != '\0' && str_prefix("prefix", argument))
	{
		if (strlen(ch->pcdata->prefix) + strlen(argument) >= MAX_INPUT_LENGTH - 1)
			char_puts("Line to long, prefix not processed.\n",ch);
		else
		{
			snprintf(prefix, sizeof(prefix), "%s %s", ch->pcdata->prefix, argument);
			argument = prefix;
		}
	}

	if (!IS_PC(ch)
	||  ch->pcdata->alias[0] == NULL
	||  !str_prefix("alias",argument) || !str_prefix("unalias",argument)
	||  !str_prefix("prefix",argument)) {
		interpret(ch, argument);
		return;
	}

	strnzcpy(buf, sizeof(buf),argument);
	debug_printf(6, "substitute_alias[0]");

	if (ch->pcdata->alias[alias] != NULL)
	{	
		for (alias = 0; alias < MAX_ALIAS; alias++)  /* go through the aliases */
			if (!str_prefix(ch->pcdata->alias[alias], argument))
			{
				point[0] = one_argument(argument, name, sizeof(name));

				if (!strcmp(ch->pcdata->alias[alias], name))
				{
					buf[0] = '\0';
					str = ch->pcdata->alias_sub[alias];
					for (i = 0 ; i < 9; i++) {
						if (strlen(point[i]) > 0) {
							point[i+1] = one_argument(point[i], name, sizeof(name));
						} else {
							point[i]++;
							point[i+1] = NULL;
							break;
						}
					}
					for (counter = 0, counter2 = 0; str[counter] != '\0'; counter++) {
						if (str[counter] == '%') {
							counter++;
							if(str[counter] == '%') {
								buf[counter2] = str[counter];
								counter2++;
								continue;
							}
							for (i = 0 ; i < 9 ; i++) {
									// непродуманная конструкция (не проще проще проверить соотв-ий point на NULL, превдарительно заполнив NULL'ями ?!!!)
								if (point [i+1] == NULL)
									break;
								else if (str[counter] == i+49) {
									tmp = point[i+1]-point[i]-1;
									buf[counter2] = '\0';
									strncat(buf, point[i], tmp);
									counter2 += tmp;
									break;
								}
							}
						} else {
							buf[counter2] = str[counter];
							counter2++;
						}
					}
					strnzcpy (buf, sizeof(buf), get_multi_command(d, buf, TRUE));
					break;
				}

				if (strlen(buf) >= MAX_INPUT_LENGTH-1)
				{
					char_puts("Alias substitution too long. Truncated.\n",ch);
					buf[MAX_INPUT_LENGTH -1] = '\0';
				}
			}
	}
	debug_printf(6, "substitute_alias[1]");
	interpret(ch, buf);
}

void do_alia(CHAR_DATA *ch, const char *argument)
{
	char_puts("I'm sorry, alias must be entered in full.\n",ch);
    return;
}

/* !!! */
void do_alias(CHAR_DATA *ch, const char *argument)                                        
{                                                                                         
    CHAR_DATA *rch;                                                                       
    char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];                                    
    int pos;                                                                              
                                                                                          
    if (ch->desc == NULL)                                                                 
        rch = ch;                                                                         
    else                                                                                  
        rch = ch->desc->original ? ch->desc->original : ch;                               
                                                                                          
    if (IS_NPC(rch))                                                                      
        return;                                                                           

    argument = one_argument(argument, arg, sizeof(arg));
                                                                                          
    if (arg[0] == '\0')                                                                   
    {                                                                                     
        if (rch->pcdata->alias[0] == NULL)                                                
        {                                                                                 
            char_puts("You have no aliases defined.\n",ch);                               
            return;                                                                       
        }                                                                                 
        char_puts("Your current aliases are:\n",ch);                                      
                                                                                          
        for (pos = 0; pos < MAX_ALIAS; pos++)                                             
        {                                                                                 
            if (rch->pcdata->alias[pos] == NULL                                           
            ||  rch->pcdata->alias_sub[pos] == NULL)                                      
                break;
                                                                                          
            char_printf(ch,"    %s:  %s\n",rch->pcdata->alias[pos],                       
                    rch->pcdata->alias_sub[pos]);                                         
		}
        return;                                                                           
    }                                                                                     
                                                                                          
    if (!str_prefix("una",arg) || !str_cmp("alias",arg))                                  
    {                                                                                     
        char_puts("Sorry, that word is reserved.\n",ch);                                  
        return;                                                                           
    }                                                                                     
                                                                                          
    if (argument[0] == '\0')                                                              
    {                                                                                     
        for (pos = 0; pos < MAX_ALIAS; pos++)                                             
        {                                                                                 
            if (rch->pcdata->alias[pos] == NULL                                           
            ||  rch->pcdata->alias_sub[pos] == NULL)                                      
                break;                                                                    
                                                                                          
            if (!str_cmp(arg,rch->pcdata->alias[pos]))                                    
            {                                                                             
				snprintf(buf, sizeof(buf), "%s aliases to '%s'.\n",rch->pcdata->alias[pos],
                        rch->pcdata->alias_sub[pos]);                                     
                char_puts(buf,ch);                                                        
                return;                                                                   
            }                                                                             
        }
                                                                                          
        char_puts("That alias is not defined.\n",ch);                                     
        return;                                                                           
    }                                                                                     
                                                                                          
    if (!str_prefix(argument,"delete") || !str_prefix(argument,"prefix"))                 
    {                                                                                     
        char_puts("That shall not be done!\n",ch);                                        
        return;                                                                           
    }                                                                                     
                                                                                          
    for (pos = 0; pos < MAX_ALIAS; pos++)                                                 
    {                                                                                     
        if (rch->pcdata->alias[pos] == NULL)                                              
            break;                                                                        
                                                                                          
        if (!str_cmp(arg,rch->pcdata->alias[pos])) /* redefine an alias */                
        {                                                                                 
            free_string(rch->pcdata->alias_sub[pos]);                                     
            rch->pcdata->alias_sub[pos] = str_dup(argument);                              
            char_printf(ch,"%s is now realiased to '%s'.\n",arg,argument);                
            return;                                                                       
        }                                                                                 
     }                                                                                    
                                                                                          
     if (pos >= MAX_ALIAS)                                                                
     {                                                                                    
        char_puts("Sorry, you have reached the alias limit.\n",ch);                       
        return;                                                                           
     }
                                                                                          
     /* make a new alias */                                                               
     rch->pcdata->alias[pos]            = str_dup(arg);                                   
     rch->pcdata->alias_sub[pos]        = str_dup(argument);                              
     char_printf(ch,"%s is now aliased to '%s'.\n",arg,argument);                         
}


void do_unalias(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *rch;
    char arg[MAX_INPUT_LENGTH];
    int pos;
    bool found = FALSE;
 
    if (ch->desc == NULL)
	rch = ch;
    else
	rch = ch->desc->original ? ch->desc->original : ch;
 
    if (IS_NPC(rch))
	return;
 
    argument = one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
    {
	char_puts("Unalias what?\n",ch);
	return;
    }

    for (pos = 0; pos < MAX_ALIAS; pos++)
    {
	if (rch->pcdata->alias[pos] == NULL)
	    break;

	if (found)
	{
	    rch->pcdata->alias[pos-1]		= rch->pcdata->alias[pos];
	    rch->pcdata->alias_sub[pos-1]	= rch->pcdata->alias_sub[pos];
	    rch->pcdata->alias[pos]		= NULL;
	    rch->pcdata->alias_sub[pos]		= NULL;
	    continue;
	}

	if(!strcmp(arg,rch->pcdata->alias[pos]))
	{
	    char_puts("Alias removed.\n",ch);
	    free_string(rch->pcdata->alias[pos]);
	    free_string(rch->pcdata->alias_sub[pos]);
	    rch->pcdata->alias[pos] = NULL;
	    rch->pcdata->alias_sub[pos] = NULL;
	    found = TRUE;
	}
    }

    if (!found)
	char_puts("No alias of that name to remove.\n",ch);
}

typedef struct config_t config_t;
struct config_t {
	const char	* name;
	DO_FUN		* do_fun;
};

config_t config_tabble[] =
{
	{"toggle",	do_toggle},
	{"language",	do_lang},
	{"channels",	do_channels},
	{"prompt",	do_prompt},
	{"bprompt",	do_bprompt},
	{"scroll",	do_scroll},
	{"trust",	do_trust},
	{"twit",	do_twit},
	{"password",	do_password},
	{NULL}
};

DO_FUN(do_config)
{
	char arg[MAX_INPUT_LENGTH];
	config_t *ct;
	
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}
	
	if (argument[0] == '\0')
	{
		char_printf(ch, "Syntax:\n"
			"   config toggle\n"
			"   config language\n"
			"   config channels\n"
			"   config prompt  [prompt | default | show]\n"
			"   config bprompt [prompt | default | show]\n"
			"   config scroll [number]\n"
			"   config trust [group | clan | all | none]\n"
			"   config twit [toggle_name]\n"
			"   config deaf   <-%s\n"
			"   config noexp  <-%s\n"
			"   config password <old> <new>\n",
			IS_SET(ch->comm,COMM_DEAF) ? "ON" : "OFF",
			IS_SET(ch->pcdata->otherf, OTHERF_NOEXP) ? "ON" : "OFF");
		char_puts("See also: mentalblock, auto, wimpy, visible, nofollow, alias(unalias),\n"
			  "          outfit, title, description, worth, speak, clear(cls), split.\n",
			  ch);
		return;
		
	}

	/* log if arg not pa"ssword" */
	if (str_prefix("pa", argument))
	{
		log_printf("Log_c %s: %s", ch->name, argument);
		wiznet("Log_c $N: $t", ch, argument, WIZ_SECURE, 0, ch->level);
	}

	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_prefix(arg, "deaf")) {
		if (IS_SET(ch->comm,COMM_DEAF)) {
			char_puts("You can now hear tells again.\n",ch);
			REMOVE_BIT(ch->comm, COMM_DEAF);
		} else {
			char_puts("From now on, you won't hear tells.\n",ch);
			SET_BIT(ch->comm, COMM_DEAF);
		}
		return;
	} else if (!str_prefix(arg, "noexp")) {
		if (IS_SET(ch->pcdata->otherf, OTHERF_NOEXP)) {
			REMOVE_BIT(ch->pcdata->otherf, OTHERF_NOEXP);
			char_puts("You can now get expirience again.\n", ch);
		} else {
			SET_BIT(ch->pcdata->otherf, OTHERF_NOEXP);
			char_puts("From now on, you won't get expirience.\n", ch);
		}
		return;
	}
	for (ct = config_tabble; ct->name; ct++)
		if (!str_prefix(arg, ct->name)) {
			(*ct->do_fun)(ch, argument);
			return;
		}
	do_config(ch, str_empty);
}

void doprintf(DO_FUN *fn, CHAR_DATA* ch, const char* fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	fn(ch, buf);
	va_end(ap);
}

