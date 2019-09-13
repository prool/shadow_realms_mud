/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: religion.h,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#ifndef _RELIGION_H_
#define _RELIGION_H_

/*
#define RELIGION_NONE		0
#define RELIGION_ATUM_RA	1
#define RELIGION_ZEUS		2
#define RELIGION_SIEBELE	3
#define RELIGION_SHAMASH	4
#define RELIGION_EHRUMEN	5
#define RELIGION_AHURAMAZDA	6
#define RELIGION_DEIMOS 	7
#define RELIGION_PHOBOS 	8
#define RELIGION_ODIN		9
#define RELIGION_TESHUB		10
#define RELIGION_ARES		11
#define RELIGION_GOKTENGRI	12
#define RELIGION_HERA		13
#define RELIGION_VENUS		14
#define RELIGION_SETH		15
#define RELIGION_ENKI		16
#define RELIGION_EROS		17
*/

/*
 *	for rel->flags
 */
#define RELIG_AUC_FREE		(A) 	// free auction
#define RELIG_GREED		(B)	// nodrop/... cost_above_much...
#define RELIG_EXTRA_HIT		(C)	// extra one_hit
#define RELIG_CHEAP_HEALER	(D)	// reduce cost for healer list
#define RELIG_CHECK_BENEDICTION	(E)	// advance spell in benediction group
#define RELIG_ADVANCE_DAMAGE	(F)	// add (3 .. 3+faith/500)% any damage
#define RELIG_DARK_MALADICTION	(G)	// advance spell in maladiction group for night

/*
 *	for rel->change_flags and for rel->change_faith[]->that 
 */
#define RELF_SACRIFICE_OBJ	(A)	// char sacrifice obj
#define RELF_RECALL		(B)	// ... recall
#define RELF_REBORN		(C)	// reincarnation before time...
#define RELF_DEATH		(D)	// death
#define RELF_PUT_OBJ_ALTAR	(E)	// put obj in own altar
#define RELF_DO_RESCUE		(F)	// rescue other
#define RELF_FI_STRONG_V	(G)	// fight with strong victim
#define RELF_FI_BARE_PC_V	(H)	// fight with bare pc
#define RELF_DO_FLEE		(I)	// flee
#define RELF_DO_GET_OBJ_PC_COR	(J)	// get object from pc's corpse
#define RELF_CURE_OTHER		(K)	// cure spells on other
#define RELF_BENEDICTION_OTHER	(L)	// benediction spells on other
#define RELF_AID_OTHER		(M)	// do_aid on other
#define RELF_KILL_PC		(N)	// kill pc
#define RELF_KILL_STRONG_NPC	(O)	// kill strong (victim->level >= ch->level) npc
#define RELF_DRINK_LIQ		(P)	// drink liquer
#define RELF_DRINK_NONLIQ	(Q)	// drink not liq (example water)
#define RELF_JOIN_GROUP		(R)	// join to some group
#define RELF_KILL_STRONG_EVIL	(S)	// kill strong evil pc/npc
#define RELF_KILL_GOOD		(T)	// kill good pc/npc
#define RELF_FIGHT_NONEVIL	(U)	// fight with not evil
#define RELF_KILL_STRONG_GOOD	(V)	// kill strong good pc/npc
#define RELF_KILL_EVIL		(W)	// kill evil pc/npc
#define RELF_CAST_MALADICTION	(X)	// success cast maladiction spell
#define RELF_GET_OBJ_ALTAR	(Y)	// get obj from any altar

extern varr religions;
extern flag_t	religion_flags[];
extern flag_t	religion_faith_flags[];

/* Religion structure   (RELIGION_DATA) */
typedef struct religion_add_skill
{
	int 	sn;
	int	level;
	int	add_percent;
	int	max_percent;
	int	hard;
} religion_add_skill;

typedef struct religion_fight_spell
{
	int	sn;
	int	percent;		// modificator of percent  [1..100]
	bool	is_for_honorable;
	bool	to_char;
	int	faith;			// cost in faith
} religion_fight_spell;

typedef struct religion_faith_change
{
	flag32_t	that;
	bool		is_like;	// incrase or decrase faith
	int		value;		// value of change faith (or other needed val)
} religion_faith_change;

struct religion_type
{
	const char *	name;
	mlstring *	desc;
	int		vnum_tattoo;
	int		vnum_templeman;
	CHAR_DATA *	god;			/* God (mob of god) */
	int		vnum_god;
	int		godroom;
	int		cost_gold;
	int		cost_qp;
	flag32_t	flags;			/* Bits for specifical parametrs */
	int		recall_change_exp;	/* change exp in % */
	int		change_exp_death;	/* change exp in % */
	altar_t		altar;
	varr		add_skills;	// type religion_add_skill
	varr		fight_spells;	// type religion_fight_spell
	flag32_t	change_flags;	// need for more faster search in change_faith
	varr		change_faith;	// type religion_faith_change
	int		ghost_timer_default;	// in seconds
	int		ghost_timer_plevel;	// timer = ghost_timer_default + ghost_timer_plevel * ch->level / 5
	flag32_t	allow_align;		// alignment restrictions

	/* Specifical parametrs */

	int		change_buy;
	int		change_sell;
	int		change_sacrifice;
};

altar_t *get_altar(CHAR_DATA *ch);
#define RELIGION(vn)    ((RELIGION_DATA *) varr_get (&religions, (vn)))
int		rlook_god(int vnum);
const char *	religion_name(int religion);
int		rel_lookup(const char *name);
int		get_char_faith(CHAR_DATA *ch);
int		change_faith(CHAR_DATA *ch, flag32_t that, int change);
int		reduce_faith(CHAR_DATA *ch, int minus, bool mess);
int		incrase_faith(CHAR_DATA *ch, int plus, bool mess);
int		religion_fight_cast(CHAR_DATA *ch);

/*
 *	rel_magic.c
 */
bool get_random_ill(AFFECT_DATA *aff, int power);

#define		GET_CHAR_RELIGION(ch)	((IS_PC(ch) && ch->pcdata->religion) ? ch->pcdata->religion->data : NULL)
#define		GET_TATTOO(ch)		(get_eq_char(ch, WEAR_TATTOO))
#define		WAIT_TIME_REINCARNATION(ch)	((ch->level < MIN_PK_LEVEL ?	\
			120 : ( GET_CHAR_RELIGION(ch) ?				\
				GET_CHAR_RELIGION(ch)->ghost_timer_default	\
				+ ch->level * GET_CHAR_RELIGION(ch)->ghost_timer_plevel	\
				: GHOST_DELAY_TIME + ch->level * GHOST_DELAY_PLEVEL	\
			)) - (current_time - ch->last_death_time))

#endif
