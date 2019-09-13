/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: mem.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

/***************************************************************************
 *  File: mem.c                                                            *
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

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "db.h"

/*
 * Globals
 */
extern          int                     top_reset;
extern          int                     top_area;
extern          int                     top_exit;
extern          int                     top_room;
extern		int			top_mprog_index;

void	aff_free	(AFFECT_DATA *af);

RESET_DATA *new_reset_data(void)
{
	RESET_DATA *pReset;

	pReset = calloc(1, sizeof(*pReset));
	if (pReset == NULL)
		crush_mud();
	pReset->command = 'X';

	top_reset++;
	return pReset;
}

void free_reset_data(RESET_DATA *pReset)
{
	if (!pReset)
		return;
	top_reset--;
	free(pReset);
}

AREA_DATA *new_area(void)
{
	AREA_DATA *pArea;

	pArea = calloc(1, sizeof(*pArea));
	if (pArea == NULL)
		crush_mud();
	pArea->vnum		= top_area;
	pArea->file_name	= str_printf("area%d.are", pArea->vnum);
	pArea->builders		= str_empty;
	pArea->name		= str_dup("New area");
	pArea->empty		= TRUE;              /* ROM patch */
	pArea->security		= 1;
	pArea->wpd10		= 5;

	top_area++;
	return pArea;
}

/*****************************************************************************
 Name:		area_lookup
 Purpose:	Returns pointer to area with given vnum.
 Called by:	do_aedit(olc.c).
 ****************************************************************************/
AREA_DATA *area_lookup(int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next)
		if (pArea->vnum == vnum)
			return pArea;

	return 0;
}

AREA_DATA *area_vnum_lookup(int vnum)
{
	AREA_DATA *pArea;

	for (pArea = area_first; pArea; pArea = pArea->next) {
		 if (vnum >= pArea->min_vnum
		 &&  vnum <= pArea->max_vnum)
		     return pArea;
	}

	return 0;
}

void free_area(AREA_DATA *pArea)
{
	free_string(pArea->name);
	free_string(pArea->file_name);
	free_string(pArea->builders);
	free_string(pArea->credits);
	top_area--;
	free(pArea);
}

EXIT_DATA *new_exit(void)
{
	EXIT_DATA *pExit;

        pExit = calloc(1, sizeof(*pExit));
        if (pExit == NULL)
        	crush_mud();
	pExit->keyword = str_empty;

        top_exit++;
	return pExit;
}

void free_exit(EXIT_DATA *pExit)
{
	if (!pExit)
		return;

	free_string(pExit->keyword);
	mlstr_free(pExit->description);

	top_exit--;
	free(pExit);
}

ROOM_INDEX_DATA *new_room_index(void)
{
	ROOM_INDEX_DATA *pRoom;

        pRoom = calloc(1, sizeof(*pRoom));
        if (pRoom == NULL)
        	crush_mud();
	pRoom->owner = str_empty;
	pRoom->heal_rate = 100;
	pRoom->mana_rate = 100;

        top_room++;
	return pRoom;
}

void free_room_index(ROOM_INDEX_DATA *pRoom)
{
	int door;
	RESET_DATA *pReset, *next;
	ROOM_HISTORY_DATA *rh, *rh_next;
	AFFECT_DATA *paf, *paf_next;
	CHAR_DATA *mob, *mnext;
	OBJ_DATA *obj, *onext;
	ROOM_INDEX_DATA *void_room = get_room_index(ROOM_VNUM_VOID);
	
	
	/* remove all chars and objects */
	for (mob = pRoom->people; mob; mob = mnext) {
		mnext = mob->next_in_room;
		if (IS_NPC(mob))
			extract_char(mob, 0);
		else {
			char_from_room(mob);
			char_to_room(mob, void_room);
		}
	}
	for (obj = pRoom->contents; obj; obj = onext) {
		onext = obj->next_content;
		extract_obj(obj, 0);
	}
	
	/* Delete raffects */
	for (paf = pRoom->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		affect_remove_room(pRoom, paf);
	}
	mlstr_free(pRoom->name);
	mlstr_free(pRoom->description);
	free_string(pRoom->owner);

	for (door = 0; door < MAX_DIR; door++)
        	if (pRoom->exit[door]) {
        		ROOM_INDEX_DATA *pToRoom;
        		int rev;
        		
        		/* Remove ToRoom Exit */
        		pToRoom = pRoom->exit[door]->to_room.r;
        		rev = rev_dir[door];
        		if (pToRoom && pToRoom->exit[rev]
        		&& pToRoom->exit[rev]->to_room.r == pRoom) {
        			free_exit(pToRoom->exit[rev]);
        			pToRoom->exit[rev] = NULL;
        		}
        		
        		/* Remove this exit */
			free_exit(pRoom->exit[door]);
		}
	ed_free(pRoom->ed);
	
	for (pReset = pRoom->reset_first; pReset; pReset = next) {
		next = pReset->next;
		free_reset_data(pReset);
	}
	
	/* Delete history tracks */
	for (rh = pRoom->history; rh; rh = rh_next) {
		rh_next = rh->next;
		free_string(rh->name);
		free(rh);
	}
	
	top_room--;
	free(pRoom);
}

SHOP_DATA *new_shop(void)
{
	SHOP_DATA *pShop;

        pShop = calloc(1, sizeof(*pShop));
        if (pShop == NULL)
        	crush_mud();
	pShop->profit_buy   =   100;
	pShop->profit_sell  =   100;
	pShop->close_hour   =   23;
	pShop->vnums_of_buy.nsize = sizeof(int);
	pShop->vnums_of_buy.nstep = 1;

        top_shop++;
        
        if (!shop_first)
        	shop_first = pShop;
       	if (shop_last)
       		shop_last->next = pShop;
       	shop_last = pShop;
       	/*
       	 *	Do not forget about pMob->pShop->keeper !!!
       	 */
	return pShop;
}

void free_shop(SHOP_DATA *pShop)
{
	if (!pShop)
		return;
	top_shop--;
	
	if (pShop == shop_first)
	{
		if (!pShop->next)
		{
			shop_first = NULL;
			shop_last = NULL;
		} else {
			shop_first = pShop->next;
		}
	} else {
		SHOP_DATA *ipShop;
		
		for (ipShop = shop_first; ipShop; ipShop = ipShop->next)
		{
			if (ipShop->next == pShop)
			{
				if (!pShop->next)
				{
					shop_last = ipShop;
					shop_last->next = NULL;
				} else {
					ipShop->next = pShop->next;
				}
			}
		}
	}
	varr_free(&pShop->vnums_of_buy);
	free(pShop);
}

OBJ_INDEX_DATA *new_obj_index(void)
{
	OBJ_INDEX_DATA *pObj;

        pObj = calloc(1, sizeof(*pObj));
        if (pObj == NULL)
        	crush_mud();

	pObj->name		= str_dup(str_empty);
	pObj->item_type		= ITEM_TRASH;
	pObj->material		= str_dup("unknown");
	pObj->condition		= 100;
	pObj->limit		= -1;

        top_obj_index++;
	return pObj;
}

void free_obj_index(OBJ_INDEX_DATA *pObj)
{
	AFFECT_DATA *paf, *paf_next;

	if (!pObj)
		return;

	free_string(pObj->name);
	free_string(pObj->material);
	mlstr_free(pObj->short_descr);
	mlstr_free(pObj->description);

	for (paf = pObj->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
	}

	ed_free(pObj->ed);
    
	top_obj_index--;
	free(pObj);
}

MOB_INDEX_DATA *new_mob_index(void)
{
	MOB_INDEX_DATA *pMob;

        pMob = calloc(1, sizeof(*pMob));
        if (pMob == NULL)
        	crush_mud();
	pMob->name		= str_dup(str_empty);
	pMob->act		= ACT_NPC;
	pMob->race		= rn_lookup("human");
	pMob->material		= str_dup("unknown");
	pMob->size		= SIZE_MEDIUM;
	pMob->start_pos		= POS_STANDING;
	pMob->default_pos	= POS_STANDING;

	top_mob_index++;
	return pMob;
}

void free_mob_index(MOB_INDEX_DATA *pMob)
{
	if (!pMob)
		return;

	free_string(pMob->name);
	free_string(pMob->material);
	mlstr_free(pMob->short_descr);
	mlstr_free(pMob->long_descr);
	mlstr_free(pMob->description);
	mptrig_free(pMob->mptrig_list);
	free_shop(pMob->pShop);

	top_mob_index--;
	free(pMob);
}

MPCODE *mpcode_list;

MPCODE *mpcode_new(void)
{
	MPCODE *mpcode;

        mpcode = calloc(1, sizeof(*mpcode));
        if (mpcode == NULL)
        	crush_mud();
	mpcode->code = str_empty;

	top_mprog_index++;
	return mpcode;
}

void mpcode_add(MPCODE *mpcode)
{
	if (mpcode_list == NULL)
		mpcode_list = mpcode;
	else {
		mpcode->next = mpcode_list;
		mpcode_list = mpcode;
	}
}

MPCODE *mpcode_lookup(int vnum)
{
	MPCODE *mpcode;
	for (mpcode = mpcode_list; mpcode; mpcode = mpcode->next)
	    	if (mpcode->vnum == vnum)
        		return mpcode;
	return NULL;
}    
 
void mpcode_free(MPCODE *mpcode)
{
	if (!mpcode)
		return;

	free_string(mpcode->code);

	/* Remove mpcode from mpcode_list */
	if (mpcode == mpcode_list)
		mpcode_list = mpcode->next;
	else {
		MPCODE *prev;

		for (prev = mpcode_list; prev; prev = prev->next)
			if (prev->next == mpcode) {
				prev->next = mpcode->next;
				break;
			}
		if (prev == NULL)
			bug("mpcode_free: mpcode not found in mpcode_list.", 0);
	}

	top_mprog_index--;
	free(mpcode);
}
