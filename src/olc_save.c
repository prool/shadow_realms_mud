/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: olc_save.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

/**************************************************************************
 *  File: olc_save.c                                                       *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

/*
 * olc_save.c
 * This takes care of saving all the .are information.
 */

#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "merc.h"
#include "obj_prog.h"
#include "olc.h"
#include "db.h"
#include "lang.h"
#include "socials.h"

#define DIF(a,b) (~((~a)|(b)))

/*
 *  Verbose writes reset data in plain english into the comments
 *  section of the resets.  It makes areas considerably larger but
 *  may aid in debugging.
 */
#define VERBOSE

static void save_print(CHAR_DATA *ch, const char *format, ...);

/*****************************************************************************
 Name:		save_area_list
 Purpose:	Saves the listing of files to be loaded at startup.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area_list()
{
	FILE *fp;
	AREA_DATA *pArea;

	if ((fp = dfopen(AREA_PATH, AREA_LIST, "w")) == NULL)
		return;

	for (pArea = area_first; pArea; pArea = pArea->next)
		fprintf(fp, "%s\n", pArea->file_name);

	fprintf(fp, "$\n");
	fclose(fp);
}

void save_mobprogs(FILE *fp, AREA_DATA *pArea)
{
	MPCODE *mpcode;
        int i;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++) {
        	if ((mpcode = mpcode_lookup(i)) != NULL) {
			if (!found) {
        			fprintf(fp, "#MOBPROGS\n");
				found = TRUE;
			}
			fprintf(fp, "#%d\n", i);
			fwrite_string(fp, NULL, mpcode->code);
		}
        }

	if (found)
        	fprintf(fp,"#0\n\n");
}

/*****************************************************************************
 Name:		save_mobile
 Purpose:	Save one mobile to file, new format -- Hugin
 Called by:	save_mobiles (below).
 ****************************************************************************/
void save_mobile(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
    race_t *r = race_lookup(pMobIndex->race);
    MPTRIG *mptrig;
    flag64_t temp;
    struct attack_type *atd;
    int i;

    if (r == NULL) {
	log_printf("save_mobile: %d: unknown race", pMobIndex->race);
	return;
    }

    fprintf(fp, "#%d\n",	pMobIndex->vnum);
    fwrite_string(fp, NULL,	pMobIndex->name);
    mlstr_fwrite(fp, NULL,	pMobIndex->short_descr);
    mlstr_fwrite(fp, NULL,	pMobIndex->long_descr);
    mlstr_fwrite(fp, NULL,	pMobIndex->description);
    fwrite_string(fp, NULL,	r->name);
    fprintf(fp, "%s ",		format_flags(pMobIndex->act & ~r->act));
    fprintf(fp, "%s ",		format_flags(pMobIndex->affected_by & ~r->aff));
    fprintf(fp, "%d %d\n",	pMobIndex->alignment , pMobIndex->group);
    fprintf(fp, "%d ",		pMobIndex->level);
    fprintf(fp, "%d ",		pMobIndex->hitroll);
    fprintf(fp, "%dd%d+%d ",	pMobIndex->hit[DICE_NUMBER], 
				pMobIndex->hit[DICE_TYPE], 
				pMobIndex->hit[DICE_BONUS]);
    fprintf(fp, "%dd%d+%d ",	pMobIndex->mana[DICE_NUMBER], 
				pMobIndex->mana[DICE_TYPE], 
				pMobIndex->mana[DICE_BONUS]);
    fprintf(fp, "%dd%d+%d ",	pMobIndex->damage[DICE_NUMBER], 
				pMobIndex->damage[DICE_TYPE], 
				pMobIndex->damage[DICE_BONUS]);
    fprintf(fp, "%s\n",		(atd = GET_ATTACK_T(pMobIndex->dam_type)) ?
    				mlstr_mval(atd->name) : "none");
    fprintf(fp, "%d %d %d %d\n",
				pMobIndex->ac[AC_PIERCE] / 10, 
				pMobIndex->ac[AC_BASH]   / 10, 
				pMobIndex->ac[AC_SLASH]  / 10, 
				pMobIndex->ac[AC_EXOTIC] / 10);
    fprintf(fp, "%s ",		format_flags(pMobIndex->off_flags & ~r->off));
    fprintf(fp, "%s ",		format_flags(pMobIndex->imm_flags & ~r->imm));
    fprintf(fp, "%s ",		format_flags(pMobIndex->res_flags & ~r->res));
    fprintf(fp, "%s\n",		format_flags(pMobIndex->vuln_flags & ~r->vuln));
    fprintf(fp, "%s %s %s %d\n",
			flag_string(position_table, pMobIndex->start_pos),
			flag_string(position_table, pMobIndex->default_pos),
			flag_string(sex_table, pMobIndex->sex),
			pMobIndex->wealth);
    fprintf(fp, "%s ",		format_flags(pMobIndex->form & ~r->form));
    fprintf(fp, "%s ",		format_flags(pMobIndex->parts & ~r->parts));

    fprintf(fp, "%s ",		flag_string(size_table, pMobIndex->size));
    fprintf(fp, "%s\n",	IS_NULLSTR(pMobIndex->material) ? pMobIndex->material : "unknown");

/* save diffs */
    if ((temp = DIF(r->act, pMobIndex->act)))
     	fprintf(fp, "F act %s\n", format_flags(temp));

    if ((temp = DIF(r->aff, pMobIndex->affected_by)))
     	fprintf(fp, "F aff %s\n", format_flags(temp));

    if ((temp = DIF(r->off, pMobIndex->off_flags)))
     	fprintf(fp, "F off %s\n", format_flags(temp));

    if ((temp = DIF(r->imm, pMobIndex->imm_flags)))
     	fprintf(fp, "F imm %s\n", format_flags(temp));

    if ((temp = DIF(r->res, pMobIndex->res_flags)))
     	fprintf(fp, "F res %s\n", format_flags(temp));

    if ((temp = DIF(r->vuln, pMobIndex->vuln_flags)))
     	fprintf(fp, "F vul %s\n", format_flags(temp));

    if ((temp = DIF(r->form, pMobIndex->form)))
     	fprintf(fp, "F for %s\n", format_flags(temp));

    if ((temp = DIF(r->parts, pMobIndex->parts)))
    	fprintf(fp, "F par %s\n", format_flags(temp));

    for (mptrig = pMobIndex->mptrig_list; mptrig; mptrig = mptrig->next)
    {
        fprintf(fp, "M %s %d %s~\n",
        flag_string(mptrig_types, mptrig->type), mptrig->vnum,
                fix_string(mptrig->phrase));
    }

//    if (pMobIndex->clan)
//		fwrite_string(fp, "C", clan_name(pMobIndex->clan));
    if (pMobIndex->invis_level)
		fprintf(fp, "I %d\n", pMobIndex->invis_level);
    if (pMobIndex->fvnum)
		fprintf(fp, "V %d\n", pMobIndex->fvnum);

    if (pMobIndex->slang != gsn_common_slang		/* SR */
    && skill_lookup(pMobIndex->slang)
    && IS_SET(SKILL(pMobIndex->slang)->group, GROUP_SLANG))
    		fprintf(fp, "L '%s'\n", SKILL(pMobIndex->slang)->name);

    for (i = 0; i < QUANTITY_MAGIC + 1; i ++)
	if (pMobIndex->saving_throws[i])
	{
		fprintf(fp, "S");
		for (i = 0; i < QUANTITY_MAGIC + 1; i ++)
			fprintf(fp, " %d", pMobIndex->saving_throws[i]); 
		fprintf(fp, "\n");
		break;
	}
}

/*****************************************************************************
 Name:		save_mobiles
 Purpose:	Save #MOBILES secion of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_mobiles(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMob;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMob = get_mob_index(i))) {
			if (!found) {
				fprintf(fp, "#MOBILES\n");
				found = TRUE;
			}
			save_mobile(fp, pMob);
		}

	if (found)
		fprintf(fp, "#0\n\n");
}

/*****************************************************************************
 Name:		save_object
 Purpose:	Save one object to file.
                new ROM format saving -- Hugin
 Called by:	save_objects (below).
 ****************************************************************************/
void save_object(FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
    char letter;
    AFFECT_DATA *pAf;
    ED_DATA *pEd;
    struct attack_type *atd;

    fprintf(fp, "#%d\n",	pObjIndex->vnum);
    fwrite_string(fp, NULL,	pObjIndex->name);
    mlstr_fwrite(fp, NULL,	pObjIndex->short_descr);
    mlstr_fwrite(fp, NULL,	pObjIndex->description);
    fwrite_string(fp, NULL,	pObjIndex->material);
    fprintf(fp, "%s ",		flag_string(item_types, pObjIndex->item_type));
    fprintf(fp, "%s ",		format_flags(pObjIndex->extra_flags));
    fprintf(fp, "%s\n",		format_flags(pObjIndex->wear_flags));

/*
 *  Using format_flags to write most values gives a strange
 *  looking area file, consider making a case for each
 *  item type later.
 */

    switch (pObjIndex->item_type)
    {
        default:
	    fprintf(fp, "%s %s %s %s %s\n",
			format_flags(pObjIndex->value[0]),
	    		format_flags(pObjIndex->value[1]),
	    		format_flags(pObjIndex->value[2]),
	    		format_flags(pObjIndex->value[3]),
	    		format_flags(pObjIndex->value[4]));
	    break;

	case ITEM_MONEY:
	case ITEM_ARMOR:
	case ITEM_DIG:
	case ITEM_HORSE_TICKET:
	    fprintf(fp, "%d %d %d %d %d\n",
			pObjIndex->value[0],
	    		pObjIndex->value[1],
	    		pObjIndex->value[2],
	    		pObjIndex->value[3],
	    		pObjIndex->value[4]);
		break;

        case ITEM_DRINK_CON:
        case ITEM_FOUNTAIN:
            fprintf(fp, "%d %d '%s' %d %d\n",
                     pObjIndex->value[0],
                     pObjIndex->value[1],
                     liq_table[pObjIndex->value[2]].liq_name,
		     pObjIndex->value[3],
		     pObjIndex->value[4]);
            break;

        case ITEM_CONTAINER:
            fprintf(fp, "%d %s %d %d %d\n",
                     pObjIndex->value[0],
                     format_flags(pObjIndex->value[1]),
                     pObjIndex->value[2],
                     pObjIndex->value[3],
                     pObjIndex->value[4]);
            break;

        case ITEM_WEAPON:
            fprintf(fp, "%s %d %d %s %s\n",
                     flag_string(weapon_class, pObjIndex->value[0]),
                     pObjIndex->value[1],
                     pObjIndex->value[2],
                     (atd = GET_ATTACK_T(pObjIndex->value[3])) ?
                     mlstr_mval(atd->name) : "none",
                     format_flags(pObjIndex->value[4]));
            break;
            
        case ITEM_PILL:
        case ITEM_POTION:
        case ITEM_SCROLL:
	    fprintf(fp, "%d '%s' '%s' '%s' '%s'\n",
/* no negative numbers */
		     pObjIndex->value[0] > 0 ? pObjIndex->value[0] : 0,
		     pObjIndex->value[1] != -1 ?
			skill_name(pObjIndex->value[1]) : str_empty,
		     pObjIndex->value[2] != -1 ?
		     	skill_name(pObjIndex->value[2]) : str_empty,
		     pObjIndex->value[3] != -1 ?
		     	skill_name(pObjIndex->value[3]) : str_empty,
		     pObjIndex->value[4] != -1 ?
		     	skill_name(pObjIndex->value[4]) : str_empty);
	    break;

        case ITEM_STAFF:
        case ITEM_WAND:
	    fprintf(fp, "%d %d %d '%s' %d\n",
	    			pObjIndex->value[0],
	    			pObjIndex->value[1],
	    			pObjIndex->value[2],
	    			pObjIndex->value[3] != -1 ?
	    				skill_name(pObjIndex->value[3]) : str_empty,
	    			pObjIndex->value[4]);
	    break;
	case ITEM_PORTAL:
	    fprintf(fp, "%s %s %s %d %s\n",
			format_flags(pObjIndex->value[0]),
	    		format_flags(pObjIndex->value[1]),
	    		format_flags(pObjIndex->value[2]),
	    		pObjIndex->value[3],
	    		format_flags(pObjIndex->value[4]));
	    break;
	case ITEM_LIGHT:
	case ITEM_TATTOO:
	case ITEM_TREASURE:
	    fprintf(fp, "%s %s %d %s %s\n",
			format_flags(pObjIndex->value[0]),
	    		format_flags(pObjIndex->value[1]),
	    		pObjIndex->value[2],
	    		format_flags(pObjIndex->value[3]),
	    		format_flags(pObjIndex->value[4]));
	    break;
    }

    fprintf(fp, "%d ", pObjIndex->level);
    fprintf(fp, "%d ", pObjIndex->weight);
    fprintf(fp, "%d ", pObjIndex->cost);

         if (pObjIndex->condition >  90) letter = 'P';
    else if (pObjIndex->condition >  75) letter = 'G';
    else if (pObjIndex->condition >  50) letter = 'A';
    else if (pObjIndex->condition >  25) letter = 'W';
    else if (pObjIndex->condition >  10) letter = 'D';
    else if (pObjIndex->condition >   0) letter = 'B';
    else                                   letter = 'R';

    fprintf(fp, "%c\n", letter);

    for (pAf = pObjIndex->affected; pAf; pAf = pAf->next)
    {
	if (pAf->where == TO_OBJECT || pAf->bitvector == 0) {
		switch(pAf->location) {
			case APPLY_10_SKILL:
			case APPLY_5_SKILL:
				fprintf(fp, "A\n%d '%s'\n",
					pAf->location,
					skill_name(pAf->modifier));
				break;
			default:
				fprintf(fp, "A\n%d %d\n", 
				pAf->location, pAf->modifier);
				break;
		}
	} else {
		fprintf(fp, "F\n");

		switch(pAf->where)
		{
			case TO_AFFECTS:
				fprintf(fp, "A ");
				break;
			case TO_IMMUNE:
				fprintf(fp, "I ");
				break;
			case TO_RESIST:
				fprintf(fp, "R ");
				break;
			case TO_VULN:
				fprintf(fp, "V ");
				break;
			default:
				log_printf("olc_save: vnum %d: "
					   "invalid affect->where: %d",
					   pObjIndex->vnum, pAf->where);
				break;
		}
		
		switch(pAf->location) {
			case APPLY_10_SKILL:
			case APPLY_5_SKILL:
				fprintf(fp, "%d '%s' %s\n", pAf->location, skill_name(pAf->modifier),
					format_flags(pAf->bitvector | AFF_SKILL));
				break;
			default:
				fprintf(fp, "%d %d %s\n", pAf->location,
					pAf->modifier, format_flags(pAf->bitvector));
				break;
		}
	}
    }

    for (pEd = pObjIndex->ed; pEd; pEd = pEd->next)
		ed_fwrite(fp, pEd, NULL);

//    if (pObjIndex->clan)
//		fwrite_string(fp, "C", clan_name(pObjIndex->clan));
    fprintf(fp, "G %s\n", flag_string(gender_table, pObjIndex->gender));
}

/*****************************************************************************
 Name:		save_objects
 Purpose:	Save #OBJECTS section of an area file.
 Called by:	save_area(olc_save.c).
 Notes:         Changed for ROM OLC.
 ****************************************************************************/
void save_objects(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObj;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObj = get_obj_index(i))) {
			if (!found) {
    				fprintf(fp, "#OBJECTS\n");
				found = TRUE;
			}
			save_object(fp, pObj);
    		}

	if (found)
		fprintf(fp, "#0\n\n");
}

static int exitcmp(const void *p1, const void *p2)
{
	return (*(EXIT_DATA**)p1)->orig_door - (*(EXIT_DATA**)p2)->orig_door;
}

void save_room(FILE *fp, ROOM_INDEX_DATA *pRoomIndex)
{
	int door;
	ED_DATA *pEd;
	EXIT_DATA *pExit;
	EXIT_DATA *exit[MAX_DIR];
	int max_door;

        fprintf(fp, "#%d\n",	pRoomIndex->vnum);
	mlstr_fwrite(fp, NULL,	pRoomIndex->name);
	mlstr_fwrite(fp, NULL,	pRoomIndex->description);
	fprintf(fp, "0 ");
	
	{
		/*
		 *	Convert reset_rnd_exits to flag ROOM_HASH_EXITS.
		 */
		RESET_DATA *pReset;
		for (pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next)
			if (pReset->command == 'R') {
				SET_BIT(pRoomIndex->room_flags, ROOM_HASH_EXITS);
				break;
			}
		
	}
	
        fprintf(fp, "%s ",	format_flags(pRoomIndex->room_flags));
        fprintf(fp, "%s\n",	flag_string(sector_types,
					    pRoomIndex->sector_type));

        for (pEd = pRoomIndex->ed; pEd; pEd = pEd->next)
		ed_fwrite(fp, pEd, NULL);

	/* sort exits (to minimize diffs) */
	for (max_door = 0, door = 0; door < MAX_DIR; door++)
		if ((pExit = pRoomIndex->exit[door]))
			exit[max_door++] = pExit;
	qsort(exit, max_door, sizeof(*exit), exitcmp);

	for (door = 0; door < max_door; door++) {
		pExit = exit[door];
		if (pExit->to_room.r) {
 
	 		/* HACK : TO PREVENT EX_LOCKED etc without EX_ISDOOR
 			   to stop booting the mud */
 			if (IS_SET(pExit->rs_flags, EX_CLOSED)
 			||  IS_SET(pExit->rs_flags, EX_LOCKED)
 			||  IS_SET(pExit->rs_flags, EX_PICKPROOF)
 			||  IS_SET(pExit->rs_flags, EX_NOPASS)
 			||  IS_SET(pExit->rs_flags, EX_EASY)
 			||  IS_SET(pExit->rs_flags, EX_HARD)
 			||  IS_SET(pExit->rs_flags, EX_INFURIATING)
 			||  IS_SET(pExit->rs_flags, EX_NOCLOSE)
 			||  IS_SET(pExit->rs_flags, EX_NOLOCK) )
 				SET_BIT(pExit->rs_flags, EX_ISDOOR);
 			else
 				REMOVE_BIT(pExit->rs_flags, EX_ISDOOR);
 
			fprintf(fp, "D%d %d\n",   pExit->orig_door, pExit->distance);
			mlstr_fwrite(fp, NULL,	  pExit->description);
			fprintf(fp, "%s~\n",      pExit->keyword);
			fprintf(fp, "%s %d %d\n",
				format_flags(pExit->rs_flags | EX_BITVAL),
				pExit->key,
				pExit->to_room.r->vnum);
		}
	}

	if (pRoomIndex->mana_rate != 100 || pRoomIndex->heal_rate != 100)
		fprintf (fp, "M %d H %d\n", pRoomIndex->mana_rate,
					    pRoomIndex->heal_rate);
		 			     
	if (!IS_NULLSTR(pRoomIndex->owner))
		fprintf (fp, "O %s~\n" , pRoomIndex->owner);

//	if (pRoomIndex->clan)
//		fwrite_string(fp, "C", clan_name(pRoomIndex->clan));

	fprintf(fp, "S\n");
}

/*****************************************************************************
 Name:		save_rooms
 Purpose:	Save #ROOMS section of an area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_rooms(FILE *fp, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoomIndex;
	bool found = FALSE;
	int i;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i))) {
			if (!found) {
				fprintf(fp, "#ROOMS\n");
				found = TRUE;
			}
			save_room(fp, pRoomIndex);
		}

	if (found)
		fprintf(fp, "#0\n\n");
}

void save_special(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
#if defined(VERBOSE)
	fprintf(fp, "M %d %s\t* %s\n",
		pMobIndex->vnum,
		spec_name(pMobIndex->spec_fun),
		mlstr_mval(pMobIndex->short_descr));
#else
	fprintf(fp, "M %d %s\n",
		pMobIndex->vnum, spec_name(pMobIndex->spec_fun));
#endif
}

/*****************************************************************************
 Name:		save_specials
 Purpose:	Save #SPECIALS section of area file.
 Called by:	save_area(olc_save.c).
 ****************************************************************************/
void save_specials(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMobIndex;
	bool found = FALSE;
    
	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i))
		&&  pMobIndex->spec_fun) {
			if (!found) {
				fprintf(fp, "#SPECIALS\n");
				found = TRUE;
			}
			save_special(fp, pMobIndex);
		}

	if (found)
		fprintf(fp, "S\n\n");
}

int rexit_type_lookup(flag32_t rs_flags)
{
	int type;

	if (IS_SET(rs_flags, EX_CLOSED)) {
		if (IS_SET(rs_flags, EX_LOCKED))
			type = 2;
		else
			type = 1;
	} else
		type = 0;
	
	if (IS_SET(rs_flags, EX_BURYED))
		type += 3;

	return type;
}

/*
 * This function is obsolete.  It it not needed but has been left here
 * for historical reasons.  It is used currently for the same reason.
 *
 * I don't think it's obsolete in ROM -- Hugin.
 */
void save_door_reset(FILE *fp, ROOM_INDEX_DATA *pRoomIndex, EXIT_DATA *pExit)
{
	int type = rexit_type_lookup(pExit->rs_flags);
#if defined(VERBOSE)
	fprintf(fp,
		"D 0 %d %d %d\t* %s: door to the %s: %s\n", 
		pRoomIndex->vnum,
		pExit->orig_door,
		type,
		mlstr_mval(pRoomIndex->name),
		dir_name[pExit->orig_door],
		flag_string(door_resets, type));
#else
	fprintf(fp, "D 0 %d %d %d\n", 
		pRoomIndex->vnum,
		pExit->orig_door,
		type);
#endif
}

void save_reset(FILE *fp, AREA_DATA *pArea,
		ROOM_INDEX_DATA *pRoomIndex, RESET_DATA *pReset)
{
	switch (pReset->command) {
	default:
		bug("Save_resets: bad command %c.", pReset->command);
		break;

#if defined(VERBOSE)
	case 'M':
		fprintf(fp, "M 0 %d %d %d %d\t* %s (%s)\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4,
			mlstr_mval(get_mob_index(pReset->arg1)->short_descr),
			mlstr_mval(get_room_index(pReset->arg3)->name));
		break;

	case 'O':
	case 'B':
	case 'H':
		fprintf(fp, "%c 0 %d 0 %d\t* %s (%s)\n", 
			pReset->command,
			pReset->arg1,
			pReset->arg3,
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr),
			mlstr_mval(get_room_index(pReset->arg3)->name));
		break;

	case 'P':
	case 'U':
	case 'N':
	case 'A':
		fprintf(fp, "%c 0 %d %d %d %d\t* %s: %s\n", 
			pReset->command,
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4,
			mlstr_mval(get_obj_index(pReset->arg3)->short_descr),
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr));
		break;

	case 'G':
		fprintf(fp, "G 0 %d %d\t\t*\t%s\n",
			pReset->arg1,
			pReset->arg2,		// max number of objs
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr));
		break;

	case 'E':
		fprintf(fp, "E 0 %d 0 %d\t\t*\t%s: %s\n",
			pReset->arg1,
			pReset->arg3,
			mlstr_mval(get_obj_index(pReset->arg1)->short_descr),
			flag_string(wear_loc_strings, pReset->arg3));
		break;

	case 'D':
		break;

/*	case 'R':
		pRoomIndex = get_room_index(pReset->arg1);
		fprintf(fp, "R 0 %d %d\t* %s: randomize\n", 
			pReset->arg1,
			pReset->arg2,
			mlstr_mval(pRoomIndex->name));
		break;
 */
#else
	case 'M':
		fprintf(fp, "M 0 %d %d %d %d\n", 
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4);
		break;

	case 'O':
	case 'B':
	case 'H':
		fprintf(fp, "%c 0 %d 0 %d\n", 
			pReset->command,
			pReset->arg1,
			pReset->arg3);
		break;

	case 'P':
	case 'U':
	case 'N':
	case 'A':
		fprintf(fp, "%c 0 %d %d %d %d\n",
			pReset->command,
			pReset->arg1,
			pReset->arg2,
			pReset->arg3,
			pReset->arg4);
		break;

	case 'G':
		fprintf(fp, "G 0 %d %d\n", pReset->arg1, pReset->arg2);
		break;

	case 'E':
		fprintf(fp, "E 0 %d 0 %d\n",
			pReset->arg1,
			pReset->arg3);
		break;

	case 'D':
		break;

/*	case 'R':
		fprintf(fp, "R 0 %d %d\n", 
			pReset->arg1,
			pReset->arg2);
		break;
 */
#endif
	}
}

/*****************************************************************************
 Name:		save_resets
 Purpose:	Saves the #RESETS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_resets(FILE *fp, AREA_DATA *pArea)
{
	ROOM_INDEX_DATA *pRoomIndex;
	RESET_DATA *pReset;
	EXIT_DATA *pExit;
	int door;
	bool found = FALSE;
	int i;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i)))
			for (door = 0; door < MAX_DIR; door++)
				if ((pExit = pRoomIndex->exit[door])
				&&  pExit->to_room.r 
				&&  rexit_type_lookup(pExit->rs_flags)) {
					if (!found) {
						fprintf(fp, "#RESETS\n");
						found = TRUE;
					}
    					save_door_reset(fp, pRoomIndex, pExit);
				}

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pRoomIndex = get_room_index(i)))
    			for (pReset = pRoomIndex->reset_first; pReset; pReset = pReset->next) {
				if (!found) {
					fprintf(fp, "#RESETS\n");
					found = TRUE;
				}
				save_reset(fp, pArea, pRoomIndex, pReset);
			}

	if (found)
		fprintf(fp, "S\n\n");
}

void save_shop(FILE *fp, MOB_INDEX_DATA *pMobIndex)
{
	SHOP_DATA *pShopIndex;
	int iTrade;
	int * ip;

	pShopIndex = pMobIndex->pShop;

	fprintf(fp, "%d ", pShopIndex->keeper);
	for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
		if (pShopIndex->buy_type[iTrade] != 0)
			fprintf(fp, "%d ", pShopIndex->buy_type[iTrade]);
		else
			fprintf(fp, "0 ");
	}
	fprintf(fp, "%d %d ", pShopIndex->profit_buy, pShopIndex->profit_sell);
	fprintf(fp, "%d %d ", pShopIndex->open_hour, pShopIndex->close_hour);
	fprintf(fp, "%s ", format_flags(pShopIndex->flags));
	for (iTrade = 0; iTrade < pShopIndex->vnums_of_buy.nused; iTrade++) {
		if ((ip = VARR_GET(&pShopIndex->vnums_of_buy, iTrade)) == NULL)
			continue;
		fprintf(fp, "%d ", *ip);
	}
	fprintf(fp, "~\n");
}

/*****************************************************************************
 Name:		save_shops
 Purpose:	Saves the #SHOPS section of an area file.
 Called by:	save_area(olc_save.c)
 ****************************************************************************/
void save_shops(FILE *fp, AREA_DATA *pArea)
{
	MOB_INDEX_DATA *pMobIndex;
	int i;
	bool found = FALSE;
    
	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i))
		&&  pMobIndex->pShop) {
			if (!found) {
				fprintf(fp, "#SHOPS\n");
				found = TRUE;
			}
			save_shop(fp, pMobIndex);
		}

	if (found)
		fprintf(fp, "0\n\n");
}

void save_olimits(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObj;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObj = get_obj_index(i)) != NULL
		&&  pObj->limit != -1) {
			if (!found) {
				fprintf(fp, "#OLIMITS\n");
				found = TRUE;
			}
			fprintf(fp, "O %d %d\t* %s\n",
				i, pObj->limit, mlstr_mval(pObj->short_descr));
		}

	if (found)
		fprintf(fp, "S\n\n");
}

void save_omprog(FILE *fp, OBJ_INDEX_DATA *pObjIndex)
{
	int i;

	for (i = 0; i < OPROG_MAX; i++)
		if (pObjIndex->oprogs[i] != NULL)
			fprintf(fp, "O %d %s %s\t* `%s'\n",
				pObjIndex->vnum,
				optype_table[i],
				oprog_name_lookup(pObjIndex->oprogs[i]),
				mlstr_mval(pObjIndex->short_descr));
}

void save_omprogs(FILE *fp, AREA_DATA *pArea)
{
	int i;
	OBJ_INDEX_DATA *pObjIndex;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pObjIndex = get_obj_index(i)) != NULL
		&&  pObjIndex->oprogs) {
			if (!found) {
				fprintf(fp, "#OMPROGS\n");
				found = TRUE;
			}
			save_omprog(fp, pObjIndex);
		}

	if (found)
		fprintf(fp, "S\n\n");
}

void save_practicers(FILE *fp, AREA_DATA *pArea)
{
	int i;
	MOB_INDEX_DATA *pMobIndex;
	bool found = FALSE;

	for (i = pArea->min_vnum; i <= pArea->max_vnum; i++)
		if ((pMobIndex = get_mob_index(i)) != NULL
		&&  pMobIndex->practicer != 0) {
			if (!found) {
				fprintf(fp, "#PRACTICERS\n");
				found = TRUE;
			}
    			fprintf(fp, "M %d %s~\t* %s\n",
				pMobIndex->vnum,
				flag_string(skill_groups, pMobIndex->practicer),
				mlstr_mval(pMobIndex->short_descr));
		}

	if (found)
		fprintf(fp, "S\n\n");
}

void save_helps(FILE *fp, AREA_DATA *pArea)
{
	HELP_DATA *pHelp = pArea->help_first;

	if (pHelp == NULL)
		return;
		
	fprintf(fp, "#HELPS\n");

	for (; pHelp; pHelp = pHelp->next_in_area) {
		fprintf(fp, "%d %s~\n",
			pHelp->level, fix_string(pHelp->keyword));
		mlstr_fwrite(fp, NULL, pHelp->text);
	}

	fprintf(fp, "-1 $~\n\n");
}

/*****************************************************************************
 Name:		save_area
 Purpose:	Save an area, note that this format is new.
 Called by:	do_asave(olc_save.c).
 ****************************************************************************/
void save_area(AREA_DATA *pArea)
{
	FILE *fp;
	int flags;

	if ((fp = dfopen(AREA_PATH, pArea->file_name, "w")) == NULL)
		return;

	fprintf(fp, "#AREADATA\n");
	fprintf(fp, "Name %s~\n",	pArea->name);
	fwrite_string(fp, "Builders", pArea->builders);
	fprintf(fp, "VNUMs %d %d\n",	pArea->min_vnum, pArea->max_vnum);
	fwrite_string(fp, "Credits", pArea->credits);
	fprintf(fp, "Security %d\n",	pArea->security);
	if (pArea->wpd10 != 5)
		fprintf(fp, "WPD10 %d\n", pArea->wpd10);
	fprintf(fp, "LevelRange %d %d\n",
		pArea->min_level, pArea->max_level);
	if (!mlstr_null(pArea->resetmsg))
		mlstr_fwrite(fp, "ResetMessage", pArea->resetmsg);

	flags = pArea->flags & ~AREA_CHANGED;
	if (flags)
		fwrite_string(fp, "Flags", flag_string(area_flags, flags));
//	if (pArea->clan)
//		fwrite_string(fp, "Clan", clan_name(pArea->clan));
	fprintf(fp, "End\n\n");

	if (pArea->min_vnum && pArea->max_vnum) {
		save_mobiles(fp, pArea);
		save_objects(fp, pArea);
		save_rooms(fp, pArea);
		save_specials(fp, pArea);
		save_resets(fp, pArea);
		save_shops(fp, pArea);
		save_olimits(fp, pArea);
		save_mobprogs(fp, pArea);
		save_practicers(fp, pArea);
		save_omprogs(fp, pArea);
	}
	save_helps(fp, pArea);

	fprintf(fp, "#$\n");

	fclose(fp);
}

/*
void save_clan(CHAR_DATA *ch, clan_t *clan)
{
	int i;
	FILE *fp;

	// save clan data
	if ((fp = dfopen(CLANS_PATH, clan->file_name, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
			   CLANS_PATH, PATH_SEPARATOR, clan->file_name,
			   strerror(errno));
		return;
	}
		
	fprintf(fp, "#CLAN\n");

	fwrite_string(fp, "Name", clan->name);
	if (clan->recall_vnum)
		fprintf(fp, "Recall %d\n", clan->recall_vnum);
	if (clan->obj_vnum)
		fprintf(fp, "Item %d\n", clan->obj_vnum);
	if (clan->mark_vnum)
		fprintf(fp, "Mark %d\n", clan->mark_vnum);
	if (clan->altar_vnum)
		fprintf(fp, "Altar %d\n", clan->altar_vnum);

	REMOVE_BIT(clan->flags, CLAN_CHANGED);
	if (clan->flags)
		fprintf(fp, "Flags %s~\n",
			flag_string(clan_flags, clan->flags));

	for (i = 0; i < clan->skills.nused; i++) {
		clskill_t *cs = VARR_GET(&clan->skills, i);

		if (cs->sn > 0) 
			fprintf(fp, "Skill '%s' %d %d\n",
				skill_name(cs->sn), cs->level, cs->percent);
	}

	fprintf(fp, "End\n\n"
		    "#$\n");
	fclose(fp);

	// save plists
	if ((fp = dfopen(PLISTS_PATH, clan->file_name, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s", PLISTS_PATH,
			   PATH_SEPARATOR, clan->file_name,
			   strerror(errno));
		return;
	}

	fprintf(fp, "#PLISTS\n");

	fwrite_string(fp, "Leaders", clan->leader_list);
	fwrite_string(fp, "Seconds", clan->second_list);
	fwrite_string(fp, "Members", clan->member_list);

	fprintf(fp, "End\n\n"
		    "#$\n");
	fclose(fp);

	save_print(ch, "    %s (%s)", clan->name, clan->file_name);
}
*/

void save_skill(FILE *fp, skill_t *skill)
{
	struct	spell_pt	*spell = skill->spell;

	fprintf(fp, "#SKILL\n");
	fprintf(fp, "Name %s~\n", skill->name);
	fprintf(fp, "Type %s\n", spell ? "spell" : "skill");
	fprintf(fp, "Group %s\n", flag_string(skill_groups, skill->group));
	if (skill->pgsn)
		fprintf(fp, "Gsn %s\n", namedp_name(gsn_table, skill->pgsn));
	if (!mlstr_null(skill->noun_damage))
		mlstr_fwrite(fp, "NounDamage", skill->noun_damage);
	if (!mlstr_null(skill->msg_off))
		mlstr_fwrite(fp, "WearOff", skill->msg_off);
	if (!mlstr_null(skill->msg_obj))
		mlstr_fwrite(fp, "ObjWearOff", skill->msg_obj);
	if (skill->min_level)
		fprintf(fp, "MinLev %d\n",	skill->min_level);
	if (skill->remort_cost != DEFAULT_PRICE_SKILL)
		fprintf(fp, "Cost %d\n",	skill->remort_cost);
	if (skill->min_mana)
		fprintf(fp, "MinMana %d\n",	skill->min_mana);
	if (skill->beats)
		fprintf(fp, "Beats %d\n",	skill->beats);
	if (skill->flags)
		fprintf(fp, "Flags %s~\n",
			flag_string(skill_flags, skill->flags & ~SKILL_CREATED));
	if (skill->sell.nused > 0)
	{
		int iSell;
		sellskill_t *sell;
		for (iSell = 0; iSell < skill->sell.nused; iSell++) {
			sell = (sellskill_t *) varr_get(&skill->sell, iSell);
			fprintf(fp, "Sell %s %s~ %d %d %d\n",
				flag_string(sell_skill_type, sell->flag),
				sell->flag == SSF_ALL ? str_empty : sell->name,
				sell->minlev,
				sell->cost,
				sell->vnum_seller);
		}
	}

	if (spell)
	{
		if (spell->spell_fun)
			fprintf(fp, "SpellFun %s\n",
				namedp_name(spellfn_table, spell->spell_fun));
		else
			log_printf("Warning [save_skill]: %s hasn't SpellFun", skill->name);

		if (spell->minimum_position)
			fprintf(fp, "MinPos %s\n",
				flag_string(position_table, spell->minimum_position));
		if (spell->target)
			fprintf(fp, "Target %s\n",
				flag_string(skill_targets, spell->target));
		fprintf(fp, "Mschool %s~\n",
				flag_string(irv_flags, spell->mschool));
		if (spell->delay)
			fprintf(fp, "Delay %d\n", spell->delay);
		fprintf(fp, "SpellFlags %s~\n",
				flag_string(spell_flags_table, spell->type));
		if (spell->components.nused > 0)
		{
			int i;
			struct spell_component *comp;
			
			for (i = 0; i < spell->components.nused; i++)
			{
				comp = (struct spell_component *) varr_get(&spell->components, i);
				if (get_obj_index(comp->vnum) == NULL)
					continue;
				fprintf(fp, "Component %d %s~\n",
						comp->vnum,
						flag_string(componets_flags_table, comp->flags));
			}
		}
	}

	fprintf(fp, "End\n\n");
}

void save_skills(CHAR_DATA *ch)
{
	int sec = ch ? (IS_NPC(ch) ? 0 : ch->pcdata->security) : 9;
	FILE *fp;
	skill_t *skill;
	int i;

	if (sec < SECURITY_SKILL) {
		save_print(ch, "perm den.");
		return;
	}

	if ((fp = dfopen(ETC_PATH, SKILLS_CONF, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
				ETC_PATH, PATH_SEPARATOR, SKILLS_CONF,
				strerror(errno));
		return;
	}
	for (i = 0; i < skills.nused; i++) {
		skill = VARR_GET(&skills, i);
		save_skill(fp, skill);
	}

	fprintf(fp, "#$\n");
	fclose(fp);
	save_print(ch, "Skills saved.");
}

/*
void save_clans(CHAR_DATA *ch)
{
	int i;
	FILE *fp;
	bool found = FALSE;
	int sec = ch ? (IS_NPC(ch) ? 0 : ch->pcdata->security) : 9;

	if (sec < SECURITY_CLAN) {
		save_print(ch, "Insufficient security to save clans.");
		return;
	}

	if ((fp = dfopen(CLANS_PATH, CLAN_LIST, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
			   CLANS_PATH, PATH_SEPARATOR, CLAN_LIST,
			   strerror(errno));
		return;
	}

	save_print(ch, "Saved clans:");

	for (i = 0; i < clans.nused; i++) {
		fprintf(fp, "%s\n", CLAN(i)->file_name);
		if (IS_SET(CLAN(i)->flags, CLAN_CHANGED)) {
			save_clan(ch, CLAN(i));
			found = TRUE;
		}
	}

	fprintf(fp, "$\n");
	fclose(fp);

	if (!found)
		save_print(ch, "    None.");
}
*/

void save_msgdb(CHAR_DATA *ch)
{
	int i;
	FILE *fp;
	int sec = ch ? (IS_NPC(ch) ? 0 : ch->pcdata->security) : 9;

	if (sec < SECURITY_MSGDB) {
		save_print(ch, "Insufficient security to save msgdb.");
		return;
	}

	if ((fp = dfopen(LANG_PATH, MSG_FILE, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
			   LANG_PATH, PATH_SEPARATOR, MSG_FILE,
			   strerror(errno));
		return;
	}

	for (i = 0; i < MAX_MSG_HASH; i++) {
		varr *v = msg_hash_table+i;
		int j;

		for (j = 0; j < v->nused; j++) {
			mlstring **mlp = VARR_GET(v, j);
			if (!mlstr_nlang(*mlp))
				continue;
			mlstr_fwrite(fp, NULL, *mlp);
		}
	}

	fprintf(fp, "$~\n");
	fclose(fp);
	save_print(ch, "Msgdb saved.");
}

bool save_lang(CHAR_DATA *ch, lang_t *l)
{
	int i;
	FILE *fp;
	lang_t *sl;
	int flags;

	if ((fp = dfopen(LANG_PATH, l->file_name, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
			   LANG_PATH, PATH_SEPARATOR, l->file_name,
			   strerror(errno));
		return FALSE;
	}

	fprintf(fp, "#LANG\n"
		    "Name %s\n", l->name);
	if ((sl = varr_get(&langs, l->slang_of)))
		fprintf(fp, "SlangOf %s\n", sl->name);
	flags = l->flags & ~LANG_CHANGED;
	if (flags)
		fprintf(fp, "Flags %s~\n", flag_string(lang_flags, flags));
	fprintf(fp, "End\n\n");

	for (i = 0; i < MAX_RULECL; i++) {
		rulecl_t *rcl = l->rules + i;

		if (!IS_NULLSTR(rcl->file_impl)
		||  !IS_NULLSTR(rcl->file_expl)) {
			fprintf(fp, "#RULECLASS\n"
				    "Class %s\n",
				flag_string(rulecl_names, i));
			fwrite_string(fp, "Impl", rcl->file_impl);
			fwrite_string(fp, "Expl", rcl->file_expl);
			fprintf(fp, "End\n\n");
		}
	}

	fprintf(fp, "#$\n");
	fclose(fp);
	return TRUE;
}

void save_langs(CHAR_DATA *ch)
{
	int lang;
	bool list = FALSE;
	int sec = ch ? (IS_NPC(ch) ? 0 : ch->pcdata->security) : 9;

	if (sec < SECURITY_MSGDB) {
		save_print(ch, "Insufficient security to save langs.");
		return;
	}

	for (lang = 0; lang < langs.nused; lang++) {
		lang_t *l = VARR_GET(&langs, lang);

		if (IS_SET(l->flags, LANG_CHANGED)
		&&  save_lang(ch, l)) {
			save_print(ch, "Language '%s' saved (%s%c%s).",
				   l->name, LANG_PATH, PATH_SEPARATOR,
				   l->file_name);
			l->flags &= ~LANG_CHANGED;
			list = TRUE;
		}
	}

	if (list) {
		FILE *fp;

		if ((fp = dfopen(LANG_PATH, LANG_LIST, "w")) == NULL) {
			save_print(ch, "%s%c%s: %s",
				   LANG_PATH, PATH_SEPARATOR, LANG_LIST,
				   strerror(errno));
			return;
		}

		for (lang = 0; lang < langs.nused; lang++) {
			lang_t *l = VARR_GET(&langs, lang);
			fprintf(fp, "%s\n", l->file_name);
		}
		fprintf(fp, "$\n");
		fclose(fp);
	}
}

void save_rule(FILE *fp, rule_t *r)
{
	int i;

	fprintf(fp, "#RULE\n"
		    "Name %s~\n", r->name);
	if (r->arg)
		fprintf(fp, "BaseLen %d\n", r->arg);
	for (i = 0; i < r->f->v.nused; i++) {
		char **p = VARR_GET(&r->f->v, i);
		if (IS_NULLSTR(*p))
			continue;
		fprintf(fp, "Form %d %s~\n", i, *p);
	}
	fprintf(fp, "End\n\n");
}

void save_expl(CHAR_DATA *ch, lang_t *l, rulecl_t *rcl)
{
	int i;
	FILE *fp;

	if (!IS_SET(rcl->flags, RULES_EXPL_CHANGED))
		return;

	if ((fp = dfopen(LANG_PATH, rcl->file_expl, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
			   LANG_PATH, PATH_SEPARATOR, rcl->file_expl,
			   strerror(errno));
		return;
	}

	for (i = 0; i < MAX_RULE_HASH; i++) {
		int j;

		for (j = 0; j < rcl->expl[i].nused; j++) {
			rule_t *r = VARR_GET(rcl->expl+i, j);
			save_rule(fp, r);
		}
	}

	fprintf(fp, "#$\n");
	fclose(fp);

	save_print(ch, "Explicit rules (%s%c%s) saved "
		       "(lang '%s', rules type '%s').",
		   LANG_PATH, PATH_SEPARATOR, rcl->file_expl,
		   l->name, flag_string(rulecl_names, rcl->rulecl));
	rcl->flags &= ~RULES_EXPL_CHANGED;
}

void save_impl(CHAR_DATA *ch, lang_t *l, rulecl_t *rcl)
{
	int i;
	FILE *fp;

	if (!IS_SET(rcl->flags, RULES_IMPL_CHANGED))
		return;

	if ((fp = dfopen(LANG_PATH, rcl->file_impl, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
			   LANG_PATH, PATH_SEPARATOR, rcl->file_impl,
			   strerror(errno));
		return;
	}

	for (i = 0; i < rcl->impl.nused; i++) {
		rule_t *r = VARR_GET(&rcl->impl, i);
		save_rule(fp, r);
	}

	fprintf(fp, "#$\n");
	fclose(fp);

	save_print(ch, "Implicit rules (%s%c%s) saved "
		       "(lang '%s', rules type '%s').",
		   LANG_PATH, PATH_SEPARATOR, rcl->file_impl,
		   l->name, flag_string(rulecl_names, rcl->rulecl));
	rcl->flags &= ~RULES_IMPL_CHANGED;
}

void save_rules(CHAR_DATA *ch)
{
	int lang;
	int sec = ch ? (IS_NPC(ch) ? 0 : ch->pcdata->security) : 9;

	if (sec < SECURITY_MSGDB) {
		save_print(ch, "Insufficient security to save rules.");
		return;
	}

	for (lang = 0; lang < langs.nused; lang++) {
		int i;
		lang_t *l = VARR_GET(&langs, lang);

		for (i = 0; i < MAX_RULECL; i++) {
			save_expl(ch, l, l->rules+i);
			save_impl(ch, l, l->rules+i);
		}
	}
}

void save_social(FILE *fp, social_t *soc)
{
	fprintf(fp, "#SOCIAL\n");
	fprintf(fp, "name %s\n", soc->name);
	fprintf(fp, "min_pos %s\n",
		flag_string(position_table, soc->min_pos));
	mlstr_fwrite(fp, "found_char", soc->found_char);
	mlstr_fwrite(fp, "found_vict", soc->found_vict);
	mlstr_fwrite(fp, "found_notvict", soc->found_notvict);
	mlstr_fwrite(fp, "noarg_char", soc->noarg_char);
	mlstr_fwrite(fp, "noarg_room", soc->noarg_room);
	mlstr_fwrite(fp, "self_char", soc->self_char);
	mlstr_fwrite(fp, "self_room", soc->self_room);
	mlstr_fwrite(fp, "notfound_char", soc->notfound_char);
	fprintf(fp, "end\n\n");
}

void save_socials(CHAR_DATA *ch)
{
	int i;
	FILE *fp;
	int sec = ch ? (IS_NPC(ch) ? 0 : ch->pcdata->security) : 9;

	if (sec < SECURITY_SOCIALS) {
		save_print(ch, "Insufficient security to save socials.");
		return;
	}

	if ((fp = dfopen(ETC_PATH, SOCIALS_CONF, "w")) == NULL) {
		save_print(ch, "%s%c%s: %s",
			   ETC_PATH, PATH_SEPARATOR, SOCIALS_CONF,
			   strerror(errno));
		return;
	}

	for (i = 0; i < socials.nused; i++) {
		social_t *soc = VARR_GET(&socials, i);
		save_social(fp, soc);
	}

	fprintf(fp, "#$\n");
	fclose(fp);
	save_print(ch, "Socials saved.");
}

void save_areas(CHAR_DATA *ch, int flags)
{
	AREA_DATA *pArea;
	bool found = FALSE;
	int sec;

	if (!ch)       /* Do an autosave */
		sec = 9;
	else if (!IS_NPC(ch))
    		sec = ch->pcdata->security;
	else
    		sec = 0;

	if (ch)
		char_puts("Saved zones:\n", ch);
	else
		log("Saved zones:");

	for (pArea = area_first; pArea; pArea = pArea->next) {
		/* Builder must be assigned this area. */
		if (ch && !IS_BUILDER(ch, pArea))
			continue;

		if (flags && !IS_SET(pArea->flags, flags))
			continue;

		found = TRUE;
		save_area(pArea);

		if (ch)
			char_printf(ch, "    %s (%s)\n",
				    pArea->name, pArea->file_name);
		else
			log_printf("    %s (%s)",
				   pArea->name, pArea->file_name);
		REMOVE_BIT(pArea->flags, flags);
	}

	if (!found) {
		if (ch)
			char_puts("    None.\n", ch);
		else
			log("    None.");
	}
	else
		save_area_list();
}

/*****************************************************************************
 Name:		do_asave
 Purpose:	Entry point for saving area data.
 Called by:	interpreter(interp.c)
 ****************************************************************************/
void do_asave(CHAR_DATA *ch, const char *argument)
{
	if (argument[0] == '\0') {
		if (ch)
			do_help(ch, "'OLC ASAVE'");
		return;
	}

	/* Save the world, only authorized areas. */
	/* -------------------------------------- */
	if (!str_cmp("world", argument)) {
		save_areas(ch, 0);
		if (ch)
			char_puts("You saved the world.\n", ch);
		else
			log("Saved the world");
		return;
	}

	/* Save changed areas, only authorized areas. */
	/* ------------------------------------------ */
	if (!str_cmp("changed", argument)) {
		save_areas(ch, AREA_CHANGED);
		if (ch)
			char_puts("You saved changed areas.\n", ch);
		else
			log("Saved changed areas");
		return;
	}

	if (!str_cmp("skills", argument)) {
		save_skills(ch);
		if (ch)
			char_puts("You saved skills table.\n", ch);
		else
			log("Saved skills table");
		return;
	}

	if (!str_cmp("rules", argument)) {
		save_rules(ch);
		return;
	}

/*	if (!str_cmp("clans", argument)) {
		save_clans(ch);
		return;
	}
*/

	if (!str_cmp("msgdb", argument)) {
		save_msgdb(ch);
		return;
	}

	if (!str_cmp("langs", argument)) {
		save_langs(ch);
		return;
	}

	if (!str_cmp("socials", argument)) {
		save_socials(ch);
		return;
	}

	/* Show correct syntax. */
	/* -------------------- */
	if (ch)
		do_asave(ch, str_empty);
}

static void save_print(CHAR_DATA *ch, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	if (ch)
		char_printf(ch, "%s\n", buf);
	else
		log(buf);
	wiznet("$t", ch, buf, WIZ_OLC, 0, 0);
}
