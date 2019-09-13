/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: magic.c,v 1.34 2007/11/21 10:25:47 rem Exp $
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
#include "update.h"
#include "fight.h"
#include "quest.h"

DECLARE_DO_FUN(do_yell		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_stand		);

extern int gsn_anathema;

flag32_t lookup_spell_msc(int sn)
{
	skill_t *sk = SKILL(sn);
	
	if (!sk || !sk->spell)
		return 0;
	return sk->spell->mschool;
}

/*
 * for casting different rooms 
 * returned value is the range 
 */
int allowed_other(CHAR_DATA *ch, int sn)
{
	if (IS_SET(SKILL(sn)->flags, SKILL_RANGE))
		return ch->level / 20 + 1;
	return 0;
}

/*
 * check_trust - check if victim allow ch to cast SPELL_QUESTIONABLE spell
 */
bool check_trust(CHAR_DATA *ch, CHAR_DATA *victim)
{
	if (ch == victim)
		return TRUE;

	if (IS_NPC(victim) && ch)
	{
		if (victim->mount == ch)
			return TRUE;
		else
			return is_same_group(ch, victim);
	}

	if (IS_ATTACKER(victim) && !IS_ATTACKER(ch))
		return FALSE;

	if (IS_SET(victim->pcdata->trust, TRUST_ALL))
		return TRUE;

	return (IS_SET(victim->pcdata->trust, TRUST_GROUP) &&
		is_same_group(ch, victim))
		/*|| (ch->clan && IS_SET(victim->pcdata->trust, TRUST_CLAN) &&
		ch->clan == victim->clan )*/;
}

/*
 * spellbane - check if 'bch' deflects the spell of 'ch'
 */
bool spellbane(CHAR_DATA *bch, CHAR_DATA *ch, int bane_chance, int bane_damage)
{
	if (HAS_SKILL(bch, gsn_spellbane)
	&&  number_percent() < bane_chance) {
		if (ch == bch) {
	        	act_puts("Your spellbane deflects the spell!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			act("$n's spellbane deflects the spell!",
			    ch, NULL, NULL, TO_ROOM);
			damage(ch, ch, bane_damage, gsn_spellbane,
			       DAM_NEGATIVE, TRUE);
		}
	        else {
			act_puts("$N deflects your spell!",
				 ch, NULL, bch, TO_CHAR, POS_DEAD);
			act("You deflect $n's spell!",
			    ch, NULL, bch, TO_VICT);
			act("$N deflects $n's spell!",
			    ch, NULL, bch, TO_NOTVICT);
			if (!is_safe(bch, ch))
				damage(bch, ch, bane_damage, gsn_spellbane,
				       DAM_NEGATIVE, TRUE);
			check_improve(bch, gsn_spellbane, TRUE, 8);
	        }
	        return TRUE;
	}

	return FALSE;
}

int add_slevel_mschool(CHAR_DATA *ch, flag32_t msc, bool check_improve)
{
	int i;
	int slevel;
	int count;

	if (!IS_PC(ch) || !msc)
		return 0;

	slevel = count = 0;

	for(i = 1; msc && i <= QUANTITY_MAGIC + 1; i++)
		if (IS_SET(msc, magic_schools[i].bit_force))
		{
			switch(get_magic_rank(ch, i))
			{
				case 2:	slevel++;
					break;
				case 3:	slevel += 3;
					break;
			}
			count++;
			REMOVE_BIT(msc, magic_schools[i].bit_force);
			if (check_improve)
				get_char_msc(ch, i, 8);
		}
	if (count)
		return slevel / count;
	else
		return 0;
}

int check_slevel_staff(CHAR_DATA *ch, int sn, int loc)
{
	OBJ_DATA	*staff;
	skill_t		*sk;

	if ((staff = get_eq_char(ch, loc))
	&& staff->pIndexData->item_type == ITEM_STAFF
	&& (sk = skill_lookup(sn)))
	{
		if (staff->value[3] == sn)
			return number_range(2, 2 + staff->value[0] / 25);
		else
			return number_range(1, 1 + staff->level / 30);
	}

	return 0;
}

//#define		GSTF_CAST_FAR		(A)
//#define		GSTF_MISSING_TARGET	(B)
//#define		GSTF_OFFENSIVE		(C)

int get_spell_target(CHAR_DATA *ch, int sn, void **vo, bool message,
		flag32_t *flags, int *door, const char *arg)
{
	skill_t		*sk = SKILL(sn);
	struct spell_pt	*spell = sk->spell;
	int		target = TARGET_NONE;
	CHAR_DATA	*victim = NULL;
	int		range;
	OBJ_DATA	*obj = NULL;

	*vo = NULL;
	switch (spell->target)
	{
		default:
			bug("get_spell_target: bad target for gsn %d", *(sk->pgsn));
			return -1;
		case TAR_IGNORE:
			// bch = ch;
			break;
		case TAR_CHAR_OFFENSIVE:
			if (arg[0] == '\0') {
				if ((victim = ch->fighting) == NULL)
				{
					if (message)
						char_puts("Cast the spell on whom?\n",
							ch);
					return -1;
				}
			} else if ((range = allowed_other(ch, sn)) > 0) {
				if ((victim = get_char_spell(ch, arg,
					door, range)) == NULL)
				{
					// SET_BIT(*flags, SPT_MISSING_TARGET);
					return -1;
				}
				if (victim->in_room != ch->in_room)
				{
					SET_BIT(*flags, SPT_CAST_FAR);
					if (IS_NPC(victim))
					{
						if (room_is_private(ch->in_room))
						{
							if (message)
								char_puts("You can't cast this spell from private room right now.\n",
									ch);
							return -1;
						}
						if (IS_SET(victim->pIndexData->act,
								ACT_NOTRACK))
						{
							if (message)
								act_puts("You can't cast this spell to $N at this distance.",
									ch, NULL, victim, TO_CHAR, POS_DEAD);
							return -1;
						}
					}
				}
			} else if ((victim = get_char_room(ch, arg)) == NULL)
			{
				// SET_BIT(*flags, SPT_MISSING_TARGET);
				if (message)
					char_puts("They aren't here.\n", ch);
				return -1;
			}
	
			target = TARGET_CHAR;
			// bch = victim;
			// bane_chance = 2*get_skill(bch, gsn_spellbane)/3;
			break;
		case TAR_CHAR_DEFENSIVE:
			if (arg[0] == '\0')
				victim = ch;
			else if ((victim = get_char_room(ch, arg)) == NULL) {
				if (message)
					char_puts("They aren't here.\n", ch);
				return -1;
			}
			target = TARGET_CHAR;
			// bch = victim;
			break;
		case TAR_CHAR_SELF:
			if (arg[0] == '\0')
				victim = ch;
			else if ((victim = get_char_room(ch, arg)) == NULL
			|| victim != ch) {
				if (message)
					char_puts("You cannot cast this spell "
						"on another.\n", ch);
				return -1;
			}
			target = TARGET_CHAR;
			// bch = victim;
			break;
		case TAR_OBJ_INV:
			if (arg[0] == '\0') {
				if (message)
					char_puts("What should the spell be cast upon?\n",
							ch);
				return -1;
			}
			if ((obj = get_obj_carry(ch, arg)) == NULL) {
				if (message)
					char_puts("You are not carrying that.\n", ch);
				return -1;
			}
			target = TARGET_OBJ;
			// bch = ch;
			break;
		case TAR_OBJ_CHAR_OFF:
			if (arg[0] == '\0') {
				if ((victim = ch->fighting) == NULL) {
					// SET_BIT(*flags, SPT_MISSING_TARGET);
					if (message)
						char_puts("Cast the spell on whom or what?\n",
							ch);
					return -1;
				}
				target = TARGET_CHAR;
			} else if ((victim = get_char_room(ch, arg)))
				target = TARGET_CHAR;
			else if ((obj = get_obj_here(ch, arg)))
				target = TARGET_OBJ;
			else {
				if (message)
					char_puts("You don't see that here.\n",ch);
				// SET_BIT(*flags, SPT_MISSING_TARGET);
				return -1;
			}
			// bch = victim;
			break;
		case TAR_OBJ_CHAR_DEF:
			if (arg[0] == '\0')
			{
				victim = ch;
				target = TARGET_CHAR;
			} else if ((victim = get_char_room(ch, arg)))
				target = TARGET_CHAR;
			else if ((obj = get_obj_carry(ch, arg)))
				target = TARGET_OBJ;
			else {
				if (message)
					char_puts("You don't see that here.\n",ch);
				return -1;
			}
			// bch = victim;
			break;
	}

	switch (target) {
	    case TARGET_CHAR:
		*vo = (void*) victim;
		switch (spell->target) {
		    case TAR_CHAR_DEFENSIVE:
		    case TAR_OBJ_CHAR_DEF:
			if (IS_ATTACKER(victim) && !IS_ATTACKER(ch))
			{
				if (message)
					char_puts("You can't cast defensive spell on attacker.\n", ch);
				return -1;
			}
			if (IS_SET(sk->flags, SKILL_QUESTIONABLE)
			&&  !check_trust(ch, victim))
			{
				if (message)
					char_puts("They do not trust you enough "
						"for this spell.\n", ch);
				return -1;
			}
			break;
		    case TAR_CHAR_OFFENSIVE:
		    case TAR_OBJ_CHAR_OFF:
			if (IS_SET(sk->flags, SKILL_QUESTIONABLE))
				SET_BIT(*flags, check_trust(ch, victim) ?
						0 : SPT_OFFENSIVE);
			else
				SET_BIT(*flags, SPT_OFFENSIVE);

			if (SET_BIT(*flags, SPT_OFFENSIVE)
			&& is_safe(ch, victim))
				return -1;
			break;
		}
		break;
	    case TARGET_OBJ:
		*vo = (void*) obj;
		break;
	}
	
	return target;
}

/*
 *	return TRUE if component finded or not need.
 */
 
bool get_need_component(CHAR_DATA *ch, struct spell_component *comp, bool after, bool success, OBJ_DATA **obj)
{
	OBJ_INDEX_DATA *pObj;

	*obj = NULL;

	if (after)
	{
		if (!IS_SET(comp->flags, SPC_CHECK_AFTER_DELAY))
			return TRUE;
	} else {
		if (IS_SET(comp->flags, SPC_CHECK_AFTER_DELAY))
			return TRUE;
	}

	if (success && !IS_SET(comp->flags, SPC_SUCCESSFULL))
		return TRUE;

	if ((pObj = get_obj_index(comp->vnum)) == NULL)
		return TRUE;

	if (IS_SET(comp->flags, SPC_NEED_EQ))
	{
		*obj = get_obj_type_wear(pObj, ch, FALSE);
	} else if (IS_SET(comp->flags, SPC_NEED_GROUND))
	{
		*obj = get_obj_type_in_room_ch(pObj, ch, FALSE);
	} else {
		/* inventory */
		*obj = get_obj_type_carry(pObj, ch, FALSE);
	}

	if (*obj == NULL
	|| (!IS_SET(comp->flags, SPC_NOTNEED_SEE) && !can_see_obj(ch, *obj)))
	{
		*obj = NULL;
		//count++;
		if (IS_SET(comp->flags, SPC_SUFFICIENT))
			return TRUE;
		else
			return FALSE;
	}
	
	return TRUE;
}

bool check_spell_components(CHAR_DATA *ch, varr *components, bool after, bool success)
{
	struct spell_component *comp;
	int i, count = 0;
	OBJ_INDEX_DATA *pObj;
	OBJ_DATA *obj;

	for (i = 0; i < components->nused; i++)
	{
		comp = (struct spell_component *) VARR_GET(components, i);
		
		if (get_need_component(ch, comp, after, success, &obj))
		{
			if (obj
			&&(!IS_SET(comp->flags, SPC_SUCCESSFULL) || success)
			&& IS_SET(comp->flags, SPC_SUFFICIENT))
			{
				if (IS_SET(comp->flags, SPC_SPEND))
				{
					act("$p flares brightly and vanishes!", ch, obj, NULL, TO_CHAR);
					extract_obj(obj, 0);
				} else
					act("You draw upon the power of $p.", ch, obj, NULL, TO_CHAR);
				return TRUE;
			}
		} else {
			count++;
			break;
		}
	}

	if (count == 0)
	{
		for (i = 0; i < components->nused; i++)	// spend all needed components
		{
			comp = (struct spell_component *) VARR_GET(components, i);

			if (!get_need_component(ch, comp, after, success, &obj)
			|| obj == NULL)
				continue;

			if ((!IS_SET(comp->flags, SPC_SUCCESSFULL) || success)
			&& !IS_SET(comp->flags, SPC_SUFFICIENT))
			{
				if (IS_SET(comp->flags, SPC_SPEND))
				{
					act("$p flares brightly and vanishes!", ch, obj, NULL, TO_CHAR);
					extract_obj(obj, 0);
				} else
					act("You draw upon the power of $p.", ch, obj, NULL, TO_CHAR);
			}
		}
		return TRUE;
	}

	count = 0;
	for (i = 0; i < components->nused; i++)
	{
		comp = (struct spell_component *) VARR_GET(components, i);
		if (!IS_SET(comp->flags, SPC_SHOW_HELP))
			continue;
		if (count++ == 0)
			char_puts("For cast this spell you need in next component(s):\n", ch);
		if ((pObj = get_obj_index(comp->vnum)) == NULL)
			continue;
		char_printf(ch, "%s %s%s.\n",
			mlstr_cval(pObj->short_descr, ch),
			IS_SET(comp->flags, SPC_NEED_EQ) ? "in equipment"
			: (IS_SET(comp->flags, SPC_NEED_EQ) ? "at ground near"
			: "in inventory"),
			IS_SET(comp->flags, SPC_SUFFICIENT) ? " [sufficient]" : str_empty );
	}

	if (count == 0 && components->nused > 0)
		char_puts("Cast be in need of something.", ch);

	return FALSE;
}

extern int debug_level;
bool do_cast_done(spell_spool_t *sspell)
{
	CHAR_DATA *ch;
//	char arg1[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	// OBJ_DATA *obj;
	//void *vo;
	int mana;
	int move;
	int /*sn = -1,*/ wait;
	//int target;
	// int door, range;
	int door;
	// bool cast_far = FALSE;
	//bool offensive = FALSE;
	//flag32_t	flags;
	//bool nowait = FALSE;
	//int chance = 0, chance2;
	skill_t *sk;
	class_t *cl;
	bool quest_complete = FALSE;
	CHAR_DATA *bch;		/* char to check spellbane on */
	int bane_chance;	/* spellbane chance */
	/*const char *target_name = str_empty;*/


	if ((ch = sspell->ch) == NULL
	|| ch->in_room == NULL
	|| JUST_KILLED(ch)
	|| (cl = class_lookup(ch->class)) == NULL
	|| sspell->sn <= 0 || sspell->sn >= skills.nused)
		return FALSE;
	
	if (is_affected(ch, gsn_shielding)) {
		char_puts("You reach for the True Source and feel something "
			  "stopping you.\n", ch);
		return FALSE;
	}

	/*if (sspell->percent < 2) {
		if (sspell->percent)
			char_puts("You can't use this spell yet.\n", ch);
		else
			char_puts("You don't know any spells of that name.\n", ch);
		return FALSE;
	}
	*/

	sk = SKILL(sspell->sn);
	
	if (HAS_SKILL(ch, gsn_vampire)
	&&  !is_affected(ch, gsn_vampire)
	&&  !IS_SET(sk->flags, SKILL_CLAN)) {
		char_puts("You must transform to vampire before casting!\n",
			  ch);
		return FALSE;
	}

	if ((is_affected(ch, gsn_garble) || is_affected(ch, gsn_deafen))
	&& IS_SET(sk->spell->type, SPT_VERBAL)) {
		char_puts("You can't get the right intonations.\n", ch);
		return FALSE;
	}

	if (ch->position < sk->spell->minimum_position
	&& !IS_SET(sspell->flags, SPT_NOWAIT)
	&& IS_SET(sspell->flags, SPT_CHECKPOS_AFTER)) {
		char_puts("You can't concentrate enough.\n", ch);
		return FALSE;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NOMAGIC)) {
		char_puts("Your spell fizzles out and fails.\n", ch);
		act("$n's spell fizzles out and fails.",
		    ch, NULL, NULL, TO_ROOM);
		return FALSE;
	}

	if (IS_SET(sspell->flags, SPT_MANA_AFTER))
	{
		if (IS_PC(ch))
			mana = mana_cost(ch, sspell->sn);
		else
			mana = 10 + ch->level / 10;

		if (ch->mana < mana && !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough mana.\n", ch);
			return FALSE;
		}
	} else
		mana = 0;
		
	debug_printf(9, "do_cast_done get target");
	/*
	 * Locate targets.
	 */
	if (IS_SET(sspell->flags, SPT_TARG_LOCATE))
	{
		flag32_t	flags = 0;
		void		*vo = NULL;

		door = 0;
		//if (debug_level > 4)
		//	log_printf("do_cast_done attempt get_spell_target %s (%s)",
		//		sk->name, sspell->arg);
				
		if ((sspell->target = get_spell_target(ch, sspell->sn,
			&vo, TRUE, &flags, &door,
			sspell->arg)) == -1)
		{
			return FALSE;
		}
		sspell->flags	|= flags;
		sspell->vo	= vo;
	} else {
		if (sspell->target == -1)
		{
			char_puts("Target was lost.\n", ch);
			return FALSE;
		}

		if ((door = sspell->flags & SPT_DOOR_BITS) == SPT_DOOR_BITS)
			door = -1;
	}

	bane_chance	= 100;
	if (sspell->target == TARGET_CHAR)
	{
		if ((victim = sspell->vo) == NULL)
		{
			log_printf("[bug] do_cast_done: NULL victim with TARGET_CHAR");
			return FALSE;
		}
		bch = victim;
		if (sk->spell->target == TAR_CHAR_OFFENSIVE && bch)
			bane_chance = 2*get_skill(bch, gsn_spellbane)/3;
	} else {
		victim = NULL;
		if (sk->spell->target == TAR_IGNORE)
			bch = ch;
		else
			bch = NULL;
	}

	
	if (check_spell_components(ch, &(sk->spell->components), TRUE, FALSE) == FALSE)
		return FALSE;

	if (IS_SET(sspell->flags, SPT_OFFENSIVE)
	&& victim
	&& IS_PC(ch)
	&& IS_PC(victim)
	&& victim != ch
	&& ch->fighting != victim
	&& victim->fighting != ch
	&& !is_same_group(ch, victim))
		doprintf(do_yell, victim,
			 "Die, %s, you sorcerous dog!",
			 PERS(ch, victim));

	wait = sk->beats;
	/*
	if (IS_AFFECTED(ch, AFF_HASTE))
		wait -= wait * number_range(1,
			get_curr_stat(ch, STAT_DEX) / 7) / 8;
	if (IS_AFFECTED(ch, AFF_SLOW))
		wait *= 2;
	if (IS_SET(flags, SPT_CAST_FAR))
		wait = (wait * (5 -
			add_slevel_mschool(ch, sk->spell->mschool, FALSE))) / 2;
	*/

	WAIT_STATE(ch, wait);
	move = UMAX(sk->min_level / 10, 2);

	if (ch->move < move || number_percent() > sspell->percent) {
		debug_printf(9, "do_cast_done ch_p_false");
		char_puts("You lost your concentration.\n", ch);
		check_improve(ch, sspell->sn, FALSE, 1);
		if (mana)
			ch->mana = UMAX(0, ch->mana - mana / 2);
		if (ch->move > 0)
			ch->move = UMAX(0, ch->move - move / 2);
		if (IS_SET(sspell->flags, SPT_CAST_FAR))
			REMOVE_BIT(sspell->flags, SPT_CAST_FAR);
	} else {
		debug_printf(9, "do_cast_done ch_p_true");
		if (mana)
			ch->mana = UMAX(0, ch->mana - mana);
		ch->move -= move;

		check_spell_components(ch, &(sk->spell->components), TRUE, TRUE);

		if (bch && spellbane(bch, ch, bane_chance, 3 * bch->level))
			return TRUE;

		if (sspell->target == TARGET_CHAR
		&& IS_NPC(victim)
		&& IS_PC(ch)
		&& GET_QUEST_TYPE(ch) == QUESTT_CASTMOB
		&& victim->hunter
		&& victim->hunter == ch
		&& ch->pcdata->quest->gsn == sspell->sn) {
			victim->hunter = NULL;
			ch->pcdata->quest->gsn = 0;
			ch->pcdata->quest->target1 = NULL;
			quest_complete = TRUE;
		}

		debug_printf(9, "do_cast_done call spell_fun");	
		if (!sk->spell->spell_fun)
		{
			bug("not found spell fun [%d]", sspell->sn);
			return FALSE;
		}
		sk->spell->spell_fun(sspell);
		debug_printf(10, "do_cast_done complete call spell_fun");
		if (quest_complete)
			act_puts("You have almost completed your QUEST!\n"
			"Return to questmaster before your time "
			"runs out!",
			ch, NULL, NULL, TO_CHAR, POS_DEAD);
		check_improve(ch, sspell->sn, TRUE, 1);
	}
	
		
	if (IS_SET(sspell->flags, SPT_CAST_FAR) && door != -1)
	{
		debug_printf(10, "do_cast_done call path_to_track");
		path_to_track(ch, victim, door);
	} else if (victim && IS_SET(sspell->flags, SPT_OFFENSIVE)
	&& victim != ch && victim->master != ch
	&& ch->in_room && ch->in_room == victim->in_room
	&& victim->fighting == NULL
	&& victim->position > POS_SLEEPING) {
		debug_printf(10, "do_cast_done call multi_hit");
		multi_hit(victim, ch, TYPE_UNDEFINED);
	}
	return TRUE;
}

void spell_sadd(spell_spool_t *spell, bool dup_arg);
TIME_INFO_DATA  *calc_time(int  add_pulse);

void do_cast(CHAR_DATA *ch, const char *argument)
{
	spell_spool_t	sspell;
	char		arg1[MAX_INPUT_LENGTH];
	CHAR_DATA	*victim;
	// OBJ_DATA *obj;
	//void *vo;
	int mana;
	//int move;
	int sn = -1, wait;
	// int target;
	// int door, range;
	//int door;
	// bool cast_far = FALSE;
	//bool offensive = FALSE;
	//flag32_t	flags;
	bool nowait = FALSE;
	int slevel;
	int chance = 0 /*, chance2*/;
	skill_t *sk;
	class_t *cl;
	int faith;
	//bool quest_complete = FALSE;
	RELIGION_DATA *rel;
	//CHAR_DATA *bch;		/* char to check spellbane on */
	//int bane_chance;	/* spellbane chance */
	const char *target_name;

	if (ch->nsp_spool != -1)
	{
		char_puts("You are already cast some spell now.\n", ch);
		return;
	}

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	if (HAS_SKILL(ch, gsn_spellbane)) {
		char_puts("You are Battle Rager, not the filthy magician.\n",
			  ch);
		return;
	}

	target_name = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] == '\0') {
		char_puts("Cast which what where?\n", ch);
		return;
	}

	if (IS_NPC(ch)) {
		if (!str_cmp(arg1, "nowait") && !IS_AFFECTED(ch, AFF_CHARM)) {
			target_name = one_argument(target_name,
						   arg1, sizeof(arg1));
			if (ch->wait)
				ch->wait = 0;
			nowait = TRUE;
		}
		else if (ch->wait) 
			return;
	}
	sn = sn_lookup(arg1);

	if ((chance = get_skill(ch, sn)) < 2) {
		if (chance)
			char_puts("You can't use this spell yet.\n", ch);
		else
			char_puts("You don't know any spells of that name.\n", ch);
		return;
	}

	sk = SKILL(sn);
	
	if (HAS_SKILL(ch, gsn_vampire)
	&&  !is_affected(ch, gsn_vampire)
	&&  !IS_SET(sk->flags, SKILL_CLAN)) {
		char_puts("You must transform to vampire before casting!\n",
			  ch);
		return;
	}
	
	if (sk->spell == NULL) {
		char_puts("That's not a spell.\n", ch);
		return;
	}

	if (IS_SET(sk->spell->type, SPT_VERBAL)
	&& ch->in_room && ch->in_room->sector_type == SECT_UNDER_WATER)
	{
		char_puts("Under water ?!\n", ch);
		return;
	}

	if ((is_affected(ch, gsn_garble) || is_affected(ch, gsn_deafen))
	&& IS_SET(sk->spell->type, SPT_VERBAL)) {
		char_puts("You can't get the right intonations.\n", ch);
		return;
	}

	if (ch->position < sk->spell->minimum_position
	&& !nowait
	&& IS_SET(sk->spell->type, SPT_CHECKPOS_BEFORE)) {
		char_puts("You can't concentrate enough.\n", ch);
		return;
	}

	if (!IS_SET(sk->spell->type, SPT_MANA_AFTER))
	{
		if (IS_PC(ch))
			mana = mana_cost(ch, sn);
		else
			mana = 10 + sk->min_mana * 2;

		if (ch->mana < mana && !IS_IMMORTAL(ch)) {
			char_puts("You don't have enough mana.\n", ch);
			return;
		}
	} else
		mana = 0;

	/*
	 * Locate targets.
	 */
	victim		= NULL;
	sspell.flags	= sk->spell->type;

	if (IS_SET(sspell.flags, SPT_TARG_OBJECT))
	{
		int door = 0;

		if ((sspell.target = get_spell_target(ch, sn, &(sspell.vo),
			TRUE, &(sspell.flags), &door, target_name)) == -1)
		{
			return;
		}
		
		if (door == -1)
			SET_BIT(sspell.flags, SPT_DOOR_BITS);
		else
			SET_BIT(sspell.flags, door);
		if (sspell.target == TARGET_CHAR)
			victim = sspell.vo;
	} else {
		sspell.vo	= NULL;
		sspell.target	= -1;
	}

	if (nowait)
		SET_BIT(sspell.flags, SPT_NOWAIT);

	if (action_spell(ch, sk, victim))
		SET_BIT(sspell.flags, SPT_VICTSEEACTION);

	if (check_spell_components(ch, &(sk->spell->components), FALSE, FALSE) == FALSE)
		return;
	
	wait = sk->spell->delay;
	if (IS_AFFECTED(ch, AFF_HASTE))
		wait -= wait * number_range(1,
			get_curr_stat(ch, STAT_DEX) / 7) / 8;
	if (IS_AFFECTED(ch, AFF_SLOW))
		wait *= 2;

	if (IS_SET(sspell.flags, SPT_CAST_FAR))
	{
		wait = (wait * (5 -
			add_slevel_mschool(ch, sk->spell->mschool, FALSE))) / 2;
	}
		
	if (IS_SET(sk->flags, SKILL_CLERIC)
	&& IS_PC(ch))
	{
		if (GET_CHAR_RELIGION(ch))
		{
			if ((faith = get_char_faith(ch)) > 99)
				chance += number_range(1, faith > 1000 ? (10 + (faith - 1000) / 200) : faith / 100);
			else if (faith < -199)
				chance -= number_range(1, - faith / 200);
		} else
			chance /= 3;
	}
	
	if (MOUNTED(ch)) {
		int perc, sk;

		if ((sk = get_skill(ch, gsn_riding_fight)) &&
		  (perc = number_percent()) < sk )
		{
			chance = (chance * (170 + perc)) / 300;
			check_improve(ch, gsn_riding_fight, TRUE, 28);
		} else {
			chance /= 2;
			check_improve(ch, gsn_riding_fight, FALSE, 32);
		}
	}

	chance = UMAX(2, chance);

	// move = UMAX(sk->min_level / 10, 2);
	sspell.percent	= chance;
	if (IS_SET(cl->flags, CLASS_MAGIC))
		slevel = LEVEL(ch) - UMAX(0, (LEVEL(ch) / 20));
	else
		slevel = LEVEL(ch) - UMAX(5, (LEVEL(ch) / 10));

	if ((chance = get_skill(ch, gsn_spell_craft))) {
		if (number_percent() < chance) {
			slevel = LEVEL(ch);
			check_improve(ch, gsn_spell_craft, TRUE, 2);
		}
		else
			check_improve(ch, gsn_spell_craft, FALSE, 2);
	}


	if (IS_SET(sk->group, GROUP_MALADICTIONS))
	{
		if (IS_EVIL(ch)
		&& (chance = get_skill(ch, gsn_improved_maladiction)))
		{
			if (number_percent() < chance) {
				slevel = LEVEL(ch);
				slevel -= (ch->alignment * chance) / 12000;
				check_improve(ch, gsn_improved_maladiction,
					TRUE, 2);
			}
			else
				check_improve(ch, gsn_improved_maladiction,
					  FALSE, 2);
		}
			
		if ((rel = GET_CHAR_RELIGION(ch))
		&& IS_SET(rel->flags, RELIG_DARK_MALADICTION)
		&& (time_info.hour >= 18 || time_info.hour < 9)
		&& (chance = number_range(500, 3000))
				< get_char_faith(ch))
			slevel += 1 + chance / 900;
	}

	if (IS_SET(sk->group, GROUP_BENEDICTIONS))
	{
		if (IS_GOOD(ch)
		&& (chance = get_skill(ch, gsn_improved_benediction)))
		{
			if (number_percent() < chance) {
				slevel = LEVEL(ch);
				slevel += (ch->alignment * chance) / 10000;
				check_improve(ch, gsn_improved_benediction,
						  TRUE, 2);
			}
			else
				check_improve(ch, gsn_improved_benediction,
						  FALSE, 2);
		}
			
		if ((rel = GET_CHAR_RELIGION(ch))
		&& IS_SET(rel->flags, RELIG_CHECK_BENEDICTION)
		&& (chance = number_range(500, 10000))
				< get_char_faith(ch))
			slevel += 1 + chance / 2000;
	}

	if ((chance = get_skill(ch, gsn_mastering_spell))
	&&  number_percent() < chance) {
		slevel += number_range(1,4);
		check_improve(ch, gsn_mastering_spell, TRUE, 2);
	}
		
	if (IS_PC(ch)) {
		if (slevel >= 20)
			slevel += get_curr_stat(ch,STAT_INT) - 20;
		else
			slevel += (get_curr_stat(ch,STAT_INT) - 20) / 2;
		slevel = UMAX(1, slevel);
		if (get_curr_stat(ch, STAT_WIS) > 20)
			slevel++;
	}
		
	switch(sk->spell->target)
	{
		case TAR_CHAR_OFFENSIVE:
		case TAR_OBJ_CHAR_OFF:
			break;
		default:
			slevel += add_slevel_mschool(ch,
					sk->spell->mschool, TRUE);
	}

	if (IS_SET(sspell.flags, SPT_CAST_FAR))
		slevel = slevel - UMIN(5, slevel * 0.1);
		
	if (IS_AFFECTED(ch, AFF_WEAKEN))
		slevel *= 0.75;

	if (IS_PC(ch))
		slevel += ch->pcdata->add_spell_level;

	if (IS_SET(sk->flags, SKILL_CLERIC)
	&& IS_PC(ch))
	{
		if (GET_CHAR_RELIGION(ch))
		{
			if ((faith = get_char_faith(ch)) > 999)
			{
				int mod = number_range(1, faith / 1000);

				slevel += mod;
				if (IS_PC(ch))
					reduce_faith(ch, 1, FALSE);
			} else if (faith < -499)
					slevel -= number_range(1, - faith / 500);
		} else
			slevel /= 2;
	}
		
	/* slevel += check_slevel_staff(ch, sn, WEAR_HOLD);
	 * slevel += check_slevel_staff(ch, sn, WEAR_SECOND_WIELD);
	 */
	slevel += check_slevel_staff(ch, sn, WEAR_WIELD);

	if (slevel < 1)
		slevel = 1;

	if (mana)
	{
		ch->mana = UMAX(0, ch->mana - mana);
		//ch->move = UMAX(0, ch->move - move);   decrease after delay
	}

	sspell.sn	= sn;
	sspell.level	= slevel;
	sspell.arg	= target_name;
	sspell.ch	= ch;
	memcpy(&sspell.then_do, calc_time(wait), sizeof(TIME_INFO_DATA));

	spell_sadd(&sspell, TRUE);
	if (sk->spell->delay > 0)
		char_puts("You begin cast spell...\n", ch);
	WAIT_STATE(ch, wait);
	
	if (victim && IS_SET(sspell.flags, SPT_VICTSEEACTION)
	&& IS_SET(sspell.flags, SPT_OFFENSIVE)
	&& !IS_SET(sspell.flags, SPT_CAST_FAR)
	&& victim != ch && victim->master != ch
	&& number_percent() < (get_curr_stat(victim, STAT_INT)
		- get_curr_stat(ch, STAT_CHA) / 2)
	) {
		if (ch->in_room
		&& ch->in_room == victim->in_room
		&& victim->fighting == NULL
		&& victim->position > POS_SLEEPING) {
			doprintf(interpret, victim,
				"say And what are you doing, %s ?!",
				mlstr_mval(ch->short_descr));
			multi_hit(victim, ch, TYPE_UNDEFINED);
	}

/*		"Stupid coders !!! :)  see on top, that need"  [by Xor]
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;
			if (victim == vch && victim->fighting == NULL) {
				if (victim->position != POS_SLEEPING)
					multi_hit(victim, ch, TYPE_UNDEFINED);
				break;
			}
		}
		Ahtung! Аставить для патомкоф как память о нашем тяжёлом детстве
 */
	}
}

/*
 * Cast spells at targets using a magical object.
 */
bool obj_cast_spell(int sn, int level,
		    CHAR_DATA *ch, CHAR_DATA *victim, OBJ_DATA *obj)
{
	void *vo = NULL;
	int target = TARGET_NONE;
	skill_t *spell;
	CHAR_DATA *bch = NULL;
	int bane_chance = 100;
	int bane_damage = 0;
	bool offensive = FALSE;
	bool ret;

	if (sn <= 0
	||  (spell = skill_lookup(sn)) == NULL
	||  spell->spell == NULL)
		return FALSE;

	switch (spell->spell->target) {
	default:
		bug("Obj_cast_spell: bad target for sn %d.", sn);
		return FALSE;

	case TAR_IGNORE:
		bch = ch;
		bane_damage = 10*bch->level;
		break;

	case TAR_CHAR_OFFENSIVE:
		if (victim == NULL)
			victim = ch->fighting;
		if (victim == NULL) {
			char_puts("You can't do that.\n", ch);
			return FALSE;
		}

		target = TARGET_CHAR;
		bch = victim;
		bane_damage = 10*bch->level;
		bane_chance = 2 * get_skill(victim, gsn_spellbane) / 3;
		break;

	case TAR_CHAR_DEFENSIVE:
	case TAR_CHAR_SELF:
		if (victim == NULL)
			victim = ch;
		target = TARGET_CHAR;
		bch = victim;
		bane_damage = 10*bch->level;
		break;

	case TAR_OBJ_INV:
		if (obj == NULL) {
			char_puts("You can't do that.\n", ch);
			return FALSE;
		}
		target = TARGET_OBJ;
		bch = ch;
		bane_damage = 3*bch->level;
		break;

	case TAR_OBJ_CHAR_OFF:
		if (!victim && !obj) {
			if (ch->fighting)
				victim = ch->fighting;
			else {
				char_puts("You can't do that.\n", ch);
				return FALSE;
			}
		}

		if (victim) {
			target = TARGET_CHAR;
			bch = victim;
			bane_damage = 3*bch->level;
		}
		else
			target = TARGET_OBJ;
		break;

	case TAR_OBJ_CHAR_DEF:
		if (!victim && !obj) {
			victim = ch;
			target = TARGET_CHAR;
		}
		else if (victim) {
			target = TARGET_CHAR;
		}
		else {
			target = TARGET_OBJ;
		}

		if (victim) {
			bch = victim;
			bane_damage = 3*bch->level;
		}
		break;
	}

	switch (target) {
	case TARGET_NONE:
		vo = NULL;
		break;

	case TARGET_OBJ:
		if (IS_SET(obj->hidden_flags, OHIDE_EXTRACTED))
			return FALSE;
		vo = (void*) obj;
		break;

	case TARGET_CHAR:
		vo = (void *) victim;

		switch (spell->spell->target) {
		case TAR_CHAR_DEFENSIVE:
		case TAR_OBJ_CHAR_DEF:
			if (IS_ATTACKER(victim) && !IS_ATTACKER(ch)) {
				char_puts("You can't cast defensive spell on attacker.\n", ch);
				return FALSE;
			}
			if (IS_SET(spell->flags, SKILL_QUESTIONABLE)
			&&  !check_trust(ch, victim)) {
				char_puts("They do not trust you enough "
					  "for this spell.\n", ch);
				return FALSE;
			}
			break;

		case TAR_CHAR_OFFENSIVE:
		case TAR_OBJ_CHAR_OFF:
			offensive = TRUE;
			if (IS_SET(spell->flags, SKILL_QUESTIONABLE))
				offensive = !check_trust(ch, victim);

			if (offensive) {
				if (is_safe(ch, victim)) {
					char_puts("Something isn't right...\n",
						  ch);
					return FALSE;
				}
			}
			break;
		}
	}

	if (bch && spellbane(bch, ch, bane_chance, bane_damage))
		return TRUE;

	{
		spell_spool_t	sspell;	

		sspell.sn = sn;
		sspell.level = level;
		sspell.ch = ch;
		sspell.vo = vo;
		sspell.ch = ch;
		sspell.arg = str_empty;
		sspell.percent = 95;
		sspell.target = target;

		ret = spell->spell->spell_fun(&sspell);
	}

	if (offensive && victim != ch && victim->master != ch) {
		//CHAR_DATA *vch;
		//CHAR_DATA *vch_next;

		if (victim->in_room && victim->in_room == ch->in_room)
		{
			doprintf(do_yell, victim,
					"Help! %s is attacking me!",
					PERS(ch, victim));
			if (victim->fighting == NULL)
			{
				multi_hit(victim, ch, TYPE_UNDEFINED);
			}
		}
		/*		таже хня
		for (vch = ch->in_room->people; vch; vch = vch_next) {
			vch_next = vch->next_in_room;

			if (victim == vch)
				doprintf(do_yell, victim,
					 "Help! %s is attacking me!",
					 PERS(ch, victim));

			if (victim == vch && victim->fighting == NULL) {
				multi_hit(victim, ch, TYPE_UNDEFINED);
				break;
			}
		}
		*/
	}
	return ret;
}

/*
 * Spell functions.
 */
//bool spell_acid_blast(int sn, int level, CHAR_DATA *ch, void *vo, int target, int percent)
bool spell_acid_blast(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 18);
/*	if (saves_spell_simply(level, victim, DAM_ACID))
		dam /= 2;
 */
 	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ACID,
					lookup_spell_msc(sspell->sn)))
 		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ACID,TRUE);
	return TRUE;
}

bool spell_armor(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
		  char_puts("You are already armored.\n", sspell->ch);
		else
		  act("$N is already armored.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	af.where	 = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = 7 + sspell->level / 6;
	af.modifier  = UMAX(20,10 + sspell->level / 4); /* af.modifier  = -20;*/
	af.location  = APPLY_AC;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel someone protecting you.\n", victim);
	if (sspell->ch != victim)
		act("$N is protected by your magic.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_blindness(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_BLIND))
	{
		act("$N is already blinded.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc((sspell->sn))))  {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.location  = APPLY_HITROLL;
	af.modifier  = -8;
	af.duration  = 3+sspell->level / 15;
	af.bitvector = AFF_BLIND;
	affect_to_char(victim, &af);
	char_puts("You are blinded!\n", victim);
	act("$n appears to be blinded.",victim,NULL,NULL,TO_ROOM);

	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	
	return TRUE;
}

bool spell_burning_hands(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level , 2) + 7;
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_FIRE, lookup_spell_msc((sspell->sn))))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_FIRE,TRUE);
	return TRUE;
}

bool spell_call_lightning(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;

	if (!IS_OUTSIDE(sspell->ch)) {
		char_puts("You must be out of doors.\n", sspell->ch);
		return FALSE;
	}

	if (weather_info.sky < SKY_RAINING) {
		char_puts("You need bad weather.\n", sspell->ch);
		return FALSE;
	}

	dam = dice(sspell->level, 9);

	char_puts("Gods' lightning strikes your foes!\n", sspell->ch);
	act("$n calls lightning to strike $s foes!", sspell->ch, NULL, NULL, TO_ROOM);

	for (vch = char_list; vch; vch = vch_next) {
		vch_next	= vch->next;

		if (vch->in_room == NULL)
			continue;

		if (vch->in_room == sspell->ch->in_room) {
			if (is_safe_spell(sspell->ch, vch, TRUE))
				continue;
			damage(sspell->ch, vch,
			       saves_spell(sspell->ch, sspell->level, vch, DAM_LIGHTNING, lookup_spell_msc((sspell->sn))) ?
			       dam / 2 : dam,
			       sspell->sn,DAM_LIGHTNING,TRUE);
			continue;
		}

		if (vch->in_room->area == sspell->ch->in_room->area
		&&  IS_OUTSIDE(vch)
		&&  IS_AWAKE(vch))
		    char_puts("Lightning flashes in the sky.\n", vch);
	}
	return TRUE;
}

/* RT calm spell stops all fighting in the room */
bool spell_calm(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	int mlevel = 0;
	int count = 0;
	int high_level = 0;
	int chance;
	AFFECT_DATA af;

	/* get sum of all mobile levels in the room */
	for (vch = sspell->ch->in_room->people; vch != NULL; vch = vch->next_in_room)
	{
		if (vch->position == POS_FIGHTING)
		{
		    count++;
		    if (IS_NPC(vch))
		      mlevel += vch->level;
		    else
		      mlevel += vch->level/2;
		    high_level = UMAX(high_level,vch->level);
		}
	}

	/* compute chance of stopping combat */
	chance = 4 * sspell->level - high_level + 2 * count;

	if (IS_IMMORTAL(sspell->ch)) /* always works */
	  mlevel = 0;

	if (number_range(0, chance) >= mlevel) { /* hard to stop large fights */
		for (vch = sspell->ch->in_room->people; vch; vch = vch->next_in_room) {
			if (vch == sspell->ch)
				continue;
			if (IS_NPC(vch)
			&&  (IS_SET(vch->imm_flags, MAGIC_MENTAL) ||
			     IS_SET(vch->imm_flags, FORCE_MAGIC) 	  ||
			     IS_SET(vch->pIndexData->act, ACT_UNDEAD)))
				continue;

			if (IS_AFFECTED(vch, AFF_CALM | AFF_BERSERK)
			||  is_affected(vch, gsn_frenzy))
				continue;

			char_puts("A wave of calm passes over you.\n", vch);
			if (vch->fighting || vch->position == POS_FIGHTING)
				stop_fighting(vch, FALSE);

			af.where = TO_AFFECTS;
			af.type = (sspell->sn);
			af.level = sspell->level;
			af.duration = sspell->level/4;
			af.location = APPLY_HITROLL;
			if (!IS_NPC(vch))
				af.modifier = -5;
			else
				af.modifier = -2;
			af.bitvector = AFF_CALM;
			affect_to_char(vch, &af);

			af.location = APPLY_DAMROLL;
			affect_to_char(vch, &af);
		}
	}
	return TRUE;
}

bool spell_cancellation(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	bool found = FALSE;
	int level;

	level = sspell->level + 2;

	/* unlike dispel magic, the victim gets NO save */

	/* begin running through the spells */

	if (check_dispel(level,victim,sn_lookup("armor")))
		found = TRUE;
 
	if (check_dispel(level,victim,sn_lookup("enhanced armor")))
		found = TRUE;
 
	if (check_dispel(level,victim,sn_lookup("bless")))
	    found = TRUE;
 
	if (check_dispel(level,victim,sn_lookup("blindness")))
	{
	    found = TRUE;
	    act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(level,victim,sn_lookup("calm")))
	{
		found = TRUE;
		act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
	}
 
	if (check_dispel(level,victim,sn_lookup("change sex")))
	{
	    found = TRUE;
		act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(level,victim,sn_lookup("charm person")))
	{
		found = TRUE;
		act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(level,victim,sn_lookup("chill touch")))
	{
		found = TRUE;
		act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(level,victim,sn_lookup("curse")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("detect evil")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("detect good")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("detect hidden")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("detect invis")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("detect hidden")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("detect magic")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("faerie fire")))
	{
		act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("fly")))
	{
		act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,gsn_frenzy))
	{
		act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("giant strength")))
	{
		act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("haste")))
	{
		act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("dragon wit")))
	{
		act("$n becomes foolish.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
	
	
	if (check_dispel(level,victim,sn_lookup("elven beauty")))
	{
		act("$n looks little ugly.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
	
	if (check_dispel(level,victim,sn_lookup("trollish vigor")))
	{
		act("$n's body is squeeze.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
	
	if (check_dispel(level,victim,sn_lookup("sagacity")))
	{
		act("$n becomes less wise.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("infravision")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("invis")))
	{
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("mass invis")))
	{
		act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("pass door")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("protection evil")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("protection good")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("sanctuary"))) {
		act("The white aura around $n's body vanishes.",
		    victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level, victim, gsn_black_shroud)) {
		act("The black aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level, victim, sn_lookup("shield"))) {
		act("The shield protecting $n vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("sleep")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("slow")))
	{
		act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("stone skin")))
	{
		act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("weaken")))
	{
		act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
 
	if (check_dispel(level,victim,sn_lookup("shielding")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("web")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("fear")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("protection heat")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("protection cold")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("magic resistance")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("hallucination")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("terangreal")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("power word stun")))
		found = TRUE;

	if (check_dispel(level,victim,sn_lookup("corruption")))
	{
		act("$n looks healthier.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(level,victim,sn_lookup("web")))
	{
		act("The webs around $n dissolve.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (found)
	    char_puts("Ok.\n", sspell->ch);
	else
	    char_puts("Spell failed.\n", sspell->ch);
	return TRUE;
}

bool spell_cause_light(const spell_spool_t *sspell)
{
	damage(sspell->ch, (CHAR_DATA *) sspell->vo, dice(1, 8) + sspell->level / 3, sspell->sn,DAM_HARM,TRUE);
	return TRUE;
}

bool spell_cause_critical(const spell_spool_t *sspell)
{
	damage(sspell->ch, (CHAR_DATA *) sspell->vo, dice(3, 8) + sspell->level - 6, sspell->sn,DAM_HARM,TRUE);
	return TRUE;
}

bool spell_cause_serious(const spell_spool_t *sspell)
{
	damage(sspell->ch, (CHAR_DATA *) sspell->vo, dice(2, 8) + sspell->level / 2, sspell->sn,DAM_HARM,TRUE);
	return TRUE;
}

bool spell_chain_lightning(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	CHAR_DATA *vch,*last_vict,*next_vict;
	bool found;
	int dam;
	int level = sspell->level;

	/* first strike */

	act("A lightning bolt leaps from $n's hand and arcs to $N.",
		sspell->ch,NULL,victim,TO_ROOM);
	act("A lightning bolt leaps from your hand and arcs to $N.",
		sspell->ch,NULL,victim,TO_CHAR);
	act("A lightning bolt leaps from $n's hand and hits you!",
		sspell->ch,NULL,victim,TO_VICT);

	dam = dice(level, 6);
	if (saves_spell(sspell->ch, level, victim,DAM_LIGHTNING, lookup_spell_msc((sspell->sn))))
		dam /= 3;
	damage(sspell->ch,victim,dam,(sspell->sn),DAM_LIGHTNING,TRUE);

	last_vict = victim;
	level -= 4;   /* decrement damage */

	/* new targets */
	while (level > 0) {
		found = FALSE;
		for (vch = sspell->ch->in_room->people; vch; vch = next_vict) {
			next_vict = vch->next_in_room;

			if (vch == last_vict)
				continue;

			if (is_safe_spell(sspell->ch, vch, TRUE)) {
				act("The bolt passes around $n's body.",
				    sspell->ch, NULL, NULL, TO_ROOM);
				act("The bolt passes around your body.",
				    sspell->ch, NULL, NULL, TO_CHAR);
				continue;
			}

			found = TRUE;
			last_vict = vch;
			act("The bolt arcs to $n!", vch, NULL, NULL, TO_ROOM);
			act("The bolt hits you!", vch, NULL, NULL, TO_CHAR);
			dam = dice(level,6);

			if (saves_spell(sspell->ch, level, vch, DAM_LIGHTNING, lookup_spell_msc((sspell->sn))))
				dam /= 3;
			damage(sspell->ch, vch, dam, sspell->sn, DAM_LIGHTNING, TRUE);
			level -= 4;  /* decrement damage */
		}   /* end target searching loop */

		if (found)
			continue;

/* no target found, hit the caster */
		if (sspell->ch == NULL)
			return TRUE;

		if (last_vict == sspell->ch) { /* no double hits */
			act("The bolt seems to have fizzled out.",
			    sspell->ch, NULL, NULL, TO_ROOM);
			act("The bolt grounds out through your body.",
			    sspell->ch, NULL, NULL, TO_CHAR);
			return TRUE;
		}

		last_vict = sspell->ch;
		act("The bolt arcs to $n...whoops!", sspell->ch, NULL, NULL, TO_ROOM);
		char_puts("You are struck by your own lightning!\n", sspell->ch);
		dam = dice(level,6);
		if (saves_spell(sspell->ch, level, sspell->ch, DAM_LIGHTNING, lookup_spell_msc((sspell->sn))))
			dam /= 3;
		damage(sspell->ch, sspell->ch, dam, sspell->sn, DAM_LIGHTNING, TRUE);
		level -= 4;  /* decrement damage */
	} /* now go back and find more targets */
	return TRUE;
}

bool spell_healing_light(const spell_spool_t *sspell)
{
	AFFECT_DATA af,af2;

	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
		char_puts("This room has already been healed by light.\n", sspell->ch);
		return FALSE;
	}

	af.where     = TO_ROOM_CONST;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 25;
	af.location  = APPLY_ROOM_HEAL;
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
	char_puts("The room starts to be filled with healing light.\n", sspell->ch);
	act("The room starts to be filled with $n's healing light.", sspell->ch,NULL,NULL,TO_ROOM);
	return TRUE;
}

bool spell_charm_person(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	char buf[MAX_INPUT_LENGTH];
	AFFECT_DATA af;
	int ladj;		/* sspell->level adjustment (fot not equal sex) */
	int level = sspell->level;

	if (count_charmed(sspell->ch))
		return FALSE;

	if (victim == sspell->ch) {
		char_puts("You like yourself even better!\n", sspell->ch);
		return FALSE;
	}

	if (!can_see(victim, sspell->ch) || !IS_AWAKE(victim)){
		char_puts("How you can charm if victim not see you ?!\n", sspell->ch);
		return FALSE;
	}

	if (victim->position == POS_FIGHTING){
		char_puts("Victim busy to fight and don't digress.", sspell->ch);
		return FALSE;
	}

	if (!IS_NPC(victim) && !IS_NPC(sspell->ch))
		level += get_curr_stat(sspell->ch, STAT_CHA) -
			 get_curr_stat(victim, STAT_CHA);
	if (!IS_NPC(sspell->ch) && !IS_NPC(victim))
		level += - sspell->ch->pcdata->condition[COND_DRUNK] +
			(victim->pcdata->condition[COND_DRUNK] > 10 ? 0
			 : - victim->pcdata->condition[COND_DRUNK]);

	ladj = (sspell->ch->sex == victim->sex ? 0 : 2);
	if (IS_AFFECTED(victim, AFF_CHARM)
	||  IS_AFFECTED(sspell->ch, AFF_CHARM)
	||  level+ladj < victim->level
	||  IS_SET(victim->imm_flags, MAGIC_MENTAL)
	||  saves_spell(sspell->ch, level, victim, DAM_CHARM, lookup_spell_msc((sspell->sn)))
	||  (IS_NPC(victim) && victim->pIndexData->pShop != NULL)
	||  (victim->in_room &&
	     IS_SET(victim->in_room->room_flags, ROOM_BATTLE_ARENA)))
		return TRUE;

	if (is_safe(sspell->ch, victim))
		return FALSE;

	if (victim->master)
		stop_follower(victim);
	add_follower(victim, sspell->ch);

	if (sspell->ch->leader == victim) 
		sspell->ch->leader = NULL;

	victim->leader = sspell->ch;

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= level;
	af.duration	= number_fuzzy(level / 5);
	af.location	= 0;
	af.modifier	= 0;
	af.bitvector	= AFF_CHARM;
	affect_to_char(victim, &af);
	act("Isn't $n just so nice?", sspell->ch, NULL, victim, TO_VICT);
	if (sspell->ch != victim)
		act("$N looks at you with adoring eyes.",
		    sspell->ch, NULL, victim, TO_CHAR);

	if (IS_NPC(victim) && !IS_NPC(sspell->ch)) {
		victim->last_fought= sspell->ch;
		if (number_percent() < (4 + victim->level - sspell->ch->level
		   - (get_curr_stat(sspell->ch, STAT_CHA) - 18) / 2) * 10)
		 	add_mind(victim, sspell->ch->name);
		else if (victim->in_mind == NULL) {
			snprintf(buf, sizeof(buf), "%d", victim->in_room->vnum);
			victim->in_mind = str_dup(buf);
		}
	}
	return TRUE;
}


bool spell_chill_touch(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	int dam;

	dam = number_range(1, sspell->level);
	if (!saves_spell(sspell->ch, sspell->level, victim, DAM_COLD, lookup_spell_msc(sspell->sn))) {
		act("$n turns blue and shivers.", victim, NULL, NULL, TO_ROOM);
		af.where     = TO_AFFECTS;
		af.type      = sspell->sn;
		af.level     = sspell->level;
		af.duration  = 6;
		af.location  = APPLY_STR;
		af.modifier  = -1;
		af.bitvector = 0;
		affect_join(victim, &af);
	}
	else
		dam /= 2;

	damage(sspell->ch, victim, dam, sspell->sn, DAM_COLD, TRUE);
	return TRUE;
}

bool spell_colour_spray(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level,3) + 13;
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_LIGHT, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	else {
		spell_spool_t sspellb;

		sspellb.sn	= sn_lookup("blindness");
		sspellb.level	= sspell->level / 2;
		sspellb.ch	= sspell->ch;
		sspellb.vo	= victim;
		sspellb.target	= TARGET_CHAR;
		sspellb.percent = sspell->percent;
		
		spell_blindness(&sspellb);
	}

	damage(sspell->ch, victim, dam, sspell->sn, DAM_LIGHT, TRUE);
	return TRUE;
}

bool spell_continual_light(const spell_spool_t *sspell)
{
	OBJ_DATA *light;

	if (sspell->arg[0] != '\0')  /* do a glow on some object */
	{
		light = get_obj_carry(sspell->ch,sspell->arg);

		if (light == NULL)
		{
		    char_puts("You don't see that here.\n", sspell->ch);
		    return FALSE;
		}

		if (IS_OBJ_STAT(light,ITEM_GLOW))
		{
		    act("$p is already glowing.", sspell->ch,light,NULL,TO_CHAR);
		    return FALSE;
		}

		SET_BIT(light->extra_flags,ITEM_GLOW);
		act("$p glows with a white light.", sspell->ch,light,NULL,TO_ALL);
		return TRUE;
	}

	light = create_obj(get_obj_index(OBJ_VNUM_LIGHT_BALL), 0);
	light->timer = UMAX(9, sspell->level / 2);
	obj_to_room(light, sspell->ch->in_room);
	act("$n twiddles $s thumbs and $p appears.",   sspell->ch, light, NULL, TO_ROOM);
	act("You twiddle your thumbs and $p appears.", sspell->ch, light, NULL, TO_CHAR);
	return TRUE;
}



bool spell_control_weather(const spell_spool_t *sspell)
{
	if (!IS_OUTSIDE(sspell->ch)) {
		char_puts("You can't change the weather indoors.\n", sspell->ch);
		return FALSE;
	}

	if (!str_cmp(sspell->arg, "hot"))
		weather_info.change_t += dice(UMAX(1, sspell->level / 30), 2);
	else if (!str_cmp(sspell->arg, "cold"))
		weather_info.change_t -= dice(UMAX(1, sspell->level / 30), 2);
	else if (!str_cmp(sspell->arg, "dry"))
		weather_info.change_w -= dice(UMAX(1, sspell->level / 30), 2);
	else if (!str_cmp(sspell->arg, "wet"))
		weather_info.change_w += dice(UMAX(1, sspell->level / 30), 2);
	else  {
		char_puts ("Do you want it to get hot, cold, dry or wet?\n", sspell->ch);
		return FALSE;
	}

	char_puts("Ok.\n", sspell->ch);
	return TRUE;
}


bool spell_create_food(const spell_spool_t *sspell)
{
	OBJ_DATA *mushroom;
	int	percent;

	mushroom = create_obj(get_obj_index(OBJ_VNUM_MUSHROOM), 0);
	mushroom->value[0] = UMAX(2, sspell->level / 2);
	mushroom->value[1] = UMAX(2, sspell->level / 2);

	/* Poison */
	percent = UMAX(5, sspell->percent);
	if (percent > 90)
		percent = 90 + (percent - 90) / 2;
	if (number_percent() >= percent){
		mushroom->value[3] = 1;
	}

	obj_to_room(mushroom, sspell->ch->in_room);
	act("$p suddenly appears.", sspell->ch, mushroom, NULL, TO_ROOM);
	act("$p suddenly appears.", sspell->ch, mushroom, NULL, TO_CHAR);
	return TRUE;
}

bool spell_create_water(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	int water;
	int percent;

	if (obj->pIndexData->item_type != ITEM_DRINK_CON) {
		char_puts("It is unable to hold water.\n", sspell->ch);
		return FALSE;
	}

	if (obj->value[2] != LIQ_WATER && obj->value[1] != 0) {
		char_puts("It contains some other liquid.\n", sspell->ch);
		return FALSE;
	}

	water = UMIN(sspell->level * (weather_info.sky >= SKY_RAINING ? 4 : 2),
			 obj->value[0] - obj->value[1]);

	if (water > 0) {
		obj->value[2] = LIQ_WATER;
		obj->value[1] += water;

		if (!is_name("water", obj->name)) {
			const char *p = obj->name;
			obj->name = str_printf("%s %s", obj->name, "water");
			free_string(p);
		}

		/* Poison */
		percent = UMAX(10, sspell->percent);
		if (percent > 90)
			percent = 90 + (percent - 90) / 2;
		if (number_percent() > percent){
			obj->value[3] = 1;
		}

		act("$p is filled.", sspell->ch, obj, NULL, TO_CHAR);
	}

	return TRUE;
}


bool spell_create_rose(const spell_spool_t *sspell)
{
	OBJ_DATA *rose;
	
	if (sspell->arg[0] == '\0') {
		char_puts("What rose do you want to create?\n", sspell->ch);
		return FALSE;
	}
	
	rose = create_obj(get_obj_index(OBJ_VNUM_ROSE), 0);
	rose->short_descr = mlstr_printf(rose->pIndexData->short_descr, sspell->arg);
	rose->description = mlstr_printf(rose->pIndexData->description, sspell->arg);
	
	rose->timer = UMAX(3, sspell->level / 3);
	act("$n has created $p.", sspell->ch, rose, NULL, TO_ROOM);
	act("You create $p.", sspell->ch, rose, NULL, TO_CHAR);
	obj_to_char(rose, sspell->ch);
	return TRUE;
}

bool spell_create_spring(const spell_spool_t *sspell)
{
	OBJ_DATA *spring;
	int percent;

	spring = create_obj(get_obj_index(OBJ_VNUM_SPRING), 0);
	spring->timer = sspell->level;

	percent = UMAX(5, sspell->percent);
	if (percent > 80)
		percent = 80 + (percent - 80) / 2;
	if (number_percent() > percent)
	{
		spring->value[3] = 1;
	}

	obj_to_room(spring, sspell->ch->in_room);
	act("$p flows from the ground.", sspell->ch, spring, NULL, TO_ROOM);
	act("$p flows from the ground.", sspell->ch, spring, NULL, TO_CHAR);
	return TRUE;
}

bool spell_cure_blindness(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (!is_affected(victim, gsn_blindness))
	{
		if (victim == sspell->ch)
		  char_puts("You aren't blind.\n", sspell->ch);
		else
		  act("$N doesn't appear to be blinded.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (check_dispel(sspell->level,victim,gsn_blindness))
	{
		char_puts("Your vision returns!\n", victim);
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
		if (sspell->ch != victim)
			change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	else
		char_puts("Spell failed.\n", sspell->ch);
	return TRUE;
}



bool spell_cure_critical(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int heal;

	if (victim->hit >= victim->max_hit)
		return FALSE;
	heal = dice(3, 8) + sspell->level / 2 ;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	char_puts("You feel better!\n", victim);
	if (sspell->ch != victim)
	{
		char_puts("Ok.\n", sspell->ch);
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}

/* RT added to cure plague */
bool spell_cure_disease(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (!is_affected(victim, gsn_plague))
	{
		if (victim == sspell->ch)
		  char_puts("You aren't ill.\n", sspell->ch);
		else
		  act("$N doesn't appear to be diseased.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (check_dispel(sspell->level,victim,gsn_plague))
	{
		char_puts("Your sores vanish.\n",victim);
		act("$n looks relieved as $s sores vanish.",victim,NULL,NULL,TO_ROOM);
		if (sspell->ch != victim)
			change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	else
		char_puts("Spell failed.\n", sspell->ch);
	return TRUE;
}



bool spell_cure_light(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int heal;

	if (victim->hit >= victim->max_hit)
		return FALSE;
	heal = dice(1, 8) + sspell->level / 4 + 5;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	char_puts("You feel better!\n", victim);
	if (sspell->ch != victim)
	{
		char_puts("Ok.\n", sspell->ch);
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}

bool spell_cure_poison(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (!is_affected(victim, gsn_poison))
	{
		if (victim == sspell->ch)
		  char_puts("You aren't poisoned.\n", sspell->ch);
		else
		  act("$N doesn't appear to be poisoned.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (check_dispel(sspell->level,victim,gsn_poison))
	{
		char_puts("A warm feeling runs through your body.\n",victim);
		act("$n looks much better.",victim,NULL,NULL,TO_ROOM);
		if (sspell->ch != victim)
			change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	else
		char_puts("Spell failed.\n", sspell->ch);
	return TRUE;
}

bool spell_cure_serious(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int heal;

	if (victim->hit >= victim->max_hit)
		return FALSE;
	heal = dice(2, 8) + sspell->level / 3 + 10 ;
	victim->hit = UMIN(victim->hit + heal, victim->max_hit);
	update_pos(victim);
	char_puts("You feel better!\n", victim);
	if (sspell->ch != victim) {
		char_puts("Ok.\n", sspell->ch);
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}

bool spell_bless(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	/* deal with the object case first */
	if (sspell->target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) sspell->vo;
		if (IS_OBJ_STAT(obj,ITEM_BLESS))
		{
		    act("$p is already blessed.", sspell->ch,obj,NULL,TO_CHAR);
		    return FALSE;
		}

		if (IS_OBJ_STAT(obj,ITEM_EVIL))
		{
		    AFFECT_DATA *paf;

		    paf = affect_find(obj->affected,gsn_curse);
		    if (!saves_dispel(sspell->level,paf != NULL ? paf->level : obj->level,0))
		    {
			if (paf != NULL)
			    affect_remove_obj(obj,paf);
			act("$p glows a pale blue.", sspell->ch,obj,NULL,TO_ALL);
			REMOVE_BIT(obj->extra_flags,ITEM_EVIL);
			return TRUE;
		    }
		    else
		    {
			act("The evil of $p is too powerful for you to overcome.",
			    sspell->ch,obj,NULL,TO_CHAR);
			return TRUE;
		    }
		}
		affect_enchant(obj);

		af.where	= TO_OBJECT;
		af.type		= sspell->sn;
		af.level	= sspell->level;
		af.duration	= (6 + sspell->level / 2);
		af.location	= APPLY_SAVES;
		af.modifier	= 1;
		af.bitvector	= ITEM_BLESS;
		affect_to_obj(obj,&af);

		act("$p glows with a holy aura.", sspell->ch,obj,NULL,TO_ALL);
		return TRUE;
	}

	/* character target */
	victim = (CHAR_DATA *) sspell->vo;

	if (is_affected(victim, sn_lookup("curse"))) {
		if (!check_dispel(sspell->level + 2,victim,sn_lookup("curse"))) {
			if (victim == sspell->ch)
				char_puts("You could not remove curse.\n", sspell->ch);
			else
				act("You could not remove curse from $N.\n", sspell->ch,NULL,victim,TO_CHAR);
		} else
			char_puts("Curse was removed by bless.\n", sspell->ch);
		return TRUE;
	}

	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
		  char_puts("You are already blessed.\n", sspell->ch);
		else
		  act("$N already has divine favor.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = (6 + sspell->level / 2);
	af.location  = APPLY_HITROLL;
	af.modifier  = UMIN(1, sspell->level / 8);
	af.bitvector = AFF_BLESS;
	affect_to_char(victim, &af);

	af.location  = APPLY_SAVES;
	af.modifier  = sspell->level / 8;
	af.bitvector = 0;
	affect_to_char(victim, &af);
	char_puts("You feel righteous.\n", victim);
	if (sspell->ch != victim) {
		act("You grant $N the favor of your god.", sspell->ch,NULL,victim,TO_CHAR);
		change_faith(sspell->ch, RELF_BENEDICTION_OTHER, 0);
	}
	return TRUE;
}


bool spell_curse(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	/* deal with the object case first */
	if (sspell->target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) sspell->vo;
		if (IS_OBJ_STAT(obj,ITEM_EVIL))
		{
		    act("$p is already filled with evil.", sspell->ch,obj,NULL,TO_CHAR);
		    return FALSE;
		}

		if (IS_OBJ_STAT(obj,ITEM_BLESS))
		{
		    AFFECT_DATA *paf;

		    paf = affect_find(obj->affected,sn_lookup("bless"));
			if (!saves_dispel(sspell->level,paf != NULL ? paf->level : obj->level,0))
		    {
			if (paf != NULL)
			    affect_remove_obj(obj,paf);
			act("$p glows with a red aura.", sspell->ch,obj,NULL,TO_ALL);
			REMOVE_BIT(obj->extra_flags,ITEM_BLESS);
			return TRUE;
		    }
		    else
		    {
			act("The holy aura of $p is too powerful for you to overcome.",
			    sspell->ch,obj,NULL,TO_CHAR);
			return TRUE;
		    }
		}
		affect_enchant(obj);

		af.where        = TO_OBJECT;
		af.type         = sspell->sn;
		af.level        = sspell->level;
		af.duration     = (8 + sspell->level / 5);
		af.location     = APPLY_SAVES;
		af.modifier     = -1;
		af.bitvector    = ITEM_EVIL;
		affect_to_obj(obj,&af);

		act("$p glows with a malevolent aura.", sspell->ch,obj,NULL,TO_ALL);
		return TRUE;
	}

	/* character curses */
	victim = (CHAR_DATA *) sspell->vo;

	if (is_affected(victim, sn_lookup("bless"))){
		if (!check_dispel(sspell->level + 2,victim,sn_lookup("bless"))){
			if (victim == sspell->ch)
				char_puts("You could not remove bless.\n", sspell->ch);
			else
				act("You could not remove bless from $N.\n", sspell->ch,NULL,victim,TO_CHAR);
		} else
			char_puts("Bless was removed by curse.\n", sspell->ch);
		return TRUE;
	}


	if (is_affected(victim, sspell->sn)) {
		if (victim == sspell->ch)
			char_puts("You are already cursed.\n", sspell->ch);
		else
			act("$N already cursed.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level,victim,DAM_NEGATIVE, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (8 + sspell->level / 10);
	af.location  = APPLY_HITROLL;
	af.modifier  = -1 * (sspell->level / 8);
	af.bitvector = AFF_CURSE;
	affect_to_char(victim, &af);

	af.location  = APPLY_SAVES;
	af.modifier  = 0 - (sspell->level / 8);
	affect_to_char(victim, &af);

	char_puts("You feel unclean.\n", victim);
	if (sspell->ch != victim) {
		act("$N looks very uncomfortable.", sspell->ch,NULL,victim,TO_CHAR);
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	}
	return TRUE;
}

bool spell_anathema(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	CHAR_DATA* victim = (CHAR_DATA*) sspell->vo;
	int strength = 0;
	int level = sspell->level;

	if (IS_GOOD(victim))
		strength = IS_EVIL(sspell->ch) ? 2 : (IS_GOOD(sspell->ch) ? 0 : 1);
	else if (IS_EVIL(victim))
		strength = IS_GOOD(sspell->ch) ? 2 : (IS_EVIL(sspell->ch) ? 0 : 1);
	else 
		strength = (IS_GOOD(sspell->ch) || IS_EVIL(sspell->ch)) ? 1:0;

	if (!strength) {
		act_puts("Oh, no. Your god seems to like $N.",
			 sspell->ch, NULL, victim, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	if (is_affected(victim, sspell->sn)) {
		act_puts("$N is already cursed.",
			 sspell->ch, NULL, victim, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	level += (strength - 1) * 3;

	if (saves_spell(sspell->ch, level, victim, DAM_HOLY, lookup_spell_msc(sspell->sn))) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	af.where 	= TO_AFFECTS;
	af.type  	= sspell->sn;
	af.level 	= level;
	af.duration	= (8 + level / 10);
	af.location	= APPLY_HITROLL;
	af.modifier	= - (strength * level) / 5;
	af.bitvector	= AFF_CURSE;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_SAVES;
//	af.modifier	= 0 - (level * strength) / 5;
	af.bitvector    = 0;
	affect_to_char(victim, &af);

	af.location	= APPLY_LEVEL;
	af.modifier	= -strength;
	affect_to_char(victim, &af);
	
	af.location     = APPLY_SPELL_AFFECT;
	af.modifier     = -strength * 3;
	affect_to_char(victim, &af);
	
	act("$n looks very uncomfortable.", victim, NULL, NULL, TO_ROOM);
	char_puts("You feel unclean.\n", victim);
	change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_ray_of_truth (const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam, align;

	if (IS_EVIL(sspell->ch))
	{
		victim = sspell->ch;
		char_puts("The energy explodes inside you!\n", sspell->ch);
	}

	if (victim != sspell->ch)
	{
		act("$n raises $s hand, and a blinding ray of light shoots forth!",
		    sspell->ch,NULL,NULL,TO_ROOM);
		char_puts(
		   "You raise your hand and a blinding ray of light shoots forth!\n",
		   sspell->ch);
	}

	if (IS_GOOD(victim))
	{
		act("$n seems unharmed by the light.",victim,NULL,victim,TO_ROOM);
		char_puts("The light seems powerless to affect you.\n",victim);
		return FALSE;
	}

	dam = dice(sspell->level, 10);
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_HOLY, lookup_spell_msc(sspell->sn)))
		dam /= 2;

	align = victim->alignment;
	align -= 100;

	if (align < -600)
		align = -600 + (align + 600) / 2;

	dam = (dam * align * align) / 800000;

	{
		spell_spool_t sspellb;

		sspellb.sn	= gsn_blindness;
		sspellb.level	= (sspell->level * 3) / 4;
		sspellb.ch	= sspell->ch;
		sspellb.vo	= (void *) victim;
		sspellb.target	= TARGET_CHAR;
		sspellb.percent	= sspell->percent;

		spell_blindness(&sspellb);
	}
	damage(sspell->ch, victim, dam, sspell->sn, DAM_HOLY ,TRUE);
	return TRUE;
}

/* RT replacement demonfire spell */
bool spell_demonfire(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	if (/* IS_PC(sspell->ch) && */ !IS_EVIL(sspell->ch))
	{
		victim = sspell->ch;
		char_puts("The demons turn upon you!\n", sspell->ch);
	}

	if (victim != sspell->ch)
	{
		act("$n calls forth the demons of Hell upon $N!",
		    sspell->ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the demons of Hell!",
		    sspell->ch,NULL,victim,TO_VICT);
		char_puts("You conjure forth the demons of hell!\n", sspell->ch);
	}
	dam = dice(sspell->level, 10);
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_NEGATIVE, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	{
		spell_spool_t sspellb;

		sspellb.sn	= gsn_curse;
		sspellb.level	= (sspell->level * 3) / 4;
		sspellb.ch	= sspell->ch;
		sspellb.vo	= (void *) victim;
		sspellb.target	= TARGET_CHAR;
		sspellb.percent	= sspell->percent / 2;

		spell_curse(&sspellb);
	}

	damage(sspell->ch, victim, dam, sspell->sn, DAM_NEGATIVE ,TRUE);
	return TRUE;
}

/* added by chronos */
bool spell_bluefire(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	if (/*!IS_NPC(sspell->ch) && */ !IS_NEUTRAL(sspell->ch))
	{
		victim = sspell->ch;
		char_puts("Your blue fire turn upon you!\n", sspell->ch);
	}

	if (victim != sspell->ch)
	{
		act("$n calls forth the blue fire of earth $N!",
		    sspell->ch,NULL,victim,TO_ROOM);
		act("$n has assailed you with the neutrals of earth!",
		    sspell->ch,NULL,victim,TO_VICT);
		char_puts("You conjure forth the blue fire!\n", sspell->ch);
	}

	dam = dice(sspell->level, 11);
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_FIRE, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_FIRE ,TRUE);
	return TRUE;
}

bool spell_detect_evil(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_EVIL))
	{
		if (victim == sspell->ch)
		  char_puts("You can already sense evil.\n", sspell->ch);
		else
		  act("$N can already detect evil.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = (5 + sspell->level / 3);
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_EVIL;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_detect_good(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_GOOD))
	{
		if (victim == sspell->ch)
		  char_puts("You can already sense good.\n", sspell->ch);
		else
		  act("$N can already detect good.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (5 + sspell->level / 3);
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_GOOD;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_detect_hidden(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_HIDDEN)) {
		if (victim == sspell->ch)
			char_puts("You are already as alert as you can be.\n",
				  sspell->ch);
		else
			act("$N can already sense hidden lifeforms.",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 5 + sspell->level / 3;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_DETECT_HIDDEN;
	affect_to_char(victim, &af);
	char_puts("Your awareness improves.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_detect_fade(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_FADE)) {
		if (victim == sspell->ch)
			char_puts("You are already as alert as you can be.\n",
				  sspell->ch);
		else
			act("$N can already sense faded lifeforms.",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 5 + sspell->level / 3;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_DETECT_FADE;
	affect_to_char(victim, &af);
	char_puts("Your awareness improves.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_detect_invis(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_INVIS))
	{
		if (victim == sspell->ch)
		  char_puts("You can already see invisible.\n", sspell->ch);
		else
		  act("$N can already see invisible things.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (5 + sspell->level / 3);
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_INVIS;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_detect_magic(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_MAGIC))
	{
		if (victim == sspell->ch)
		  char_puts("You can already sense magical auras.\n", sspell->ch);
		else
		  act("$N can already detect magic.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = (5 + sspell->level / 3);
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_MAGIC;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_detect_poison(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;

	if (obj->pIndexData->item_type == ITEM_DRINK_CON || obj->pIndexData->item_type == ITEM_FOOD)
	{
		if (obj->value[3] != 0)
		    char_puts("You smell poisonous fumes.\n", sspell->ch);
		else
		    char_puts("It looks delicious.\n", sspell->ch);
		return TRUE;
	}
	
	char_puts("It doesn't look poisoned.\n", sspell->ch);
	return FALSE;
}

bool spell_dispel_evil(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	if (!IS_NPC(sspell->ch) && IS_EVIL(sspell->ch))
		victim = sspell->ch;

	if (IS_GOOD(victim))
	{
		act("Gods protects $N.", sspell->ch, NULL, victim, TO_ROOM);
		return FALSE;
	}

	if (IS_NEUTRAL(victim))
	{
		act("$N does not seem to be affected.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (victim->hit > (sspell->ch->level * 4))
	  dam = dice(sspell->level, 7);
	else
	  dam = UMAX(victim->hit, dice(sspell->level,7));
	
	dam += dam * (sspell->ch->alignment - victim->alignment) / 2000;
	
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_HOLY, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_HOLY ,TRUE);
	return TRUE; 
}

bool spell_dispel_good(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	if (!IS_NPC(sspell->ch) && IS_GOOD(sspell->ch))
		victim = sspell->ch;

	if (IS_EVIL(victim))
	{
		act("$N is protected by $S evil.", sspell->ch, NULL, victim, TO_ROOM);
		return FALSE;
	}

	if (IS_NEUTRAL(victim))
	{
		act("$N does not seem to be affected.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (victim->hit > (sspell->ch->level * 4))
	  dam = dice(sspell->level, 7);
	else
	  dam = UMAX(victim->hit, dice(sspell->level,7));

	dam += dam * ( - sspell->ch->alignment + victim->alignment) / 2000;

	if (saves_spell(sspell->ch, sspell->level, victim,DAM_NEGATIVE, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_NEGATIVE ,TRUE);
	return TRUE;
}

/* modified for enhanced use */
bool spell_dispel_magic(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	bool found = FALSE;

	if (saves_spell(sspell->ch, sspell->level, victim,DAM_OTHER, lookup_spell_msc(sspell->sn)))
	{
		char_puts("You feel a brief tingling sensation.\n",victim);
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	/* begin running through the spells */

	if (check_dispel(sspell->level,victim,sn_lookup("armor")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("enhanced armor")))
		found = TRUE;
 
	if (check_dispel(sspell->level,victim,sn_lookup("bless")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("blindness")))
	{
		found = TRUE;
		act("$n is no longer blinded.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(sspell->level,victim,sn_lookup("calm")))
	{
		found = TRUE;
		act("$n no longer looks so peaceful...",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(sspell->level,victim,sn_lookup("change sex")))
	{
		found = TRUE;
		act("$n looks more like $mself again.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(sspell->level,victim,sn_lookup("charm person")))
	{
	    found = TRUE;
	    act("$n regains $s free will.",victim,NULL,NULL,TO_ROOM);
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("chill touch")))
	{
	    found = TRUE;
	    act("$n looks warmer.",victim,NULL,NULL,TO_ROOM);
	}

	if (check_dispel(sspell->level,victim,sn_lookup("curse")))
	    found = TRUE;
 
	if (check_dispel(sspell->level,victim,sn_lookup("detect evil")))
	    found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("detect good")))
		found = TRUE;
 
	if (check_dispel(sspell->level,victim,sn_lookup("detect hidden")))
	    found = TRUE;
 
	if (check_dispel(sspell->level,victim,sn_lookup("detect invis")))
	    found = TRUE;
 
	    found = TRUE;
 
	if (check_dispel(sspell->level,victim,sn_lookup("detect hidden")))
	    found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("detect magic")))
	    found = TRUE;
 
	if (check_dispel(sspell->level,victim,sn_lookup("faerie fire")))
	{
	    act("$n's outline fades.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("fly")))
	{
	    act("$n falls to the ground!",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim, gsn_frenzy))
	{
	    act("$n no longer looks so wild.",victim,NULL,NULL,TO_ROOM);;
	    found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("giant strength")))
	{
	    act("$n no longer looks so mighty.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("haste")))
	{
	    act("$n is no longer moving so quickly.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
	
	if (check_dispel(sspell->level,victim,sn_lookup("dragon wit")))
	{
		act("$n becomes foolish.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
	
	
	if (check_dispel(sspell->level,victim,sn_lookup("elven beauty")))
	{
		act("$n looks little ugly.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
	
	if (check_dispel(sspell->level,victim,sn_lookup("trollish vigor")))
	{
		act("$n's body is squeeze.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
	
	if (check_dispel(sspell->level,victim,sn_lookup("sagacity")))
	{
		act("$n becomes less wise.", victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("infravision")))
	    found = TRUE;
 
	if (check_dispel(sspell->level,victim,sn_lookup("invis")))
	{
	    act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("mass invis")))
	{
	    act("$n fades into existance.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}

	if (check_dispel(sspell->level,victim,sn_lookup("pass door")))
	    found = TRUE;
 

	if (check_dispel(sspell->level,victim,sn_lookup("protection evil")))
	    found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("protection good")))
	    found = TRUE;
 
	if (check_dispel(sspell->level,victim, gsn_sanctuary)) {
		act("The white aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(sspell->level, victim, gsn_black_shroud)) {
		act("The black aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}

	if (IS_AFFECTED(victim,AFF_SANCTUARY) 
	&&  !saves_dispel(sspell->level, victim->level,-1)
	&&  !is_affected(victim, gsn_sanctuary)) {
		REMOVE_BIT(victim->affected_by,AFF_SANCTUARY);
		act("The white aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
		found = TRUE;
	}
 
	if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)
	&&  !saves_dispel(sspell->level, victim->level, -1)
	&&  !is_affected(victim, gsn_black_shroud)) {
		REMOVE_BIT(victim->affected_by, AFF_BLACK_SHROUD);
		act("The black aura around $n's body vanishes.",
		    victim, NULL, NULL, TO_ROOM);
	}

	if (check_dispel(sspell->level,victim,sn_lookup("shield")))
	{
	    act("The shield protecting $n vanishes.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("sleep")))
	    found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("slow")))
	{
	    act("$n is no longer moving so slowly.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("stone skin")))
	{
	    act("$n's skin regains its normal texture.",victim,NULL,NULL,TO_ROOM);
	    found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("weaken")))
	{
	    act("$n looks stronger.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}
 
	if (check_dispel(sspell->level,victim,sn_lookup("shielding")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("web")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("fear")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("protection heat")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("protection cold")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("magic resistance")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("hallucination")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("terangreal")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("power word stun")))
		found = TRUE;

	if (check_dispel(sspell->level,victim,sn_lookup("corruption")))
	{
		act("$n looks healthier.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (check_dispel(sspell->level,victim,sn_lookup("web")))
	{
		act("The webs around $n dissolve.",victim,NULL,NULL,TO_ROOM);
		found = TRUE;
	}

	if (found)
	    char_puts("Ok.\n", sspell->ch);
	else
	    char_puts("Spell failed.\n", sspell->ch);
	return TRUE;
}

bool spell_earthquake(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;

	char_puts("The earth trembles beneath your feet!\n", sspell->ch);
	act("$n makes the earth tremble and shiver.", sspell->ch, NULL, NULL, TO_ROOM);

	for (vch = char_list; vch; vch = vch_next) {
		vch_next	= vch->next;

		if (vch->in_room == NULL)
			continue;

		if (vch->in_room == sspell->ch->in_room) {
			if (is_safe_spell(sspell->ch, vch, TRUE))
				continue;

			if (IS_AFFECTED(vch,AFF_FLYING))
				damage(sspell->ch, vch, 0, sspell->sn, DAM_BASH, TRUE);
			else{
				damage(sspell->ch, vch, sspell->level + dice(2, 8), sspell->sn,
					   DAM_BASH,TRUE);
				if (!saves_spell(sspell->ch, sspell->level, vch, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
					WAIT_STATE(vch, PULSE_VIOLENCE);
			}
			continue;
		}

		if (vch->in_room->area == sspell->ch->in_room->area)
			char_puts("The earth trembles and shivers.\n", vch);
	}
	return TRUE;
}

bool spell_enchant_armor(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	AFFECT_DATA *paf;
	int result, fail;
	int ac_bonus, added;
	bool ac_found = FALSE;

	if (obj->pIndexData->item_type != ITEM_ARMOR)
	{
		char_puts("That isn't an armor.\n", sspell->ch);
		return FALSE;
	}

	if (obj->wear_loc != -1)
	{
		char_puts("The item must be carried to be enchanted.\n", sspell->ch);
		return FALSE;
	}

	/* this means they have no bonus */
	ac_bonus = 0;
	fail = 25;	/* base 25% chance of failure */

	/* find the bonuses */

	if (!IS_SET(obj->hidden_flags, OHIDE_ENCHANTED))
	{
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		{
		    if (paf->location == APPLY_AC)
		    {
		    	ac_bonus = -1 * paf->modifier;
			ac_found = TRUE;
		    	fail += 5 * (ac_bonus * ac_bonus);
	 	    }

		    else  /* things get a little harder */
		    	fail += 20;
		}
 	} else {
		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location == APPLY_AC)
		  	{
			    ac_bonus = -1 * paf->modifier;
			    ac_found = TRUE;
			    fail += 5 * (ac_bonus * ac_bonus);
			}

			else /* things get a little harder */
			    fail += 20;
		}
	}

	/* apply other modifiers */
	fail -= sspell->level;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
		fail -= 15;
	if (IS_OBJ_STAT(obj,ITEM_GLOW))
		fail -= 5;

	fail = URANGE(5,fail,85);

	result = number_percent();

	/* the moment of truth */
	if (result < (fail / 5))  /* item destroyed */
	{
		act("$p flares blindingly... and evaporates!", sspell->ch,obj,NULL,TO_CHAR);
		act("$p flares blindingly... and evaporates!", sspell->ch,obj,NULL,TO_ROOM);
		extract_obj(obj, 0);
		return TRUE;
	}

	if (result < (fail / 3)) /* item disenchanted */
	{
		AFFECT_DATA *paf_next;

		act("$p glows brightly, then fades...oops.", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows brightly, then fades.", sspell->ch,obj,NULL,TO_ROOM);

		/* remove all affects */
		for (paf = obj->affected; paf != NULL; paf = paf_next)
		{
		    paf_next = paf->next;
		    aff_free(paf);
		}
		obj->affected = NULL;

		SET_BIT(obj->hidden_flags, OHIDE_ENCHANTED);
		return TRUE;
	}

	if (result <= fail)  /* failed, no bad result */
	{
		char_puts("Nothing seemed to happen.\n", sspell->ch);
		return TRUE;
	}

	/* okay, move all the old flags into new vectors if we have to */
	affect_enchant(obj);

	if (result <= (90 - sspell->level/5))  /* success! */
	{
		act("$p shimmers with a gold aura.", sspell->ch,obj,NULL,TO_CHAR);
		act("$p shimmers with a gold aura.", sspell->ch,obj,NULL,TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		added = 1;
	}
	
	else  /* exceptional enchant */
	{
		act("$p glows a brillant gold!", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows a brillant gold!", sspell->ch,obj,NULL,TO_ROOM);
		SET_BIT(obj->extra_flags,ITEM_MAGIC);
		SET_BIT(obj->extra_flags,ITEM_GLOW);
		added = 2;
	}
			
	/* now add the enchantments */

	if (obj->level < LEVEL_HERO)
		obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

	if (ac_found)
	{
		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
		    if (paf->location == APPLY_AC)
		    {
			paf->type = sspell->sn;
			paf->modifier += added;
			paf->level = UMAX(paf->level, sspell->level);
		    }
		}
	}
	else /* add a new affect */
	{
 	paf = aff_new();

		paf->where	= TO_OBJECT;
		paf->type	= sspell->sn;
		paf->level	= sspell->level;
		paf->duration	= -1;
		paf->location	= APPLY_AC;
		paf->modifier	=  added;
		paf->bitvector  = 0;
		paf->next	= obj->affected;
		obj->affected	= paf;
	}
	return TRUE;
}

bool spell_enchant_weapon(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	AFFECT_DATA *paf;
	int result, fail;
	int hit_bonus, dam_bonus, added;
	bool hit_found = FALSE, dam_found = FALSE;

	if (obj->pIndexData->item_type != ITEM_WEAPON)
	{
		char_puts("That isn't a weapon.\n", sspell->ch);
		return FALSE;
	}

	if (obj->wear_loc != -1)
	{
		char_puts("The item must be carried to be enchanted.\n", sspell->ch);
		return FALSE;
	}

	/* this means they have no bonus */
	hit_bonus = 0;
	dam_bonus = 0;
	fail = 25;	/* base 25% chance of failure */

	/* find the bonuses */

	if (!IS_SET(obj->hidden_flags, OHIDE_ENCHANTED))
	{
		for (paf = obj->pIndexData->affected; paf != NULL; paf = paf->next)
		{
		        if (paf->location == APPLY_HITROLL)
		        {
			    	hit_bonus = paf->modifier;
				hit_found = TRUE;
			    	fail += 2 * (hit_bonus * hit_bonus);
 		 	}

		  	  else if (paf->location == APPLY_DAMROLL)
			    {
			    	dam_bonus = paf->modifier;
				dam_found = TRUE;
		    		fail += 2 * (dam_bonus * dam_bonus);
			    }

		 	   else  /* things get a little harder */
			    	fail += 25;
		}
	} else {
 
		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
			if (paf->location == APPLY_HITROLL)
	  		{
			    hit_bonus = paf->modifier;
			    hit_found = TRUE;
			    fail += 2 * (hit_bonus * hit_bonus);
			}

			else if (paf->location == APPLY_DAMROLL)
  			{
			    dam_bonus = paf->modifier;
			    dam_found = TRUE;
			    fail += 2 * (dam_bonus * dam_bonus);
			}

			else /* things get a little harder */
			    fail += 25;
		}
	}

	/* apply other modifiers */
	fail -= 3 * sspell->level/2;

	if (IS_OBJ_STAT(obj,ITEM_BLESS))
		fail -= 15;
	if (IS_OBJ_STAT(obj,ITEM_GLOW))
		fail -= 5;

	fail = URANGE(5,fail,95);

	result = number_percent();

	/* the moment of truth */
	if (result < (fail / 5))  /* item destroyed */
	{
		act("$p shivers violently and explodes!", sspell->ch,obj,NULL,TO_CHAR);
		act("$p shivers violently and explodeds!", sspell->ch,obj,NULL,TO_ROOM);
		extract_obj(obj, 0);
		return TRUE;
	}

	if (result < (fail / 2)) /* item disenchanted */
	{
		AFFECT_DATA *paf_next;

		act("$p glows brightly, then fades...oops.", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows brightly, then fades.", sspell->ch,obj,NULL,TO_ROOM);

		/* remove all affects */
		for (paf = obj->affected; paf != NULL; paf = paf_next)
		{
		    paf_next = paf->next; 
		    aff_free(paf);
		}
		obj->affected = NULL;

		SET_BIT(obj->hidden_flags, OHIDE_ENCHANTED);
		return TRUE;
	}

	if (result <= fail)  /* failed, no bad result */
	{
		char_puts("Nothing seemed to happen.\n", sspell->ch);
		return TRUE;
	}

	/* okay, move all the old flags into new vectors if we have to */
	affect_enchant(obj);

	if (result <= (100 - sspell->level/5))  /* success! */
	{
		act("$p glows blue.", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows blue.", sspell->ch,obj,NULL,TO_ROOM);
		SET_BIT(obj->extra_flags, ITEM_MAGIC);
		added = 1;
	}
	
	else  /* exceptional enchant */
	{
		act("$p glows a brillant blue!", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows a brillant blue!", sspell->ch,obj,NULL,TO_ROOM);
		SET_BIT(obj->extra_flags,ITEM_MAGIC);
		SET_BIT(obj->extra_flags,ITEM_GLOW);
		added = 2;
	}
			
	/* now add the enchantments */ 

	if (obj->level < LEVEL_HERO - 1)
		obj->level = UMIN(LEVEL_HERO - 1,obj->level + 1);

	if (dam_found)
	{
		for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
		    if (paf->location == APPLY_DAMROLL)
		    {
			paf->type = sspell->sn;
			paf->modifier += added;
			paf->level = UMAX(paf->level, sspell->level);
			if (paf->modifier > 4)
			    SET_BIT(obj->extra_flags,ITEM_HUM);
		    }
		}
	}
	else /* add a new affect */
	{
		paf = aff_new();

		paf->where	= TO_OBJECT;
		paf->type	= sspell->sn;
		paf->level	= sspell->level;
		paf->duration	= -1;
		paf->location	= APPLY_DAMROLL;
		paf->modifier	=  added;
		paf->bitvector  = 0;
		paf->next	= obj->affected;
		obj->affected	= paf;
	}

	if (hit_found)
	{
	    for (paf = obj->affected; paf != NULL; paf = paf->next)
		{
	        if (paf->location == APPLY_HITROLL)
	        {
			paf->type = sspell->sn;
	            paf->modifier += added;
	            paf->level = UMAX(paf->level, sspell->level);
	            if (paf->modifier > 4)
	                SET_BIT(obj->extra_flags,ITEM_HUM);
	        }
		}
	}
	else /* add a new affect */
	{
	    paf = aff_new();
 
	    paf->type       = sspell->sn;
	    paf->level      = sspell->level;
	    paf->duration   = -1;
	    paf->location   = APPLY_HITROLL;
	    paf->modifier   =  added;
	    paf->bitvector  = 0;
	    paf->next       = obj->affected;
	    obj->affected   = paf;
	}
	return TRUE;
}



/*
 * Drain XP, MANA, HP.
 * Caster gains HP.
 */
bool spell_energy_drain(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	int dam;

	if (saves_spell(sspell->ch, sspell->level, victim,DAM_NEGATIVE, lookup_spell_msc(sspell->sn)))
	{
		char_puts("You feel a momentary chill.\n",victim);
		return TRUE;
	}


	if (victim->level <= 2)
	{
		dam		 = sspell->ch->hit + 1;
	}
	else
	{
		gain_exp(victim, 0 - number_range(sspell->level/5, 3 * sspell->level / 5), FALSE, FALSE);
		victim->mana	/= 1.5;
		victim->move	/= 1.5;
		dam		 = dice(1, sspell->level);
		sspell->ch->hit		+= dam;
	}

	if (number_percent() < sspell->percent / 5) {
		af.where 	= TO_AFFECTS;
		af.type		= sspell->sn;
		af.level	= sspell->level/2;
		af.duration	= (6 + sspell->level/12);
		af.location	= APPLY_LEVEL;
		af.modifier	= -1;
		af.bitvector	= 0;

		affect_join(victim, &af);

	}


	char_puts("You feel your life slipping away!\n",victim);
	char_puts("Wow....what a rush!\n", sspell->ch);
	damage(sspell->ch, victim, dam, sspell->sn, DAM_NEGATIVE, TRUE);
	return TRUE;
}

bool spell_hellfire(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	damage(sspell->ch, victim, dice(sspell->level, 7), sspell->sn, DAM_FIRE, TRUE);
	return TRUE;
}

bool spell_iceball(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;
	int movedam;

	dam = dice(sspell->level , 12);
	movedam     = number_range(5, sspell->level / 2 + sspell->percent / 6);

	for (vch = sspell->ch->in_room->people; vch; vch = vch_next) {
		vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch, vch, TRUE))
			continue;

		if (saves_spell(sspell->ch, sspell->level, vch, DAM_COLD, lookup_spell_msc(sspell->sn)))
			dam /= 2;
		vch->move -= UMIN(vch->move, movedam);
		damage(sspell->ch, vch, dam, sspell->sn, DAM_COLD, TRUE);
	}
	return TRUE;
}

bool spell_fireball(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 15);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_FIRE, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_FIRE ,TRUE);
	return TRUE;
}

bool spell_fireproof(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
	{
		act("$p is already protected from burning.", sspell->ch,obj,NULL,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_OBJECT;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = number_fuzzy(sspell->level / 4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = ITEM_BURN_PROOF;

	affect_to_obj(obj,&af);

	act("You protect $p from fire.", sspell->ch,obj,NULL,TO_CHAR);
	act("$p is surrounded by a protective aura.", sspell->ch,obj,NULL,TO_ROOM);
	return TRUE;
}

bool spell_flamestrike(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 10);
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_FIRE, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_FIRE ,TRUE);
	return TRUE;
}

bool spell_faerie_fire(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_FAERIE_FIRE))
	{
		act("$N already has this affect.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 10 + sspell->level / 5;
	af.location	= APPLY_AC;
	af.modifier	= -1 * 3 * sspell->level;
	af.bitvector	= AFF_FAERIE_FIRE;
	affect_to_char(victim, &af);
	char_puts("You are surrounded by a pink outline.\n", victim);
	act("$n is surrounded by a pink outline.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

extern ROOM_INDEX_DATA *check_place(CHAR_DATA *ch, const char *argument);                                
                                                                                                         
bool spell_faerie_fog(const spell_spool_t *sspell)               
{                                                                                                        
        CHAR_DATA *ich;                                                                                  
        ROOM_INDEX_DATA *room = sspell->ch->in_room;                                                             
                                                                                                         
	if ( sspell->arg[0]
	&& (room = check_place(sspell->ch, sspell->arg)) == NULL) {
		char_puts("You cannot send fog that much far.\n", sspell->ch);
		return FALSE;
	}                                                                                                
                                                                                                         
        act("$n conjures a cloud of purple smoke.", sspell->ch, NULL, NULL, TO_ROOM);                            
        char_puts("You conjure a cloud of purple smoke.\n", sspell->ch);                                         
                                                                                                         
        for (ich = room->people; ich != NULL; ich = ich->next_in_room) {                                 
                if (IS_PC(ich) && ich->pcdata->invis_level > 0)                                                                
                        continue;                                                                        
                                                                                                         
                if (ich == sspell->ch || saves_spell(sspell->ch, sspell->level, ich, DAM_OTHER, lookup_spell_msc(sspell->sn)))
                        continue;                                                                        
                                                                                                         
                affect_bit_strip(ich, TO_AFFECTS, AFF_INVIS | AFF_IMP_INVIS);                            
                REMOVE_BIT(ich->affected_by, AFF_HIDE | AFF_FADE |                                       
                                             AFF_CAMOUFLAGE | AFF_INVIS |                                
                                             AFF_IMP_INVIS);                                             
                                                                                                         
                act("$n is revealed!", ich, NULL, NULL, TO_ROOM);                                        
                char_puts("You are revealed!\n", ich);                                                   
        }
        return TRUE;
}

bool spell_floating_disc(const spell_spool_t *sspell)
{
	OBJ_DATA *disc, *floating;

	floating = get_eq_char(sspell->ch,WEAR_FLOAT);
	if (floating != NULL && IS_OBJ_STAT(floating,ITEM_NOREMOVE))
	{
		act("You can't remove $p.", sspell->ch,floating,NULL,TO_CHAR);
		return FALSE;
	}

	disc = create_obj(get_obj_index(OBJ_VNUM_DISC), 0);
	disc->value[0]	= sspell->ch->level * 10; /* 10 pounds per sspell->level capacity */
	disc->value[3]	= sspell->ch->level * 5; /* 5 pounds per level max per item */
	disc->timer		= sspell->ch->level * 2 - number_range(0, sspell->level / 2); 

	act("$n has created a floating black disc.", sspell->ch,NULL,NULL,TO_ROOM);
	char_puts("You create a floating disc.\n", sspell->ch);
	obj_to_char(disc, sspell->ch);
	wear_obj(sspell->ch,disc, 0,TRUE);
	return TRUE;
}

bool spell_fly(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_FLYING))
	{
		if (victim == sspell->ch)
		  char_puts("You are already airborne.\n", sspell->ch);
		else
		  act("$N doesn't need your help to fly.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = sspell->level + 3;
	af.location  = 0;
	af.modifier  = 0;
	af.bitvector = AFF_FLYING;
	affect_to_char(victim, &af);
	char_puts("Your feet rise off the ground.\n", victim);
	act("$n's feet rise off the ground.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

/* RT clerical berserking spell */
bool spell_frenzy(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn) || IS_AFFECTED(victim,AFF_BERSERK))
	{
		if (victim == sspell->ch)
		  char_puts("You are already in a frenzy.\n", sspell->ch);
		else
		  act("$N is already in a frenzy.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (is_affected(victim,sn_lookup("calm")))
	{
		if (victim == sspell->ch)
		  char_puts("Why don't you just relax for a while?\n", sspell->ch);
		else
		  act("$N doesn't look like $e wants to fight anymore.",
		      sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if ((IS_GOOD(sspell->ch) && !IS_GOOD(victim)) ||
		(IS_NEUTRAL(sspell->ch) && !IS_NEUTRAL(victim)) ||
		(IS_EVIL(sspell->ch) && !IS_EVIL(victim))
	  )
	{
		act("Your god doesn't seem to like $N", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type 	 = sspell->sn;
	af.level	 = sspell->level;
	af.duration	 = sspell->level / 3;
	af.modifier  = sspell->level / 6;
	af.bitvector = 0;

	af.location  = APPLY_HITROLL;
	affect_to_char(victim,&af);

	af.location  = APPLY_DAMROLL;
	affect_to_char(victim,&af);

	af.modifier  = -1 * 10 * (sspell->level / 12);
	af.location  = APPLY_AC;
	affect_to_char(victim,&af);

	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_BENEDICTION_OTHER, 0);
	char_puts("You are filled with holy wrath!\n",victim);
	act("$n gets a wild look in $s eyes!",victim,NULL,NULL,TO_ROOM);
	return TRUE;
}

static inline void
gate(CHAR_DATA *ch, CHAR_DATA *victim)
{
	transfer_char(ch, NULL, victim->in_room,
		      "$N steps through a gate and vanishes.",
		      "You step through a gate and vanish.",
		      "$N has arrived through a gate.");
}

bool spell_gate(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->level >= sspell->level + 3
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	if (sspell->ch->pet && sspell->ch->in_room == sspell->ch->pet->in_room)
		pet = sspell->ch->pet;

	gate(sspell->ch, victim);
	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (sspell->ch->pet->position != POS_STANDING)
			do_stand(pet, str_empty);
		gate(pet, victim);
	}
	return TRUE;
}

bool spell_harm(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = UMAX( 20, victim->hit - dice(1,4));
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_HARM, lookup_spell_msc(sspell->sn)))
		dam = UMIN(50, dam / 2);
	dam = UMIN(100, dam);
	damage(sspell->ch, victim, dam, sspell->sn, DAM_HARM ,TRUE);
	return TRUE;
}

bool spell_giant_strength(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
		  char_puts("You are already as strong as you can get!\n", sspell->ch);
		else
		  act("$N can't get any stronger.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= (10 + sspell->level / 3);
	af.location	= APPLY_STR;
	af.modifier	= UMAX(2, sspell->level / 15);
	af.bitvector	= 0;
	affect_to_char(victim, &af);
	char_puts("Your muscles surge with heightened power!\n", victim);
	act("$n's muscles surge with heightened power.",victim,NULL,NULL,TO_ROOM);
	return TRUE;
}

bool spell_dragon_wit(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	
	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
			char_puts("You are already clever like a dragon!\n", sspell->ch);
		else
			act("$N can't be more clever.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	
	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 5 + sspell->level / 8;
	af.location	= APPLY_INT;
	af.modifier	= URANGE(2, sspell->level / 20, 4);
	af.bitvector	= 0;
	affect_to_char(victim, &af);
	
	char_puts("The wit of the dragon enters you.\n", victim);
	act("$n looks a bit clever now.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_pray(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	
	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
			char_puts("You have already great favor!\n", sspell->ch);
		else
			act("$N can't get any more favor.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	
	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= (3 + sspell->level / 10);
	af.bitvector	= 0;
	af.modifier	= URANGE(1, sspell->level / 40, 2);
	
	af.location	= APPLY_STR;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_INT;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_WIS;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_DEX;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_CON;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_CHA;
	affect_to_char(victim, &af);

	char_puts("Your pray have reached the Gods.\n", sspell->ch);
	act("$n prays to heaven.", sspell->ch, NULL, NULL, TO_ROOM);
	if (sspell->ch != victim) {
		change_faith(sspell->ch, RELF_BENEDICTION_OTHER, 0);
		char_printf(victim, "Pray of %s's blessed you.\n", sspell->ch->name);
	}
	return TRUE;
}

bool spell_trollish_vigor(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	
	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
			char_puts("You already feel yourself like a troll!\n", sspell->ch);
		else
			act("$N can't be more vigor.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	
	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= (3 + sspell->level / 8);
	af.location	= APPLY_CON;
	af.modifier	= URANGE(2, sspell->level / 20, 4);
	af.bitvector	= 0;
	affect_to_char(victim, &af);

	af.location	= APPLY_SIZE;
	af.modifier	= 1;
	affect_to_char(victim, &af);

	char_puts("You feel yourself like a troll.\n", victim);
	act("$n gets trollish vigor.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_elven_beauty(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	
	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
			char_puts("You are already beauty!\n", sspell->ch);
		else
			act("$N can't be more beauty.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	
	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= (5 + sspell->level / 8);
	af.location	= APPLY_CHA;
	af.modifier	= URANGE(2, sspell->level / 20, 4);
	af.bitvector	= 0;
	affect_to_char(victim, &af);
	
	char_puts("You look more pretty.\n", victim);
	act("$n looks a little bit beauty.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_sagacity(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	
	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
			char_puts("You are already sagacity as you can!\n", sspell->ch);
		else
			act("$N can't be more sagacity.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	
	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= (5 + sspell->level / 8);
	af.location	= APPLY_WIS;
	af.modifier	= URANGE(2, sspell->level / 20, 4);
	af.bitvector	= 0;
	affect_to_char(victim, &af);
	
	char_puts("You get wisdom of kindred.\n", victim);
	act("$n looks a more sagacity.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_aquabreath(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	
	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
			char_puts("You are already have gills!\n", sspell->ch);
		else
			act("$N already has gills.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}
	
	af.where        = TO_AFFECTS;
	af.type         = sspell->sn;
	af.level        = sspell->level;
	af.duration     = (5 + sspell->level / 4);
	af.location     = APPLY_NONE;
	af.modifier     = 0;
	af.bitvector    = AFF_AQUABREATH;
	affect_to_char(victim, &af);
	char_puts("At you gills have appeared.\n", victim);
	act("At $n gills has appeared.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

/* RT haste spell */
bool spell_haste(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
 
	if (is_affected(victim, sspell->sn)
	||  IS_AFFECTED(victim,AFF_HASTE)
	||  (IS_NPC(victim) && IS_SET(victim->pIndexData->off_flags, OFF_FAST))) {
		if (victim == sspell->ch)
			char_puts("You can't move any faster!\n", sspell->ch);
		else
			act("$N is already moving as fast as $E can.",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (IS_AFFECTED(victim,AFF_SLOW))
	{
		if (!check_dispel(sspell->level,victim,sn_lookup("slow")))
		{
		    if (victim != sspell->ch)
			char_puts("Spell failed.\n", sspell->ch);
		    char_puts("You feel momentarily faster.\n",victim);
		}
			act("$n is moving less slowly.",victim,NULL,NULL,TO_ROOM);
		return TRUE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	if (victim == sspell->ch)
	  af.duration  = sspell->level/2;
	else
	  af.duration  = sspell->level/4;
	af.location  = APPLY_DEX;
	af.modifier  = UMAX(2, sspell->level / 15);
	af.bitvector = AFF_HASTE;
	affect_to_char(victim, &af);
	char_puts("You feel yourself moving more quickly.\n", victim);
	act("$n is moving more quickly.",victim,NULL,NULL,TO_ROOM);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_heal(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (victim->hit >= victim->max_hit)
		return FALSE;
	victim->hit = UMIN(victim->hit + 100 + sspell->level / 10, victim->max_hit);
	update_pos(victim);
	char_puts("A warm feeling fills your body.\n", victim);
	if (sspell->ch != victim) {
		char_puts("Ok.\n", sspell->ch);
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}

bool spell_heat_metal(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	OBJ_DATA *obj_lose, *obj_next;
	int dam = 0;
	bool fail = TRUE;

   if (!saves_spell(sspell->ch, sspell->level + 2,victim,DAM_FIRE, lookup_spell_msc(sspell->sn))
   &&  !IS_SET(victim->imm_flags, MAGIC_FIRE))
   {
		for (obj_lose = victim->carrying;
		      obj_lose != NULL;
		      obj_lose = obj_next)
		{
		    obj_next = obj_lose->next_content;
		    if (number_range(1,2 * sspell->level) > obj_lose->level
		    &&   !saves_spell(sspell->ch, sspell->level,victim,DAM_FIRE, lookup_spell_msc(sspell->sn))
		    &&   is_metal(obj_lose)
		    &&   !IS_OBJ_STAT(obj_lose,ITEM_BURN_PROOF))
		    {
			switch (obj_lose->pIndexData->item_type)
			{
			case ITEM_ARMOR:
			if (obj_lose->wear_loc != -1) /* remove the item */
			{
			    if (can_drop_obj(victim,obj_lose)
			    &&  (obj_lose->weight / 10) <
				number_range(1,2 * get_curr_stat(victim,STAT_DEX))
			    &&  remove_obj(victim, obj_lose->wear_loc, TRUE))
			    {
				act("$n yelps and throws $p to the ground!",
				    victim,obj_lose,NULL,TO_ROOM);
				act("You remove and drop $p before it burns you.",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 3);
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
				fail = FALSE;
			    }
			    else /* stuck on the body! ouch! */
			    {
				act("Your skin is seared by $p!",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level));
				fail = FALSE;
			    }

			}
			else /* drop it if we can */
			{
			    if (can_drop_obj(victim,obj_lose))
			    {
				act("$n yelps and throws $p to the ground!",
				    victim,obj_lose,NULL,TO_ROOM);
				act("You and drop $p before it burns you.",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 6);
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
				fail = FALSE;
			    }
			    else /* cannot drop */
			    {
				act("Your skin is seared by $p!",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 2);
				fail = FALSE;
			    }
			}
			break;
			case ITEM_WEAPON:
			if (obj_lose->wear_loc != -1) /* try to drop it */
			{
			    if (IS_WEAPON_STAT(obj_lose,WEAPON_FLAMING))
				continue;

			    if (can_drop_obj(victim,obj_lose)
			    &&  remove_obj(victim,obj_lose->wear_loc,TRUE))
			    {
				act("$n is burned by $p, and throws it to the ground.",
				    victim,obj_lose,NULL,TO_ROOM);
				char_puts(
				    "You throw your red-hot weapon to the ground!\n",
				    victim);
				dam += 1;
				obj_from_char(obj_lose);
				obj_to_room(obj_lose,victim->in_room);
				fail = FALSE;
			    }
			    else /* YOWCH! */
			    {
				char_puts("Your weapon sears your flesh!\n",
				    victim);
				dam += number_range(1,obj_lose->level);
				fail = FALSE;
			    }
			}
			else /* drop it if we can */
			{
			    if (can_drop_obj(victim,obj_lose))
			    {
				act("$n throws a burning hot $p to the ground!",
				    victim,obj_lose,NULL,TO_ROOM);
				act("You and drop $p before it burns you.",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 6);
				obj_from_char(obj_lose);
				obj_to_room(obj_lose, victim->in_room);
				fail = FALSE;
			    }
			    else /* cannot drop */
			    {
				act("Your skin is seared by $p!",
				    victim,obj_lose,NULL,TO_CHAR);
				dam += (number_range(1,obj_lose->level) / 2);
				fail = FALSE;
			    }
			}
			break;
			}
		    }
		}
	}
	if (fail)
	{
		char_puts("Your spell had no effect.\n", sspell->ch);
		char_puts("You feel momentarily warmer.\n",victim);
		return FALSE;
	}
	else /* damage! */
	{
		if (saves_spell(sspell->ch, sspell->level,victim,DAM_FIRE, lookup_spell_msc(sspell->sn)))
		    dam = 2 * dam / 3;
		damage(sspell->ch,victim,dam, sspell->sn,DAM_FIRE,TRUE);
		return TRUE;
	}
}

/* RT really nasty high-level attack spell */
bool spell_holy_word(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam;
	int sn_bless, sn_curse;

	if ((sn_bless = sn_lookup("bless")) < 0
	||  (sn_curse = sn_lookup("curse")) < 0)
		return FALSE;

	act("$n utters a word of divine power!", sspell->ch, NULL, NULL, TO_ROOM);
	char_puts("You utter a word of divine power.\n", sspell->ch);

	for (vch = sspell->ch->in_room->people; vch != NULL; vch = vch_next) {
		vch_next = vch->next_in_room;

		if ((IS_GOOD(sspell->ch) && IS_GOOD(vch)) ||
		    (IS_EVIL(sspell->ch) && IS_EVIL(vch)) ||
		    (IS_NEUTRAL(sspell->ch) && IS_NEUTRAL(vch))) {
			spell_spool_t sspellb;

			sspellb.level	= sspell->level;
			sspellb.ch	= sspell->ch;
			sspellb.vo	= vch;
			sspellb.target	= TARGET_CHAR;
			sspellb.percent = sspell->percent;
		
			char_puts("You feel full more powerful.\n", vch);
			sspellb.sn = gsn_frenzy;
			spell_frenzy(&sspellb);
			sspellb.sn = sn_bless;
			spell_bless(&sspellb);
			continue;
		}

		if (is_safe_spell(sspell->ch, vch, TRUE))
			continue;

		if ((IS_GOOD(sspell->ch) && IS_EVIL(vch))
		||  (IS_EVIL(sspell->ch) && IS_GOOD(vch))) {
			spell_spool_t sspellb;

			sspellb.level	= sspell->level;
			sspellb.ch	= sspell->ch;
			sspellb.vo	= vch;
			sspellb.target	= TARGET_CHAR;
			sspellb.percent = sspell->percent;
		
			sspellb.sn = sn_curse;

			spell_curse(&sspellb);
			char_puts("You are struck down!\n",vch);
			dam = dice(sspell->level, 6);
			damage(sspell->ch, vch, dam, sspell->sn, DAM_ENERGY, TRUE);
			continue;
		}

		if (IS_NEUTRAL(sspell->ch)) {
			spell_spool_t sspellb;

			sspellb.level	= sspell->level / 2;
			sspellb.ch	= sspell->ch;
			sspellb.vo	= vch;
			sspellb.target	= TARGET_CHAR;
			sspellb.percent = sspell->percent;
		
			sspellb.sn = sn_curse;

			spell_curse(&sspellb);
			char_puts("You are struck down!\n", vch);
			dam = dice(sspell->level, 4);
			damage(sspell->ch, vch, dam, sspell->sn, DAM_ENERGY, TRUE);
		}
	}

	char_puts("You feel drained.\n", sspell->ch);
	gain_exp(sspell->ch, -1 * number_range(1,10) * 5, FALSE, FALSE);
	sspell->ch->move /= (4/3);
	sspell->ch->hit /= (4/3);
	return TRUE;
}

bool spell_identify(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	BUFFER *output;

	output = buf_new(-1);
	format_obj(output, obj);
	if (!IS_SET(obj->hidden_flags, OHIDE_ENCHANTED))
		format_obj_affects(output, obj->pIndexData->affected,
				   FOA_F_NODURATION | FOA_F_NOAFFECTS);
	format_obj_affects(output, obj->affected, FOA_F_NOAFFECTS);
	page_to_char(buf_string(output), sspell->ch);
	buf_free(output);
	return TRUE;
}

bool spell_infravision(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_INFRARED)) {
		if (victim == sspell->ch)
			char_puts("You can already see in the dark.\n", sspell->ch);
		else
			act("$N already has infravision.\n",
			    sspell->ch, NULL, victim,TO_CHAR);
		return FALSE;
	}
	act("$n's eyes glow red.\n", sspell->ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= 2 * sspell->level;
	af.location	= APPLY_NONE;
	af.modifier	= 0;
	af.bitvector	= AFF_INFRARED;
	affect_to_char(victim, &af);
	char_puts("Your eyes glow red.\n", victim);
	return TRUE;
}

bool spell_invisibility(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;

	/* object invisibility */
	if (sspell->target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) sspell->vo;

		if (IS_OBJ_STAT(obj,ITEM_INVIS))
		{
		    act("$p is already invisible.", sspell->ch,obj,NULL,TO_CHAR);
		    return FALSE;
		}

		af.where	= TO_OBJECT;
		af.type		= sspell->sn;
		af.level	= sspell->level;
		af.duration	= sspell->level / 4 + 12;
		af.location	= APPLY_NONE;
		af.modifier	= 0;
		af.bitvector	= ITEM_INVIS;
		affect_to_obj(obj,&af);

		act("$p fades out of sight.", sspell->ch,obj,NULL,TO_ALL);
		return TRUE;
	}

	/* character invisibility */
	victim = (CHAR_DATA *) sspell->vo;

	if (IS_AFFECTED(victim, AFF_INVIS))
		return FALSE;

	act("$n fades out of existence.", victim, NULL, NULL, TO_ROOM);

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (sspell->level / 8 + 10);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_INVIS;
	affect_to_char(victim, &af);
	char_puts("You fade out of existence.\n", victim);
	return TRUE;
}

bool spell_know_alignment(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	char *msg;

		 if (IS_GOOD(victim)) msg = "$N has a pure and good aura.";
	else if (IS_NEUTRAL(victim)) msg = "$N act as no align.";
	else msg = "$N is the embodiment of pure evil!.";

	act(msg, sspell->ch, NULL, victim, TO_CHAR);

	if (IS_PC(victim)) 
	{
	 if (victim->pcdata->ethos == ETHOS_LAWFUL)
	 	msg = "$N upholds the laws.";
	 else if (victim->pcdata->ethos == ETHOS_NEUTRAL)
	 	msg = "$N seems ambivalent to society.";
	 else if (victim->pcdata->ethos == ETHOS_CHAOTIC)
	 	msg = "$N seems very chaotic.";
	 else msg = "$N doesn't know where they stand on the laws.";
	 act(msg, sspell->ch, NULL, victim, TO_CHAR);
	}
	return TRUE;
}

bool spell_lightning_bolt(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level,4) + 12;
	if (saves_spell(sspell->ch, sspell->level, victim,DAM_LIGHTNING, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_LIGHTNING ,TRUE);
	return TRUE;
}

bool spell_locate_object(const spell_spool_t *sspell)
{
	BUFFER *buffer = NULL;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int number = 0, max_found;

	number = 0;
	max_found = IS_IMMORTAL(sspell->ch) ? 200 : 2 * sspell->level;

	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(sspell->ch, obj) || !is_name(sspell->arg, obj->name)
		||  IS_OBJ_STAT(obj,ITEM_NOLOCATE)
		||  number_percent() > 2 * sspell->level
		||  sspell->ch->level < obj->level)
			continue;

		if (buffer == NULL)
			buffer = buf_new(-1);
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL;
						in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL
		&&  can_see(sspell->ch,in_obj->carried_by))
		    buf_printf(buffer, "One is carried by %s\n",
			fix_short(PERS(in_obj->carried_by, sspell->ch)));
		else
		{
		    if (IS_IMMORTAL(sspell->ch) && in_obj->in_room != NULL)
			buf_printf(buffer, "One is in %s [Room %d]\n",
				mlstr_cval(in_obj->in_room->name, sspell->ch),
				in_obj->in_room->vnum);
		    else
			buf_printf(buffer, "One is in %s\n",
			    in_obj->in_room == NULL ?
			    "somewhere" :
			    mlstr_cval(in_obj->in_room->name, sspell->ch));
		}

		if (number >= max_found)
			break;
	}

	if (buffer == NULL)
		char_puts("Nothing like that in heaven or earth.\n", sspell->ch);
	else {
		page_to_char(buf_string(buffer), sspell->ch);
		buf_free(buffer);
	}
	return TRUE;
}

bool spell_magic_missile(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int level;

	static const int dam_each[] =
	{
		 0,
		 3,  3,  4,  4,  5,	 6,  6,  6,  6,  6,
		 7,  7,  7,  7,  7,	 8,  8,  8,  8,  8,
		 9,  9,  9,  9,  9,	10, 10, 10, 10, 10,
		11, 11, 11, 11, 11,	12, 12, 12, 12, 12,
		13, 13, 13, 13, 13,	14, 14, 14, 14, 14
	};

	int dam;
	if (victim == NULL || sspell->ch == NULL)
	{
		bug("spell_magic_missile: null victim [%d]", victim == NULL ? 1 : 0);
		return FALSE;
	}
	
	debug_printf(8, "sp_mm_000");

	if (is_affected(victim, gsn_protective_shield))  {
		const char *text = sspell->ch->level > 4 ? "missiles" : "missile";

		act("Your magic $t fizzle out near your victim.",
		    sspell->ch, text, victim, TO_CHAR);
		act("Your shield blocks $N's magic $t.",
		    victim, text, sspell->ch, TO_CHAR);
		return FALSE;
	}
	
	debug_printf(8, "sp_mm_010");

	level = UMIN(sspell->level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
	level = UMAX(0, level);
	
	debug_printf(8, "sp_mm_020");
	if (sspell->ch->level > 50)
		dam = level / 4;
	else
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		
	debug_printf(8, "sp_mm_030");

	if (saves_spell(sspell->ch, level, victim, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	debug_printf(8, "sp_mm_040");
	
	damage(sspell->ch, victim, dam, sspell->sn, DAM_ENERGY ,TRUE);
	debug_printf(8, "sp_mm_111");

	if (sspell->ch->level > 4)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(sspell->ch, level, victim, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
			dam /= 2;
		damage(sspell->ch, victim, dam, sspell->sn, DAM_ENERGY ,TRUE);
	}
	if (sspell->ch->level > 8)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(sspell->ch, level, victim,DAM_ENERGY, lookup_spell_msc(sspell->sn)))
			dam /= 2;
		damage(sspell->ch, victim, dam, sspell->sn, DAM_ENERGY ,TRUE);
	}
	if (sspell->ch->level > 12)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(sspell->ch, level, victim,DAM_ENERGY, lookup_spell_msc(sspell->sn)))
			dam /= 2;
		damage(sspell->ch, victim, dam, sspell->sn, DAM_ENERGY ,TRUE);
	}
	if (sspell->ch->level > 16)  {
		dam = number_range(dam_each[level] / 2, dam_each[level] * 2);
		if (saves_spell(sspell->ch, level, victim,DAM_ENERGY, lookup_spell_msc(sspell->sn)))
			dam /= 2;
		damage(sspell->ch, victim, dam, sspell->sn, DAM_ENERGY ,TRUE);
	}
	return TRUE;
}

bool spell_mass_healing(const spell_spool_t *sspell)
{
	CHAR_DATA *gch;
	bool ret = FALSE;

	for (gch = sspell->ch->in_room->people; gch != NULL; gch = gch->next_in_room)
		if ((IS_NPC(sspell->ch) && IS_NPC(gch))
		||  (!IS_NPC(sspell->ch) && !IS_NPC(gch))) {
			spell_spool_t sspellb;

			sspellb.level	= sspell->level;
			sspellb.ch	= sspell->ch;
			sspellb.vo	= gch;
			sspellb.target	= TARGET_CHAR;
			sspellb.percent = sspell->percent;
		
			sspellb.sn	= sn_lookup("heal");
			ret = ret ? TRUE : spell_heal(&sspellb);
			sspellb.sn	= sn_lookup("refresh");
			ret = ret ? TRUE : spell_refresh(&sspellb);
		}
	return ret;
}

bool spell_mass_invis(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	CHAR_DATA *gch;
	bool ret = FALSE;

	for (gch = sspell->ch->in_room->people; gch != NULL; gch = gch->next_in_room)
	{
		if (!is_same_group(gch, sspell->ch) || IS_AFFECTED(gch, AFF_INVIS))
		    continue;
		act("$n slowly fades out of existence.", gch, NULL, NULL, TO_ROOM);
		char_puts("You slowly fade out of existence.\n", gch);

		af.where     = TO_AFFECTS;
		af.type      = sspell->sn;
		af.level     = sspell->level/2;
		af.duration  = 24;
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		af.bitvector = AFF_INVIS;
		affect_to_char(gch, &af);
		ret = TRUE;
	}
	char_puts("Ok.\n", sspell->ch);

	return ret;
}

bool spell_pass_door(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_PASS_DOOR))
	{
		if (victim == sspell->ch)
		  char_puts("You are already out of phase.\n", sspell->ch);
		else
		  act("$N is already shifted out of phase.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = number_fuzzy(sspell->level / 4);
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_PASS_DOOR;
	affect_to_char(victim, &af);
	act("$n turns translucent.", victim, NULL, NULL, TO_ROOM);
	char_puts("You turn translucent.\n", victim);
	return TRUE;
}

/* RT plague spell, very nasty */

bool spell_plague(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	bool nplague;

	if (saves_spell(sspell->ch, sspell->level,victim,DAM_DISEASE, lookup_spell_msc(sspell->sn)) ||
		(IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_UNDEAD)))
	{
		if (sspell->ch == victim)
		  char_puts("You feel momentarily ill, but it passes.\n", sspell->ch);
		else
		  act("$N seems to be unaffected.", sspell->ch,NULL,victim,TO_CHAR);
		return TRUE;
	}

	af.where     = TO_AFFECTS;
	af.type 	 = sspell->sn;
	af.level	 = sspell->level * 3/4;
	af.duration  = (10 + sspell->level / 10);
	if (IS_AFFECTED(victim, AFF_PLAGUE)) {
		af.location  = APPLY_NONE;
		af.modifier  = 0;
		nplague = FALSE; 
	} else {
		af.location  = APPLY_STR;
		af.modifier  = -number_range(4, 6);
		nplague = TRUE;
	}
	af.bitvector = AFF_PLAGUE;
	affect_join(victim,&af);

	char_puts
	  ("You scream in agony as plague sores erupt from your skin.\n",victim);
	act("$n screams in agony as plague sores erupt from $s skin.",
		victim,NULL,NULL,TO_ROOM);
	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);

	if (nplague) {
		char_puts("You feel your strength slip away.\n", victim);
		act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
	}
	return TRUE;
}

bool spell_poison(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	AFFECT_DATA af;


	if (sspell->target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) sspell->vo;

		if (obj->pIndexData->item_type == ITEM_FOOD || obj->pIndexData->item_type == ITEM_DRINK_CON)
		{
		    if (IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
		    {
			act("Your spell fails to corrupt $p.", sspell->ch,obj,NULL,TO_CHAR);
			return FALSE;
		    }
		    obj->value[3] = 1;
		    act("$p is infused with poisonous vapors.", sspell->ch,obj,NULL,TO_ALL);
		    return TRUE;
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
		    ||  IS_OBJ_STAT(obj,ITEM_BLESS) || IS_OBJ_STAT(obj,ITEM_BURN_PROOF))
		    {
			act("You can't seem to envenom $p.", sspell->ch,obj,NULL,TO_CHAR);
			return FALSE;
		    }

		    if (IS_WEAPON_STAT(obj,WEAPON_POISON))
		    {
			act("$p is already envenomed.", sspell->ch,obj,NULL,TO_CHAR);
			return FALSE;
		    }

		    af.where	 = TO_WEAPON;
		    af.type	 = sspell->sn;
		    af.level	 = sspell->level / 2;
		    af.duration	 = sspell->level/8;
		    af.location	 = 0;
		    af.modifier	 = 0;
		    af.bitvector = WEAPON_POISON;
		    affect_to_obj(obj,&af);

		    act("$p is coated with deadly venom.", sspell->ch,obj,NULL,TO_ALL);
		    return TRUE;
		}

		act("You can't poison $p.", sspell->ch,obj,NULL,TO_CHAR);
		return FALSE;
	}

	victim = (CHAR_DATA *) sspell->vo;

	if (saves_spell(sspell->ch, sspell->level, victim,DAM_POISON, lookup_spell_msc(sspell->sn)))
	{
		act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
		char_puts("You feel momentarily ill, but it passes.\n",victim);
		return TRUE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (10 + sspell->level / 10);
	af.location  = APPLY_STR;
	af.modifier  = -2;
	af.bitvector = AFF_POISON;
	affect_join(victim, &af);
	char_puts("You feel very sick.\n", victim);
	act("$n looks very ill.",victim,NULL,NULL,TO_ROOM);
	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_protection_evil(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_PROTECT_EVIL) 
	||   IS_AFFECTED(victim, AFF_PROTECT_GOOD))
	{
		if (victim == sspell->ch)
		  char_puts("You are already protected.\n", sspell->ch);
		else
		  act("$N is already protected.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (10 + sspell->level / 5);
	af.location  = APPLY_SAVES;
	af.modifier  = 1 + sspell->level/10;
	af.bitvector = AFF_PROTECT_EVIL;
	affect_to_char(victim, &af);
	char_puts("You feel holy and pure.\n", victim);
	if (sspell->ch != victim)
		act("$N is protected from evil.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_protection_good(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_PROTECT_GOOD)
	||   IS_AFFECTED(victim, AFF_PROTECT_EVIL))
	{
		if (victim == sspell->ch)
		  char_puts("You are already protected.\n", sspell->ch);
		else
		  act("$N is already protected.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (10 + sspell->level / 5);
	af.location  = APPLY_SAVES;
	af.modifier  = 1 + sspell->level / 10;
	af.bitvector = AFF_PROTECT_GOOD;
	affect_to_char(victim, &af);
	char_puts("You feel aligned with darkness.\n", victim);
	if (sspell->ch != victim)
		act("$N is protected from good.", sspell->ch,NULL,victim,TO_CHAR);
	return TRUE;
}

bool spell_recharge(const spell_spool_t *sspell)
{
	OBJ_DATA *obj = (OBJ_DATA *) sspell->vo;
	int chance, percent2;

	if (obj->pIndexData->item_type != ITEM_WAND && obj->pIndexData->item_type != ITEM_STAFF)
	{
		char_puts("That item does not carry charges.\n", sspell->ch);
		return FALSE;
	}

	if (obj->value[3] >= 3 * sspell->level / 2)
	{
		char_puts("Your skills are not great enough for that.\n", sspell->ch);
		return TRUE;
	}

	if (obj->value[1] == 0)
	{
		char_puts("That item has already been recharged once.\n", sspell->ch);
		return FALSE;
	}

	chance = 40 + 2 * sspell->level;

	chance -= obj->value[3]; /* harder to do high-level spells */
	chance -= (obj->value[1] - obj->value[2]) *
		      (obj->value[1] - obj->value[2]);

	chance = UMAX(sspell->level/2,chance);

	percent2 = number_percent();

	if (percent2 < chance / 2)
	{
		act("$p glows softly.", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows softly.", sspell->ch,obj,NULL,TO_ROOM);
		obj->value[2] = UMAX(obj->value[1],obj->value[2]);
		obj->value[1] = 0;
		return TRUE;
	}

	else if (percent2 <= chance)
	{
		int chargeback,chargemax;

		act("$p glows softly.", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows softly.", sspell->ch,obj,NULL,TO_CHAR);

		chargemax = obj->value[1] - obj->value[2];

		if (chargemax > 0)
		    chargeback = UMAX(1,chargemax * percent2 / 100);
		else
		    chargeback = 0;

		obj->value[2] += chargeback;
		obj->value[1] = 0;
		return TRUE;
	}

	else if (percent2 <= UMIN(95, 3 * chance / 2))
	{
		char_puts("Nothing seems to happen.\n", sspell->ch);
		if (obj->value[1] > 1)
		    obj->value[1]--;
		return TRUE;
	}

	else /* whoops! */
	{
		act("$p glows brightly and explodes!", sspell->ch,obj,NULL,TO_CHAR);
		act("$p glows brightly and explodes!", sspell->ch,obj,NULL,TO_ROOM);
		extract_obj(obj, 0);
	}
	return TRUE;
}

bool spell_refresh(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int move;

	if (victim->move >= victim->max_move)
		return FALSE;
	move = number_range(10 + sspell->level / 2, 10 + sspell->level * 1.1);
	victim->move = UMIN(victim->move + move, victim->max_move);
	if (victim->max_move == victim->move)
		char_puts("You feel fully refreshed!\n",victim);
	else
		char_puts("You feel less tired.\n", victim);
	if (sspell->ch != victim) {
		change_faith(sspell->ch, RELF_CURE_OTHER, 0);
		char_puts("Ok.\n", sspell->ch);
	}
	return TRUE;
}

bool spell_remove_curse(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	bool found = FALSE;

	/* do object cases first */
	if (sspell->target == TARGET_OBJ)
	{
		obj = (OBJ_DATA *) sspell->vo;

		if (IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
		{
		    if (!IS_OBJ_STAT(obj,ITEM_NOUNCURSE)
		    &&  !saves_dispel(sspell->level + 2,obj->level,0))
		    {
			REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
			REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
			act("$p glows blue.", sspell->ch,obj,NULL,TO_ALL);
			return TRUE;
		    }

		    act("The curse on $p is beyond your power.", sspell->ch,obj,NULL,TO_CHAR);
		    return TRUE;
		}
		else  {
		  char_puts("Nothing happens...\n", sspell->ch);
		  return FALSE;
		}
	}

	/* characters */
	victim = (CHAR_DATA *) sspell->vo;

	if (check_dispel(sspell->level,victim,gsn_curse))
	{
		char_puts("You feel better.\n",victim);
		act("$n looks more relaxed.",victim,NULL,NULL,TO_ROOM);
		if (sspell->ch != victim)
			change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}

   for (obj = victim->carrying; (obj != NULL && !found); obj = obj->next_content)
   {
		if ((IS_OBJ_STAT(obj,ITEM_NODROP) || IS_OBJ_STAT(obj,ITEM_NOREMOVE))
		&&  !IS_OBJ_STAT(obj,ITEM_NOUNCURSE))
		{   /* attempt to remove curse */
		    if (!saves_dispel(sspell->level,obj->level,0))
		    {
			found = TRUE;
			REMOVE_BIT(obj->extra_flags,ITEM_NODROP);
			REMOVE_BIT(obj->extra_flags,ITEM_NOREMOVE);
			act("Your $p glows blue.",victim,obj,NULL,TO_CHAR);
			act("$n's $p glows blue.",victim,obj,NULL,TO_ROOM);
		    }
		 }
	}
	return TRUE;
}

bool spell_sanctuary(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (victim == sspell->ch)
			char_puts("You are already in sanctuary.\n", sspell->ch);
		else
			act("$N is already in sanctuary.",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (is_affected(victim, gsn_black_shroud)) {
		if (victim == sspell->ch)
	 		char_puts("But you are surrounded by black shroud.\n",
				  sspell->ch);
		else
			act("But $n is surrounded by black shroud.\n",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level / 6;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SANCTUARY;
	affect_to_char(victim, &af);
	act("$n is surrounded by a white aura.", victim, NULL, NULL, TO_ROOM);
	char_puts("You are surrounded by a white aura.\n", victim);
	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_BENEDICTION_OTHER, 0);
	return TRUE; 
}

bool spell_black_shroud(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA*) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_SANCTUARY)) {
		if (victim == sspell->ch)
			char_puts("But you are in sanctuary.\n", sspell->ch);
		else
			act("But $N in sanctuary.", sspell->ch, NULL, victim,TO_CHAR);
		return FALSE;
	}

	if (!IS_EVIL(sspell->ch)) {
		char_puts("The gods are infuriated!.\n", sspell->ch);
		damage(sspell->ch, sspell->ch, dice(sspell->level, IS_GOOD(sspell->ch) ? 2 : 1),
		       TYPE_HIT, DAM_HOLY, TRUE);
		return FALSE;
	}
	
	if (!IS_EVIL(victim)) {
		act("Your god does not seems to like $N", 
		    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (is_affected(victim, sspell->sn)) {
		if (victim == sspell->ch)
			char_puts("You are already protected.\n", sspell->ch);
		else
			act("$N is already protected.\n",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = sspell->level/6;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_BLACK_SHROUD;
	affect_to_char(victim, &af);
	act ("$n is surrounded by black aura.", victim, NULL, NULL, TO_ROOM);
	char_puts("You are surrounded by black aura.\n", victim);
	return TRUE;
}

bool spell_shield(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn))
	{
		if (victim == sspell->ch)
		  char_puts("You are already shielded from harm.\n", sspell->ch);
		else
		  act("$N is already protected by a shield.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (8 + sspell->level / 3);
	af.location  = APPLY_AC;
	af.modifier  = UMAX(20,10 + sspell->level / 3); /* af.modifier  = -20;*/
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n is surrounded by a force shield.", victim, NULL, NULL, TO_ROOM);
	char_puts("You are surrounded by a force shield.\n", victim);
	return TRUE;
}

bool spell_shocking_grasp(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int level;

	static const int dam_each[] =
	{
		 6,
		 8,  10,  12,  14,  16,	 18, 20, 25, 29, 33,
		36, 39, 39, 39, 40,	40, 41, 41, 42, 42,
		43, 43, 44, 44, 45,	45, 46, 46, 47, 47,
		48, 48, 49, 49, 50,	50, 51, 51, 52, 52,
		53, 53, 54, 54, 55,	55, 56, 56, 57, 57
	};

	int dam;

	level	= UMIN(sspell->level, sizeof(dam_each)/sizeof(dam_each[0]) - 1);
	level	= UMAX(0, level);
		if (sspell->ch->level > 50)
	dam 	= level / 2 ;
		else
	dam	= number_range(dam_each[level] / 2, dam_each[level] * 2);
	if (saves_spell(sspell->ch, level, victim,DAM_LIGHTNING, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_LIGHTNING ,TRUE);
	return TRUE;
}

bool spell_sleep(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_SLEEP))
		return FALSE;

	if ( (IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_UNDEAD))
	||  sspell->level < victim->level
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_CHARM, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = 1 + sspell->level/10;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_SLEEP;
	affect_join(victim, &af);

	if (IS_AWAKE(victim))
	{
		char_puts("You feel very sleepy ..... zzzzzz.\n", victim);
		act("$n goes to sleep.", victim, NULL, NULL, TO_ROOM);
		victim->position = POS_SLEEPING;
	}
	return TRUE;
}

bool spell_slow(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn) || IS_AFFECTED(victim,AFF_SLOW))
	{
		if (victim == sspell->ch)
		  char_puts("You can't move any slower!\n", sspell->ch);
		else
		  act("$N can't get any slower than that.",
		      sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level,victim,DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  IS_SET(victim->imm_flags,FORCE_MAGIC)
	||  IS_SET(victim->imm_flags,MAGIC_EARTH))
	{
		if (victim != sspell->ch)
		    char_puts("Nothing seemed to happen.\n", sspell->ch);
		char_puts("You feel momentarily lethargic.\n",victim);
		return TRUE;
	}

	if (IS_AFFECTED(victim,AFF_HASTE))
	{
		if (!check_dispel(sspell->level,victim,sn_lookup("haste")))
		{
		    if (victim != sspell->ch)
			char_puts("Spell failed.\n", sspell->ch);
		    char_puts("You feel momentarily slower.\n",victim);
		    return TRUE;
		}

		act("$n is moving less quickly.",victim,NULL,NULL,TO_ROOM);
		return TRUE;
	}


	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (4 + sspell->level / 12);
	af.location  = APPLY_DEX;
	af.modifier  = - UMAX(2, sspell->level / 12);
	af.bitvector = AFF_SLOW;
	affect_to_char(victim, &af);
	char_puts("You feel yourself slowing d o w n...\n", victim);
	act("$n starts to move in slow motion.",victim,NULL,NULL,TO_ROOM);
	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}


bool spell_stone_skin(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(sspell->ch, sspell->sn))
	{
		if (victim == sspell->ch)
		  char_puts("Your skin is already as hard as a rock.\n", sspell->ch);
		else
		  act("$N is already as hard as can be.", sspell->ch,NULL,victim,TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (10 + sspell->level / 5);
	af.location  = APPLY_AC;
	af.modifier  = UMAX(40,20 + sspell->level / 2);  /*af.modifier=-40;*/ 
	af.bitvector = 0;
	affect_to_char(victim, &af);
	act("$n's skin turns to stone.", victim, NULL, NULL, TO_ROOM);
	char_puts("Your skin turns to stone.\n", victim);
	return TRUE;
}

bool spell_summon(const spell_spool_t *sspell)
{
	bool failed = FALSE;
	CHAR_DATA *victim;

	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->in_room == NULL) {
		char_puts("You failed.\n", sspell->ch);
		return FALSE;
	}

	if (victim == sspell->ch
	||  victim->level >= sspell->level + 3
	||  victim->fighting != NULL
	||  !can_see_room(sspell->ch, victim->in_room)
	||  IS_SET(sspell->ch->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
					    ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(victim->in_room->room_flags, ROOM_SAFE | ROOM_NORECALL |
						ROOM_PEACE | ROOM_NOSUMMON)
	||  IS_SET(sspell->ch->in_room->area->flags, AREA_CLOSED)
	||  room_is_private(sspell->ch->in_room)
	||  IS_SET(victim->imm_flags, MAGIC_MENTAL)
	||  (victim->in_room->exit[0] == NULL &&
	     victim->in_room->exit[1] == NULL &&
	     victim->in_room->exit[2] == NULL &&
	     victim->in_room->exit[3] == NULL &&
	     victim->in_room->exit[4] == NULL &&
	     victim->in_room->exit[5] == NULL))
		failed = TRUE;
	else if (IS_NPC(victim)) {
		if (victim->pIndexData->pShop != NULL
		||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
		||  IS_SET(victim->pIndexData->act, ACT_AGGRESSIVE)
		||  IS_SET(sspell->ch->in_room->room_flags, ROOM_NOMOB))
			failed = TRUE;
	}
	else {
/*		if (victim->level > LEVEL_HERO
		||  ((!in_PK(sspell->ch, victim) ||
		      sspell->ch->in_room->area != victim->in_room->area) &&
		     IS_SET(victim->pcdata->plr_flags, PLR_NOSUMMON))
*/
		if (check_mentalblock(sspell->ch, victim, 25,
			"You failed to summon victim.\n",
			"Someone attempted to summon you.\n")
		||  !guild_ok(victim, sspell->ch->in_room)
		|| IS_IMMORTAL(victim))
			failed = TRUE;
	}

	if (failed) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	if (IS_NPC(victim) && victim->in_mind == NULL) {
		char buf[MAX_INPUT_LENGTH];
		snprintf(buf, sizeof(buf), "%d", victim->in_room->vnum);
		victim->in_mind = str_dup(buf);
	}

	transfer_char(victim, sspell->ch, sspell->ch->in_room,
		      "$N disappears suddenly.",
		      "$n has summoned you!",
		      "$N arrives suddenly.");
	return TRUE;
}

bool spell_teleport(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (victim->in_room == NULL
	|| IS_SET(victim->in_room->room_flags, ROOM_NORECALL))
	{
		char_puts("Teleport unaffected here.\n", sspell->ch);
		return FALSE;
	}

	if ((IS_PC(sspell->ch) && victim->fighting && number_bits(2))
	|| (victim != sspell->ch && saves_spell(sspell->ch, sspell->level - 5, victim,
		DAM_MENTAL, lookup_spell_msc(sspell->sn))))
	{
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	transfer_char(victim, sspell->ch, get_random_room(victim, NULL),
		      "$N vanishes!",
		      "You have been teleported!", 
		      "$N slowly fades into existence.");
	return TRUE;
}

bool spell_bamf(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (victim->in_room == NULL
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  IS_SET(victim->in_room->room_flags, ROOM_PEACE | ROOM_SAFE)) {
		send_to_char("You failed.\n", sspell->ch);
		return TRUE;
	}

	transfer_char(victim, sspell->ch,
		      get_random_room(victim, victim->in_room->area),
		      "$N vanishes!",
		      "You have been teleported.",
		      "$N slowly fades into existence.");
	return TRUE;
}

bool spell_ventriloquate(const spell_spool_t *sspell)
{
	char speaker[MAX_INPUT_LENGTH];
	CHAR_DATA *vch;
	const char * target_name;

	target_name = one_argument(sspell->arg, speaker, sizeof(speaker));

	for (vch = sspell->ch->in_room->people; vch; vch = vch->next_in_room) {
		if (is_name(speaker, vch->name))
			continue;

		if (saves_spell(sspell->ch, sspell->level, vch, DAM_OTHER, lookup_spell_msc(sspell->sn))) {
			act("Someone makes $t say '{G$T{x'",
			    vch, speaker, target_name, TO_CHAR);
		}
		else {
			act("$t says '{G$T{x'",
			    vch, speaker, target_name, TO_CHAR);
		}
	}
	return TRUE;
}

bool spell_weaken(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (is_affected(victim, sspell->sn))
	{
		act("$N is already weaken.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level, victim,DAM_OTHER, lookup_spell_msc(sspell->sn)))
		return TRUE;

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->level;
	af.duration  = (4 + sspell->level / 12);
	af.location  = APPLY_STR;
	af.modifier  = -1 * (2 + sspell->level / 12);
	af.bitvector = AFF_WEAKEN;
	affect_to_char(victim, &af);
	char_puts("You feel your strength slip away.\n", victim);
	act("$n looks tired and weak.",victim,NULL,NULL,TO_ROOM);
	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_word_of_recall(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	ROOM_INDEX_DATA *location;
	class_t *vcl;
	CHAR_DATA *pet;

	if (IS_NPC(victim))
		return FALSE;
	if (victim->fighting
	&&  (vcl = class_lookup(victim->class))
	&&  !CAN_FLEE(victim, vcl)) {
		if (victim == sspell->ch)
			char_puts("Your honour doesn't let you recall!.\n", sspell->ch);
		else
			char_printf(sspell->ch, "You can't cast this spell to a "
					"honourable fighting %s!\n",
				    vcl->name);
		return FALSE;
	}

	if (victim->desc && IS_PUMPED(victim)) {
		act_puts("You are too pumped to pray now.",
			 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return FALSE;
	}

	act("$n prays for transportation!", sspell->ch, NULL, NULL, TO_ROOM);

	if (IS_SET(victim->in_room->room_flags, ROOM_NORECALL)
	||  IS_AFFECTED(victim, AFF_CURSE)
	||  IS_RAFFECTED(victim->in_room, RAFF_CURSE)) {
		char_puts("Spell failed.\n", victim);
		return FALSE;
	}

	if (victim->fighting) {
		if (victim == sspell->ch)
			gain_exp(victim, 0 - (victim->level + 25), FALSE, FALSE);
		stop_fighting(victim, TRUE);
	}

	sspell->ch->move /= 2;
	pet = victim->pet;
	location = get_recall(victim);
	recall(victim, location);

	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			do_stand(pet, str_empty);
		recall(pet, location);
	}
	return TRUE;
}

/*
 * Draconian spells.
 */
bool spell_acid_breath(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam, hp_dam, dice_dam, hpch;

	act("$n spits acid at $N.", sspell->ch,NULL,victim,TO_NOTVICT);
	act("$n spits a stream of corrosive acid at you.", sspell->ch,NULL,victim,TO_VICT);
	act("You spit acid at $N.", sspell->ch,NULL,victim,TO_CHAR);

	hpch = UMAX(12, sspell->ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = dice(sspell->level,16);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	
	if (saves_spell(sspell->ch, sspell->level,victim,DAM_ACID, lookup_spell_msc(sspell->sn)))
	{
		acid_effect(victim, sspell->level/2, dam/4, TARGET_CHAR);
		damage(sspell->ch, victim, dam/2, sspell->sn, DAM_ACID, TRUE);
	}
	else
	{
		acid_effect(victim, sspell->level,dam,TARGET_CHAR);
		damage(sspell->ch,victim,dam, sspell->sn,DAM_ACID,TRUE);
	}
	return TRUE;
}

bool spell_fire_breath(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam;
	int hpch;

	act("$n breathes forth a cone of fire.", sspell->ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a cone of hot fire over you!", sspell->ch,NULL,victim,TO_VICT);
	act("You breath forth a cone of fire.", sspell->ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(10, sspell->ch->hit);
	hp_dam  = number_range(hpch/9+1, hpch/5);
	dice_dam = dice(sspell->level,20);

	dam = UMAX(hp_dam + dice_dam /10, dice_dam + hp_dam / 10);
	fire_effect(victim->in_room, sspell->level,dam/2,TARGET_ROOM);

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch,vch,TRUE)
		||  (IS_NPC(vch) && IS_NPC(sspell->ch) 
		&&  (sspell->ch->fighting != vch /*|| vch->fighting != sspell->ch */)))
		    continue;

		if (vch == victim) /* full damage */
		{
		    if (saves_spell(sspell->ch, sspell->level,vch,DAM_FIRE, lookup_spell_msc(sspell->sn)))
		    {
			fire_effect(vch, sspell->level/2, dam/4, TARGET_CHAR);
			damage(sspell->ch, vch, dam/2, sspell->sn, DAM_FIRE, TRUE);
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
	return TRUE;
}

bool spell_frost_breath(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	CHAR_DATA *vch, *vch_next;
	int dam,hp_dam,dice_dam, hpch;

	act("$n breathes out a freezing cone of frost!", sspell->ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a freezing cone of frost over you!",
		sspell->ch,NULL,victim,TO_VICT);
	act("You breath out a cone of frost.", sspell->ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(12, sspell->ch->hit);
	hp_dam = number_range(hpch/11 + 1, hpch/6);
	dice_dam = dice(sspell->level,16);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	cold_effect(victim->in_room, sspell->level,dam/2,TARGET_ROOM); 

	for (vch = victim->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch,vch,TRUE)
		||  (IS_NPC(vch) && IS_NPC(sspell->ch) 
		&&   (sspell->ch->fighting != vch /*|| vch->fighting != sspell->ch*/)))
		    continue;

		if (vch == victim) /* full damage */
		{
		    if (saves_spell(sspell->ch, sspell->level,vch,DAM_COLD, lookup_spell_msc(sspell->sn)))
		    {
			cold_effect(vch, sspell->level/2, dam/4, TARGET_CHAR);
			damage(sspell->ch, vch, dam/2, sspell->sn, DAM_COLD, TRUE);
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
	return TRUE;
}

	
bool spell_gas_breath(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam,hp_dam,dice_dam,hpch;

	act("$n breathes out a cloud of poisonous gas!", sspell->ch,NULL,NULL,TO_ROOM);
	act("You breath out a cloud of poisonous gas.", sspell->ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(16, sspell->ch->hit);
	hp_dam = number_range(hpch/15+1,8);
	dice_dam = dice(sspell->level,12);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);
	poison_effect(sspell->ch->in_room, sspell->level,dam,TARGET_ROOM);

	for (vch = sspell->ch->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch,vch,TRUE)
		||  (IS_NPC(sspell->ch) && IS_NPC(vch) 
		&&   (sspell->ch->fighting == vch || vch->fighting == sspell->ch)))
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
	return TRUE;
}

bool spell_lightning_breath(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam,hp_dam,dice_dam,hpch;

	act("$n breathes a bolt of lightning at $N.", sspell->ch,NULL,victim,TO_NOTVICT);
	act("$n breathes a bolt of lightning at you!", sspell->ch,NULL,victim,TO_VICT);
	act("You breathe a bolt of lightning at $N.", sspell->ch,NULL,victim,TO_CHAR);

	hpch = UMAX(10, sspell->ch->hit);
	hp_dam = number_range(hpch/9+1,hpch/5);
	dice_dam = dice(sspell->level,20);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

	if (saves_spell(sspell->ch, sspell->level,victim,DAM_LIGHTNING, lookup_spell_msc(sspell->sn)))
	{
		shock_effect(victim, sspell->level/2,dam/4,TARGET_CHAR);
		damage(sspell->ch,victim,dam/2, sspell->sn,DAM_LIGHTNING,TRUE);
	}
	else
	{
		shock_effect(victim, sspell->level,dam,TARGET_CHAR);
		damage(sspell->ch,victim,dam, sspell->sn,DAM_LIGHTNING,TRUE); 
	}
	return TRUE;
}

/*
 * Spells for mega1.are from Glop/Erkenbrand.
 */
bool spell_general_purpose(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;
 
	dam = number_range(25, 100);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_PIERCE, lookup_spell_msc(sspell->sn)))
	    dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_PIERCE ,TRUE);
	return TRUE;
}

bool spell_high_explosive(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;
 
	dam = number_range(30, 120);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_PIERCE, lookup_spell_msc(sspell->sn)))
	    dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn, DAM_PIERCE ,TRUE);
	return TRUE;
}

bool spell_find_object(const spell_spool_t *sspell) 
{
	BUFFER *buffer = NULL;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int number = 0, max_found;

	number = 0;
	max_found = IS_IMMORTAL(sspell->ch) ? 200 : 2 * sspell->level;


	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(sspell->ch, obj) || !is_name(sspell->arg, obj->name)
			|| number_percent() > 2 * sspell->level
			||   sspell->ch->level < obj->level)
		    continue;

		if (buffer == NULL)
			buffer = buf_new(-1);
		number++;

		for (in_obj = obj; in_obj->in_obj != NULL;
						in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL
		&&  can_see(sspell->ch,in_obj->carried_by))
			buf_printf(buffer, "One is carried by %s\n",
				fix_short(PERS(in_obj->carried_by, sspell->ch)));
		else {
			if (IS_IMMORTAL(sspell->ch) && in_obj->in_room != NULL)
				buf_printf(buffer, "One is in %s [Room %d]\n",
					mlstr_cval(in_obj->in_room->name, sspell->ch),
					in_obj->in_room->vnum);
			else
				buf_printf(buffer, "One is in %s\n",
					in_obj->in_room == NULL ?
					"somewhere" :
					mlstr_cval(in_obj->in_room->name, sspell->ch));
		}

		if (number >= max_found)
			break;
	}

	if (buffer == NULL)
		char_puts("Nothing like that in heaven or earth.\n", sspell->ch);
	else {
		page_to_char(buf_string(buffer), sspell->ch);
		buf_free(buffer);
	}
	return TRUE;
}

bool spell_lightning_shield(const spell_spool_t *sspell) 
{
	AFFECT_DATA af,af2;

	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
		char_puts("This room has already shielded.\n", sspell->ch);
		return FALSE;
	}

	if (is_affected(sspell->ch, sspell->sn))
	{
		char_puts("This spell is used too recently.\n", sspell->ch);
		return FALSE;
	}
   
	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 40;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_LSHIELD;
	affect_to_room(sspell->ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sspell->sn;
	af2.level	 = sspell->ch->level;
	af2.duration  = sspell->level / 10;
	af2.modifier  = 0;
	af2.location  = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(sspell->ch, &af2);

	sspell->ch->in_room->owner = str_qdup(sspell->ch->name);
	char_puts("The room starts to be filled with lightnings.\n", sspell->ch);
	act("The room starts to be filled with $n's lightnings.", sspell->ch,NULL,NULL,TO_ROOM);
	return TRUE;
}

bool spell_shocking_trap(const spell_spool_t *sspell) 
{
	AFFECT_DATA af,af2;

	if (is_affected_room(sspell->ch->in_room, sspell->sn))
	{
		char_puts("This room has already trapped with shocks waves.\n", sspell->ch);
		return FALSE;
	}

	if (is_affected(sspell->ch, sspell->sn))
	{
		char_puts("This spell is used too recently.\n", sspell->ch);
		return FALSE;
	}
   
	af.where     = TO_ROOM_AFFECTS;
	af.type      = sspell->sn;
	af.level     = sspell->ch->level;
	af.duration  = sspell->level / 40;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = RAFF_SHOCKING;
	affect_to_room(sspell->ch->in_room, &af);

	af2.where     = TO_AFFECTS;
	af2.type      = sspell->sn;
	af2.level	 = sspell->level;
	af2.duration  = sspell->ch->level / 10;
	af2.modifier  = 0;
	af2.location  = APPLY_NONE;
	af2.bitvector = 0;
	affect_to_char(sspell->ch, &af2);
	char_puts("The room starts to be filled with shock waves.\n", sspell->ch);
	act("The room starts to be filled with $n's shock waves.", sspell->ch,NULL,NULL,TO_ROOM);
	return TRUE;
}

bool spell_acid_arrow(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 12);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ACID, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ACID,TRUE);
	return TRUE;
}


/* energy spells */
bool spell_etheral_fist(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 12);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("A fist of black, otherworldly ether rams into $N, leaving $M looking stunned!"
			, sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ENERGY,TRUE);
	return TRUE;
}

bool spell_spectral_furor(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 8);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("The fabric of the cosmos strains in fury about $N!",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ENERGY,TRUE);
	return TRUE;
}

bool spell_disruption(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 9);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("A weird energy encompasses $N, causing you to question $S continued existence.",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ENERGY,TRUE);
	return TRUE;
}


bool spell_sonic_resonance(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 7);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ENERGY, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("A cylinder of kinetic energy enshrouds $N causing $S to resonate.",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ENERGY,TRUE);
	return TRUE;
}

/* mental */
bool spell_mind_wrack(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 7);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_MENTAL, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("$n stares intently at $N, causing $N to seem very lethargic.",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_MENTAL,TRUE);
	return TRUE;
}

bool spell_mind_wrench(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 9);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_MENTAL, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("$n stares intently at $N, causing $N to seem very hyperactive.",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_MENTAL,TRUE);
	return TRUE;
}

/* acid */
bool spell_sulfurus_spray(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 7);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ACID, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("A stinking spray of sulfurous liquid rains down on $N." ,
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ACID,TRUE);
	return TRUE;
}

bool spell_caustic_font(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 9);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ACID, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("A fountain of caustic liquid forms below $N. The smell of $S degenerating tissues is revolting! ",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ACID,TRUE);
	return TRUE;
}

bool spell_acetum_primus(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 8);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_ACID, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("A cloak of primal acid enshrouds $N, sparks form as it consumes all it touches. ",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_ACID,TRUE);
	return TRUE;
}

/*  Electrical  */

bool spell_galvanic_whip(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 7);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_LIGHTNING, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("$n conjures a whip of ionized particles, which lashes ferociously at $N.",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_LIGHTNING,TRUE);
	return TRUE;
}

bool spell_magnetic_trust(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 8);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_LIGHTNING, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("An unseen energy moves nearby, causing your hair to stand on end!",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_LIGHTNING,TRUE);
	return TRUE;
}

bool spell_quantum_spike(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	dam = dice(sspell->level, 9);
	if (saves_spell(sspell->ch, sspell->level, victim, DAM_LIGHTNING, lookup_spell_msc(sspell->sn)))
		dam /= 2;
	act("$N seems to dissolve into tiny unconnected particles, then is painfully reassembled.",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_LIGHTNING,TRUE);
	return TRUE;
}

/* negative */
bool spell_hand_of_undead(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;

	if (saves_spell(sspell->ch, sspell->level, victim, DAM_NEGATIVE, lookup_spell_msc(sspell->sn))) {
		char_puts("You feel a momentary chill.\n",victim);
		return TRUE;
	}

	if (IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_UNDEAD)) {
		 char_puts("Your victim is unaffected by hand of undead.\n", sspell->ch);
		 return FALSE;
	}

	if (victim->level <= 2)
		dam		 = sspell->ch->hit + 1;
	else {
		dam = dice(sspell->level, 10);
		victim->mana	-= (victim->mana * sspell->level * sspell->percent) / 25000;
		victim->move	-= (victim->move * number_range(1, 4)) / 8;
		sspell->ch->hit		+= dam / number_range(2, 3);
	}

	char_puts("You feel your life slipping away!\n",victim);
	act("$N is grasped by an incomprehensible hand of undead!",
			sspell->ch,NULL,victim,TO_NOTVICT);
	damage(sspell->ch, victim, dam, sspell->sn,DAM_NEGATIVE,TRUE);
	return TRUE;
}

static inline void
astral_walk(CHAR_DATA *ch, CHAR_DATA *victim)
{
	transfer_char(ch, victim, victim->in_room,
		      "$N disappears in a flash of light!",
		      "You travel via astral planes and go to $n.",
		      "$N appears in a flash of light!");
}

/* travel via astral plains */
bool spell_astral_walk(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;
	CHAR_DATA *pet = NULL;

	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->level >= sspell->level + 3
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE; 
	}

	if (sspell->ch->pet && sspell->ch->in_room == sspell->ch->pet->in_room)
		pet = sspell->ch->pet;

	astral_walk(sspell->ch, victim);
	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (pet->position != POS_STANDING)
			do_stand(pet, str_empty);
		astral_walk(pet, victim);
	}
	return TRUE;
}


/* vampire version astral walk */
bool spell_mist_walk(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;


	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->level >= sspell->level - 5
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	transfer_char(sspell->ch, NULL, victim->in_room,
		      "$N dissolves into a cloud of glowing mist, then vanishes!",
		      "You dissolve into a cloud of glowing mist, then flow to your target.",
		      "A cloud of glowing mist engulfs you, then withdraws to unveil $N!");
	return TRUE;
}

/*  Cleric version of astra_walk  */
bool spell_solar_flight(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;

	if  (time_info.hour > 18 || time_info.hour < 8) {
		 char_puts("You need sunlight for solar flight.\n", sspell->ch);
		 return FALSE;
	}

	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->level >= sspell->level + 1
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	transfer_char(sspell->ch, NULL, victim->in_room,
		      "$N disappears in a blinding flash of light!",
		      "You dissolve in a blinding flash of light!",
		      "$N appears in a blinding flash of light!");
	return TRUE;
}

/* travel via astral plains */
bool spell_helical_flow(const spell_spool_t *sspell)
{
	CHAR_DATA *victim;


	if ((victim = get_char_world(sspell->ch, sspell->arg)) == NULL
	||  victim->level >= sspell->level + 3
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_OTHER, lookup_spell_msc(sspell->sn))
	||  !can_gate(sspell->ch, victim)) {
		char_puts("You failed.\n", sspell->ch);
		return TRUE;
	}

	transfer_char(sspell->ch, NULL, victim->in_room,
		      "$N coils into an ascending column of colour, vanishing into thin air.",
		      "You coil into an ascending column of colour, vanishing into thin air.",
		      "A coil of colours descends from above, revealing $N as it dissipates.");
	return TRUE;
}

bool spell_corruption(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_CORRUPTION)) {
		act("$N is already corrupting.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (IS_IMMORTAL(victim)
	||  saves_spell(sspell->ch, sspell->level, victim, DAM_NEGATIVE, lookup_spell_msc(sspell->sn))
	||  (IS_NPC(victim) && IS_SET(victim->pIndexData->act, ACT_UNDEAD))) {
		if (sspell->ch == victim)
			act_puts("You feel momentarily ill, but it passes.",
				 sspell->ch, NULL, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts("$N seems to be unaffected.",
				 sspell->ch, NULL, victim, TO_CHAR, POS_DEAD);
		return TRUE;
	}

	af.where     = TO_AFFECTS;
	af.type 	 = sspell->sn;
	af.level	 = sspell->level * 3/4;
	af.duration  = (10 + sspell->level / 5);
	af.location  = APPLY_NONE;
	af.modifier  = 0; 
	af.bitvector = AFF_CORRUPTION;
	affect_join(victim,&af);

	act("You scream in agony as you start to decay into dust.",
	    victim, NULL, NULL, TO_CHAR);
	act("$n screams in agony as $n start to decay into dust.",
	    victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_hurricane(const spell_spool_t *sspell)
{
	CHAR_DATA *vch;
	CHAR_DATA *vch_next;
	int dam,hp_dam,dice_dam,hpch;

	act("$n prays the gods of the storm for help.", sspell->ch,NULL,NULL,TO_NOTVICT);
	act("You pray the gods of the storm to help you.", sspell->ch,NULL,NULL,TO_CHAR);

	hpch = UMAX(16, sspell->ch->hit);
	hp_dam = number_range(hpch/15+1,8);
	dice_dam = dice(sspell->level,12);

	dam = UMAX(hp_dam + dice_dam/10,dice_dam + hp_dam/10);

	for (vch = sspell->ch->in_room->people; vch != NULL; vch = vch_next)
	{
		vch_next = vch->next_in_room;

		if (is_safe_spell(sspell->ch,vch,TRUE)
		||  (IS_NPC(sspell->ch) && IS_NPC(vch) 
		&&   (sspell->ch->fighting == vch || vch->fighting == sspell->ch)))
		    continue;

		if (!IS_AFFECTED(vch,AFF_FLYING)) dam /= 2;

		if (vch->size == SIZE_TINY)  dam *= 1.5;
		else if (vch->size == SIZE_SMALL)  dam *= 1.3;
		else if (vch->size == SIZE_MEDIUM)  dam *= 1;
		else if (vch->size == SIZE_LARGE)  dam *= 0.9;
		else if (vch->size == SIZE_HUGE)  dam *= 0.7;
		else dam *= 0.5;

		if (saves_spell(sspell->ch, sspell->level,vch,DAM_OTHER, lookup_spell_msc(sspell->sn)))
		    damage(sspell->ch,vch,dam/2, sspell->sn,DAM_OTHER,TRUE);
		else
		    damage(sspell->ch,vch,dam, sspell->sn,DAM_OTHER,TRUE);
	}
	return TRUE;
}


bool spell_detect_undead(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;

	if (IS_AFFECTED(victim, AFF_DETECT_UNDEAD)) {
		if (victim == sspell->ch)
			char_puts("You can already sense undead.\n", sspell->ch);
		else
			act("$N can already detect undead.",
			    sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	af.where     = TO_AFFECTS;
	af.type      = sspell->sn;
	af.level	 = sspell->level;
	af.duration  = (5 + sspell->level / 3);
	af.modifier  = 0;
	af.location  = APPLY_NONE;
	af.bitvector = AFF_DETECT_UNDEAD;
	affect_to_char(victim, &af);
	char_puts("Your eyes tingle.\n", victim);
	if (sspell->ch != victim)
		char_puts("Ok.\n", sspell->ch);
	return TRUE;
}

bool spell_take_revenge(const spell_spool_t *sspell)
{
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	ROOM_INDEX_DATA *room = NULL;
	bool found = FALSE;
 
	if (IS_NPC(sspell->ch) && !IS_GHOST(sspell->ch)) {
		char_puts("It is too late to take revenge.\n", sspell->ch);
		return FALSE;
	}

	for (obj = object_list; obj; obj = obj->next) {
		if (obj->pIndexData->vnum != OBJ_VNUM_CORPSE_PC
		||  !IS_OWNER(sspell->ch, obj))
			continue;

		found = TRUE;
		for (in_obj = obj; in_obj->in_obj; in_obj = in_obj->in_obj)
			;

		if (in_obj->carried_by != NULL)
			room = in_obj->carried_by->in_room;
		else
			room = in_obj->in_room;
		break;
	}

	if (!found || room == NULL)
		char_puts("Unluckily your corpse is devoured.\n", sspell->ch);
	else
		transfer_char(sspell->ch, NULL, room, NULL, NULL, NULL);
	return TRUE;
}

bool spell_mana_pool(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	
	if (victim->mana >= victim->max_mana)
	    return FALSE;
	victim->mana = UMIN(victim->mana + 100 + sspell->level / 10, victim->max_mana);
	char_puts("You feel powerful.\n", victim);
	if (sspell->ch != victim) {
	    char_puts("Ok.\n", sspell->ch);
	    change_faith(sspell->ch, RELF_CURE_OTHER, 0);
	}
	return TRUE;
}
