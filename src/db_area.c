/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: db_area.c,v 1.31 2003/04/22 07:35:22 xor Exp $
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "merc.h"
#include "obj_prog.h"
#include "db.h"
#include "olc.h"

DECLARE_DBLOAD_FUN(load_area);
DECLARE_DBLOAD_FUN(load_areadata);
DECLARE_DBLOAD_FUN(load_helps);
//DECLARE_DBLOAD_FUN(load_old_mob);
DECLARE_DBLOAD_FUN(load_mobiles);
DECLARE_DBLOAD_FUN(load_mobprogs);
//DECLARE_DBLOAD_FUN(load_old_obj);
DECLARE_DBLOAD_FUN(load_objects);
DECLARE_DBLOAD_FUN(load_resets);
DECLARE_DBLOAD_FUN(load_rooms);
DECLARE_DBLOAD_FUN(load_shops);
DECLARE_DBLOAD_FUN(load_omprogs);
DECLARE_DBLOAD_FUN(load_olimits);
DECLARE_DBLOAD_FUN(load_specials);
DECLARE_DBLOAD_FUN(load_practicers);
DECLARE_DBLOAD_FUN(load_resetmsg);
DECLARE_DBLOAD_FUN(load_aflag);

DECLARE_DBINIT_FUN(init_area);

DBFUN dbfun_areas[] = {
	{ "AREA",		load_area	},
	{ "AREADATA",		load_areadata	},
	{ "HELPS",		load_helps	},
//	{ "MOBOLD",		load_old_mob	},
	{ "MOBILES",		load_mobiles	},
	{ "MOBPROGS",		load_mobprogs	},
//	{ "OBJOLD",		load_old_obj	},
	{ "OBJECTS",		load_objects	},
	{ "RESETS",		load_resets	},
	{ "ROOMS",		load_rooms	},
	{ "SHOPS",		load_shops	},
	{ "OMPROGS",		load_omprogs	},
	{ "OLIMITS",		load_olimits	},
	{ "SPECIALS",		load_specials	},
	{ "PRACTICERS",		load_practicers	},
	{ "RESETMESSAGE",	load_resetmsg	},
	{ "flag_t",		load_aflag	},
	{ NULL }
};

DBDATA db_areas = { dbfun_areas, init_area };

AREA_DATA *		area_current;

//static int	slot_lookup	(int slot);
//static void	convert_mobile	(MOB_INDEX_DATA *pMobIndex);

DBINIT_FUN(init_area)
{
	area_current = NULL;
}

/*
 * Snarf an 'area' header line.
 */
DBLOAD_FUN(load_area)
{
	AREA_DATA *pArea;

	pArea = alloc_perm(sizeof(*pArea));
	pArea->reset_first	= NULL;
	pArea->reset_last	= NULL;
	pArea->help_first	= NULL;
	pArea->help_last	= NULL;
	free_string(fread_string(fp));		/* file name */
	pArea->file_name	= get_filename(filename);
	pArea->builders		= str_empty;

	pArea->security		= 9;
	pArea->vnum		= top_area;

	pArea->name		= fread_string(fp);
	fread_letter(fp);			/* '{' */
	pArea->min_level	= fread_number(fp);
	pArea->max_level	= fread_number(fp);
	fread_letter(fp);			/* '}' */
	pArea->credits		= str_dup(fread_word(fp));	
	free_string(fread_string(fp));
	pArea->min_vnum		= fread_number(fp);
	pArea->max_vnum		= fread_number(fp);
	pArea->age		= 15;
	pArea->nplayer		= 0;
	pArea->empty		= FALSE;
	pArea->count		= 0;
	pArea->resetmsg		= NULL;
	pArea->flags		= 0;
	pArea->wpd10		= 5;

	if (area_first == NULL)
		area_first = pArea;
	if (area_last != NULL) 
		area_last->next = pArea;

	area_last	= pArea;
	pArea->next	= NULL;
	area_current	= pArea;

	top_area++;
}

/* OLC
 * Snarf an 'area' header line.   Check this format.  MUCH better.  Add fields
 * too.
 *
 * #AREAFILE
 * Name   { All } Locke    Newbie School~
 * Repop  A teacher pops in the room and says, 'Repop coming!'~
 * Recall 3001
 * End
 */
DBLOAD_FUN(load_areadata)
{
	AREA_DATA *	pArea;
	char *		word;
	bool		fMatch;

	pArea			= alloc_perm(sizeof(*pArea));
	pArea->age		= 15;
	pArea->wpd10		= 5;
	pArea->nplayer		= 0;
	pArea->file_name	= get_filename(filename);
	pArea->builders		= str_empty;
	pArea->vnum		= top_area;
	pArea->name		= str_dup("New Area");
	pArea->security		= 9;                    /* 9 -- Hugin */
	pArea->min_vnum		= 0;
	pArea->max_vnum		= 0;
	pArea->flags		= 0;
	pArea->min_level	= 0;
	pArea->max_level	= 0;          
	pArea->resetmsg		= NULL;
 
	for (; ;) {
		word   = feof(fp) ? "End" : fread_word(fp);
		fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'B':
			SKEY("Builders", pArea->builders);
			break;
		case 'C':
			//KEY("Clan", pArea->clan, fread_clan(fp));
			SKEY("Credits", pArea->credits);
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				fMatch = TRUE;
				if (area_first == NULL)
			        	area_first = pArea;
				if (area_last  != NULL)
			        	area_last->next = pArea;
				area_last	= pArea;
				pArea->next	= NULL;
				area_current	= pArea;
				top_area++;
				return;
			}
			break;
		case 'F':
			KEY("Flags", pArea->flags,
			    fread_fstring(area_flags, fp));
			break;
		case 'L':
			if (!str_cmp(word, "LevelRange")) {
				pArea->min_level = fread_number(fp);
				pArea->max_level = fread_number(fp);
			}
			break;
		case 'N':
			SKEY("Name", pArea->name);
			break;
		case 'R':
			MLSKEY("ResetMessage", pArea->resetmsg);
			break;
		case 'S':
			KEY("Security", pArea->security, fread_number(fp));
			break;
		case 'V':
			if (!str_cmp(word, "VNUMs")) {
				pArea->min_vnum = fread_number(fp);
				pArea->max_vnum = fread_number(fp);
			}
			break;
		case 'W':
			KEY("WPD10", pArea->wpd10, fread_number(fp));
			break;
		}
	}
}
 
/*
 * Snarf a help section.
 */
DBLOAD_FUN(load_helps)
{
	HELP_DATA *pHelp;
	int level;
	const char *keyword;

	if (!area_current) {  /* OLC */
		db_error("load_helps", "no #AREA seen yet.");
		return;
	}

	for (; ;) {
		level		= fread_number(fp);
		keyword		= fread_string(fp);
	
		if (keyword[0] == '$')
			break;
	
		pHelp		= help_new();
		pHelp->level	= level;
		pHelp->keyword	= keyword;
		pHelp->text	= mlstr_fread(fp);

		help_add(area_current, pHelp);
	}
}

/*
 * Snarf a mob section.  old style 
 */
#if 0
DBLOAD_FUN(load_old_mob)
{
	MOB_INDEX_DATA *pMobIndex;
	/* for race updating */
	int race;
	char name[MAX_STRING_LENGTH];

	if (!area_current) {  /* OLC */
		db_error("load_old_mob", "no #AREA seen yet.");
		return;
	}

	for (; ;) {
		int vnum;
		char letter;
		int iHash;

		letter	= fread_letter(fp);
		if (letter != '#') {
			db_error("load_old_mob", "# not found.");
			return;
		}

		vnum	= fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_mob_index(vnum)) {
			db_error("load_old_mob", "vnum %d duplicated.", vnum);
			return;
		}
		fBootDb = TRUE;

		pMobIndex		= alloc_perm(sizeof(*pMobIndex));
		pMobIndex->short_descr	= NULL;
		pMobIndex->long_descr	= NULL;
		pMobIndex->description	= NULL;

		pMobIndex->vnum		= vnum;
		pMobIndex->name	= fread_string(fp);
		pMobIndex->short_descr	= mlstr_fread(fp);
		pMobIndex->long_descr	= mlstr_fread(fp);
		pMobIndex->description	= mlstr_fread(fp);

		pMobIndex->act		= fread_flags(fp) | ACT_NPC;
		pMobIndex->affected_by	= fread_flags(fp);

		pMobIndex->practicer	= 0;
		pMobIndex->pShop	= NULL;

		pMobIndex->alignment	= fread_number(fp);
		letter			= fread_letter(fp);
		pMobIndex->level	= fread_number(fp);
		/*
		 * The unused stuff is for imps who want to use the old-style
		 * stats-in-files method.
		 */
				  fread_number(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* 'd'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* '+'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* 'd'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		/* '+'	*/	  fread_letter(fp);	/* Unused */
				  fread_number(fp);	/* Unused */
		pMobIndex->wealth       = fread_number(fp)/20;	
		/* xp can't be used! */	  fread_number(fp);	/* Unused */
		pMobIndex->start_pos	= fread_number(fp);	/* Unused */
		pMobIndex->default_pos	= fread_number(fp);	/* Unused */

		if (pMobIndex->start_pos < POS_SLEEPING)
			pMobIndex->start_pos = POS_STANDING;
		if (pMobIndex->default_pos < POS_SLEEPING)
			pMobIndex->default_pos = POS_STANDING;

		/*
		 * Back to meaningful values.
		 */
		pMobIndex->sex			= fread_number(fp);

		/* compute the race BS */
		one_argument(pMobIndex->name, name, sizeof(name));
 
		if (name[0] == '\0' || (race =  rn_lookup(name)) == 0) {
			race_t *r;

			/* fill in with blanks */
			pMobIndex->race = rn_lookup("human");
			r = RACE(pMobIndex->race);
			pMobIndex->affected_by = pMobIndex->affected_by |
						 r->aff;
			pMobIndex->off_flags = OFF_DODGE | OFF_DISARM |
				OFF_TRIP | ASSIST_VNUM;
			pMobIndex->imm_flags = 0;
			pMobIndex->res_flags = 0;
			pMobIndex->vuln_flags = 0;
			pMobIndex->form = FORM_EDIBLE | FORM_SENTIENT |
				FORM_BIPED | FORM_MAMMAL;
			pMobIndex->parts = PART_HEAD | PART_ARMS | PART_LEGS |
				PART_HEART | PART_BRAINS | PART_GUTS;
		}
		else {
			race_t *r = RACE(race);

			pMobIndex->race = race;
			pMobIndex->affected_by = 
				pMobIndex->affected_by | r->aff;
			pMobIndex->off_flags = OFF_DODGE | OFF_DISARM |
				OFF_TRIP | ASSIST_RACE | r->off;
			pMobIndex->imm_flags = r->imm;
			pMobIndex->res_flags = r->res;
			pMobIndex->vuln_flags = r->vuln;
			pMobIndex->form = r->form;
			pMobIndex->parts = r->parts;
		}

		if (letter != 'S') {
			db_error("load_old_mob", "vnum %d non-S.", vnum);
			return;
		}

		convert_mobile(pMobIndex);		/* ROM OLC */

		iHash			= vnum % MAX_KEY_HASH;
		pMobIndex->next		= mob_index_hash[iHash];
		mob_index_hash[iHash]	= pMobIndex;
		top_mob_index++;
							/* OLC */
		top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;
		vnum_check(area_current, vnum);			/* OLC */
	}
}

#endif

/*
 * Load mobprogs section
 */
DBLOAD_FUN(load_mobprogs)
{
    MPCODE *mpcode;

    if (area_current == NULL) {
	db_error("load_mobprogs", "no #AREA seen yet.");
	return;
    }

    for (; ;)
    {
	int vnum;
	const char *code;
	char letter;

	letter		  = fread_letter(fp);
	if (letter != '#') {
	    db_error("load_mobprogs", "# not found.");
	    return;
	}

	vnum		 = fread_number(fp);
	if (vnum == 0)
	    break;

	if (mpcode_lookup(vnum) != NULL) {
	    db_error("load_mobprogs", "vnum %d duplicated.", vnum);
	    return;
	}
	code 		= fread_string(fp);

	mpcode		= mpcode_new();
	mpcode->vnum  	= vnum;
	mpcode->code  	= code;
	mpcode_add(mpcode);
    }
}

/*
 * Snarf an obj section.  old style 
 */
#if 0
DBLOAD_FUN(load_old_obj)
{
	OBJ_INDEX_DATA *pObjIndex;

	if (!area_current) {
		db_error("load_old_obj", "no #AREA seen yet.");
		return;
	}

	for (; ;) {
		int vnum;
		char letter;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#') {
			db_error("load_old_obj", "# not found.");
			return;
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_obj_index(vnum)) {
			db_error("load_old_obj", "vnum %d duplicated.", vnum);
			return;
		}
		fBootDb = TRUE;

		pObjIndex		= alloc_perm(sizeof(*pObjIndex));
		pObjIndex->short_descr	= NULL;
		pObjIndex->description	= NULL;
		pObjIndex->vnum		= vnum;
		pObjIndex->reset_num	= 0;

		pObjIndex->name		= fread_string(fp);
		pObjIndex->short_descr	= mlstr_fread(fp);
		pObjIndex->description	= mlstr_fread(fp);
		/* Action description */  fread_string(fp);
		pObjIndex->material	= str_dup("copper");

		pObjIndex->item_type	= fread_number(fp);
//		pObjIndex->extra_flags	= fread_flags(fp) | ITEM_OLDSTYLE;
		pObjIndex->extra_flags	= fread_flags(fp);
		pObjIndex->wear_flags	= fread_flags(fp);
		pObjIndex->value[0]	= fread_number(fp);
		pObjIndex->value[1]	= fread_number(fp);
		pObjIndex->value[2]	= fread_number(fp);
		pObjIndex->value[3]	= fread_number(fp);
		pObjIndex->value[4]	= 0;
		pObjIndex->level	= 0;
		pObjIndex->condition 	= 100;
		pObjIndex->weight	= fread_number(fp);
		pObjIndex->cost		= fread_number(fp);	/* Unused */
//		if (vnum > 50000 && vnum < 50100)
//			pObjIndex->cost	= fread_number(fp);
		/* Cost per day */	  fread_number(fp);
		pObjIndex->limit	= -1;
		pObjIndex->oprogs	= NULL;

		if (pObjIndex->item_type == ITEM_WEAPON)
			if (is_name("two",pObjIndex->name) 
			||  is_name("two-handed",pObjIndex->name) 
			||  is_name("claymore",pObjIndex->name))
				SET_BIT(pObjIndex->value[4], WEAPON_TWO_HANDS);

		for (; ;) {
			char letter;

			letter = fread_letter(fp);

			if (letter == 'A') {
				AFFECT_DATA *paf;

				paf		= alloc_perm(sizeof(*paf));
				paf->where	= TO_OBJECT;
				paf->type	= -1;
				paf->level	= 20; /* RT temp fix */
				paf->duration	= -1;
				paf->location	= fread_number(fp);
				paf->modifier	= fread_number(fp);
				paf->bitvector	= 0;
				SLIST_ADD(AFFECT_DATA,
					  pObjIndex->affected, paf);
				top_affect++;
			}

			else if (letter == 'E') 
				ed_fread(fp, &pObjIndex->ed);
			else {
				xungetc(letter, fp);
				break;
			}
		}

		/* fix armors */
		if (pObjIndex->item_type == ITEM_ARMOR) {
			pObjIndex->value[1] = pObjIndex->value[0];
			pObjIndex->value[2] = pObjIndex->value[1];
		}

		/*
		 * Translate spell "slot numbers" to internal "skill numbers."
		 */
		switch (pObjIndex->item_type) {
		case ITEM_PILL:
		case ITEM_POTION:
		case ITEM_SCROLL:
			pObjIndex->value[1] = slot_lookup(pObjIndex->value[1]);
			pObjIndex->value[2] = slot_lookup(pObjIndex->value[2]);
			pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
			pObjIndex->value[4] = slot_lookup(pObjIndex->value[4]);
			break;

		case ITEM_STAFF:
		case ITEM_WAND:
			pObjIndex->value[3] = slot_lookup(pObjIndex->value[3]);
			break;
		}

		iHash			= vnum % MAX_KEY_HASH;
		pObjIndex->next		= obj_index_hash[iHash];
		obj_index_hash[iHash]	= pObjIndex;
		top_obj_index++;
								/* OLC */
		top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj;
		vnum_check(area_current, vnum);			/* OLC */
	}
}
#endif

/*
 * Snarf a reset section. Adjust levels of ITEM_OLDSTYLE objects on the fly.
 * if loading an old-style area it's assumed that #SHOPS section
 * (if any) has already been parsed
 */
DBLOAD_FUN(load_resets)
{
	MOB_INDEX_DATA *pLastMob = NULL;
	ROOM_INDEX_DATA *pLastRoom = NULL;

	if (area_current == NULL) {
		db_error("load_resets", "no #AREA seen yet.");
		return;
	}

	for (; ;) {
		RESET_DATA *pReset;

		ROOM_INDEX_DATA *pRoom;
		EXIT_DATA *pexit = NULL;
		char letter;
		OBJ_INDEX_DATA *pObj;

		if ((letter = fread_letter(fp)) == 'S')
			break;

		if (letter == '*') {
			fread_to_eol(fp);
			continue;
		}

		pReset		= alloc_perm(sizeof(*pReset));
		pReset->command	= letter;
		/* if_flag */	  fread_number(fp);
		pReset->arg1	= fread_number(fp);
		pReset->arg2	= fread_number(fp);
		pReset->arg3	= (letter == 'G' || letter == 'R') ?
				  0 : fread_number(fp);
		switch(letter)
		{
			default:
				pReset->arg4 = 0; 
				break;
			case 'P':
			case 'M':
			case 'U':
			case 'N':
			case 'A':
				pReset->arg4 = fread_number(fp);
				break;
		}
		fread_to_eol(fp);

		/*
		 * Validate parameters.
		 * We're calling the index functions for the side effect.
		 */
		switch (letter) {
		default:
			db_error("load_resets", "bad command '%c'.", letter);
			return;

		case 'M':
			if ((pLastMob = get_mob_index(pReset->arg1)) == NULL
			||  (pRoom = get_room_index(pReset->arg3)) == NULL)
				break;

			new_reset(pRoom, pReset);
			pLastRoom = pRoom;
			break;

		case 'O':
		case 'B':
		case 'H':
			if ((pObj = get_obj_index(pReset->arg1)) == NULL
			||  (pRoom = get_room_index(pReset->arg3)) == NULL)
				break;

			pObj->reset_num++;
			new_reset(pRoom, pReset);
			pLastRoom = pRoom;

/*			if (IS_SET(pObj->extra_flags, ITEM_OLDSTYLE)) {
				if (!pLastMob) {
					db_error("load_resets",
						 "can't calculate obj level: "
						 "no mob reset yet");
					return;
				}
				pObj->level = pObj->level < 1 ?
					pLastMob->level :
					UMIN(pLastMob->level, pObj->level);
			}
 */
			break;

		case 'P':
		case 'U':
		case 'N':
		case 'A':
			{
			OBJ_INDEX_DATA *pObjTo;

			if ((pObj = get_obj_index(pReset->arg1)) == NULL
			||  (pObjTo = get_obj_index(pReset->arg3)) == NULL)
				break;

			if (!pLastRoom) {
				db_error("load_resets", "room undefined");
				return;
			}

			new_reset(pLastRoom, pReset);
			pObj->reset_num++;

/*			if (IS_SET(pObj->extra_flags, ITEM_OLDSTYLE))
				pObj->level = pObj->level < 1 ?
					pObjTo->level :
					UMIN(pObjTo->level, pObj->level);
 */
			break;
		}

		case 'G':
		case 'E':
			if ((pObj = get_obj_index(pReset->arg1)) == NULL)
				break;

			if (!pLastRoom) {
				db_error("load_resets", "room undefined");
				return;
			}

			pObj->reset_num++;
			new_reset(pLastRoom, pReset);

/*			if (IS_SET(pObj->extra_flags, ITEM_OLDSTYLE)) {
				if (!pLastMob) {
					db_error("load_resets",
						 "can't calculate obj level: "
						 "no mob reset yet");
					return;
				}
				if (pLastMob->pShop) {
					switch(pObj->item_type) {
					default:
						pObj->level =
							UMAX(0, pObj->level);
						break;
					case ITEM_PILL:
					case ITEM_POTION:
						pObj->level =
							UMAX(5, pObj->level);
						break;
					case ITEM_SCROLL:
					case ITEM_ARMOR:
					case ITEM_WEAPON:
						pObj->level =
							UMAX(10, pObj->level);
						break;
					case ITEM_WAND:
					case ITEM_TREASURE:
						pObj->level =
							UMAX(15, pObj->level);
						break;
					case ITEM_STAFF:
						pObj->level =
							UMAX(20, pObj->level);
						break;
					}
				}
				else
					pObj->level = pObj->level < 1 ?
						pLastMob->level :
						UMIN(pObj->level,
						     pLastMob->level);
			}
 */
			break;

		case 'D':
			if ((pRoom = get_room_index(pReset->arg1)) == NULL)
				break;

			if (pReset->arg2 < 0
			||  pReset->arg2 >= MAX_DIR
			||  (pexit = pRoom->exit[pReset->arg2]) == NULL
			||  !IS_SET(pexit->rs_flags, EX_ISDOOR | EX_BURYED)) {
				db_error("load_resets",
					 "'D': exit %d not door (or bury).",
					 pReset->arg2);
				return;
			}

			switch (pReset->arg3) {
			default:
				db_error("load_resets",
					 "'D': bad 'locks': %d.",
					 pReset->arg3);
				return;

			case 0:
				break;
			case 1:
				SET_BIT(pexit->rs_flags, EX_CLOSED);
				SET_BIT(pexit->exit_info, EX_CLOSED);
				break;
			case 2:
				SET_BIT(pexit->rs_flags, EX_CLOSED | EX_LOCKED);
				SET_BIT(pexit->exit_info, 
					EX_CLOSED | EX_LOCKED);
				break;
			case 3:
				SET_BIT(pexit->rs_flags, EX_BURYED);
				SET_BIT(pexit->exit_info, EX_BURYED);
				break;
			case 4:
				SET_BIT(pexit->rs_flags, EX_BURYED | EX_CLOSED);
				SET_BIT(pexit->exit_info, EX_BURYED | EX_CLOSED);
				break;
			case 5:
				SET_BIT(pexit->rs_flags,
					EX_BURYED | EX_CLOSED | EX_LOCKED);
				SET_BIT(pexit->exit_info,
					EX_BURYED | EX_CLOSED | EX_LOCKED);
				break;
			}

			break;

		case 'R':
			break;

/*			if ((pRoom = get_room_index(pReset->arg1)) == NULL)
				break;

			if (pReset->arg2 < 0 || pReset->arg2 > MAX_DIR) {
				db_error("load_resets", "'R': bad exit %d.",
					pReset->arg2);
				break;
			}

			if (pRoom)
				new_reset(pRoom, pReset);

			break;
 */
		}
	}
}

/*
 * Snarf a room section.
 */
DBLOAD_FUN(load_rooms)
{
	ROOM_INDEX_DATA *pRoomIndex;

	if (area_current == NULL) {
		db_error("load_rooms", "no #AREA seen yet.");
		return;
	}

	for (; ;) {
		int vnum;
		char letter;
		int door;
		int iHash;

		letter = fread_letter(fp);
		if (letter != '#') {
			db_error("load_rooms", "# not found.");
			return;
		}

		vnum = fread_number(fp);
		if (vnum == 0)
			break;

		fBootDb = FALSE;
		if (get_room_index(vnum)) {
			db_error("load_rooms", "vnum %d duplicated.", vnum);
			return;
		}
		fBootDb = TRUE;

		pRoomIndex		= alloc_perm(sizeof(*pRoomIndex));
		pRoomIndex->name	= NULL;
		pRoomIndex->description	= NULL;

		pRoomIndex->owner	= str_dup(str_empty);
		pRoomIndex->people	= NULL;
		pRoomIndex->contents	= NULL;
		pRoomIndex->ed		= NULL;
		pRoomIndex->history     = NULL;
		pRoomIndex->area	= area_current;
		pRoomIndex->vnum	= vnum;
		pRoomIndex->name	= mlstr_fread(fp);
		pRoomIndex->description	= mlstr_fread(fp);
		/* Area number */	  fread_number(fp);
		pRoomIndex->room_flags	= fread_flags(fp);
 
		pRoomIndex->sector_type	= fread_fword(sector_types, fp);
		pRoomIndex->light	= 0;
		for (door = 0; door <= 5; door++)
			pRoomIndex->exit[door] = NULL;

		/* defaults */
		pRoomIndex->heal_rate = 100;
		pRoomIndex->mana_rate = 100;
		pRoomIndex->affected = NULL;
		pRoomIndex->affected_by = 0;
		pRoomIndex->aff_next = NULL;

		for (; ;) {
			letter = fread_letter(fp);

			if (letter == 'S')
				break;

			if (letter == 'H') /* healing room */
				pRoomIndex->heal_rate = fread_number(fp);
		
			else if (letter == 'M') /* mana room */
				pRoomIndex->mana_rate = fread_number(fp);

			else if (letter == 'D') {
				EXIT_DATA *pexit;
				int locks;
	
				door = fread_number(fp);
				if (door < 0 || door > 5) {
					db_error("load_rooms",
						 "vnum %d has bad door number.",
						 vnum);
					return;
				}
								
				pexit			= alloc_perm(sizeof(*pexit));
				pexit->distance		= fread_number(fp);
				
				pexit->description	= mlstr_fread(fp);
				pexit->keyword		= fread_string(fp);
				pexit->exit_info	= 0;
				pexit->rs_flags		= 0;	/* OLC */
				locks			= fread_flags(fp);
				pexit->key		= fread_number(fp);
				pexit->to_room.vnum	= fread_number(fp);
				pexit->orig_door	= door;	/* OLC */

				if (!IS_SET(locks, EX_BITVAL)) {
					switch (locks) {
					case 1:
						pexit->exit_info = EX_ISDOOR;
						pexit->rs_flags  = EX_ISDOOR;
						break;
					case 2:
						pexit->exit_info = EX_ISDOOR |
								   EX_PICKPROOF;
						pexit->rs_flags  = EX_ISDOOR |
								   EX_PICKPROOF;
						break;
					case 3:
						pexit->exit_info = EX_ISDOOR |
								   EX_NOPASS;
						pexit->rs_flags  = EX_ISDOOR |
								   EX_NOPASS;
						break;
					case 4:
						pexit->exit_info = EX_ISDOOR |
								   EX_NOPASS |
								   EX_PICKPROOF;
						pexit->rs_flags  = EX_ISDOOR |
								   EX_NOPASS |
								   EX_PICKPROOF;
						break;
					}
				}
				else {
					pexit->exit_info = locks;
					pexit->rs_flags = locks;
				}
	
				pRoomIndex->exit[door] = pexit;
				top_exit++;
			}
			else if (letter == 'E') 
				ed_fread(fp, &pRoomIndex->ed);
			else if (letter == 'O') {
				if (pRoomIndex->owner[0] != '\0') {
					db_error("load_rooms",
						 "duplicate owner.");
					return;
				}

				pRoomIndex->owner = fread_string(fp);
			}/*
			else if (letter == 'C') {
				if (pRoomIndex->clan) {
					db_error("load_rooms",
						 "duplicate clan.");
					return;
				}
				pRoomIndex->clan = fread_clan(fp);
			}*/
			else {
				db_error("load_rooms",
					 "vnum %d has flag '%c' (not 'DES').",
					 vnum, letter);
				return;
			}
		}

		iHash			= vnum % MAX_KEY_HASH;
		pRoomIndex->next	= room_index_hash[iHash];
		room_index_hash[iHash]	= pRoomIndex;
		top_room++;
								/* OLC */
		top_vnum_room = top_vnum_room < vnum ? vnum : top_vnum_room;
		vnum_check(area_current, vnum);			/* OLC */
	}
}

/*
 * Snarf a shop section.
 */
DBLOAD_FUN(load_shops)
{
	SHOP_DATA *pShop;
	const char *str;
	const char *argument;
	char word[MAX_INPUT_LENGTH];

	for (; ;) {
		MOB_INDEX_DATA *pMobIndex;
		int iTrade;

		pShop			= new_shop();
		pShop->keeper		= fread_number(fp);
		if (pShop->keeper == 0)
 		    break;
		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++)
		    pShop->buy_type[iTrade]	= fread_number(fp);
		pShop->profit_buy	= fread_number(fp);
		pShop->profit_sell	= fread_number(fp);
		pShop->open_hour	= fread_number(fp);
		pShop->close_hour	= fread_number(fp);
		pShop->flags		= fread_flags(fp);
		str			= fread_string(fp);
					  fread_to_eol(fp);
		
			/* Load list of buys items (vnums) */
		argument = str;
		for (;;) {
			argument = one_argument(argument, word, sizeof(word));
			if (word[0] == '\0')
				break;
			if (is_number(word))
			{
				int *ip;
				
				ip = varr_enew(&pShop->vnums_of_buy);
				*ip = atoi(word);
			} else {
				db_error("load_shops", "word '%s' must be numeric.", word);
			}
		}
		
		free_string(str);
		pMobIndex		= get_mob_index(pShop->keeper);
		pMobIndex->pShop	= pShop;

		if (shop_first == NULL)
		    shop_first = pShop;
		if (shop_last  != NULL)
		    shop_last->next = pShop;

		shop_last	= pShop;
		pShop->next	= NULL;
		top_shop++;
	}

	return;
}

/*
 * Snarf spec proc declarations.
 */
DBLOAD_FUN(load_specials)
{
	for (; ;)
	{
		MOB_INDEX_DATA *pMobIndex;
		char letter;

		switch (letter = fread_letter(fp))
		{
		default:
		    db_error("load_specials", "letter '%c' not *MS.", letter);
		    return;

		case 'S':
		    return;

		case '*':
		    break;

		case 'M':
		    pMobIndex		= get_mob_index	(fread_number (fp));
		    pMobIndex->spec_fun	= spec_lookup	(fread_word   (fp));
		    if (pMobIndex->spec_fun == 0) {
			db_error("load_specials", "'M': vnum %d.",
				 pMobIndex->vnum);
			return;
		    }
		    break;
		}

		fread_to_eol(fp);
	}
}

/*
 * Snarf can prac declarations.
 */
DBLOAD_FUN(load_practicers)
{
	for (; ;) {
		MOB_INDEX_DATA *pMobIndex;
		char letter;

		switch (letter = fread_letter(fp)) {
		default:
			db_error("load_practicers", "letter '%c' not *MS.",
				 letter);
			return;

		case 'S':
			return;

		case '*':
			break;

		case 'M':
			pMobIndex = get_mob_index(fread_number(fp));
			SET_BIT(pMobIndex->practicer,
				fread_fstring(skill_groups, fp));
			break;
		}

		fread_to_eol(fp);
	}
}

DBLOAD_FUN(load_olimits)
{
	int vnum;
	int limit;
	char ch;
	OBJ_INDEX_DATA *pIndex;

	for (ch = fread_letter(fp); ch != 'S'; ch = fread_letter(fp)) {
		switch(ch) {
		case 'O':
		  vnum = fread_number(fp);
		  limit = fread_number(fp);
		  if ((pIndex = get_obj_index(vnum)) == NULL) {
			db_error("Load_olimits", "bad vnum %d", vnum);
			return;
		  }
		  else
			pIndex->limit = limit;
		  break;

		case '*':
		  fread_to_eol(fp);
		  break;

		default:
		  db_error("load_olimits", "bad command '%c'", ch);
		  return;
		}
	}
}

DBLOAD_FUN(load_resetmsg)
{
	area_current->resetmsg = mlstr_fread(fp);
}

DBLOAD_FUN(load_aflag)
{
	area_current->flags = fread_flags(fp);
}

#if 0
/*
 *	Old format bits
 */
#define IMM_SUMMON		(A)
#define IMM_CHARM		(B)
#define IMM_MAGIC		(C)
#define IMM_WEAPON		(D)
#define IMM_BASH		(E)
#define IMM_PIERCE		(F)
#define IMM_SLASH		(G)
#define IMM_FIRE		(H)
#define IMM_COLD		(I)
#define IMM_LIGHTNING		(J)
#define IMM_ACID		(K)
#define IMM_POISON		(L)
#define IMM_NEGATIVE		(M)
#define IMM_HOLY		(N)
#define IMM_ENERGY		(O)
#define IMM_MENTAL		(P)
#define IMM_DISEASE		(Q)
#define IMM_DROWNING		(R)
#define IMM_LIGHT		(S)
#define IMM_SOUND		(T)
#define IMM_STEAL		(U)
//				(V)
#define IMM_WOOD		(X)
#define IMM_SILVER		(Y)
#define IMM_IRON		(Z)

typedef struct tables_convert_irv_t {
	flag32_t	old;
	flag32_t	new;
} tables_convert_irv_t;

tables_convert_irv_t tables_convert_irv[] =
{
	{ IMM_SUMMON,	MAGIC_MENTAL	},
	{ IMM_CHARM,	MAGIC_MENTAL	},
	{ IMM_MAGIC,	MAGIC_ALL	},
	{ IMM_WEAPON,	FORCE_WEAPON	},
	{ IMM_BASH,	FORCE_BASH	},
	{ IMM_PIERCE,	FORCE_PIERCE	},
	{ IMM_SLASH,	FORCE_SLASH	},
	{ IMM_FIRE,	MAGIC_FIRE	},
	{ IMM_COLD,	FORCE_COLD	},
	{ IMM_LIGHTNING,MAGIC_AIR	},
	{ IMM_ACID,	FORCE_ACID	},
	{ IMM_POISON,	FORCE_POISON	},
	{ IMM_NEGATIVE,	MAGIC_DEATH	},
	{ IMM_HOLY,	MAGIC_HOLY	},
	{ IMM_ENERGY,	FORCE_ENERGY	},
	{ IMM_MENTAL,	MAGIC_MENTAL	},
	{ IMM_DISEASE,	FORCE_DISEASE	},
	{ IMM_DROWNING,	FORCE_DROWNING	},
	{ IMM_LIGHT,	FORCE_LIGHT	},
	{ IMM_SOUND,	MAGIC_AIR	},
	{ IMM_STEAL,	FORCE_STEAL	},
	{ IMM_WOOD,	FORCE_WOOD	},
	{ IMM_SILVER,	FORCE_SILVER	},
	{ IMM_IRON,	FORCE_IRON	},
	{ NULL }
};

/*
 *	This function uses for convert old imm/res/vuln bits to new format
 *			--
 *			*** Xor **
 */
flag32_t	convert_to_irv(flag32_t old)
{
	const tables_convert_irv_t *ct = tables_convert_irv;
	flag32_t new = 0;
	
	while (ct->old != NULL) {
		if (IS_SET(old, ct->old))
			SET_BIT(new, ct->new);
		ct++;
	}
	
	return new;
}
#endif

int slang_lookup(const char *name);
/*
 * Snarf a mob section.  new style
 */
DBLOAD_FUN(load_mobiles)
{
    MOB_INDEX_DATA *pMobIndex;
 
    if (!area_current) {
        db_error("load_mobiles", "no #AREA seen yet.");
	return;
    }

    for (; ;) {
	race_t *r;
        int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter(fp);
        if (letter != '#') {
            db_error("load_mobiles", "# not found.");
	    return;
	}
 
        vnum                            = fread_number(fp);
        if (vnum == 0)
            break;
 
        fBootDb = FALSE;
        if (get_mob_index(vnum)) {
        	db_error("load_mobiles", "vnum %d duplicated.", vnum);
		return;
	}
        fBootDb = TRUE;
 
        pMobIndex                       = alloc_perm(sizeof(*pMobIndex));
        pMobIndex->short_descr          = NULL;
        pMobIndex->long_descr           = NULL;
        pMobIndex->description          = NULL;

        pMobIndex->vnum                 = vnum;
        pMobIndex->slang		= gsn_common_slang;	/* SR */
	newmobs++;
        pMobIndex->name			= fread_string(fp);
        pMobIndex->short_descr		= mlstr_fread(fp);
        pMobIndex->long_descr		= mlstr_fread(fp);
        pMobIndex->description		= mlstr_fread(fp);
	pMobIndex->race		 	= rn_lookup(fread_string(fp));
	r = RACE(pMobIndex->race);

        pMobIndex->act                  = fread_flags(fp) | ACT_NPC | r->act;
        pMobIndex->affected_by          = fread_flags(fp) | r->aff;

	pMobIndex->practicer		= 0;
        pMobIndex->pShop                = NULL;

        pMobIndex->alignment            = fread_number(fp);

        pMobIndex->group                = fread_number(fp);

        pMobIndex->level                = fread_number(fp);

/*	if (abs(pMobIndex->alignment) > 400){
        	if (pMobIndex->alignment > 0)
        		pMobIndex->alignment = UMIN(1000, 400 + pMobIndex->level * 5);
        	else
        		pMobIndex->alignment = UMAX(-1000, -400 - pMobIndex->level * 5);
        }
 */
        pMobIndex->hitroll              = fread_number(fp);  

	/* read hit dice */
        pMobIndex->hit[DICE_NUMBER]     = fread_number(fp);  
        /* 'd'          */                fread_letter(fp); 
        pMobIndex->hit[DICE_TYPE]   	= fread_number(fp);
        /* '+'          */                fread_letter(fp);   
        pMobIndex->hit[DICE_BONUS]      = fread_number(fp); 

 	/* read mana dice */
	pMobIndex->mana[DICE_NUMBER]	= fread_number(fp);
					  fread_letter(fp);
	pMobIndex->mana[DICE_TYPE]	= fread_number(fp);
					  fread_letter(fp);
	pMobIndex->mana[DICE_BONUS]	= fread_number(fp);
	
/*	if (pMobIndex->mana[DICE_NUMBER] * pMobIndex->mana[DICE_TYPE] + pMobIndex->mana[DICE_BONUS] < 500){
		pMobIndex->mana[DICE_NUMBER] = pMobIndex->level;
		pMobIndex->mana[DICE_TYPE] = 25;
		pMobIndex->mana[DICE_BONUS] = 200;
	}
*/

	/* read damage dice */
	pMobIndex->damage[DICE_NUMBER]	= fread_number(fp);
					  fread_letter(fp);
	pMobIndex->damage[DICE_TYPE]	= fread_number(fp);
					  fread_letter(fp);
	pMobIndex->damage[DICE_BONUS]	= fread_number(fp);
	pMobIndex->dam_type		= attack_lookup(fread_word(fp));

	/* read armor class */
	pMobIndex->ac[AC_PIERCE]	= fread_number(fp) * 10;
	pMobIndex->ac[AC_BASH]		= fread_number(fp) * 10;
	pMobIndex->ac[AC_SLASH]		= fread_number(fp) * 10;
	pMobIndex->ac[AC_EXOTIC]	= fread_number(fp) * 10;

	/* read flags and add in data from the race table */
	pMobIndex->off_flags		= fread_flags(fp) | r->off;

	pMobIndex->imm_flags		= fread_flags(fp) | r->imm;
	pMobIndex->res_flags		= fread_flags(fp) | r->res;
	pMobIndex->vuln_flags		= fread_flags(fp) | r->vuln;

	/* vital statistics */
	pMobIndex->start_pos		= flag_value(position_table,
						     fread_word(fp));
	pMobIndex->default_pos		= flag_value(position_table,
						     fread_word(fp));
	pMobIndex->sex			= flag_value(sex_table, fread_word(fp));

	pMobIndex->wealth               = fread_number(fp);

/*	if (vnum > 99){
		if (pMobIndex->level < 25)
			pMobIndex->wealth	= pMobIndex->level;
		else if (pMobIndex->level < 70)
			pMobIndex->wealth	= 25 + (pMobIndex->level - 25) / 2;
		else
			pMobIndex->wealth	= 48 + (pMobIndex->level - 70) / 4;
	}
*/
	
	pMobIndex->form			= fread_flags(fp) | r->form;
	pMobIndex->parts		= fread_flags(fp) | r->parts;
	/* size */
	pMobIndex->size			= fread_fword(size_table, fp);
	pMobIndex->material		= str_dup(fread_word(fp));
 
 	for (; ;)
        {
            letter = fread_letter(fp);

	    if (letter == 'A') {
		char *word;
		long vector;

                word	= fread_word(fp);
		vector	= fread_flags(fp);

		if (!str_prefix(word,"det"))
		    SET_BIT(pMobIndex->affected_by, vector);
	    }/*
	    else if (letter == 'C') {
		if (pMobIndex->clan) {
		    db_error("load_mobiles", "duplicate clan.");
		    return;
		}
		pMobIndex->clan = fread_clan(fp);
	    }*/
	    else if (letter == 'I') 
		pMobIndex->invis_level = fread_number(fp);
	    else if (letter == 'V') 
		pMobIndex->fvnum = fread_number(fp);
            else if (letter == 'F') {
		char *word;
		long vector;

                word                    = fread_word(fp);
		vector			= fread_flags(fp);

		if (!str_prefix(word,"act"))
		    REMOVE_BIT(pMobIndex->act,vector);
                else if (!str_prefix(word,"aff"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
		else if (!str_prefix(word,"off"))
		    REMOVE_BIT(pMobIndex->affected_by,vector);
		else if (!str_prefix(word,"imm"))
		    REMOVE_BIT(pMobIndex->imm_flags, vector);
		else if (!str_prefix(word,"res"))
		    REMOVE_BIT(pMobIndex->res_flags,vector);
		else if (!str_prefix(word,"vul"))
		    REMOVE_BIT(pMobIndex->vuln_flags,vector);
		else if (!str_prefix(word,"for"))
		    REMOVE_BIT(pMobIndex->form,vector);
		else if (!str_prefix(word,"par"))
		    REMOVE_BIT(pMobIndex->parts,vector);
		else {
		    db_error("flag remove", "flag not found.");
		    return;
		}
	     }
	     else if ( letter == 'M' )
	     {
		MPTRIG *mptrig;
		char *word;
		int type;
		const char *phrase;
		int vnum;
		
		word = fread_word(fp);
		if ((type = flag_value(mptrig_types, word)) == 0) {
			db_error("load_mobiles", "vnum %d: "
				   "'%s': invalid mob prog trigger",
				   pMobIndex->vnum, word);
			return;
		}

		vnum   = fread_number(fp);
		phrase = fread_string(fp);

		mptrig = mptrig_new(type, phrase, vnum);
		mptrig_add(pMobIndex, mptrig);
		free_string(phrase);
	     }
	     else if (letter == 'L')	/* Slang   - by SR */
	     {
		char	*tmp;
		int	sn;
		
		tmp = fread_word(fp);
		sn = slang_lookup(tmp);
		if (sn < 0
		|| !IS_SET(SKILL(sn)->group, GROUP_SLANG))
		{
			db_error("load_mobiles", "Slang %s: "
				"unknown slang(skill)",
				tmp);
			return;
		}
		pMobIndex->slang = sn;
	     }
	     else if (letter == 'S')	/* Saves vs Spell - by SR */
	     {
	     	int i;

	     	for (i = 0; i < QUANTITY_MAGIC + 1; i ++)
	     		pMobIndex->saving_throws[i] = fread_number(fp);
	     }
	     else
	     {
		xungetc(letter,fp);
		break;
	     }
	}

	if (IS_SET(pMobIndex->affected_by, AFF_SANCTUARY)
	&&  IS_EVIL(pMobIndex)) {
		touch_vnum(pMobIndex->vnum);
		REMOVE_BIT(pMobIndex->affected_by, AFF_SANCTUARY);
		SET_BIT(pMobIndex->affected_by, AFF_BLACK_SHROUD);
	}

        iHash                   = vnum % MAX_KEY_HASH;
        pMobIndex->next         = mob_index_hash[iHash];
        mob_index_hash[iHash]   = pMobIndex;
        top_mob_index++;
        top_vnum_mob = top_vnum_mob < vnum ? vnum : top_vnum_mob;  /* OLC */
        vnum_check(area_current, vnum);                            /* OLC */
    }
}

/*
 * Snarf an obj section. new style
 */
DBLOAD_FUN(load_objects)
{
    OBJ_INDEX_DATA *pObjIndex;
 
    if (!area_current) {
        db_error("load_objects", "no #AREA seen yet.");
	return;
    }

    for (; ;)
    {
        int vnum;
        char letter;
        int iHash;
 
        letter                          = fread_letter(fp);
        if (letter != '#') {
            db_error("load_objects", "# not found.");
	    return;
	}
 
        vnum                            = fread_number(fp);
        if (vnum == 0)
            break;
 
        fBootDb = FALSE;
        if (get_obj_index(vnum)) {
        	db_error("load_objects", "vnum %d duplicated.", vnum);
		return;
	}
        fBootDb = TRUE;
 
        pObjIndex                       = alloc_perm(sizeof(*pObjIndex));
        pObjIndex->short_descr          = NULL;
        pObjIndex->description          = NULL;

        pObjIndex->vnum                 = vnum;
	pObjIndex->reset_num		= 0;
	newobjs++;
        pObjIndex->name                 = fread_string(fp);
        pObjIndex->short_descr		= mlstr_fread(fp);
        pObjIndex->description		= mlstr_fread(fp);
        pObjIndex->material		= fread_string(fp);
	pObjIndex->oprogs		= NULL;

	pObjIndex->item_type		= fread_fword(item_types, fp);
        pObjIndex->extra_flags          = fread_flags(fp);
        pObjIndex->wear_flags           = fread_flags(fp);
	switch(pObjIndex->item_type)
	{
	case ITEM_WEAPON:
	    pObjIndex->value[0]		= flag_value(weapon_class,
						     fread_word(fp));
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= attack_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_flags(fp);
	    break;
	case ITEM_CONTAINER:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_flags(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= fread_number(fp);
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
        case ITEM_DRINK_CON:
	case ITEM_FOUNTAIN:
            pObjIndex->value[0]         = fread_number(fp);
            pObjIndex->value[1]         = fread_number(fp);
	    pObjIndex->value[2]		= liq_lookup(fread_word(fp));
            pObjIndex->value[3]         = fread_number(fp);
            pObjIndex->value[4]         = fread_number(fp);
            break;
	case ITEM_WAND:
	case ITEM_STAFF:
	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= fread_number(fp);
	    pObjIndex->value[2]		= fread_number(fp);
	    pObjIndex->value[3]		= sn_lookup(fread_word(fp));
	    pObjIndex->value[4]		= fread_number(fp);
	    break;
	case ITEM_POTION:
	case ITEM_PILL:
	case ITEM_SCROLL:
 	    pObjIndex->value[0]		= fread_number(fp);
	    pObjIndex->value[1]		= sn_lookup(fread_word(fp));
	    pObjIndex->value[2]		= sn_lookup(fread_word(fp));
	    pObjIndex->value[3]		= sn_lookup(fread_word(fp));
	    pObjIndex->value[4]		= sn_lookup(fread_word(fp));
	    break;
	case ITEM_MONEY:
	case ITEM_ARMOR:
	case ITEM_DIG:
	case ITEM_HORSE_TICKET:
		pObjIndex->value[0]	= fread_number(fp);
		pObjIndex->value[1]	= fread_number(fp);
		pObjIndex->value[2]	= fread_number(fp);
		pObjIndex->value[3]	= fread_number(fp);
		pObjIndex->value[4]	= fread_number(fp);
		break;
	default:
            pObjIndex->value[0]             = fread_flags(fp);
            pObjIndex->value[1]             = fread_flags(fp);
            pObjIndex->value[2]             = fread_flags(fp);
            pObjIndex->value[3]             = fread_flags(fp);
	    pObjIndex->value[4]		    = fread_flags(fp);
	    break;
	}
	pObjIndex->level		= fread_number(fp);
        pObjIndex->weight               = fread_number(fp);
        pObjIndex->cost                 = fread_number(fp); 

/*        if (vnum > 7900 && vnum < 7999)
        if (pObjIndex->item_type == ITEM_TREASURE
        	|| pObjIndex->item_type == ITEM_JEWELRY
        	|| pObjIndex->item_type == ITEM_GEM){
        	pObjIndex->cost		= pObjIndex->level * 10;
        } else if (pObjIndex->item_type != ITEM_MONEY
        	&& pObjIndex->item_type != ITEM_TRASH){
        	pObjIndex->cost		= pObjIndex->level * 2;
  	}
*/

        
        pObjIndex->limit                = -1; 

        /* condition */
        letter 				= fread_letter(fp);
	switch (letter)
 	{
	    case ('P') :		pObjIndex->condition = 100; break;
	    case ('G') :		pObjIndex->condition =  90; break;
	    case ('A') :		pObjIndex->condition =  75; break;
	    case ('W') :		pObjIndex->condition =  50; break;
	    case ('D') :		pObjIndex->condition =  25; break;
	    case ('B') :		pObjIndex->condition =  10; break;
	    case ('R') :		pObjIndex->condition =   0; break;
	    default:			pObjIndex->condition = 100; break;
	}
 
        for (; ;)
        {
            char letter;
 
            letter = fread_letter(fp);
 
            if (letter == 'A')
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm(sizeof(*paf));
		paf->where		= TO_OBJECT;
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number(fp);
                if (paf->location == APPLY_10_SKILL
                 || paf->location == APPLY_5_SKILL) {
                 	char *tmp;
                 	int sn;

                	tmp 		= fread_word(fp);
                	sn = sn_lookup(tmp);
                	if (sn < 0) {
                		db_error("load_obects", "unknown skill/spell.");
                		return;
                	}
                	paf->modifier	= sn;
                } else
                	paf->modifier	= fread_number(fp);
                paf->bitvector          = 0;
		SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
                top_affect++;
            }/*
	    else if (letter == 'C') {
		if (pObjIndex->clan) {
		    db_error("load_objects", "duplicate clan.");
		    return;
		}
		pObjIndex->clan = fread_clan(fp);
	    }*/
	    else if (letter == 'G')
		pObjIndex->gender = fread_fword(gender_table, fp);
	    else if (letter == 'F')
            {
                AFFECT_DATA *paf;
 
                paf                     = alloc_perm(sizeof(*paf));
		letter 			= fread_letter(fp);
		switch (letter)
	 	{
		case 'A':
                    paf->where          = TO_AFFECTS;
		    break;
		case 'I':
		    paf->where		= TO_IMMUNE;
		    break;
		case 'R':
		    paf->where		= TO_RESIST;
		    break;
		case 'V':
		    paf->where		= TO_VULN;
		    break;
		case 'D':
		    paf->where		= TO_AFFECTS;
		    break;
		default:
			db_error("load_objects",
				 "vnum %d: '%c': bad where on flag.",
			        pObjIndex->vnum, letter);
			return;
		}
                paf->type               = -1;
                paf->level              = pObjIndex->level;
                paf->duration           = -1;
                paf->location           = fread_number(fp);
               	if (paf->location == APPLY_10_SKILL
                 || paf->location == APPLY_5_SKILL) {
                 	char *tmp;
                 	int sn;

                	tmp 		= fread_word(fp);
                	sn = sn_lookup(tmp);
                	if (sn < 0) {
                		db_error("load_obects", "unknown skill/spell.");
                		return;
                	}
                	paf->modifier	= sn;
                } else
                	paf->modifier           = fread_number(fp);
                paf->bitvector          = fread_flags(fp);
		SLIST_ADD(AFFECT_DATA, pObjIndex->affected, paf);
                top_affect++;
            }
 
            else if (letter == 'E')
		ed_fread(fp, &pObjIndex->ed);
            else
            {
                xungetc(letter, fp);
                break;
            }
        }
 
        iHash                   = vnum % MAX_KEY_HASH;
        pObjIndex->next         = obj_index_hash[iHash];
        obj_index_hash[iHash]   = pObjIndex;
        top_obj_index++;
        top_vnum_obj = top_vnum_obj < vnum ? vnum : top_vnum_obj; /* OLC */
        vnum_check(area_current, vnum);				  /* OLC */
    }
}

/*
 * Snarf a mprog section
 */
DBLOAD_FUN(load_omprogs)
{
  char progtype[MAX_INPUT_LENGTH];
  char progname[MAX_INPUT_LENGTH];
  
    for (; ;) {
	OBJ_INDEX_DATA *pObjIndex;
	char letter;
	
	switch (letter = fread_letter(fp)) {
	default:
	    db_error("load_omprogs", "letter '%c' not *IMS.", letter);
	    return;

	case 'S':
	    return;

	case '*':
	    break;
	    
        case 'O':
	    pObjIndex = get_obj_index (fread_number (fp));
	    strnzcpy(progtype, sizeof(progtype), fread_word(fp));
	    strnzcpy(progname, sizeof(progname), fread_word(fp));
	    oprog_set(pObjIndex, progtype, progname);
	    break;
	
	case 'M':
	    /* will be removed at all after new mob prog code integration */
	    break;  
	}

	fread_to_eol(fp);
    }
}

/*
 * Lookup a skill by slot number.
 * Used for object loading.
 */
#if 0
static int slot_lookup(int slot)
{
	int sn;

	if (slot <= 0)
		return -1;

	for (sn = 0; sn < skills.nused; sn++)
		if (slot == SKILL(sn)->slot)
			return sn;

	db_error("slot_lookup", "bad slot %d.", slot);
	return -1;
}
#endif

/*****************************************************************************
 Name:		convert_mobile
 Purpose:	Converts an old format mob into new format
 Called by:	load_old_mob (db.c).
 Note:          Dug out of create_mob (db.c)
 Author:        Hugin
 ****************************************************************************/
#if 0
void convert_mobile(MOB_INDEX_DATA *pMobIndex)
{
	int i;
	int type, number, bonus;
	int level;

	level = pMobIndex->level;

	pMobIndex->act              |= ACT_WARRIOR;

	/*
	 * Calculate hit dice.  Gives close to the hitpoints
	 * of old format mobs created with create_mob()  (db.c)
	 * A high number of dice makes for less variance in mobiles
	 * hitpoints.
	 * (might be a good idea to reduce the max number of dice)
	 *
	 * The conversion below gives:

	level:     dice         min         max        diff       mean
	 1:      1d2+6        7 (7)        8 (8)       1 (1)       8 (8)
	 2:      1d3+15      16 (15)      18 (18)      2 (3)      17 (17)
	 3:      1d6+24      25 (24)      30 (30)      5 (6)      27 (27)
	 5:     1d17+42      43 (42)      59 (59)     16 (17)     51 (51)
	10:     3d22+96      99 (95)     162 (162)    63 (67)    131 ()
	15:     5d30+161    166 (159)    311 (311)   145 (150)   239 ()
	30:    10d61+416    426 (419)   1026 (1026)  600 (607)   726 ()
	50:   10d169+920    930 (923)   2610 (2610) 1680 (1688) 1770 ()

	The values in parenthesis give the values generated in create_mob.
		Diff = max - min.  Mean is the arithmetic mean.
	(hmm.. must be some roundoff error in my calculations.. smurfette got
	 1d6+23 hp at level 3 ? -- anyway.. the values above should be
	 approximately right..)

	 */

	type   = level*level*27/40;
	number = UMIN(type/40 + 1, 10); /* how do they get 11 ??? */
	type   = UMAX(2, type/number);
	bonus  = UMAX(0, level*(8 + level)*.9 - number*type);

	pMobIndex->hit[DICE_NUMBER]    = number;
	pMobIndex->hit[DICE_TYPE]      = type;
	pMobIndex->hit[DICE_BONUS]     = bonus;

	pMobIndex->mana[DICE_NUMBER]   = level;
	pMobIndex->mana[DICE_TYPE]     = 10;
	pMobIndex->mana[DICE_BONUS]    = 100;

	/*
	 * Calculate dam dice.  Gives close to the damage
	 * of old format mobs in damage()  (fight.c)
	 */
	type   = level*7/4;
	number = UMIN(type/8 + 1, 5);
	type   = UMAX(2, type/number);
	bonus  = UMAX(0, level*9/4 - number*type);

	pMobIndex->damage[DICE_NUMBER] = number;
	pMobIndex->damage[DICE_TYPE]   = type;
	pMobIndex->damage[DICE_BONUS]  = bonus;

	switch (number_range(1, 3)) {
		case (1): pMobIndex->dam_type =  3;       break;  /* slash  */
		case (2): pMobIndex->dam_type =  7;       break;  /* pound  */
		case (3): pMobIndex->dam_type = 11;       break;  /* pierce */
	}

	for (i = 0; i < 3; i++)
		pMobIndex->ac[i]= interpolate(level, 100, -100);
	pMobIndex->ac[3]	= interpolate(level, 100, 0);    /* exotic */

	pMobIndex->wealth           /= 100;
	pMobIndex->size              = SIZE_MEDIUM;
	pMobIndex->material          = str_dup("none");

	if (IS_SET(pMobIndex->affected_by, AFF_SANCTUARY)
	&&  IS_EVIL(pMobIndex)) {
		REMOVE_BIT(pMobIndex->affected_by, AFF_SANCTUARY);
		SET_BIT(pMobIndex->affected_by, AFF_BLACK_SHROUD);
	}

	touch_vnum(pMobIndex->vnum);
	++newmobs;
}
#endif
