/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: fight.c,v 1.35 2007/01/27 22:52:13 rem Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos} 	bulut@rorqual.cc.metu.edu.tr	   *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *
 *     By using this code, you have agreed to follow the terms of the	   *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence		   *
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdlib.h>

#if !defined (WIN32)
#	include <unistd.h>
#endif

#include "merc.h"
#include "quest.h"
#include "fight.h"
#include "rating.h"
#include "update.h"
#include "mob_prog.h"
#include "obj_prog.h"

DECLARE_DO_FUN(do_crush		);
DECLARE_DO_FUN(do_emote		);
DECLARE_DO_FUN(do_dismount	);
DECLARE_DO_FUN(do_bash		);
DECLARE_DO_FUN(do_berserk	);
DECLARE_DO_FUN(do_disarm	);
DECLARE_DO_FUN(do_kick		);
DECLARE_DO_FUN(do_dirt		);
DECLARE_DO_FUN(do_trip		);
DECLARE_DO_FUN(do_tail		);
DECLARE_DO_FUN(do_look_in	);
DECLARE_DO_FUN(do_get		);
DECLARE_DO_FUN(do_sacrifice	);
DECLARE_DO_FUN(do_visible	);
DECLARE_DO_FUN(do_recall	);
DECLARE_DO_FUN(do_flee		);
DECLARE_DO_FUN(do_clan		);
DECLARE_DO_FUN(do_scalp		);

/*
 * Local functions.
 */
void	check_assist		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_dodge		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_parry		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
bool	check_block		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
bool	check_blink		(CHAR_DATA *ch, CHAR_DATA *victim);
bool	check_hand_block	(CHAR_DATA *ch, CHAR_DATA *victim);
void	dam_message		(CHAR_DATA *ch, CHAR_DATA *victim, int dam,
				 int dt, bool immune, int dam_type);
void	death_cry		(CHAR_DATA *ch);
void	death_cry_org		(CHAR_DATA *ch, int part);
void	group_gain		(CHAR_DATA *ch, CHAR_DATA *victim);
int	xp_compute		(CHAR_DATA *gch, CHAR_DATA *victim,
				 int total_levels, int members);
bool	is_safe 		(CHAR_DATA *ch, CHAR_DATA *victim);

OBJ_DATA * make_corpse		(CHAR_DATA *ch);
void	one_hit 		(CHAR_DATA *ch, CHAR_DATA *victim, int dt,
				 int loc);
void	mob_hit 		(CHAR_DATA *ch, CHAR_DATA *victim, int dt);
void	set_fighting		(CHAR_DATA *ch, CHAR_DATA *victim);
void	disarm			(CHAR_DATA *ch, CHAR_DATA *victim,
				 int disarm_second);
int	critical_strike		(CHAR_DATA *ch, CHAR_DATA *victim, int dam);
void	check_eq_damage		(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
void	check_shield_damage	(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
void	check_weapon_damage	(CHAR_DATA *ch, CHAR_DATA *victim, int loc);
int 	check_forest		(CHAR_DATA *ch);

#define FOREST_ATTACK 1
#define FOREST_DEFENCE 2
#define FOREST_NONE 0

void	handle_death		(CHAR_DATA *ch, CHAR_DATA *victim);

int    check_material_weapon(CHAR_DATA *ch, int dam, OBJ_DATA *weapon)
{
	int bit;
	int immune = IS_NORMAL;
	
	if (dam == 0 || weapon == NULL)
		return dam;

	if (check_material(weapon, "wood") || check_material(weapon, "oak")
	||  check_material(weapon, "hardwood") || check_material(weapon, "softwood"))
	{
		bit	= FORCE_WOOD;
	} else if (check_material(weapon, "silver"))
	{
		bit	= FORCE_SILVER;
	} else if (check_material(weapon, "iron") || check_material(weapon, "metal"))
	{
		bit	= FORCE_IRON;
	} else
		return dam;

	if (IS_SET(ch->imm_flags, bit))
		immune = IS_IMMUNE;
	else if (IS_SET(ch->res_flags, bit) && immune != IS_IMMUNE)
		immune = IS_RESISTANT;
	else if (IS_SET(ch->vuln_flags, bit)){
		if (immune == IS_IMMUNE)
			immune = IS_RESISTANT;
		else if (immune == IS_RESISTANT)
			immune = IS_NORMAL;
		else
			immune = IS_VULNERABLE;
	}
	
	if (immune == IS_NORMAL)
		return dam;

	switch(immune) {
		case IS_IMMUNE:
			dam = 0;
			break;
		case IS_RESISTANT:
			dam -= dam/3;
			break;
		case IS_VULNERABLE:
			dam += dam/2;
			break;
		default:
			bug("Error in check_material_weapon", 0);
			return dam;
	}

	return dam;
}

/*
 * Gets all money from the corpse.
 */
void get_gold_corpse(CHAR_DATA *ch, OBJ_DATA *corpse)
{
	OBJ_DATA *tmp, *tmp_next;
	for (tmp = corpse->contains; tmp; tmp = tmp_next) {
		tmp_next = tmp->next_content;
		if (tmp->pIndexData->item_type == ITEM_MONEY)
			get_obj(ch, tmp, corpse);
	}
}

int check_forest(CHAR_DATA* ch)
{
	AFFECT_DATA* paf;

	if (ch->in_room->sector_type != SECT_FOREST
	&& ch->in_room->sector_type != SECT_HILLS
	&& ch->in_room->sector_type != SECT_MOUNTAIN) 
		return FOREST_NONE;
	for (paf = ch->affected; paf; paf = paf->next) {
		if (paf->type == gsn_forest_fighting) {
			if (paf->location == APPLY_AC) 
				return FOREST_DEFENCE;
			else 
				return FOREST_ATTACK;
		}
	}
	return FOREST_NONE;
}



/*
 * Control the fights going on.
 * Called periodically by update_handler.
 */
void violence_update(void)
{
	CHAR_DATA *ch;
	CHAR_DATA *ch_next;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;

	for (ch = char_list; ch; ch = ch_next) {
		ch_next = ch->next;

		/* decrement the wait */
		if (ch->desc == NULL)
			ch->wait = UMAX(0, ch->wait - PULSE_VIOLENCE);

		if ((victim = ch->fighting) == NULL || ch->in_room == NULL)
			continue;

		if (IS_AWAKE(ch)
		&& ch->in_room == victim->in_room
		&& !IS_GHOST(victim) && !IS_GHOST(ch))
			multi_hit(ch, victim, TYPE_UNDEFINED);
		else
			stop_fighting(ch, FALSE);

		if ((victim = ch->fighting) == NULL)
			continue;

		if (!IS_NPC(victim))
			ch->last_fought = victim;

		SET_FIGHT_TIME(ch);

		if (victim->in_room != ch->in_room)
			continue;
		
		if (religion_fight_cast(ch))
			continue;

		for (obj = ch->carrying; obj; obj = obj_next) {
			obj_next = obj->next_content;
			if (ch->fighting == NULL)
				break;
			oprog_call(OPROG_FIGHT, obj, ch, NULL);
		}

		if ((victim = ch->fighting) == NULL
		||  victim->in_room != ch->in_room)
			continue;

		/*
		 * Fun for the whole family!
		 */
		check_assist(ch, victim);
		if (IS_NPC(ch)) {
			if (HAS_TRIGGER(ch, TRIG_FIGHT))
				mp_percent_trigger(ch, victim, NULL, NULL,
						   TRIG_FIGHT);
			if (HAS_TRIGGER(ch, TRIG_HPCNT))
				mp_hprct_trigger(ch, victim);
		}
	}
}

bool check_bare_char(CHAR_DATA *ch)
{
	int loc;
	int eq_c = 0;
	
	for (loc = 0; loc < MAX_WEAR; loc++)
		if (get_eq_char(ch, loc))
			eq_c++;
	return eq_c < (number_range(30, 40) * MAX_WEAR) / 100;
}

/* for auto assisting */
void check_assist(CHAR_DATA *ch,CHAR_DATA *victim)
{
	CHAR_DATA *rch, *rch_next;

	if (IS_GHOST(victim) || IS_GHOST(ch))
		return;

	for (rch = ch->in_room->people; rch != NULL; rch = rch_next) {
		rch_next = rch->next_in_room;

		if (IS_AWAKE(rch) && rch->fighting == NULL) {
		    /* quick check for ASSIST_PLAYER */
		    if (IS_PC(ch) && IS_NPC(rch)
		    &&  IS_SET(rch->pIndexData->off_flags, ASSIST_PLAYERS)
		    &&	rch->level + 6 > victim->level) {
			do_emote(rch, "screams and attacks!");
			multi_hit(rch,victim,TYPE_UNDEFINED);
			continue;
		    }

		    /* PCs next */
		    if (((IS_PC(rch) && IS_SET(rch->pcdata->plr_flags, PLR_AUTOASSIST))
			|| IS_AFFECTED(rch, AFF_CHARM))
		    &&  is_same_group(ch,rch)
		    &&  !is_safe_nomessage(rch, victim))
		    {
			multi_hit (rch,victim,TYPE_UNDEFINED);
			if (IS_NPC(victim))
				change_faith(rch, RELF_FI_STRONG_V, victim->level - rch->level);
			if (IS_PC(victim)
			&& check_bare_char(victim))
				change_faith(rch, RELF_FI_BARE_PC_V, 0);
			if (!IS_EVIL(victim))
				change_faith(rch, RELF_FIGHT_NONEVIL, 0);
			continue;
		    }

		    if (IS_PC(ch) && RIDDEN(rch) == ch)
		    {
			multi_hit(rch,victim,TYPE_UNDEFINED);
			continue;
		    }

		    /* now check the NPC cases */

		    if (IS_NPC(ch)) {
			if ((IS_NPC(rch) && IS_SET(rch->pIndexData->off_flags,ASSIST_ALL))
			||   (IS_NPC(rch) && rch->race == ch->race
			   && IS_SET(rch->pIndexData->off_flags,ASSIST_RACE))
			||   (IS_NPC(rch) && IS_SET(rch->pIndexData->off_flags,ASSIST_ALIGN)
			   &&	((IS_GOOD(rch)	  && IS_GOOD(ch))
			     ||  (IS_EVIL(rch)	  && IS_EVIL(ch))
			     ||  (IS_NEUTRAL(rch) && IS_NEUTRAL(ch))))
			||   (rch->pIndexData == ch->pIndexData
			   && IS_SET(rch->pIndexData->off_flags,ASSIST_VNUM))) {
			    CHAR_DATA *vch;
			    CHAR_DATA *target;
			    int number;

			    if (number_bits(1) == 0)
				continue;

			    target = NULL;
			    number = 0;

			    for (vch = ch->in_room->people; vch; vch = vch->next_in_room)
			    {
				if (can_see(rch,vch)
				&&  is_same_group(vch,victim)
				&&  number_range(0,number) == 0)
				{
				    target = vch;
				    number++;
				}
			    }

			    if (target != NULL)
			    {
				do_emote(rch,"screams and attacks!");
				multi_hit(rch,target,TYPE_UNDEFINED);
			    }
			}
		    }
		}
	}
}

DECLARE_DO_FUN(do_say		);
/*
 * Do one group of attacks.
 */
void multi_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	int     chance, chance2;
	OBJ_DATA	* wield = get_eq_char(ch, WEAR_WIELD);
	RELIGION_DATA	* rel;

	/* no attacks for stunnies -- just a check */
	if (ch->position < POS_RESTING
	|| IS_GHOST(ch))
		return;

	/* ridden's adjustment */
	if (RIDDEN(victim) && !IS_NPC(victim->mount)) {
		if (victim->mount->fighting == NULL
		|| victim->mount->fighting == ch)
			victim = victim->mount;
		else
			do_dismount(victim->mount, str_empty);
	}

	if (IS_AFFECTED(ch,AFF_WEAK_STUN)) {
		act_puts("You are too stunned to respond $N's attack.",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n is too stunned to respond your attack.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		REMOVE_BIT(ch->affected_by, AFF_WEAK_STUN);
		return;
	}

	if (IS_AFFECTED(ch,AFF_STUN) && number_percent() > get_curr_stat(ch, STAT_CON)) {
		act_puts("You are too stunned to respond $N's attack.",
			 ch, NULL, victim, TO_CHAR, POS_FIGHTING);
		act_puts("$n is too stunned to respond your attack.",
			 ch, NULL, victim, TO_VICT, POS_FIGHTING);
		act_puts("$n seems to be stunned.",
			 ch, NULL, victim, TO_NOTVICT, POS_FIGHTING);
/*		REMOVE_BIT(ch->affected_by, AFF_STUN);
 *		affect_bit_strip(ch, TO_AFFECTS, AFF_STUN);
 *		SET_BIT(ch->affected_by, AFF_WEAK_STUN);
 */
		return;
	}
	
	if (IS_NPC(ch)) {
		mob_hit(ch, victim, dt);
		return;
	}

	one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim)
		return;

	if ((chance = get_skill(ch, gsn_area_attack))
	&&  number_percent() < chance && ch->move > 0) {
		int count = 0, max_count;
		CHAR_DATA *vch, *vch_next;

		check_improve(ch, gsn_area_attack, TRUE, 6);

		if (LEVEL(ch) < 50)
			max_count = 1;
		else if (LEVEL(ch) < 70)
			max_count = 2;
		else if (LEVEL(ch) < 85)
			max_count = 3;
		else
			max_count = 4;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;
			if (vch != victim && vch->fighting == ch) {
				one_hit(ch, vch, dt, WEAR_WIELD);
				if (++count >= max_count)
					break;
			}
		}
	}
	
	if (ch->fighting != victim)
		return;

	if (IS_AFFECTED(ch, AFF_HASTE)
	&& number_percent() < (CAN_SECOND_HAND_USE(ch, wield) ? 40 : 20)
	&& ch->move > 0)
		one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_cleave
	|| dt == gsn_ambush || dt == gsn_dual_backstab || dt == gsn_circle
	|| dt == gsn_assassinate || dt == gsn_vampiric_bite || dt == gsn_knife)
		return;

	chance2 = get_curr_stat(ch , STAT_DEX) - get_curr_stat(victim , STAT_DEX);
	
	if      (chance2 > 5)
		chance2 = 5 + (chance2-5)/2;
	else if (chance2 < -5)
		chance2 = - 5 + (chance2+5)/2;
	
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance2++;
	if (IS_AFFECTED(victim, AFF_HASTE))
		chance2--;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance2--;
	if (IS_AFFECTED(victim, AFF_SLOW))
		chance2++;
	if (ch->move <= 0)
		chance2 -= 4;
		
	chance2 *= 5;
	
	if ((chance = get_skill(ch, gsn_second_attack)) > 1) {
	   if (number_percent() < (chance + chance2) / 2){
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, gsn_second_attack, TRUE, 5);
		if (ch->fighting != victim)
			return;
	   } else check_improve(ch, gsn_second_attack, FALSE, 10);
	}

	if((chance = get_skill(ch, gsn_third_attack)) > 1) {
	   if (number_percent() < (chance+chance2) / 3) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, gsn_third_attack, TRUE, 6);
		if (ch->fighting != victim)
			return;
	   } else check_improve(ch, gsn_third_attack, FALSE, 12);
	}

	if ((chance = get_skill(ch,gsn_fourth_attack)) > 1) {
	   if (number_percent() < (chance+chance2) / 4) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, gsn_fourth_attack, TRUE, 7);
		if (ch->fighting != victim)
			return;
	   } else check_improve(ch, gsn_fourth_attack, FALSE, 14);
	}

	if ((chance = get_skill(ch,gsn_fifth_attack)) > 1) {
	   if (number_percent() < (chance+chance2) / 5) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		check_improve(ch, gsn_fifth_attack, TRUE, 8);
		if (ch->fighting != victim)
		    return;
	   } else check_improve(ch, gsn_fifth_attack, FALSE, 16);
	}

	if (check_forest(ch) == FOREST_ATTACK) {
		chance = get_skill(ch, gsn_forest_fighting);
		while (number_percent() < (chance+chance2)) {
			one_hit(ch, victim, dt, WEAR_WIELD);
			check_improve (ch, gsn_forest_fighting, TRUE, 8);
			if (ch->fighting != victim)
				return;
			chance /= 2;
		}
	}
		
	if (CAN_SECOND_HAND_USE(ch, wield))
	{
		wield = get_eq_char(ch, WEAR_SECOND_WIELD);
		if ( (chance = get_skill(ch, gsn_second_weapon)) > 1) {
			if ( number_percent() < chance + chance2 ) {
				one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
				check_improve(ch, gsn_second_weapon, TRUE, 2);
				if (ch->fighting != victim)
					return;
			} else
				check_improve(ch, gsn_second_weapon, FALSE, 10);
		} else if ( (chance = get_skill(ch, gsn_second_dagger)) > 1) {
			if (!wield
			|| wield->pIndexData->item_type != ITEM_WEAPON
			|| wield->value[0] != WEAPON_DAGGER) {
				return;
			} else if ( number_percent() < chance + chance2 ) {
				one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
				check_improve(ch, gsn_second_dagger, TRUE, 3);
				if (ch->fighting != victim)
					return;
			} else
				check_improve(ch, gsn_second_dagger, FALSE, 8);
		}

		if ((chance = get_skill(ch,gsn_secondary_attack)) > 1) {
			if (number_percent() < (chance+chance2) / 2 ) {
				one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
				check_improve(ch, gsn_secondary_attack, TRUE, 5);
				if (ch->fighting != victim)
					return;
			} else check_improve(ch, gsn_secondary_attack, FALSE, 12);
		}

		if ((chance = get_skill(ch,gsn_extra_attack)) > 1) {
			if (number_percent() < (chance+chance2) / 4 ) {
				one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
				check_improve(ch, gsn_extra_attack, TRUE, 7);
				if (ch->fighting != victim)
					return;
			} else check_improve(ch, gsn_extra_attack, FALSE, 14);
		}
	}
	
	if ((rel = GET_CHAR_RELIGION(ch))
	&& IS_SET(rel->flags, RELIG_EXTRA_HIT)
	&& get_char_faith(ch) / 10 > number_range(49, 8000))
	{
		reduce_faith(ch, 1, FALSE);
		do_say(ch, "My honor is my life!");
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

}

bool mob_cast(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int min_level;
	char *spell;
	
	if (!number_bits(2))
		return TRUE;
	switch (number_bits(3))	{
		default: if (IS_AFFECTED(ch, AFF_HASTE))
			 	return FALSE;
			 min_level = 19; spell = "haste";        break;
		case 0:
		case 1:  if (IS_AFFECTED(victim, AFF_SLOW))
				return FALSE;
			 min_level = 18; spell = "slow";         break;
		case 2:  
		case 3:  if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
				return FALSE;
			 min_level = 0; spell = "faerie fire";  break;
		case 4:
		case 5:  min_level = 20; spell = "dispel magic"; break;
	}
	
	if (ch->level < min_level /*|| number_bits(1)*/)
		return FALSE;
	doprintf(interpret, ch, "cast '%s'", spell);
	return TRUE;
}

void mob_cast_undead(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (mob_cast(ch, victim))
		return;
	if (number_bits(1)){
		if (ch->level < 45)
			doprintf(interpret, ch, "cast poison");
		else
			doprintf(interpret, ch, "cast plague");
		return;
	}

	if (ch->level < 50)
		doprintf(interpret, ch, "cast 'energy drain'");
	else
		doprintf(interpret, ch, "cast 'hand of undead'");
}

void mob_cast_mage(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (mob_cast(ch, victim))
		return;
	if (number_bits(1)){
		if (!IS_AFFECTED(victim, AFF_CURSE))
			doprintf(interpret, ch, "cast curse");
		else if (!IS_AFFECTED(victim, AFF_WEAKEN))
			doprintf(interpret, ch, "cast weaken");
		else
			doprintf(interpret, ch, "cast 'magic missile'");
		return;
	}
	if (ch->level < 20)
		doprintf(interpret, ch, "cast 'shocking grasp'");
	else if (ch->level < 44)
		doprintf(interpret, ch, "cast 'lightning bolt'");
	else if (ch->level < 59)
		doprintf(interpret, ch, "cast 'acid arrow'");
	else
		doprintf(interpret, ch, "cast 'acid blast'");
}

void mob_cast_cleric(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (mob_cast(ch, victim))
		return;
	if (ch->hit < ch->max_hit / 2) {
		if (ch->level < 23)
			doprintf(interpret, ch, "cast 'cure serious'");
		else if (ch->level < 54)
			doprintf(interpret, ch, "cast heal");
		else
			doprintf(interpret, ch, "cast 'master healing'");
		return;
	}
	
	if (number_bits(1)){
		switch(number_bits(1)){
			case 0 : doprintf(interpret, ch, "cast nowait armor");
			default: doprintf(interpret, ch, "cast nowait shield");
		}
		return;
	}
	
	if (ch->level < 55)
		doprintf(interpret, ch, "cast 'cause critical'");
	else
		doprintf(interpret, ch, "cast 'blade barrier'");
}

/* procedure for all mobile attacks */
void mob_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt)
{
	CHAR_DATA *vch, *vch_next;
	int chance, chance2;
	flag64_t act = ch->pIndexData->act;
	flag64_t off = ch->pIndexData->off_flags;

	/* no attack by ridden mobiles except spec_casts */
	if (RIDDEN(ch)) {
		if (ch->fighting != victim) {
			stop_fighting(ch, FALSE);
			set_fighting(ch, victim);
		}
		return;
	}

	one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim)
		return;

	/* Area attack -- BALLS nasty! */

	if (IS_SET(off, OFF_AREA_ATTACK) && ch->move > 0) {
		int count = 0, max_count = 1 + ch->level / 25;
		
		for (vch = ch->in_room->people; vch != NULL; vch = vch_next) {
			vch_next = vch->next_in_room;
			if ((vch != victim && vch->fighting == ch))
				one_hit(ch, vch, dt, WEAR_WIELD);
				if (++count >= max_count)
					break;
		}
	}

	if (ch->fighting != victim)
		return;

	if ((IS_AFFECTED(ch, AFF_HASTE) || IS_SET(off, OFF_FAST))
	   && ch->move > 0 && number_percent() < 30)
		one_hit(ch, victim, dt, WEAR_WIELD);

	if (ch->fighting != victim || dt == gsn_backstab || dt == gsn_circle ||
		dt == gsn_dual_backstab || dt == gsn_cleave || dt == gsn_ambush
			|| dt == gsn_vampiric_bite || dt == gsn_knife)
		return;

	chance2 = get_curr_stat(ch , STAT_DEX) - get_curr_stat(victim , STAT_DEX);

	if      (chance2 > 5)
		chance2 = 5 + (chance2-5)/2;
	else if (chance2 < -5)
		chance2 = - 5 + (chance2+5)/2;

	if (IS_AFFECTED(ch, AFF_HASTE))
		chance2++;
	if (IS_AFFECTED(victim, AFF_HASTE))
		chance2--;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance2--;
	if (IS_AFFECTED(victim, AFF_SLOW))
		chance2++;
	if (ch->move <= 0)
		chance2 -= 4;
	
	chance2 *= 5;

	if ((chance = get_skill(ch, gsn_second_attack)) > 0)
	   if (number_percent() < (chance+chance2) / 2) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}
	

	if ((chance = get_skill(ch, gsn_third_attack)) > 0)
	   if (number_percent() < (chance + chance2) / 4) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

	if ((chance = get_skill(ch, gsn_fourth_attack)) > 0)
	   if (number_percent() < (chance + chance2) / 6) {
		one_hit(ch, victim, dt, WEAR_WIELD);
		if (ch->fighting != victim)
			return;
	}

	if (CAN_SECOND_HAND_USE(ch, get_eq_char(ch, WEAR_WIELD)))
	{
		if ( (chance = get_skill(ch, gsn_second_weapon)) > 0
		&& number_percent() < (chance+chance2) ) {
			one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
			if (ch->fighting != victim)
				return;
		}

		if ((chance = get_skill(ch,gsn_secondary_attack)) > 0
		&& (number_percent() < (chance+chance2) / 2) ) {
			one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
			if (ch->fighting != victim)
				return;
		}

		if ((chance = get_skill(ch,gsn_extra_attack)) > 0
		&& (number_percent() < (chance+chance2) / 4) ) {
			one_hit(ch, victim, dt, WEAR_SECOND_WIELD);
			if (ch->fighting != victim)
				return;
		}
	}

	/* PC waits */

	if (ch->wait > 1)
		return;

	switch (number_range(0, 2)) {
	case 0: 
		if (IS_SET(act, ACT_UNDEAD)) {
			mob_cast_undead(ch, victim);
			return;
		}
		break;
	case 1:
		if (IS_SET(act, ACT_MAGE)) {
			mob_cast_mage(ch, victim);
			return;
		}
		break;
	case 2:
		if (IS_SET(act, ACT_CLERIC)) {
			mob_cast_cleric(ch, victim);
			return;
		}
		break;
	}
	
	if (ch->fighting != victim)
		return;

	/* now for the skills */

	switch (number_range(0, 7)) {
	case 0:
		if (IS_SET(off, OFF_BASH))
			do_bash(ch, str_empty);
		break;

	case 1:
		if (IS_SET(off, OFF_BERSERK)
		&&  !IS_AFFECTED(ch, AFF_BERSERK))
			do_berserk(ch, str_empty);
		break;


	case 2:
		if (IS_SET(off, OFF_DISARM)
		||  IS_SET(act, ACT_WARRIOR | ACT_THIEF)) {
			if (number_range(0, 1)
			&&  get_eq_char(victim, WEAR_SECOND_WIELD))
				do_disarm(ch, "second");
			else if (get_eq_char(victim, WEAR_WIELD))
				do_disarm(ch, str_empty);
		}
		break;

	case 3:
		if (IS_SET(off, OFF_KICK))
			do_kick(ch, str_empty);
		break;

	case 4:
		if (IS_SET(off, OFF_DIRT_KICK))
			do_dirt(ch, str_empty);
		break;

	case 5:
		if (IS_SET(off, OFF_TAIL))
			do_tail(ch, str_empty);
		break;

	case 6:
		if (IS_SET(off, OFF_TRIP))
			do_trip(ch, str_empty);
		break;
	case 7:
		if (IS_SET(off, OFF_CRUSH))
			do_crush(ch, str_empty);
		break;
	}
	/*
	 * If victim is charmed, ch might attack
	 * victim's master.
	 */
	if (IS_NPC(victim)
	&&  IS_AFFECTED(victim, AFF_CHARM)
	&&  victim->master
	&&  victim->master->in_room == ch->in_room
	&&  number_bits(2)
	&&  ch->wait < PULSE_VIOLENCE / 3
	&&  !check_blink(ch, victim->master)
	&&  ch->fighting == victim) {
		stop_fighting(ch, FALSE);
		set_fighting(ch, victim->master);
	}
}

int get_dam_type(CHAR_DATA *ch, OBJ_DATA *wield, int *dt)
{
	int dam_type;
	struct attack_type *atd;

	if (*dt == TYPE_UNDEFINED) {
		*dt = TYPE_HIT;
		if (wield &&  wield->pIndexData->item_type == ITEM_WEAPON)
			*dt += wield->value[3];
		else
			*dt += ch->dam_type;
	}
	
	if (*dt < TYPE_HIT)
		if (wield && wield->pIndexData->item_type == ITEM_WEAPON)
			dam_type = (atd = GET_ATTACK_T(wield->value[3])) ? atd->damage: 0;
		else
			dam_type = (atd = GET_ATTACK_T(ch->dam_type)) ? atd->damage: 0;
	else
		dam_type = (atd = GET_ATTACK_T(*dt - TYPE_HIT)) ? atd->damage: 0;

	if (dam_type == TYPE_UNDEFINED)
		dam_type = DAM_BASH;

	return dam_type;
}

/*
 * Hit one guy once.
 */
void one_hit(CHAR_DATA *ch, CHAR_DATA *victim, int dt, int loc)
{
	OBJ_DATA *wield;
	int victim_ac;
	int thac0;
	int thac0_00;
	int thac0_32;
	int dam;
	int diceroll, roll;
	int sn, sk, sk2;
	int dam_type;
	bool counter;
	bool result;
	int sercount;
	int dam_flags;
	int victim_hit;

	sn = -1;
	counter = FALSE;

	/* just in case */
	if (victim == ch || ch == NULL || victim == NULL)
		return;

	/*
	 * Can't beat a dead char!
	 * Guard against weird room-leavings.
	 */
	if (victim->position == POS_DEAD
	|| ch->in_room != victim->in_room)
		return;

	if (ch->move > 0 && !MOUNTED(ch))
		ch->move--;

	/*
	 * Figure out the type of damage message.
	 */
	wield = get_eq_char(ch, loc);
	dam_flags = DAMF_SHOW;
	
	if (loc == WEAR_SECOND_WIELD)
		dam_flags |= DAMF_SECOND;
	dam_type = get_dam_type(ch, wield, &dt);
	if (wield && wield->pIndexData->item_type == ITEM_STAFF)
		dam_flags |= DAMF_STAFF;
	if (wield && !IS_SET(dam_flags, DAMF_STAFF) && wield->value[0] == WEAPON_EXOTIC)
		dam_flags |= DAMF_EXOTIC;

	/* get the weapon skill */
	sn = get_weapon_sn(wield);
	
	if (sn == gsn_bow) {
		if ((wield->condition -= 2) < 1) {
			act("{gThe {r$P{g breaks into pieces.{x",
				ch, NULL, wield, TO_ROOM);
			extract_obj(wield, 0);
			return;
		} else if (number_percent() < 30) {
			act("{gYour careless actions with {r$P{g causes it.{x",
				ch, NULL, wield, TO_CHAR);
			act("{g$n's careless actions with {r$P{g causes it.{x",
				ch, NULL, wield, TO_ROOM);
		}
		
	}
	sk = 20 + get_weapon_skill(ch, sn);

	/*
	 * Calculate to-hit-armor-class-0 versus armor.
	 */
	if (IS_NPC(ch)) {
		flag64_t act = ch->pIndexData->act;

		thac0_00 = 20;
		if (IS_SET(act, ACT_WARRIOR))
			thac0_32 = -6;
		else if (IS_SET(act, ACT_THIEF))
			thac0_32 = -10;
		else if (IS_SET(act, ACT_CLERIC))
			thac0_32 = 2;
		else if (IS_SET(act, ACT_MAGE))
			thac0_32 = 6;
		else
			thac0_32 = -4;
	}
	else {
		class_t *cl;

		if ((cl = class_lookup(ch->class)) == NULL)
			return;

		thac0_00 = cl->thac0_00;
		thac0_32 = cl->thac0_32;
	}

	thac0  = interpolate(LEVEL(ch), thac0_00, thac0_32);

	thac0 -= GET_HITROLL(ch) * sk / 125;

	if (thac0 < 0)
		thac0 = thac0 / 2;

	if (thac0 < -5)
		thac0 = -5 + (thac0 + 5) / 2;

	thac0 += 8 * (100 - sk) / 100;
	if (get_skill(ch, gsn_improved_fight) > (roll = number_percent())) {
		check_improve(victim, gsn_improved_fight, TRUE, 8);
		thac0 -= UMAX(1, roll / 12);
	}

	if (dt == gsn_backstab)
		thac0 -= get_skill(ch, gsn_backstab) / 10;
	else if (dt == gsn_dual_backstab)
		thac0 -= get_skill(ch, gsn_dual_backstab) / 10;
	else if (dt == gsn_cleave)
		thac0 -= get_skill(ch, gsn_cleave) / 10;
	else if (dt == gsn_ambush)
		thac0 -= get_skill(ch, gsn_ambush) / 10;
	else if (dt == gsn_vampiric_bite)
		thac0 -= get_skill(ch, gsn_vampiric_bite) / 10;
	else if (dt == gsn_charge)
		thac0 -= get_skill(ch, gsn_charge) / 10;
	
	if (thac0 < -30)
		thac0 = -30 + (thac0 + 30) / 2;		// added by Xor

	switch(dam_type) {
	case DAM_PIERCE:victim_ac = GET_AC(victim,AC_PIERCE)	/10;	break;
	case DAM_BASH:	victim_ac = GET_AC(victim,AC_BASH)	/10;	break;
	case DAM_SLASH:	victim_ac = GET_AC(victim,AC_SLASH)	/10;	break;
	default:	victim_ac = GET_AC(victim,AC_EXOTIC)	/10;	break;
	}

	if (victim_ac > 15)
		victim_ac = (victim_ac - 15) / 4 + 15;
			// in past: victim_ac = (victim_ac - 15) / 5 + 15; !!!
			// хотя так и так 1000 AC не держит 100 hitroll
			//	раньше ещё хуже было!!!
			// щас при 1000AC и при 100 hitroll ->
			//  -> thac0 + victim_ac = примерно 0 (вообщем то нормально
			//	если принять во внимание дальнейшие parry, etc...)
			//	Может вообще убрать thac и полностью перейти
			//	на skill'ный метод уклонений как более продвинутый
			//	и логичный ?!!!!	(в to do далёкий)

/*	if (check_clan_skill(ch) == NULL && 
	    get_skill(victim, gsn_armor_use) > 70) {
		check_improve(victim, gsn_armor_use, TRUE, 8);
		victim_ac -= (victim->level) / 2;
	}
 */

	if (!can_see(ch, victim)) {
		 if ((sk2 = get_skill(ch, gsn_blind_fighting))
		 &&  number_percent() < sk2)
			check_improve(ch,gsn_blind_fighting,TRUE,16);
		 else
			victim_ac += 6;
	}

	if (victim->position < POS_FIGHTING)
		victim_ac -= 4;

	if (victim->position < POS_RESTING)
		victim_ac -= 8;

	/*
	 * The moment of excitement!
	 */
	while ((diceroll = number_bits(5)) >= 20)
		;

	if (diceroll == 0
	|| (diceroll != 19 && diceroll < thac0 + victim_ac)) {
		/* Miss. */
		damage(ch, victim, 0, dt, dam_type, dam_flags);
		tail_chain();
		return;
	}

	/*
	 * Hit.
	 * Calc damage.
	 */

	if (IS_NPC(ch) && wield == NULL)
		dam = dice(ch->damage[DICE_NUMBER], ch->damage[DICE_TYPE]);
	else {
		if (sn != -1)
			check_improve(ch, sn, TRUE, 5);
		if (IS_SET(dam_flags, DAMF_STAFF)) {
			dam = number_range(1 + 2 * sk / 100,
				3 * wield->level / 5 * sk / 100);
		} else if (wield) {
			dam = dice(wield->value[1],
				   wield->value[2]) * sk / 100;

			/* no shield = more */
			if (get_eq_char(ch, WEAR_SHIELD) == NULL)
				dam = dam * 21/20;

			/* sharpness! */
			if (IS_WEAPON_STAT(wield, WEAPON_SHARP)) {
				int percent;

				if ((percent = number_percent()) <= (sk / 8))
					dam = 2 * dam + (dam * 2 * percent / 100);
				if (number_percent() > 85 && --wield->condition < 50) {
					REMOVE_BIT(wield->value[4], WEAPON_SHARP);
					act("You $p has become {mblunted{x!", ch, wield, NULL, TO_CHAR);
				}
			}

			/* holy weapon */
			if (IS_WEAPON_STAT(wield, WEAPON_HOLY)
			&&  IS_GOOD(ch) && IS_EVIL(victim)
			&&  number_percent() < (-victim->alignment) / 30) {
				act("$n's flesh is burned with the holy aura of $p.", victim, wield, NULL, TO_ROOM);
				act_puts("Your flesh is burned with the holy aura of $p.", victim, wield, NULL, TO_CHAR, POS_DEAD);
				dam += dam * 120 / 100;
			}
			/* Improved weapon */
			if ((sk2 = get_skill(ch, gsn_improved_weapon)) &&
				(diceroll = number_percent()) <= sk2)
			{
				check_improve(ch, gsn_improved_weapon, TRUE, 6);
				dam += (dam * diceroll) / 90;
			}

		} else {
			dam = number_range(1 + 4 * sk / 100,
					   2 * LEVEL(ch) / 3 * sk / 100);
			/* master hand */
			if ((sk2 = get_skill(ch, gsn_master_hand))
			&& number_percent() <= sk2) {
				check_improve(ch, gsn_master_hand, TRUE, 6);
				dam += dam / 2;
				if (number_percent() < sk2 / 6 + LEVEL(ch) - LEVEL(victim))
				{
					SET_BIT(victim->affected_by,
						AFF_WEAK_STUN);
					act_puts("You hit $N with a stunning "
						 "force!", ch, NULL, victim,
						 TO_CHAR, POS_DEAD);
					act_puts("$n hit you with a stunning "
						 "force!", ch, NULL, victim,
						 TO_VICT, POS_DEAD);
					act_puts("$n hits $N with a stunning "
						 "force!", ch, NULL, victim,
						 TO_NOTVICT, POS_DEAD);
					check_improve(ch, gsn_master_hand,
						      TRUE, 6);
				}
			}

			/* 'mastering pound' skill */
			if ((sk2 = get_skill(ch, gsn_mastering_pound)) &&
			    (diceroll = number_percent()) <= sk2) {
				check_improve(ch, gsn_mastering_pound, TRUE, 6);
				dam += (dam * diceroll) / 100;
			}

		}
	}

	/*
	 * Bonuses.
	 */
	/* Ridinig */
	if (MOUNTED(ch)) {
		int skill;
		
		if (wield
		&& (skill = get_skill(ch, gsn_riding_fight)) > number_percent()) {
			dam += (dam * skill) / 500;
			check_improve(ch, gsn_riding_fight, TRUE, 24);
		} else {
			dam *= 0.8;
			check_improve(ch, gsn_riding_fight, FALSE, 32);
		}
	}

	/* Enchanced damage */
	if ((sk2 = get_skill(ch, gsn_enhanced_damage))
	&&  (diceroll = number_percent()) <= sk2) {
		check_improve(ch, gsn_enhanced_damage, TRUE, 6);
		dam += dam * diceroll * sk2 / 10000;
	}

	/* Mastering sword && Katana */
	if (sn == gsn_sword
	&&  !IS_SET(dam_flags, DAMF_STAFF)
	&&  (sk2 = get_skill(ch, gsn_mastering_sword))
	&&  number_percent() <= sk2
	&&  wield) {
		OBJ_DATA *katana = wield;
		check_improve(ch, gsn_mastering_sword, TRUE, 6);
		dam += dam * 110 /100;

		if (IS_WEAPON_STAT(katana, WEAPON_KATANA)
		&& (katana->ed == NULL
			|| strstr(mlstr_mval(katana->ed->description), ch->name)
		)) {
			AFFECT_DATA *paf;

			/* auto repair katana */
			if (wield->condition < 100
			&& number_percent() < 30)
			{
				if(++wield->condition >= 100
				&& IS_SET(wield->pIndexData->value[4], WEAPON_SHARP)
				&& !IS_WEAPON_STAT(wield, WEAPON_SHARP)) {
					SET_BIT(wield->value[4], WEAPON_SHARP);
					act("{wThe {W$P{w become sharp again.{x",
						ch, NULL, wield, TO_ROOM);
				}
			}

			if (number_percent() < 11	// 10%
			&& (katana->cost = ++katana->cost % 25) == 0) {
				int new_mod;

				if ((paf = affect_find(katana->affected,
					gsn_katana))
				&& (new_mod = UMIN(paf->modifier + 1,
					ch->level / 3)) > paf->modifier)
				{
					ch->hitroll += new_mod - paf->modifier;
					paf->modifier = new_mod;
					
					if (paf->next != NULL) {
						ch->damroll += new_mod
							- paf->next->modifier;
						paf->next->modifier = new_mod;
					}
					act("$n's katana glows blue.\n",
					    ch, NULL, NULL, TO_ROOM);
					char_puts("Your katana glows blue.\n",ch);
				} else
					katana->cost -= 24;
			}
		}
	}

	/* Improved Bow */
	if (sn == gsn_bow &&
		(sk2 = get_skill(ch, gsn_mastering_bow)) &&
		number_percent() <= sk2) {
		check_improve(ch, gsn_mastering_bow, TRUE, 6);
		dam += (dam * sk2) /100;
	}

	/* Piercing pain */
	if (sn == gsn_dagger
		&& (sk2 = get_skill(ch, gsn_piercing_pain)) > 1
		&& (diceroll = number_range(1, 150)) <= sk2)
	{
		if (diceroll > 95
		&& number_percent() < 50) {
			
			if (!IS_AFFECTED(victim, AFF_CORRUPTION))
			{
				AFFECT_DATA af;
			
				af.where	= TO_AFFECTS;
				af.type		= gsn_corruption;
				af.level	= ch->level;
				af.duration	= 1;
				af.location	= APPLY_HITROLL;
				af.modifier	= diceroll / 10;
				af.bitvector	= AFF_CORRUPTION;
				affect_to_char(victim, &af);
			}
			
			act_puts("Your $p found weak spot on $N's body.",
				ch, wield, victim, TO_CHAR, POS_DEAD);
			act("$n's $p found weak spot on your body.",
				ch, wield, victim, TO_VICT);
			act("$n's $p found weak spot on $N's body.",
				ch, wield, victim, TO_NOTVICT);
			check_improve(ch, gsn_piercing_pain, TRUE, 2);
			dam += (dam * diceroll) / 100;
		} else {
			check_improve(ch, gsn_piercing_pain, TRUE, 12);
			dam += (dam * diceroll) / 200;
		}
	}

	/* Check BERSERK */
	if (is_affected(ch, gsn_berserk))
		dam *= 1.1;

	if (!IS_AWAKE(victim))
		dam *= 2;
	else if (victim->position < POS_FIGHTING)
		dam = dam * 3 / 2;

	sercount = number_percent();
	if (dt == gsn_backstab || dt == gsn_vampiric_bite)
		sercount += 40;
	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch))
		sercount += 10;
	sercount *= 1.5;
	if (victim->fighting == NULL && !IS_NPC(victim)
	&&  !is_safe_nomessage(victim, ch)
	&&  !is_safe_nomessage(ch,victim)
	&&  (victim->position == POS_SITTING ||
	     victim->position == POS_STANDING)
	&&  dt != gsn_assassinate
	&&  (sercount <= get_skill(victim, gsn_counter))) {
		counter = TRUE;
		check_improve(victim,gsn_counter,TRUE,1);
		act("$N turns your attack against you!",
		    ch, NULL, victim, TO_CHAR);
		act("You turn $n's attack against $m!",
		    ch, NULL, victim, TO_VICT);
		act("$N turns $n's attack against $m!",
		    ch, NULL, victim, TO_NOTVICT);
		ch->fighting = victim;
	}
	else if (!victim->fighting)
		check_improve(victim, gsn_counter, FALSE, 1);

	if (dt == gsn_backstab && (IS_NPC(ch) || wield))
		dam = LEVEL(ch) / 11 * dam + LEVEL(ch);
	else if (dt == gsn_dual_backstab && (IS_NPC(ch) || wield))
		dam = LEVEL(ch) / 14 * dam + LEVEL(ch);
	else if (dt == gsn_circle)
		dam = (LEVEL(ch)/40 + 1) * dam + LEVEL(ch);
	else if (dt == gsn_knife)
		dam = (LEVEL(ch)/28 + 1) * dam + LEVEL(ch);
	else if (dt == gsn_vampiric_bite && is_affected(ch, gsn_vampire))
		dam = (LEVEL(ch)/13 + 1) * dam + LEVEL(ch);
	else if (dt == gsn_cleave && wield != NULL) 
		dam = (dam * 2 + ch->level);
	else if (dt == gsn_assassinate)
		dam *= 2;
	else if (dt == gsn_charge)
		dam *= LEVEL(ch)/12;

	if (ch->size > victim->size)
		dam += dam * (ch->size - victim->size) / 5;

	/*
	 *	Add damroll - check Improved fight
	 */
	if ((sk2 = get_skill(ch, gsn_improved_fight)) &&
		(diceroll = number_percent()) < sk2 / 2)
	{
		check_improve(ch, gsn_improved_fight, TRUE, 14);
		dam += number_range(100, 150 - UMIN(50, diceroll))
			* GET_DAMROLL(ch) * UMIN(100, sk) / (100 * 100);
		//dam += (dam * diceroll) / 90;
	} else
		dam += GET_DAMROLL(ch) * UMIN(100, sk) / 100;

	if (dt == gsn_ambush)
		dam *= UMAX(3, LEVEL(ch)/12);

	if ((sk2 = get_skill(ch, gsn_deathblow)) > 1
	   && dt != gsn_backstab
	   && dt != gsn_dual_backstab
	   && dt != gsn_cleave
	   && dt != gsn_assassinate
	   && dt != gsn_ambush
	   && dt != gsn_vampiric_bite
	   && dt != gsn_knife) {
		if (number_percent() <  (sk2/8)) {
			act("You deliver a blow of deadly force!",
			    ch, NULL, NULL, TO_CHAR);
			act("$n delivers a blow of deadly force!",
			    ch, NULL, NULL, TO_ROOM);
			dam = LEVEL(ch)*dam/20;
			check_improve(ch, gsn_deathblow, TRUE, 1);
		}
		else
			check_improve(ch, gsn_deathblow, FALSE, 3);
	}
	

	if (dam <= 0)
		dam = 1;

	dam = check_material_weapon(victim, dam, wield);
		
	victim_hit = victim->hit;

	if (counter) {
		result = damage(ch, ch, 2*dam, dt, dam_type, DAMF_SHOW);
		multi_hit(victim, ch, TYPE_UNDEFINED);
	}
	else
		result = damage(ch, victim, dam, dt, dam_type, dam_flags);

	/* vampiric bite gives hp to ch from victim */
	if (dt == gsn_vampiric_bite && result) {
		int hit_ga = UMIN((dam / 2), victim_hit);

		ch->hit += hit_ga;
		ch->hit  = UMIN(ch->hit, ch->max_hit);
		update_pos(ch);
		char_puts("Your health increases as you suck "
			  "your victim's blood.\n", ch);
	}

	/* but do we have a funky weapon? */
	if (result && wield != NULL && ch->fighting == victim) {
		int dam;

		if (IS_WEAPON_STAT(wield, WEAPON_VORPAL)) {
			int chance;

			chance = get_skill(ch, get_weapon_sn(wield)) +
				 get_curr_stat(ch, STAT_STR) * 4;

			if (chance > number_range(1, 200000)
			&&  !IS_IMMORTAL(victim)) {
				act("$p makes an huge arc in the air, "
				    "chopping $n's head OFF!",
				     victim, wield, NULL, TO_ROOM);
				act("$p whistles in the air, "
				    "chopping your head OFF!",
				    victim, wield, NULL, TO_CHAR);
				act("$n is DEAD!", victim, NULL, NULL, TO_ROOM);
				char_puts("You have been KILLED!\n", victim);
				victim->position = POS_DEAD;
				handle_death(ch, victim);
				return;
			}
		}

		if (IS_WEAPON_STAT(wield, WEAPON_POISON)) {
			int level;
			AFFECT_DATA *poison, af;

			if ((poison = affect_find(wield->affected,
							gsn_poison)) == NULL)
				level = wield->level;
			else
				level = poison->level;

			if (!saves_spell_simply(level / 2,victim, DAM_POISON)) {
				char_puts("You feel poison coursing "
					  "through your veins.", victim);
				act("$n is poisoned by the venom on $p.",
				    victim, wield, NULL, TO_ROOM);

				af.where     = TO_AFFECTS;
				af.type      = gsn_poison;
				af.level     = level * 3/4;
				af.duration  = level / 2;
				af.location  = APPLY_STR;
				af.modifier  = -1;
				af.bitvector = AFF_POISON;
				affect_join(victim, &af);
			}

			/* weaken the poison if it's temporary */
			if (poison != NULL) {
				poison->level = UMAX(0,poison->level - 2);
				poison->duration = UMAX(0,poison->duration - 1);
				if (poison->level == 0
				||  poison->duration == 0)
					act("The poison on $p has worn off.",
					    ch, wield, NULL, TO_CHAR);
			}
		}

		if (IS_WEAPON_STAT(wield, WEAPON_VAMPIRIC)) {
			dam = number_range(1 + wield->level / 4, wield->level / 2 + 2);
			act("$p draws life from $n.",
			    victim, wield, NULL, TO_ROOM);
			act("You feel $p drawing your life away.",
			    victim, wield, NULL, TO_CHAR);
			damage(ch, victim, dam, 0, DAM_NEGATIVE, DAMF_NONE);
			ch->hit += dam/2;
		}

		if (IS_WEAPON_STAT(wield, WEAPON_FLAMING)) {
			dam = number_range(1 + wield->level / 4, wield->level * 2 / 3 + 2);
			act("$n is burned by $p.", victim, wield, NULL, TO_ROOM);
			act("$p sears your flesh.",
				victim, wield, NULL, TO_CHAR);
			fire_effect((void *) victim, wield->level/2, dam,
				    TARGET_CHAR);
			damage(ch, victim, dam, 0, DAM_FIRE, DAMF_NONE);
		}

		if (IS_WEAPON_STAT(wield, WEAPON_FROST)) {
			dam = number_range(1 + wield->level / 5, wield->level / 3 + 2);
			act("$p freezes $n.", victim, wield, NULL, TO_ROOM);
			act("The cold touch of $p surrounds you with ice.",
				victim, wield, NULL, TO_CHAR);
			cold_effect(victim, wield->level/2, dam, TARGET_CHAR);
			damage(ch, victim, dam, 0, DAM_COLD, DAMF_NONE);
		}

		if (IS_WEAPON_STAT(wield, WEAPON_SHOCKING)) {
			dam = number_range(1 + wield->level / 4, wield->level / 3 + 2);
			act("$n is struck by lightning from $p.",
			    victim, wield, NULL, TO_ROOM);
			act("You are shocked by $p.",
			    victim, wield, NULL, TO_CHAR);
			shock_effect(victim, wield->level/2, dam, TARGET_CHAR);
			damage(ch, victim, dam, 0, DAM_LIGHTNING, DAMF_NONE);
		}
	}

	tail_chain();
}


void sac_obj(CHAR_DATA * ch, OBJ_DATA *obj);	/* act_obj.c */
/*
 * handle_death - called from `damage' if `ch' has killed `victim'
 */
void handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	bool vnpc = IS_NPC(victim) ? TRUE : FALSE;
	ROOM_INDEX_DATA *vroom = victim->in_room;
	bool is_duel = !IS_NPC(victim) 
		&& (!IS_NPC(ch) || IS_AFFECTED(ch, AFF_CHARM)) 
		&& IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA);
	OBJ_DATA *corpse;
	class_t *cl;
	long percent_exp = 0, base_exp_tl = 0;
	int blood;

	debug_printf(8, "handle_death");

	if (IS_NPC(victim)
	&& IS_SET(victim->pIndexData->act, ACT_GOD))
		return;

	group_gain(ch, victim);

	/*
	 * Death trigger
	 */
	if (vnpc && HAS_TRIGGER(victim, TRIG_DEATH)) {
		victim->position = POS_STANDING;
		mp_percent_trigger(victim, ch, NULL, NULL, TRIG_DEATH);
	}

	blood = victim->size + number_range(1, get_curr_stat(victim, STAT_CON) / 7);
	debug_printf(8, "handle_death2");
	corpse = raw_kill(ch, victim);

	/* RT new auto commands */
	if (IS_PC(ch) && vnpc && vroom == ch->in_room
	&& corpse && corpse->in_room == ch->in_room
	/*&&  (corpse = get_obj_list(ch, "corpse", ch->in_room->contents))*/)
	{
		if (HAS_SKILL(ch, gsn_vampire)) {
			act_puts("$n suck {Rblood{x from $N's corpse!!",
				 ch, NULL,victim,TO_ROOM,POS_SLEEPING);
			char_puts("You suck {Rblood{x "
				  "from the corpse!!\n\n", ch);
			gain_condition(ch, COND_BLOODLUST, blood);
		}

		if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOLOOK))
			do_look_in(ch, "corpse");
		if (corpse->contains) {
			/* corpse exists and not empty */
			if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOLOOT))
				do_get(ch, "all corpse");
			else if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOGOLD))
				get_gold_corpse(ch, corpse);
		}

		if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOSCALP))
			do_scalp(ch, "corpse");

		if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOSAC))
			//do_sacrifice(ch, "corpse");
			sac_obj(ch, corpse);
	}
	
	debug_printf(8, "handle_death3");

	if (ch != victim)
	{
		if (IS_PC(victim))
			change_faith(ch, RELF_KILL_PC, 0);

		if (IS_NPC(victim)
		&& victim->level >= ch->level)
			change_faith(ch, RELF_KILL_STRONG_NPC, 0);

		if (IS_EVIL(victim))
		{
			change_faith(ch, RELF_KILL_EVIL, 0);
			if (victim->level >= ch->level)
				change_faith(ch, RELF_KILL_STRONG_EVIL, 0);
		}
		
		if (IS_GOOD(victim))
		{
			change_faith(ch, RELF_KILL_GOOD, 0);
			if (victim->level >= ch->level)
				change_faith(ch, RELF_KILL_STRONG_GOOD, 0);
		}
	}
	debug_printf(8, "handle_death4");

	if (vnpc || victim->position == POS_STANDING)
		return;

	if (is_duel)
		return;

	/* Dying penalty: 2/3 way back. */
	if (IS_SET(victim->pcdata->plr_flags, PLR_WANTED)
	&&  victim->level > 1) {
		REMOVE_BIT(victim->pcdata->plr_flags, PLR_WANTED);
		victim->level--;
		victim->pcdata->plevels++;
		victim->pcdata->exp = exp_for_level(victim, victim->level);
		victim->pcdata->exp_tl = 0;
	}
	else
//		if (victim->exp_tl > 0)
//			gain_exp(victim, -victim->exp_tl*2/3);
	{
		base_exp_tl = exp_for_level(victim, victim->level+1) - exp_for_level(victim, victim->level);
		percent_exp = ( ((float)exp_to_level(victim)) / ((float) base_exp_tl ) * 100);
		if (GET_CHAR_RELIGION(victim))
			base_exp_tl = (GET_CHAR_RELIGION(victim)->change_exp_death * base_exp_tl) / 100;
		if (percent_exp < 40)
			gain_exp(victim, - (base_exp_tl / 3), FALSE, FALSE);
		else if (percent_exp < 80)
			gain_exp(victim, - (base_exp_tl / 4), FALSE, FALSE);
		else if (percent_exp < 200)
			gain_exp(victim, - (base_exp_tl / 5), FALSE, FALSE);
		if (percent_exp > 100 )
			switch(number_range(0, 3)){
			case 0: --victim->pcdata->perm_hit;
				--victim->max_hit;
				char_puts("You lose your {Rhealth{x !\n", victim);
				break;
			case 1: --victim->pcdata->perm_mana;
				--victim->max_mana;
				char_puts("You lose your {Cmana{x !\n", victim);
				break;
			case 2: --victim->pcdata->perm_move;
				--victim->max_move;
				char_puts("You lose your {Gmove{x !\n", victim);
				break;
			default: char_puts("{YLuck{x smiled to you !\n", victim);
				break;
			}
	}

	victim->pcdata->death++;

	change_faith(victim, RELF_DEATH, victim->pcdata->death);
	
	debug_printf(8, "handle_death5");

	/* Die too much and is deleted ... :( */
	if ((cl = class_lookup(victim->class))
	&&  !CAN_FLEE(ch, cl)) {
//		victim->perm_stat[STAT_CHA]--;
		if (victim->pcdata->death > cl->death_limit) {
			char msg[MAX_STRING_LENGTH];

			snprintf(msg, sizeof(msg),
				 "%d deaths limit for %s",
				 cl->death_limit, cl->name);
			delete_player(victim, msg);
			return;
		}
		else if (victim->pcdata->death == cl->death_limit)
			char_puts("{RWarning!{x You have last death for you class.\n", victim);
	}

	else if (victim->pcdata->perm_hit < 1) {
		delete_player(victim, "lack of hitpoints");
		return;
	}
/*	else
 *	char_puts("You feel your life power has decreased "
 *			  "with this death.\n", victim);
 */
}

/*
 * Inflict damage from a hit.
 */
bool damage(CHAR_DATA *ch, CHAR_DATA *victim,
	    int dam, int dt, int dam_type, int dam_flags)
{
	bool immune;
	int dam2;
	int loc;
	int ac;
	int drunk;
	RELIGION_DATA *rel; 

	if (JUST_KILLED(victim))
		return FALSE;

	if (victim != ch) {
		/*
		 * Certain attacks are forbidden.
		 * Most other attacks are returned.
		 */
		if (victim->position > POS_STUNNED) {
			if (victim->fighting == NULL) {
				set_fighting(victim, ch);
				if (IS_NPC(victim)
				&&  HAS_TRIGGER(victim, TRIG_KILL))
					mp_percent_trigger(victim, ch, NULL,
							   NULL, TRIG_KILL);
			}
			if (victim->timer <= 4)
				victim->position = POS_FIGHTING;
		}

		if (victim->position > POS_STUNNED) {
			if (ch->fighting == NULL)
				set_fighting(ch, victim);
		}

		/*
		 * More charm and group stuff.
		 */
		if (victim->master == ch)
			stop_follower(victim);
		if (ch->mount == victim)
			break_horse(ch);
		else if (victim->mount == ch)
			break_horse(victim);

		if (MOUNTED(victim) == ch || RIDDEN(victim) == ch) {
			REMOVE_BIT(ch->comm, COMM_RIDING);
			REMOVE_BIT(victim->comm, COMM_RIDING);
		}
	}
	debug_printf(11, "ddd000");

	/*
	 * No one in combat can hide, be invis or camoed.
	 */
	do_visible(ch, str_empty);

	/*
	 * Damage modifiers.
	 */
	if (IS_AFFECTED(victim, AFF_SANCTUARY)
	&&  !((dt == gsn_cleave) && (number_percent() < 50)))
		dam /= 2;

	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) 
		dam = (4*dam)/7;

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) && IS_EVIL(ch))
		dam -= dam / 4;

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD) && IS_GOOD(ch))
		dam -= dam / 4;

/*	if (IS_AFFECTED(victim, AFF_UNDEAD) && (dam_type == DAM_NEGATIVE))
		dam = 0;
 */

	if (is_affected(victim,gsn_resistance))
		dam = (3 * dam)/5;

	if (is_affected(victim, gsn_protection_heat) && (dam_type == DAM_FIRE))
		dam -= dam / 4;
	if (is_affected(victim, gsn_protection_cold) && (dam_type == DAM_COLD))
		dam -= dam / 4;
	
	if (IS_PC(ch) && (drunk = ch->pcdata->condition[COND_DRUNK]) > 10)
		dam -= (dam * UMIN(10, drunk / 5)) / 100;

	immune = FALSE;
	loc = IS_SET(dam_flags, DAMF_SECOND) ? WEAR_SECOND_WIELD : WEAR_WIELD;

	debug_printf(11, "ddd001");
	/*
	 * Check for parry, and dodge.
	 */
	if (dt >= TYPE_HIT && ch != victim) {
		/*
		 * some funny stuff
		 */
		if (is_affected(victim, gsn_mirror)) {
			act("$n shatters into tiny fragments of glass.",
			    victim, NULL, NULL, TO_ROOM);
			extract_char(victim, 0);
			return FALSE;
		}

		if (check_parry(ch, victim, loc))
			return FALSE;
		if (get_eq_char(victim, WEAR_SECOND_WIELD) && number_percent() > 50)
		    if (check_parry(ch, victim, loc))
				return FALSE;
		if (check_block(ch, victim, loc))
			return FALSE;
		if (check_dodge(ch, victim))
			return FALSE;
		if (check_blink(ch, victim))
			return FALSE;
		if (check_hand_block(ch, victim))
			return FALSE;
/*		if (check_illusion(ch, victim))
			return FALSE;
 */
		
/*		if (check_reflection(ch, victim))
			if ( number_percent() >= 25 || 
			     ch->mana < 200 ) 
				return FALSE;
			else {
				act("$N magical block reflect your attack against you!",
					ch, NULL, victim, TO_CHAR);
				act("You magical block reflect $n's attack against $m!",
					ch, NULL, victim, TO_VICT);
				act("$N magical block reflect $n's attack against $m!",
					ch, NULL, victim, TO_NOTVICT);
				ch->mana -= 50;
				victim = ch;
				dam *= 2;
			}
 */
	}
	debug_printf(11, "ddd002");

	if (ch != victim) {
		if (dam_type == DAM_FIRE ||
		    dam_type == DAM_COLD ||
		    dam_type == DAM_LIGHTNING ||
		    dam_type == DAM_ACID ||
		    dam_type == DAM_NEGATIVE ||
		    dam_type == DAM_HOLY ||
		    dam_type == DAM_ENERGY ||
		    dam_type == DAM_MENTAL)
		{
		/*	if (check_reflection(ch, victim))
				if ( number_percent() >= 25 || 
				     ch->mana < 200 )
					return FALSE;
				else
				{
					act("$N magical block reflect your attack against you!",
						ch, NULL, victim, TO_CHAR);
					act("You magical block reflect $n's attack against $m!",
						ch, NULL, victim, TO_VICT);
					act("$N magical block reflect $n's attack against $m!",
						ch, NULL, victim, TO_NOTVICT);
					ch->mana -= 50;
					victim = ch;
					dam *= 2;
				}
		 */
			dam2 = (get_curr_stat(ch, STAT_INT) -
			        get_curr_stat(victim, STAT_WIS));
		}
		else 
		{
			dam2 = get_curr_stat(ch, STAT_STR) - get_curr_stat(victim, STAT_CON);

			/* Check for GOOD armor class (if weapon not exotic) */
			if (!IS_SET(dam_flags, DAMF_EXOTIC)){
				int	dp;
				switch (dam_type){
					case DAM_BASH:		ac = get_ac_condition(victim, AC_BASH, LEVEL(ch));
										break;
					case DAM_PIERCE:	ac = get_ac_condition(victim, AC_PIERCE, LEVEL(ch));
										break;
					case DAM_SLASH:		ac = get_ac_condition(victim, AC_SLASH, LEVEL(ch));
										break;
					default:		ac = get_ac_condition(victim, AC_EXOTIC, LEVEL(ch));
										break;
				}

				dp	= (GET_HITROLL(ch) * 90) / UMAX(8, LEVEL(victim)) - 100;
				if (dp > 0)
					dp = UMIN(dp > 30 ? (30 + (dp - 30) / 2) : dp, 100);
				else
					dp = 0;
				dam	-= (dam * (ac - dp)) / 300;
				
			}
		}

		if      (dam2 > 5)
			dam2 = (dam2 - 5) / 2 + 5;
		else if (dam2 < -5 )
			dam2 = -(dam2 + 5) / 2 - 5;

		dam += (dam * dam2) / 20;

	}
	
	if ((rel = GET_CHAR_RELIGION(ch))
	&& IS_SET(rel->flags, RELIG_ADVANCE_DAMAGE)
	&& get_char_faith(ch) > 999)
		dam += (number_range(3, 3 + get_char_faith(ch) / 500)
			* dam) / 100;

	switch(check_immune(victim, dam_type)) {
	case IS_IMMUNE:
		immune = TRUE;
		dam = 0;
		break;

	case IS_RESISTANT:
		dam -= dam/3;
		break;

	case IS_VULNERABLE:
		dam += dam/2;
		break;
	}

	if (dt >= TYPE_HIT && ch != victim) {
		if ((dam2 = critical_strike(ch, victim, dam)) != 0)
			dam = dam2;
	}

	if (IS_SET(dam_flags, DAMF_SHOW))
		dam_message(ch, victim, dam, dt, immune, dam_type);

	if (dam == 0)
		return FALSE;
	debug_printf(11, "ddd005");

	if (dt >= TYPE_HIT && ch != victim)
		check_eq_damage(ch, victim, loc);

	/*
	 * Hurt the victim.
	 * Inform the victim of his new state.
	 */
	victim->hit -= dam;
	if (victim->hit < 1 && (IS_IMMORTAL(victim)
	|| (IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_GOD))))
		victim->hit = 1;

	update_pos(victim);

	switch(victim->position) {
	case POS_MORTAL:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is mortally wounded, and will die soon, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts( "You are mortally wounded, and will die soon, if not aided.\n", victim);
		break;

	case POS_INCAP:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is incapacitated and will slowly die, if not aided.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts( "You are incapacitated and will slowly die, if not aided.\n", victim);
		break;

	case POS_STUNNED:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		act("$n is stunned, but will probably recover.",
		    victim, NULL, NULL, TO_ROOM);
		char_puts("You are stunned, but will probably recover.\n",
			     victim);
		break;

	case POS_DEAD:
		act("$n is DEAD!!", victim, 0, 0, TO_ROOM);
		if (IS_NPC(victim))
			wiznet("$N is DEAD!! :(", victim, NULL, WIZ_MOBDEATHS, 0, 0);
		else {
			wiznet("{W$N{x is DEAD!! :(", victim, NULL, WIZ_DEATHS, 0, 0);
			if (IS_SET(victim->pcdata->plr_flags, PLR_LOG) || fLogAll)
				log_printf("%s is DEAD!! :(", victim->name);
		}
		char_puts("You have been KILLED!!\n\n", victim);
		break;

	default:
		if (dam_type == DAM_HUNGER || dam_type == DAM_THIRST) break;
		if (dam > victim->max_hit / 4)
			char_puts("That really did HURT!\n", victim);
		if (victim->hit < victim->max_hit / 4)
			char_puts("You sure are BLEEDING!\n", victim);
		break;
	}

	/*
	 * Sleep spells and extremely wounded folks.
	 */
	if (!IS_AWAKE(victim) && victim->fighting)
		victim->fighting = NULL;
	debug_printf(11, "ddd006");

	/*
	 * Payoff for killing things.
	 */
	if (victim->position == POS_DEAD) {
		handle_death(ch, victim);
		return TRUE;
	}

	if (victim == ch)
		return TRUE;

	/*
	 * Mob's stop fighting if victim->hp <= 0;
	 */

	if (victim->position <= POS_STUNNED && IS_NPC(ch)
	&& IS_PC(victim)
	&& get_curr_stat(victim, STAT_CHA) > number_range(0, 33)){
		ch->last_fought = NULL;
		stop_fighting(ch, FALSE);
		remove_mind(ch, victim->name);
		return TRUE;
	}


	debug_printf(11, "ddd008");
	/*
	 * Take care of link dead people.
	 */
	if (!IS_NPC(victim)
	&&  victim->desc == NULL
	&&  !IS_SET(victim->comm, COMM_NOFLEE)) {
		if (number_range(0, victim->wait) == 0) {
			do_flee(victim, str_empty);
			return TRUE;
		}
	}

	/*
	 * Wimp out?
	 */
	if (IS_NPC(victim) && dam > 0 && victim->wait < PULSE_VIOLENCE / 2) {
		flag64_t act = victim->pIndexData->act;
		if ((IS_SET(act, ACT_WIMPY) && number_bits(2) == 0 &&
		     victim->hit < GET_WIMPY(victim))
		||  (IS_AFFECTED(victim, AFF_CHARM) &&
		     victim->master != NULL &&
		     victim->master->in_room != victim->in_room)
		||  (IS_AFFECTED(victim, AFF_DETECT_FEAR) &&
		     !IS_SET(act, ACT_NOTRACK))) {
			do_flee(victim, str_empty);
			victim->last_fought = NULL;
		}
	}

	if (!IS_NPC(victim)
	&&  victim->hit > 0
	&&  (victim->hit <= victim->pcdata->wimpy || IS_AFFECTED(victim, AFF_DETECT_FEAR))
	&&  victim->wait < PULSE_VIOLENCE / 2)
		do_flee(victim, str_empty);

	tail_chain();
	return TRUE;
}

static bool /*inline*/ /* inline delete by prool */
is_safe_raw(CHAR_DATA *ch, CHAR_DATA *victim)
{
	/*
	 *	ghosts are safe and cann't atack !
	 */
	if (IS_GHOST(ch) || IS_GHOST(victim))
		return TRUE;
	
	if (IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_GOD))
		return TRUE;

	if (IS_PC(victim)) {
		/*int clan;

		// clan defenders can attack anyone in their clan
		if (victim->in_room
		&&  (clan = victim->in_room->area->clan)
		&&  victim->clan != clan
		&&  ch->clan == clan)
			return FALSE;
		*/

		if (ch != victim &&
			((!IS_ATTACKER(victim) || !IS_ATTACKER(ch)) && !IS_NPC(ch)))
			return TRUE;
	}
	else if (IS_EXTRACTED(victim))
		return TRUE;

	if (victim->fighting == ch
	||  ch == victim
	||  IS_IMMORTAL(ch))
		return FALSE;

	/* handle ROOM_PEACE flags */
	if ((victim->in_room && IS_SET(victim->in_room->room_flags, ROOM_PEACE))
	||  (ch->in_room && IS_SET(ch->in_room->room_flags, ROOM_PEACE)))
		return TRUE;

	/* link dead players whose adrenalin is not gushing are safe */
	if (!IS_NPC(victim) && !IS_PUMPED(victim) && victim->desc == NULL)
		return TRUE;

	if (!IS_NPC(ch) && !IS_NPC(victim)
	&& IS_SET(ch->in_room->room_flags, ROOM_LAW)) {
/*
 *		if (!IS_SET(victim->pcdata->plr_flags, PLR_WANTED)
 *		&& !IS_SET(ch->pcdata->plr_flags, PLR_WANTED)) {
 *			char_puts("This room is under supervision of the law! "
 *				"Now you're {RCRIMINAL{x!\n", ch);
 *			SET_BIT(ch->pcdata->plr_flags, PLR_WANTED);
 *		}
 */
		if (ch->pcdata->ethos != ETHOS_CHAOTIC && (
			(victim->pcdata->ethos == ETHOS_LAWFUL && number_bits(2))
			|| (victim->pcdata->ethos == ETHOS_NEUTRAL
			   && ch->pcdata->ethos == ETHOS_LAWFUL && number_bits(3) == 0)))
		{
			switch (ch->pcdata->ethos)
			{
				case ETHOS_LAWFUL:
							char_puts("{yYou become neutral ethos!{x\n", ch);
							ch->pcdata->ethos = ETHOS_NEUTRAL;
							break;
				default:
							char_puts("{yYou become chaotic ethos!{x\n", ch);
							ch->pcdata->ethos = ETHOS_CHAOTIC;
							break;
			}
		}
	}

	return !in_PK(ch, victim);
}

/*
 * generic safe-checking function wrapper
 *
 * all the checks are done is_safe_raw to properly strip PLR_GHOST
 * flag if victim is not safe. add you checks there
 */
bool is_safe_nomessage(CHAR_DATA *ch, CHAR_DATA *victim)
{
	bool safe;
	CHAR_DATA *mount;

	if (IS_NPC(ch)
	&&  IS_AFFECTED(ch, AFF_CHARM)
	&&  ch->master
	&&  ch->in_room == ch->master->in_room)
		return is_safe_nomessage(ch->master, victim);

	if (IS_NPC(victim)
	&&  IS_AFFECTED(victim, AFF_CHARM)
	&&  victim->master)
		return is_safe_nomessage(ch, victim->master);

	if ((mount = RIDDEN(victim)))
		return is_safe_nomessage(ch, mount);

	if ((safe = is_safe_raw(ch, victim)) || IS_NPC(ch))
		return safe;

	return safe;
}

bool is_safe(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (is_safe_nomessage(ch, victim)) {
		if (!IS_ATTACKER(ch) && !IS_NPC(victim)) {
			char_puts("You must become attacker for PK.\n", ch);
		}
		act("The gods protect $N.",ch,NULL,victim,TO_CHAR);
		act("The gods protect $N from $n.",ch,NULL,victim,TO_ROOM);
		return TRUE;
	}
	return FALSE;
}

bool is_safe_spell(CHAR_DATA *ch, CHAR_DATA *victim, bool area)
{
#if 0
	if (ch == victim && !area)
		return TRUE;
#endif
	if (area) {
		if (IS_IMMORTAL(victim)
		||  is_same_group(ch, victim)
		||  ch == victim
		||  RIDDEN(ch) == victim
		||  MOUNTED(ch) == victim)
			return TRUE;
	}

	return is_safe(ch, victim);
}

/*
 * Check for parry.
 */
bool check_parry(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	int chance;
	OBJ_DATA *wield;
	int weapon_sn;

	if (!IS_AWAKE(victim))
		return FALSE;

	wield = get_eq_char(victim, loc);
	if (wield == NULL && !IS_NPC(victim))
		return FALSE;
	chance = get_skill(victim, gsn_parry) / 2.3;
	if (chance == 0)
		return FALSE;

	chance += (get_curr_stat(victim, STAT_DEX) - get_curr_stat(ch, STAT_DEX)) * 2
		- ((victim->wait > PULSE_VIOLENCE / 2) ? 10 : 0);

	/* Esli net oruzhia to proverka na hand_to_hand */
	/* sn = -1 -> exotic; sn = 0 -> hand_to_hand */
	if (IS_PC(victim) && (weapon_sn = get_weapon_sn(wield)) != -1)
		chance += (get_weapon_skill(victim, weapon_sn) -
					get_weapon_skill(ch, weapon_sn)) / 10;

	if (check_forest(victim) == FOREST_DEFENCE
		&&  (number_percent() < get_skill(victim, gsn_forest_fighting)))
	{
		chance += chance / (8 - get_skill(victim, gsn_forest_fighting) / 25);
		check_improve (victim, gsn_forest_fighting, TRUE, 7);
	} else
		check_improve (victim, gsn_forest_fighting, FALSE, 18);

	if (IS_AFFECTED(victim, AFF_HASTE))
		chance +=10;
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance -=10;
	if (IS_AFFECTED(victim, AFF_SLOW))
		chance -=15;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance +=15;
	if (!can_see(victim, ch) && number_percent() > get_skill(victim, gsn_blind_fighting))
		chance -=15;

	chance += LEVEL(victim) - LEVEL(ch);
	if (chance > 75)
		chance = UMIN(95, 75 + (chance - 75) / 3);

	if (number_percent() >= chance )
		return FALSE;

        act("You parry $n's attack.", ch, NULL, victim, TO_VICT | ACT_VERBOSE);
        act("$N parries your attack.", ch, NULL, victim, TO_CHAR | ACT_VERBOSE);

        check_weapon_damage(ch, victim, loc);
        if (number_percent() > chance) {
                /* size and weight */
				chance += ch->carry_weight / 25;
                chance -= victim->carry_weight / 20;

                if (ch->size < victim->size)
                        chance += (ch->size - victim->size) * 25;
                else
                        chance += (ch->size - victim->size) * 10;

                /* stats */
                chance += get_curr_stat(ch, STAT_STR);
                chance -= get_curr_stat(victim, STAT_DEX) * 4/3;

                if (IS_AFFECTED(ch, AFF_FLYING))
                        chance -= 10;

                /* speed */
				if (IS_NPC(ch) && IS_SET(ch->pIndexData->off_flags, OFF_FAST))
                        chance += 10;
                if (IS_NPC(victim) && IS_SET(victim->pIndexData->off_flags,
                                             OFF_FAST))
						chance -= 20;

                /* level */
                chance += (LEVEL(ch) - LEVEL(victim)) * 2;

                /* now the attack */
                if (number_percent() < (chance / 20 )) {
                        act("You couldn't manage to keep your position!",
                            ch, NULL, victim, TO_VICT);
                        act("You fall down!", ch, NULL, victim, TO_VICT);
                        act("$N couldn't manage to hold your attack "
                            "and falls down!",
                            ch, NULL, victim, TO_CHAR);
                        act("$n stunning force makes $N falling down.",
                            ch, NULL, victim, TO_NOTVICT);

						WAIT_STATE(victim, PULSE_VIOLENCE);
                        victim->position = POS_RESTING;
                }
        }

        check_improve(victim, gsn_parry, TRUE, 6);
        return TRUE;
}

/*
 * check blink
 */
bool check_blink(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (IS_NPC(victim))
		return FALSE;
	if (!IS_SET(victim->pcdata->plr_flags, PLR_BLINK))
		return FALSE;

	else if ((chance = get_skill(victim, gsn_blink) / 2) == 0)
		return FALSE;

	chance += (get_curr_stat(victim , STAT_WIS)
		- get_curr_stat(ch , STAT_WIS)) * 5 / 2
		- ((victim->wait > PULSE_VIOLENCE / 2) ? 10 : 0);

	chance += LEVEL(victim) - LEVEL(ch);
	/* Obozhau takie "plavnie" formulki... :) */
	if (chance > 75)
		chance = UMIN(95, 75 + (chance - 75) / 3);

	if (number_percent() >= chance
	||  number_percent() < 10
	||  victim->mana < 10)
		return FALSE;

	victim->mana -= UMAX(victim->level / 10, 1);

	act("You blink out $n's attack.",
	    ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N blinks out your attack.",
	    ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	check_improve(victim, gsn_blink, TRUE, 5);
	return TRUE;
}

/*
 * Check for shield block.
 */
bool check_block(CHAR_DATA *ch, CHAR_DATA *victim, int loc)
{
	int chance;

	if (!IS_AWAKE(victim))
		return FALSE;

	if (get_eq_char(victim, WEAR_SHIELD) == NULL)
		return FALSE;

	chance = get_skill(victim, gsn_shield_block) / 2;
	if (chance <= 0)
		return FALSE;

	if (check_forest(victim) == FOREST_DEFENCE 
	&& (number_percent() < get_skill(victim, gsn_forest_fighting))) {
		chance += chance / (8 - get_skill(victim, gsn_forest_fighting) / 25 );
		check_improve (victim, gsn_forest_fighting, TRUE, 7);
	} else  check_improve (victim, gsn_forest_fighting, FALSE, 18);

	if (MOUNTED(victim))
		chance *= 1.2;

	chance += LEVEL(victim) - LEVEL(ch)
		- ((victim->wait > PULSE_VIOLENCE / 2) ? 10 : 0);

	if (!can_see(victim, ch) && number_percent() > get_skill(victim, gsn_blind_fighting))
		chance -=15;

	if (chance > 75)
		chance = UMIN(95, 75 + (chance - 75) / 3);

	if (number_percent() >= chance)
		return FALSE;

	act("Your shield blocks $n's attack.",
	    ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N deflects your attack with $S shield.",
	    ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	check_shield_damage(ch, victim, loc);
	check_improve(victim, gsn_shield_block, TRUE, 6);
	return TRUE;
}

/*
 * Check for illusion
 */

/*bool check_illusion(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;
	
	if (IS_NPC(victim))
		return FALSE;

	if (!IS_SET(victim->pcdata->plr_flags, PLR_ILLUSION))
		return FALSE;
	
	else {
		chance = get_curr_stat(victim , STAT_WIS) - get_curr_stat(ch , STAT_WIS);
		if (chance > 5)
			chance = 5;
		else if (chance < -5)
			chance = -5;
		chance *= 5;
		chance = (chance + get_skill(victim, gsn_illusion)) / 3
			- ((victim->wait > PULSE_VIOLENCE / 2) ? 10 : 0);
	}
	
	if (number_percent() >= chance
	    ||  victim->mana < 50)
		return FALSE;
	victim->mana -= 10;
	act("$n's attack your illusion.",
	   ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("Your attack $N illusion.",
	   ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	check_improve(victim, gsn_illusion, TRUE, 6);
	return TRUE;
	
}
*/

/* 
 * Check for hand block
 */

bool check_hand_block(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_AWAKE(victim) 
	|| IS_NPC(victim) 
	|| get_eq_char(victim, WEAR_WIELD)
	|| ( (get_eq_char(victim, WEAR_SECOND_WIELD)
		|| get_eq_char(victim, WEAR_HOLD)
		|| get_eq_char(victim, WEAR_SHIELD) ) && number_percent() > 50)
	|| (chance = get_skill(victim, gsn_hand_block) <= 0))
		return FALSE;

	chance = URANGE(5, (chance * 2) / 5 + (LEVEL(victim) - LEVEL(ch)) * 2
			 + (get_curr_stat(victim, STAT_DEX) - get_curr_stat(ch, STAT_DEX)) * 3,
			 60)
		- ((victim->wait > PULSE_VIOLENCE / 2) ? 10 : 0);

	if (IS_AFFECTED(victim, AFF_HASTE))
		chance +=10;
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance -=10;
	if (IS_AFFECTED(victim, AFF_SLOW))
		chance -=15;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance +=15;
	if (!can_see(victim, ch) && number_percent() > get_skill(victim, gsn_blind_fighting))
		chance -=15;

	if (number_percent() < chance) {
		act("Your hand blocks $n's attack.", 
			ch, NULL, victim, TO_VICT|ACT_VERBOSE);
		act("$N blocks your attack with $S hand.",
			ch, NULL, victim, TO_CHAR|ACT_VERBOSE);
		check_improve(victim, gsn_hand_block, TRUE, 5);
		return TRUE;
	}
	return FALSE;
}



/*
 * Check for dodge.
 */
bool check_dodge(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;

	if (!IS_AWAKE(victim))
		return FALSE;

	if (MOUNTED(victim))
		return FALSE;

	chance  = (get_skill(victim, gsn_dodge) * 2) / 5;
	/* chance for high dex. */
	chance += 2 * (get_curr_stat(victim,STAT_DEX) - 18)
		- ((victim->wait > PULSE_VIOLENCE / 2) ? 10 : 0);
	
	if (check_forest(victim) == FOREST_DEFENCE 
	  && (get_skill(victim, gsn_forest_fighting) > number_percent())) {
		chance += chance / (8 - get_skill(victim, gsn_forest_fighting) / 25 );
		check_improve (victim, gsn_forest_fighting, TRUE, 7);
	} else
		check_improve (victim, gsn_forest_fighting, FALSE, 18);

	if (IS_AFFECTED(victim, AFF_HASTE))
		chance +=10;
	if (IS_AFFECTED(ch, AFF_HASTE))
		chance -=10;
	if (IS_AFFECTED(victim, AFF_SLOW))
		chance -=15;
	if (IS_AFFECTED(ch, AFF_SLOW))
		chance +=15;
	if (!can_see(victim, ch) && number_percent() > get_skill(victim, gsn_blind_fighting))
		chance -=15;

	chance += LEVEL(victim) - LEVEL(ch);
	/* If size small -> better dodge !!! */
	chance += (ch->size - victim->size) * 5;

	if (chance > 75)
		chance = UMIN(95, 75 + (chance - 75) / 3);

	if (number_percent() >= chance)
		return FALSE;

	act("You dodge $n's attack.", ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("$N dodges your attack.", ch, NULL, victim, TO_CHAR	| ACT_VERBOSE);

	if (number_percent() < get_skill(victim,gsn_dodge) / 20
	&&  !(IS_AFFECTED(ch, AFF_FLYING) || ch->position < POS_FIGHTING)) {
		/* size */
		if (victim->size < ch->size)
			/* bigger = harder to trip */
			chance -= (ch->size - victim->size) * 10;

		/* dex */
		chance += get_curr_stat(victim, STAT_DEX);
		chance -= get_curr_stat(ch, STAT_DEX) * 3 / 2;

		if (IS_AFFECTED(victim, AFF_FLYING))
			chance -= 10;

		/* speed */
		if ((IS_NPC(victim) && IS_SET(victim->pIndexData->off_flags,
					      OFF_FAST))
		||  IS_AFFECTED(victim, AFF_HASTE))
			chance += 10;
		if ((IS_NPC(ch) && IS_SET(ch->pIndexData->off_flags, OFF_FAST))
		||  IS_AFFECTED(ch, AFF_HASTE))
			chance -= 20;

		/* level */
		chance += (LEVEL(victim) - LEVEL(ch)) * 2;

		/* now the attack */
		if (number_percent() < (chance / 20)) {
			act("$n lost his postion and fall down!",
			    ch, NULL, victim, TO_VICT);
			act("As $N moves you lost your position fall down!",
			    ch, NULL, victim, TO_CHAR);
			act("As $N dodges $N's attack, $N lost his position "
			    "and falls down.", ch, NULL, victim, TO_NOTVICT);

			WAIT_STATE(ch, PULSE_VIOLENCE);
			ch->position = POS_RESTING;
		}
	}
	check_improve(victim, gsn_dodge, TRUE, 6);
	return TRUE;
}

/*
 *  check for REFLECTION
 */

/*

bool check_reflection(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int chance;
	if (IS_NPC(victim))
		return FALSE;

	if (!IS_SET(victim->pcdata->plr_flags, PLR_REFLECTION))
		return FALSE;
	
	else 
	{
		chance = get_curr_stat(victim , STAT_WIS) - get_curr_stat(ch , STAT_WIS);
		if (chance > 5)
			chance = 5 + (chance - 5) / 2;
		else if (chance < -5)
			chance = -5 + (chance +5) / 2;

		chance *= 4;
		chance = chance + get_skill(victim, gsn_reflection) / 4
			- ((victim->wait > PULSE_VIOLENCE / 2) ? 10 : 0);
	}
	if (number_percent() >= chance
	    || victim->mana < 200)
	{
		check_improve(victim, gsn_reflection, FALSE, 18);
		return FALSE;
	}
		
	victim->mana -= 20 + victim->level / 2;
	
	act("$n's attack your magical block.",
	    ch, NULL, victim, TO_VICT | ACT_VERBOSE);
	act("Your attack $N magical block.",
	    ch, NULL, victim, TO_CHAR | ACT_VERBOSE);
	    
	check_improve(victim, gsn_reflection, TRUE, 7);
	
	return TRUE;
}


 */

/*
 * Set position of a victim.
 */
void update_pos(CHAR_DATA *victim)
{
	int live_hp;

	if (victim->hit > 0) {
		if (victim->position <= POS_STUNNED) {
			if (IS_AFFECTED(victim, AFF_SLEEP)) {
				REMOVE_BIT(victim->affected_by, AFF_SLEEP);
				affect_bit_strip(victim, TO_AFFECTS, AFF_SLEEP);
			}

			victim->position = POS_STANDING;
		}
		return;
	}

	if (IS_NPC(victim) && victim->hit < 1) {
		victim->position = POS_DEAD;
		return;
	}

	live_hp = UMIN(-10, -get_curr_stat(victim, STAT_CON) * victim->level / 20);

	if (victim->hit < /*-10*/ live_hp) {
		victim->position = POS_DEAD;
		return;
	}

	if (victim->hit < /*-5*/ live_hp / 2 )
		victim->position = POS_MORTAL;
	else if (victim->hit < /*-3*/ live_hp / 3 )
		victim->position = POS_INCAP;
	else
		victim->position = POS_STUNNED;
}

/*
 * Start fights.
 */
void set_fighting(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (ch->fighting != NULL) {
		bug("Set_fighting: already fighting", 0);
		return;
	}

	if (IS_AFFECTED(ch, AFF_SLEEP)) {
		REMOVE_BIT(ch->affected_by, AFF_SLEEP);
		affect_bit_strip(ch, TO_AFFECTS, AFF_SLEEP);
	}

	ch->fighting = victim;
	ch->position = POS_FIGHTING;
}

static /*inline*/ void STOP_FIGHTING(CHAR_DATA *ch)
{
	ch->fighting = NULL;
	ch->position = IS_NPC(ch) ? ch->default_pos : POS_STANDING;
	update_pos(ch);
}

/*
 * Stop fights.
 */
void stop_fighting(CHAR_DATA *ch, bool fBoth)
{
	CHAR_DATA *fch;

	STOP_FIGHTING(ch);
	if (!fBoth)
		return;

	for (fch = char_list; fch; fch = fch->next) {
		if (fch->fighting == ch)
			STOP_FIGHTING(fch);
	}
}

/*
 * Make a corpse out of a character.
 */
OBJ_DATA * make_corpse(CHAR_DATA *ch)
{
	OBJ_DATA *corpse;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int i;

	if (IS_NPC(ch)) {
		corpse	= create_obj_of(get_obj_index(OBJ_VNUM_CORPSE_NPC),
					ch->short_descr);
		corpse->timer	= number_range(3, 6);
		if (ch->gold > 0 || ch->silver > 0) {
			OBJ_DATA *money = create_money(ch->gold, ch->silver);
			if (IS_SET(ch->form,FORM_INSTANT_DECAY))
				obj_to_room(money, ch->in_room);
			else
				obj_to_obj(money, corpse);
		}
	}
	else {
		corpse	= create_obj_of(get_obj_index(OBJ_VNUM_CORPSE_PC),
					ch->short_descr);
		if (IS_GOOD(ch))
		  i = 0;
		if (IS_EVIL(ch))
		  i = 2;
		else
		  i = 1;

		corpse->timer= number_range(25, 40);
		corpse->altar = get_altar(ch);

		if (ch->gold > 0 || ch->silver > 0)
			obj_to_obj(create_money(ch->gold, ch->silver), corpse);
	}

	corpse->owner = mlstr_dup(ch->short_descr);
	
	if (IS_ATTACKER(ch))
		SET_BIT(corpse->wear_flags, ITEM_CORPSE_ATTACKER);
	corpse->level = ch->level;

	ch->gold = 0;
	ch->silver = 0;

	for (obj = ch->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		obj_from_char(obj);
		if (obj->pIndexData->item_type == ITEM_POTION)
		    obj->timer = number_range(500,1000);
		if (obj->pIndexData->item_type == ITEM_SCROLL)
		    obj->timer = number_range(1000,2500);
		if (IS_SET(obj->extra_flags,ITEM_ROT_DEATH))  {
		    obj->timer = number_range(5,10);
		    if (obj->pIndexData->item_type == ITEM_POTION)
		       obj->timer += obj->level * 20;
		}
		REMOVE_BIT(obj->extra_flags,ITEM_VIS_DEATH);
		REMOVE_BIT(obj->extra_flags,ITEM_ROT_DEATH);

		if (IS_SET(obj->hidden_flags, OHIDE_INFINITY_SELL)  ||
		    (obj->pIndexData->limit != -1 &&
			(obj->pIndexData->count > obj->pIndexData->limit)))
		  {
		    extract_obj(obj, 0);
		    continue;
		  }
		else if (IS_SET(ch->form,FORM_INSTANT_DECAY))
		  obj_to_room(obj, ch->in_room);

		else
		  obj_to_obj(obj, corpse);
	}

	obj_to_room(corpse, ch->in_room);
	return corpse;
}

void death_cry(CHAR_DATA *ch)
{
  death_cry_org(ch, -1);
}

/*
 * Improved Death_cry contributed by Diavolo.
 */
void death_cry_org(CHAR_DATA *ch, int part)
{
	ROOM_INDEX_DATA *was_in_room;
	char *msg;
	int door;
	int vnum;

	vnum = 0;
	msg = "You hear $n's death cry.";

	if (part == -1)
	  part = number_bits(4);

	switch (part) {
	case  0:
		msg  = "$n hits the ground ... DEAD.";
		break;
	case  1:
		// if (ch->material == 0) {
		msg  = "$n splatters blood on your armor.";
		break;
		// }
		/* FALLTHRU */
	case  2:
		if (IS_SET(ch->parts, PART_GUTS)) {
			msg = "$n spills $s guts all over the floor.";
			vnum = OBJ_VNUM_GUTS;
		}
		break;
	case  3:
		if (IS_SET(ch->parts, PART_HEAD)) {
			msg  = "$n's severed head plops on the ground.";
			vnum = OBJ_VNUM_SEVERED_HEAD;
		}
		break;
	case  4:
		if (IS_SET(ch->parts, PART_HEART)) {
			msg  = "$n's heart is torn from $s chest.";
			vnum = OBJ_VNUM_TORN_HEART;
		}
		break;
	case  5:
		if (IS_SET(ch->parts, PART_ARMS)) {
			msg  = "$n's arm is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_ARM;
		}
		break;
	case  6:
		if (IS_SET(ch->parts, PART_LEGS)) {
			msg  = "$n's leg is sliced from $s dead body.";
			vnum = OBJ_VNUM_SLICED_LEG;
		}
		break;
	case 7:
		if (IS_SET(ch->parts, PART_BRAINS)) {
			msg = "$n's head is shattered, and $s brains splash all over you.";
			vnum = OBJ_VNUM_BRAINS;
		}
		break;
	}

	act(msg, ch, NULL, NULL, TO_ROOM);

	if (vnum) {
		OBJ_DATA *obj;

		obj = create_obj_of(get_obj_index(vnum), ch->short_descr);
		obj->level = ch->level;
		obj->owner = mlstr_dup(ch->short_descr);
		obj->timer = number_range(4, 7);

		if (obj->pIndexData->item_type == ITEM_FOOD) {
			if (IS_SET(ch->form,FORM_POISON))
				obj->value[3] = 1;
			else if (!IS_SET(ch->form,FORM_EDIBLE))
				SET_BIT(obj->extra_flags, ITEM_NOT_EDIBLE);
		}

		obj_to_room(obj, ch->in_room);
	}

	if (IS_NPC(ch))
		msg = "You hear something's death cry.";
	else
		msg = "You hear someone's death cry.";

	if ((was_in_room = ch->in_room)) {
		for (door = 0; door <= 5; door++) {
			EXIT_DATA *pexit;

			if ((pexit = was_in_room->exit[door]) != NULL
			&&   pexit->to_room.r != NULL
			&&   pexit->to_room.r != was_in_room) {
				ch->in_room = pexit->to_room.r;
				act(msg, ch, NULL, NULL, TO_ROOM);
			}
		}
		ch->in_room = was_in_room;
	}
}

	/* return corpse if has been maked */
OBJ_DATA * raw_kill_org(CHAR_DATA *ch, CHAR_DATA *victim, int part)
{
	CHAR_DATA *tmp_ch, *tmp_ch_next;
	OBJ_DATA *obj,*obj_next;
	int i;
	OBJ_DATA *tattoo, *clanmark, *corpse;
	//bool crumble;

	if (IS_NPC(victim)
	&& IS_SET(victim->pIndexData->act, ACT_GOD))
		return NULL;

	for (obj = victim->carrying;obj != NULL;obj = obj_next) {
		obj_next = obj->next_content;
		if (obj->wear_loc != WEAR_NONE
		&&  oprog_call(OPROG_DEATH, obj, victim, NULL)) {
			victim->position = POS_STANDING;
			return NULL;
		}
	}

	/* don't remember killed victims anymore */
	if (IS_NPC(ch))
		remove_mind(ch, victim->name);

	stop_fighting(victim, TRUE);
	if (ch->fighting == victim)
		stop_fighting(ch, FALSE);
//	debug_printf(8, "raw_kill_org000");

	rating_update(ch, victim);
	debug_printf(8, "raw_kill_org010");
	quest_handle_death(ch, victim);
	debug_printf(8, "raw_kill_org020");
	RESET_FIGHT_TIME(victim);
	debug_printf(8, "raw_kill_org030");
	victim->last_death_time = current_time;
	death_cry_org(victim, part);
	
	debug_printf(8, "raw_kill_org001");

	tattoo = get_eq_char(victim, WEAR_TATTOO);
	clanmark = get_eq_char(victim, WEAR_CLANMARK);
	if (tattoo != NULL)
		obj_from_char(tattoo);
	if (clanmark != NULL)
		obj_from_char(clanmark);

	if (IS_NPC(victim)
	&& !IS_SET(victim->pIndexData->act, ACT_NOCRUMBLE_BODY)
	&& number_percent() < UMAX(ch->level - victim->level - 10, 0) * 5)
	{
		act("You last hit crumble body of $N in little pieces.", ch, NULL, victim, TO_CHAR);
		act("$n's last hit crumbles body of $N in little pieces.", ch, NULL, victim, TO_ROOM);
		//crumble = TRUE;
		corpse = NULL;
	} else {
		corpse = make_corpse(victim);
		//crumble = FALSE;
	}
	
	if (IS_NPC(victim)) {
		debug_printf(8, "raw_kill_org012");
		victim->pIndexData->killed++;
		extract_char(victim, (corpse ? 0 : XC_F_COUNT)/*(crumble ? XC_F_COUNT : 0)*/);
		return corpse;
	}
	debug_printf(8, "raw_kill_org022");

	char_puts("You turn into an invincible ghost for a few minutes.\n"
		  "As long as you don't attack anything.\n",
		  victim);

	extract_char(victim, XC_F_INCOMPLETE | (corpse ? 0 : XC_F_COUNT)
				/*(crumble ? XC_F_COUNT : 0)*/);

	while (victim->affected)
		affect_remove(victim, victim->affected);
	victim->affected_by	= 0;
	for (i = 0; i < 4; i++)
		victim->armor[i] = -100;
	victim->position	= POS_RESTING;
	victim->hit		= victim->max_hit / 10;
	victim->mana		= victim->max_mana / 10;
	victim->move		= victim->max_move;
	update_pos(victim);
	set_ghost(victim);

	/* RT added to prevent infinite deaths */
	REMOVE_BIT(victim->pcdata->plr_flags, PLR_BOUGHT_PET);

	victim->pcdata->condition[COND_THIRST] = 40;
	victim->pcdata->condition[COND_HUNGER] = 40;
	victim->pcdata->condition[COND_FULL] = 40;
	victim->pcdata->condition[COND_BLOODLUST] = 40;
	victim->pcdata->condition[COND_DESIRE] = 40;
	victim->pcdata->condition[COND_DRUNK] = 0;

	if (tattoo != NULL) {
		obj_to_char(tattoo, victim);
		equip_char(victim, tattoo, WEAR_TATTOO);
	}
	
	debug_printf(8, "raw_kill_org004");

	if (clanmark != NULL) {
		obj_to_char(clanmark, victim);
		equip_char(victim, clanmark, WEAR_CLANMARK);
	}

	if (victim->level > 1)
		save_char_obj(victim, FALSE);
		
	debug_printf(8, "raw_kill_org006");

	/*
	 * Calm down the tracking mobiles
	 */
	for (tmp_ch = npc_list; tmp_ch; tmp_ch = tmp_ch_next) {
		tmp_ch_next = tmp_ch->next;
		if (tmp_ch->last_fought == victim)
			tmp_ch->last_fought = NULL;
		remove_mind(tmp_ch, victim->name);
		if (tmp_ch->target == victim 
		&&  tmp_ch->pIndexData->vnum == MOB_VNUM_STALKER) {
			//doprintf(do_clan, tmp_ch,
			//	"%s is dead and I can leave the realm.",
			//	PERS(victim, tmp_ch));
			extract_char(tmp_ch, 0);
		}
	}
	return corpse;
}

void group_gain(CHAR_DATA *ch, CHAR_DATA *victim)
{
	CHAR_DATA *lch;
	CHAR_DATA *gch;
	int xp;
	int members;
	int group_levels;

	if (!IS_NPC(victim) || victim == ch)
		return;

	if (IS_SET(victim->pIndexData->act, ACT_PET)
	||  victim->pIndexData->vnum < 100
	||  victim->master
	||  victim->leader)
		return;

	lch = ch->leader ? ch->leader : ch;

	members = 0;
	group_levels = 0;
	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (is_same_group(gch, ch)) {
			if (IS_NPC(gch))
			{
				if (IS_SET(gch->pIndexData->act, ACT_SUMMONED))
					continue;
				if (gch->level < lch->level
				|| gch->level - lch->level <= 12)
					members++;

				if (gch->level < lch->level)
					group_levels += lch->level;
				else 
					group_levels += gch->level;
			} else {
				if (abs(gch->level - lch->level) <= 8)
					members++;
				group_levels += gch->level;
			}

		}
	}

	for (gch = ch->in_room->people; gch; gch = gch->next_in_room) {
		if (!is_same_group(gch, ch) || IS_NPC(gch))
			continue;

		if (gch->level - lch->level > 8) {
			char_puts("You are too high for this group.\n", gch);
			continue;
		}

		if (gch->level - lch->level < -8) {
			char_puts("You are too low for this group.\n", gch);
			continue;
		}

		xp = xp_compute(gch, victim, group_levels, members);
		gain_exp(gch, xp, TRUE, TRUE);
	}
}

/*
 * Compute xp for a kill.
 * Also adjust alignment of killer.
 * Edit this function to change xp computations.
 */
int xp_compute(CHAR_DATA *gch, CHAR_DATA *victim, int total_levels, int members)
{
	int xp;
	int base_exp;
	int level_range = victim->level - gch->level;
//	int neg_cha = 0, pos_cha = 0;
	int align = 0;

/* base exp */
	switch (level_range) {
	case -9:	base_exp =   1; 	break;
	case -8:	base_exp =   2; 	break;
	case -7:	base_exp =   5; 	break;
	case -6:	base_exp =   9; 	break;
	case -5:	base_exp =  11; 	break;
	case -4:	base_exp =  22; 	break;
	case -3:	base_exp =  33; 	break;
	case -2:	base_exp =  43; 	break;
	case -1:	base_exp =  60; 	break;
	case  0:	base_exp =  74; 	break;
	case  1:	base_exp =  84; 	break;
	case  2:	base_exp =  99; 	break;
	case  3:	base_exp = 121; 	break;
	case  4:	base_exp = 143; 	break;
	default:
		if (level_range > 4)
			base_exp = 140 + 20 * (level_range - 4);
		else
			base_exp = 0;
	}

/* calculate exp multiplier */
#if 0
	if (IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_NOALIGN))
		xp = base_exp;
	else
#endif
#if 0
	if ((IS_EVIL(gch) && IS_GOOD(victim))
	||  (IS_EVIL(victim) && IS_GOOD(gch)))
		xp = base_exp * 8/5;
	else if (IS_GOOD(gch) && IS_GOOD(victim))
		xp = base_exp / 2;
	else if (!IS_NEUTRAL(gch) && IS_NEUTRAL(victim))
		xp = base_exp * 1.1;
	else if (IS_NEUTRAL(gch) && !IS_NEUTRAL(victim))
		xp = base_exp * 1.3;
	else
		xp = base_exp;
#else
	xp = base_exp;
#endif

	/* more exp at the low levels */
	if (gch->level < 6)
		xp = 15 * xp / (gch->level + 4);

	/* randomize the rewards */
	xp = number_range(xp * 3/4, xp * 5/4);

/* adjust for grouping */

	xp = xp * gch->level / total_levels;
	if (members <= 5)
	{
		xp *= members;
		xp = xp * (150 - (members - 1)*15) / 100;
	}

#if 0
	xp += (xp * (gch->max_hit - gch->hit)) / (gch->max_hit * 5);
#endif

	if (IS_GOOD(victim)) {
		gch->pcdata->good_killed++;
		if (IS_EVIL(gch)){
			if (victim->alignment > (0 - gch->alignment)
				&& gch->pcdata->good_killed > gch->pcdata->evil_killed){
				align = - (victim->alignment + gch->alignment) / 4;
				if (gch->alignment < -900)
					align /= 4;
				else if (gch->alignment < -500)
					align /= 2;
				align = URANGE(-10, align, -1);
			}
		} else if (IS_GOOD(gch)){
			align = - (victim->alignment / 20);
			if (gch->alignment > 900)
				align *= 2;
			else if (gch->alignment > 500)
				align *= 1.5;
			else if (gch->alignment < 200)
				align /= 2;
		} else { // if not evil & not good == neutral
			align = - (victim->alignment / 6);
			align = URANGE( -10, align, -1);
			align = UMAX(- 100 - gch->alignment, align);
		}
	}
	else if (IS_NEUTRAL(victim)) {
		gch->pcdata->neutral_killed++;
	}
	else if (IS_EVIL(victim)) {
		gch->pcdata->evil_killed++;
		if (IS_GOOD(gch)){
			if ((0 - victim->alignment) > gch->alignment
				&& gch->pcdata->evil_killed > gch->pcdata->good_killed){
				align = - (victim->alignment + gch->alignment) / 4;
				if (gch->alignment > 900)
					align /= 4;
				else if (gch->alignment > 500)
					align /= 2;
				align = URANGE(1, align, 10);
			}
				
		} else if (IS_EVIL(gch)){
			align = - (victim->alignment / 30);
			if (gch->alignment < -900)
				align *= 1.5;
			else if (gch->alignment > -200)
				align /= 2;

		} else { // is_neutral
			align = - (victim->alignment / 3);
			align = URANGE( 1, align, 10);
			align = UMIN(100 - gch->alignment, align);
		}
	}
	
	if ((align > 0 && gch->alignment !=  1000) ||
	    (align < 0 && gch->alignment != -1000))
		gch->alignment = URANGE(-1000, gch->alignment + align, 1000);
	wrath_of_gods(gch, align);
	

/*	if (IS_GOOD(gch)) {
		if (IS_GOOD(victim)) {
			gch->pcdata->good_killed++;
			neg_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			gch->pcdata->neutral_killed++;
			pos_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			gch->pcdata->evil_killed++;
			pos_cha = 1;
		}
	}
	else if (IS_NEUTRAL(gch)) {
		if (IS_GOOD(victim)) {
			gch->pcdata->good_killed++;
			pos_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			gch->pcdata->neutral_killed++;
			neg_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			gch->pcdata->has_killed++;
			pos_cha =1;
		}
	}
	else if (IS_EVIL(gch)) {
		if (IS_GOOD(victim)) {
			gch->pcdata->has_killed++;
			pos_cha = 1;
		}
		else if (IS_NEUTRAL(victim)) {
			gch->pcdata->has_killed++;
			pos_cha = 1;
		}
		else if (IS_EVIL(victim)) {
			gch->pcdata->anti_killed++;
			neg_cha = 1;
		}
	}
 */


/*	if (neg_cha) {
		if ((gch->pcdata->anti_killed % 100) == 99) {
			char_printf(gch, "You have killed %d %s up to now.\n",
				    gch->pcdata->anti_killed,
				    IS_GOOD(gch) ?	"goods" :
				    IS_EVIL(gch) ?	"evils" :
							"neutrals");
			if (gch->perm_stat[STAT_CHA] > 3 && IS_GOOD(gch)) {
				char_puts("So your charisma "
					  "has reduced by one.\n", gch);
				gch->perm_stat[STAT_CHA] -= 1;
			}
		}
	}
	else if (pos_cha) {
		if ((gch->pcdata->has_killed % 200) == 199) {
			char_printf(gch, "You have killed %d %s up to now.\n",
				    gch->pcdata->anti_killed,
				    IS_GOOD(gch) ?	"anti-goods" :
				    IS_EVIL(gch) ?	"anti-evils" :
							"anti-neutrals");
			if (gch->perm_stat[STAT_CHA] <
						get_max_train(gch, STAT_CHA)
			&&  IS_GOOD(gch)) {
				char_puts("So your charisma "
					  "has increased by one.\n", gch);
				gch->perm_stat[STAT_CHA] += 1;
			}
		}
	}
 */
	return xp;
}

void dam_message(CHAR_DATA *ch, CHAR_DATA *victim,
		 int dam, int dt, bool immune, int dam_type)
{
	mlstring *msg_char;
	mlstring *msg_vict = NULL;
	mlstring *msg_notvict;
	mlstring *attack = NULL;
	bool  vs_vp = FALSE;
	DAMAGES_MSG *dm = NULL;
	int i;
	struct attack_type *atd;
	char damstr[2][100];
	
	if (dam){
		snprintf (damstr[0], sizeof(damstr[1]), " {M(%.2f%%){x", (float)dam / victim->max_hit * 100);
		snprintf (damstr[1], sizeof(damstr[2]), " {M(%.2f%%  %d hp){x",
				(float)dam / victim->max_hit * 100, dam);
	} else {
		damstr[0][0] = 0;
		damstr[1][0] = 0;
	}
	
	for (i = 0; i < damages_data.list_damages_msg.nused; i++)
	{
		dm = varr_get(&damages_data.list_damages_msg, i);
		
		if (dm == NULL)
			continue;
		if (dam <= dm->max_dam)
		{
			if (dam < dm->min_dam) {
				if ((dm->max_dam == dm->min_dam) &&
                                    (dm->min_dam == -1))
					break;
				continue;
			}
			break;
		}
		
	}

	if (dm == NULL)
	{
		bug("Dam_message: Damage %d not in damages.txt.", dam);
		dm = VARR_GET(&damages_data.list_damages_msg, damages_data.list_damages_msg.nused);
	}

	if (dt == TYPE_HIT || dt == TYPE_HUNGER) {
		if (ch == victim) {
			switch (dam_type) {
			case DAM_HUNGER:
				msg_notvict = damages_data.to_n_dt_cv_hunger.msg_notvict;
				msg_char = damages_data.to_n_dt_cv_hunger.msg_char;
				break;

			case DAM_THIRST:
				msg_notvict = damages_data.to_n_dt_cv_thirst.msg_notvict;
				msg_char = damages_data.to_n_dt_cv_thirst.msg_char;
				break;

			case DAM_LIGHT_V:
				msg_notvict = damages_data.to_n_dt_cv_light.msg_notvict;
				msg_char = damages_data.to_n_dt_cv_light.msg_char;
				break;

			case DAM_TRAP_ROOM:
				msg_notvict = damages_data.to_n_dt_cv_trap.msg_notvict;
				msg_char = damages_data.to_n_dt_cv_trap.msg_char;
				break;

			default:
				msg_notvict = damages_data.to_n_dt_cv.msg_notvict;
				msg_char = damages_data.to_n_dt_cv.msg_char;
				break;
			}
		}
		else {
			msg_notvict = damages_data.to_n_dt.msg_notvict;
			msg_char = damages_data.to_n_dt.msg_char;
			msg_vict = damages_data.to_n_dt.msg_vict;
		}
	}
	else {
		skill_t *sk;

#define MAX_DAMAGE_VNUM 40
#define VIEW_PERCENT_LEVEL 15
#define VIEW_HP_LEVEL	   60

		if ((sk = skill_lookup(dt)))
			attack = sk->noun_damage;
		else if (dt >= TYPE_HIT && dt <= TYPE_HIT + MAX_DAMAGE_VNUM)
		{
			attack = (atd = GET_ATTACK_T(dt - TYPE_HIT)) ?
			atd->noun :
			((struct attack_type *) GET_ATTACK_T(0))->noun;
		} else {
			bug("Dam_message: bad dt %d.", dt);
			dt = TYPE_HIT;
			attack = ((struct attack_type *) GET_ATTACK_T(0))->noun;
		}

		if (immune) {
			if (ch == victim) {
				msg_notvict = damages_data.to_imm_cv.msg_notvict;
				msg_char = damages_data.to_imm_cv.msg_char;
			}
			else {
				msg_notvict = damages_data.to_imm.msg_notvict;
				msg_char = damages_data.to_imm.msg_char;
				msg_vict = damages_data.to_imm.msg_vict;
			}
		}
		else {
			vs_vp = TRUE; /* vs = vp */

			if (ch == victim) {
				msg_notvict = damages_data.to_sp_cv.msg_notvict;
				msg_char = damages_data.to_sp_cv.msg_char;
			}
			else {
				msg_notvict = damages_data.to_sp.msg_notvict;
				msg_char = damages_data.to_sp.msg_char;
				msg_vict = damages_data.to_sp.msg_vict;
			}
		}
	}
	
	if (!immune
	   && ch->level >= VIEW_PERCENT_LEVEL
	   &&	!((IS_AFFECTED(ch, AFF_BLIND)
	   	&& !(IS_PC(ch)
	   	    && IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT))
	   	&& ch != victim
	   	)
	   )
	)
	{
		if (ch->level < VIEW_HP_LEVEL || !IS_SET(ch->comm, COMM_HPDAMAGE))
			msg_char = mlstr_printf(msg_char, damstr[0]);
		else
			msg_char = mlstr_printf(msg_char, damstr[1]);
	} else
		msg_char = mlstr_printf(msg_char, str_empty);

	/***/
	if (ch == victim) {
		act_puts3_mlstr(msg_notvict, ch, dm->msg_p, NULL, attack,
			  TO_ROOM | ACT_TRANS | ACT_NOBLIND | ACT_FULLMLSTR, POS_RESTING);
	
		act_puts3_mlstr(msg_char, ch, vs_vp ? dm->msg_p : dm->msg_s,
			NULL, attack, TO_CHAR | ACT_TRANS | ACT_FULLMLSTR, POS_RESTING);
	}
	else {
		act_puts3_mlstr(msg_notvict, ch, dm->msg_p, victim, attack,
			  TO_NOTVICT | ACT_TRANS | ACT_NOBLIND | ACT_FULLMLSTR, POS_RESTING);

		act_puts3_mlstr(msg_char, ch, vs_vp ? dm->msg_p : dm->msg_s,
			victim, attack, TO_CHAR | ACT_TRANS | ACT_FULLMLSTR, POS_RESTING);
		
		if (victim->level < VIEW_PERCENT_LEVEL
		|| (IS_AFFECTED(victim, AFF_BLIND)
			&& !(!IS_NPC(victim) && IS_SET(victim->pcdata->plr_flags, PLR_HOLYLIGHT))
		   )
		)
			msg_vict = mlstr_printf(msg_vict, str_empty);
		else if (victim->level < VIEW_HP_LEVEL || !IS_SET(victim->comm, COMM_HPDAMAGE))
			msg_vict = mlstr_printf(msg_vict, damstr[0]);
		else
			msg_vict = mlstr_printf(msg_vict, damstr[1]);
		
		act_puts3_mlstr(msg_vict, ch, dm->msg_p, victim, attack,
			  TO_VICT | ACT_TRANS | ACT_FULLMLSTR, POS_RESTING);
		mlstr_free(msg_vict);
	}
	mlstr_free(msg_char);
}

void do_kill(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Kill whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (ch->position == POS_FIGHTING) {
		if (victim == ch->fighting)
			char_puts("You do the best you can!\n", ch);
		else if (victim->fighting != ch)
			char_puts("One battle at a time, please.\n",ch);
		else {
			act("You start aiming at $N.",ch,NULL,victim,TO_CHAR);
			ch->fighting = victim;
		}
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("You must MURDER a player.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("You hit yourself.  Ouch!\n", ch);
		multi_hit(ch, ch, TYPE_UNDEFINED);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (is_safe(ch, victim))
		return;
	
	WAIT_STATE(ch, PULSE_VIOLENCE);

	if (!IS_EVIL(victim))
		change_faith(ch, RELF_FIGHT_NONEVIL, 0);

	change_faith(ch, RELF_FI_STRONG_V, victim->level - ch->level);
	
	if ((chance = get_skill(ch, gsn_mortal_strike))
	&&  get_eq_char(ch, WEAR_WIELD)
	&&  ch->level > (victim->level - 5)) {
		chance /= 30;
		chance += 1 + (ch->level - victim->level) / 2;
		if (number_percent() < chance) {
			act_puts("Your flash strike instantly slays $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n flash strike instantly slays $N!",
				 ch, NULL, victim, TO_NOTVICT,
				 POS_RESTING);
			act_puts("$n flash strike instantly slays you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			damage(ch, victim, (victim->hit + 1),
			       gsn_mortal_strike, DAM_NONE, DAMF_SHOW);
			check_improve(ch, gsn_mortal_strike, TRUE, 1);
			return;
		} else
			check_improve(ch, gsn_mortal_strike, FALSE, 3);
	}

	multi_hit(ch, victim, TYPE_UNDEFINED);
}

void do_murde(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to MURDER, spell it out.\n", ch);
	return;
}

void do_murder(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Murder whom?\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM)
	||  (IS_NPC(ch) && IS_SET(ch->pIndexData->act, ACT_PET)))
		return;

	if ((victim = get_char_room(ch, arg)) == NULL) {
		WAIT_STATE(ch, MISSING_TARGET_DELAY);
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Suicide is a mortal sin.\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_CHARM) && ch->master == victim) {
		act("$N is your beloved master.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (ch->position == POS_FIGHTING) {
		char_puts("You do the best you can!\n", ch);
		return;
	}

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, PULSE_VIOLENCE);

	if (IS_PC(victim)
	&& check_bare_char(victim))
		change_faith(ch, RELF_FI_BARE_PC_V, 0);

	if (!IS_EVIL(victim))
		change_faith(ch, RELF_FIGHT_NONEVIL, 0);

	if ((chance = get_skill(ch, gsn_mortal_strike))
	&&  get_eq_char(ch, WEAR_WIELD)
	&&  ch->level > (victim->level - 5)) {
		chance /= 30;
		chance += 1 + (ch->level - victim->level) / 2;
		if (number_percent() < chance) {
			act_puts("Your flash strike instantly slays $N!",
				 ch, NULL, victim, TO_CHAR, POS_RESTING);
			act_puts("$n flash strike instantly slays $N!",
				 ch, NULL, victim, TO_NOTVICT,
				 POS_RESTING);
			act_puts("$n flash strike instantly slays you!",
				 ch, NULL, victim, TO_VICT, POS_DEAD);
			damage(ch, victim, (victim->hit + 1),
			       gsn_mortal_strike, DAM_NONE, DAMF_SHOW);
			check_improve(ch, gsn_mortal_strike, TRUE, 1);
			return;
		} else
			check_improve(ch, gsn_mortal_strike, FALSE, 3);
	}

	multi_hit(ch, victim, TYPE_UNDEFINED);
}

void do_flee(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *victim;
	int attempt;
	class_t *cl;

	if (RIDDEN(ch)) {
		char_puts("You should ask to your rider!\n", ch);
		return;
	}

	if (MOUNTED(ch))
		do_dismount(ch, str_empty);

	if ((victim = ch->fighting) == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	if ((cl = class_lookup(ch->class))
	&&  !CAN_FLEE(ch, cl)) {
		 char_puts("Your honour doesn't let you flee, "
			   "try dishonoring yourself.\n", ch);
		 return;
	}

	was_in = ch->in_room;
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		door = number_door();
		if (!can_move_in_exit(ch, (pexit = was_in->exit[door]), FALSE)
		         || (IS_SET(pexit->exit_info , EX_NOFLEE))
		         || (IS_NPC(ch)
		             && IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB)))
			continue;

		change_faith(ch, RELF_DO_FLEE, 0);
		move_char(ch, door, FALSE);
		if ((now_in = ch->in_room) == was_in)
		    continue;

		ch->in_room = was_in;
		act("$n has fled!", ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			act_puts("You fled from combat!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			if (ch->level < LEVEL_HERO) {
				char_printf(ch, "You lose %d exps.\n", ch->level);
				gain_exp(ch, - ch->level, FALSE, FALSE);
			}
		} else
			ch->last_fought = NULL;

		stop_fighting(ch, TRUE);
		return;
	}

	char_puts("PANIC! You couldn't escape!\n", ch);
	return;
}

void do_sla(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to SLAY, spell it out.\n", ch);
	return;
}

void do_slay(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Slay whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (ch == victim) {
		char_puts("Suicide is a mortal sin.\n", ch);
		return;
	}

	if ( ch->level < victim->level && !IS_NPC(victim)) {
		char_puts("You failed.\n", ch);
		return;
	}

	act("You slay $M in cold blood!", ch, NULL, victim, TO_CHAR);
	act("$n slays you in cold blood!", ch, NULL, victim, TO_VICT);
	act("$n slays $N in cold blood!", ch, NULL, victim, TO_NOTVICT);
	raw_kill(ch, victim);
}

/*
 * Check for obj dodge.
 */
bool check_obj_dodge(CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj, int bonus)
{
	int chance;

	if (!IS_AWAKE(victim) || MOUNTED(victim))
		return FALSE;

	if (IS_PC(victim) && HAS_SKILL(victim, gsn_spellbane) &&
		 number_percent() < 50) {
		/*
		if (victim->pcdata->clan_status) {
			act("You catch $p that had been shot to you.",
			    ch, obj, victim, TO_VICT);
			act("$N catches $p that had been shot to $M.",
			    ch, obj, victim, TO_CHAR);
			act("$n catches $p that had been shot to $m.",
			    victim, obj, ch, TO_NOTVICT);
			obj_to_char(obj, victim);
		}
		*/
		return TRUE;
	}

	if (IS_NPC(victim))
		chance  = UMIN(40, victim->level / 2);
	else {
		chance  = get_skill(victim, gsn_dodge) / 2;
		/* chance for high dex. */
		chance += 2 * (get_curr_stat(victim, STAT_DEX) - 18);
	}

	chance -= bonus; // bonus - eto chance popadania arrow/spear
	chance = UMAX(5, chance / 2);
	if (number_percent() > chance)
		return FALSE;

	act("You dodge $p that had been shot to you.",
	    ch, obj, victim, TO_VICT);
	act("$N dodges $p that had been shot to $M.",
	    ch, obj, victim, TO_CHAR);
	act("$n dodges $p that had been shot to $m.",
	    victim, obj, ch, TO_NOTVICT);
	obj_to_room(obj, victim->in_room);
	check_improve(victim, gsn_dodge, TRUE, 6);

	return TRUE;
}

void do_dishonor(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *was_in;
	ROOM_INDEX_DATA *now_in;
	CHAR_DATA *gch;
	int attempt, level = 0;
	int sn_dishonor;
	int chance;

	if (RIDDEN(ch)) {
		char_puts("You should ask to your rider!\n", ch);
		return;
	}

	if ((sn_dishonor = sn_lookup("dishonor")) < 0
	||  !HAS_SKILL(ch, sn_dishonor)) {
		char_puts("Which honor?\n", ch);
		return;
	}

	if (ch->fighting == NULL) {
		if (ch->position == POS_FIGHTING)
			ch->position = POS_STANDING;
		char_puts("You aren't fighting anyone.\n", ch);
		return;
	}

	for (gch = char_list; gch; gch = gch->next)
		  if (is_same_group(gch, ch->fighting) || gch->fighting == ch)
			level += gch->level;

	if ((ch->fighting->level - ch->level) < 5 && ch->level > (level / 3)) {
		 char_puts("Your fighting doesn't worth "
			   "to dishonor yourself.\n", ch);
		 return;
	}

	was_in = ch->in_room;
	chance = get_skill(ch, sn_dishonor);
	for (attempt = 0; attempt < 6; attempt++) {
		EXIT_DATA *pexit;
		int door;

		if (number_percent() >= chance)
			continue;

		door = number_door();
		if (!can_move_in_exit(ch, (pexit = was_in->exit[door]), FALSE)
		|| IS_SET(pexit->exit_info, EX_NOFLEE)
		|| (IS_NPC(ch) &&
		    IS_SET(pexit->to_room.r->room_flags, ROOM_NOMOB)))
			continue;

		change_faith(ch, RELF_DO_FLEE, 0);
		move_char(ch, door, FALSE);
		if ((now_in = ch->in_room) == was_in)
			continue;

		ch->in_room = was_in;
		act("$n has dishonored $mself!",
		    ch, NULL, NULL, TO_ROOM);
		ch->in_room = now_in;

		if (!IS_NPC(ch)) {
			char_puts("You dishonored yourself "
				     "and flee from combat.\n",ch);
			if (ch->level < LEVEL_HERO) {
				char_printf(ch, "You lose %d exps.\n",
						ch->level * 2);
				gain_exp(ch, - (ch->level) * 2, FALSE, FALSE);
			}
		}
		else
			ch->last_fought = NULL;

		stop_fighting(ch, TRUE);
		if (MOUNTED(ch))
			do_dismount(ch,str_empty);

		check_improve(ch, sn_dishonor, TRUE, 1);
		return;
	}

	char_puts("PANIC! You couldn't escape!\n", ch);
	check_improve(ch, sn_dishonor, FALSE, 1);
}

void do_surrender(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *mob;

	if (!IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	if ((mob = ch->fighting) == NULL) {
		char_puts("But you're not fighting!\n", ch);
		return;
	}
	act("You surrender to $N!", ch, NULL, mob, TO_CHAR);
	act("$n surrenders to you!", ch, NULL, mob, TO_VICT);
	act("$n tries to surrender to $N!", ch, NULL, mob, TO_NOTVICT);
	stop_fighting(ch, TRUE);
 
	if (!IS_NPC(ch) && IS_NPC(mob) 
	&&  (!HAS_TRIGGER(mob, TRIG_SURR) ||
	     !mp_percent_trigger(mob, ch, NULL, NULL, TRIG_SURR))) {
		act("$N seems to ignore your cowardly act!",
		    ch, NULL, mob, TO_CHAR);
		multi_hit(mob, ch, TYPE_UNDEFINED);
	}
}
