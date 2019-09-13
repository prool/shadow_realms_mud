/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * Shadow Realms 2001 - 2002
 */

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

bool get_random_ill(AFFECT_DATA *aff, int power);
flag32_t lookup_spell_msc(int sn);

bool spell_feebling(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	AFFECT_DATA af;
	int power;

	if (IS_AFFECTED(victim, AFF_FEEBLING))
	{
		act("$N already feel self very feeble.", sspell->ch, NULL, victim, TO_CHAR);
		return FALSE;
	}

	if (saves_spell(sspell->ch, sspell->level,victim,DAM_DISEASE, lookup_spell_msc(sspell->sn))
	|| !(get_random_ill(&af, (power = number_range(1, UMAX(2, sspell->percent / 10 + sspell->level / 8))))))
	{
		char_puts("You feel momentarily ill, but it passes.\n",victim);
		act("$n turns slightly green, but it passes.",victim,NULL,NULL,TO_ROOM);
		return TRUE;
	}

	if (!saves_spell(sspell->ch, sspell->level - 8,victim,DAM_DISEASE, lookup_spell_msc(sspell->sn))
	&& power > 4)
	{
		af.bitvector |= AFF_FEEBLING;
		char_puts("You feel very feeble.\n", victim);
		act("$n looks very feeble.",victim,NULL,NULL,TO_ROOM);
	} else
	{
		char_puts("You feel feeble.\n", victim);
		act("$n looks very feeble.",victim,NULL,NULL,TO_ROOM);
	}

	affect_join(victim, &af);

	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_light_stream(const spell_spool_t *sspell)
{
	bool ret = FALSE;
	OBJ_DATA *obj;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	
	if (room_is_dark(victim))
		return FALSE;

	if ((obj = get_eq_char(sspell->ch, WEAR_LIGHT))
	&& obj->pIndexData->item_type == ITEM_LIGHT
	&& obj->value[2] < obj->pIndexData->value[2]
	&& number_percent() < sspell->level / 5 + sspell->percent / 5)
	{
		obj->value[2] = UMIN(obj->value[2] + sspell->level / 45 + 1,
			obj->pIndexData->value[2]);
		ret = TRUE;
	}
	
	if (victim->move < victim->max_move
	&& number_percent() < sspell->level / 5 + sspell->percent / 5)
	{
		victim->move = UMIN(victim->move + sspell->level / 10 + 3,
			victim->max_move);
		ret = TRUE;
	}
	
	if (victim->mana < victim->max_mana
	&& number_percent() < sspell->level / 5 + sspell->percent / 5)
	{
		victim->mana = UMIN(victim->mana + sspell->level / 5,
			victim->max_mana);
		ret = TRUE;
	}
	
	if (victim->hit < victim->max_hit
	&& number_percent() < sspell->level / 5 + sspell->percent / 5)
	{
		victim->hit = UMIN(victim->hit + sspell->level / 5 + 5,
			victim->max_hit);
		ret = TRUE;
	}
	
	if (ret)
	{
		act("You feel warm of sun day.", victim, NULL, NULL, TO_CHAR);
		act("A warm of light touch $n.", victim, NULL, NULL, TO_ROOM);
	}

	return ret;
}

bool spell_sunstroke(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int dam;
	AFFECT_DATA af;
	
	if (room_is_dark(victim))
		return FALSE;

	DAZE_STATE(victim, PULSE_VIOLENCE * number_range(1, sspell->level / 10));

	dam = dice(sspell->level, UMAX(4, sspell->level / 7));

	if (saves_spell(sspell->ch, sspell->level - 5, victim, DAM_FIRE, lookup_spell_msc(sspell->sn))
	|| number_bits(1))
	{
		if (saves_spell(sspell->ch, sspell->level + 5, victim, DAM_FIRE, lookup_spell_msc(sspell->sn)))
		{
			if (number_bits(2) == 0)
				dam /= 2;
		} else if (!is_affected(victim, sspell->sn))
		{
			af.where	= TO_AFFECTS;
			af.type		= sspell->sn;
			af.level	= sspell->level;
			af.duration	= number_range(1, 1 + sspell->level / 10);
			af.location	= APPLY_HITROLL;
			af.modifier 	= - sspell->level / 8;
			af.bitvector	= 0;
			affect_to_char(victim, &af);
		}
		damage(sspell->ch, victim, dam, sspell->sn, DAM_FIRE ,TRUE);
	} else {
		if (victim->fighting)
			stop_fighting(victim, TRUE);

		af.where	= TO_AFFECTS;
		af.type		= sspell->sn;
		af.level	= sspell->level;
		af.duration	= 1 + sspell->level / 20;
		af.location	= APPLY_STR;
		af.modifier	= -1 * (2 + sspell->level / 12);
		af.bitvector	= AFF_WEAKEN | AFF_SLEEP;
		affect_to_char(victim, &af);
		victim->position = POS_SLEEPING;
	}

	act("Sunstroke damages your mind.", victim, NULL, NULL, TO_CHAR);
	act("Sunstroke damages $n's mind.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_unhonor(const spell_spool_t *sspell)
{
	AFFECT_DATA af;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;

	if (is_affected(victim, sspell->sn))
		return FALSE;
	
	af.where	= TO_AFFECTS;
	af.type		= sspell->sn;
	af.level	= sspell->level;
	af.duration	= UMAX(1, sspell->level / 10);
	af.bitvector	= 0;
	af.modifier	= -1;
	
	af.location	= APPLY_STR;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_INT;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_WIS;
	affect_to_char(victim, &af);
	
	af.location	= APPLY_DEX;
	affect_to_char(victim, &af);
	
	char_puts("Your conscience has awake.", victim);
	act("$n's conscience has awake.", sspell->ch, NULL, NULL, TO_ROOM);
	
	if (sspell->ch != victim)
		change_faith(sspell->ch, RELF_CAST_MALADICTION, 0);
	return TRUE;
}

bool spell_taxes(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int amount;
	int s_g, s_s;
	int s1;
	int s2;
	bool silver;
	
	s_g = victim->gold + IS_PC(victim) ? victim->pcdata->bank_g : 0;
	s_s = victim->silver + IS_PC(victim) ? victim->pcdata->bank_s : 0;
	s1 = s_g * 100 + s_s;
	s2 = number_range(URANGE(2, s1 / 100, 5), URANGE(10, s1 / 30, 300));
	
	if (s2 <= s_s)
	{
		if (victim->silver >= s2)
			victim->silver -= s2;
		else {	/* IS_PC !!! */
			victim->pcdata->bank_s -= (s2 - victim->silver);
			victim->silver = 0;
		}
		amount = s2;
		silver = TRUE;
	} else
	{
		amount = s2 / 100 + 1;
		if (s_g < amount)
		{
			RELIGION_DATA *r = GET_CHAR_RELIGION(victim);
	 
			victim->silver = victim->gold = 0;
			if (IS_PC(victim))
				victim->pcdata->bank_s =
					victim->pcdata->bank_g = 0;
			act("Cruel taxes find your savings and capture all money.",
				victim, NULL, NULL, TO_CHAR);
			act("$t dislike your empty pockets!", victim,
				r ? r->name : "Gods", NULL, TO_CHAR);
			act("Cruel taxes find $n's savings and capture all $s money.",
				victim, NULL, NULL, TO_ROOM);
			if (r)
				reduce_faith(victim, UMIN(15, s2 * 2), FALSE);
			return TRUE;
		} else if (victim->gold >= amount)
			victim->gold -= amount;
		else {	/* IS_PC !!! */
			victim->pcdata->bank_g -= (amount - victim->gold);
			victim->gold = 0;
		}
		silver = FALSE;
	}

	act("Cruel taxes find your savings and capture $j $T.", victim, (const void*) amount, silver ? "silver" : "gold", TO_CHAR | ACT_TRANS);
	act("Cruel taxes find $n's savings and capture some $s money.", victim, NULL, NULL, TO_ROOM);
	return TRUE;
}

bool spell_swindle(const spell_spool_t *sspell)
{
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int silver, gold;
	
	silver = gold = 0;
	if (number_percent() * victim->silver > number_percent() * victim->gold)
	{
		if (victim->silver)
			victim->silver -= (silver = number_range(1, UMIN(66, victim->silver / 5 + 1)));
	} else {
		if (victim->gold)
			victim->gold -= (gold = number_range(1, UMIN(8, victim->gold / 8 + 1)));
	}
	
	if (silver || gold) {
		int amount = silver ? silver : gold;

		act_puts3("Your swindle extract $J $t from $N's pockets.",
			sspell->ch, silver ? "silver" : "gold", victim,
			(const void*) amount, TO_CHAR | ACT_TRANS, POS_DEAD);
		act_puts3("$n's swindle extract $J $t from your pockets.",
			sspell->ch, silver ? "silver" : "gold", victim,
			(const void*) amount, TO_VICT | ACT_TRANS, POS_RESTING);
		act("$n's swindle extract some money from $N's pockets.", sspell->ch, NULL, victim, TO_NOTVICT);
	}
	return TRUE;
}

bool spell_remove_tattoo(const spell_spool_t *sspell)
{
	OBJ_DATA *tattoo;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
 
	tattoo = get_eq_char(victim, WEAR_TATTOO);
	if (tattoo != NULL)
	{
	  extract_obj(tattoo, 0);
	  act("Through a painful process, your tattoo has been destroyed by $n.",
	sspell->ch, NULL, victim, TO_VICT);
	  act("You remove the tattoo from $N.", sspell->ch, NULL, victim, TO_CHAR);
	  act("$N's tattoo is destroyed by $n.", sspell->ch, NULL, victim, TO_NOTVICT);
	  return TRUE;
	}
	else
	act("$N doesn't have any tattoos.", sspell->ch, NULL, victim, TO_CHAR);
	return FALSE;
}

/*
 *	Generialka randomnoi boliachki :)
 *		by Xor for ^SR^
 */
bool get_random_ill(AFFECT_DATA *af, int power)
{
	int loc;	// apply location
	int mod;
	int np = ((130 - number_range(1, 60)) * power) / 100;	// (+-)30%

	if (np <= 0)
		return FALSE;

	if (np >= 10 && number_percent() < 40)
	{
		switch(number_range(1, 11))
		{
			default:
				loc = APPLY_STR;
				mod = 1 + np / 15;
				break;
			case 3:
			case 4:
				loc = APPLY_DEX;
				mod = 1 + np / 15;
				break;
			case 5:
			case 6:
				loc = APPLY_CON;
				mod = 1 + np / 15;
				break;
			case 7:
			case 8:
				loc = APPLY_CHA;
				mod = 1 + np / 14;
				break;
			case 9:
				if (number_bits(2)) {
					loc = APPLY_LEVEL;
					mod = 1 + np / 15;
				} else {
					loc = APPLY_SIZE;
					mod = 1 + np / 25;
				}
				break;
			case 10:
				loc = APPLY_INT;
				mod = 1 + np / 18;
				break;
			case 11:
				loc = APPLY_WIS;
				mod = 1 + np / 20;
				break;
		}
	} else {
		switch(number_range(1, 9))
		{
			default:
				loc = APPLY_HIT;
				mod = np * 2.5;
				break;
			case 2:
				loc = APPLY_MOVE;
				mod = np * 2;
				break;
			case 3:
				loc = APPLY_MANA;
				mod = np * 3;
				break;
			case 4:
				loc = APPLY_HITROLL;
				mod = np;
				break;
			case 5:
				loc = APPLY_DAMROLL;
				mod = np * 1.8;
				break;
			case 6:
				loc = APPLY_SAVES;
				mod = np * 1.5;
				break;
			case 7:
				switch(number_range(1, 6))
				{
					default:loc = APPLY_SAVING_EARTH;
						break;
					case 2:	loc = APPLY_SAVING_FIRE;
						break;
					case 3:	loc = APPLY_SAVING_AIR;
						break;
					case 4: loc = APPLY_SAVING_WATER;
						break;
					case 5:	loc = APPLY_SAVING_LIFE;
						break;
					case 6:	loc = APPLY_SAVING_MENTAL;
						break;
				}
				mod = np * 3;
				break;
			case 8:
				loc = APPLY_AC;
				mod = np * 2;
				break;
			case 9:
				loc = APPLY_EXP;
				mod = np / 1.5;
				break;
		}
	}
	
	if (!mod)
		return FALSE;

	af->where	= TO_AFFECTS;
	af->type	= gsn_feebling;
	af->level	= 10 + power * 2;
	af->duration	= number_range(0, power);
	af->location	= loc;
	af->modifier	= - mod;
	af->bitvector	= 0;

	return TRUE;
}

#if 0
	/* in skills.conf */
#SKILL
Name tattoo~
Type spell
Group none
SpellFun spell_tattoo
Target chardef
MinPos stand
MinMana 10
Slot 551
Msc earth~
End

void spell_tattoo(const spell_spool_t *sspell)
{
	OBJ_DATA *tattoo;
	CHAR_DATA *victim = (CHAR_DATA *) sspell->vo;
	int i;

	if (IS_NPC(victim))
	{
	  act("$N is too dumb to worship you!", sspell->ch, NULL, victim, TO_CHAR);
	  return;
	}

	for (i = 0; i < MAX_RELIGION; i++)
	{
	  if (!str_cmp(ch->name, religion_name(i)))
	{
		  tattoo = get_eq_char(victim, WEAR_TATTOO);
	 	  if (tattoo != NULL)
		    {
	 	      act("$N is already tattooed!  You'll have to remove it first.",
		ch, NULL, victim, TO_CHAR);
	  	      act("$n tried to give you another tattoo but failed.",
		ch, NULL, victim, TO_VICT);
	          act("$n tried to give $N another tattoo but failed.",
		ch, NULL, victim, TO_NOTVICT);
	  	      return;
		    }
		  else
		    { 
	  	      tattoo = create_obj(get_obj_index(RELIGION(i)->vnum_tattoo),0);
	   	      act("You tattoo $N with $p!",ch, tattoo, victim, TO_CHAR);
	  	      act("$n tattoos $N with $p!",ch,tattoo,victim,TO_NOTVICT);
	  	      act("$n tattoos you with $p!",ch,tattoo,victim,TO_VICT);

	  	      obj_to_char(tattoo,victim);
	  	      equip_char(victim, tattoo, WEAR_TATTOO);
	      return;
	    }
	}    
	}
	char_puts("You don't have a religious tattoo.\n", ch);
	return;    
}
#endif
