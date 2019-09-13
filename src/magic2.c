/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: magic2.c,v 1.31 2003/04/22 07:35:22 xor Exp $
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

#include "merc.h"
#include "fight.h"
#include "quest.h"
#include "rating.h"
#include "fight.h"

DECLARE_DO_FUN(do_scan2);
/* command procedures needed */
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_kill		);
DECLARE_DO_FUN(do_wear		);
int	find_door	(CHAR_DATA *ch, char *arg);
int	check_exit	(const char *argument);

//extern const char *target_name;
flag32_t lookup_spell_msc(int sn);

ROOM_INDEX_DATA * check_place(CHAR_DATA *ch, const char *argument) 
{
	EXIT_DATA *pExit;
	ROOM_INDEX_DATA *dest_room;
	int number,door;
	int range = (ch->level / 10) + 1;
	char arg[MAX_INPUT_LENGTH];
	
	number = number_argument(argument, arg, sizeof(arg));
	if ((door = check_exit(arg)) == -1)
		return NULL;

	dest_room = ch->in_room;
	while (number > 0)
	{
		number--;
		if (--range < 1)
			return NULL;
		if (!can_look_in_exit(ch, (pExit = dest_room->exit[door])))
			break;
		dest_room = pExit->to_room.r;
		if (number < 1)
			return dest_room;
	}
	return NULL;
}

bool make_spell(SPELL_FUN *fun, int sn, int level, CHAR_DATA *ch, void *vo,
		int target, int percent)
{
	spell_spool_t sspellb;

	sspellb.sn	= sn;
	sspellb.level	= level;
	sspellb.ch	= ch;
	sspellb.vo	= vo;
	sspellb.target	= target;
	sspellb.percent	= percent;
	sspellb.arg	= str_empty;

	return fun(&sspellb);
}
	
bool spell_portal(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal/*, *stone*/;

	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->level >= sspell->level + 3
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_NONE, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}   

	/*
	stone = get_eq_char(sspell->ch, WEAR_HOLD);
	if (!IS_IMMORTAL(sspell->ch) 
	&&  (stone == NULL || stone->pIndexData->item_type != ITEM_WARP_STONE))
	{
	char_puts("You lack the proper component for this spell.\n", sspell->ch);
		return FALSE;
	}

	if (stone != NULL && stone->pIndexData->item_type == ITEM_WARP_STONE)
	{
	 	act("You draw upon the power of $p.", sspell->ch,stone,NULL,TO_CHAR);
	 	act("It flares brightly and vanishes!", sspell->ch,stone,NULL,TO_CHAR);
	 	extract_obj(stone, 0);
	}
	*/

	portal = create_obj(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 2 + sspell->level / 25; 
	portal->value[3] = victim->in_room->vnum;

	obj_to_room(portal, sspell->ch->in_room);

	act("$p rises up from the ground.", sspell->ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.", sspell->ch,portal,NULL,TO_CHAR);
	return TRUE;
}

bool spell_nexus(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	OBJ_DATA *portal/*, *stone*/;
	ROOM_INDEX_DATA *to_room, *from_room;

	from_room = sspell->ch->in_room;
 
	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->level >= sspell->level + 3
	||  !can_see_room(sspell->ch, from_room)
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_NONE, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}   
 
	to_room = victim->in_room;
	
	/*
	stone = get_eq_char(sspell->ch,WEAR_HOLD);
	if (!IS_IMMORTAL(sspell->ch)
	&&  (stone == NULL || stone->pIndexData->item_type != ITEM_WARP_STONE)) {
		char_puts("You lack the proper component for this spell.\n", sspell->ch);
		return FALSE;
	}
 
	if (stone != NULL && stone->pIndexData->item_type == ITEM_WARP_STONE) {
		act("You draw upon the power of $p.", sspell->ch,stone,NULL,TO_CHAR);
		act("It flares brightly and vanishes!", sspell->ch,stone,NULL,TO_CHAR);
		extract_obj(stone, 0);
	}
	*/

	/* portal one */ 
	portal = create_obj(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 1 + sspell->level / 10;
	portal->value[3] = to_room->vnum;
 
	obj_to_room(portal,from_room);
 
	act("$p rises up from the ground.", sspell->ch,portal,NULL,TO_ROOM);
	act("$p rises up before you.", sspell->ch,portal,NULL,TO_CHAR);

	/* no second portal if rooms are the same */
	if (to_room == from_room)
		return FALSE;

	/* portal two */
	portal = create_obj(get_obj_index(OBJ_VNUM_PORTAL),0);
	portal->timer = 1 + sspell->level/10;
	portal->value[3] = from_room->vnum;

	obj_to_room(portal,to_room);

	if (to_room->people != NULL) {
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_ROOM);
		act("$p rises up from the ground.",to_room->people,portal,NULL,TO_CHAR);
	}
	return TRUE;
}

bool spell_disintegrate(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	CHAR_DATA *tmp_ch;
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	int i,dam=0;
	OBJ_DATA *tattoo, *clanmark; 
	AFFECT_DATA af;
	
	if (saves_spell(sspell->ch, sspell->level - 2, victim, DAM_ENERGY, lookup_spell_msc(sspell->sn))
	||  IS_IMMORTAL(victim)
	/*||  IS_CLAN_GUARD(victim)*/) {
		dam = dice( sspell->level, 24) ;
		damage(sspell->ch, victim, dam, sspell->sn, DAM_ENERGY, TRUE);
		
		if (!JUST_KILLED(sspell->ch))	// example ch == victim
		{
			if (number_bits(1) && number_percent() < sspell->percent)
				return TRUE;
			dam = dice( sspell->level, 10);
			if (saves_spell_simply(sspell->level - 5, sspell->ch, DAM_ENERGY))
				dam /= 2;
			char_puts("Cast so much spell you feel pain's wave through your's body.\n", sspell->ch);
			damage(sspell->ch, sspell->ch, dam, sn_lookup("cause critical"), DAM_ENERGY, TRUE);
		}
		return TRUE;
	}

	act_puts("$N's thin light ray {Y### {RDISINTEGRATES{Y ###{x you!", 
	      victim, NULL, sspell->ch, TO_CHAR, POS_RESTING);
	act_puts("$n's thin light ray {Y### {RDISINTEGRATES{Y ###{x $N!", 
	      sspell->ch, NULL, victim, TO_NOTVICT, POS_RESTING);
	act_puts("Your thin light ray {Y### {RDISINTEGRATES{Y ###{x $N!", 
	      sspell->ch, NULL, victim, TO_CHAR, POS_RESTING);
	char_puts("You have been KILLED!!\n\n", victim);

	act("$N does not exist anymore!\n", sspell->ch, NULL, victim, TO_CHAR);
	act("$N does not exist anymore!\n", sspell->ch, NULL, victim, TO_ROOM);

	char_puts("You turn into an invincible ghost for a few minutes.\n",
	             victim);
	char_puts("As long as you don't attack anything.\n", victim);

	quest_handle_death(sspell->ch, victim);

	/*  disintegrate the objects... */
	tattoo = get_eq_char(victim, WEAR_TATTOO); /* keep tattoos for later */
	if (tattoo != NULL)
		obj_from_char(tattoo);
	if ((clanmark = get_eq_char(victim, WEAR_CLANMARK)) != NULL) 
		obj_from_char(clanmark);

	victim->gold = 0;
	victim->silver = 0;

	for (obj = victim->carrying; obj != NULL; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, 0);
	}

	/* Otdacha */
	if (!JUST_KILLED(sspell->ch))	// example ch == victim
	{
		dam = dice( sspell->level, 12);
		if (saves_spell_simply(sspell->level - 5, sspell->ch, DAM_ENERGY))
			dam /= 2;
		char_puts("Cast so much spell you feel pain's wave through your's body.\n", sspell->ch);
		damage(sspell->ch, sspell->ch, dam, sn_lookup("cause critical"), DAM_ENERGY, TRUE);
	
		if ((number_bits(1) || number_percent() > sspell->percent)
			&& !saves_spell_simply(UMAX(1, sspell->level - 10), sspell->ch, DAM_ENERGY))
		{
			char_puts("Power of spell shock you.\n", sspell->ch);
			af.type         = gsn_disintegrate;
			af.where        = TO_AFFECTS;
			af.level        = sspell->level / 2;
			af.duration     = sspell->level / 30 + 1;
			af.location     = APPLY_NONE;
			af.modifier     = 0;
			af.bitvector    = AFF_SLEEP;
			affect_join (sspell->ch, &af);
		
			sspell->ch->position = POS_SLEEPING;
		}
	}

	if (IS_NPC(victim)) {
		victim->pIndexData->killed++;
		extract_char(victim, 0);
		return TRUE;
	}
	
	rating_update(sspell->ch, victim);
	extract_char(victim, XC_F_INCOMPLETE);

	while (victim->affected)
		affect_remove(victim, victim->affected);
	victim->affected_by   = 0;
	for (i = 0; i < 4; i++)
		victim->armor[i]= -100;
	victim->position      = POS_RESTING;
	victim->hit           = 1;
	victim->mana  	  = 1;

	REMOVE_BIT(victim->pcdata->plr_flags, PLR_WANTED | PLR_BOUGHT_PET);

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

	if (clanmark != NULL) {
		obj_to_char(clanmark, victim);
		equip_char(victim, clanmark, WEAR_CLANMARK);
	}

	for (tmp_ch = npc_list; tmp_ch; tmp_ch = tmp_ch->next)
		if (tmp_ch->last_fought == victim)
			tmp_ch->last_fought = NULL;
	return TRUE;
}

bool spell_bark_skin(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn))
	{
	if (victim == sspell->ch)
	  char_puts("Your skin is already covered in bark.\n", sspell->ch); 
	else
	  act("$N is already as hard as can be.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}
	af.where	 = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level;
	af.location  = APPLY_AC;
	af.modifier  = sspell->level * 2;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin becomes covered in bark.", victim, NULL, NULL, TO_ROOM);
	char_puts("Your skin becomes covered in bark.\n", victim);
	return TRUE;
}

bool spell_ranger_staff(const spell_spool_t *sspell)
{
	OBJ_DATA *staff;
	AFFECT_DATA tohit;
	AFFECT_DATA todam;

	staff = create_obj(get_obj_index(OBJ_VNUM_RANGER_STAFF), 0);
	staff->level = sspell->ch->level;
	staff->value[1] = 4 + sspell->level / 15;
	staff->value[2] = 4 + sspell->level / 15;

	char_puts("You create a ranger's staff!\n", sspell->ch);
	act("$n creates a ranger's staff!", sspell->ch,NULL,NULL,TO_ROOM);

	tohit.where		   = TO_OBJECT;
	tohit.type               = sspell->sn;
	tohit.level              = sspell->ch->level; 
	tohit.duration           = -1;
	tohit.location           = APPLY_HITROLL;
	tohit.modifier           = 2 + sspell->level/5;
	tohit.bitvector          = 0;
	affect_to_obj(staff,&tohit);

	todam.where		   = TO_OBJECT;
	todam.type               = sspell->sn;
	todam.level              = sspell->ch->level; 
	todam.duration           = -1;
	todam.location           = APPLY_DAMROLL;
	todam.modifier           = 2 + sspell->level/5;
	todam.bitvector          = 0;
	affect_to_obj(staff,&todam);


	staff->timer = sspell->level;
	staff->level = sspell->ch->level;
	
	obj_to_char(staff, sspell->ch);
	return TRUE;
}

bool spell_transform(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn) || sspell->ch->hit > sspell->ch->max_hit)
	{
	  char_puts("You are already overflowing with health.\n", sspell->ch);
	  return FALSE;
	}

	sspell->ch->hit += UMIN(30000 - sspell->ch->max_hit, sspell->ch->max_hit);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24;
	af.location           = APPLY_HIT;
	af.modifier           = UMIN(30000 - sspell->ch->max_hit, sspell->ch->max_hit);
	af.bitvector          = 0;
	affect_to_char(sspell->ch,&af);


	char_puts("Your mind clouds as your health increases.\n", sspell->ch);
	return TRUE;
}

bool spell_mental_knife(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	if (sspell->ch->level < 40)
	dam = dice( sspell->level,8);
	else if (sspell->ch->level < 65)
	dam = dice( sspell->level,11);
	else dam = dice( sspell->level,14);

	if (saves_spell(sspell->ch, sspell->level,victim, DAM_MENTAL, lookup_spell_msc(sspell->sn)))
	      dam /= 2;
	damage(sspell->ch,victim,dam, sspell->sn,DAM_MENTAL, TRUE);
	if (JUST_KILLED(victim))
		return TRUE;

	if (!is_affected(victim, sspell->sn) && !saves_spell(sspell->ch, sspell->level, victim, DAM_MENTAL, lookup_spell_msc(sspell->sn)))
	{
	  af.where		    = TO_AFFECTS;
	  af.type               = sspell->sn;
	  af.level              = sspell->level; 
	  af.duration           = sspell->level;
	  af.location           = APPLY_INT;
	  af.modifier           = -7;
	  af.bitvector          = 0;
	  affect_to_char(victim,&af);

	  af.location = APPLY_WIS;
	  affect_to_char(victim,&af);
	  act("Your mental knife sears $N's mind!", sspell->ch,NULL,victim,TO_CHAR);
	  act("$n's mental knife sears your mind!", sspell->ch,NULL,victim,TO_VICT);
	  act("$n's mental knife sears $N's mind!", sspell->ch,NULL,victim,TO_NOTVICT);
	}
	return TRUE;
}

bool spell_demon_summon(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *demon;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You lack the power to summon another demon right now.\n",
		   sspell->ch);
	  return FALSE;
	}

	char_puts("You attempt to summon a demon.\n", sspell->ch);
	act("$n attempts to summon a demon.", sspell->ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  gch->pIndexData->vnum == MOB_VNUM_DEMON) {
			char_puts("Two demons are more than you can control!\n",
				  sspell->ch);
			return FALSE;
		}
	}

	demon = create_mob(get_mob_index(MOB_VNUM_DEMON));

	for (i=0;i < MAX_STATS; i++)
	{
	  demon->perm_stat[i] = sspell->ch->perm_stat[i];
	}

	demon->max_hit = IS_NPC(sspell->ch)? URANGE(sspell->ch->max_hit,1 * sspell->ch->max_hit,30000)
		: URANGE(sspell->ch->pcdata->perm_hit, sspell->ch->hit,30000);
	demon->hit = demon->max_hit;
	demon->max_mana = IS_NPC(sspell->ch)? sspell->ch->max_mana : sspell->ch->pcdata->perm_mana;
	demon->mana = demon->max_mana;
	demon->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		demon->armor[i] = interpolate(demon->level,-100,100);
	demon->armor[3] = interpolate(demon->level,-100,0);
	demon->gold = 0;
	demon->timer = 0;
	demon->damage[DICE_NUMBER] = number_range(sspell->level /15, sspell->level/10);   
	demon->damage[DICE_TYPE] = number_range(sspell->level /3, sspell->level/2);
	demon->damage[DICE_BONUS] = number_range(sspell->level /8, sspell->level/6);

	char_puts("A demon arrives from the underworld!\n", sspell->ch);
	act("A demon arrives from the underworld!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	char_to_room(demon, sspell->ch->in_room);
	if (JUST_KILLED(demon))
		return TRUE;

	if (number_percent() < 40)
	{
	  if (can_see(demon, sspell->ch))
	    do_say(demon, "You dare disturb me?!");
	  else
	    do_say(demon, "Who dares disturb me?!");
	  do_murder(demon, sspell->ch->name);
	}
	else {
	SET_BIT(demon->affected_by, AFF_CHARM);
	demon->master = demon->leader = sspell->ch;
	}
	return TRUE;
}

bool spell_scourge(const spell_spool_t *sspell)	
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;

	if (sspell->ch->level < 40)
		dam = dice( sspell->level, 6);
	else if (sspell->ch->level < 65)
		dam = dice( sspell->level, 9);
	else
		dam = dice( sspell->level, 12);

	for (vch = sspell->ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch, vch, TRUE)
		||  is_affected(vch, sspell->sn))	/* XXX ??? */
			continue;

	    if (number_percent() < sspell->level)
	      make_spell(spell_poison, gsn_poison, sspell->level, sspell->ch, vch, TARGET_CHAR, sspell->percent);

	    if (number_percent() < sspell->level)
	      make_spell(spell_blindness, gsn_blindness, sspell->level, sspell->ch,vch, TARGET_CHAR, sspell->percent);

	    if (number_percent() < sspell->level)
	      make_spell(spell_weaken, gsn_weaken, sspell->level, sspell->ch, vch, TARGET_CHAR, sspell->percent);

	        if (saves_spell(sspell->ch, sspell->level,vch, DAM_FIRE, lookup_spell_msc(sspell->sn)))
	      dam /= 2;
	    damage(sspell->ch, vch, dam, sspell->sn, DAM_FIRE, TRUE);
	}
	return TRUE;
}

bool spell_benediction(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	int strength = 0;
	if (is_affected(victim, sspell->sn)) {
		if (victim == sspell->ch) 
			act("You are already blessed.", 
				sspell->ch, NULL, NULL, TO_CHAR);
		else 
			act("$N is already blessed.", 
				sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}
	if (IS_EVIL(victim)) 
		strength = IS_EVIL(sspell->ch) ? 2 : (IS_GOOD(sspell->ch) ? 0 : 1);
	if (IS_GOOD(victim))
		strength = IS_GOOD(sspell->ch) ? 2 : (IS_EVIL(sspell->ch) ? 0 : 1);
	if (IS_NEUTRAL(victim)) 
		strength = IS_NEUTRAL(sspell->ch) ? 2 : 1;
	if (!strength) {
		act("Your god does not seems to like $N", 
			sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}
	af.where 	= TO_AFFECTS;
	af.type  	= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 5 + sspell->level / 2;
	af.location	= APPLY_HITROLL;
	af.modifier	= sspell->level / 8 * strength;
	af.bitvector	= 0;

	affect_to_char(victim, &af);

	af.location	= APPLY_SAVES;
	af.modifier	= sspell->level / 8 * strength;

	affect_to_char(victim, &af);

	af.location	= APPLY_LEVEL;
	af.modifier	= strength;

	affect_to_char(victim, &af);
	act("You feel righteous.\n", victim, NULL, NULL, TO_CHAR);
	if (victim != sspell->ch) {
		act("You grant $N favor of your god.", 
			sspell->ch, NULL, victim, TO_CHAR);
		change_faith(sspell->ch, RELF_BENEDICTION_OTHER, 0);
	}
	return TRUE;
}


bool spell_manacles(const spell_spool_t *sspell)	
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (IS_NPC(victim) || !IS_SET(victim->pcdata->plr_flags, PLR_WANTED)) {
		act("But $N is not wanted.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (is_affected(victim, sspell->sn))
		return FALSE;

	if (!saves_spell(sspell->ch, sspell->ch->level, victim,DAM_CHARM, lookup_spell_msc(sspell->sn)))
	{
	  AFFECT_DATA af;

	  af.where		    = TO_AFFECTS;
	  af.type               = sspell->sn;
	  af.level              = sspell->level; 
	  af.duration           = 5 + sspell->level/5;
	  af.bitvector          = 0;

	  af.modifier           = 0 - (get_curr_stat(victim, STAT_DEX)-4);
	  af.location           = APPLY_DEX;
	  affect_to_char(victim, &af);

	  af.modifier           = -5;
	  af.location           = APPLY_HITROLL;
	  affect_to_char(victim, &af);

	
	  af.modifier           = -10;
	  af.location           = APPLY_DAMROLL;
	  affect_to_char(victim, &af);

	  make_spell(spell_charm_person, gsn_charm_person, sspell->level, sspell->ch, sspell->vo, TARGET_CHAR, sspell->percent);
	}
	return TRUE;
}

bool spell_shield_of_ruler(const spell_spool_t *sspell) 
{
	int shield_vnum;
	OBJ_DATA *shield;
	AFFECT_DATA af;

	if (sspell->level >= 71)
		shield_vnum = OBJ_VNUM_RULER_SHIELD4;
	else if (sspell->level >= 51)
		shield_vnum = OBJ_VNUM_RULER_SHIELD3;
	else if (sspell->level >= 31)
		shield_vnum = OBJ_VNUM_RULER_SHIELD2;
	else
		shield_vnum = OBJ_VNUM_RULER_SHIELD1;

	shield = create_obj(get_obj_index(shield_vnum), 0);
	shield->level = sspell->ch->level;
	shield->timer = sspell->level;
	shield->cost  = 0;
	obj_to_char(shield, sspell->ch);
  
	af.where	= TO_OBJECT;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= -1;
	af.modifier	= sspell->level / 8;
	af.bitvector	= 0;

	af.location	= APPLY_HITROLL;
	affect_to_obj(shield, &af);

	af.location	= APPLY_DAMROLL;
	affect_to_obj(shield, &af);

	af.modifier	= sspell->level/2;
	af.location	= APPLY_AC;
	affect_to_obj(shield, &af);

	af.modifier	= UMAX(1, sspell->level / 30);
	af.location	= APPLY_CHA;
	affect_to_obj(shield, &af);

	act("You create $p!", sspell->ch, shield, NULL, TO_CHAR);
	act("$n creates $p!", sspell->ch, shield, NULL, TO_ROOM);
	return TRUE;
}
 
bool spell_guard_call(const spell_spool_t *sspell)
{
	CHAR_DATA *gch;
	CHAR_DATA *guard;
	CHAR_DATA *guard2;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You lack the power to call another two guards now.\n", sspell->ch);
	  return FALSE;
	}

	do_yell(sspell->ch, "Guards! Guards!");

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  gch->pIndexData->vnum == MOB_VNUM_SPECIAL_GUARD) {
			do_say(gch, "What? I'm not good enough?");
			return FALSE;
		}
	}

	guard = create_mob(get_mob_index(MOB_VNUM_SPECIAL_GUARD));

	for (i=0;i < MAX_STATS; i++)
	guard->perm_stat[i] = sspell->ch->perm_stat[i];

	guard->max_hit = 2 * sspell->ch->max_hit;
	guard->hit = guard->max_hit;
	guard->max_mana = sspell->ch->max_mana;
	guard->mana = guard->max_mana;
	guard->alignment = sspell->ch->alignment;
	guard->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		guard->armor[i] = interpolate(guard->level,-100,200);
	guard->armor[3] = interpolate(guard->level,-100,100);
	guard->sex = sspell->ch->sex;
	guard->gold = 0;
	guard->timer = 0;

	guard->damage[DICE_NUMBER] = number_range(sspell->level /18, sspell->level/14);   
	guard->damage[DICE_TYPE] = number_range(sspell->level /4, sspell->level/3);
	guard->damage[DICE_BONUS] = number_range(sspell->level /10, sspell->level/8);

	guard2 = create_mob(guard->pIndexData);
	clone_mob(guard,guard2);
	
	SET_BIT(guard->affected_by, AFF_CHARM);
	SET_BIT(guard2->affected_by, AFF_CHARM);
	guard->master = guard2->master = sspell->ch;
	guard->leader = guard2->leader = sspell->ch;

	char_puts("Two guards come to your rescue!\n", sspell->ch);
	act("Two guards come to $n's rescue!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 12;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	char_to_room(guard, sspell->ch->in_room);
	char_to_room(guard2, sspell->ch->in_room);
	return TRUE;
}

bool spell_nightwalker(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *walker;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn)) {
		act_puts("You feel too weak to summon a Nightwalker now.",
			 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	act_puts("You attempt to summon a Nightwalker.",
		 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);
	act("$n attempts to summon a Nightwalker.", sspell->ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  gch->pIndexData->vnum == MOB_VNUM_NIGHTWALKER) {
			act_puts("Two Nightwalkers are more than "
				 "you can control!",
				 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return FALSE;
		}
	}

	walker = create_mob(get_mob_index(MOB_VNUM_NIGHTWALKER));

	for (i = 0; i < MAX_STATS; i++)
		walker->perm_stat[i] = sspell->ch->perm_stat[i];

	walker->max_hit = IS_NPC(sspell->ch) ? sspell->ch->max_hit : sspell->ch->pcdata->perm_hit;
	walker->hit = walker->max_hit;
	walker->max_mana = sspell->ch->max_mana;
	walker->mana = walker->max_mana;
	walker->level = sspell->ch->level;
	for (i = 0; i < 3; i++)
		walker->armor[i] = interpolate(walker->level, -100, 100);
	walker->armor[3] = interpolate(walker->level, -100, 0);
	walker->gold = 0;
	walker->timer = 0;
	walker->damage[DICE_NUMBER] = number_range(sspell->level /15, sspell->level/10);   
	walker->damage[DICE_TYPE]   = number_range(sspell->level /3, sspell->level/2);
	walker->damage[DICE_BONUS]  = 0;
	 
	act_puts("$N rises from the shadows!",
		 sspell->ch, NULL, walker, TO_CHAR, POS_DEAD);
	act("$N rises from the shadows!", sspell->ch, NULL, walker, TO_ROOM);
	act_puts("$N kneels before you.",
		 sspell->ch, NULL, walker, TO_CHAR, POS_DEAD);
	act("$N kneels before $n!", sspell->ch, NULL, walker, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level; 
	af.duration	= 24;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	walker->master = walker->leader = sspell->ch;

	char_to_room(walker, sspell->ch->in_room);
	return TRUE;
}
	
bool spell_eyes_of_intrigue(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;

	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL)
	  {
	char_puts("Your spy network reveals no such player.\n", sspell->ch);
	return FALSE;
	  }

	if ((victim->level > sspell->ch->level + 7) && saves_spell(sspell->ch, sspell->level, victim, DAM_NONE, lookup_spell_msc(sspell->sn)))
	  {
	char_puts("Your spy network cannot find that player.\n", sspell->ch);
	return TRUE;
	  }

	if (sspell->ch->in_room == victim->in_room)
		do_look(sspell->ch, str_empty);
	else
		look_at(sspell->ch, victim->in_room);
	return TRUE;
}

bool spell_shadow_cloak(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	/*
	if (sspell->ch->clan != victim->clan)
	  {
	char_puts("You may only use this spell on fellow clan members.\n", sspell->ch);
	return FALSE;
	  }
	*/

	if (is_affected(victim, sspell->sn))
	{
	if (victim == sspell->ch)
	  char_puts("You are already protected by a shadow cloak.\n", sspell->ch);
	else
	  act("$N is already protected by a shadow cloak.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	af.where	 = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = 24;
	af.modifier  = sspell->level;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel the shadows protect you.\n", victim);
	if (sspell->ch != victim)
	act("A cloak of shadows protect $N.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}
	
bool spell_nightfall(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	OBJ_DATA  *obj;
	AFFECT_DATA af;
	
	if (is_affected(sspell->ch, sspell->sn)) {
		char_puts("You can't find the power to control objs.\n",
			  sspell->ch);
		return FALSE;
	}

	for (vch = sspell->ch->in_room->people; vch; vch = vch->next_in_room) {
		if (is_same_group(sspell->ch, vch))
			continue;

		for (obj = vch->carrying; obj; obj = obj->next_content) {
			if (obj->pIndexData->item_type != ITEM_LIGHT
			||  obj->value[2] == 0
			||  saves_spell(sspell->ch, sspell->level, vch, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
				continue;

			act("$p flickers and goes out!", sspell->ch, obj, NULL, TO_ALL);
			obj->value[2] = 0; 

			if (obj->wear_loc == WEAR_LIGHT
			&&  sspell->ch->in_room->light > 0)
				sspell->ch->in_room->light--;
		}
	}

	for (obj = sspell->ch->in_room->contents; obj; obj = obj->next_content) {
		if (obj->pIndexData->item_type != ITEM_LIGHT
		||  obj->value[2] == 0)
			continue;

		act("$p flickers and goes out!", sspell->ch, obj, NULL, TO_ALL);
		obj->value[2] = 0; 
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 2;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	af.bitvector	= 0;
	affect_to_char(sspell->ch, &af);
	return TRUE;
}
	      
bool spell_garble(const spell_spool_t *sspell)	
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (sspell->ch == victim) 
	{
	char_puts("Garble whose speech?\n", sspell->ch);
	return FALSE;
	}

	if (is_affected(victim, sspell->sn)) 
	{
	act("$N's speech is already garbled.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	if (is_safe_nomessage(sspell->ch,victim)) {
	  char_puts("You cannot garble that person.\n", sspell->ch);
	  return FALSE;
	} 

	if (saves_spell(sspell->ch, sspell->level,victim, DAM_MENTAL, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where	= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 10;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(victim,&af);

	act("You have garbled $N's speech!", sspell->ch,NULL,victim,TO_CHAR);
	char_puts("You feel your tongue contort.\n",victim);
	return TRUE;
}

bool spell_confuse(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	CHAR_DATA *rch;
	int count=0;

	if (is_safe(sspell->ch,victim))
		return FALSE;

	if (is_affected(victim,gsn_confuse)) {
		act("$N is already thoroughly confused.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level,victim, DAM_MENTAL, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 10;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(victim,&af);

	for (rch = sspell->ch->in_room->people; rch; rch = rch->next_in_room)
		if (rch == sspell->ch
		&&  !can_see(sspell->ch, rch))
			count++;

	for (rch = sspell->ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch != sspell->ch
		&&  can_see(sspell->ch, rch)
		&&  number_range(1, count) == 1)
			break;
	}

	if (rch)
		do_murder(victim, rch->name);
	do_murder(victim, sspell->ch->name);
	return TRUE;
}

bool spell_terangreal(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	
	if (IS_NPC(victim))
		return FALSE;

	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SLEEP;
	affect_join(victim, &af);

	if (IS_AWAKE(victim))
	{
		char_puts("You are overcome by a sudden surge of fatigue.\n",
		     victim);
		act("$n falls into a deep sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}

	return TRUE;
}
	
bool spell_kassandra(const spell_spool_t *sspell)
{

	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn))
	  {
	char_puts
	  ("The kassandra has been used for this purpose too recently.\n",
	   sspell->ch);
	return FALSE;
	  }
	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 5;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(sspell->ch, &af);
	sspell->ch->hit = UMIN(sspell->ch->hit + 150, sspell->ch->max_hit);
	update_pos(sspell->ch);
	char_puts("A warm feeling fills your body.\n", sspell->ch);
	act("$n looks better.", sspell->ch, NULL, NULL, TO_ROOM);
	return TRUE;
}  
	
	
bool spell_sebat(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("The kassandra has been used for that too recently.\n"
		   , sspell->ch); 
	  return FALSE;
	}
	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level;
	af.location  = APPLY_AC;
	af.modifier  = 30;
	af.bitvector = 0;
	affect_to_char(sspell->ch, &af);
	act("$n is surrounded by a mystical shield.", sspell->ch, NULL,NULL,TO_ROOM);
	char_puts("You are surrounded by a mystical shield.\n", sspell->ch);
	return TRUE;
}
	

bool spell_matandra(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;
	AFFECT_DATA af;
	
	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts
	("The kassandra has been used for this purpose too recently.\n",
	 sspell->ch);
	  return FALSE;
	}
	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 5;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(sspell->ch, &af);
	dam = dice( sspell->level, 7);
	
	damage(sspell->ch,victim,dam, sspell->sn,DAM_HOLY, TRUE);
	return TRUE;
}  
	
bool spell_amnesia(const spell_spool_t *sspell)  
{
	int i;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (IS_NPC(victim))
		return FALSE;

	for (i = 0; i < victim->pcdata->learned.nused; i++) {
		pcskill_t *ps = VARR_GET(&victim->pcdata->learned, i);
		ps->percent /= 2;
	}

	act("You feel your memories slip away.",victim,NULL,NULL,TO_CHAR);
	act("$n gets a blank look on $s face.",victim,NULL,NULL,TO_ROOM);
	return TRUE;
}
	

bool spell_chaos_blade(const spell_spool_t *sspell)
{
	OBJ_DATA *blade;
	AFFECT_DATA af;
	
	blade = create_obj(get_obj_index(OBJ_VNUM_CHAOS_BLADE), 0);
	blade->level = sspell->ch->level;
	blade->timer = sspell->level * 2;
	blade->value[2] = (sspell->ch->level / 10) + 3;  

	char_puts("You create a blade of chaos!\n", sspell->ch);
	act("$n creates a blade of chaos!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = sspell->level / 6;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(blade,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(blade,&af);

	obj_to_char(blade, sspell->ch);
	return TRUE;
}    

bool spell_wrath(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;
	AFFECT_DATA af;
	
	if (!IS_NPC(sspell->ch) && IS_EVIL(sspell->ch))
		victim = sspell->ch;
	
	if (IS_GOOD(victim)) {
		act("The gods protect $N.", sspell->ch, NULL, victim, TO_ROOM);
		return FALSE;
	}
	
	if (IS_NEUTRAL(victim)) {
		act("$N does not seem to be affected.",
		    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	dam = dice( sspell->level, 12);

	if (saves_spell(sspell->ch, sspell->level, victim, DAM_HOLY, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_HOLY, TRUE);
	if (JUST_KILLED(victim))
		return TRUE;

	if (IS_AFFECTED(victim, AFF_CURSE)
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_HOLY, lookup_spell_msc(sspell->sn)))
		return TRUE;
	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 2 * sspell->level;
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * (sspell->level / 8);
	af.bitvector = AFF_CURSE;
	affect_to_char(victim, &af);
	
	af.location  = APPLY_SAVES;
	af.modifier  = -1 * sspell->level / 8;
	affect_to_char(victim, &af);
	
	char_puts("You feel unclean.\n", victim);
	if (sspell->ch != victim)
		act("$N looks very uncomfortable.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_stalker(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	CHAR_DATA *stalker;
	AFFECT_DATA af;
	int i;

	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	  ||   victim == sspell->ch || victim->in_room == NULL
	  || IS_NPC(victim) || !IS_SET(victim->pcdata->plr_flags, PLR_WANTED)) {
	  char_puts("You failed.\n", sspell->ch);
	  return FALSE;
	}

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("This power is used too recently.\n", sspell->ch);
	  return FALSE;
	}

	char_puts("You attempt to summon a stalker.\n", sspell->ch);
	act("$n attempts to summon a stalker.", sspell->ch,NULL,NULL,TO_ROOM);

	stalker = create_mob(get_mob_index(MOB_VNUM_STALKER));

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 6;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	for (i=0;i < MAX_STATS; i++)
	{
	  stalker->perm_stat[i] = victim->perm_stat[i];
	}

	stalker->max_hit = UMIN(30000,2 * victim->max_hit);
	stalker->hit = stalker->max_hit;
	stalker->max_mana = victim->max_mana;
	stalker->mana = stalker->max_mana;
	stalker->level = victim->level;

	stalker->damage[DICE_NUMBER] = 
		number_range(victim->level/18, victim->level/14);   
	stalker->damage[DICE_TYPE] = 
		number_range(victim->level/4, victim->level/3);
	stalker->damage[DICE_BONUS] = 
		number_range(victim->level/10, victim->level/8);

	for (i=0; i < 3; i++)
		stalker->armor[i] = interpolate(stalker->level,-100,100);
	stalker->armor[3] = interpolate(stalker->level,-100,0);
	stalker->gold = 0;
	stalker->affected_by |= (AFF_DETECT_IMP_INVIS | AFF_DETECT_FADE | AFF_DETECT_EVIL |
				 AFF_DETECT_INVIS | AFF_DETECT_MAGIC | AFF_DETECT_HIDDEN |
				 AFF_DETECT_GOOD | /* AFF_DARK_VISION |*/ AFF_HIDE | AFF_IMP_INVIS);
	
	stalker->target = victim;
	//stalker->clan   = sspell->ch->clan;
	char_puts("An invisible stalker arrives to stalk you!\n",victim);
	act("An invisible stalker arrives to stalk $n!",victim,NULL,NULL,TO_ROOM);
	char_puts("An invisible stalker has been sent.\n", sspell->ch);

	char_to_room(stalker,victim->in_room);
	return TRUE;
}

static inline void
tesseract_other(CHAR_DATA *ch, CHAR_DATA *victim, ROOM_INDEX_DATA *to_room)
{
	transfer_char(victim, ch, to_room,
		      NULL,
		      "$n utters some strange words and, "
		      "with a sickening lurch, you feel time\n"
		      "and space shift around you.",
		      "$N arrives suddenly.");
}
				
bool spell_tesseract(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	CHAR_DATA *wch;
	CHAR_DATA *wch_next;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(sspell->ch,sspell->arg)) == NULL
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}
	
	if (sspell->ch->pet != NULL && sspell->ch->in_room == sspell->ch->pet->in_room)
		pet = sspell->ch->pet;

	for (wch = sspell->ch->in_room->people; wch; wch = wch_next) {
		wch_next = wch->next_in_room;
		if (wch != sspell->ch && wch != pet && is_same_group(wch, sspell->ch))
			tesseract_other(sspell->ch, wch, victim->in_room);
	}

	act("With a sudden flash of light, $n and $s friends disappear!",
		sspell->ch, NULL, NULL, TO_ROOM);
	transfer_char(sspell->ch, NULL, victim->in_room,
		      NULL,
		      "As you utter the words, time and space seem to blur.\n."
		      "You feel as though space and time are shifting.\n"
		      "all around you while you remain motionless.",
		      "$N arrives suddenly.");
	
	if (pet) 
		tesseract_other(sspell->ch, pet, victim->in_room);
	return TRUE;
}

bool spell_brew(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	OBJ_DATA *potion;
	OBJ_DATA *vial;
	int spell;

	if (obj->pIndexData->item_type != ITEM_TRASH && obj->pIndexData->item_type != ITEM_TREASURE
	&& obj->pIndexData->item_type != ITEM_KEY)
	  {
	char_puts("That can't be transformed into a potion.\n", sspell->ch);
	return FALSE;
	  }
	
	if (obj->wear_loc != -1)
	  {
	char_puts("The item must be carried to be brewed.\n", sspell->ch);
	return FALSE;
	  }

	for(vial = sspell->ch->carrying; vial != NULL; vial=vial->next_content)
	  if (vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL)
	    break;
	if ( vial == NULL)  {
	char_puts("You don't have any vials to brew the potion into.\n"
			, sspell->ch);
	return FALSE;
	}
 

	if (number_percent() < 50)
	  { 
	char_puts("You failed and destroyed it.\n", sspell->ch);
	extract_obj(obj, 0);
	return TRUE;
	  }
	
	if (obj->pIndexData->item_type == ITEM_TRASH)
		potion = create_obj(get_obj_index(OBJ_VNUM_POTION_SILVER), 0);
	else if (obj->pIndexData->item_type == ITEM_TREASURE)
		potion = create_obj(get_obj_index(OBJ_VNUM_POTION_GOLDEN), 0);
	else
		potion = create_obj(get_obj_index(OBJ_VNUM_POTION_SWIRLING), 0);
	potion->level = sspell->ch->level;
	potion->value[0] = sspell->level;

	spell = 0;

	switch (obj->pIndexData->item_type) {
	 case ITEM_TRASH:
		switch(number_bits(3)) {
		case 0:
	  		spell = sn_lookup("fireball");
			break;
		case 1:
	  		spell = sn_lookup("cure poison");
			break;
		case 2:
	  		spell = sn_lookup("cure blind");
			break;
		case 3:
	  		spell = sn_lookup("cure disease");
			break;
		case 4:
	  		spell = sn_lookup("word of recall");
			break;
		case 5: 
			spell = sn_lookup("protection good");
			break;
		case 6:
			spell = sn_lookup("protection evil");
			break;
		case 7: 
			spell = sn_lookup("sanctuary");
			break;
		};
		break;
	case ITEM_TREASURE:
		switch(number_bits(3)) {
		case 0:
	  		spell = sn_lookup("cure critical");
	  		break;
		case 1:
	  		spell = sn_lookup("haste");
	  		break;
		case 2:
	  		spell = gsn_frenzy;
	  		break;
		case 3:
	  		spell = sn_lookup("create spring");
	  		break;
		case 4:
	  		spell = sn_lookup("holy word");
	  		break;
		case 5:
	  		spell = sn_lookup("invis");
	  		break;
		case 6:
	  		spell = sn_lookup("cure light");
	  		break;
		case 7:
	  		spell = sn_lookup("cure serious");
	  		break;
		};
		break;
	case ITEM_KEY:
	switch (number_bits(3)) {
		case 0:
	  		spell = sn_lookup("detect magic");
	 		break;
		case 1:
	  		spell = sn_lookup("detect invis");
	  		break;
		case 2:
	  		spell = sn_lookup("pass door");
	  		break;
		case 3:
	  		spell = sn_lookup("detect hidden");
	  		break;
		case 4:
	  		spell = sn_lookup("improved detect");
	  		break;
		case 5:
	  		spell = sn_lookup("acute vision");
	  		break;
		case 6:
	  		spell = sn_lookup("detect good");
	  		break;
		case 7:
	  		spell = sn_lookup("detect evil");
			break;
	  };
	};
	potion->value[1] = spell;
	extract_obj(obj, 0);
	act("You brew $p from your resources!", sspell->ch, potion, NULL, TO_CHAR);
	act("$n brews $p from $s resources!", sspell->ch, potion, NULL, TO_ROOM);

	obj_to_char(potion, sspell->ch);
	extract_obj(vial, 0);
	return TRUE;
}


bool spell_shadowlife(const spell_spool_t *sspell)	
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	CHAR_DATA *shadow;
	AFFECT_DATA af;
	int i;

	if (IS_NPC(victim))
	{
	  char_puts("Now, why would you want to do that?!?\n", sspell->ch);
	  return FALSE;
	}

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You don't have the strength to raise a Shadow now.\n",
		   sspell->ch);
	  return FALSE;
	}

	act("You give life to $N's shadow!", sspell->ch, NULL, victim, TO_CHAR);
	act("$n gives life to $N's shadow!", sspell->ch, NULL, victim, TO_NOTVICT);
	act("$n gives life to your shadow!", sspell->ch, NULL, victim, TO_VICT);
	
	shadow = create_mob_of(get_mob_index(MOB_VNUM_SHADOW),
			       victim->short_descr);

	for (i = 0; i < MAX_STATS; i++)
		shadow->perm_stat[i] = sspell->ch->perm_stat[i];
	
	shadow->max_hit = (3 * sspell->ch->max_hit) / 4;
	shadow->hit = shadow->max_hit;
	shadow->max_mana = (3 * sspell->ch->max_mana) / 4;
	shadow->mana = shadow->max_mana;
	shadow->alignment = sspell->ch->alignment;
	shadow->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		shadow->armor[i] = interpolate(shadow->level,-100,100);
	shadow->armor[3] = interpolate(shadow->level,-100,0);
	shadow->sex = victim->sex;
	shadow->gold = 0;

	shadow->target  = victim;
	
	af.where	= TO_AFFECTS;
	af.type         = sspell->sn;
	af.level        = sspell->level; 
	af.duration     = 24;
	af.bitvector    = 0;
	af.modifier     = 0;
	af.location     = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	char_to_room(shadow, sspell->ch->in_room);
	do_murder(shadow, victim->name);
	return TRUE;
}  

bool spell_ruler_badge(const spell_spool_t *sspell)
{
	OBJ_DATA *badge;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	OBJ_DATA *obj_next;
	AFFECT_DATA af;

	if (get_eq_char(sspell->ch, WEAR_NECK)  != NULL)
	{
		char_puts("But you are wearing something else.\n", sspell->ch);
		return FALSE;
	}

	for (badge = sspell->ch->carrying; badge != NULL;
	   badge = obj_next)
	{
	  obj_next = badge->next_content;
	  if (badge->pIndexData->vnum == OBJ_VNUM_DEPUTY_BADGE 
	  || badge->pIndexData->vnum == OBJ_VNUM_RULER_BADGE)
	{
	  act("Your $p vanishes.", sspell->ch, badge, NULL, TO_CHAR);
	  obj_from_char(badge);
	  extract_obj(badge, 0);
	  continue;
	}
	}

	badge = create_obj(get_obj_index(OBJ_VNUM_RULER_BADGE), 0);
	badge->level = sspell->ch->level;

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = sspell->level;
	af.bitvector    = 0;

	af.location     = APPLY_HIT;
	affect_to_obj(badge,&af);

	af.location     = APPLY_MANA;
	affect_to_obj(badge,&af);

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = sspell->level / 8;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(badge,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(badge,&af);


	badge->timer = 200;
	act("You wear the ruler badge!", sspell->ch, NULL, NULL, TO_CHAR);
	act("$n wears the $s ruler badge!", sspell->ch, NULL, NULL, TO_ROOM);

	obj_to_char(badge,victim);
	if (get_eq_char(sspell->ch, WEAR_NECK)  == NULL)
		equip_char(sspell->ch, badge, WEAR_NECK);
	else
	{
    		char_puts("But you are wearing something else.\n", sspell->ch);
	}
	return TRUE;
}    

bool spell_remove_badge(const spell_spool_t *sspell)
{
	OBJ_DATA *badge;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	OBJ_DATA *obj_next;

	badge = 0;

	for (badge = victim->carrying; badge != NULL;
	   badge = obj_next)
	{
	  obj_next = badge->next_content;
	  if (badge->pIndexData->vnum == OBJ_VNUM_DEPUTY_BADGE 
	  || badge->pIndexData->vnum == OBJ_VNUM_RULER_BADGE)
	{
	  act("Your $p vanishes.", sspell->ch, badge, NULL, TO_CHAR);
	  act("$n's $p vanishes.", sspell->ch, badge, NULL, TO_ROOM);
	  
	  obj_from_char(badge);
	  extract_obj(badge, 0);
	  continue;
	}
	}
	return TRUE;
}

bool spell_shield_ruler(const spell_spool_t *sspell)
{
	int shield_vnum;
	OBJ_DATA *shield;
	AFFECT_DATA af;

	if (sspell->level >= 71) shield_vnum = OBJ_VNUM_RULER_SHIELD4;
	else if (sspell->level >= 51) shield_vnum = OBJ_VNUM_RULER_SHIELD3;
	else if (sspell->level >= 31) shield_vnum = OBJ_VNUM_RULER_SHIELD2;
	else shield_vnum = OBJ_VNUM_RULER_SHIELD1;

	shield = create_obj(get_obj_index(shield_vnum), 0);
	shield->level = sspell->ch->level;
	shield->timer = sspell->level;
	shield->cost  = 0;
	obj_to_char(shield, sspell->ch);

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = sspell->level/8;
	af.bitvector    = 0;
	af.location     = APPLY_HITROLL;

	affect_to_obj(shield,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(shield,&af);

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = sspell->level/2;
	af.bitvector    = 0;
	af.location     = APPLY_AC;
	affect_to_obj(shield,&af);

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = UMAX(1, sspell->level / 30);
	af.bitvector    = 0;
	af.location     = APPLY_CHA;
	affect_to_obj(shield,&af);

	act("You create $p!", sspell->ch,shield,NULL,TO_CHAR);
	act("$n creates $p!", sspell->ch,shield,NULL,TO_ROOM);
	return TRUE;
}

bool spell_dragon_strength(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You are already full of the strength of the dragon.\n",
		sspell->ch);
	  return FALSE;
	}

	af.where		= TO_AFFECTS;
	af.type = sspell->sn;
	af.level = sspell->level;
	af.duration = sspell->level / 3;
	af.bitvector = 0;

	af.modifier = 2;
	af.location = APPLY_HITROLL;
	affect_to_char(sspell->ch, &af);

	af.modifier = 2;
	af.location = APPLY_DAMROLL;
	affect_to_char(sspell->ch, &af);

	af.modifier = -10;
	af.location = APPLY_AC;
	affect_to_char(sspell->ch, &af);

	af.modifier = 2;
	af.location = APPLY_STR;
	affect_to_char(sspell->ch, &af);

	af.modifier = -2;
	af.location = APPLY_DEX;
	affect_to_char(sspell->ch, &af);

	char_puts("The strength of the dragon enters you.\n", sspell->ch);
	act("$n looks a bit meaner now.", sspell->ch, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_dragon_breath(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level , 6);
	if (!is_safe_spell(sspell->ch, victim, TRUE))
	{
	  if (saves_spell(sspell->ch, sspell->level, victim, DAM_FIRE, lookup_spell_msc(sspell->sn)))
	dam /= 2;
	  damage(sspell->ch, victim, dam, sspell->sn, DAM_FIRE, TRUE);
	}
	return TRUE;
}

bool spell_golden_aura(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	CHAR_DATA *vch = sspell->vo;

	for (vch = sspell->ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
	  if (!is_same_group(vch, sspell->ch)) 
	continue;

	  if (is_affected(vch, sspell->sn) || is_affected(vch, gsn_bless) || 
	  IS_AFFECTED(vch, AFF_PROTECT_EVIL))
	{
	  if (vch == sspell->ch)
	    char_puts("You are already protected by a golden aura.\n", sspell->ch);
	  else
	    act("$N is already protected by a golden aura.", sspell->ch,NULL,vch,TO_CHAR);
	  continue;
	}
	
	  af.where		= TO_AFFECTS;
	  af.type      = sspell->sn;
	  af.level	 = sspell->level;
	  af.duration  = 6 + sspell->level;
	  af.modifier  = 0;
	  af.location  = APPLY_NONE;
	  af.bitvector = AFF_PROTECT_EVIL;
	  affect_to_char(vch, &af);
	  
	  af.modifier = sspell->level/8;
	  af.location = APPLY_HITROLL;
	  af.bitvector = 0;
	  affect_to_char(vch, &af);
	  
	  af.modifier = sspell->level/8;
	  af.location = APPLY_SAVES;
	  affect_to_char(vch, &af);

	  char_puts("You feel a golden aura around you.\n", vch);
	  if (sspell->ch != vch)
	act("A golden aura surrounds $N.", sspell->ch,NULL,vch,TO_CHAR);
	}
	return TRUE;
}

bool spell_dragonplate(const spell_spool_t *sspell)	
{
	OBJ_DATA *plate;
	AFFECT_DATA af;
	
	plate = create_obj(get_obj_index(OBJ_VNUM_PLATE), 0);
	plate->level = sspell->ch->level;
	plate->timer = 2 * sspell->level;
	plate->cost  = 0;
	plate->level  = sspell->ch->level;

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = sspell->level / 8;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(plate,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(plate,&af);

	obj_to_char(plate, sspell->ch);
	
	act("You create $p!", sspell->ch,plate,NULL,TO_CHAR);
	act("$n creates $p!", sspell->ch,plate,NULL,TO_ROOM);
	return TRUE;
}

bool spell_squire(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *squire;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn)) {
		char_puts("You cannot command another squire right now.\n", sspell->ch);
		return FALSE;
	}

	char_puts("You attempt to summon a squire.\n", sspell->ch);
	act("$n attempts to summon a squire.", sspell->ch, NULL, NULL, TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  gch->pIndexData->vnum == MOB_VNUM_SQUIRE) {
			char_puts("Two squires are more than you need!\n", sspell->ch);
			return FALSE;
		}
	}

	squire = create_mob(get_mob_index(MOB_VNUM_SQUIRE));

	for (i=0;i < MAX_STATS; i++)
		squire->perm_stat[i] = sspell->ch->perm_stat[i];

	squire->max_hit = sspell->ch->max_hit;
	squire->hit = squire->max_hit;
	squire->max_mana = sspell->ch->max_mana;
	squire->mana = squire->max_mana;
	squire->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		squire->armor[i] = interpolate(squire->level,-100,100);
	squire->armor[3] = interpolate(squire->level,-100,0);
	squire->gold = 0;

	squire->damage[DICE_NUMBER] = number_range(sspell->level /20, sspell->level/15);   
	squire->damage[DICE_TYPE] = number_range(sspell->level /4, sspell->level/3);
	squire->damage[DICE_BONUS] = number_range(sspell->level /10, sspell->level/8);

	char_puts("A squire arrives from nowhere!\n", sspell->ch);
	act("A squire arrives from nowhere!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level; 
	af.duration	= 24;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	squire->master = squire->leader = sspell->ch;
	char_to_room(squire, sspell->ch->in_room);
	return TRUE;
}

bool spell_dragonsword(const spell_spool_t *sspell)	
{
	int sword_vnum;
	OBJ_DATA *sword;
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA af;
	const char	* targs;
	
	targs = one_argument(sspell->arg, arg, sizeof(arg));
	sword_vnum = 0;

	if (!str_cmp(arg, "sword"))
	sword_vnum = OBJ_VNUM_DRAGONSWORD;
	else if (!str_cmp(arg, "mace"))
	sword_vnum = OBJ_VNUM_DRAGONMACE;
	else if (!str_cmp(arg, "dagger"))
	sword_vnum = OBJ_VNUM_DRAGONDAGGER;
	else if (!str_cmp(arg, "lance"))
	sword_vnum = OBJ_VNUM_DRAGONLANCE;
	else
	{
	  char_puts("You can't make a DragonSword like that!", sspell->ch);
	  return FALSE;
	}

	sword = create_obj(get_obj_index(sword_vnum), 0);
	sword->level = sspell->ch->level;
	sword->timer = sspell->level * 2;
	sword->cost  = 0;
	if (sspell->ch->level  < 50)
		sword->value[2] = (sspell->ch->level / 10);
	else sword->value[2] = (sspell->ch->level / 6) - 3;
	sword->level = sspell->ch->level;

	af.where        = TO_OBJECT;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = -1;
	af.modifier     = sspell->level / 5;
	af.bitvector    = 0;

	af.location     = APPLY_HITROLL;
	affect_to_obj(sword,&af);

	af.location     = APPLY_DAMROLL;
	affect_to_obj(sword,&af);

	if (IS_GOOD(sspell->ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
	else if (IS_NEUTRAL(sspell->ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
	else if (IS_EVIL(sspell->ch))
	 SET_BIT(sword->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));	
	obj_to_char(sword, sspell->ch);
	
	act("You create $p!", sspell->ch,sword,NULL,TO_CHAR);
	act("$n creates $p!", sspell->ch,sword,NULL,TO_ROOM);
	return TRUE;
}

bool spell_entangle(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;
	
	if (sspell->ch->in_room->sector_type == SECT_INSIDE
	||  sspell->ch->in_room->sector_type == SECT_CITY
	||  sspell->ch->in_room->sector_type == SECT_DESERT
	||  sspell->ch->in_room->sector_type == SECT_AIR) {
		char_puts("No plants can grow here.\n", sspell->ch);
		return FALSE;
	}
	  
	dam = number_range( sspell->level, 4 * sspell->level);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_PIERCE, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	
	damage(sspell->ch, victim, dam, sspell->sn, DAM_PIERCE, TRUE);
	if (JUST_KILLED(victim))
		return TRUE;
	
	act("The thorny plants spring up around $n, entangling $s legs!",
	    victim, NULL, NULL, TO_ROOM);
	act("The thorny plants spring up around you, entangling your legs!",
	    victim, NULL, NULL, TO_CHAR);

	if (saves_spell(sspell->ch, sspell->level - 10, victim, DAM_PIERCE, lookup_spell_msc(sspell->sn)))
		victim->move -= victim->move / number_range(3, 5);
	else if (number_bits(2))
		victim->move /= 2;
	else
		victim->move = 0;

	if (!is_affected(victim, sspell->sn)) {
		AFFECT_DATA todex;
	  
		todex.type	= sspell->sn;
		todex.level	= sspell->level;
		todex.duration	= sspell->level / 10;
		todex.location	= APPLY_DEX;
		todex.modifier	= -1;
		todex.bitvector	= 0;
		affect_to_char(victim, &todex);
	}
	return TRUE;
}

bool spell_holy_armor(const spell_spool_t *sspell) 
{
	AFFECT_DATA af;
	 
	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You are already protected from harm.", sspell->ch);
	  return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level;
	af.location  = APPLY_AC;
	af.modifier  = UMAX(10, 10 * (sspell->level / 5));
	af.bitvector = 0;
	affect_to_char(sspell->ch, &af);
	act("$n is protected from harm.", sspell->ch,NULL,NULL,TO_ROOM);
	char_puts("Your are protected from harm.\n", sspell->ch);
	return TRUE;
}  

bool spell_love_potion(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 50;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	char_puts("You feel like looking at people.\n", sspell->ch);
	return TRUE;
}

bool spell_protective_shield(const spell_spool_t *sspell) {
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	 
	if (is_affected(victim, sspell->sn))
	{
	  if (victim == sspell->ch)
	   	char_puts("You are already surrounded by a protective shield.\n",
		     sspell->ch); 
	  else
	   	act("$N is already surrounded by a protective shield.", sspell->ch,NULL,
	    victim,TO_CHAR);
	  return FALSE;
	}
	af.where	= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = number_fuzzy(sspell->level / 30) + 3;
	af.location  = APPLY_AC;
	af.modifier  = -20;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n is surrounded by a protective shield.",victim,NULL,NULL,TO_ROOM);
	char_puts("You are surrounded by a protective shield.\n", victim);
	return TRUE;
}
	
bool spell_deafen(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (sspell->ch == victim) {
		char_puts("Deafen who?\n", sspell->ch);
		return FALSE;
	}

	if (is_affected(victim, sspell->sn)) {
		act("$N is already deaf.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (is_safe_nomessage(sspell->ch,victim)) {
	  char_puts("You cannot deafen that person.\n", sspell->ch);
	  return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level,victim, DAM_NONE, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 10;
	af.modifier  = 0;
	af.location  = 0;
	af.bitvector = 0;
	affect_to_char(victim,&af);

	act("You have deafened $N!", sspell->ch,NULL,victim,TO_CHAR);
	char_puts("A loud ringing fills your ears...you can't hear anything!\n",
	       victim);
	return TRUE;
}

bool spell_disperse(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn)) {
		char_puts("You aren't up to dispersing this crowd.\n", sspell->ch);
		return FALSE;
	}

	for (vch = sspell->ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (vch == sspell->ch
		||  !vch->in_room
		||  IS_SET(vch->in_room->room_flags, ROOM_NORECALL)
		||  IS_SET(vch->imm_flags, MAGIC_MENTAL)
		||  IS_IMMORTAL(vch))
			continue;

		if (IS_NPC(vch)) {
			if (IS_SET(vch->pIndexData->act, ACT_AGGRESSIVE))
				continue;
		}
		else {
			if (is_safe_nomessage(sspell->ch, vch))
				continue;
		}

		transfer_char(vch, NULL, get_random_room(vch, NULL),
			      "$N vanishes!",
			      "The world spins around you!",
			      "$N slowly fades into existence.");
	}

	af.where = TO_AFFECTS;
	af.type = sspell->sn;
	af.level = sspell->level;
	af.duration = 15;
	af.modifier = 0;
	af.location = APPLY_NONE;
	af.bitvector = 0;
	affect_to_char(sspell->ch, &af);
	return TRUE;
}

bool spell_honor_shield(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn))
	{
	  if (victim == sspell->ch)
	    char_puts("But you're already protected by your honor.\n", sspell->ch);
	  else
	char_puts("They're already protected by their honor.\n", sspell->ch);
	  return FALSE;
	}

	af.where		= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = 24 + sspell->level / 10 + sspell->percent / 20;
	af.modifier  = 30 + sspell->level / 3;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(victim, &af);

	make_spell(spell_remove_curse, sn_lookup("remove curse"), sspell->level, sspell->ch, victim, TARGET_CHAR, sspell->percent);
	make_spell(spell_bless, sn_lookup("bless"), sspell->level, sspell->ch, victim, TARGET_CHAR, sspell->percent);

	char_puts("Your honor protects you.\n", victim);
	act("$n's Honor protects $m.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}
	
bool spell_acute_vision(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_ACUTE_VISION)) {
		if (victim == sspell->ch)
			char_puts("Your vision is already acute. \n", sspell->ch);
		else
			act("$N already sees acutely.",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 3 + sspell->level / 5;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_ACUTE_VISION;
	affect_to_char(victim, &af);
	char_puts("Your vision sharpens.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_dragons_breath(const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam;
	int hpch;

	act("You call the dragon lord to help you.", sspell->ch, NULL, NULL, TO_CHAR);
	act("$n start to breath like a dragon.", sspell->ch, NULL, victim, TO_NOTVICT);
	act("$n breath disturbs you!", sspell->ch, NULL, victim, TO_VICT);
	act("You breath the breath of lord of Dragons.", sspell->ch, NULL, NULL, TO_CHAR);

	hpch = UMAX(10, sspell->ch->hit);
	hp_dam = number_range(hpch/9+1, hpch/5);
	dice_dam = dice( sspell->level,20);

	dam = UMAX(hp_dam + dice_dam / 5, dice_dam + hp_dam / 5);
	
	switch(dice(1, 5)) {
	case 1:
		fire_effect(victim->in_room, sspell->level, dam/2, TARGET_ROOM);

		for (vch = victim->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch, vch, TRUE)
		||  (IS_NPC(vch) && IS_NPC(sspell->ch) &&
		     (sspell->ch->fighting != vch || vch->fighting != sspell->ch)))
			continue;

	if (vch == victim) { /* full damage */
	    if (saves_spell(sspell->ch, sspell->level,vch,DAM_FIRE, lookup_spell_msc(sspell->sn)))
	    {
		fire_effect(vch, sspell->level/2,dam/4,TARGET_CHAR);
		damage(sspell->ch,vch,dam/2, sspell->sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch, sspell->level,dam,TARGET_CHAR);
		damage(sspell->ch,vch,dam, sspell->sn,DAM_FIRE,TRUE);
	    }
	}
	else /* partial damage */
	{
	    if (saves_spell(sspell->ch, sspell->level - 2,vch,DAM_FIRE, lookup_spell_msc(sspell->sn)))
	    {
		fire_effect(vch, sspell->level/4,dam/8,TARGET_CHAR);
		damage(sspell->ch,vch,dam/4, sspell->sn,DAM_FIRE,TRUE);
	    }
	    else
	    {
		fire_effect(vch, sspell->level/2,dam/4,TARGET_CHAR);
		damage(sspell->ch,vch,dam/2, sspell->sn,DAM_FIRE,TRUE);
	    }
	}
	}
	break;

	case 2:
		if (saves_spell(sspell->ch, sspell->level,victim,DAM_ACID, lookup_spell_msc(sspell->sn))) {
			acid_effect(victim, sspell->level/2, dam/4, TARGET_CHAR);
			damage(sspell->ch, victim, dam/2, sspell->sn, DAM_ACID,TRUE);
		}
		else {
			acid_effect(victim, sspell->level, dam, TARGET_CHAR);
			damage(sspell->ch, victim, dam, sspell->sn, DAM_ACID,TRUE);
		}
		break;

	case 3:
		cold_effect(victim->in_room, sspell->level, dam/2, TARGET_ROOM); 

		for (vch = victim->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch, vch, TRUE)
		||  (IS_NPC(vch) && IS_NPC(sspell->ch) &&
		     (sspell->ch->fighting != vch || vch->fighting != sspell->ch)))
			continue;

	if (vch == victim) /* full damage */
	{
	    if (saves_spell(sspell->ch, sspell->level,vch,DAM_COLD, lookup_spell_msc(sspell->sn)))
	    {
		cold_effect(vch, sspell->level/2,dam/4,TARGET_CHAR);
		damage(sspell->ch,vch,dam/2, sspell->sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch, sspell->level,dam,TARGET_CHAR);
		damage(sspell->ch,vch,dam, sspell->sn,DAM_COLD,TRUE);
	    }
	}
	else
	{
	    if (saves_spell(sspell->ch, sspell->level - 2,vch,DAM_COLD, lookup_spell_msc(sspell->sn)))
	    {
		cold_effect(vch, sspell->level/4,dam/8,TARGET_CHAR);
		damage(sspell->ch,vch,dam/4, sspell->sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		cold_effect(vch, sspell->level/2,dam/4,TARGET_CHAR);
		damage(sspell->ch,vch,dam/2, sspell->sn,DAM_COLD,TRUE);
	    }
	}
	}
	break;
	case 4:
		poison_effect(sspell->ch->in_room, sspell->level, dam, TARGET_ROOM);

	for (vch = sspell->ch->in_room->people; vch; vch = vch_next) {
	vch_next = vch->next_in_room;

	if (is_safe_spell(sspell->ch,vch,TRUE)
	||  (IS_NPC(sspell->ch) && IS_NPC(vch) 
	&&  (sspell->ch->fighting == vch || vch->fighting == sspell->ch)))
	    continue;

	if (saves_spell(sspell->ch, sspell->level,vch,DAM_POISON, lookup_spell_msc(sspell->sn)))
	{
	    poison_effect(vch, sspell->level/2,dam/4,TARGET_CHAR);
	    damage(sspell->ch,vch,dam/2, sspell->sn,DAM_POISON,TRUE);
	}
	else
	{
	    poison_effect(vch, sspell->level,dam,TARGET_CHAR);
	    damage(sspell->ch,vch,dam, sspell->sn,DAM_POISON,TRUE);
	}
	}
	break;
	case 5:
		if (saves_spell(sspell->ch, sspell->level, victim, DAM_LIGHTNING, lookup_spell_msc(sspell->sn))) {
			shock_effect(victim, sspell->level/2, dam/4, TARGET_CHAR);
			damage(sspell->ch, victim, dam/2, sspell->sn, DAM_LIGHTNING, TRUE);
		}
		else {
			shock_effect(victim, sspell->level, dam, TARGET_CHAR);
			damage(sspell->ch, victim, dam, sspell->sn, DAM_LIGHTNING, TRUE); 
		}
		break;
	}
	return TRUE;
}

bool spell_sand_storm(const spell_spool_t *sspell)
 {
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam;
	int hpch;

	if ((sspell->ch->in_room->sector_type == SECT_AIR)
	|| (sspell->ch->in_room->sector_type == SECT_WATER)
	|| (sspell->ch->in_room->sector_type == SECT_UNDER_WATER))
	{
	 char_puts("You don't find any sand here to make storm.\n", sspell->ch);
	 sspell->ch->wait = 0;
	 return FALSE;
	}

	act("$n creates a storm with sands on the floor.", sspell->ch,NULL,NULL,TO_ROOM);
	act("You create the ..sand.. storm.", sspell->ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(10, sspell->ch->hit);
	hp_dam  = number_range(hpch/9+1, hpch/5);
	dice_dam = dice( sspell->level,20);

	dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
	sand_effect(sspell->ch->in_room, sspell->level,dam/2,TARGET_ROOM);

	for (vch = sspell->ch->in_room->people; vch != NULL; vch = vch_next)
	{
	vch_next = vch->next_in_room;

	if (is_safe_spell(sspell->ch,vch,TRUE)
	||  (IS_NPC(vch) && IS_NPC(sspell->ch)
	&&   (sspell->ch->fighting != vch /*|| vch->fighting != ch*/)))
	    continue;

	    if (saves_spell(sspell->ch, sspell->level,vch,DAM_COLD, lookup_spell_msc(sspell->sn)))
	    {
		sand_effect(vch, sspell->level/2,dam/4,TARGET_CHAR);
		damage(sspell->ch,vch,dam/2, sspell->sn,DAM_COLD,TRUE);
	    }
	    else
	    {
		sand_effect(vch, sspell->level,dam,TARGET_CHAR);
		damage(sspell->ch,vch,dam, sspell->sn,DAM_COLD,TRUE);
	    }
	}
	return TRUE;
}

bool spell_scream(const spell_spool_t *sspell)
{
	CHAR_DATA *vch, *vch_next;
	int dam=0,hp_dam,dice_dam;
	int hpch;

	act("$n screams with a disturbing NOISE!.", sspell->ch,NULL,NULL,TO_ROOM);
	act("You scream with a powerful sound.", sspell->ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(10, sspell->ch->hit);
	hp_dam  = number_range(hpch/9+1, hpch/5);
	dice_dam = dice( sspell->level,20);
	dam = UMAX(hp_dam + dice_dam /10 , dice_dam + hp_dam /10);

	scream_effect(sspell->ch->in_room, sspell->level,dam/2,TARGET_ROOM);

	for (vch = sspell->ch->in_room->people; vch != NULL; vch = vch_next)
	{
	vch_next = vch->next_in_room;

	if (is_safe_spell(sspell->ch,vch,TRUE))
	    continue;

	    if (saves_spell(sspell->ch, sspell->level,vch,DAM_ENERGY, lookup_spell_msc(sspell->sn)))
	    {
		scream_effect(vch, sspell->level/2,dam/4,TARGET_CHAR);
/*		damage(sspell->ch,vch,dam/2, sspell->sn,DAM_ENERGY,TRUE); */
	     if (vch->fighting)  stop_fighting(vch , TRUE);
	    }
	    else
	    {
		scream_effect(vch, sspell->level,dam,TARGET_CHAR);
/*		damage(sspell->ch,vch,dam, sspell->sn,DAM_ENERGY,TRUE); */
	     if (vch->fighting)  stop_fighting(vch , TRUE);
	    }
	}
	return TRUE;
}

bool spell_attract_other(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (sspell->ch->sex == victim->sex) {
		char_puts("You'd better try your chance on other sex!\n", sspell->ch);
		return FALSE;
	}
	make_spell(spell_charm_person, sspell->sn, sspell->level+2, sspell->ch, sspell->vo, sspell->target, sspell->percent);
	return TRUE;
}

static void
cb_strip(int lang, const char **p, void *arg)
{
	char buf[MAX_STRING_LENGTH];
	const char *r = mlstr_val((mlstring*) arg, lang);
	const char *q;

	if (IS_NULLSTR(*p)
	||  (q = strstr(r, "%s")) == NULL)
		return;

	strnzncpy(buf, sizeof(buf), r, q-r);
	if (!str_prefix(buf, *p)) {
		const char *s = strdup(*p + strlen(buf));
		free_string(*p);
		*p = s;
	}
}

bool spell_animate_dead(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	CHAR_DATA *undead;
	OBJ_DATA *obj,*obj2,*next;
	AFFECT_DATA af;
	int i;
	int chance;
	int u_level;

	/* deal with the object case first */
	if (sspell->target == TARGET_OBJ) {
		MOB_INDEX_DATA *undead_idx;
		mlstring *ml;

		obj = (OBJ_DATA *) sspell->vo;

		if (!(obj->pIndexData->item_type == ITEM_CORPSE_NPC 
		|| obj->pIndexData->item_type == ITEM_CORPSE_PC)) {
			char_puts("You can animate only corpses!\n", sspell->ch);
			return FALSE;
		}

		if (is_affected(sspell->ch, sspell->sn)) {
			char_puts("You cannot summon the strength "
				     "to handle more undead bodies.\n", sspell->ch);
			return FALSE;
		}

		if (count_charmed(sspell->ch)) 
			return FALSE;

		if (sspell->ch->in_room != NULL
		&&  IS_SET(sspell->ch->in_room->room_flags, ROOM_NOMOB)) {
			char_puts("You can't animate deads here.\n", sspell->ch);
			return FALSE;
		}

		/* can't animate PC corpses in ROOM_BATTLE_ARENA */
		if (obj->pIndexData->item_type == ITEM_CORPSE_PC
		&&  obj->in_room
		&&  IS_SET(obj->in_room->room_flags, ROOM_BATTLE_ARENA)
		&&  !IS_OWNER(sspell->ch, obj)) {
			char_puts("You cannot do that.\n", sspell->ch);
			return FALSE;
		}

		if (IS_SET(sspell->ch->in_room->room_flags,
			   ROOM_SAFE | ROOM_PEACE | ROOM_PRIVATE |
			   ROOM_SOLITARY)) {
			char_puts("You can't animate here.\n", sspell->ch);
			return FALSE;
		}

		chance = URANGE(5, get_skill(sspell->ch, sspell->sn)+(sspell->level-obj->level)*7, 95);
		if (number_percent() > chance) {
			act_puts("You failed and destroyed it.\n",
				 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n tries to animate $p, but fails and destroys it.",
			    sspell->ch, obj, NULL, TO_ROOM);
			for (obj2 = obj->contains; obj2; obj2 = next) {
				next = obj2->next_content;
				obj_from_obj(obj2);
				obj_to_room(obj2, sspell->ch->in_room);
			}
			extract_obj(obj, 0);
			return TRUE;
		}

		undead_idx = get_mob_index(MOB_VNUM_UNDEAD);
		ml = mlstr_dup(obj->owner);

		/*
		 * strip "The undead body of "
		 */
		mlstr_for_each(&ml, undead_idx->short_descr, cb_strip);

		undead = create_mob_of(undead_idx, ml);
		mlstr_free(ml);

		for (i = 0; i < MAX_STATS; i++)
			undead->perm_stat[i] = UMIN(25, 12+obj->level/10);
		u_level = UMIN (obj->level, sspell->level+((obj->level
				- sspell->level)/3)*2); 

		undead->max_hit = dice(20,u_level*2)+u_level*20; 
		undead->hit = undead->max_hit;
		undead->max_mana = dice(u_level,10)+100;
		undead->mana = undead->max_mana;
		undead->max_move = 50 + sspell->percent;
		undead->move = undead->max_move / 2;
		undead->alignment = -1000;
		undead->level = u_level;

		for (i = 0; i < 3; i++)
			undead->armor[i] = interpolate(undead->level,-100, sspell->level * 1.5);
		undead->armor[3] = interpolate(undead->level, -50, 200);
		undead->sex = sspell->ch->sex;
		undead->gold = 0;
		undead->silver = 0;
		undead->damage[DICE_NUMBER] = obj->level / 5.5;
		undead->damage[DICE_TYPE]   = 6;
		undead->damage[DICE_BONUS]  = u_level/2 +10;
	
		undead->master = sspell->ch;
		undead->leader = sspell->ch;

		undead->name = str_printf(undead->name, obj->name);

		for (obj2 = obj->contains; obj2; obj2 = next) {
			next = obj2->next_content;
			obj_from_obj(obj2);
			obj_to_char(obj2, undead);
		}

		af.where     = TO_AFFECTS;
		af.type      = sspell->sn;
		af.level     = sspell->ch->level;
		af.duration  = (sspell->ch->level / 10);
		af.modifier  = 0;
		af.bitvector = 0;
		af.location  = APPLY_NONE;
		affect_to_char(sspell->ch, &af);

		act_puts("With mystic power, you animate it!",
			 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);
		act("With mystic power, $n animates $p!",
		    sspell->ch, obj, NULL, TO_ROOM);

		act_puts("$N looks at you and plans to make you "
			 "pay for distrurbing its rest!",
			 sspell->ch, NULL, undead, TO_CHAR, POS_DEAD);

		extract_obj(obj, 0);
		char_to_room(undead, sspell->ch->in_room);
		if (!JUST_KILLED(undead))
			do_wear(undead, "all");
		return TRUE;
	}

	victim = (CHAR_DATA *) sspell->vo;

	if (victim == sspell->ch) {
		char_puts("But you aren't dead!!\n", sspell->ch);
		return FALSE;
	}

	char_puts("But it ain't dead!!\n", sspell->ch);
	return FALSE;
}

bool spell_enhanced_armor(const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn))
	{
	if (victim == sspell->ch)
	  char_puts("You are already enhancedly armored.\n", sspell->ch);
	else
	  act("$N is already enhancedly armored.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}
	af.where	 = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = 24;
	af.modifier  = 60;
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel protected from all attacks.\n", victim);
	if (sspell->ch != victim)
	act("$N is protected by your magic.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_meld_into_stone(const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	 
	if (is_affected(victim, sspell->sn))
	{
	  if (victim == sspell->ch)
	   	char_puts("Your skin is already covered with stone.\n",
		     sspell->ch); 
	  else
	   	act("$N's skin is already covered with stone.", sspell->ch,NULL,
	    victim,TO_CHAR);
	  return FALSE;
	}
	af.where	= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 10;
	af.location  = APPLY_AC;
	af.modifier  = 100;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin melds into stone.",victim,NULL,NULL,TO_ROOM);
	char_puts("Your skin melds into stone.\n", victim);
	return TRUE;
}
	
bool spell_web(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (saves_spell (sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn)))
		return TRUE;

	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
			char_puts("You are already webbed.\n", sspell->ch);
		else
			act("$N is already webbed.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 1;
	af.location	= APPLY_HITROLL;
	af.modifier	= -1 * (sspell->level / 6); 
	af.where	= TO_AFFECTS;
	af.bitvector	= AFF_DETECT_WEB;
	affect_to_char(victim, &af);

	af.location	= APPLY_DEX;
	af.modifier	= -2;
	affect_to_char(victim, &af);

	af.location	= APPLY_DAMROLL;
	af.modifier	= -1 * (sspell->level / 6);  
	affect_to_char(victim, &af);
	char_puts("You are emeshed in thick webs!\n", victim);
	if (sspell->ch != victim)
		act("You emesh $N in a bundle of webs!", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_group_defense(const spell_spool_t *sspell) 
{
	CHAR_DATA *gch;
	AFFECT_DATA af;
	int shield_sn, armor_sn;

	shield_sn = sn_lookup("shield");
	armor_sn = sn_lookup("armor");

	for(gch = sspell->ch->in_room->people; gch != NULL; gch=gch->next_in_room)
	{
		if(!is_same_group(gch, sspell->ch))
			continue;
		if(is_affected(gch, armor_sn))
		{
	    	    if(gch == sspell->ch)
			char_puts("You are already armored.\n", sspell->ch);
		    else
			act("$N is already armored.", sspell->ch, NULL, gch, TO_CHAR);
		  continue;
		}

		af.type      = armor_sn;
		af.level     = sspell->level;
		af.duration  = sspell->level;
		af.location  = APPLY_AC;
		af.modifier  = 20;
		af.bitvector = 0;
		affect_to_char(gch, &af);

		char_puts("You feel someone protecting you.\n",gch);
		if(sspell->ch != gch)
			act("$N is protected by your magic.",
				sspell->ch, NULL, gch, TO_CHAR);
		
		if(!is_same_group(gch, sspell->ch))
			continue;
		if(is_affected(gch, shield_sn))
		{
		  if(gch == sspell->ch) 	
			char_puts("You are already shielded.\n", sspell->ch);
		  else 
		        act("$N is already shielded.", sspell->ch, NULL, gch, TO_CHAR);
		  continue;
		}

		af.type      = shield_sn;
		af.level     = sspell->level;
		af.duration  = sspell->level;
		af.location  = APPLY_AC;
		af.modifier   = 20;
		af.bitvector = 0;
		affect_to_char(gch, &af);

		char_puts("You are surrounded by a force shield.\n",gch);
		if(sspell->ch != gch)
			act("$N is surrounded by a force shield.",
				sspell->ch, NULL, gch, TO_CHAR);

	}
	return TRUE;
}

bool spell_inspire(const spell_spool_t *sspell) 
{
	CHAR_DATA *gch;
	AFFECT_DATA af;
	int bless_sn;

	bless_sn = sn_lookup("bless");

	for(gch = sspell->ch->in_room->people; gch != NULL; gch=gch->next_in_room)
	{
		if(!is_same_group(gch, sspell->ch))
			continue;
		if (is_affected(gch, bless_sn))
		{
		  if(gch == sspell->ch)
		     char_puts("You are already inspired.\n", sspell->ch);
		  else
		     act("$N is already inspired.",
			sspell->ch, NULL, gch, TO_CHAR);
		  continue;
		}
		af.type      = bless_sn;
		af.level     = sspell->level;
		af.duration  = 6 + sspell->level;
		af.location  = APPLY_HITROLL;
		af.modifier  = sspell->level/12;
		af.bitvector = 0;
		affect_to_char(gch, &af);

		af.location  = APPLY_SAVES;
		af.modifier  = sspell->level/12;
		affect_to_char(gch, &af);

		char_puts("You feel inspired!\n", gch);
		if(sspell->ch != gch)
			act("You inspire $N with the Creator's power!",
				sspell->ch, NULL, gch, TO_CHAR);

	}
	return TRUE;
}

bool spell_mass_sanctuary(const spell_spool_t *sspell) 
{
	CHAR_DATA *gch;

	for (gch = sspell->ch->in_room->people; gch; gch = gch->next_in_room) {
		if (is_same_group(sspell->ch, gch)) {
			make_spell(spell_sanctuary, gsn_sanctuary, sspell->level, sspell->ch,
				(void*) gch, TARGET_CHAR, sspell->percent); 
		}
	}
	return TRUE;
}

bool spell_mend(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	int result,skill;

	if (obj->condition > 99)
	{
	char_puts("That item is not in need of mending.\n", sspell->ch);
	return FALSE;
	}

	if (obj->wear_loc != -1)
	{
	char_puts("The item must be carried to be mended.\n", sspell->ch);
	return FALSE;
	}

	skill = get_skill(sspell->ch,gsn_mend) / 2;
	result = number_percent () + skill; 

	if (IS_OBJ_STAT(obj,ITEM_GLOW))
	  result -= 5;
	if (IS_OBJ_STAT(obj,ITEM_MAGIC))
	  result += 5;

	if (result >= 50)
	{
	act("$p glows brightly, and is whole again.  Good Job!", sspell->ch,obj,NULL,TO_CHAR);
	act("$p glows brightly, and is whole again.", sspell->ch,obj,NULL,TO_ROOM);
	obj->condition += result;
	obj->condition = UMIN(obj->condition , 100);
	return TRUE;
	}

	else if (result >=10)
	{
	char_puts("Nothing seemed to happen.\n", sspell->ch);
	return TRUE;
	}

	else
	{
	act("$p flares blindingly... and evaporates!", sspell->ch,obj,NULL,TO_CHAR);
	act("$p flares blindingly... and evaporates!", sspell->ch,obj,NULL,TO_ROOM);
	extract_obj(obj, 0);
	return TRUE;
	}
}

bool spell_shielding(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (saves_spell(sspell->ch, sspell->level, victim, DAM_NONE, lookup_spell_msc(sspell->sn))) {
		act("$N shivers slightly, but it passes quickly.",
		    sspell->ch, NULL, victim, TO_CHAR);
		char_puts("You shiver slightly, but it passes quickly.\n",
			  victim);
		return TRUE;
	}

	if (is_affected(victim, sspell->sn)) {
		af.type    = sspell->sn;
		af.level   = sspell->level;
		af.duration = sspell->level / 20;
		af.location = APPLY_NONE;
		af.modifier = 0;
		af.bitvector = 0;
		affect_to_char(victim, &af);
		act("You wrap $N in more flows of Spirit.",
		    sspell->ch, NULL, victim, TO_CHAR);
		char_puts("You feel the shielding get stronger.\n",victim);
		return TRUE;
	}

	af.type	= sspell->sn;
	af.level    = sspell->level;
	af.duration = sspell->level / 15;
	af.location = APPLY_NONE;
	af.modifier	= 0;
	af.bitvector = 0;
	affect_join(victim, &af);

	char_puts("You feel as if you have lost touch with something.\n",
		  victim);
	act("You shield $N from the True Source.", sspell->ch, NULL, victim, TO_CHAR);
	return TRUE;
}


bool spell_link(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int random, tmpmana;

	random = number_percent () * sspell->ch->mana * 2 / 500;
	tmpmana = sspell->ch->mana;
	sspell->ch->mana = 0;
	if (IS_PC(sspell->ch))
		sspell->ch->pcdata->endur /= 2;
	tmpmana = (.5 * tmpmana);
	tmpmana = ((tmpmana + random)/2);
	victim->mana = victim->mana + tmpmana;    
	return TRUE;
}

bool spell_power_word_kill(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	act_puts("{DA stream of darkness{x from your finger surrounds $N.", 
		sspell->ch, NULL, victim, TO_CHAR, POS_RESTING);
	act_puts("{DA stream of darkness{x from $n's finger surrounds $N.", 
		sspell->ch, NULL, victim, TO_NOTVICT, POS_RESTING);
	act_puts("{DA stream of darkness{x from $N's finger surrounds you.", 
		victim, NULL, sspell->ch, TO_CHAR, POS_RESTING);

	if (saves_spell(sspell->ch, sspell->level - 2, victim, DAM_MENTAL, lookup_spell_msc(sspell->sn))
	||  IS_IMMORTAL(victim)
	/*||  IS_CLAN_GUARD(victim)*/) {
		dam = dice(sspell->level , 24) ;
		damage(sspell->ch, victim , dam , sspell->sn, DAM_MENTAL, TRUE);
		
		if (!JUST_KILLED(sspell->ch))
		{
			if (number_bits(1) && number_percent() < sspell->percent)
				return TRUE;
			dam = dice( sspell->level, 12);
			if (saves_spell_simply(sspell->level - 8, sspell->ch, DAM_ENERGY))
				dam /= 2;
			char_puts("Cast so much spell you feel pain's wave through your's body.\n", sspell->ch);
			damage(sspell->ch, sspell->ch, dam, sn_lookup("cause critical"), DAM_ENERGY, TRUE);
		}
		return TRUE;
	}

	char_puts("You have been KILLED!\n", victim);

	act("$N has been killed!\n", sspell->ch, NULL, victim, TO_CHAR);
	act("$N has been killed!\n", sspell->ch, NULL, victim, TO_ROOM);

	raw_kill(sspell->ch, victim);
	if (!JUST_KILLED(sspell->ch))
	{
		if ((number_bits(1) || number_percent() > sspell->percent)
			&& !saves_spell_simply(UMAX(1, sspell->level - 5), sspell->ch, DAM_ENERGY))
			dam = dice( sspell->level, 16);
		else
			dam = dice( sspell->level, 14);
		if (saves_spell_simply(sspell->level - 2, sspell->ch, DAM_ENERGY))
			dam /= 2;
		char_puts("Cast so much spell you feel pain's wave through your's body.\n", sspell->ch);
		damage(sspell->ch, sspell->ch, dam, sn_lookup("cause critical"), DAM_ENERGY, TRUE);
	}
	return TRUE;
}

bool spell_eyed_sword(const spell_spool_t *sspell) 
{
	OBJ_DATA *eyed;
	int i;
/*
	if (IS_SET(sspell->ch->quest,QUEST_EYE)
	{
	 char_puts("You created your sword ,before.\n", sspell->ch);
	 return;
	}
	SET_BIT(sspell->ch->quest,QUEST_EYE);
*/
	if (IS_GOOD(sspell->ch))
		i=0;
	else if (IS_EVIL(sspell->ch))
		i=2;
	else i = 1;
	
	eyed	= create_obj_of(get_obj_index(OBJ_VNUM_EYED_SWORD),
				sspell->ch->short_descr);
	eyed->level = sspell->ch->level;
	eyed->owner = mlstr_dup(sspell->ch->short_descr);
	eyed->ed = ed_new2(eyed->pIndexData->ed, sspell->ch->name);
	eyed->value[2] = (sspell->ch->level / 10) + 3;  
	eyed->cost = 0;
	obj_to_char(eyed, sspell->ch);
	char_puts("You create YOUR sword with your name.\n", sspell->ch);
	char_puts("Don't forget that you won't be able to create this weapon anymore.\n", sspell->ch);
	return TRUE;
}

bool spell_lion_help(const spell_spool_t *sspell) 
{
	CHAR_DATA *lion;
	CHAR_DATA *victim;
	AFFECT_DATA af;
	char arg[MAX_INPUT_LENGTH];
	int i;
	const char	*targs;
	
	targs = one_argument(sspell->arg, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Whom do you want to have killed.\n", sspell->ch);
		return FALSE;
	}

	if ((victim = get_char_area(sspell->ch,arg)) == NULL) {
		char_puts("Noone around with that name.\n", sspell->ch);
		return FALSE;
	}

	if (is_safe_nomessage(sspell->ch, victim)) {
		char_puts("God protects your victim.\n", sspell->ch);
		return FALSE;
	}	

	char_puts("You call for a hunter lion.\n", sspell->ch);
	act("$n shouts a hunter lion.", sspell->ch, NULL, NULL, TO_ROOM);

	if (is_affected(sspell->ch, sspell->sn)) {
		char_puts("You cannot summon the strength to handle more "
			  "lions right now.\n", sspell->ch);
		return FALSE;
	}

	if (sspell->ch->in_room != NULL
	&&  IS_SET(sspell->ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No lions can listen you.\n", sspell->ch);
		return FALSE;
	}

	if (IS_SET(sspell->ch->in_room->room_flags,
		   ROOM_SAFE | ROOM_PEACE | ROOM_PRIVATE | ROOM_SOLITARY)
	||  (sspell->ch->in_room->exit[0] == NULL &&
	     sspell->ch->in_room->exit[1] == NULL &&
	     sspell->ch->in_room->exit[2] == NULL &&
	     sspell->ch->in_room->exit[3] == NULL &&
	     sspell->ch->in_room->exit[4] == NULL &&
	     sspell->ch->in_room->exit[5] == NULL)
	||  (sspell->ch->in_room->sector_type != SECT_FIELD &&
	     sspell->ch->in_room->sector_type != SECT_FOREST &&
	     sspell->ch->in_room->sector_type != SECT_MOUNTAIN &&
	     sspell->ch->in_room->sector_type != SECT_HILLS)) {
		char_puts("No hunter lion can come to you.\n", sspell->ch);
		return FALSE;
	}

	lion = create_mob(get_mob_index(MOB_VNUM_HUNTER));

	for (i=0;i < MAX_STATS; i++)
		lion->perm_stat[i] = UMIN(25,2 * sspell->ch->perm_stat[i]);

	lion->max_hit =  UMIN(30000, sspell->ch->max_hit * 1.2);
	lion->hit = lion->max_hit;
	lion->max_mana = sspell->ch->max_mana;
	lion->mana = lion->max_mana;
	lion->alignment = sspell->ch->alignment;
	lion->level = UMIN(100, sspell->ch->level);
	for (i=0; i < 3; i++)
	lion->armor[i] = interpolate(lion->level,-100,100);
		lion->armor[3] = interpolate(lion->level,-100,0);
	lion->sex = sspell->ch->sex;
	lion->gold = 0;
	lion->damage[DICE_NUMBER] = number_range(sspell->level /15, sspell->level/10);   
	lion->damage[DICE_TYPE] = number_range(sspell->level /3, sspell->level/2);
	lion->damage[DICE_BONUS] = number_range(sspell->level /8, sspell->level/6);
	
	char_puts("A hunter lion comes to kill your victim!\n", sspell->ch);
	act("A hunter lion comes to kill $n's victim!",
	    sspell->ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->ch->level; 
	af.duration	= 24;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(sspell->ch, &af);  
	lion->hunting = victim;
	char_to_room(lion, sspell->ch->in_room);
	if (!JUST_KILLED(lion))
		return TRUE;
	hunt_victim(lion);
	return TRUE;
}

bool spell_magic_jar(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	OBJ_DATA *vial;
	OBJ_DATA *fire;
	int i;
	const char *tmp;

	if (victim == sspell->ch)
	{
		char_puts("You like yourself even better.\n", sspell->ch);
		return FALSE;
	}

	if (IS_NPC(victim))
	{
		char_puts("Your victim is a npc. Not necessary!.\n", sspell->ch);
		return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level ,victim,DAM_MENTAL, lookup_spell_msc(sspell->sn)))
	   {
	    char_puts("You failed.\n", sspell->ch);
	    return TRUE;
	   }

	for(vial = sspell->ch->carrying; vial != NULL; vial=vial->next_content)
	  if (vial->pIndexData->vnum == OBJ_VNUM_POTION_VIAL)
	    break;

	if ( vial == NULL)  {
	 char_puts("You don't have any vials to put your victim's spirit.\n"
			, sspell->ch);
	return FALSE;
	}
	extract_obj(vial, 0);
	if (IS_GOOD(sspell->ch))
		i=0;
	else if (IS_EVIL(sspell->ch))
		i=2;
	else i = 1;

	fire	= create_obj_of(get_obj_index(OBJ_VNUM_MAGIC_JAR),
				victim->short_descr);
	fire->level = sspell->ch->level;
	fire->owner = mlstr_dup(sspell->ch->short_descr);

	fire->ed = ed_new2(fire->pIndexData->ed, victim->name);

	fire->cost = 0;
	fire->timer = sspell->level + sspell->percent / 2;
	
	tmp = fire->name;
	fire->name = str_printf("%s %s", fire->name, victim->name);
	free_string(tmp);
	
	obj_to_char(fire , sspell->ch);    
	SET_BIT(victim->pcdata->plr_flags, PLR_NOEXP);
	char_printf(sspell->ch,"You catch %s's spirit in to your vial.\n",
		    victim->name);
	return TRUE;
}

DO_FUN(do_flee);

bool turn_spell(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam, align;

	if (IS_EVIL(sspell->ch)) {
		victim = sspell->ch;
		char_puts("The energy explodes inside you!\n", sspell->ch);
	}

	if (victim != sspell->ch) {
		act("$n raises $s hand, and a blinding ray of light "
		    "shoots forth!",
		    sspell->ch, NULL, NULL, TO_ROOM);
		char_puts("You raise your hand and a blinding ray of light "
			  "shoots forth!\n", sspell->ch);
	}

	if (IS_GOOD(victim) || IS_NEUTRAL(victim)) {
		act("$n seems unharmed by the light.",
		    victim, NULL, victim, TO_ROOM);
		char_puts("The light seems powerless to affect you.\n",
			  victim);
		return FALSE;
	}

	dam = dice( sspell->level, 10);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_HOLY, lookup_spell_msc(sspell->sn)))
		dam /= 2;

	align = victim->alignment;
	align -= 350;

	if (align < -1000)
		align = -1000 + (align + 1000) / 3;

	dam = (dam * align * align) / 1000000;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_HOLY, TRUE);
	if (!JUST_KILLED(victim))
		do_flee(victim, str_empty);
	return TRUE;
}

bool spell_turn(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn)) {
		char_puts("This power is used too recently.", sspell->ch);
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 5;
	af.modifier	= 0;
	af.location	= 0;
	af.bitvector	= 0;
	affect_to_char(sspell->ch, &af);

	for (vch = sspell->ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch, vch, TRUE))
			continue;
		make_spell(turn_spell, sspell->sn, sspell->ch->level, sspell->ch,
				vch, sspell->target, sspell->percent);
	}
	return TRUE;
}


bool spell_fear(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	class_t *vcl;

	if ((vcl = class_lookup(victim->class))
	&&  !CAN_FLEE(victim, vcl)) {
		if (victim == sspell->ch)
			char_puts("You are beyond this power.\n", sspell->ch);
		else
			char_puts("Your victim is beyond this power.\n", sspell->ch);
		return FALSE;
	}

	if (is_affected(victim,gsn_fear))
		return FALSE;
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_OTHER, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where	= TO_AFFECTS;
	af.type		= gsn_fear;
	af.level	= sspell->level;
	af.duration	= sspell->level / 10;
	af.location	= 0;
	af.modifier	= 0;
	af.bitvector	= AFF_DETECT_FEAR;
	affect_to_char(victim, &af);
	char_puts("You are afraid as much as a rabbit.\n", victim);
	act("$n looks with afraid eyes.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_protection_heat (const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, gsn_protection_heat))
	{
	if (victim == sspell->ch)
	  char_puts("You are already protected from heat.\n", sspell->ch);
	else
	  act("$N is already protected from heat.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	if (is_affected(victim, gsn_protection_cold))
	{
	if (victim == sspell->ch)
	  char_puts("You are already protected from cold.\n", sspell->ch);
	else
	  act("$N is already protected from cold.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	if (is_affected(victim, gsn_fire_shield))
	{
	if (victim == sspell->ch)
	  char_puts("You are already using fire shield.\n", sspell->ch);
	else
	  act("$N is already using fire shield.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = gsn_protection_heat;
	af.level     = sspell->level;
	af.duration  = 24;
	af.location  = APPLY_SAVES;
	af.modifier  = 1;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel strengthed against heat.\n", victim);
	if (sspell->ch != victim)
	act("$N is protected against heat.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_protection_cold (const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, gsn_protection_cold))
	{
	if (victim == sspell->ch)
	  char_puts("You are already protected from cold.\n", sspell->ch);
	else
	  act("$N is already protected from cold.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	if (is_affected(victim, gsn_protection_heat))
	{
	if (victim == sspell->ch)
	  char_puts("You are already protected from heat.\n", sspell->ch);
	else
	  act("$N is already protected from heat.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	if (is_affected(victim, gsn_fire_shield))
	{
	if (victim == sspell->ch)
	  char_puts("You are already using fire shield.\n", sspell->ch);
	else
	  act("$N is already using fire shield.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}
	af.where     = TO_AFFECTS;
	af.type      = gsn_protection_cold;
	af.level     = sspell->level;
	af.duration  = 24;
	af.location  = APPLY_SAVES;
	af.modifier  = 1;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel strengthed against cold.\n", victim);
	if (sspell->ch != victim)
	act("$N is protected against cold.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_fire_shield (const spell_spool_t *sspell)
{
	OBJ_INDEX_DATA *pObjIndex;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *fire;
	int i;
	const char	*targs;

	targs = one_argument(sspell->arg, arg, sizeof(arg));
	if (str_cmp(arg, "cold") && str_cmp(arg, "fire")) {
		char_puts("You must specify the type.\n", sspell->ch);
		return FALSE;
	}

	if (IS_GOOD(sspell->ch))
		i = 0;
	else if (IS_EVIL(sspell->ch))
		i = 2;
	else
		i = 1;
	
	pObjIndex = get_obj_index(OBJ_VNUM_FIRE_SHIELD);
	fire	= create_obj(pObjIndex, 0);
	fire->level = sspell->ch->level;
	name_add(&fire->name, arg, NULL, NULL);

	fire->owner = mlstr_dup(sspell->ch->short_descr);
	fire->ed = ed_new2(fire->pIndexData->ed, arg);

	fire->cost = 0;
	fire->timer = 5 * sspell->ch->level ;
	if (IS_GOOD(sspell->ch))
		SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_EVIL));
	else if (IS_NEUTRAL(sspell->ch))
		SET_BIT(fire->extra_flags,(ITEM_ANTI_GOOD | ITEM_ANTI_EVIL));
	else if (IS_EVIL(sspell->ch))
		SET_BIT(fire->extra_flags,(ITEM_ANTI_NEUTRAL | ITEM_ANTI_GOOD));
	obj_to_char(fire, sspell->ch);
	char_puts("You create the fire shield.\n", sspell->ch);
	return TRUE;
}

bool spell_witch_curse(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (is_affected(victim, gsn_witch_curse)) {
		char_puts("It has already underflowing with health.\n", sspell->ch);
		return FALSE;
	}

	if (IS_IMMORTAL(victim)
	/*||  IS_CLAN_GUARD(victim)*/) {
		damage(sspell->ch, victim, dice( sspell->level, 8), sspell->sn, DAM_NEGATIVE, TRUE);
		return TRUE;
	}

	sspell->ch->hit -= (2 * sspell->level);

	af.where	= TO_AFFECTS;
	af.type         = gsn_witch_curse;
	af.level        = sspell->level; 
	af.duration     = 6;
	af.location     = APPLY_HIT;
	af.modifier     = - sspell->level;
	af.bitvector    = 0;
	affect_to_char(victim, &af);

	act("Now $n got the path to death.", victim, NULL, NULL, TO_ROOM);
	act("Now you got the path to death.", victim, NULL, NULL, TO_CHAR);
	if (sspell->ch != victim)		// :)))
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_knock (const spell_spool_t *sspell)
{
	char arg[MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	int chance=0;
	int door;
	const	int	rev_dir		[]		=
	{
	    2, 3, 0, 1, 5, 4
	};
	const char	* targs;

	targs = one_argument(sspell->arg, arg, sizeof(arg));
 
	if (arg[0] == '\0')
	{
	char_puts("Knock which door or direction.\n", sspell->ch);
	return FALSE;
	}

	if (sspell->ch->fighting)
	{	
	char_puts("Wait until the fight finishes.\n", sspell->ch);
	return FALSE;
	}

	if ((door = find_door(sspell->ch, arg)) >= 0
	&& (pexit = sspell->ch->in_room->exit[door])
	&& !IS_SET(pexit->exit_info, EX_BURYED)
	&& pexit->to_room.r)
	{
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *pexit_rev;

	if (!IS_SET(pexit->exit_info, EX_CLOSED))
	    { char_puts("It's already open.\n",      sspell->ch); return FALSE; }
	if (!IS_SET(pexit->exit_info, EX_LOCKED))
	    { char_puts("Just try to open it.\n",     sspell->ch); return FALSE; }
	if (IS_SET(pexit->exit_info, EX_NOPASS))
	    { char_puts("A mystical shield protects the exit.\n", sspell->ch); 
	      return FALSE; }
	chance = sspell->ch->level / 5 + get_curr_stat(sspell->ch,STAT_INT) + get_skill(sspell->ch, sspell->sn) / 5;

	act("You knock $d, and try to open $d!",
		sspell->ch,NULL,pexit->keyword,TO_CHAR);
	act("You knock $d, and try to open $d!",
		sspell->ch,NULL,pexit->keyword,TO_ROOM);

	if (room_dark(sspell->ch->in_room))
		chance /= 2;

	/* now the attack */
	if (number_percent() < chance)
	 {
	REMOVE_BIT(pexit->exit_info, EX_LOCKED);
	REMOVE_BIT(pexit->exit_info, EX_CLOSED);
	act("$n knocks the the $d and opens the lock.", sspell->ch, NULL, 
		pexit->keyword, TO_ROOM);
	act_puts("You successed to open the door.",
		 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);

	/* open the other side */
	if ((to_room   = pexit->to_room.r           ) != NULL
	&&   (pexit_rev = to_room->exit[rev_dir[door]]) != NULL
	&&   pexit_rev->to_room.r == sspell->ch->in_room)
	{
	    CHAR_DATA *rch;

	    REMOVE_BIT(pexit_rev->exit_info, EX_CLOSED);
	    REMOVE_BIT(pexit_rev->exit_info, EX_LOCKED);
	    for (rch = to_room->people; rch != NULL; rch = rch->next_in_room)
		act("The $d opens.", rch, NULL, pexit_rev->keyword, TO_CHAR);
	}
	 }
	else
	 {
	act("You couldn't knock the $d!",
	    sspell->ch,NULL,pexit->keyword,TO_CHAR);
	act("$n failed to knock $d.",
	    sspell->ch,NULL,pexit->keyword,TO_ROOM);
	 }
	return TRUE;
	}

	char_puts("You can't see that here.\n", sspell->ch);
	return FALSE;
}


bool spell_magic_resistance (const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (!is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You are now resistive to magic.\n", sspell->ch);

	  af.where = TO_RESIST;
	  af.type = sspell->sn;
	  af.duration = sspell->level / 10;
	  af.level = sspell->ch->level;
	  af.bitvector = FORCE_MAGIC;
	  af.location = APPLY_SAVING_MENTAL;
	  af.modifier = 5;
	  affect_to_char(sspell->ch, &af);
	  return TRUE;
	}
	else 
	  char_puts("You are already resistive to magic.\n", sspell->ch);
	 return FALSE;
}

bool spell_hallucination (const spell_spool_t *sspell) 
{
 char_puts("That spell is under construction.\n", sspell->ch);
 return FALSE;
}

bool spell_wolf(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *demon;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn))
	{
	char_puts("You lack the power to summon another wolf right now.\n", sspell->ch);
	  return FALSE;
	}

	char_puts("You attempt to summon a wolf.\n", sspell->ch);
	act("$n attempts to summon a wolf.", sspell->ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  gch->pIndexData->vnum == MOB_VNUM_WOLF) {
			char_puts("Two wolfs are more than you can control!\n",
				  sspell->ch);
			return FALSE;
		}
	}

	demon = create_mob(get_mob_index(MOB_VNUM_WOLF));

	for (i=0;i < MAX_STATS; i++)
	{
	  demon->perm_stat[i] = sspell->ch->perm_stat[i];
	}

	demon->max_hit = IS_NPC(sspell->ch)? URANGE(sspell->ch->max_hit,1 * sspell->ch->max_hit,30000)
		: URANGE(sspell->ch->pcdata->perm_hit, sspell->ch->hit,30000);
	demon->hit = demon->max_hit;
	demon->max_mana = IS_NPC(sspell->ch)? sspell->ch->max_mana : sspell->ch->pcdata->perm_mana;
	demon->mana = demon->max_mana;
	demon->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		demon->armor[i] = interpolate(demon->level,-100,100);
	demon->armor[3] = interpolate(demon->level,-100,0);
	demon->gold = 0;
	demon->timer = 0;
	demon->damage[DICE_NUMBER] = number_range(sspell->level /15, sspell->level/10);   
	demon->damage[DICE_TYPE] = number_range(sspell->level /3, sspell->level/2);
	demon->damage[DICE_BONUS] = number_range(sspell->level /8, sspell->level/6);

	char_puts("The wolf arrives and bows before you!\n", sspell->ch);
	act("A wolf arrives from somewhere and bows!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	demon->master = demon->leader = sspell->ch;
	char_to_room(demon, sspell->ch->in_room);
	return TRUE;
}

bool spell_vampiric_blast(const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice( sspell->level, 12);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ACID, lookup_spell_msc(sspell->sn)))
	dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ACID,TRUE);
	return TRUE;
}

bool spell_dragon_skin(const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	 
	if (is_affected(victim, sspell->sn))
	{
	  if (victim == sspell->ch)
	   	char_puts("Your skin is already hard as rock.\n",
		     sspell->ch); 
	  else
	   	act("$N's skin is already hard as rock.", sspell->ch,NULL,
	    victim,TO_CHAR);
	  return FALSE;
	}
	af.where	= TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level;
	af.location  = APPLY_AC;
	af.modifier  = 4 * sspell->level;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin is now hard as rock.",victim,NULL,NULL,TO_ROOM);
	char_puts("Your skin is now hard as rock.\n", victim);
	return TRUE;
}
	

bool spell_mind_light(const spell_spool_t *sspell)
{
	AFFECT_DATA af,af2;

	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	char_puts("This room has already had booster of mana.\n", sspell->ch);
	return FALSE;
	}

	af.where     = TO_ROOM_CONST;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 30;
	af.location  = APPLY_ROOM_MANA;
	af.modifier  = sspell->level;
	af.bitvector = 0;
	affect_to_room(sspell->ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sspell->sn;
	af2.level	 = sspell->level;
	af2.duration  = sspell->level / 10;
	af2.modifier  = 0;
	af2.location  = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(sspell->ch, &af2);
	char_puts("The room starts to be filled with mind light.\n", sspell->ch);
	act("The room starts to be filled with $n's mind light.", sspell->ch,NULL,NULL,TO_ROOM);
	return TRUE;
}

bool spell_insanity (const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_NPC(victim))
	{
	 char_puts("This spell can cast on PC's only.\n", sspell->ch);
	 return FALSE;
	}

	if (IS_AFFECTED(victim,AFF_BLOODTHIRST)
	|| saves_spell(sspell->ch, sspell->level, victim,DAM_OTHER, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 10;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_BLOODTHIRST;
	affect_to_char(victim, &af);
	char_puts("You are aggressive as a battlerager.\n", victim);
	act("$n looks with red eyes.",victim,NULL,NULL,TO_ROOM);
	return TRUE;
}


bool spell_power_stun (const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;


	if (is_affected(victim, sspell->sn))
		return FALSE;
	
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_OTHER, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 80;
	af.location  = APPLY_DEX;
	af.modifier  = - 3;
	af.bitvector = AFF_STUN;
	affect_to_char(victim, &af);
	char_puts("You are stunned.\n", victim);
	act_puts("$n is stunned.",victim,NULL,NULL,TO_ROOM,POS_SLEEPING);
	return TRUE;
}



bool spell_improved_invis(const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_IMP_INVIS))
		return FALSE;

	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 10 ;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_IMP_INVIS;
	affect_to_char(victim, &af);
	char_puts("You fade out of existence.\n", victim);
	return TRUE;
}

bool spell_improved_detect(const spell_spool_t *sspell) 
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_IMP_INVIS))
	{
	if (victim == sspell->ch)
	  char_puts("You can already see improved invisible.\n", sspell->ch);
	else
	  act("$N can already see improved invisible mobiles.", sspell->ch,NULL,victim,TO_CHAR);
	return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 3;
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_IMP_INVIS;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (sspell->ch != victim)
	char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_severity_force(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	char_printf(sspell->ch,"You cracked the ground towards the %s.\n",victim->name);
	act("$n cracked the ground towards you!.", sspell->ch, NULL, victim, TO_VICT);
	dam = dice(sspell->level , 12);
	damage(sspell->ch,victim,dam, sspell->sn,DAM_NONE,TRUE);
	return TRUE;
}

bool spell_randomizer(const spell_spool_t *sspell)
{
	AFFECT_DATA af,af2;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts
	("Your power of randomness has been exhausted for now.\n",
	 sspell->ch);
	  return FALSE;
	}

	if (IS_SET(sspell->ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts(
	    "This room is far too orderly for your powers to work on it.\n",
		   sspell->ch);
	  return FALSE;
	}
	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	char_puts("This room has already been randomized.\n", sspell->ch);
	return FALSE;
	}

	if (number_bits(1) == 0)
	{
	  char_puts("Despite your efforts, the universe resisted chaos.\n", sspell->ch);
	  af2.where     = TO_AFFECTS;
	  af2.type      = sspell->sn;
	  af2.level	    = sspell->ch->level;
	  af2.duration  = sspell->level / 10;
	  af2.modifier  = 0;
	  af2.location  = APPLY_NONE;
	  af2.bitvector = 0;
	  affect_to_char(sspell->ch, &af2);
	  return TRUE;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_RANDOMIZER;
	affect_to_room(sspell->ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sspell->sn;
	af2.level	  = sspell->ch->level;
	af2.duration  = sspell->level / 5;
	af2.modifier  = 0;
	af2.location  = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(sspell->ch, &af2);
	char_puts("The room was successfully randomized!\n", sspell->ch);
	char_puts("You feel very drained from the effort.\n", sspell->ch);
	sspell->ch->hit -= UMIN(200, sspell->ch->hit/2);
	act("The room starts to randomize exits.", sspell->ch,NULL,NULL,TO_ROOM);
	return TRUE;
}

bool  spell_bless_weapon(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (obj->pIndexData->item_type != ITEM_WEAPON)
	{
	char_puts("That isn't a weapon.\n", sspell->ch);
	return FALSE;
	}

	if (obj->wear_loc != -1)
	{
	char_puts("The item must be carried to be blessed.\n", sspell->ch);
	return FALSE;
	}

	if (obj->pIndexData->item_type == ITEM_WEAPON)
	{
	 if (IS_WEAPON_STAT(obj,WEAPON_FLAMING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_FROST)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VAMPIRIC)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHARP)
	    ||  IS_WEAPON_STAT(obj,WEAPON_VORPAL)
	    ||  IS_WEAPON_STAT(obj,WEAPON_SHOCKING)
	    ||  IS_WEAPON_STAT(obj,WEAPON_HOLY)
	    ||  IS_OBJ_STAT(obj,ITEM_BLESS) 
	    || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	 {
	act("You can't seem to bless $p.", sspell->ch,obj,NULL,TO_CHAR);
	return FALSE;
	 }
	}
	if (IS_WEAPON_STAT(obj,WEAPON_HOLY))
	{
	act("$p is already blessed for holy attacks.", sspell->ch,obj,NULL,TO_CHAR);
	return FALSE;
	}

	af.where	 = TO_WEAPON;
	af.type	 = sspell->sn;
	af.level	 = sspell->level / 2;
	af.duration	 = sspell->level/8;
	af.location	 = 0;
	af.modifier	 = 0;
	af.bitvector = WEAPON_HOLY;
	affect_to_obj(obj,&af);

	act("$p is prepared for holy attacks.", sspell->ch,obj,NULL,TO_ALL);
	return TRUE;

}

bool spell_resilience(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (!is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You are now resistive to draining attacks.\n", sspell->ch);

	  af.where = TO_RESIST;
	  af.type = sspell->sn;
	  af.duration = sspell->level / 10;
	  af.level = sspell->ch->level;
	  af.bitvector = MAGIC_AIR;
	  af.location = 0;
	  af.modifier = 0;
	  affect_to_char(sspell->ch, &af);
	  return TRUE;
	}
	else 
	  char_puts("You are already resistive to draining attacks.\n", sspell->ch);
	 return FALSE;
}

bool spell_superior_heal(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int bonus = 170 + sspell->level + dice(1,20);

	if (victim->hit >= victim->max_hit)
		return FALSE;
	victim->hit = UMIN(victim->hit + bonus, victim->max_hit);
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (sspell->ch != victim) {
		char_puts("Ok.\n", sspell->ch);
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}

bool spell_master_healing(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int bonus = 300 + sspell->level + dice(1,40);

	if (victim->hit >= victim->max_hit)
		return FALSE;
	victim->hit = UMIN(victim->hit + bonus, victim->max_hit);
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (sspell->ch != victim) {
		char_puts("Ok.\n", sspell->ch);
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}

bool spell_group_heal(const spell_spool_t *sspell)
{
	CHAR_DATA *gch;
	int heal_sn, refresh_sn;

	heal_sn = sn_lookup("master healing");
	refresh_sn = sn_lookup("refresh");

	if (heal_sn < 0 || refresh_sn < 0)
		return FALSE;

	for (gch = sspell->ch->in_room->people; gch != NULL; gch = gch->next_in_room) {
		if (is_same_group(sspell->ch, gch)) {
			make_spell(spell_heal, heal_sn, sspell->level, sspell->ch, gch, TARGET_CHAR, sspell->percent);
			make_spell(spell_refresh, refresh_sn, sspell->level, sspell->ch, gch, TARGET_CHAR, sspell->percent);
		}
	}
	return TRUE;
}

bool spell_restoring_light(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int nsn, mana_add;

	if (IS_AFFECTED(victim,AFF_BLIND))
	{
	 nsn = sn_lookup("cure blindness");
	 make_spell(spell_cure_blindness, nsn, sspell->level, sspell->ch,(void *)victim,TARGET_CHAR, sspell->percent);
	}
	if (IS_AFFECTED(victim,AFF_CURSE))
	{
	 nsn = sn_lookup("remove curse");
	 make_spell(spell_remove_curse, nsn, sspell->level, sspell->ch,(void *)victim,TARGET_CHAR, sspell->percent);
	}
	if (IS_AFFECTED(victim,AFF_POISON))
	{
	 make_spell(spell_cure_poison, gsn_cure_poison, sspell->level, sspell->ch,(void *)victim,TARGET_CHAR, sspell->percent);
	}
	if (IS_AFFECTED(victim,AFF_PLAGUE))
	{
	 nsn = sn_lookup("cure disease");
	 make_spell(spell_cure_disease, nsn, sspell->level, sspell->ch,(void *)victim,TARGET_CHAR, sspell->percent);
	}

	if (victim->hit != victim->max_hit)
	{
		 mana_add = UMIN((victim->max_hit - victim->hit), sspell->ch->mana);
		 victim->hit = UMIN(victim->hit + mana_add, victim->max_hit);
		 sspell->ch->mana -= mana_add;
	}
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}


bool spell_lesser_golem(const spell_spool_t *sspell)
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You lack the power to create another golem right now.\n",
		   sspell->ch);
	  return FALSE; 
	}

	char_puts("You attempt to create a lesser golem.\n", sspell->ch);
	act("$n attempts to create a lesser golem.", sspell->ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  (gch->pIndexData->vnum == MOB_VNUM_LESSER_GOLEM
		     || gch->pIndexData->vnum == MOB_VNUM_ADAMANTITE_GOLEM))
		{
			char_puts("You may control only one lesser or adamantite golem!\n", sspell->ch);
			return FALSE;
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_LESSER_GOLEM));

	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,12 + sspell->ch->level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 2;
	golem->perm_stat[STAT_CON] += 2;

	golem->max_hit = IS_NPC(sspell->ch)? URANGE(sspell->ch->max_hit,1 * sspell->ch->max_hit,30000)
		: UMIN((2 * sspell->ch->pcdata->perm_hit) + 400,30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(sspell->ch)? sspell->ch->max_mana : sspell->ch->pcdata->perm_mana;
	golem->mana = golem->max_mana;
	golem->max_move = 50 + sspell->percent * 2;
	golem->move = golem->max_move;
	golem->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		golem->armor[i] = interpolate(golem->level,-100,80);
	golem->armor[3] = interpolate(golem->level,-100,-10);
	golem->gold = 0;
	golem->timer = 0;
	golem->damage[DICE_NUMBER] = 3;   
	golem->damage[DICE_TYPE] = 10;
	golem->damage[DICE_BONUS] = sspell->ch->level / 2;
	golem->hitroll = sspell->percent / 4;
	golem->damroll = sspell->percent / 4;
	
	golem->carry_number = 1000; /* don't get and wear obj's */

	char_puts("You created a lesser golem!\n", sspell->ch);
	act("$n creates a lesser golem!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24 + sspell->percent / 10;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	golem->master = golem->leader = sspell->ch;
	char_to_room(golem, sspell->ch->in_room);
	return TRUE;
}


bool spell_stone_golem(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn)) 
	{
	  char_puts("You lack the power to create another golem right now.\n",
		   sspell->ch);
	  return FALSE;
	}

	char_puts("You attempt to create a stone golem.\n", sspell->ch);
	act("$n attempts to create a stone golem.", sspell->ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  (gch->pIndexData->vnum == MOB_VNUM_STONE_GOLEM
		    || gch->pIndexData->vnum == MOB_VNUM_ADAMANTITE_GOLEM))
		{
			char_puts("You may control only one stone or adamantite golem!\n", sspell->ch);
			return FALSE;
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_STONE_GOLEM));


	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,12 + sspell->ch->level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 2;
	golem->perm_stat[STAT_CON] += 2;

	golem->max_hit = IS_NPC(sspell->ch)? URANGE(sspell->ch->max_hit,1 * sspell->ch->max_hit,30000)
		: UMIN((5 * sspell->ch->pcdata->perm_hit) + 2000, 30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(sspell->ch)? sspell->ch->max_mana : sspell->ch->pcdata->perm_mana;
	golem->mana = golem->max_mana;
	golem->max_move = 100 + sspell->percent / 2;
	golem->move = golem->max_move;
	golem->level = sspell->ch->level;
	for (i=0; i < 3; i++)
	golem->armor[i] = interpolate(golem->level,-100,100);
		golem->armor[3] = interpolate(golem->level,-100,0);
	golem->gold = 0;
	golem->timer = 0;
	golem->damage[DICE_NUMBER] = 8;   
	golem->damage[DICE_TYPE] = 4;
	golem->damage[DICE_BONUS] = sspell->ch->level / 2;
	golem->hitroll = sspell->percent / 3;
	golem->damroll = sspell->percent / 2;

	golem->carry_number = 1000; /* don't get and wear obj's */

	char_puts("You created a stone golem!\n", sspell->ch);
	act("$n creates a stone golem!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	golem->master = golem->leader = sspell->ch;
	char_to_room(golem, sspell->ch->in_room);
	return TRUE;
}

bool spell_iron_golem(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn)) 
	{
	  char_puts("You lack the power to create another golem right now.\n", sspell->ch);
	  return FALSE;
	}

	char_puts("You attempt to create an iron golem.\n", sspell->ch);
	act("$n attempts to create an iron golem.", sspell->ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  (gch->pIndexData->vnum == MOB_VNUM_IRON_GOLEM
		    || gch->pIndexData->vnum == MOB_VNUM_ADAMANTITE_GOLEM))
		{
			char_puts("You may control only one iron or adamantite golem!\n", sspell->ch);
			return FALSE;
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_IRON_GOLEM));


	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,12 + sspell->ch->level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 1;
	golem->perm_stat[STAT_CON] += 2;

	golem->max_hit = IS_NPC(sspell->ch)? URANGE(sspell->ch->max_hit,1 * sspell->ch->max_hit,30000)
		: UMIN((10 * sspell->ch->pcdata->perm_hit) + 1000, 30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(sspell->ch)? sspell->ch->max_mana : sspell->ch->pcdata->perm_mana;
	golem->mana = golem->max_mana;
	golem->max_move = 100 + sspell->percent;
	golem->move = golem->max_move;
	golem->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		golem->armor[i] = interpolate(golem->level,-100,120);
	golem->armor[3] = interpolate(golem->level,-100,10);
	golem->gold = 0;
	golem->timer = 0;
	golem->damage[DICE_NUMBER] = 11;   
	golem->damage[DICE_TYPE] = 5;
	golem->damage[DICE_BONUS] = sspell->ch->level / 2 + 10;
	golem->hitroll = sspell->percent * 0.6;
	golem->damroll = sspell->percent * 0.6;

	golem->carry_number = 1000; /* don't get and wear obj's */

	char_puts("You created an iron golem!\n", sspell->ch);
	act("$n creates an iron golem!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	golem->master = golem->leader = sspell->ch;
	char_to_room(golem, sspell->ch->in_room);
	return TRUE;
}

bool spell_adamantite_golem(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *golem;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You lack the power to create another golem right now.\n",
		   sspell->ch);
	  return FALSE;
	}

	char_puts("You attempt to create an Adamantite golem.\n", sspell->ch);
	act("$n attempts to create an Adamantite golem.", sspell->ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == sspell->ch
		&& (gch->pIndexData->vnum == MOB_VNUM_ADAMANTITE_GOLEM
		   || gch->pIndexData->vnum == MOB_VNUM_LESSER_GOLEM
		   || gch->pIndexData->vnum == MOB_VNUM_STONE_GOLEM
		   || gch->pIndexData->vnum == MOB_VNUM_IRON_GOLEM))
		{
			char_puts("You may control only one adamantite golem (and no another golem)!\n", sspell->ch);
			return FALSE;
		}
	}

	golem = create_mob(get_mob_index(MOB_VNUM_ADAMANTITE_GOLEM));

	for (i = 0; i < MAX_STATS; i ++)
	   golem->perm_stat[i] = UMIN(25,13 + sspell->ch->level/10);
	        
	golem->perm_stat[STAT_STR] += 3;
	golem->perm_stat[STAT_INT] -= 2;
	golem->perm_stat[STAT_CON] += 2;
	golem->perm_stat[STAT_DEX] += 1;

	golem->max_hit = IS_NPC(sspell->ch)? URANGE(sspell->ch->max_hit,1 * sspell->ch->max_hit,30000)
		: UMIN((10 * sspell->ch->pcdata->perm_hit) + 4000, 30000);
	golem->hit = golem->max_hit;
	golem->max_mana = IS_NPC(sspell->ch)? sspell->ch->max_mana : sspell->ch->pcdata->perm_mana;
	golem->mana = golem->max_mana;
	golem->max_move = 100 + sspell->percent * 2;
	golem->move = golem->max_move * 0.6;
	golem->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		golem->armor[i] = interpolate(golem->level,-100,140);
	golem->armor[3] = interpolate(golem->level,-100,20);
	golem->gold = 0;
	golem->timer = 0;
	golem->damage[DICE_NUMBER] = 13;   
	golem->damage[DICE_TYPE] = 9;
	golem->damage[DICE_BONUS] = sspell->ch->level / 2 + sspell->percent / 8;
	golem->hitroll = sspell->percent;
	golem->damroll = sspell->percent;

	golem->carry_number = 1000; /* don't get and wear obj's */

	char_puts("You created an Adamantite golem!\n", sspell->ch);
	act("$n creates an Adamantite golem!", sspell->ch,NULL,NULL,TO_ROOM);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	golem->master = golem->leader = sspell->ch;
	char_to_room(golem, sspell->ch->in_room);
	return TRUE;
}

bool spell_sanctify_lands(const spell_spool_t *sspell)
{
	if (number_bits(1) == 0)
	{
	  char_puts("You failed.\n", sspell->ch);
	  return TRUE;
	}

	if (IS_RAFFECTED(sspell->ch->in_room,RAFF_CURSE))
	{
	 affect_strip_room(sspell->ch->in_room,gsn_cursed_lands);
	 char_puts("The curse of the land wears off.\n", sspell->ch);
	 act("The curse of the land wears off.\n", sspell->ch,NULL,NULL,TO_ROOM);
	}
	if (IS_RAFFECTED(sspell->ch->in_room,RAFF_POISON))
	{
	 affect_strip_room(sspell->ch->in_room,gsn_deadly_venom);
	 char_puts("The land seems more healthy.\n", sspell->ch);
	 act("The land seems more healthy.\n", sspell->ch,NULL,NULL,TO_ROOM);
	}
	if (IS_RAFFECTED(sspell->ch->in_room,RAFF_SLEEP))
	{
	 char_puts("The land wake up from mysterious dream.\n", sspell->ch);
	 act("The land wake up from mysterious dream.\n", sspell->ch,NULL,NULL,TO_ROOM);
	 affect_strip_room(sspell->ch->in_room,gsn_mysterious_dream);
	}
	if (IS_RAFFECTED(sspell->ch->in_room,RAFF_PLAGUE))
	{
	 char_puts("The disease of the land has been treated.\n", sspell->ch);
	 act("The disease of the land has been treated.\n", sspell->ch,NULL,NULL,TO_ROOM);
	 affect_strip_room(sspell->ch->in_room,gsn_black_death);
	}
	if (IS_RAFFECTED(sspell->ch->in_room,RAFF_SLOW))
	{
	 char_puts("The lethargic mist dissolves.\n", sspell->ch);
	 act("The lethargic mist dissolves.\n", sspell->ch,NULL,NULL,TO_ROOM);
	 affect_strip_room(sspell->ch->in_room,gsn_lethargic_mist);
	}
	return TRUE;
}


bool spell_deadly_venom(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (IS_SET(sspell->ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",  sspell->ch);
	  return FALSE;
	}
	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	 char_puts("This room has already been effected by deadly venom.\n", sspell->ch);
	 return FALSE;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_POISON;
	affect_to_room(sspell->ch->in_room, &af);

	char_puts("The room starts to be filled by poison.\n", sspell->ch);   
	act("The room starts to be filled by poison.\n", sspell->ch,NULL,NULL,TO_ROOM);
	if (number_bits(2) == 0)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_cursed_lands(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (IS_SET(sspell->ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n", sspell->ch);
	  return FALSE;
	}
	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	 char_puts("This room has already been cursed.\n", sspell->ch);
	 return FALSE;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_CURSE;
	affect_to_room(sspell->ch->in_room, &af);

	char_puts("The gods has forsaken the room.\n", sspell->ch);   
	act("The gos has forsaken the room.\n", sspell->ch,NULL,NULL,TO_ROOM);
	if (number_bits(2) == 0)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_lethargic_mist(const spell_spool_t *sspell)
{
	 AFFECT_DATA af;

	if (IS_SET(sspell->ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",  sspell->ch);
	  return FALSE;
	}
	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	 char_puts("This room has already been full of lethargic mist.\n", sspell->ch);
	 return FALSE;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_SLOW;
	affect_to_room(sspell->ch->in_room, &af);

	char_puts("The air in the room makes you slowing down.\n", sspell->ch);   
	act("The air in the room makes you slowing down.\n", sspell->ch,NULL,NULL,TO_ROOM);
	if (number_bits(2) == 0)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_black_death(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (IS_SET(sspell->ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n", sspell->ch);
	  return FALSE;
	}
	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	 char_puts("This room has already been diseased.\n", sspell->ch);
	 return FALSE; 
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_PLAGUE;
	affect_to_room(sspell->ch->in_room, &af);

	char_puts("The room starts to be filled by disease.\n", sspell->ch);   
	act("The room starts to be filled by disease.\n", sspell->ch,NULL,NULL,TO_ROOM);
	if (number_bits(2) == 0)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_mysterious_dream(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (IS_SET(sspell->ch->in_room->room_flags, ROOM_LAW))
	{
	  char_puts("This room is protected by gods.\n",  sspell->ch);
	  return FALSE;
	}
	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	 char_puts("This room has already been affected by sleep gas.\n", sspell->ch);
	 return FALSE;
	}

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 15;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_SLEEP;
	affect_to_room(sspell->ch->in_room, &af);

	char_puts("The room starts to be seen good place to sleep.\n", sspell->ch);   
	act("The room starts to be seen good place to you.\n", sspell->ch,NULL,NULL,TO_ROOM);
	return TRUE;
}

bool spell_polymorph(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	int race;
	race_t *r;

	if (is_affected(sspell->ch, sspell->sn)) {
		char_puts("You are already polymorphed.\n", sspell->ch); 
		return FALSE;
	}

	if (sspell->arg == NULL || sspell->arg[0]=='\0') {
		char_puts("Usage: cast 'polymorph' <pcracename>.\n", sspell->ch); 
		return FALSE;
	}

	race = rn_lookup(sspell->arg);
	r = RACE(race);
	if (!r->pcdata) {
		char_puts("That is not a valid race to polymorph.\n", sspell->ch); 
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= sspell->level/10;
	af.location	= APPLY_RACE;
	af.modifier	= race;
	af.bitvector	= 0;
	affect_to_char(sspell->ch, &af);

	act("$n polymorphes $mself to $t.", sspell->ch, r->name, NULL, TO_ROOM);
	act("You polymorph yourself to $t.", sspell->ch, r->name, NULL, TO_CHAR);
	return TRUE;
}

bool spell_blade_barrier(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	act("Many sharp blades appear around $n and crash $N.",
	sspell->ch,NULL,victim,TO_ROOM);
	act("Many sharp blades appear around you and crash $N.",
	sspell->ch,NULL,victim,TO_CHAR);
	act("Many sharp blades appear around $n and crash you!",
	sspell->ch,NULL,victim,TO_VICT);

	dam = dice( sspell->level,5);
	if (saves_spell(sspell->ch, sspell->level,victim,DAM_PIERCE, lookup_spell_msc(sspell->sn)))
		dam /= 3;
	damage(sspell->ch,victim,dam, sspell->sn,DAM_PIERCE,TRUE);
	act("The blade barriers crash $n!",victim,NULL,NULL,TO_ROOM);

	dam = dice( sspell->level,4);
	if (saves_spell(sspell->ch, sspell->level,victim,DAM_PIERCE, lookup_spell_msc(sspell->sn)))
		dam /= 3;
	else
		WAIT_STATE(victim, PULSE_VIOLENCE / 2);
	damage(sspell->ch,victim,dam, sspell->sn,DAM_PIERCE,TRUE);
	act("The blade barriers crash you!",victim,NULL,NULL,TO_CHAR);
	if (number_percent() < 75)
		return TRUE;

	act("The blade barriers crash $n!",victim,NULL,NULL,TO_ROOM);
	dam = dice( sspell->level,2);
	if (saves_spell(sspell->ch, sspell->level,victim,DAM_PIERCE, lookup_spell_msc(sspell->sn)))
		dam /= 3;
	else
		WAIT_STATE(victim, PULSE_VIOLENCE);
	damage(sspell->ch,victim,dam, sspell->sn,DAM_PIERCE,TRUE);
	act("The blade barriers crash you!",victim,NULL,NULL,TO_CHAR);
	if (number_percent() < 50)
		return TRUE;

	act("The blade barriers crash $n!",victim,NULL,NULL,TO_ROOM);
	dam = dice( sspell->level,3);
	if (saves_spell(sspell->ch, sspell->level,victim,DAM_PIERCE, lookup_spell_msc(sspell->sn)))
		dam /= 3;
	else
		WAIT_STATE(victim, PULSE_VIOLENCE * 2);
	damage(sspell->ch,victim,dam, sspell->sn,DAM_PIERCE,TRUE);
	act("The blade barriers crash you!",victim,NULL,NULL,TO_CHAR);
	return TRUE;
}

bool spell_protection_negative (const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (!is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You are now immune to negative attacks.\n", sspell->ch);

	  af.where = TO_IMMUNE;
	  af.type = sspell->sn;
	  af.duration = sspell->level / 4;
	  af.level = sspell->ch->level;
	  af.bitvector = FORCE_DEATH;
	  af.location = 0;
	  af.modifier = 0;
	  affect_to_char(sspell->ch, &af);
	  return TRUE;
	}
	else 
	  char_puts("You are already immune to negative attacks.\n", sspell->ch);
	 return FALSE;
}


bool spell_ruler_aura(const spell_spool_t *sspell)
{
	AFFECT_DATA af;

	if (!is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You now feel more self confident in rulership.\n", sspell->ch);

	  af.where = TO_IMMUNE;
	  af.type = sspell->sn;
	  af.duration = sspell->level / 4;
	  af.level = sspell->ch->level;
	  af.bitvector = MAGIC_MENTAL;
	  af.location = 0;
	  af.modifier = 0;
	  affect_to_char(sspell->ch, &af);
	  return TRUE;
	}
	else 
	  char_puts("You are as much self confident as you can.\n", sspell->ch);
	return FALSE;
}

bool spell_evil_spirit(const spell_spool_t *sspell)
{
 AREA_DATA *pArea = sspell->ch->in_room->area;
 ROOM_INDEX_DATA *room;
 AFFECT_DATA af,af2;
 int i;

 if (IS_RAFFECTED(sspell->ch->in_room, RAFF_ESPIRIT)
	|| is_affected_room(sspell->ch->in_room, sspell->sn))
	{
	 char_puts("The zone is already full of evil spirit.\n", sspell->ch);
	 return FALSE;
	}

 if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("Your power of evil spirit is less for you, now.\n", sspell->ch);
	  return FALSE;
	}

	if (IS_SET(sspell->ch->in_room->room_flags, ROOM_LAW)) {
		char_puts("Holy aura in this room prevents your powers to work on it.\n", sspell->ch);
		return FALSE;
	}

	af2.where	= TO_AFFECTS;
	af2.type	= sspell->sn;
	af2.level	= sspell->ch->level;
	af2.duration	= sspell->level / 5;
	af2.modifier	= 0;
	af2.location	= APPLY_NONE;
	af2.bitvector	= 0;
	affect_to_char(sspell->ch, &af2);

	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 25;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_ESPIRIT;

	for (i=pArea->min_vnum; i<pArea->max_vnum; i++)  
	{
	 if ((room = get_room_index(i)) == NULL) continue;
	 affect_to_room(room, &af);
	 if (room->people) 
	act("The zone is starts to be filled with evil spirit.",room->people,NULL,NULL,TO_ALL);
	}
	return TRUE;
}

bool spell_disgrace(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (!is_affected(victim, sspell->sn) && !saves_spell(sspell->ch, sspell->level, victim, DAM_MENTAL, lookup_spell_msc(sspell->sn)))
	{
	  af.where		    = TO_AFFECTS;
	  af.type               = sspell->sn;
	  af.level              = sspell->level; 
	  af.duration           = sspell->level;
	  af.location           = APPLY_CHA;
	  af.modifier           = - (5 + sspell->level / 10);
	  af.bitvector          = 0;
	  affect_to_char(victim,&af);

	  act("$N feels $M less confident!", sspell->ch,NULL,victim,TO_ALL);
	  return TRUE;
	}
	else char_puts("You failed.\n", sspell->ch);
	return FALSE;
}

bool spell_control_undead(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if  (!IS_NPC(victim) || !IS_SET(victim->pIndexData->act, ACT_UNDEAD)) {
		act("$N doesn't seem to be an undead.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	make_spell(spell_charm_person, sspell->sn, sspell->level, sspell->ch,
			sspell->vo, sspell->target, sspell->percent);
	return TRUE;
}

bool spell_assist(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn))
	  {
	char_puts("This power is used too recently.\n", sspell->ch);
	return FALSE;
	  }

	af.where	 = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 50;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(sspell->ch, &af);

	victim->hit += 100 + sspell->level * 5;
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	act("$n looks better.", victim, NULL, NULL, TO_ROOM);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}  
	
bool spell_life_stream(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn))
	  {
	char_puts("This power is used too recently.\n", sspell->ch);
	return FALSE;
	  }

	af.where	 = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 50;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = 0;
	affect_to_char(sspell->ch, &af);

	victim->hit += sspell->level * 5;
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	act("$n looks better.", victim, NULL, NULL, TO_ROOM);
	if (sspell->ch != victim) {
		char_puts("Ok.\n", sspell->ch);
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}  
	
bool spell_summon_shadow(const spell_spool_t *sspell)	
{
	CHAR_DATA *gch;
	CHAR_DATA *shadow;
	AFFECT_DATA af;
	int i;

	if (is_affected(sspell->ch, sspell->sn))
	{
	  char_puts("You lack the power to summon another shadow right now.\n",
		   sspell->ch);
	  return FALSE;
	}

	char_puts("You attempt to summon a shadow.\n", sspell->ch);
	act("$n attempts to summon a shadow.", sspell->ch,NULL,NULL,TO_ROOM);

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == sspell->ch
		&&  gch->pIndexData->vnum == MOB_VNUM_SUM_SHADOW) {
			char_puts("Two shadows are more than you "
				  "can control!\n", sspell->ch);
			return FALSE;
		}
	}

	shadow = create_mob(get_mob_index(MOB_VNUM_SUM_SHADOW));

	for (i=0;i < MAX_STATS; i++)
	{
	  shadow->perm_stat[i] = sspell->ch->perm_stat[i];
	}

	shadow->max_hit = IS_NPC(sspell->ch)? URANGE(sspell->ch->max_hit,1 * sspell->ch->max_hit,30000)
		: URANGE(sspell->ch->pcdata->perm_hit, sspell->ch->hit,30000);
	shadow->hit = shadow->max_hit;
	shadow->max_mana = IS_NPC(sspell->ch)? sspell->ch->max_mana : sspell->ch->pcdata->perm_mana;
	shadow->mana = shadow->max_mana;
	shadow->level = sspell->ch->level;
	for (i=0; i < 3; i++)
		shadow->armor[i] = interpolate(shadow->level,-100,100);
	shadow->armor[3] = interpolate(shadow->level,-100,0);
	shadow->gold = 0;
	shadow->timer = 0;
	shadow->damage[DICE_NUMBER] = number_range(sspell->level /15, sspell->level/10);   
	shadow->damage[DICE_TYPE] = number_range(sspell->level /3, sspell->level/2);
	shadow->damage[DICE_BONUS] = number_range(sspell->level /8, sspell->level/6);

	act("A shadow conjures!", sspell->ch,NULL,NULL,TO_ALL);

	af.where		= TO_AFFECTS;
	af.type               = sspell->sn;
	af.level              = sspell->level; 
	af.duration           = 24;
	af.bitvector          = 0;
	af.modifier           = 0;
	af.location           = APPLY_NONE;
	affect_to_char(sspell->ch, &af);  

	shadow->master = shadow->leader = sspell->ch;
	char_to_room(shadow, sspell->ch->in_room);
	return TRUE;
}

bool spell_farsight(const spell_spool_t *sspell)
{
	ROOM_INDEX_DATA *room;

	if ((room = check_place(sspell->ch, sspell->arg)) == NULL) {
		char_puts("You cannot see that much far.\n", sspell->ch);
		return FALSE;
	}

	if (sspell->ch->in_room == room)
		do_look(sspell->ch, "auto");
	else 
		look_at(sspell->ch, room);
	return TRUE;
}

bool spell_remove_fear(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (check_dispel( sspell->level,victim,gsn_fear))
	{
		char_puts("You feel more brave.\n",victim);
		act("$n looks more conscious.",victim,NULL,NULL,TO_ROOM);
		if (sspell->ch != victim)
			change_faith(sspell->ch, RELF_BENEDICTION_OTHER, 0);
		return TRUE;
	}
	else char_puts("You failed.\n", sspell->ch);
	return FALSE;
}

bool spell_desert_fist(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	if ((sspell->ch->in_room->sector_type != SECT_HILLS)
	&&  (sspell->ch->in_room->sector_type != SECT_MOUNTAIN)
	&&  (sspell->ch->in_room->sector_type != SECT_DESERT)) {
		char_puts("You don't find any sand here to create a fist.\n",
			  sspell->ch);
		sspell->ch->wait = 0;
		return FALSE;
	}

	act("An existing parcel of sand rises up and forms a fist and pummels $n.",
	    victim, NULL, NULL, TO_ROOM);
	act("An existing parcel of sand rises up and forms a fist and pummels you.",
	    victim, NULL, NULL, TO_CHAR);
	dam = dice( sspell->level, 14);
	sand_effect(victim, sspell->level, dam, TARGET_CHAR);
	damage(sspell->ch, victim, dam, sspell->sn, DAM_OTHER, TRUE);
	return TRUE;
}

bool spell_mirror(const spell_spool_t *sspell)	
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	int mirrors, new_mirrors;
	CHAR_DATA *gch;
	CHAR_DATA *tmp_vict;
	int order;

	if (IS_NPC(victim)) {
		send_to_char("Only players can be mirrored.\n", sspell->ch);
		return FALSE;
	}

	for (mirrors = 0, gch = npc_list; gch; gch = gch->next)
		if (is_affected(gch, gsn_mirror)
		&&  is_affected(gch, gsn_doppelganger)
		&&  gch->doppel == victim)
			mirrors++;

	if (mirrors >= sspell->level/5) {
		if (sspell->ch == victim) 
			char_puts("You cannot be further mirrored.\n", sspell->ch);
		else
			act("$N cannot be further mirrored.",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.level	= sspell->level;
	af.modifier	= 0;
	af.location	= 0;
	af.bitvector	= 0;

	for (tmp_vict = victim; is_affected(tmp_vict, gsn_doppelganger);
	     tmp_vict = tmp_vict->doppel);

	order = number_range(0, sspell->level/5 - mirrors);

	for (new_mirrors = 0; mirrors + new_mirrors < sspell->level/5; new_mirrors++) {
		gch = create_mob(get_mob_index(MOB_VNUM_MIRROR_IMAGE));
		free_string(gch->name);
		mlstr_free(gch->short_descr);
		mlstr_free(gch->long_descr);
		mlstr_free(gch->description);
		gch->name = str_qdup(tmp_vict->name);
		gch->short_descr = mlstr_dup(tmp_vict->short_descr);
		gch->long_descr = mlstr_printf(gch->pIndexData->long_descr,
					       tmp_vict->name,
					       tmp_vict->pcdata->title);
		gch->description = mlstr_dup(tmp_vict->description);
		gch->sex = tmp_vict->sex;
    
		af.type = gsn_doppelganger;
		af.duration = sspell->level;
		affect_to_char(gch, &af);

		af.type = gsn_mirror;
		af.duration = -1;
		affect_to_char(gch,&af);

		gch->max_hit = gch->hit = 1;
		gch->level = 1;
		gch->doppel = victim;
		gch->master = victim;

		if (sspell->ch == victim) {
			char_puts("A mirror image of yourself appears beside you!\n",
				  sspell->ch);
			act("A mirror image of $n appears beside $M!",
			    sspell->ch, NULL, victim, TO_ROOM);
		}
		else {
			act("A mirror of $N appears beside $M!",
			    sspell->ch, NULL, victim, TO_CHAR);
			act("A mirror of $N appears beside $M!",
			    sspell->ch,NULL,victim,TO_NOTVICT);
			char_puts("A mirror image of yourself appears beside you!\n",
				  victim);
		}

		char_to_room(gch, victim->in_room);
		if (new_mirrors == order) {
			char_from_room(victim);
			char_to_room(victim, gch->in_room);
			if (JUST_KILLED(victim))
				break;
		}
	}
	return TRUE;
}    
 
bool spell_doppelganger(const spell_spool_t *sspell)	
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (sspell->ch == victim || (is_affected(sspell->ch, sspell->sn) && sspell->ch->doppel == victim)) {
		act("You already look like $M.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (IS_NPC(victim)) {
		act("$N is too different from yourself to mimic.",
		    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (IS_IMMORTAL(victim)) {
		char_puts("Yeah, sure. And I'm the Pope.\n", sspell->ch);
		return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level, victim, DAM_CHARM, lookup_spell_msc(sspell->sn))) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	act("You change form to look like $N.", sspell->ch, NULL, victim, TO_CHAR);
	act("$n changes form to look like YOU!", sspell->ch, NULL, victim, TO_VICT);
	act("$n changes form to look like $N!", sspell->ch, NULL, victim, TO_NOTVICT);

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level; 
	af.duration	= 2 * sspell->level / 3;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= 0;

	affect_to_char(sspell->ch, &af); 
	sspell->ch->doppel = victim;
	return TRUE;
}
 
bool spell_hunger_weapon(const spell_spool_t *sspell)
{
		OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
		AFFECT_DATA af;
		int chance = sspell->percent;

	if (obj->pIndexData->item_type != ITEM_WEAPON) {
        	char_puts("That's not a weapon.\n", sspell->ch);
        	return FALSE;
        } 

        if (obj->wear_loc != WEAR_NONE) {
        	char_puts("The item must be carried to be cursed.\n", sspell->ch);
        	return FALSE;
        } 

	if (IS_WEAPON_STAT(obj, WEAPON_HOLY)
	||  IS_OBJ_STAT(obj, ITEM_BLESS)
	||  IS_OBJ_STAT(obj, ITEM_ANTI_EVIL)) {
		act("The gods are infuriated!", sspell->ch, NULL, NULL, TO_ALL);
		damage(sspell->ch, sspell->ch, (sspell->ch->hit - 1) > 1000 ? 1000 : (sspell->ch->hit - 1),
		       TYPE_HIT, DAM_HOLY, TRUE);
		return TRUE;
	} 

        if (IS_WEAPON_STAT(obj, WEAPON_VAMPIRIC)) {
        	act("$p is already hungry for enemy life.",
		    sspell->ch, obj, NULL, TO_CHAR);
        	return FALSE;
	}

	if (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD))
		chance *= 2;

	if (IS_WEAPON_STAT(obj, WEAPON_FLAMING))	chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_FROST))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_SHARP))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_VORPAL))		chance /= 2;
	if (IS_WEAPON_STAT(obj, WEAPON_SHOCKING))	chance /= 2;

	if (number_percent() < chance) {
		affect_enchant(obj);

		af.where	= TO_WEAPON;
		af.type 	= sspell->sn;
		af.level	= sspell->level;
		af.duration	= sspell->level/8;
		af.location	= 0;
		af.modifier	= 0;
		af.bitvector	= WEAPON_VAMPIRIC;
		affect_to_obj(obj, &af);

		if (!IS_WEAPON_STAT(obj, WEAPON_SHARP)
		&& number_percent() < chance * 2 / 3) {
			SET_BIT(obj->value[4], WEAPON_SHARP);
			act_puts("You make $p sharpes!",
				sspell->ch, obj, NULL, TO_CHAR, POS_RESTING);
		}

		SET_BIT(obj->extra_flags, ITEM_ANTI_GOOD | ITEM_ANTI_NEUTRAL);
		act("You transmit part of your hunger to $p.",
		    sspell->ch, obj, NULL, TO_CHAR);
	} 
	else 
		act("You failed.", sspell->ch, obj, NULL, TO_ALL);
	return TRUE;
}
