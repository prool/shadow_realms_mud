/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: religion.c,v 1.33 2009/08/12 14:14:10 mudsr Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include "merc.h"

/* vnums for tattoos */
#define OBJ_VNUM_TATTOO_ATUM_RA 	51
#define OBJ_VNUM_TATTOO_ZEUS		52
#define OBJ_VNUM_TATTOO_SIEBELE 	53
#define OBJ_VNUM_TATTOO_SHAMASH		54
#define OBJ_VNUM_TATTOO_AHURAMAZDA	55
#define OBJ_VNUM_TATTOO_EHRUMEN 	56
#define OBJ_VNUM_TATTOO_DEIMOS		57
#define OBJ_VNUM_TATTOO_PHOBOS		58
#define OBJ_VNUM_TATTOO_ODIN		59
#define OBJ_VNUM_TATTOO_TESHUB		60
#define OBJ_VNUM_TATTOO_ARES		61
#define OBJ_VNUM_TATTOO_GOKTENGRI	62
#define OBJ_VNUM_TATTOO_HERA		63
#define OBJ_VNUM_TATTOO_VENUS		64
#define OBJ_VNUM_TATTOO_SETH		65
#define OBJ_VNUM_TATTOO_ENKI		66
#define OBJ_VNUM_TATTOO_EROS		67

/* God's Name, name of religion, tattoo vnum  */
/*     Rudiment :)))))
const struct religion_type religion_table [] =
{
  { str_empty,		"None",			0			},
  { "Atum-Ra",		"Lawful Good",		OBJ_VNUM_TATTOO_ATUM_RA },
  { "Zeus",		"Neutral Good",		OBJ_VNUM_TATTOO_ZEUS	},
  { "Siebele",		"True Neutral",		OBJ_VNUM_TATTOO_SIEBELE },
  { "Shamash",		"God of Justice",	OBJ_VNUM_TATTOO_SHAMASH	},
  { "Ehrumen",		"Chaotic Evil",		OBJ_VNUM_TATTOO_EHRUMEN	},
  { "Ahuramazda",	"Chaotic Good",		OBJ_VNUM_TATTOO_AHURAMAZDA },
  { "Deimos",		"Lawful Evil",		OBJ_VNUM_TATTOO_DEIMOS	},
  { "Phobos",		"Neutral Evil",		OBJ_VNUM_TATTOO_PHOBOS	},
  { "Odin",		"Lawful Neutral",	OBJ_VNUM_TATTOO_ODIN	},
  { "Teshub",		"Chaotic Neutral",	OBJ_VNUM_TATTOO_TESHUB	},
  { "Ares",		"God of War",		OBJ_VNUM_TATTOO_ARES	},
  { "Goktengri",	"God of Honor",		OBJ_VNUM_TATTOO_GOKTENGRI },
  { "Hera",		"Goddess of Hate",	OBJ_VNUM_TATTOO_HERA	},
  { "Venus",		"Goddess of Beauty",	OBJ_VNUM_TATTOO_VENUS	},
  { "Seth",		"God of Anger",		OBJ_VNUM_TATTOO_SETH	},
  { "Enki",		"God of Knowledge",	OBJ_VNUM_TATTOO_ENKI	},
  { "Eros",		"God of Love",		OBJ_VNUM_TATTOO_EROS	}
};
*/

varr	religions = {sizeof(RELIGION_DATA), 2 };

flag_t	religion_flags[] =
{
	{ str_empty,		TABLE_BITVAL			},
	{ "auction_free",	RELIG_AUC_FREE,		TRUE	},
	{ "greed",		RELIG_GREED,		TRUE	},
	{ "extra_hit",		RELIG_EXTRA_HIT,	TRUE	},
	{ "cheap_healer",	RELIG_CHEAP_HEALER,	TRUE	},
	{ "check_benediction",	RELIG_CHECK_BENEDICTION,TRUE	},
	{ "advanced_damage",	RELIG_ADVANCE_DAMAGE,	TRUE	},
	{ "dark_maladiction",	RELIG_DARK_MALADICTION,	TRUE	},
	
	{ NULL }
};

flag_t religion_faith_flags[] =
{
	{ str_empty,		TABLE_BITVAL			},
	{ "sacrifice_obj",	RELF_SACRIFICE_OBJ,	TRUE	},
	{ "recall",		RELF_RECALL,		TRUE	},
	{ "reborn",		RELF_REBORN,		TRUE	},
	{ "death",		RELF_DEATH,		TRUE	},
	{ "put_obj_altar",	RELF_PUT_OBJ_ALTAR,	TRUE	},
	{ "do_rescue",		RELF_DO_RESCUE,		TRUE	},
	{ "fight_strong_victim",   RELF_FI_STRONG_V,	TRUE	},
	{ "fight_bare_pc_victim",  RELF_FI_BARE_PC_V,	TRUE	},
	{ "do_flee",		RELF_DO_FLEE,		TRUE	},
	{ "get_obj_pc_corpse",	RELF_DO_GET_OBJ_PC_COR,	TRUE	},
	{ "cure_other",		RELF_CURE_OTHER,	TRUE	},
	{ "benediction_other",	RELF_BENEDICTION_OTHER,	TRUE	},
	{ "aid_other",		RELF_AID_OTHER,		TRUE	},
	{ "kill_pc",		RELF_KILL_PC,		TRUE	},
	{ "kill_strong_npc",	RELF_KILL_STRONG_NPC,	TRUE	},
	{ "drink_liq",		RELF_DRINK_LIQ,		TRUE	},
	{ "drink_nonliq",	RELF_DRINK_NONLIQ,	TRUE	},
	{ "join_group",		RELF_JOIN_GROUP,	TRUE	},
	{ "kill_strong_evil",	RELF_KILL_STRONG_EVIL,	TRUE	},
	{ "kill_good",		RELF_KILL_GOOD,		TRUE	},
	{ "fight_nonevil",	RELF_FIGHT_NONEVIL,	TRUE	},
	{ "kill_strong_good",	RELF_KILL_STRONG_GOOD,	TRUE	},
	{ "kill_evil",		RELF_KILL_EVIL,		TRUE	},
	{ "cast_maladiction",	RELF_CAST_MALADICTION,	TRUE	},
	{ "get_obj_altar",	RELF_GET_OBJ_ALTAR,	TRUE	},
	
	{ NULL }
};

int rel_lookup(const char *name)
{
	int i;
	for (i = 0; i < religions.nused; i++)
		if (!str_prefix(name, RELIGION(i)->name))
			return i;
	return -1;
}

int rlook_god(int vnum)
{
	int i;
	for (i = 0; i < religions.nused; i++)
		if (RELIGION(i)->vnum_god == vnum)
			return i;

	return -1;
}

const char *religion_name(int religion)
{
	return RELIGION(religion)->name;
}

altar_t *get_altar(CHAR_DATA *ch)
{
	if (GET_CHAR_RELIGION(ch) == NULL)
		return get_altar_ateist(ch);

	return &(GET_CHAR_RELIGION(ch)->altar);
}

void do_reincarnation(CHAR_DATA *ch, const char *argument)
{
	int wait_time;

	if (!IS_PC(ch))
		return;

	if (argument[0] == '\0'
	|| argument[0] == '?')
	{
		char_puts("Syntax: reincarnation {{ do | info }\n", ch);
		return;
	}

	if (!IS_GHOST(ch))
	{
		char_puts("Your soul already have body.\n", ch);
		return;
	}
	
	wait_time = WAIT_TIME_REINCARNATION(ch);
	if (!str_prefix(argument, "info"))
	{
		char_printf(ch, "You have %d seconds before compulsory reincarnation.\n",
			wait_time);
		if (wait_time <= 60)
			char_puts("You may painless return to your normal form near your altar.\n", ch);		
		return;
	} else if (!str_prefix(argument, "do"))
	{
		if (ch->in_room != get_altar(ch)->room)
		{
			char_puts("You cann't reincarnation in this room.\n", ch);
			return;
		}

		if (GET_CHAR_RELIGION(ch))
		{
			char_printf(ch, "%s breathed your soul in new body.\n",
				GET_CHAR_RELIGION(ch)->name);
			if (wait_time > 60)
				change_faith(ch, RELF_REBORN, wait_time);
		} else {
			if (wait_time > 60)
			{
				char_puts("You cann't reincarnation now without patronage of god.\n", ch);
				return;
			}
			char_puts("Your {csoul{x materialised in new body. Praise to gods!\n", ch);
		}
		remove_ghost(ch);
	} else {
		do_reincarnation(ch, str_empty);
	}
}

DECLARE_DO_FUN(do_say	);
void do_religion(CHAR_DATA *ch, const char *argument)
{

	char arg[MAX_INPUT_LENGTH];
	
	if (IS_NPC(ch))
		return;
	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
	{
		char_puts("Syntax: religion {{ info [god] | devote <god> | refuse }\n",
			ch);
	} else if (!str_prefix(arg, "info")) {
		int rel;
		struct religion_type *r;

		one_argument(argument, arg, sizeof(arg));
		if ((	arg[0] == '\0'
			&& (r = GET_CHAR_RELIGION(ch)))
		|| (	arg[0] != '\0'
			&& (rel = rel_lookup(arg)) > 0
			&& (r = RELIGION(rel)))
		)
		{
			int i;

			char_printf(ch, "   %s\n", r->name);
			send_to_char(mlstr_cval(r->desc, ch), ch);
			char_printf(ch, "Templeman of this god is %s.\n",
				mlstr_cval(get_mob_index(r->vnum_templeman)->short_descr, ch));
			char_printf(ch, "Special ability(es) %s is '%s'\n",
				r->name, flag_string(religion_flags, r->flags));

			for (i = 0; i < r->add_skills.nused; i++)
			{
				religion_add_skill *sk = VARR_GET(&r->add_skills, i);
				char_printf(ch, "This god give '%s' on %d level with %d max percent and add %d percent with %d hard.\n",
					SKILL(sk->sn)->name, sk->level, sk->max_percent,
					sk->add_percent, sk->hard);
			}
			
			for (i = 0; i < r->fight_spells.nused; i++)
			{
				religion_fight_spell *sp = VARR_GET(&r->fight_spells, i);
				char_printf(ch, "After your actions wich %s to %s, he may cast %s (%d%%) on %s for %s.\n",
					sp->is_for_honorable ? "like" : "dislike",
					r->name, SKILL(sp->sn)->name,
					sp->percent,
					sp->to_char ? "you" : "victim",
					sp->is_for_honorable ? "help you" : "give you a lesson");
			}
			
			for (i = 0; i < r->change_faith.nused; i++)
			{
				religion_faith_change *fc = VARR_GET(&r->change_faith, i);
				char_printf(ch, "%s %s '%s'.\n",
					r->name, fc->is_like ? "like" : "dislike",
					flag_string(religion_faith_flags, fc->that));
			}
			
			char_printf(ch, "After your death you will wait %d seconds plus %d multiple on your level.\n",
				r->ghost_timer_default,	r->ghost_timer_plevel);

			if (r->allow_align)
				char_printf(ch, "This religion may devote only next alignment(s): %s\n",
					flag_string(ralign_names, r->allow_align));

			if (GET_CHAR_RELIGION(ch) == r)
			{
				char_printf(ch, "Your '%s' being in '{W%s{x'.\n",
					mlstr_cval(r->altar.pit->short_descr, ch),
					mlstr_cval(r->altar.room->name, ch));
				char_printf(ch, "Your current well-disposes of %s is {c%d{x(%d).\n",
					r->name, get_char_faith(ch), ch->pcdata->religion->faith);
			} else {
				char_printf(ch, "For begin way of honour this god you must have %d gold and %d quest points.\n",
					r->cost_gold, r->cost_qp);
			}
			return;
		}
		char_puts("Unknown god.\n", ch);
	} else if (!str_prefix(arg, "devote")) {
		const char *notfind = "You cann't find templeman of this god.\n";
		struct religion_type *r;
		int rel;
		CHAR_DATA *mob;
		int i;
		
		if (GET_CHAR_RELIGION(ch))
		{
			char_printf(ch, "You already serve to %s.\n", ch->pcdata->religion->data->name);
			reduce_faith(ch, 2, TRUE);
			return;
		}

		one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0'
		|| (rel = rel_lookup(arg)) < 0
		|| (r = RELIGION(rel)) == NULL)
		{
			char_puts(notfind, ch);
			return;
		}
		for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
			if (IS_NPC(mob) && mob->pIndexData->vnum == r->vnum_templeman)
				break;
		if (!mob)
		{
			char_puts(notfind, ch);
			return;
		}
		
		if (r->allow_align) {
			if (!IS_SET(r->allow_align, RALIGN(ch)))
			{
				do_say(mob, "You align restrict for my religion.");
				return;
			}
		}
		
		if (ch->gold + ch->silver / 100 < r->cost_gold)
		{
			doprintf(do_say, mob,
				"You have not enouth gold for my service.");
			return;
		}
		if (ch->pcdata->questpoints < r->cost_qp)
		{
			doprintf(do_say, mob,
				"You did not enouth quests for proof of worthy for serve my god.");
			return;
		}
		
		deduct_cost(ch, r->cost_gold * 100);
		ch->pcdata->questpoints -= r->cost_qp;

		ch->pcdata->religion = new_religion_char();
		ch->pcdata->religion->data = r;
			/* add percent to skills (/spells) */
		update_skills(ch);
		for (i = 0; i < r->add_skills.nused; i++)
		{
			religion_add_skill *ask = VARR_GET(&r->add_skills, i);
			pcskill_t *ps = ask ? pcskill_lookup(ch, ask->sn) : NULL;
			
			if (!ask || !ps)
				continue;
			ps->percent = UMIN(ps->maxpercent, ps->percent + ask->add_percent);
		}

		doprintf(do_say, mob,
			"From now on and forever, you are in the way of %s",
			r->name);
	} else if (!str_prefix(arg, "refuse")) {
		char_puts("Under construction :)\n", ch);
		/*
		 *	1) Tattoo пропадает и - max_hp ! :)
		 *	2) Отнять скиллы которые даны (и add_percent тоже)
		 *		ломает писать... как-нить потом :)))
		 *		by Xor
		 */
	} else
		do_religion(ch, str_empty);

}

int get_char_faith(CHAR_DATA *ch)
{
	RELIGION_DATA *rel;
	int faith;
	
	if ((rel = GET_CHAR_RELIGION(ch)) == NULL)
		return 0;
	faith = ch->pcdata->religion->faith;

	if (faith > 0)
	{
		if (GET_TATTOO(ch) == NULL)
			faith /= 2;
	} else {
	}
	
	return URANGE(-10000, faith, 10000);
}

/*
 *	Change faith after action (see all *.c ;))
 *	Return changed value...
 *	Check only one bit!
 */
int change_faith(CHAR_DATA *ch, flag32_t that, int change)
{
	struct religion_type *r;
	int i;
	int ret = 0;
	
	if ((r = GET_CHAR_RELIGION(ch)) == NULL
	|| !IS_SET(r->change_flags, that))
		return ret;
	
	for (i = 0; i < r->change_faith.nused; i++)
	{
		religion_faith_change *fc = VARR_GET(&r->change_faith, i);
		if (IS_SET(fc->that, that))
		{
			switch(that)
			{
				default:
					ret = fc->value;
					break;
				case RELF_SACRIFICE_OBJ:	// change = obj->cost
					ret = change > 150 ?
						(10 + (change - 150) / 20) :
						(change / 15);
					ret = URANGE(0, ret, 40);
					ret = (fc->value * ret) / 100;
					break;
				case RELF_PUT_OBJ_ALTAR:	// change = obj->cost
					ret = URANGE(0, change / 10, 50);
					ret = (fc->value * ret) / 100;
					break;
				case RELF_GET_OBJ_ALTAR:	// change = obj->cost
					ret = URANGE(0, change / 9, 60);
					ret = (fc->value * ret) / 100;
					break;
				case RELF_FI_STRONG_V:		// change = victim->level - ch->level
					if (change > 0)
						ret = fc->value;
					break;
			}
			if (ret) {
				if(fc->is_like)
					incrase_faith(ch, ret, TRUE);
				else {
					reduce_faith(ch, ret, TRUE);
					ret *= -1;
				}
			}
			return ret;
		}
	}
	bug("change_faith: error in rel->change_flags ", 0);
	return ret;
}

static bool send_like_message(CHAR_DATA *ch)
{
	int mod = ch->pcdata->religion->mod_faith;
	const char * name;
	
	if (abs(mod) < 10)		// anti-spam control - nice :)) gy-gy
		return FALSE;

	name = GET_CHAR_RELIGION(ch)->name;	

	if (mod >= 100)
		char_printf(ch ,"{G%s praise you!{x\n", name);
	else if (mod >= 60)
		char_printf(ch ,"You gladden %s's heart.\n", name);
	else if (mod >= 25)
		char_printf(ch ,"%s is satisfied.\n", name);
	else if (mod >= 10)
		char_printf(ch ,"%s like your action.\n", name);
	else if (mod > -25)
		char_printf(ch ,"%s dislike your action.\n", name);
	else if (mod > -60)
		char_printf(ch ,"%s is discontent.\n", name);
	else if (mod > -100)
		char_printf(ch ,"%s is wrath.\n", name);
	else
		char_printf(ch ,"{R%s is fury!{x\n", name);

	ch->pcdata->religion->mod_faith = 0;
	return TRUE;
}

int reduce_faith(CHAR_DATA *ch, int minus, bool mess)
{
	/*
	 *	if (IS_NPC(ch) || !GET_CHAR_RELIGION(ch))
	 *		return;
	 */
	int f = ch->pcdata->religion->faith;

	if (minus == 0)
		return f;

	if (ch->pcdata->religion->data->allow_align
	&& !IS_SET(ch->pcdata->religion->data->allow_align, RALIGN(ch)))
		minus *= 2;
		
 	if (GET_TATTOO(ch) == NULL)
		minus *= 2;

	f = URANGE(-10000, f - minus, 10000);

	if (mess) {
		ch->pcdata->religion->mod_faith -= minus;
		send_like_message(ch);
	}

	return (ch->pcdata->religion->faith = f);
}

int incrase_faith(CHAR_DATA *ch, int plus, bool mess)
{
	/*
	 *	if (IS_NPC(ch) || !GET_CHAR_RELIGION(ch))
	 *		return;
	 */
	int f = ch->pcdata->religion->faith;

	if (ch->pcdata->religion->data->allow_align
	&& !IS_SET(ch->pcdata->religion->data->allow_align, RALIGN(ch)))
		return f;

	if (GET_TATTOO(ch) == NULL)
		plus /= 2;
	
	f = URANGE(-10000, f + plus, 10000);

	if (mess) {
		ch->pcdata->religion->mod_faith += plus;
		send_like_message(ch);
	}
	
	return (ch->pcdata->religion->faith = f);
}

bool make_spell(SPELL_FUN *fun, int sn, int level, CHAR_DATA *ch, void *vo,
		int target, int percent);
inline bool religion_cast(CHAR_DATA *ch, CHAR_DATA *victim, religion_fight_spell *spell, int level)
{
	return make_spell(SKILL(spell->sn)->spell->spell_fun, spell->sn,
			level, ch, victim, TARGET_CHAR, 100);
}

static CHAR_DATA *prepare_god(CHAR_DATA *ch, RELIGION_DATA *rel, CHAR_DATA *victim)
{
	if (rel->god && !IS_EXTRACTED(rel->god) && victim->in_room)
	{
		char_from_room(rel->god);
		char_to_room(rel->god, victim->in_room);
		return rel->god;
	} else
		return ch;
}

static void return_god(CHAR_DATA *god, RELIGION_DATA *rel)
{
	ROOM_INDEX_DATA *room;

	if (IS_NPC(god)
	&& IS_SET(god->pIndexData->act, ACT_GOD)
	&& (room = get_room_index(rel->godroom)))
	{
		char_from_room(god);
		char_to_room(god, room);
	}
}

int religion_fight_cast(CHAR_DATA *ch)
{
	/* return TRUE if need stop fighting [if victim dead] */
	CHAR_DATA *victim;
	struct religion_type *r;
	bool good = FALSE;
	bool bad = FALSE;
	religion_fight_spell *spell;
	int i, faith;
	CHAR_DATA *god, *tocast;
	
	if ((r = GET_CHAR_RELIGION(ch)) == NULL)
		return FALSE;

	if ((victim = ch->fighting) == NULL)
		return TRUE;
	
	/* See faith and cast honored or anti spell
	 *	[-10000 <= faith <= 10000]
	 */
	faith = get_char_faith(ch);
	if (faith >= -number_range(50,100))
		good = TRUE;
	if (faith <=  number_range(100, 300))
		bad = TRUE;

	for (i = 0; i < r->fight_spells.nused; i++) {
		spell = VARR_GET(&r->fight_spells, i);
		tocast = spell->to_char ? ch : victim;
		
		if (spell->is_for_honorable
		&& good
		&& (!r->allow_align || IS_SET(r->allow_align, RALIGN(ch)))
		&& (spell->percent * (faith + 100)) / 1000 >  number_range(1, 10000)
		&& !(!spell->to_char && ((100 * victim->hit) / UMAX(1,victim->max_hit)) < 15)
		)
		{		/* honored spells */
			god = prepare_god(ch, r, tocast);
			if (religion_cast(god, tocast, spell, ch->level + 5))
				reduce_faith(ch, spell->faith, FALSE);
			return_god(god, r);
			break;
		} else if (!spell->is_for_honorable
		&& bad
		&& (spell->percent * (-faith + 1000)) / 1000 > number_range(1, 5000))
		{		/* anti spells */
			god = prepare_god(ch, r, tocast);
			if (religion_cast(god, tocast, spell, ch->level + 10))
				incrase_faith(ch, spell->faith, FALSE);
			return_god(god, r);
			break;
		}
	}


	/* Check changes in fight */
	if (JUST_KILLED(ch)
		/* || !ch->fighting || ch->fighting != victim */
	|| (victim = ch->fighting) == NULL
	|| victim->in_room != ch->in_room)
		return TRUE;
	return FALSE;
}
