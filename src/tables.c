/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: tables.c,v 1.33 2003/05/12 20:41:30 xor Exp $
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
#include <time.h>
#include "merc.h"
#include "lang.h"
#include "db.h"

/*
 * first element of each flag_t[] table describes type of values
 * in the table.
 */

/*
flag_t slang_table[] =
{
	{ "",			TABLE_INTVAL,			},

	{ "common",		SLANG_COMMON,		TRUE	},
	{ "human",		SLANG_HUMAN,		TRUE	},
	{ "elvish",		SLANG_ELVISH,		TRUE	},
	{ "dwarvish",		SLANG_DWARVISH,		TRUE	},
	{ "gnomish",		SLANG_GNOMISH,		TRUE	},
	{ "giant",		SLANG_GIANT,		TRUE	},
	{ "trollish",		SLANG_TROLLISH,		TRUE	},
	{ "cat",		SLANG_CAT,		TRUE	},
	{ "mothertongue",	SLANG_MAX,		FALSE	},

	{ NULL }
};
*/

flag_t size_table[] =
{ 
	{ "",			TABLE_INTVAL			},

	{ "tiny",		SIZE_TINY,		TRUE	},
	{ "small", 		SIZE_SMALL,		TRUE	},
	{ "medium",		SIZE_MEDIUM,		TRUE	},
	{ "large",		SIZE_LARGE,		TRUE	},
	{ "huge", 		SIZE_HUGE,		TRUE	},
	{ "giant",		SIZE_GIANT,		TRUE	},
	{ "gargantuan",		SIZE_GARGANTUAN,	TRUE	},

	{ NULL }
};

flag_t act_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "npc",		ACT_NPC,		FALSE	},
	{ "sentinel",		ACT_SENTINEL,		TRUE	},
	{ "scavenger",		ACT_SCAVENGER,		TRUE	},
	{ "aggressive",		ACT_AGGRESSIVE,		TRUE	},
	{ "stay_area",		ACT_STAY_AREA,		TRUE	},
	{ "wimpy",		ACT_WIMPY,		TRUE	},
	{ "pet",		ACT_PET,		TRUE	},
	{ "train",		ACT_TRAIN,		TRUE	},
	{ "practice",		ACT_PRACTICE,		TRUE	},
	{ "hunter",		ACT_HUNTER,		TRUE	},
	{ "undead",		ACT_UNDEAD,		TRUE	},
	{ "cleric",		ACT_CLERIC,		TRUE	},
	{ "mage",		ACT_MAGE,		TRUE	},
	{ "thief",		ACT_THIEF,		TRUE	},
	{ "warrior",		ACT_WARRIOR,		TRUE	},
	{ "noalign",		ACT_NOALIGN,		TRUE	},
	{ "nopurge",		ACT_NOPURGE,		TRUE	},
	{ "outdoors",		ACT_OUTDOORS,		TRUE	},
	{ "questor",		ACT_QUESTOR,		TRUE	},
	{ "indoors",		ACT_INDOORS,		TRUE	},
	{ "rideable",		ACT_RIDEABLE,		TRUE	},
	{ "healer",		ACT_HEALER,		TRUE	},
//	{ "gain",		ACT_GAIN,		TRUE	},
	{ "repairman",		ACT_REPAIRMAN,		TRUE	},
	{ "update_always",	ACT_UPDATE_ALWAYS,	TRUE	},
	{ "changer",		ACT_CHANGER,		TRUE	},
	{ "notrack",		ACT_NOTRACK,		TRUE	},
	//{ "clan_guard",		ACT_CLAN_GUARD,		TRUE	},
	{ "summoned",		ACT_SUMMONED,		TRUE	},
	{ "sage",		ACT_SAGE,		TRUE	},
	{ "sheriff",		ACT_SHERIFF,		TRUE	},
	{ "stableman",		ACT_STABLEMAN,		TRUE	},
	{ "runner",		ACT_RUNNER,		TRUE	},
	{ "nocrumble_body",	ACT_NOCRUMBLE_BODY,	TRUE	},

	{ NULL }
};

flag_t plr_flags[] =
{
	{ "",			TABLE_BITVAL			},

//	{ "npc",		PLR_NPC,		FALSE	},
	{ "autolook",		PLR_AUTOLOOK,		FALSE	},
	{ "autoassist",		PLR_AUTOASSIST,		FALSE	},
	{ "autoexit",		PLR_AUTOEXIT,		FALSE	},
	{ "autoloot",		PLR_AUTOLOOT,		FALSE	},
	{ "autosac",		PLR_AUTOSAC,		FALSE	},
	{ "autogold",		PLR_AUTOGOLD,		FALSE	},
	{ "autosplit",		PLR_AUTOSPLIT,		FALSE	},
	{ "autoscalp",          PLR_AUTOSCALP,          FALSE   },
	{ "bought_pet",		PLR_BOUGHT_PET,		FALSE	},
	{ "holylight",		PLR_HOLYLIGHT,		FALSE	},
//	{ "have_boat",		PLR_HAVEBOAT,		FALSE	},
	{ "mentalblock",	PLR_MENTALBLOCK,	FALSE	},
	{ "nofollow",		PLR_NOFOLLOW,		FALSE	},
	{ "permit",		PLR_PERMIT,		TRUE	},
	{ "log",		PLR_LOG,		FALSE	},
	{ "deny",		PLR_DENY,		FALSE	},
	{ "freeze",		PLR_FREEZE,		FALSE	},
	{ "wanted",		PLR_WANTED,		FALSE	},
	{ "ghost",		PLR_GHOST,		FALSE	},
	{ "notitle",		PLR_NOTITLE,		FALSE	},
	{ "noexp",		PLR_NOEXP,		FALSE	},
	{ "hara_kiri",		PLR_HARA_KIRI,		FALSE	},
	{ "blink",		PLR_BLINK,		FALSE	},

	{ NULL }
};

flag_t affect_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "blind",		AFF_BLIND,		TRUE	},
	{ "invisible",		AFF_INVIS,		TRUE	},
	{ "detect_evil",	AFF_DETECT_EVIL,	TRUE	},
	{ "detect_invis",	AFF_DETECT_INVIS,	TRUE	},
	{ "detect_magic",	AFF_DETECT_MAGIC,	TRUE	},
	{ "detect_hidden",	AFF_DETECT_HIDDEN,	TRUE	},
	{ "detect_good",	AFF_DETECT_GOOD,	TRUE	},
	{ "sanctuary",		AFF_SANCTUARY,		TRUE	},
	{ "faerie_fire",	AFF_FAERIE_FIRE,	TRUE	},
	{ "infrared",		AFF_INFRARED,		TRUE	},
	{ "curse",		AFF_CURSE,		TRUE	},
	{ "corruption",		AFF_CORRUPTION,		TRUE	},
	{ "poison",		AFF_POISON,		TRUE	},
	{ "protect_evil",	AFF_PROTECT_EVIL,	TRUE	},
	{ "protect_good",	AFF_PROTECT_GOOD,	TRUE	},
	{ "sneak",		AFF_SNEAK,		TRUE	},
	{ "hide",		AFF_HIDE,		TRUE	},
	{ "sleep",		AFF_SLEEP,		TRUE	},
	{ "charm",		AFF_CHARM,		TRUE	},
	{ "flying",		AFF_FLYING,		TRUE	},
	{ "pass_door",		AFF_PASS_DOOR,		TRUE	},
	{ "haste",		AFF_HASTE,		TRUE	},
	{ "calm",		AFF_CALM,		TRUE	},
	{ "plague",		AFF_PLAGUE,		TRUE	},
	{ "weaken",		AFF_WEAKEN,		TRUE	},
//	{ "dark_vision",	AFF_DARK_VISION,	TRUE	},
	{ "berserk",		AFF_BERSERK,		TRUE	},
//	{ "swim",		AFF_SWIM,		TRUE	},
	{ "regeneration",	AFF_REGENERATION,	TRUE	},
	{ "slow",		AFF_SLOW,		TRUE	},
	{ "camouflage",		AFF_CAMOUFLAGE,		TRUE	},
	{ "imp_invis",		AFF_IMP_INVIS,		TRUE	},
	{ "fade",		AFF_FADE,		TRUE	},
	{ "scream",		AFF_SCREAM,		TRUE	},
	{ "bloodthirst",	AFF_BLOODTHIRST,	TRUE	},
	{ "stun",		AFF_STUN,		TRUE	},
	{ "weak_stun",		AFF_WEAK_STUN,		TRUE	},
	{ "detect_imp_invis",	AFF_DETECT_IMP_INVIS,	TRUE	},
	{ "detect_fade",	AFF_DETECT_FADE,	TRUE	},
	{ "detect_undead",	AFF_DETECT_UNDEAD,	TRUE	},
	{ "detect_fear",	AFF_DETECT_FEAR,	TRUE	},
//	{ "detect_form_tree",	AFF_DETECT_FORM_TREE,	TRUE	},
//	{ "detect_form_grass",	AFF_DETECT_FORM_GRASS,	TRUE	},
	{ "detect_web",		AFF_DETECT_WEB,		TRUE	},
	{ "detect_life",	AFF_DETECT_LIFE,	TRUE	},
	{ "acute_vision",	AFF_ACUTE_VISION,	TRUE	},
	{ "black_shroud",	AFF_BLACK_SHROUD,	TRUE	},
	{ "bless",		AFF_BLESS,		TRUE	},
	{ "aquabreath",		AFF_AQUABREATH,		TRUE	},
	{ "feebling",		AFF_FEEBLING,		TRUE	},

	{ NULL }
};

flag_t shop_flags[] =
{
	{ "",			TABLE_BITVAL			},
	
	{ "view_lore",		SHOP_VIEW_LORE,		FALSE	},
	{ "view_identify",	SHOP_VIEW_IDENTIFY,	FALSE	},
	{ "view_brief",		SHOP_VIEW_BRIEF,	TRUE	},
	
	{ NULL }
};

flag_t off_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "area_attack",	OFF_AREA_ATTACK,	TRUE	},
	{ "backstab",		OFF_BACKSTAB,		TRUE	},
	{ "bash",		OFF_BASH,		TRUE	},
	{ "berserk",		OFF_BERSERK,		TRUE	},
	{ "disarm",		OFF_DISARM,		TRUE	},
	{ "dodge",		OFF_DODGE,		TRUE	},
	{ "fade",		OFF_FADE,		TRUE	},
	{ "fast",		OFF_FAST,		TRUE	},
	{ "kick",		OFF_KICK,		TRUE	},
	{ "dirt_kick",		OFF_DIRT_KICK,		TRUE	},
	{ "parry",		OFF_PARRY,		TRUE	},
	{ "rescue",		OFF_RESCUE,		TRUE	},
	{ "tail",		OFF_TAIL,		TRUE	},
	{ "trip",		OFF_TRIP,		TRUE	},
	{ "crush",		OFF_CRUSH,		TRUE	},
	{ "assist_all",		ASSIST_ALL,		TRUE	},
	{ "assist_align",	ASSIST_ALIGN,		TRUE	},
	{ "assist_race",	ASSIST_RACE,		TRUE	},
	{ "assist_players",	ASSIST_PLAYERS,		TRUE	},
	{ "assist_guard",	ASSIST_GUARD,		TRUE	},
	{ "assist_vnum",	ASSIST_VNUM,		TRUE	},
	{ "swimming",		OFF_SWIMMING,		TRUE	},

	{ NULL }
};

flag_t irv_flags[] =
{
	{ "",			TABLE_BITVAL			},
	
	{ "magic",		FORCE_MAGIC,		TRUE	},
	{ "water",		MAGIC_WATER,		TRUE	},
	{ "air",		MAGIC_AIR,		TRUE	},
	{ "earth",		MAGIC_EARTH,		TRUE	},
	{ "fire",		MAGIC_FIRE,		TRUE	},
	{ "death",		FORCE_DEATH,		TRUE	},	// MAGIC_LIFE
	{ "mental",		MAGIC_MENTAL,		TRUE	},

	{ "holy",		FORCE_HOLY,		TRUE	},
	{ "bash",		FORCE_BASH,		TRUE	},
	{ "pierce",		FORCE_PIERCE,		TRUE	},
	{ "slash",		FORCE_SLASH,		TRUE	},
	{ "weapon",		FORCE_WEAPON,		TRUE	},
	{ "wood",		FORCE_WOOD,		TRUE	},
	{ "silver",		FORCE_SILVER,		TRUE	},
	{ "iron",		FORCE_IRON,		TRUE	},
	{ "steal",		FORCE_STEAL,		TRUE	}, // :))
	{ "poison",		FORCE_POISON,		TRUE	},
	{ "cold",		FORCE_COLD,		TRUE	},
	{ "disease",		FORCE_DISEASE,		TRUE	},
	{ "light",		FORCE_LIGHT,		TRUE	},
	{ "energy",		FORCE_ENERGY,		TRUE	},
	{ "acid",		FORCE_ACID,		TRUE	},
	{ "drowning",		FORCE_DROWNING,		TRUE	},
	
	{ NULL }	
};
#if 0
flag_t imm_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "summon",		IMM_SUMMON,		TRUE	},
	{ "charm",		IMM_CHARM,		TRUE	},
	{ "magic",		IMM_MAGIC,		TRUE	},
	{ "weapon",		IMM_WEAPON,		TRUE	},
	{ "bash",		IMM_BASH,		TRUE	},
	{ "pierce",		IMM_PIERCE,		TRUE	},
	{ "slash",		IMM_SLASH,		TRUE	},
	{ "fire",		IMM_FIRE,		TRUE	},
	{ "cold",		IMM_COLD,		TRUE	},
	{ "lightning",		IMM_LIGHTNING,		TRUE	},
	{ "acid",		IMM_ACID,		TRUE	},
	{ "poison",		IMM_POISON,		TRUE	},
	{ "negative",		IMM_NEGATIVE,		TRUE	},
	{ "holy",		IMM_HOLY,		TRUE	},
	{ "energy",		IMM_ENERGY,		TRUE	},
	{ "mental",		IMM_MENTAL,		TRUE	},
	{ "disease",		IMM_DISEASE,		TRUE	},
	{ "drowning",		IMM_DROWNING,		TRUE	},
	{ "light",		IMM_LIGHT,		TRUE	},
	{ "sound",		IMM_SOUND,		TRUE	},
	{ "wood",		IMM_WOOD,		TRUE	},
	{ "silver",		IMM_SILVER,		TRUE	},
	{ "iron",		IMM_IRON,		TRUE	},
	{ "steal",		IMM_STEAL,		TRUE	},

	{ NULL }
};
#endif

flag_t form_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "edible",		FORM_EDIBLE,		TRUE	},
	{ "poison",		FORM_POISON,		TRUE	},
	{ "magical",		FORM_MAGICAL,		TRUE	},
	{ "instant_decay",	FORM_INSTANT_DECAY,	TRUE	},
	{ "other",		FORM_OTHER,		TRUE	},
	{ "animal",		FORM_ANIMAL,		TRUE	},
	{ "sentient",		FORM_SENTIENT,		TRUE	},
	{ "undead",		FORM_UNDEAD,		TRUE	},
	{ "construct",		FORM_CONSTRUCT,		TRUE	},
	{ "mist",		FORM_MIST,		TRUE	},
	{ "intangible",		FORM_INTANGIBLE,	TRUE	},
	{ "biped",		FORM_BIPED,		TRUE	},
	{ "centaur",		FORM_CENTAUR,		TRUE	},
	{ "insect",		FORM_INSECT,		TRUE	},
	{ "spider",		FORM_SPIDER,		TRUE	},
	{ "crustacean",		FORM_CRUSTACEAN,	TRUE	},
	{ "worm",		FORM_WORM,		TRUE	},
	{ "blob",		FORM_BLOB,		TRUE	},
	{ "mammal",		FORM_MAMMAL,		TRUE	},
	{ "bird",		FORM_BIRD,		TRUE	},
	{ "reptile",		FORM_REPTILE,		TRUE	},
	{ "snake",		FORM_SNAKE,		TRUE	},
	{ "dragon",		FORM_DRAGON,		TRUE	},
	{ "amphibian",		FORM_AMPHIBIAN,		TRUE	},
	{ "fish",		FORM_FISH ,		TRUE	},
	{ "cold_blood",		FORM_COLD_BLOOD,	TRUE	},
	{ "extracted",		FORM_EXTRACTED,		FALSE	},

	{ NULL }
};

flag_t part_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "head",		PART_HEAD,		TRUE	},
	{ "arms",		PART_ARMS,		TRUE	},
	{ "legs",		PART_LEGS,		TRUE	},
	{ "heart",		PART_HEART,		TRUE	},
	{ "brains",		PART_BRAINS,		TRUE	},
	{ "guts",		PART_GUTS,		TRUE	},
	{ "hands",		PART_HANDS,		TRUE	},
	{ "feet",		PART_FEET,		TRUE	},
	{ "fingers",		PART_FINGERS,		TRUE	},
	{ "ear",		PART_EAR,		TRUE	},
	{ "eye",		PART_EYE,		TRUE	},
	{ "long_tongue",	PART_LONG_TONGUE,	TRUE	},
	{ "eyestalks",		PART_EYESTALKS,		TRUE	},
	{ "tentacles",		PART_TENTACLES,		TRUE	},
	{ "fins",		PART_FINS,		TRUE	},
	{ "wings",		PART_WINGS,		TRUE	},
	{ "tail",		PART_TAIL,		TRUE	},
	{ "claws",		PART_CLAWS,		TRUE	},
	{ "fangs",		PART_FANGS,		TRUE	},
	{ "horns",		PART_HORNS,		TRUE	},
	{ "scales",		PART_SCALES,		TRUE	},
	{ "tusks",		PART_TUSKS,		TRUE	},

	{ NULL }
};

flag_t comm_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "quiet",		COMM_QUIET,		TRUE	},
	{ "deaf",		COMM_DEAF,		TRUE	},
	{ "nowiz",		COMM_NOWIZ,		TRUE	},
	{ "nogossip",		COMM_NOGOSSIP,		TRUE	},
	{ "noquestion",		COMM_NOQUESTION,	TRUE	},
	{ "noauction",		COMM_NOAUCTION,		TRUE	},
	{ "nomusic",		COMM_NOMUSIC,		TRUE	},
	{ "noquote",		COMM_NOQUOTE,		TRUE	},
	//{ "noclan",		COMM_NOCLAN,		TRUE	},
	{ "quiet_editor",	COMM_QUIET_EDITOR,	TRUE	},
	{ "compact",		COMM_COMPACT,		TRUE	},
	//{ "nocompact",	COMM_NOCOMPACT,		TRUE	},
	{ "brief",		COMM_BRIEF,		TRUE	},
	{ "prompt",		COMM_PROMPT,		TRUE	},
	{ "long",		COMM_LONG,		TRUE	},
	{ "combine",		COMM_COMBINE,		TRUE	},
	{ "telnet_ga",		COMM_TELNET_GA,		TRUE	},
	{ "showaff",		COMM_SHOWAFF,		TRUE	},
	{ "color",		COMM_COLOR,		TRUE	},
	{ "noemote",		COMM_NOEMOTE,		FALSE	},
	{ "noshout",		COMM_NOSHOUT,		FALSE	},
	{ "nochat",             COMM_NOCHAT,            FALSE   },
	{ "notell",		COMM_NOTELL,		FALSE	},
	{ "nochannels",		COMM_NOCHANNELS,	FALSE	},
	{ "snoop_proof",	COMM_SNOOP_PROOF,	FALSE	},
	{ "afk",		COMM_AFK,		TRUE	},
	{ "notelnet",		COMM_NOTELNET,		TRUE	},
	{ "noiac",		COMM_NOIAC,		TRUE	},
	{ "noverbose",		COMM_NOVERBOSE,		TRUE	},
	{ "nobust",		COMM_NOBUST,		TRUE	},
	{ "noeng",		COMM_NOENG,		TRUE	},
	{ "noautoflee",		COMM_NOFLEE,		TRUE	},
	{ "syncprompt",		COMM_SYNCPROMPT,	TRUE	},
	{ "no_autotitle",	COMM_NO_AUTOTITLE,	TRUE	},
	{ "hpdamage",		COMM_HPDAMAGE,		TRUE	},
	{ "have_boat",		COMM_HAVEBOAT,		FALSE	},
	{ "cut_messages",	COMM_CUT_MESSAGE,	TRUE	},
	{ "not_vslang",		COMM_NOT_VSLANG,	TRUE	},
	{ "not_ou_examine",	COMM_NOT_OU_EXA,	TRUE	}, 

	{ NULL }
};

/*
flag_t clan_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "hidden",		CLAN_HIDDEN,		TRUE	},
	{ "changed",		CLAN_CHANGED,		FALSE	},

	{ NULL }
};
*/

flag_t area_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "closed",		AREA_CLOSED,		TRUE	},
	{ "noquest",		AREA_NOQUEST,		TRUE	},
	{ "changed",		AREA_CHANGED,		FALSE	},

	{ NULL }
};

flag_t sex_table[] =
{
	{ "",			TABLE_INTVAL			},

	{ "none",		SEX_NEUTRAL,		TRUE	},
	{ "male",		SEX_MALE,		TRUE	},
	{ "female",		SEX_FEMALE,		TRUE	},
	{ "either",		SEX_EITHER,		TRUE	},
	{ "random",		SEX_EITHER,		TRUE    },
	{ "neutral",		SEX_NEUTRAL,		TRUE	},

	{ NULL	}
};

flag_t gender_table[] =
{
	{ "",			TABLE_INTVAL			},

	{ "none",		SEX_NEUTRAL,		TRUE	},
	{ "male",		SEX_MALE,		TRUE	},
	{ "female",		SEX_FEMALE,		TRUE	},
	{ "neutral",		SEX_NEUTRAL,		TRUE	},
	{ "multi",		SEX_MULTI,		TRUE	},
	{ NULL	}
};

flag_t exit_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "door",		EX_ISDOOR,		TRUE    },
	{ "closed",		EX_CLOSED,		TRUE	},
	{ "locked",		EX_LOCKED,		TRUE	},
	{ "noflee",		EX_NOFLEE,		TRUE	},
	{ "pickproof",		EX_PICKPROOF,		TRUE	},
	{ "nopass",		EX_NOPASS,		TRUE	},
	{ "easy",		EX_EASY,		TRUE	},
	{ "hard",		EX_HARD,		TRUE	},
	{ "infuriating",	EX_INFURIATING,		TRUE	},
	{ "noclose",		EX_NOCLOSE,		TRUE	},
	{ "nolock",		EX_NOLOCK,		TRUE	},
	{ "buryed",		EX_BURYED,		TRUE	},
	{ "hidden",		EX_HIDDEN,		TRUE	}, 

	{ NULL }
};

flag_t door_resets[] =
{
	{ "",			TABLE_INTVAL			},

	{ "open",			0,		TRUE	},
	{ "closed",			1,		TRUE	},
	{ "closed and locked",		2,		TRUE	},
	{ "buryed",			3,		TRUE	},
	{ "buryed and closed",		4,		TRUE	},
	{ "bury & clo & lock",		5,		TRUE	},

	{ NULL }
};

flag_t room_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "dark",		ROOM_DARK,		TRUE	},
	{ "nomob",		ROOM_NOMOB,		TRUE	},
	{ "indoors",		ROOM_INDOORS,		TRUE	},
	{ "peace",		ROOM_PEACE,		TRUE	},
	{ "noreinc",		ROOM_NOREINC,		TRUE	},
	{ "private",		ROOM_PRIVATE,		TRUE    },
	{ "safe",		ROOM_SAFE,		TRUE	},
	{ "solitary",		ROOM_SOLITARY,		TRUE	},
	{ "pet_shop",		ROOM_PET_SHOP,		TRUE	},
	{ "norecall",		ROOM_NORECALL,		TRUE	},
	{ "imp_only",		ROOM_IMP_ONLY,		TRUE    },
	{ "gods_only",	        ROOM_GODS_ONLY,		TRUE    },
	{ "heroes_only"	,	ROOM_HEROES_ONLY,	TRUE	},
	{ "newbies_only",	ROOM_NEWBIES_ONLY,	TRUE	},
	{ "law",		ROOM_LAW,		TRUE	},
	{ "nowhere",		ROOM_NOWHERE,		TRUE	},
	{ "bank",		ROOM_BANK,		TRUE	},
	{ "nomagic",		ROOM_NOMAGIC,		TRUE	},
	{ "nosummon",		ROOM_NOSUMMON,		TRUE	},
	{ "battle_arena",	ROOM_BATTLE_ARENA,	TRUE	},
	{ "guild",		ROOM_GUILD,		TRUE	},
	{ "registry",		ROOM_REGISTRY,		TRUE	},
	{ "hush_exits",		ROOM_HASH_EXITS,	TRUE	},

	{ NULL }
};

flag_t sector_types[] =
{
	{ "",			TABLE_INTVAL			},

	{ "inside",		SECT_INSIDE,		TRUE	},
	{ "city",		SECT_CITY,		TRUE	},
	{ "field",		SECT_FIELD,		TRUE	},
	{ "forest",		SECT_FOREST,		TRUE	},
	{ "hills",		SECT_HILLS,		TRUE	},
	{ "mountain",		SECT_MOUNTAIN,		TRUE	},
	{ "water",		SECT_WATER,		TRUE	},
	{ "swim",		SECT_WATER,		FALSE	}, /* old */
	{ "noswim",		SECT_WATER,		FALSE	}, /* old */
	{ "underwater",		SECT_UNDER_WATER,	TRUE	},
	{ "swamp",		SECT_SWAMP,		TRUE	},
	{ "unused",		SECT_UNUSED,		TRUE	},
	{ "air",		SECT_AIR,		TRUE	},
	{ "desert",		SECT_DESERT,		TRUE	},

	{ NULL }
};

flag_t item_types[] =
{
	{ "",			TABLE_INTVAL			},

	{ "light",		ITEM_LIGHT,		TRUE	},
	{ "scroll",		ITEM_SCROLL,		TRUE	},
	{ "wand",		ITEM_WAND,		TRUE	},
	{ "staff",		ITEM_STAFF,		TRUE	},
	{ "weapon",		ITEM_WEAPON,		TRUE	},
	{ "treasure",		ITEM_TREASURE,		TRUE	},
	{ "armor",		ITEM_ARMOR,		TRUE	},
	{ "potion",		ITEM_POTION,		TRUE	},
	{ "clothing",		ITEM_CLOTHING,		TRUE	},
	{ "furniture",		ITEM_FURNITURE,		TRUE	},
	{ "trash",		ITEM_TRASH,		TRUE	},
	{ "container",		ITEM_CONTAINER,		TRUE	},
	{ "drink",		ITEM_DRINK_CON,		TRUE	},
	{ "key",		ITEM_KEY,		TRUE	},
	{ "food",		ITEM_FOOD,		TRUE	},
	{ "money",		ITEM_MONEY,		TRUE	},
	{ "boat",		ITEM_BOAT,		TRUE	},
	{ "npc_corpse",		ITEM_CORPSE_NPC,	TRUE	},
	{ "pc_corpse",		ITEM_CORPSE_PC,		FALSE	},
	{ "fountain",		ITEM_FOUNTAIN,		TRUE	},
	{ "pill",		ITEM_PILL,		TRUE	},
	{ "protect",		ITEM_PROTECT,		TRUE	},
	{ "map",		ITEM_MAP,		TRUE	},
	{ "portal",		ITEM_PORTAL,		TRUE	},
	{ "warp_stone",		ITEM_WARP_STONE,	TRUE	},
	{ "room_key",		ITEM_ROOM_KEY,		TRUE	},
	{ "gem",		ITEM_GEM,		TRUE	},
	{ "jewelry",		ITEM_JEWELRY,		TRUE	},
	{ "jukebox",		ITEM_JUKEBOX,		TRUE	},
	{ "tattoo",		ITEM_TATTOO,		FALSE	},
	{ "scalp_npc_corpse",   ITEM_SCALP_CORPSE_NPC,  FALSE   },
	{ "scalp_pc_corpse",    ITEM_SCALP_CORPSE_PC,   FALSE   },
	{ "horse_ticket",	ITEM_HORSE_TICKET,	TRUE	},
	{ "dig",		ITEM_DIG,		TRUE	},

	{ NULL }
};

flag_t extra_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "glow",		ITEM_GLOW,		TRUE	},
	{ "hum",		ITEM_HUM,		TRUE	},
	{ "dark",		ITEM_DARK,		TRUE	},
	{ "lock",		ITEM_LOCK,		TRUE	},
	{ "evil",		ITEM_EVIL,		TRUE	},
	{ "invis",		ITEM_INVIS,		TRUE	},
	{ "magic",		ITEM_MAGIC,		TRUE	},
	{ "nodrop",		ITEM_NODROP,		TRUE	},
	{ "bless",		ITEM_BLESS,		TRUE	},
	{ "antigood",		ITEM_ANTI_GOOD,		TRUE	},
	{ "antievil",		ITEM_ANTI_EVIL,		TRUE	},
	{ "antineutral",	ITEM_ANTI_NEUTRAL,	TRUE	},
	{ "noremove",		ITEM_NOREMOVE,		TRUE	},
	{ "nopurge",		ITEM_NOPURGE,		TRUE	},
	{ "rotdeath",		ITEM_ROT_DEATH,		TRUE	},
	{ "visdeath",		ITEM_VIS_DEATH,		TRUE	},
	{ "nosac",		ITEM_NOSAC,		TRUE	},
	{ "nonmetal",		ITEM_NONMETAL,		TRUE	},
	{ "nolocate",		ITEM_NOLOCATE,		TRUE	},
	{ "meltdrop",		ITEM_MELT_DROP,		TRUE	},
	{ "hadtimer",		ITEM_HAD_TIMER,		TRUE	},
	{ "sellextract",	ITEM_SELL_EXTRACT,	TRUE	},
	{ "burnproof",		ITEM_BURN_PROOF,	TRUE	},
	{ "nouncurse",		ITEM_NOUNCURSE,		TRUE	},
	{ "nosell",		ITEM_NOSELL,		TRUE	},
	{ "not_edible",		ITEM_NOT_EDIBLE,	TRUE	},
	{ "quest",		ITEM_QUEST,		TRUE	},
	//{ "clan_item",		ITEM_CLAN,		TRUE	},
	{ "quit_drop",		ITEM_QUIT_DROP,		TRUE	},
	{ "pit",		ITEM_PIT,		TRUE	},
	{ "nodecompose",	ITEM_NODECOMPOSE,	TRUE	},
	{ "fuzzy_level",	ITEM_FUZZY_LEVEL,	TRUE	},
	{ "can_hide_near",	ITEM_CAN_HIDE_NEAR,	TRUE	},
	{ "can_be_on",		ITEM_CAN_BE_ON,		TRUE	},
	{ "can_be_under",	ITEM_CAN_BE_UNDER,	TRUE	},
	{ "can_hide",		ITEM_CAN_HIDE,		TRUE	},
	{ "nobury",		ITEM_NOBURY,		TRUE	},

	{ NULL }
};

flag_t wear_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "take",		ITEM_TAKE,		TRUE	},
	{ "finger",		ITEM_WEAR_FINGER,	TRUE	},
	{ "neck",		ITEM_WEAR_NECK,		TRUE	},
	{ "shirt",		ITEM_WEAR_SHIRT,	TRUE	},
	{ "body",		ITEM_WEAR_BODY,		TRUE	},
	{ "head",		ITEM_WEAR_HEAD,		TRUE	},
	{ "legs",		ITEM_WEAR_LEGS,		TRUE	},
	{ "feet",		ITEM_WEAR_FEET,		TRUE	},
	{ "hands",		ITEM_WEAR_HANDS,	TRUE	},
	{ "arms",		ITEM_WEAR_ARMS,		TRUE	},
	{ "shield",		ITEM_WEAR_SHIELD,	TRUE	},
	{ "about",		ITEM_WEAR_ABOUT,	TRUE	},
	{ "waist",		ITEM_WEAR_WAIST,	TRUE	},
	{ "wrist",		ITEM_WEAR_WRIST,	TRUE	},
	{ "wield",		ITEM_WIELD,		TRUE	},
	{ "hold",		ITEM_HOLD,		TRUE	},
	{ "nosac",		ITEM_NO_SAC,		TRUE	},
	{ "float_around",	ITEM_WEAR_FLOAT,	TRUE	},
	{ "tattoo",		ITEM_WEAR_TATTOO,	TRUE	},
	//{ "clanmark",		ITEM_WEAR_CLANMARK,	TRUE	},
	{ "back",		ITEM_WEAR_BACK,		TRUE	},
	{ "glasses",		ITEM_WEAR_GLASSES,	TRUE	},
	{ "earrings",		ITEM_WEAR_EARRINGS,	TRUE	},
	{ "corpse_attacker",	ITEM_CORPSE_ATTACKER,	FALSE	},
	{ "decompose",		ITEM_DECOMPOSE,		FALSE	},

	{ NULL }
};

/*
* Used when adding an affect to tell where it goes.
* See addaffect and delaffect in act_olc.c
*/
flag_t apply_flags[] =
{
	{ "",			TABLE_INTVAL			},

	{ "strength",		APPLY_STR,		TRUE	},
	{ "dexterity",		APPLY_DEX,		TRUE	},
	{ "intelligence",	APPLY_INT,		TRUE	},
	{ "wisdom",		APPLY_WIS,		TRUE	},
	{ "constitution",	APPLY_CON,		TRUE	},
	{ "charisma",		APPLY_CHA,		TRUE	},
	{ "sex",		APPLY_SEX,		TRUE	},
	{ "level",		APPLY_LEVEL,		TRUE	},
	{ "age",		APPLY_AGE,		TRUE	},
//	{ "height",		APPLY_HEIGHT,		TRUE	},
//	{ "weight",		APPLY_WEIGHT,		TRUE	},
	{ "mana",		APPLY_MANA,		TRUE	},
	{ "hp",			APPLY_HIT,		TRUE	},
	{ "move",		APPLY_MOVE,		TRUE	},
	{ "gold",		APPLY_GOLD,		TRUE	},
	{ "experience",		APPLY_EXP,		TRUE	},
	{ "ac",			APPLY_AC,		TRUE	},
	{ "hitroll",		APPLY_HITROLL,		TRUE	},
	{ "damroll",		APPLY_DAMROLL,		TRUE	},
	{ "saves",		APPLY_SAVES,		TRUE	},
	{ "saving_earth",	APPLY_SAVING_EARTH,	TRUE	},
	{ "saving_fire",	APPLY_SAVING_FIRE,	TRUE	},
	{ "saving_air",		APPLY_SAVING_AIR,	TRUE	},
	{ "saving_water",	APPLY_SAVING_WATER,	TRUE	},
	{ "saving_life",	APPLY_SAVING_LIFE,	TRUE	},
	{ "saving_mental",	APPLY_SAVING_MENTAL,	TRUE	},
	{ "spellaffect",	APPLY_SPELL_AFFECT,	TRUE	},
	{ "size",		APPLY_SIZE,		TRUE	},
	{ "class",		APPLY_CLASS,		TRUE	},
	{ "race",		APPLY_RACE,		TRUE	},
	{ "apply_5_sk",		APPLY_5_SKILL,		TRUE	},
	{ "apply_10_sk",	APPLY_10_SKILL,		TRUE	},

	{ NULL }
};

flag_t rapply_flags[] =
{
	{ "",			TABLE_INTVAL			},

	{ "healrate",		APPLY_ROOM_HEAL,	TRUE	},
	{ "manarate",		APPLY_ROOM_MANA,	TRUE	},

	{ NULL }
};

/*
 * What is seen.
 */
flag_t wear_loc_strings[] =
{
	{ "",				TABLE_INTVAL			},

	{ "in the inventory",		WEAR_NONE,		TRUE	},
	{ "as a light",			WEAR_LIGHT,		TRUE	},
	{ "on the left finger",		WEAR_FINGER_L,		TRUE	},
	{ "on the right finger",	WEAR_FINGER_R,		TRUE	},
	{ "around the neck",		WEAR_NECK,		TRUE	},
	{ "on the body (shirt)",	WEAR_SHIRT,		TRUE	},
	{ "on the body",		WEAR_BODY,		TRUE	},
	{ "over the head",		WEAR_HEAD,		TRUE	},
	{ "on the legs",		WEAR_LEGS,		TRUE	},
	{ "on the feet",		WEAR_FEET,		TRUE	},
	{ "on the hands",		WEAR_HANDS,		TRUE	},
	{ "on the arms",		WEAR_ARMS,		TRUE	},
	{ "as a shield",		WEAR_SHIELD,		TRUE	},
	{ "about the shoulders",	WEAR_ABOUT,		TRUE	},
	{ "around the waist",		WEAR_WAIST,		TRUE	},
	{ "on the left wrist",		WEAR_WRIST_L,		TRUE	},
	{ "on the right wrist",		WEAR_WRIST_R,		TRUE	},
	{ "wielded",			WEAR_WIELD,		TRUE	},
	{ "held in the hands",		WEAR_HOLD,		TRUE	},
	{ "floating nearby",		WEAR_FLOAT,		TRUE	},
	{ "scratched tattoo",		WEAR_TATTOO,		TRUE	},
	{ "second wielded",		WEAR_SECOND_WIELD,	TRUE	},
	{ "clan mark",			WEAR_CLANMARK,		TRUE	},
	{ "stuck in",			WEAR_STUCK_IN,		TRUE	},
	{ "worn on back",		WEAR_BACK,		TRUE	},
	{ "worn on face",		WEAR_GLASSES,		TRUE	},
	{ "worn on left ear",		WEAR_EARRING_L,		TRUE	},
	{ "worn on right ear",		WEAR_EARRING_R,		TRUE	},

	{ NULL }
};

flag_t wear_loc_flags[] =
{
	{ "",			TABLE_INTVAL			},

	{ "none",		WEAR_NONE,		TRUE	},
	{ "inventory",		WEAR_NONE,		TRUE	},
	{ "light",		WEAR_LIGHT,		TRUE	},
	{ "lfinger",		WEAR_FINGER_L,		TRUE	},
	{ "rfinger",		WEAR_FINGER_R,		TRUE	},
	{ "neck",		WEAR_NECK,		TRUE	},
	{ "shirt",		WEAR_SHIRT,		TRUE	},
	{ "body",		WEAR_BODY,		TRUE	},
	{ "head",		WEAR_HEAD,		TRUE	},
	{ "legs",		WEAR_LEGS,		TRUE	},
	{ "feet",		WEAR_FEET,		TRUE	},
	{ "hands",		WEAR_HANDS,		TRUE	},
	{ "arms",		WEAR_ARMS,		TRUE	},
	{ "shield",		WEAR_SHIELD,		TRUE	},
	{ "about",		WEAR_ABOUT,		TRUE	},
	{ "waist",		WEAR_WAIST,		TRUE	},
	{ "lwrist",		WEAR_WRIST_L,		TRUE	},
	{ "rwrist",		WEAR_WRIST_R,		TRUE	},
	{ "wielded",		WEAR_WIELD,		TRUE	},
	{ "hold",		WEAR_HOLD,		TRUE	},
	{ "floating",		WEAR_FLOAT,		TRUE	},
	{ "tattoo",		WEAR_TATTOO,		TRUE	},
	{ "second",		WEAR_SECOND_WIELD,	TRUE	},
	{ "clanmark",		WEAR_CLANMARK,		TRUE	},
	{ "back",		WEAR_BACK,		TRUE	},
	{ "glasses",		WEAR_GLASSES,		TRUE	},
	{ "learring",		WEAR_EARRING_L,		TRUE	},
	{ "rearring",		WEAR_EARRING_R,		TRUE	},
	{ "stuck",              WEAR_STUCK_IN,          TRUE    },

	{ NULL }
};

flag_t cont_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "closable",		CONT_CLOSEABLE,		TRUE	},
	{ "pickproof",		CONT_PICKPROOF,		TRUE	},
	{ "closed",		CONT_CLOSED,		TRUE	},
	{ "locked",		CONT_LOCKED,		TRUE	},
	{ "put_on",		CONT_PUT_ON,		TRUE	},
	{ "quiver",		CONT_QUIVER,		TRUE	},

	{ NULL }
};

flag_t ac_type[] =
{
	{ "",			TABLE_INTVAL			},

	{ "pierce",		AC_PIERCE,		TRUE	},
	{ "bash",		AC_BASH,		TRUE	},
	{ "slash",		AC_SLASH,		TRUE	},
	{ "exotic",		AC_EXOTIC,		TRUE	},

	{ NULL }
};

flag_t weapon_class[] =
{
	{ "",			TABLE_INTVAL			},

	{ "exotic",		WEAPON_EXOTIC,		TRUE    },
	{ "sword",		WEAPON_SWORD,		TRUE    },
	{ "dagger",		WEAPON_DAGGER,		TRUE    },
	{ "spear",		WEAPON_SPEAR,		TRUE    },
	{ "mace",		WEAPON_MACE,		TRUE    },
	{ "axe",		WEAPON_AXE,		TRUE    },
	{ "flail",		WEAPON_FLAIL,		TRUE    },
	{ "whip",		WEAPON_WHIP,		TRUE    },
	{ "polearm",		WEAPON_POLEARM,		TRUE    },
	{ "bow",		WEAPON_BOW,		TRUE	},
	{ "arrow",		WEAPON_ARROW,		TRUE	},
	{ "lance",		WEAPON_LANCE,		TRUE	},

	{ NULL }
};

flag_t weapon_type2[] =
{
	{ "",			TABLE_BITVAL			},

	{ "flaming",		WEAPON_FLAMING,		TRUE	},
	{ "frost",		WEAPON_FROST,		TRUE	},
	{ "vampiric",		WEAPON_VAMPIRIC,	TRUE	},
	{ "sharp",		WEAPON_SHARP,		TRUE	},
	{ "vorpal",		WEAPON_VORPAL,		TRUE	},
	{ "twohands",		WEAPON_TWO_HANDS,	TRUE	},
	{ "shocking",		WEAPON_SHOCKING,	TRUE	},
	{ "poison",		WEAPON_POISON,		TRUE	},
	{ "holy",		WEAPON_HOLY,		TRUE	},
	{ "katana",		WEAPON_KATANA,		TRUE	},

	{ NULL }
};

#if 0
flag_t res_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "summon",		RES_SUMMON,		TRUE	},
	{ "charm",		RES_CHARM,		TRUE	},
	{ "magic",		RES_MAGIC,		TRUE	},
	{ "weapon",		RES_WEAPON,		TRUE	},
	{ "bash",		RES_BASH,		TRUE	},
	{ "pierce",		RES_PIERCE,		TRUE	},
	{ "slash",		RES_SLASH,		TRUE	},
	{ "fire",		RES_FIRE,		TRUE	},
	{ "cold",		RES_COLD,		TRUE	},
	{ "lightning",		RES_LIGHTNING,		TRUE	},
	{ "acid",		RES_ACID,		TRUE	},
	{ "poison",		RES_POISON,		TRUE	},
	{ "negative",		RES_NEGATIVE,		TRUE	},
	{ "holy",		RES_HOLY,		TRUE	},
	{ "energy",		RES_ENERGY,		TRUE	},
	{ "mental",		RES_MENTAL,		TRUE	},
	{ "disease",		RES_DISEASE,		TRUE	},
	{ "drowning",		RES_DROWNING,		TRUE	},
	{ "light",		RES_LIGHT,		TRUE	},
	{ "sound",		RES_SOUND,		TRUE	},
	{ "wood",		RES_WOOD,		TRUE	},
	{ "silver",		RES_SILVER,		TRUE	},
	{ "iron",		RES_IRON,		TRUE	},

	{ NULL }
};

flag_t vuln_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "summon",		VULN_SUMMON,		TRUE	},
	{ "charm",		VULN_CHARM,		TRUE	},
	{ "magic",		VULN_MAGIC,		TRUE	},
	{ "weapon",		VULN_WEAPON,		TRUE	},
	{ "bash",		VULN_BASH,		TRUE	},
	{ "pierce",		VULN_PIERCE,		TRUE	},
	{ "slash",		VULN_SLASH,		TRUE	},
	{ "fire",		VULN_FIRE,		TRUE	},
	{ "cold",		VULN_COLD,		TRUE	},
	{ "lightning",		VULN_LIGHTNING,		TRUE	},
	{ "acid",		VULN_ACID,		TRUE	},
	{ "poison",		VULN_POISON,		TRUE	},
	{ "negative",		VULN_NEGATIVE,		TRUE	},
	{ "holy",		VULN_HOLY,		TRUE	},
	{ "energy",		VULN_ENERGY,		TRUE	},
	{ "mental",		VULN_MENTAL,		TRUE	},
	{ "disease",		VULN_DISEASE,		TRUE	},
	{ "drowning",		VULN_DROWNING,		TRUE	},
	{ "light",		VULN_LIGHT,		TRUE	},
	{ "sound",		VULN_SOUND,		TRUE	},
	{ "wood",		VULN_WOOD,		TRUE	},
	{ "silver",		VULN_SILVER,		TRUE	},
	{ "iron",		VULN_IRON,		TRUE	},

	{ NULL }
};
#endif

flag_t position_table[] =
{
	{ "",			TABLE_INTVAL			},

	{ "dead",		POS_DEAD,		TRUE	},
	{ "mort",		POS_MORTAL,		TRUE	},
	{ "incap",		POS_INCAP,		TRUE	},
	{ "stun",		POS_STUNNED,		TRUE	},
	{ "sleep",		POS_SLEEPING,		TRUE	},
	{ "rest",		POS_RESTING,		TRUE	},
	{ "sit",		POS_SITTING,		TRUE	},
	{ "fight",		POS_FIGHTING,		TRUE	},
	{ "stand",		POS_STANDING,		TRUE	},

	{ NULL }
};

flag_t position_names[] =
{
	{ "",			TABLE_INTVAL	},

	{ "DEAD",		POS_DEAD	},
	{ "mortally wounded",	POS_MORTAL	},
	{ "incapacitated",	POS_INCAP	},
	{ "stunned",		POS_STUNNED	},
	{ "sleeping",		POS_SLEEPING	},
	{ "resting",		POS_RESTING	},
	{ "sitting",		POS_SITTING	},
	{ "fighting",		POS_FIGHTING	},
	{ "standing",		POS_STANDING	},

	{ NULL }
};

flag_t portal_flags[]=
{
	{ "",			TABLE_BITVAL			},

	{ "normal_exit",	GATE_NORMAL_EXIT,	TRUE	},
	{ "nocurse",		GATE_NOCURSE,		TRUE	},
	{ "gowith",		GATE_GOWITH,		TRUE	},
	{ "buggy",		GATE_BUGGY,		TRUE	},
	{ "random",		GATE_RANDOM,		TRUE	},

	{ NULL }
};

flag_t furniture_flags[]=
{
	{ "",			TABLE_BITVAL			},

	{ "stand_at",		STAND_AT,		TRUE	},
	{ "stand_on",		STAND_ON,		TRUE	},
	{ "stand_in",		STAND_IN,		TRUE	},
	{ "sit_at",		SIT_AT,			TRUE	},
	{ "sit_on",		SIT_ON,			TRUE	},
	{ "sit_in",		SIT_IN,			TRUE	},
	{ "rest_at",		REST_AT,		TRUE	},
	{ "rest_on",		REST_ON,		TRUE	},
	{ "rest_in",		REST_IN,		TRUE	},
	{ "sleep_at",		SLEEP_AT,		TRUE	},
	{ "sleep_on",		SLEEP_ON,		TRUE	},
	{ "sleep_in",		SLEEP_IN,		TRUE	},
	{ "put_at",		PUT_AT,			TRUE	},
	{ "put_on",		PUT_ON,			TRUE	},
	{ "put_in",		PUT_IN,			TRUE	},
	{ "put_inside",		PUT_INSIDE,		TRUE	},

	{ NULL }
};

flag_t apply_types[] =
{
	{ "",			TABLE_INTVAL			},

	{ "affects",		TO_AFFECTS,		TRUE	},
/*	{ "object",		TO_OBJECT,		TRUE	}, */
	{ "immune",		TO_IMMUNE,		TRUE	},
	{ "resist",		TO_RESIST,		TRUE	},
	{ "vuln",		TO_VULN,		TRUE	},
/*	{ "weapon",		TO_WEAPON,		TRUE	}, */

	{ NULL }
};

flag_t raffect_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "shocking",		RAFF_SHOCKING,		TRUE	},
	{ "lshield",		RAFF_LSHIELD,		TRUE	},
	{ "thief_trap",		RAFF_THIEF_TRAP,	TRUE	},
	{ "randomizer",		RAFF_RANDOMIZER,	TRUE	},
	{ "espirit",		RAFF_ESPIRIT,		TRUE	},
	{ "curse",		RAFF_CURSE,		TRUE	},
	{ "poison",		RAFF_POISON,		TRUE	},
	{ "sleep",		RAFF_SLEEP,		TRUE	},
	{ "plague",		RAFF_PLAGUE,		TRUE	},
	{ "slow",		RAFF_SLOW,		TRUE	},

	{ NULL }
};

flag_t skill_groups[] =
{
	{ "",			TABLE_BITVAL			},

	{ "none",		GROUP_NONE,		TRUE	},
	{ "weaponsmaster",	GROUP_WEAPONSMASTER,	TRUE	},
	{ "attack",		GROUP_ATTACK,		TRUE	},
	{ "beguiling",		GROUP_BEGUILING,	TRUE	},
	{ "benedictions",	GROUP_BENEDICTIONS,	TRUE	},
	{ "combat",		GROUP_COMBAT,		TRUE	},
	{ "creation",		GROUP_CREATION,		TRUE	},
	{ "curative",		GROUP_CURATIVE,		TRUE	},
	{ "detection",		GROUP_DETECTION,	TRUE	},
	{ "draconian",		GROUP_DRACONIAN,	TRUE	},
	{ "enchantment",	GROUP_ENCHANTMENT,	TRUE	},
	{ "enhancement",	GROUP_ENHANCEMENT,	TRUE	},
	{ "harmful",		GROUP_HARMFUL,		TRUE	},
	{ "healing",		GROUP_HEALING,		TRUE	},
	{ "illusion",		GROUP_ILLUSION,		TRUE	},
	{ "maladictions",	GROUP_MALADICTIONS,	TRUE	},
	{ "protective",		GROUP_PROTECTIVE,	TRUE	},
	{ "transportation",	GROUP_TRANSPORTATION,	TRUE	},
	{ "weather",		GROUP_WEATHER,		TRUE	},
	{ "fightmaster",	GROUP_FIGHTMASTER,	TRUE	},
	{ "suddendeath",	GROUP_SUDDENDEATH,	TRUE	},
	{ "meditation",		GROUP_MEDITATION,	TRUE	},
	//{ "clan",		GROUP_CLAN,		TRUE	},
	{ "defensive",		GROUP_DEFENSIVE,	TRUE	},
	{ "wizard",		GROUP_WIZARD,		TRUE	},
	{ "necromancy",		GROUP_NECROMANCY,	TRUE	},
	{ "lang",		GROUP_SLANG,		TRUE	},

	{ NULL }
};

flag_t sell_skill_type[] =
{
	{ "",		TABLE_INTVAL				},

	{ "race",	SSF_RACE,			TRUE	},
	{ "class",	SSF_CLASS,			TRUE	},
	{ "all",	SSF_ALL,			TRUE	},
	{ "sex",	SSF_SEX,			TRUE	},
	{ "align",	SSF_ALIGN,			TRUE	},
	{ "ethos",	SSF_ETHOS,			TRUE	},

	{ NULL}
};

flag_t mptrig_types[] =
{
	{ "",			TABLE_INTVAL			},

	{ "act",		TRIG_ACT,		TRUE	},
	{ "bribe",		TRIG_BRIBE,		TRUE 	},
	{ "death",		TRIG_DEATH,		TRUE    },
	{ "entry",		TRIG_ENTRY,		TRUE	},
	{ "fight",		TRIG_FIGHT,		TRUE	},
	{ "give",		TRIG_GIVE,		TRUE	},
	{ "greet",		TRIG_GREET,		TRUE    },
	{ "grall",		TRIG_GRALL,		TRUE	},
	{ "kill",		TRIG_KILL,		TRUE	},
	{ "hpcnt",		TRIG_HPCNT,		TRUE    },
	{ "random",		TRIG_RANDOM,		TRUE	},
	{ "speech",		TRIG_SPEECH,		TRUE	},
	{ "exit",		TRIG_EXIT,		TRUE    },
	{ "exall",		TRIG_EXALL,		TRUE    },
	{ "delay",		TRIG_DELAY,		TRUE    },
	{ "surr",		TRIG_SURR,		TRUE    },

	{ NULL }
};

flag_t mptrig_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "case-sensitive",	TRIG_CASEDEP,		FALSE	},

	{ NULL }
};

flag_t skill_targets[] =
{
	{ "",			TABLE_INTVAL			},

	{ "ignore",		TAR_IGNORE,		TRUE	},
	{ "charoff",		TAR_CHAR_OFFENSIVE,	TRUE	},
	{ "chardef",		TAR_CHAR_DEFENSIVE,	TRUE	},
	{ "charself",		TAR_CHAR_SELF,		TRUE	},
	{ "objinv",		TAR_OBJ_INV,		TRUE	},
	{ "objchardef",		TAR_OBJ_CHAR_DEF,	TRUE	},
	{ "objcharoff",		TAR_OBJ_CHAR_OFF,	TRUE	},

	{ NULL }
};

flag_t stat_names[] =
{
	{ "",			TABLE_INTVAL			},

	{ "str",		STAT_STR,		TRUE	},
	{ "int",		STAT_INT,		TRUE	},
	{ "dex",		STAT_DEX,		TRUE	},
	{ "wis",		STAT_WIS,		TRUE	},
	{ "con",		STAT_CON,		TRUE	},
	{ "cha",		STAT_CHA,		TRUE	},

	{ NULL }
};

flag_t skill_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "clan",		SKILL_CLAN,		TRUE	},
	{ "range",		SKILL_RANGE,		TRUE	},
	{ "area_attack",	SKILL_AREA_ATTACK,	TRUE	},
	{ "questionable",	SKILL_QUESTIONABLE,	TRUE	},
	{ "noremort",		SKILL_NOREMORT,		TRUE	},
	{ "cleric",		SKILL_CLERIC,		TRUE	},
	{ "created",		SKILL_CREATED,		FALSE	},

	{ NULL }
};

flag_t class_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "magic",		CLASS_MAGIC,		TRUE	},
	{ "noch",		CLASS_NOCH,		TRUE	},

	{ NULL }
};

flag_t race_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "undead",		RACE_UNDEAD,		TRUE	},
	{ "noch",		RACE_NOCH,		TRUE	},

	{ NULL }
};

flag_t ralign_names[] =
{
	{ "",			TABLE_BITVAL			},

	{ "evil",		RA_EVIL,		TRUE	},
	{ "neutral",		RA_NEUTRAL,		TRUE	},
	{ "good",		RA_GOOD,		TRUE	},

	{ NULL }
};

flag_t align_names[] =
{
	{ "",			TABLE_INTVAL			},

	{ "good",		ANUM_GOOD,		TRUE	},
	{ "neutral",		ANUM_NEUTRAL,		TRUE	},
	{ "evil",		ANUM_EVIL,		TRUE	},

	{ NULL }
};

flag_t lang_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "hidden",		LANG_HIDDEN,		TRUE	},
	{ "changed",		LANG_CHANGED,		FALSE	},

	{ NULL }
};

flag_t ethos_table[] =
{
	{ "",			TABLE_INTVAL			},

	{ "lawful",		ETHOS_LAWFUL,		TRUE	},
	{ "neutral",		ETHOS_NEUTRAL,		TRUE	},
	{ "chaotic",		ETHOS_CHAOTIC,		TRUE	},

	{ NULL }
};

flag_t rulecl_names[] =
{
	{ "",			TABLE_INTVAL			},

	{ "cases",		RULES_CASE,		TRUE	},
	{ "genders",		RULES_GENDER,		TRUE	},
	{ "qtys",		RULES_QTY,		TRUE	},

	{ NULL }
};

flag_t rulecl_flags[] =
{
	{ "",			TABLE_BITVAL			},

	{ "expl_changed",	RULES_EXPL_CHANGED		},
	{ "impl_changed",	RULES_IMPL_CHANGED		},

	{ NULL }
};

flag_t note_types[] =
{
	{ "",			TABLE_INTVAL,			},

	{ "note",		NOTE_NOTE			},
	{ "idea",		NOTE_IDEA			},
	{ "penalty",		NOTE_PENALTY			},
	{ "news",		NOTE_NEWS			},
	{ "change",		NOTE_CHANGES			},

	{ NULL }
};

flag_t options_table[] =
{
	{ "",			TABLE_BITVAL,			},

	{ "ascii_only_names",	OPT_ASCII_ONLY_NAMES		},

	{ NULL }
};

/* wiznet table and prototype for future flag setting */
const   struct wiznet_type      wiznet_table    []              =
{
   {    "on",           WIZ_ON,         IM },
   {    "prefix",	WIZ_PREFIX,	IM },
   {    "ticks",        WIZ_TICKS,      IM },
   {    "logins",       WIZ_LOGINS,     IM },
   {    "sites",        WIZ_SITES,      L4 },
   {    "links",        WIZ_LINKS,      L7 },
   {	"newbies",	WIZ_NEWBIE,	IM },
   {	"spam",		WIZ_SPAM,	L5 },
   {    "deaths",       WIZ_DEATHS,     IM },
   {    "resets",       WIZ_RESETS,     L4 },
   {    "mobdeaths",    WIZ_MOBDEATHS,  L4 },
   {    "flags",	WIZ_FLAGS,	L5 },
   {	"penalties",	WIZ_PENALTIES,	L5 },
   {	"saccing",	WIZ_SACCING,	L5 },
   {	"levels",	WIZ_LEVELS,	IM },
   {	"load",		WIZ_LOAD,	L2 },
   {	"restore",	WIZ_RESTORE,	L2 },
   {	"snoops",	WIZ_SNOOPS,	L2 },
   {	"switches",	WIZ_SWITCHES,	L2 },
   {	"secure",	WIZ_SECURE,	L1 },
   {	"olc",		WIZ_OLC,	IM },
   {    "reborn",	WIZ_REBORN,	IM },
   {	"rp",		WIZ_RP,		L8 },
   {	"debugolc",	WIZ_DEBUGOLC,	L3 },
   {	NULL,		0,		0  }
};

#if 0
/* attack table */
const 	struct attack_type	attack_table	[]		=
{
	{ "none",	"hit",			-1		}, /*  0 */
	{ "slice",	"slice", 		DAM_SLASH	},	
	{ "stab",	"stab",			DAM_PIERCE	},
	{ "slash",	"slash",		DAM_SLASH	},
	{ "whip",	"whip",			DAM_SLASH	},
	{ "claw",	"claw",			DAM_SLASH	}, /*  5 */
	{ "blast",	"blast",		DAM_BASH	},
	{ "pound",	"pound",		DAM_BASH	},
	{ "crush",	"crush",		DAM_BASH	},
	{ "grep",	"grep",			DAM_SLASH	},
	{ "bite",	"bite",			DAM_PIERCE	}, /* 10 */
	{ "pierce",	"pierce",		DAM_PIERCE	},
	{ "suction",	"suction",		DAM_BASH	},
	{ "beating",	"beating",		DAM_BASH	},
	{ "digestion",	"digestion",		DAM_ACID	},
	{ "charge",	"charge",		DAM_BASH	}, /* 15 */
	{ "slap",	"slap",			DAM_BASH	},
	{ "punch",	"punch",		DAM_BASH	},
	{ "wrath",	"wrath",		DAM_ENERGY	},
	{ "magic",	"magic",		DAM_ENERGY	},
	{ "divine",	"divine power",		DAM_HOLY	}, /* 20 */
	{ "cleave",	"cleave",		DAM_SLASH	},
	{ "scratch",	"scratch",		DAM_PIERCE	},
	{ "peck",	"peck",			DAM_PIERCE	},
	{ "peckb",	"peck",			DAM_BASH	},
	{ "chop",	"chop",			DAM_SLASH	}, /* 25 */
	{ "sting",	"sting",		DAM_PIERCE	},
	{ "smash",	"smash",		DAM_BASH	},
	{ "shbite",	"shocking bite",	DAM_LIGHTNING	},
	{ "flbite",	"flaming bite",		DAM_FIRE	},
	{ "frbite",	"freezing bite",	DAM_COLD	}, /* 30 */
	{ "acbite",	"acidic bite",		DAM_ACID	},
	{ "chomp",	"chomp",		DAM_PIERCE	},
	{ "drain",	"life drain",		DAM_NEGATIVE	},
	{ "thrust",	"thrust",		DAM_PIERCE	},
	{ "slime",	"slime",		DAM_ACID	}, /* 35 */
	{ "shock",	"shock",		DAM_LIGHTNING	},
	{ "thwack",	"thwack",		DAM_BASH	},
	{ "flame",	"flame",		DAM_FIRE	},
	{ "chill",	"chill",		DAM_COLD	},
	{ "kiss",	"kiss",			DAM_MENTAL	}, /* 40 */
	{ NULL }
};
#endif

/*
 * Attribute bonus tables.
 */

const   struct  str_app_type    str_app[MAX_STAT_OF+1] =
{ /* damroll  carry wield   int*/
	{ -12,  0,  0 },  /* 0  */
	{ -10,  3,  1 },  /* 1  */
	{ -9,   3,  2 },
	{ -8,  10,  3 },  /* 3  */
	{ -6,  25,  4 },
	{ -5,  55,  5 },  /* 5  */
	{ -3,  80,  6 },
	{ -2,  90,  7 },
	{ -1, 100,  8 },
	{  0, 100,  9 },
	{  0, 115, 10 },  /* 10  */
	{  0, 115, 11 },
	{  0, 130, 12 },
	{  1, 130, 13 },  /* 13  */
	{  2, 140, 14 },
	{  3, 150, 15 },  /* 15  */
	{  4, 165, 16 },
	{  5, 180, 22 },
	{  7, 200, 25 },  /* 18  */
	{  8, 225, 30 },
	{ 10, 250, 35 },  /* 20  */
	{ 11, 300, 40 },
	{ 12, 350, 45 },
	{ 13, 400, 50 },
	{ 14, 450, 55 },
	{ 16, 500, 60 },  /* 25 */
	{ 17, 550, 65 },
	{ 18, 600, 70 },
	{ 20, 650, 75 },
	{ 21, 700, 80 },
	{ 23, 750, 85 },   /* 30   */
	{ 25, 800, 90 },
	{ 27, 850, 95 },
	{ 30, 900, 100 }	/* 33 */
};


const   struct  int_app_type    int_app[MAX_STAT_OF+1] =
{ /*�����������    int*/                                                                         
    {  3 },     /*  0 */                                                        
    {  5 },     /*  1 */                                                        
    {  7 },                                                                     
    {  8 },     /*  3 */                                                        
    {  9 },                                                                     
    { 10 },     /*  5 */                                                        
    { 11 },
    { 12 },
    { 13 },
    { 15 },
    { 17 },     /* 10 */
    { 19 },
    { 22 },
    { 25 },
    { 28 },
    { 31 },     /* 15 */
    { 34 },             
    { 37 },             
    { 40 },     /* 18 */
    { 44 },             
    { 49 },     /* 20 */
    { 55 },
    { 60 },
    { 65 },
    { 70 },
    { 75 },     /* 25 */
    { 80 },
    { 85 },
    { 90 },
    { 95 },
    { 100 },     /* 30 */
    { 105 },
    { 110 },
    { 120 }	/* 33 */
};                                                                              
                                                                                
                                                                                
                                                                                
const   struct  wis_app_type    wis_app[MAX_STAT_OF+1] =
{ /* Prac         wis */
    { 0 },      /*  0 */
    { 0 },      /*  1 */
    { 0 },
    { 0 },      /*  3 */
    { 0 },
    { 1 },      /*  5 */
    { 1 },
    { 1 },              
    { 1 },              
    { 1 },              
    { 1 },      /* 10 */
    { 1 },              
    { 1 },              
    { 1 },              
    { 1 },              
    { 2 },      /* 15 */
    { 2 },              
    { 2 },              
    { 3 },      /* 18 */
    { 3 },              
    { 3 },      /* 20 */
    { 3 },              
    { 4 },              
    { 4 },              
    { 4 },
    { 5 },      /* 25 */
    { 5 },              
    { 5 },              
    { 6 },              
    { 6 },              
    { 7 },       /* 30 */
    { 7 },
    { 8 },
    { 9 }	/* 33 */
};
                                                                                
                                                                                
                                                                                
const   struct                                                                  
dex_app_type    dex_app[MAX_STAT_OF+1] =
{    /* hitroll  AC      dex */
	{ -14,	-60 },   /* 0 */
	{ -12, 	-50 },   /* 1 */
	{ -11, 	-50 },
	{ -10, 	-40 },
	{ -8,  	-30 },
	{ -7, 	-20 },   /* 5 */
	{ -5, 	-10 },
	{ -4,  	0 },
	{ -3,  	0 },
	{ -1,  	0 },
	{ 0,   	0 },   /* 10 */
	{ 0,   	0 },
	{ 0,   	0 },
	{ 0,   	0 },
	{ 1,   	0 },
	{ 3,	10 },   /* 15 */
	{ 4,	15 },
	{ 5,	20 },
	{ 7,	30 },
	{ 8,	40 },
	{ 10,	50 },   /* 20 */
	{ 11,	60 },
	{ 12,	75 },
	{ 13,	90 },
	{ 14,	105 },
	{ 16,	120 },   /* 25 */
	{ 17,	135 },
	{ 18,	150 },
	{ 20,	165 },
	{ 21,	180 },
	{ 22,	205 },    /* 30 */
	{ 24,	220 },		/* 31 */
	{ 25,	230 },		/* 32 */
	{ 27,	250 }		/* 33 */
};

                                                                                
const   struct  con_app_type    con_app[MAX_STAT_OF+1] =
{ /*Hit/lev Shock  con*/		                                                                            
    {  0 },   /*  0 */                                                      
    {  1 },   /*  1 */                                                      
    {  1 },                                                                 
    {  2 },   /*  3 */                                                      
    {  3 },                                                                 
    {  4 },   /*  5 */                                                      
    {  5 },                                                                 
    {  6 },                                                                 
    {  7 },                                                                 
    {  8 },                                                                 
    {  9 },   /* 10 */
    { 10 },                                                                 
    { 11 },                                                                 
    { 12 },                                                                 
    { 13 },                                                                 
    { 14 },   /* 15 */                                                      
    { 15 },                                                                 
    { 16 },                                                                 
    { 17 },   /* 18 */                                                      
    { 18 },                                                                 
    { 19 },   /* 20 */                                                      
    { 20 },                                                                 
    { 21 },                                                                 
    { 22 },                                                                 
    { 23 },                                                                 
    { 24 },   /* 25 */                                                      
    { 25 },                                                                 
    { 26 },                                                                 
    { 27 },                                                                 
    { 28 },                                                                 
    { 29 },    /* 30 */
    { 30 },
    { 32 },
    { 35 }
};

flag_t		componets_flags_table	[]	=
{
	{ "",			TABLE_BITVAL 			},

	{ "spend",		SPC_SPEND,		TRUE	},
	{ "successfull",	SPC_SUCCESSFULL,	TRUE	},
	{ "need_eq",		SPC_NEED_EQ,		TRUE	},
	{ "need_ground",	SPC_NEED_GROUND,	TRUE	},
	{ "check_after_delay",	SPC_CHECK_AFTER_DELAY,	TRUE	},
	{ "show_help",		SPC_SHOW_HELP,		TRUE	},
	{ "notneed_see",	SPC_NOTNEED_SEE,	TRUE	},
	{ "sufficient",		SPC_SUFFICIENT,		TRUE	},

	{ NULL }
};

flag_t		spell_flags_table		[]	=
{
	{ "",			TABLE_BITVAL			},

	{ "verbal",		SPT_VERBAL,		TRUE	},
	{ "somatic",		SPT_SOMATIC,		TRUE	},
	{ "targ_locate",	SPT_TARG_LOCATE,	TRUE	},
	{ "targ_object",	SPT_TARG_OBJECT,	TRUE	},
	{ "mana_after",		SPT_MANA_AFTER,		TRUE	},
	{ "checkpos_after",	SPT_CHECKPOS_AFTER,	TRUE	},
	{ "checkpos_before",	SPT_CHECKPOS_BEFORE,	TRUE	},

	{ "cast_far",		SPT_CAST_FAR,		FALSE	},
	{ "offensive",		SPT_OFFENSIVE,		FALSE	},
	{ "victseeaction",	SPT_VICTSEEACTION,	FALSE	},
	{ "nowait",		SPT_NOWAIT,		FALSE	},

	{ NULL }
};

/*
 * Liquid properties.
 * Used in world.obj.
 */
const	struct	liq_type	liq_table	[]	=
{
/*    name			color	proof, full, thirst, food, ssize */
    { "water",			"clear",	{   0, 1, 10, 0, 16 }	},
    { "beer",			"amber",	{  12, 1,  8, 1, 12 }	},
    { "red wine",		"burgundy",	{  30, 1,  8, 1,  5 }	},
    { "ale",			"brown",	{  15, 1,  8, 1, 12 }	},
    { "dark ale",		"dark",		{  16, 1,  8, 1, 12 }	},

    { "whisky",			"golden",	{ 120, 1,  5, 0,  2 }	},
    { "lemonade",		"pink",		{   0, 1,  9, 2, 12 }	},
    { "firebreather",		"boiling",	{ 190, 0,  4, 0,  2 }	},
    { "local specialty",	"clear",	{ 151, 1,  3, 0,  2 }	},
    { "slime mold juice",	"green",	{   0, 2, -8, 1,  2 }	},

    { "milk",			"white",	{   0, 2,  9, 3, 12 }	},
    { "tea",			"tan",		{   0, 1,  8, 0,  6 }	},
    { "coffee",			"black",	{   0, 1,  8, 0,  6 }	},
    { "blood",			"red",		{   0, 2, -1, 2,  6 }	},
    { "salt water",		"clear",	{   0, 1, -2, 0,  1 }	},

    { "coke",			"brown",	{   0, 2,  9, 2, 12 }	}, 
    { "root beer",		"brown",	{   0, 2,  9, 2, 12 }   },
    { "elvish wine",		"green",	{  35, 2,  8, 1,  5 }   },
    { "white wine",		"golden",	{  28, 1,  8, 1,  5 }   },
    { "champagne",		"golden",	{  32, 1,  8, 1,  5 }   },

    { "mead",			"honey-colored",{  34, 2,  8, 2, 12 }   },
    { "rose wine",		"pink",		{  26, 1,  8, 1,  5 }	},
    { "benedictine wine",	"burgundy",	{  40, 1,  8, 1,  5 }   },
    { "vodka",			"clear",	{ 130, 1,  5, 0,  2 }   },
    { "cranberry juice",	"red",		{   0, 1,  9, 2, 12 }	},

    { "orange juice",		"orange",	{   0, 2,  9, 3, 12 }   }, 
    { "absinthe",		"green",	{ 200, 1,  4, 0,  2 }	},
    { "brandy",			"golden",	{  80, 1,  5, 0,  4 }	},
    { "aquavit",		"clear",	{ 140, 1,  5, 0,  2 }	},
    { "schnapps",		"clear",	{  90, 1,  5, 0,  2 }   },

    { "icewine",		"purple",	{  50, 2,  6, 1,  5 }	},
    { "amontillado",		"burgundy",	{  35, 2,  8, 1,  5 }	},
    { "sherry",			"red",		{  38, 2,  7, 1,  5 }   },	
    { "framboise",		"red",		{  50, 1,  7, 1,  5 }   },
    { "rum",			"amber",	{ 151, 1,  4, 0,  2 }	},

    { "cordial",		"clear",	{ 100, 1,  5, 0,  2 }   },
    { "fruit jelly",		"colored",	{   0, 2,  5, 5, 12 }	},
    { "compote",		"tan",		{   0, 2,  9, 1, 12 }	},
    { NULL,			NULL,		{   0, 0,  0, 0,  0 }	}
};

where_t where_table[] =
{
	{ TO_AFFECTS,	affect_flags,	"'%s' affect"		},

	{ TO_IMMUNE,	irv_flags,	"immunity to '%s'"	},
	{ TO_RESIST,	irv_flags,	"resistance to '%s'"	},
	{ TO_VULN,	irv_flags,	"vulnerability to '%s'"	},
	{ -1 }
};

where_t *where_lookup(flag32_t where)
{
	where_t *wd;

	for (wd = where_table; wd->where != -1; wd++)
		if (wd->where == where)
			return wd;
	return NULL;
}

