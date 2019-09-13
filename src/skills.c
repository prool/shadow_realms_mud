/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "update.h"

extern int gsn_anathema;
varr		skills = { sizeof(skill_t), 8 };

#define SSPOOL_STEP		50
#define INIT_MAX_SSPOOL		SSPOOL_STEP * 2

varr		spell_spool = { sizeof(spell_spool_t),  SSPOOL_STEP};
varr		spell_frees = { sizeof(unsigned long int), SSPOOL_STEP};
static int	first_spell;
static int	last_spell;
static int	curr_sfree;
static int	p_sfree;

/* command procedures needed */
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_say		);

/*
 * Calculate time after <add_pulse>
 */
TIME_INFO_DATA	*calc_time(int	add_pulse)
{
	static TIME_INFO_DATA	res;

	res.pulse	= (add_pulse + time_info.pulse) % PULSE_TICK;
	add_pulse	= (add_pulse + time_info.pulse) / PULSE_TICK;
	res.hour	= (add_pulse + time_info.hour) % TIME_HOUR_OF_DAY;
	add_pulse	= (add_pulse + time_info.hour) / TIME_HOUR_OF_DAY;
	res.day		= (add_pulse + time_info.day) % TIME_DAY_OF_MONTH;
	add_pulse	= (add_pulse + time_info.day) / TIME_DAY_OF_MONTH;
	res.month	= (add_pulse + time_info.month) % TIME_MONTH_OF_YEAR;
	res.year	= (add_pulse + time_info.month) / TIME_MONTH_OF_YEAR
				+ time_info.year;

	return &res;
}

void init_sspool(void)
{
	unsigned long int	i;
	unsigned long int	* fi;

	varr_touch(&spell_spool, INIT_MAX_SSPOOL - 1);	// 100 elements
	varr_touch(&spell_frees, INIT_MAX_SSPOOL - 1);	// 100 elements
	
	last_spell = first_spell = -1;
	curr_sfree = INIT_MAX_SSPOOL;
	for (i = 0; i < INIT_MAX_SSPOOL; i++)
	{
		fi = fspool_lookup(i);
		*fi = i;
	}
	p_sfree = 0;
	log_printf("Init spell spool %d(%d)", spell_spool.nalloc,
			spell_frees.nalloc);
}

unsigned long int sr_hour(TIME_INFO_DATA *timei);
/*
 *	Не совсем точная функция так, как не учитывает
 *	если abs(t->pulse) > PULSE_TICK
 */
int timecmp(TIME_INFO_DATA *t1, TIME_INFO_DATA *t2)
{
	unsigned long int h1 = sr_hour(t1);
	unsigned long int h2 = sr_hour(t2);

	if (h1 > h2)
		return 1;
	else if (h1 < h2)
		return -1;
	else if (t1->pulse > t2->pulse)
		return 1;
	else if (t1->pulse < t2->pulse)
		return -1;
	else
		return 0;
}

void spell_sadd(spell_spool_t *spell, bool dup_arg)
{
	spell_spool_t 		*add;
	spell_spool_t		*prevs;
	unsigned long int	*fi;
	int			i, counter = 0;
	TIME_INFO_DATA		*time;

	debug_printf(5, "spell_sadd[0]");
	if (spell->ch->nsp_spool != -1)
	{
		log_printf("Warning! '%s' already has sp_spool %d.",
			spell->ch->name, spell->ch->nsp_spool);
		//return;
	}

	if (curr_sfree <= 0)
	{
		varr_touch(&spell_spool, spell_spool.nalloc);
		varr_touch(&spell_frees, spell_frees.nalloc);
		for (i = spell_spool.nalloc - SSPOOL_STEP;
				i < spell_spool.nalloc; i++)
		{
			fi = fspool_lookup(i);
			*fi = i;
		}
		curr_sfree += SSPOOL_STEP;
		p_sfree = spell_spool.nalloc - SSPOOL_STEP;
	}
	
	debug_printf(5, "spell_sadd[1]");

	fi = fspool_lookup(p_sfree);
	p_sfree++;
	add = sspool_lookup(*fi);
	curr_sfree--;

	memcpy(add, spell, sizeof(*add));
	if (dup_arg)
		add->arg = str_dup(spell->arg);

	if (add->ch)
		add->ch->nsp_spool = *fi;

	if ((i = last_spell) == -1)
	{
		first_spell = last_spell = *fi;
		add->next = add->prev = -1;
		return;
	}
	
	debug_printf(5, "spell_sadd[2]");

	time = &(add->then_do);
	while ((prevs = sspool_lookup(i))->prev != -1
		&&  timecmp(&(prevs->then_do), time) > 0
		&& counter++ <= spell_spool.nalloc)
				/* prevs->then_do > time */
	{
		i = prevs->prev;
	}
	
	if (counter >= spell_spool.nalloc)
	{
		log_printf("BUG: spell_sadd: %d %d %d %d %d %d %d: %d/%d '%s'",
			spell_spool.nalloc,
			i, prevs->prev, prevs->next,
			*fi, p_sfree, curr_sfree,
			first_spell, last_spell,
			skill_name(add->sn));
		for (i = 0; i < p_sfree; i++)
			log_printf("i: %d %d", i, *(fspool_lookup(i)));

		/*
		 *	BUG: spell_sadd: 100 1 2 2 3 4 96: 0/2 'brew'
		 *	i: 0 2
		 *	i: 1 1
		 *	i: 2 2
		 *	i: 3 3
		 */
		shutdown_prepare("*sigh ssa");
		exit(22);
		
	}

	if (timecmp(&(prevs->then_do), time) <= 0)
			/* prevs->then_do <= time */
			/* */
	{
		add->next = prevs->next;
		add->prev = i;
		prevs->next = *fi;
		if (add->next != -1)
		{
			spell_spool_t *s_next = sspool_lookup(add->next);
			s_next->prev = *fi;
		}
	} else {
		add->next = i;
		add->prev = prevs->prev;
		prevs->prev = *fi;
		if (add->prev != -1)
		{
			spell_spool_t *s_prev = sspool_lookup(add->next);
			s_prev->next = *fi;
		}
	}
	
	debug_printf(5, "spell_sadd[3]");
	if (add->next == -1)
		last_spell = *fi;
	if (add->prev == -1)
		first_spell = *fi;
}

void spell_sdel(int i, bool del_arg)
{
	spell_spool_t		*sspell;
	unsigned long int	*fs;
	
	if ((sspell = sspool_lookup(i)) == NULL)
		return;

	p_sfree--;		// nizia: fspool_lookup(--p_sfree)  !!!
	fs = fspool_lookup(p_sfree);
	if (fs == NULL)
	{
		log_printf("*** BUG: spell_sdel: %d %d", i, p_sfree);
	}

	debug_printf(8, "sp_sdel1");
	*fs = i;
	curr_sfree++;

	if (sspell->next == -1)
		last_spell = sspell->prev;
	else
		sspool_lookup(sspell->next)->prev = sspell->prev;

	if (sspell->prev == -1)
		first_spell = sspell->next;
	else
		sspool_lookup(sspell->prev)->next = sspell->next;

	if (sspell->ch)
	{
		if (sspell->ch->nsp_spool == i)
			sspell->ch->nsp_spool = -1;
		/*if (sspell->ch->nsp_spool != i)
			log_printf("Warning! spell_sdel: '%s' has other nsp_spool %d.",
				sspell->ch->name, i);
		else
			sspell->ch->nsp_spool = -1;
		*/
	}

	debug_printf(8, "sp_sdel");

	if (del_arg)
		free_string(sspell->arg);
}

void remove_sspell_obj(OBJ_DATA *obj)
{
	spell_spool_t   *spell;
	int		i, next_i;

//	debug_printf(8, "r_sp_obj");	
	for(i = first_spell; i != -1; i = next_i)
	{
		if ((spell = sspool_lookup(i)) == NULL)
		{
			log_printf("BUG: rem_ss_o: %d %d %d %d",
				i, spell_spool.nalloc, curr_sfree,
				p_sfree);
			break;
		}
		next_i = spell->next;
		
		if (spell->vo == obj) {
			spell->vo = NULL;
			spell->target = -1;
		}
	}
}

void remove_sspell_ch(CHAR_DATA *ch)
{
	spell_spool_t   *sspell;
	int		i, next_i;

//	debug_printf(8, "r_sp_ch");	
	if (ch->nsp_spool != -1
	&& (sspell = sspool_lookup(ch->nsp_spool)))
	{
		sspell->ch = NULL;
		ch->nsp_spool = -1;
		//spell_sdel(ch->nsp_spool, TRUE);
	}

	for(i = first_spell; i != -1; i = next_i)
	{
		if ((sspell = sspool_lookup(i)) == NULL)
			break;
		next_i = sspell->next;

		if (sspell->vo == ch) {
			sspell->vo = NULL;
			sspell->target = -1;
		}
		
		if (sspell->ch == ch) {
			//spell_sdel(i, TRUE);
			sspell->ch = NULL;
			if (i == ch->nsp_spool)
				ch->nsp_spool = -1;
		}
	}
}

bool do_cast_done(spell_spool_t *sspell);		/* magic.c */
extern int debug_level;
void spell_update(void)
{
	spell_spool_t	*spell;
	int		i;

	debug_printf(4, "spell_update begin");
	while((i = first_spell) != -1	/* 'i' is very important ! */
	&& timecmp(&((spell = sspool_lookup(i))->then_do),
					&time_info) <= 0)
		// then_do <= time_info
	{
		if (debug_level > 0)
			log_printf("spool spell begin %s (%s)",
					skill_name(spell->sn),
					spell->arg);
		do_cast_done(spell);
		debug_printf(9, "sp_cast done");
		if (sspool_lookup(i) != spell)
		{
			log_printf("***BUG: change first spell after cast: '%s' by %s",
				skill_name(spell->sn), spell->ch ? spell->ch->name : "X");
			shutdown_prepare("*sigh ssa");
			exit(23);
		}
		spell_sdel(i, TRUE);
		debug_printf(7, "sp_cast done end");
	}
}

const char *time_string(TIME_INFO_DATA *time)
{
	static	char ts[20];

	snprintf(ts, sizeof(ts), "%04d.%02d.%02d.%02d.%03d",
			time->year, time->month, time->day,
			time->hour, time->pulse);
	return ts;
}

void show_sspool(CHAR_DATA *ch, int idump)
{
	BUFFER		*output;
	spell_spool_t	*sspell;
	int		i_ss;
	unsigned long int *fi;

	output = buf_new(-1);
	
	buf_printf(output, "SR time is %s (YYYY.MM.DD.HH.PP)\n",
			time_string(&time_info));
	buf_printf(output, "Current allocated slots %d. Free %d[%d]. Step %d slots for new alloc.\n"
			"First: %d   Last: %d\n",
			spell_spool.nalloc, curr_sfree, p_sfree, SSPOOL_STEP,
			first_spell, last_spell);

	if (idump)
	{
		buf_add(output, "Slot next prev SName               Caster    Arguments\n");
		buf_add(output, "--------------------DUMP---------ах ты цука!----------\n");
		idump = URANGE(0, idump, spell_spool.nalloc - 1);
		for (i_ss = 0; i_ss < idump; i_ss++)
		{
			sspell = sspool_lookup(i_ss);
			buf_printf(output, "%-4d %-4d %-4d %-20.20s %-10s %s\n",
					i_ss, sspell->next, sspell->prev,
					skill_name(sspell->sn),
					sspell->ch ? mlstr_mval(sspell->ch->short_descr): str_empty,
					sspell->arg ? sspell->arg : str_empty);
		}
		for (i_ss = 0; i_ss < idump; i_ss++)
		{
			fi = fspool_lookup(i_ss);
			
			buf_printf(output, "(%d, %d)", i_ss, *fi);
		}
		 buf_printf(output, "\n");
	}

	buf_add(output, "Slot SName               Caster    Arguments Next Prev\n");
	buf_add(output, "-------------------нихуя! я тебя поймаю!!!------------\n");

	i_ss = first_spell;
	while(i_ss != -1)
	{
		sspell = sspool_lookup(i_ss);
		buf_printf(output, "%-4d %-20.20s %-10s %s %d %d\n",
			i_ss, skill_name(sspell->sn),
			sspell->ch ? mlstr_mval(sspell->ch->short_descr): str_empty, sspell->arg,
			sspell->next, sspell->prev);
		i_ss = sspell->next;
	}

	if (first_spell == -1)
		buf_add(output, "Spell spool is empty now. ");
	buf_add(output, "-----\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

magic_school_t magic_schools[QUANTITY_MAGIC + 1] =
{
        { 0,			0,	0,
        	{	3000,	3000,	3000	}},
        { MAGIC_MENTAL,		LIFE,	APPLY_SAVING_MENTAL,
        	{	3000,	3000,	3000	}},
        { MAGIC_EARTH,		AIR,	APPLY_SAVING_EARTH,
        	{	3000,	3000,	3000	}},
        { MAGIC_FIRE,		WATER,	APPLY_SAVING_FIRE,
        	{	3000,	3000,	3000	}},
        { MAGIC_AIR,		EARTH,	APPLY_SAVING_AIR,
        	{	3000,	3000,	3000	}},
        { MAGIC_WATER,		FIRE,	APPLY_SAVING_WATER,
        	{	3000,	3000,	3000	}},
        { MAGIC_LIFE,		MENTAL,	APPLY_SAVING_LIFE,
        	{	3000,	3000,	3000	}}
};
#define MRANK_1 0	// +1
#define MRANK_2	30
#define MRANK_3	85

inline int can_get_rank(int mschool)
{
	switch(mschool)
	{
		case MRANK_1:	return 1;
		case MRANK_2:	return 2;
		case MRANK_3:	return 3;
	}
	return 0;
}

int get_char_msc(CHAR_DATA *ch, int n_msc, int improve)
{
	int anti, msc;
	
	if (IS_NPC(ch) || n_msc == 0)
		return 0;
	msc = ch->pcdata->mschool[n_msc - 1];
	if (improve
	&& can_get_rank(msc) == 0
	&& msc < 100
	&& number_range(1, 50 * msc * improve) < get_curr_stat(ch, STAT_INT))
	{
		if ((ch->pcdata->mschool[n_msc - 1]++) >= 100)
			char_printf(ch, "You have become master of force '%s'.\n",
				flag_string(irv_flags, magic_schools[n_msc].bit_force));
		else
			char_printf(ch, "You have advanced in art of '%s'.\n",
				flag_string(irv_flags, magic_schools[n_msc].bit_force));
	}

	return URANGE(-80, msc - ((anti = magic_schools[n_msc].anti_magic) ?
		ch->pcdata->mschool[anti - 1] / 2 : 0),
					100);
}

/*
 *inline magic_school_t * magic_lookup(flag32_t bmsc)
 *{
 *	int i;
 *	for (i = 1; i < QUANTITY_MAGIC + 1; i++)
 *		if (IS_SET(bmsc, magic_schools[i].bit_force))
 *			return &magic_schools[i];
 *	return 0;
 *}
 */
   
int smagic_apply_lookup(int location)
{
	int i;
	for (i = 1; i <= QUANTITY_MAGIC; i++)
		if (magic_schools[i].apply_saves == location)
			return i;
	return 0;
}

int get_magic_rank(CHAR_DATA * ch, int nmsc)
{
	int m;

	if (!IS_PC(ch) || nmsc == 0 || (m = ch->pcdata->mschool[nmsc - 1]) <= 0)
		return 0;
	else if (m <= MRANK_2)
		return 1;
	else if (m <= MRANK_3)
		return 2;
	else
		return 3;
}

int get_cost_next_rank(CHAR_DATA *ch, int nmsc)
{
	int anti, rank, i, n, cost;

	if (!nmsc
	|| (rank = get_magic_rank(ch, nmsc)) >= 3   
	|| ((anti = magic_schools[nmsc].anti_magic)
		&& get_magic_rank(ch, anti)
	   )
	)
		return -1;
	for (n = 0, i = 1; i <= QUANTITY_MAGIC; i++)
		if (ch->pcdata->mschool[i - 1] > 0)
			n++;
	switch(rank + 1)
	{
		case 1:		cost = n * n * n * n + 2;	break;
		case 2:		cost = n * n * n * 8 + 12;	break;
		case 3:		cost = n * n * n * 20 + 30;	break;
		default:	cost = 300;
	}
	return UMAX(1, cost - (cost * (get_curr_stat(ch, STAT_CHA) - 18)) / 100);
}

int lookup_msc_by_str(const char *arg, CHAR_DATA *ch_show)
{
	flag32_t msc;
	int i;

	msc = flag_value(irv_flags, arg);
	if (arg[0] != '\0')
		for (i = 1; i <= QUANTITY_MAGIC; i++)
		{
			if (magic_schools[i].bit_force == msc)
				break;
		}
	else
		i = 0;
	if (ch_show
	&& (i == 0 || msc == 0 || i > QUANTITY_MAGIC)) {
		char_puts("Unknown magic school.\n", ch_show);
		return 0;
	}
	return i;
}

bool check_password(CHAR_DATA *ch, const char *pswd, bool show_txt_wait);	// search in act_info.c

void show_list_mschool(CHAR_DATA *ch)
{
	int i;
	int col = 0;

	for (i = 1; i <= QUANTITY_MAGIC; i++) {
		char_printf(ch, "%-19.18s",
			flag_string(irv_flags, magic_schools[i].bit_force));
		if (++col % 4 == 0)
			char_puts("\n", ch);
	}
	if (col % 4 != 0)
		char_puts("\n", ch);
}

void do_mschool(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;
	argument = one_argument(argument, arg, sizeof(arg));
	WAIT_STATE(ch, PULSE_VIOLENCE);
	
	if (arg[0] == '\0')
	{
		char_puts(
			"Syntax: mschool info [<mschool>]\n"
			"        mschool learn <mschool>\n"
			"        mschool renounce <mschool> <your password>\n"
			"        mschool list <mschool>\n", ch);
	} else if (!str_prefix(arg, "info") && argument[0] == '\0') {
		int rank; 
		int i;

		char_puts("You advances in magic schools next:\n", ch);
		for (i = 1; i <= QUANTITY_MAGIC; i++) {
			if ((rank = get_magic_rank(ch, i)))
				char_printf(ch, " %-12s %3d  (%3d)%% - %d rank\n",
					flag_string(irv_flags, magic_schools[i].bit_force),
					ch->pcdata->mschool[i-1],
					get_char_msc(ch, i, 0),
					rank);
			else
				char_printf(ch, " %-12s none (%3d)%%\n",
					flag_string(irv_flags, magic_schools[i].bit_force),
					get_char_msc(ch, i, 0));
		}
	} else if (!str_prefix(arg, "info")) {
		int cost;
		int msc_n;

		one_argument(argument, arg, sizeof(arg));
		if ((msc_n = lookup_msc_by_str(arg, ch)) <= 0)
			return;
		char_printf(ch, "* {M%s{x *    {m%d{x(%d)%%\n",
			flag_string(irv_flags, magic_schools[msc_n].bit_force),
			ch->pcdata->mschool[msc_n-1],
			get_char_msc(ch, msc_n, 0));
		if ((cost = get_cost_next_rank(ch, msc_n)) == -1) {
			char_puts("You cann't learn next rank of this magic school.\n", ch);
			return;
		} else {
			int vnum = magic_schools[msc_n].vnum_teacher[get_magic_rank(ch, msc_n)];
			MOB_INDEX_DATA	* teacher = get_mob_index(vnum);
			AREA_DATA	* pArea = area_vnum_lookup(vnum);
			
			if (!teacher || !pArea)
				return;
			char_printf(ch, "You may learned next rank this school for %d gold"
				" by %s.\nThat location is in general area of {G%s{x.\n",
				get_cost_next_rank(ch, msc_n), mlstr_cval(teacher->short_descr, ch),
				pArea->name);
			if (can_get_rank(ch->pcdata->mschool[msc_n-1]) == 0)
				char_puts("But now you cann't get new rank becouse you are not expirience enough.\n", ch);
		}
		
	} else if (!str_prefix(arg, "learn")) {	
		CHAR_DATA *mob;
		int msc_n;
		int cost;
		int rank;

		one_argument(argument, arg, sizeof(arg));
		if ((msc_n = lookup_msc_by_str(arg, ch)) <= 0)
			return;
		if ((cost = get_cost_next_rank(ch, msc_n)) == -1) {
			char_puts("You cann't learn next rank of this magic school.\n", ch);
			return;
		}
		if ((rank = can_get_rank(ch->pcdata->mschool[msc_n-1])) == 0) {
  			char_puts("You cann't get new rank becouse you are not expirience enough in this force.\n", ch);
			return;
		}
		for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
			if (IS_NPC(mob) && mob->pIndexData->vnum == magic_schools[msc_n].vnum_teacher[rank - 1])
				break;
		if (!mob)
		{
			char_printf(ch, "You couldn't find anyone who is able to help you advance to %d rank of '%s'.\n",
				rank, flag_string(irv_flags, magic_schools[msc_n].bit_force));
			return;
		}
		if (!can_see(mob, ch)) {
			do_say(mob, "I can't see you, my disciple.");
			return;
		}
		if (cost * 100 > ch->silver + ch->gold * 100) {
			doprintf(interpret, mob, "say Sorry, but teachers want eat too. Come back with %d gold and i help you.",
				cost);
			return;
		}
		deduct_cost(ch, cost * 100);
		ch->pcdata->mschool[msc_n - 1]++;
		doprintf(interpret, mob, "say Now, you can farther perfected in '{M%s{G'!",
			flag_string(irv_flags, magic_schools[msc_n].bit_force));
	} else if (!str_prefix(arg, "renounce")) { 
		int msc_n;
		int lose_mana;
		
		argument = one_argument(argument, arg, sizeof(arg));
		
		if ((msc_n = lookup_msc_by_str(arg, ch)) <= 0)
			return;
		if (ch->pcdata->mschool[msc_n - 1] <= 0)
		{
			char_puts("You didn't study this force.\n", ch);
			ch->pcdata->mschool[msc_n - 1] = 0;
			return;
		}
		first_arg(argument, arg, sizeof(arg), FALSE);
		if (!check_password(ch, arg, TRUE))
			return;
		ch->pcdata->perm_mana -=
			(lose_mana = number_range(1, UMAX(2, ch->pcdata->mschool[msc_n - 1] / 2)));
		ch->max_mana -= lose_mana;
		ch->pcdata->mschool[msc_n - 1] = 0;
		char_puts("With lose of force, you {Rlose{x piece of energy.\n", ch);
	} else if (!str_prefix(arg, "list")) {
		int msc_n;
		BUFFER *output;
		int i;
		int sn;
		bool found = FALSE;
		
		argument = one_argument(argument, arg, sizeof(arg));
		
		if ((msc_n = lookup_msc_by_str(arg, ch)) <= 0)
			return;
		output = buf_new(-1);
		if (IS_IMMORTAL(ch))
		{
			for (sn = 0; sn < skills.nused; sn++) {
				skill_t *sk = VARR_GET(&skills, sn);
				if (sk->spell
				&& IS_SET(sk->spell->mschool,
					magic_schools[msc_n].bit_force))
				{
					buf_printf(output, "%c %-25s | %s\n",
						pcskill_lookup(ch, sn) ? '*' : ' ',
						sk->name,
						flag_string(irv_flags, sk->spell->mschool));
					found = TRUE;
				}
			}
		} else {
			for (i = 0; i < ch->pcdata->learned.nused; i++) {
				pcskill_t *ps = varr_get(&ch->pcdata->learned, i);
				skill_t *sk;
				
				if (ps == NULL
				||  ps->percent == 0
				||  (sk = skill_lookup(ps->sn)) == NULL
				||  sk->spell == NULL
				||  !IS_SET(sk->spell->mschool, magic_schools[msc_n].bit_force))
					continue;
				buf_printf(output, "  %-25s | %s\n",
					sk->name,
					flag_string(irv_flags, sk->spell->mschool));
				found = TRUE;
			}
		}
		
		if (found)
			buf_add(output, "\n");
		else
			buf_add(output, "You known no such spells.\n");
		page_to_char(buf_string(output), ch);
		buf_free(output);
	} else
		do_mschool(ch, str_empty);
}


int	ch_skill_nok	(CHAR_DATA *ch , int sn);

/* used to converter of prac and train */
void do_gain(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *tr;

	if (IS_NPC(ch))
		return;

	/* find a trainer */
	for (tr = ch->in_room->people; tr; tr = tr->next_in_room)
		if (IS_NPC(tr)
		&&  IS_SET(tr->pIndexData->act,
			   ACT_PRACTICE | ACT_TRAIN))
			break;

	if (tr == NULL || !can_see(ch, tr)) {
		char_puts("You can't do that here.\n",ch);
		return;
	}
	
	do_say(tr, "Excuse me, but this job {Rrestricted{G by Immortal's !!!");
	return;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		do_say(tr, "You may convert 10 practices into 1 train.");
		do_say(tr, "You may revert 1 train into 10 practices.");
		do_say(tr, "Simply type 'gain convert' or 'gain revert'.");
		return;
	}

	if (!str_prefix(arg, "revert")) {
		if (ch->pcdata->train < 1) {
			do_tell_raw(tr, ch, "You are not yet ready.");
			return;
		}

		act("$N helps you apply your training to practice",
		    ch, NULL, tr, TO_CHAR);
		ch->pcdata->practice += 10;
		ch->pcdata->train -=1 ;
		return;
	}

	if (!str_prefix(arg, "convert")) {
		if (ch->pcdata->practice < 10) {
			do_tell_raw(tr, ch, "You are not yet ready.");
			return;
		}

		act("$N helps you apply your practice to training",
		    ch, NULL, tr, TO_CHAR);
		ch->pcdata->practice -= 10;
		ch->pcdata->train +=1 ;
		return;
	}

	do_tell_raw(tr, ch, "I do not understand...");
}


/* RT spells and skills show the players spells (or skills) */

void do_spells(CHAR_DATA *ch, const char *argument)
{
	char spell_list[MAX_LEVEL][MAX_STRING_LENGTH];
	int lev;
	int i, minlev, maxlev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	const char *pattern;

	if (IS_NPC(ch))
		return;

	WAIT_STATE(ch, PULSE_VIOLENCE);	
	/* initialize data */
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++) {
		spell_list[lev][0] = '\0';
	}
	
	minlev = 1; maxlev = MAX_LEVEL; pattern = argument;
	argument = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] != '\0') {
		
		if (arg1[0] == '?')
		{
			char_puts("Syntax:\n",ch);
			char_puts("  spell\n",ch);
			char_puts("  spell <MinLevel> <MaxLevel>\n",ch);
			char_puts("  spell <pattern>\n",ch);
			return;
		}
		if (is_number(arg1))
		{
			argument = one_argument(argument, arg2, sizeof(arg2));
			if (arg2[0] == '\0' || !is_number(arg1) || !is_number(arg2)) {
				do_spells(ch, "?");
				return;
			}
			minlev = atoi(arg1);
			maxlev = atoi(arg2);
			if (minlev < 0 || minlev > 100 || maxlev < 0 || maxlev > 100)
			{
				char_puts("Value range is 0 to 100.\n", ch);
				return;
			}
			if (minlev > maxlev) {
				char_puts("MaxLevel must be more MinLevel!",ch);
				return;
			}
			pattern = NULL;
		} else {
		}
	} else {
		pattern = NULL;
	}
	
	snprintf(spell_list[0], sizeof(spell_list[0]),
		"----------------------------------------------------------------------------");
	snprintf(buf, sizeof(buf),
	"\n|           |        Spell name         | Mana |   Learn    |  Max  | Hard |");
	strnzcat (spell_list[0], sizeof(spell_list[0]), buf);
	snprintf(buf, sizeof(buf),
		"\n----------------------------------------------------------------------------");
	strnzcat (spell_list[0], sizeof(spell_list[0]), buf);
	snprintf(spell_list[LEVEL_IMMORTAL+1], sizeof(spell_list[LEVEL_IMMORTAL+1]),
		"\n============================================================================");

	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		pcskill_t *ps = varr_get(&ch->pcdata->learned, i);
		skill_t *sk;

		if (ps == NULL
		||  ps->percent == 0
		||  (sk = skill_lookup(ps->sn)) == NULL
		||  sk->spell == NULL
		|| (pattern && str_infix(pattern, sk->name)))
			continue;

		found = TRUE;
		lev = ps->level;

		if (lev > maxlev || lev < minlev)
			continue;
			
			
		if (ch->level < lev)
			snprintf(buf, sizeof(buf), "| %-25s |  n/a | %4d(%3d)%% | %4d%% | %4d |",
				 sk->name, ps->percent, get_skill(ch, ps->sn), ps->maxpercent, ps->hard);
		else
			snprintf(buf, sizeof(buf), "| %-25s | %4d | %4d(%3d)%% | %4d%% | %4d |",
				 sk->name, mana_cost(ch, ps->sn), ps->percent, get_skill(ch, ps->sn), ps->maxpercent, ps->hard);


		if (spell_list[lev][0] == '\0')
			snprintf(spell_list[lev], sizeof(spell_list[lev]),
				"\n| Level %2d: %s", lev, buf);
		else {
			strnzcat(spell_list[lev], sizeof(spell_list[lev]), "\n|           ");
			strnzcat(spell_list[lev], sizeof(spell_list[lev]), buf);
		}
	}

	/* return results */
	if (!found) {
		char_puts("You know no spells.\n",ch);
		return;
	}
	
	output = buf_new(-1);
	for (lev = 0; lev <= LEVEL_IMMORTAL+1; lev++)
		if (spell_list[lev][0] != '\0')
			buf_add(output, spell_list[lev]);
	buf_add(output, "\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_skills(CHAR_DATA *ch, const char *argument)
{
	char skill_list[MAX_LEVEL][MAX_STRING_LENGTH];
	int lev;
	int i,minlev,maxlev;
	bool found = FALSE;
	char buf[MAX_STRING_LENGTH];
	BUFFER *output;
	const char *pattern;
	char arg1 [MAX_INPUT_LENGTH], arg2 [MAX_INPUT_LENGTH];

	if (IS_NPC(ch))
		return;

	WAIT_STATE(ch, PULSE_VIOLENCE);
	/* initialize data */
	for (lev = 0; lev <= LEVEL_IMMORTAL; lev++) {
		skill_list[lev][0] = '\0';
	}

	minlev = 1; maxlev = MAX_LEVEL; pattern = argument;
	argument = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] != '\0') {
		if (arg1[0] == '?')
		{
			char_puts("Syntax:\n",ch);
			char_puts("  skill\n",ch);
			char_puts("  skill <MinLevel> <MaxLevel>\n",ch);
			char_puts("  skill <pattern>\n",ch);
			return;
		}
		if (is_number(arg1))
		{
			argument = one_argument(argument, arg2, sizeof(arg2));
			if (arg2[0] == '\0' || !is_number(arg1) || !is_number(arg2))
			{
				do_skills(ch, "?");
				return;
			}
			minlev = atoi(arg1);
			maxlev = atoi(arg2);
			if (minlev < 0 || minlev > 100 || maxlev < 0 || maxlev > 100) {
				char_puts("Value range is 0 to 100.\n", ch);
				return;
			}
			if (minlev > maxlev) {
				char_puts("MaxLevel must be more MinLevel!",ch);
				return;
			}
			pattern = NULL;
		} else {
		}
	} else {
		pattern = NULL;
	}	
	snprintf(skill_list[0], sizeof(skill_list[0]),
		"---------------------------------------------------------------------");
	snprintf(buf, sizeof(buf),
	"\n|           |        Skill name         |   Learn    |  Max  | Hard |");
	strnzcat (skill_list[0], sizeof(skill_list[0]), buf);
	snprintf(buf, sizeof(buf),
		"\n---------------------------------------------------------------------");
	strnzcat (skill_list[0], sizeof(skill_list[0]), buf);
	snprintf(skill_list[LEVEL_IMMORTAL+1], sizeof(skill_list[LEVEL_IMMORTAL+1]),
		"\n=====================================================================");
	
	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		pcskill_t *ps = varr_get(&ch->pcdata->learned, i);
		skill_t *sk;

		if (ps == NULL
		||  ps->percent == 0
		||  (sk = skill_lookup(ps->sn)) == NULL
		||  sk->spell
		|| IS_SET(sk->group, GROUP_SLANG)
		|| (pattern && str_infix(pattern, sk->name)))
			continue;

		found = TRUE;
		lev = ps->level;

//		if (lev > (IS_IMMORTAL(ch) ? LEVEL_IMMORTAL : LEVEL_HERO))
//			continue;
		
		if (lev > maxlev || lev < minlev)
			continue;		

		if (ch->level < lev)
			snprintf(buf, sizeof(buf), "| %-25s |    n/a     | %4d%% | %4d |",
				 sk->name, ps->maxpercent, ps->hard);
		else
			snprintf(buf, sizeof(buf), "| %-25s | %4d(%3d)%% | %4d%% | %4d |",
				 sk->name, ps->percent, get_skill(ch, ps->sn), ps->maxpercent, ps->hard);

		if (skill_list[lev][0] == '\0')
			snprintf(skill_list[lev], sizeof(skill_list[lev]),
				"\n| Level %2d  %s", lev, buf);
		else {
			strnzcat(skill_list[lev], sizeof(skill_list[lev]), "\n|           ");
			strnzcat(skill_list[lev], sizeof(skill_list[lev]), buf);
		}
	}
	
	/* return results */
	
	if (!found) {
		char_puts("You know no skills.\n",ch);
		return;
	}
	
	output = buf_new(-1);
	for (lev = 0; lev <= LEVEL_IMMORTAL+1; lev++)
		if (skill_list[lev][0] != '\0')
			buf_add(output, skill_list[lev]);
	buf_add(output, "\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

int base_exp(CHAR_DATA *ch)
{
	int expl;
	class_t *cl;
	race_t *r;
	rclass_t *rcl;
	int t;

	if (IS_NPC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->pcdata->race)) == NULL
	||  !r->pcdata
	||  (rcl = rclass_lookup(r, cl->name)) == NULL)
		return 1500;

	expl = 1000 + r->pcdata->points + cl->points;
	if (ch->pcdata->remort)
	{
		t = ch->pcdata->remort->remorts;
		expl += ((t > 3) ?
			(300 + (t - 3) * UMAX(50, 100 - (t - 3) * 10))
			: (t * 100));
	}

	return expl * rcl->mult/100;
}

int exp_for_level(CHAR_DATA *ch, int level)
{
	int i = base_exp(ch) * level;
	return i + i * (level-1) / 20;
}

int exp_to_level(CHAR_DATA *ch)
{ 
	return IS_NPC(ch) ? 0 : exp_for_level(ch, ch->level+1) - ch->pcdata->exp;
}

/* checks for skill improvement */
void check_improve(CHAR_DATA *ch, int sn, bool success, int multiplier)
{
	pcskill_t *ps;
	int chance;
	int hard, mp;

	if (IS_NPC(ch)
	||  (ps = pcskill_lookup(ch, sn)) == NULL
	||  ps->percent < 2 || ps->percent >= ps->maxpercent
	||  ps->level > ch->level)
		return;

	hard = ps->hard;
	mp = (100 * ps->percent) / ps->maxpercent;
	
	/* check to see if the character has a chance to learn */
	chance = 10 * int_app[get_curr_stat(ch,STAT_INT)].learn;
	chance /= (multiplier * 4);
	chance += ch->level;

	if (mp < 31)
		chance *= 8;
	else if (mp < 61)
		chance *= 4;
	else if (mp < 76)
		chance *= 2;
	else if (mp > 90)
		chance = chance * 2 / 3;

	chance = URANGE (10, (100 - hard) * chance / 50, 950);
	if (number_range(1, 1000) > chance)
		return;

/* now that the character has a CHANCE to learn, see if they really have */	

	if (success) {
		chance = URANGE(5, 100 - ps->percent, 95);
		if (number_percent() < chance) {
			ps->percent++;
			gain_exp(ch, hard / 10 + ps->level, TRUE, FALSE);
			if (ps->percent == ps->maxpercent) char_printf(ch,
				"{gYou have become master of {W%s{g!{x\n",
				skill_name(sn));
			else char_printf(ch,
				"{gYou have become better at {W%s{g!{x\n",
				skill_name(sn));
		}
	}
	else {
		chance = URANGE(10, ps->percent / 2, 40);
		if (number_percent() < chance) {
			if ((ps->percent += number_range(1, 3)) > ps->maxpercent)
				ps->percent = ps->maxpercent;
			gain_exp(ch, 2 * (hard / 20 + ps->level), TRUE, FALSE);
			if (ps->percent == ps->maxpercent) char_printf(ch,
				"{gYou learn from your mistakes and you manage to master of {W%s{g!{x\n",
				skill_name(sn));
			else char_printf(ch,
				"{gYou learn from your mistakes and your {W%s{g skill improves!{x\n",
				skill_name(sn));

		}
	}
}

/*
 * simply adds sn to ch's known skills (if skill is not already known).
 */
void set_skill_raw(CHAR_DATA *ch, int sn, int percent, bool replace, int level, int mp, int hard)
{
	pcskill_t *ps;

	if (sn <= 0)
		return;

	if ((ps = pcskill_lookup(ch, sn))) {
		if (replace) {
			ps->percent = percent;
			ps->level = level;
			ps->maxpercent = mp;
			ps->hard = hard;
			return;
		}

		ps->maxpercent = (ps->maxpercent < mp) ? mp : ps->maxpercent;
		ps->level = (ps->level > level) ? level : ps->level;
		ps->hard = (ps->hard > hard) ? hard : ps->hard;
		ps->percent = (ps->percent < percent) ? percent : ps->percent;
		return;
	}
	ps = varr_enew(&ch->pcdata->learned);
	ps->sn = sn;
	ps->percent = percent;
	ps->level = level;
	ps->maxpercent = mp;
	ps->hard = hard;
	varr_qsort(&ch->pcdata->learned, cmpint);
}


void remove_skills(CHAR_DATA *ch)
{
	int i;

	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		pcskill_t *ps = VARR_GET(&ch->pcdata->learned, i);
			ps->percent = 0;
			ps->level = MAX_LEVEL+1;
			ps->hard = 100;
			ps->maxpercent = 0;
	}
}


/* use for adding/updating all skills available for that ch  */
void update_skills(CHAR_DATA *ch)
{
	int i;
	class_t *cl;
	race_t *r;
	//clan_t *clan;
	const char *p;
	pcskill_t *ps;

/* NPCs do not have skills */
	if (!IS_PC(ch)
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->pcdata->race)) == NULL
	||  !r->pcdata)
		return;

	//clan = clan_lookup(ch->clan);

/* Remove skill's that char don't know.*/
	if (!IS_IMMORTAL(ch))
		for (i = 0; i < ch->pcdata->learned.nused; i++) {
			ps = VARR_GET(&ch->pcdata->learned, i);
			if (pc_own_skill_lookup(ch, ps->sn) != NULL)
				continue;
			if (cskill_lookup(class_lookup(ch->class), ps->sn) != NULL)
				continue;
			if (rskill_lookup(race_lookup(ch->pcdata->race), ps->sn) != NULL)
				continue;
			if (sellskill_lookup(ch, ps->sn) != NULL)
				continue;
			//if (clan != NULL &&	clskill_lookup(clan, ps->sn) != NULL)
			//	continue;
			if (ch->pcdata->religion && ch->pcdata->religion->data
			&& varr_bsearch(&ch->pcdata->religion->data->add_skills, &(ps->sn), cmpint))
				continue;

			if (IS_SET(SKILL(ps->sn)->group, GROUP_SLANG))
				continue;

			set_skill_raw(ch, ps->sn, 0, TRUE, MAX_LEVEL, 0, 100);

		}



/* add class skills */
	for (i = 0; i < cl->skills.nused; i++) {
		cskill_t *cs = VARR_GET(&cl->skills, i);
		set_skill_raw(ch, cs->sn, 1, FALSE, cs->level, cs->maxpercent, cs->hard);
	}

/* add race skills */
	for (i = 0; i < r->pcdata->skills.nused; i++) {
		rskill_t *rs = VARR_GET(&r->pcdata->skills, i);
		set_skill_raw(ch, rs->sn, 1, FALSE, rs->level, 100, 40);
	}

/* add own skills */
	if (ch->pcdata->remort)
		for (i = 0; i < ch->pcdata->remort->skills.nused; i++) {
			ps = VARR_GET(&ch->pcdata->remort->skills, i);
			set_skill_raw(ch, ps->sn, ps->percent, FALSE, ps->level, ps->maxpercent, ps->hard);
		}
/* set skill 100% if it's bonus skill */
	if ((p = r->pcdata->bonus_skills))
		for (;;) {
			int sn;
			char name[MAX_STRING_LENGTH];

			p = one_argument(p, name, sizeof(name));
			if (name[0] == '\0')
				break;

			sn = sn_lookup(name);
			if (sn < 0)
				continue;
			if ((ps = pcskill_lookup(ch, sn)) == NULL)
				continue;

			set_skill_raw(ch, sn, 100, FALSE, 1, 100, 40);
		}

/* add clan skills */
	/*
	if (clan != NULL) {
		for (i = 0; i < clan->skills.nused; i++) {
			clskill_t *cs = VARR_GET(&clan->skills, i);
			set_skill_raw(ch, cs->sn, cs->percent, FALSE, cs->level, 100, 50);
		}
	}
	*/
/* add religion skills */
	if (ch->pcdata->religion && ch->pcdata->religion->data)
	{
		for (i = 0; i < ch->pcdata->religion->data->add_skills.nused; i++)
		{
			religion_add_skill *rel_skill =
				VARR_GET(&ch->pcdata->religion->data->add_skills, i);
			set_skill_raw(ch, rel_skill->sn, 1, FALSE,
				rel_skill->level, rel_skill->max_percent,
				rel_skill->hard);
		}
	}

/* add language */
	set_skill_raw(ch, r->slang, 90, FALSE, 1, 100, 50);

/* remove not matched skills */
	if (!IS_IMMORTAL(ch))
		for (i = 0; i < ch->pcdata->learned.nused; i++) {
			pcskill_t *ps = VARR_GET(&ch->pcdata->learned, i);
			if (ps->level > LEVEL_LEGEND){
				ps->percent = 0;
				ps->level = MAX_LEVEL;
				ps->hard = 100;
				ps->maxpercent = 0;
			}
		}
}

void set_skill(CHAR_DATA *ch, int sn, int percent, int level, int mp, int hard)
{
	set_skill_raw(ch, sn, percent, TRUE, level, mp, hard);
}

DO_FUN(do_glist)
{
	char arg[MAX_INPUT_LENGTH];
	int col = 0;
	flag64_t group = GROUP_NONE;
	int sn;

	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		char_puts("Syntax: glist group\n"
			  "Use 'glist ?' to get the list of groups.\n", ch);
		return;
	}

	if (!str_cmp(arg, "?")) {
		show_flags(ch, skill_groups);
		return;
	}

	if (str_prefix(arg, "none")
	&&  (group = flag_value(skill_groups, arg)) == 0) {
		char_puts("That is not a valid group.\n", ch);
		do_glist(ch, str_empty);
		return;
	}

	char_printf(ch, "Now listing group '%s':\n",
		    flag_string(skill_groups, group));

	for (sn = 0; sn < skills.nused; sn++) {
		skill_t *sk = VARR_GET(&skills, sn);
		if (group == sk->group) {
			char_printf(ch, "%c%-18s",
				    pcskill_lookup(ch, sn) ? '*' : ' ',
				    sk->name);
			if (col)
				char_puts("\n", ch);
			col = 1 - col;
		}
	}

	if (col)
		char_puts("\n", ch);
}

void do_slook(CHAR_DATA *ch, const char *argument)
{
	int sn = -1;
	char arg[MAX_INPUT_LENGTH];
	skill_t *sk;
	const char *str_sk;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Syntax : slook <skill | spell>\n",ch);
		return;
	}

/* search in known skills first */
	if (!IS_NPC(ch)) {
		pcskill_t *ps;
		ps = (pcskill_t*) skill_vlookup(&ch->pcdata->learned, arg);
		if (ps)
			sn = ps->sn;
	}

/* search in all skills */
	if (sn < 0)
		sn = sn_lookup(arg);

	if (sn < 0) { 
		char_puts("That is not a spell or skill.\n",ch);
		return; 
	}
	
	sk = SKILL(sn); 
	str_sk = sk->spell ? "Spell" : "Skill";

	char_printf(ch, "%s '%s' in group '%s'.\n",
		str_sk,
		sk->name,
		flag_string(skill_groups, sk->group));
	if (sk->spell)
		char_printf(ch, "This is spell is magic of '%s'.\n",
			flag_string(irv_flags, sk->spell->mschool));
	if (!IS_SET(sk->flags, SKILL_NOREMORT))
		char_printf(ch, "%s cost %d remort points for remort.\n",
			str_sk,
			sk->remort_cost);
	if (sk->flags)
		char_printf(ch, "%s has next ability: %s\n",
			str_sk,
			flag_string(skill_flags, sk->flags));
}

int slang_lookup(const char *name)
{
	int sn;
	
	if (IS_NULLSTR(name))
		return -1;

	for (sn = 0; sn < skills.nused; sn++)
		if (!str_prefix(name, SKILL(sn)->name)
		&& IS_SET(SKILL(sn)->group, GROUP_SLANG))
			return sn;
	return -1;
}

sellskill_t *   sellskill_lookup(CHAR_DATA *ch, int sn)
{
	static sellskill_t *sell;
	skill_t *sk = skill_lookup(sn);
	int iSell;
	class_t *cl;
	race_t *r;
	pcrace_t *pcr;
	
	if (!sk || sk->sell.nused == 0 
	   || (cl = class_lookup(ch->class)) == NULL
	   || (r = race_lookup(ch->race)) == NULL
	   || (pcr = r->pcdata) == NULL)
		return NULL;

	for (iSell = 0; iSell < sk->sell.nused; iSell++) {
		sell = (sellskill_t *) varr_get(&sk->sell, iSell);
		if ((sell->flag == SSF_ALL)
		  ||(sell->flag == SSF_RACE && (is_name(r->name, sell->name) || is_name(pcr->who_name, sell->name)))
		  ||(sell->flag == SSF_CLASS && is_name(cl->name, sell->name))
		  ||(sell->flag == SSF_SEX && is_name(flag_string(sex_table, ch->sex), sell->name))
		  ||(sell->flag == SSF_ALIGN && (
		  	(IS_GOOD(ch) && is_name("good", sell->name))
		  	|| (IS_NEUTRAL(ch) && is_name("neutral", sell->name))
		  	|| (IS_EVIL(ch) && is_name("evil", sell->name))
		  ))
		  ||(sell->flag == SSF_ETHOS && ch->pcdata && is_name(flag_string(ethos_table, ch->pcdata->ethos),sell->name)))
			return sell;
	}
	return NULL;
}

void do_research(CHAR_DATA *ch, const char *argument)
{
	class_t *cl;
	race_t *r;
	skill_t *sk;
	sellskill_t *sell;
	char arg[MAX_INPUT_LENGTH];
	int iSkill, iSell;
	BUFFER *output;
	
	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL
	   || (r = race_lookup(ch->race)) == NULL)
		return;
	
	if (argument[0] == '\0') {
		char_puts("Syntax: research list\n", ch);
		char_puts("Syntax: research do <skill/spell>\n", ch);
		if (IS_IMMORTAL(ch))
			char_puts("Syntax: research full\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	if (!str_prefix(arg, "full") && IS_IMMORTAL(ch)) {
		output = buf_new(-1);
		buf_add(output,"Full list:\n");
		for (iSkill = 0; iSkill < skills.nused; iSkill++){
			sk = skill_lookup(iSkill);
			if (sk->sell.nused)
				for (iSell = 0; iSell < sk->sell.nused; iSell++) {
					sell = (sellskill_t *) varr_get(&sk->sell, iSell);
					buf_printf(output, "%-25s-> %s VnumSeller: %6d  Cost: %6d  MinLev: %d\n"
							"                         List: %s\n",
						sk->name,
						flag_string(sell_skill_type, sell->flag),
						/*sell->flag == SSF_RACE ?		"RACE  "
							: sell->flag == SSF_CLASS ?	"CLASS "
							: sell->flag == SSF_ALL ?	"ALL   "
							: sell->flag == SSF_SEX ?	"SEX   "
							: sell->flag == SSF_ALIGN ?	"ALIGN "
							: sell->flag == SSF_ETHOS ?	"ETHOS "
							: 				"NONE  ",*/
						sell->vnum_seller, sell->cost, sell->minlev, sell->name);
				}
		}
		page_to_char(buf_string(output), ch);
		buf_free(output);
	} else if (!str_prefix(arg, "list")){
		MOB_INDEX_DATA	*pMob;
		AREA_DATA	*pArea;
		output = buf_new(-1);
		
		buf_add(output,"=============================================================================\n");
		buf_add(output,"|     Name skill/spell     |  Cost | MinLev | Instructor         |   Area   |\n");
		buf_add(output,"-----------------------------------------------------------------------------\n");
		
		for (iSkill = 0; iSkill < skills.nused; iSkill++)
		   if ((sell = sellskill_lookup(ch, iSkill)) != NULL && (sk = skill_lookup(iSkill)) != NULL) {
		   	pMob = get_mob_index(sell->vnum_seller);
		   	pArea = area_vnum_lookup(sell->vnum_seller);
			buf_printf(output, "| %-25.25s|%6d |  %3d   | %-18.18s | %-8.8s |\n",
				sk->name, sell->cost, sell->minlev,
				pMob ? mlstr_cval(pMob->short_descr, ch) : "none",
				pArea ? pArea->name : "none");
		   }
		buf_add(output,"=============================================================================\n");
		page_to_char(buf_string(output), ch);
		buf_free(output);
	} else if (!str_prefix(arg, "do")){
		CHAR_DATA	*mob;
		bool		found = FALSE;
		int		iPs;
		pcskill_t	*psk;

		argument = one_argument(argument, arg, sizeof(arg));
		if ((sk = skill_lookup(iSkill = sn_lookup(arg))) == NULL) {
			char_puts("Unknown skill/spell.\n", ch);
			return;
		}
		if ((sell = sellskill_lookup(ch, iSkill)) == NULL) {
			char_puts("You can't research this skill/spell.\n", ch);
			return;
		}
		for (iPs = 0; iPs < ch->pcdata->learned.nused; iPs++)
			if ((psk = (pcskill_t *) varr_get(&ch->pcdata->learned, iPs))->sn == iSkill) {
				if (psk->percent == 0 || psk->maxpercent < 1 || psk->level > MAX_LEVEL)
					break;
				char_puts("You don't need to research this skill/spell.\n", ch);
				return;
			}
		if (sell->minlev > ch->level) {
			char_puts("You are inexperienced for it.\n", ch);
			return;
		}
		for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room)
			if (IS_NPC(mob) && mob->pIndexData->vnum == sell->vnum_seller) {
				found = TRUE;
				break;
			}
		if (!found || mob == NULL || !can_see(ch, mob)){
			char_puts("You couldn't find anyone who is able to help you research this skill/spell.\n", ch);
			return;
		}
		if (!can_see(mob, ch)) {
			do_say(mob, "I can't see you.");
			return;
		}
		doprintf(interpret, mob, "say You must pay me %d silver for my help.", sell->cost);
		if (sell->cost > ch->silver + ch->gold * 100) {
			do_say(mob, "You can't afford research.");
			return;
		}
		deduct_cost(ch, sell->cost);
		doprintf(interpret, mob, "say Gratz! You've just researched '%s'", sk->name);
		set_skill_raw(ch, iSkill, 1, TRUE,
			UMIN( UMIN(LEVEL_HERO, sell->minlev + 10), ch->level),
			95, 55);
	} else
		do_research(ch, str_empty);
}

void do_learn(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int sn;
	CHAR_DATA *practicer;
	int adept;
	class_t *cl;
	cskill_t *cs;
	pcskill_t *ps;
	skill_t *sk;
	int hard;

	if (IS_NPC(ch) || (cl = class_lookup(ch->class)) == NULL)
		return;

	if (!IS_AWAKE(ch)) {
		char_puts("In your dreams, or what?\n", ch);
		return;
	}	

	if (argument[0] == '\0') {
		char_puts("Syntax: learn <skill | spell> <player>\n", ch);
		return;
	}

	if (ch->pcdata->practice <= 0) {
		char_puts("You have no practice sessions left.\n", ch);
		return;
	}

	argument = one_argument(argument, arg, sizeof(arg));
	ps = (pcskill_t*) skill_vlookup(&ch->pcdata->learned, arg);
	if (!ps || ps->percent == 0) {
		char_puts("You can't learn that.\n", ch);
		return;
	}

	if (sn == gsn_vampire) {
		char_puts("You can't practice that, only available "
			  "at questor.\n", ch);
		return;
	}	

	argument = one_argument(argument, arg, sizeof(arg));
		
	if ((practicer = get_char_room(ch,arg)) == NULL) {
		char_puts("Your hero is not here.\n", ch);
		return;
	}
			
	if (IS_NPC(practicer) || practicer->level != HERO) {
		char_puts("You must find a hero, not an ordinary one.\n",
			  ch);
		return;
	}

	if (!IS_SET(practicer->pcdata->plr_flags, PLR_PRACTICER)) {
		char_puts("Your hero doesn't want to teach you anything.\n",ch);
		return;
	}

	if (get_skill(practicer, sn) < (adept = get_maxskill(practicer,sn))) {
		char_puts("Your hero doesn't know that skill enough to teach you.\n",ch);
		return;
	}

	sk = SKILL(sn);
	adept /= 1.5;

	if (ps->percent >= adept) {
		char_printf(ch, "You are already learned at %s.\n",
			    sk->name);
		return;
	}

	ch->pcdata->practice--;

	cs = cskill_lookup(cl, sn);
	hard = cs ? UMAX(cs->hard, 1) : 50;
	ps->percent += int_app[get_curr_stat(ch,STAT_INT)].learn / (((float)hard)/50*2);

	act("You teach $T.", practicer, NULL, sk->name, TO_CHAR);
	act("$n teaches $T.", practicer, NULL, sk->name, TO_ROOM);
	REMOVE_BIT(practicer->pcdata->plr_flags, PLR_PRACTICER);

	if (ps->percent < adept) {
		act("You learn $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n learn $T.", ch, NULL, sk->name, TO_ROOM);
	}
	else {
		ps->percent = adept;
		act("You are now learned at $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n is now learned at $T.", ch, NULL, sk->name, TO_ROOM);
	}
}

void do_teach(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch) || ch->level != LEVEL_HERO) {
		char_puts("You must be a hero.\n",ch);
		return;
	}
	SET_BIT(ch->pcdata->plr_flags, PLR_PRACTICER);
	char_puts("Now, you can teach youngsters your 100% skills.\n",ch);
}

const char *skill_name(int sn)
{
	skill_t *sk = varr_get(&skills, sn);
	if (sk)
		return sk->name;
	return "none";
}

int get_maxskill(CHAR_DATA *ch, int sn)
{
	int skill;
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL
	/*||  (IS_SET(sk->flags, SKILL_CLAN) && !clan_item_ok(ch->clan))*/)
		return 100;

	if (!IS_NPC(ch)) {
		pcskill_t *ps;

		if ( (ps = pcskill_lookup(ch, sn)) == NULL )
			skill = 100;
		else
			skill = ps->maxpercent;
	}
	else {
		skill = 100;
	}
	
	return skill;
}


/* for returning skill information */
int get_skill(CHAR_DATA *ch, int sn)
{
	int skill;
	skill_t *sk;
	RELIGION_DATA *rel;

	if ((sk = skill_lookup(sn)) == NULL
	/*||  (IS_SET(sk->flags, SKILL_CLAN) && !clan_item_ok(ch->clan))*/)
		return 0;

	if (IS_PC(ch)) {
		pcskill_t *ps;

		if ((ps = pcskill_lookup(ch, sn)) == NULL
		||  ps->level > ch->level)
			skill = 0;
		else
			skill = ps->percent;

		if (IS_SET(sk->group, GROUP_SLANG))
			return skill;
	}
	else {
		flag64_t act = ch->pIndexData->act;
		flag64_t off = ch->pIndexData->off_flags;

		/* mobiles */
		if (sk->spell)
		{
			if (sk->min_level > ch->level
			|| IS_SET(sk->flags, SKILL_CLAN)
			|| IS_SET(sk->group, GROUP_BEGUILING | GROUP_ILLUSION
				| GROUP_TRANSPORTATION | GROUP_DETECTION))
				skill = 0;
			else if (IS_SET(act, ACT_MAGE))
				skill = 20 + ch->level * 0.6;
			else if (IS_SET(act, ACT_CLERIC))
			{
				if (IS_SET(sk->group, GROUP_CURATIVE
					| GROUP_BENEDICTIONS
					| GROUP_CURATIVE
					| GROUP_ENHANCEMENT
					| GROUP_HARMFUL
					| GROUP_HEALING
					| GROUP_PROTECTIVE))
						skill = 15 + ch->level;
				else
					skill = 5 + ch->level / 4;
			} else if (IS_SET(act, ACT_UNDEAD))
			{
				if (IS_SET(sk->group,GROUP_COMBAT
					| GROUP_DRACONIAN
					| GROUP_MALADICTIONS
					| GROUP_NECROMANCY))
						skill = 25 + ch->level * 0.8;
				else
					skill = ch->level / 5;
			} else
				skill = 0;
		} else if (sn == gsn_track)
			skill = 100;
		else if (sn == gsn_hunt)
			skill = UMIN(100, 50 + ch->level);
		else if ((sn == gsn_sneak || sn == gsn_hide || sn == gsn_pick)
		     &&  IS_SET(act, ACT_THIEF))
			skill = ch->level * 2 + 20;
		else if (sn == gsn_brandish || sn == gsn_scrolls || sn == gsn_wands)
		{
			if (IS_SET(act, ACT_MAGE | ACT_CLERIC | ACT_UNDEAD))
				skill = 10 + ch->level / 2;
			else
				skill = 5;
		} else if (sn == gsn_backstab
		     &&  (IS_SET(act, ACT_THIEF) ||
			  IS_SET(off, OFF_BACKSTAB)))
			skill = ch->level * 2 + 20;
		else if (sn == gsn_dual_backstab
		     &&  (IS_SET(act, ACT_THIEF) ||
			  IS_SET(off, OFF_BACKSTAB)))
			skill = ch->level + 20;
		else if (sn == gsn_parry && IS_SET(off, OFF_PARRY))
			skill = URANGE(20, ch->level * 2 / 3, 100);
		else if (sn == gsn_dodge && IS_SET(off, OFF_DODGE))
			skill = URANGE(30, ch->level, 111);
		else if (sn == gsn_dirt && IS_SET(off, OFF_DIRT_KICK))
			skill = ch->level * 2;
 		else if (sn == gsn_shield_block)
			skill = 10 + ch->level / 4;
		else if (sn == gsn_second_attack &&
			 (IS_SET(act, ACT_WARRIOR | ACT_THIEF)))
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_third_attack && IS_SET(act, ACT_WARRIOR))
			skill = 4 * ch->level - 40;
		else if (sn == gsn_extra_attack && IS_SET(act, ACT_WARRIOR))
			skill = 4 * ch->level - 60;
		else if (sn == gsn_fourth_attack && IS_SET(act, ACT_WARRIOR))
			skill = 4 * ch->level - 60;
		else if (sn == gsn_hand_to_hand)
			skill = 40 + 2 * ch->level;
		else if (sn == gsn_swimming && IS_SET(off, OFF_SWIMMING))
			skill = 90 + ch->level / 4;
 		else if (sn == gsn_trip && IS_SET(off, OFF_TRIP)) 
			skill = 10 + 3 * ch->level;
 		else if ((sn == gsn_bash || sn == gsn_bash_door) &&
			 IS_SET(off, OFF_BASH))
			skill = 10 + 3 * ch->level;
		else if (sn == gsn_kick && IS_SET(off, OFF_KICK))
			skill = 10 + 3 * ch->level;
		else if ((sn == gsn_critical) && IS_SET(act, ACT_WARRIOR))
			skill = ch->level;
		else if (sn == gsn_disarm &&
			 (IS_SET(off, OFF_DISARM) ||
			  IS_SET(act, ACT_WARRIOR | ACT_THIEF)))
			skill = 20 + 3 * ch->level;
		else if (sn == gsn_grip &&
			 (IS_SET(act, ACT_WARRIOR | ACT_THIEF)))
			skill = ch->level;
		else if ((sn == gsn_berserk || sn == gsn_tiger_power) &&
			 IS_SET(off, OFF_BERSERK))
			skill = 3 * ch->level;
		else if (sn == gsn_rescue)
			skill = 40 + ch->level; 
		else if (sn == gsn_sword || sn == gsn_dagger ||
			 sn == gsn_spear || sn == gsn_mace ||
			 sn == gsn_axe || sn == gsn_flail ||
			 sn == gsn_whip || sn == gsn_polearm ||
			 sn == gsn_bow || sn == gsn_arrow || sn == gsn_lance)
			skill = 40 + 5 * ch->level / 2;
		else if (sn == gsn_crush && IS_SET(off, OFF_CRUSH))
			skill = 10 + 3 * ch->level;
		else
			skill = 0;
	}

	if (IS_AFFECTED(ch, AFF_CURSE)) {
		AFFECT_DATA* paf;
		bool found = FALSE;

		for (paf = ch->affected; paf; paf=paf->next) {
			if (paf->type == gsn_anathema 
			  && paf->location == APPLY_LEVEL) {
				skill = skill * 4 / (4 - paf->modifier);
				found = TRUE;
				break;
			  }
		}
		if (!found)
			skill *= 0.85;
	}
	
	if (ch->move <= 0) {
		if (sk->spell)
			skill /= 2;
		else
			skill *= 0.75;
	}

	if (IS_AFFECTED(ch, AFF_BLESS))
		skill = skill * 1.15;

	if (ch->daze > 0) {
		if (sk->spell)
			skill /= 2;
		else
			skill *= 0.66;
	}

	if (!IS_NPC(ch) && ch->pcdata->condition[COND_DRUNK]  > 10)
		skill = UMAX(0, (skill * (100 - ch->pcdata->condition[COND_DRUNK])) / 100);

	if (sk->spell
	&& skill > 70)
		skill = 70 + (skill - 70) / 3;

	skill += (skill * (get_curr_stat(ch, STAT_WIS) - 18)) / (sk->spell ? 50 : 100);

	if (IS_AFFECTED(ch, AFF_SKILL)) {
		AFFECT_DATA* paf;
		
		for (paf = ch->affected; paf; paf=paf->next)
			if ((paf->location == APPLY_5_SKILL || paf->location == APPLY_10_SKILL)
			 && paf->modifier == sn)
				skill += paf->location == APPLY_5_SKILL ? 5 : 10;
	}
	
	if ((rel = GET_CHAR_RELIGION(ch)))
	{
		if (IS_SET(sk->group, GROUP_BENEDICTIONS)
		&& IS_SET(rel->flags, RELIG_CHECK_BENEDICTION)
		&& get_char_faith(ch) < 0)
			skill *= 0.7;
		
		if (IS_SET(sk->group, GROUP_MALADICTIONS)
		&& IS_SET(rel->flags, RELIG_DARK_MALADICTION))
		{
			if (time_info.hour >= 18 || time_info.hour < 9)
			{
				if (get_char_faith(ch) > 1000)
					skill *= 1.1;
			} else
				skill *= 0.9;
		}
	}

	skill = UMIN(skill, 200);

	return skill > 1 ? skill : 0;
}

/*
 * Lookup a skill by name.
 */
int sn_lookup(const char *name)
{
	int sn;

	if (IS_NULLSTR(name))
		return -1;

	for (sn = 0; sn < skills.nused; sn++)
		if (!str_prefix(name, SKILL(sn)->name))
			return sn;

	return -1;
}

/*
 * Lookup skill in varr.
 * First field of structure assumed to be sn
 */
void *skill_vlookup(varr *v, const char *name)
{
	int i;

	if (IS_NULLSTR(name))
		return NULL;

	for (i = 0; i < v->nused; i++) {
		skill_t *skill;
		int *psn = (int*) VARR_GET(v, i);

		if ((skill = skill_lookup(*psn))
		&&  !str_prefix(name, skill->name))
			return psn;
	}

	return NULL;
}

/* for returning weapon information */
int get_weapon_sn(OBJ_DATA *wield)
{
	int sn;

	if (wield == NULL)
		return gsn_hand_to_hand;

	if (wield->pIndexData->item_type == ITEM_STAFF)
		return gsn_staves;
	if (wield->pIndexData->item_type != ITEM_WEAPON)
		return 0;

	switch (wield->value[0]) {
	default :               sn = -1;		break;
	case(WEAPON_SWORD):     sn = gsn_sword;		break;
	case(WEAPON_DAGGER):    sn = gsn_dagger;	break;
	case(WEAPON_SPEAR):     sn = gsn_spear;		break;
	case(WEAPON_MACE):      sn = gsn_mace;		break;
	case(WEAPON_AXE):       sn = gsn_axe;		break;
	case(WEAPON_FLAIL):     sn = gsn_flail;		break;
	case(WEAPON_WHIP):      sn = gsn_whip;		break;
	case(WEAPON_POLEARM):   sn = gsn_polearm;	break;
	case(WEAPON_BOW):	sn = gsn_bow;		break;
	case(WEAPON_ARROW):	sn = gsn_arrow;		break;
	case(WEAPON_LANCE):	sn = gsn_lance;		break;
	}
	return sn;
}

int get_weapon_skill(CHAR_DATA *ch, int sn)
{
	 int sk;

/* -1 is exotic */
	if (sn == -1)
		sk = 3 * ch->level;
	else if (!IS_NPC(ch))
		sk = get_skill(ch, sn);
	else if (sn == gsn_hand_to_hand)
		sk = 40 + 2 * ch->level;
	else 
		sk = 40 + 5 * ch->level / 2;

	return URANGE(0, sk, 100);
} 

#define ASSF_NONE	(0)
#define ASSF_VERBAL	(A)	// say
#define ASSF_SOMATIC	(B)	// some action
#define ASSF_COMPONENT	(C)	// used object

/*
 * Utter mystical words and actions
 * 	Return TRUE if victim see action
 */
bool action_spell(CHAR_DATA *ch, skill_t *sk, CHAR_DATA *victim)
{
	char buf  [MAX_STRING_LENGTH];
	const char *str_verb = NULL;
	CHAR_DATA *rch;
	const char *pName;
	int iSyl;
	int length;
	flag32_t	show_bits, sflag;
	bool ret = FALSE;
	flag32_t	type = sk->spell->type;
	varr *comp = &(sk->spell->components);
			// struct spell_component *

	struct syl_type
	{
		char *	old;
		char *	new;
	};

	static const struct syl_type syl_table[] =
	{
		{ " ",		" "		},
		{ "ar",		"abra"		},
		{ "au",		"kada"		},
		{ "bless",	"fido"		},
		{ "blind",	"nose"		},
		{ "bur",	"mosa"		},
		{ "cu",		"judi"		},
		{ "de",		"oculo"		},
		{ "en",		"unso"		},
		{ "light",	"dies"		},
		{ "lo",		"hi"		},
		{ "mor",	"zak"		},
		{ "move",	"sido"		},
		{ "ness",	"lacri"		},
		{ "ning",	"illa"		},
		{ "per",	"duda"		},
		{ "ra",		"gru"		},
		{ "fresh",	"ima"		},
		{ "re",		"candus"	},
		{ "son",	"sabru"		},
		{ "tect",	"infra"		},
		{ "tri",	"cula"		},
		{ "ven",	"nofo"		},
		{ "a", "a" }, { "b", "b" }, { "c", "q" }, { "d", "e" },
		{ "e", "z" }, { "f", "y" }, { "g", "o" }, { "h", "p" },
		{ "i", "u" }, { "j", "y" }, { "k", "t" }, { "l", "r" },
		{ "m", "w" }, { "n", "i" }, { "o", "a" }, { "p", "s" },
		{ "q", "d" }, { "r", "f" }, { "s", "g" }, { "t", "h" },
		{ "u", "j" }, { "v", "z" }, { "w", "x" }, { "x", "n" },
		{ "y", "l" }, { "z", "k" },
		{ str_empty, str_empty }
	};
	
	static const flag_t spell_show_table[] =
	{
		{ "$n utters the words, '$t'.",			ASSF_VERBAL	},
		{ "$n makes some motions by hands.",		ASSF_SOMATIC	},
		{ "$n attempts to manipulate with something.",	ASSF_COMPONENT	},
		{ "$n whispers '$t' and wave by hands.",	ASSF_VERBAL | ASSF_SOMATIC	},
		{ "$n whispers '$t' at something.",		ASSF_VERBAL | ASSF_COMPONENT	},
		{ "$n attempts to make some actions with something.",	ASSF_SOMATIC | ASSF_COMPONENT	},
		{ "$n whispers '$t' at something and makes some actions.",
				ASSF_VERBAL | ASSF_SOMATIC | ASSF_COMPONENT },

		{ NULL }
	};

	if (!IS_SET(type, SPT_VERBAL | SPT_SOMATIC)
	&& comp->nused == 0)
		return ret;

	if (IS_SET(type, SPT_VERBAL))
	{
		buf[0]	= '\0';
		for (pName = sk->name; *pName != '\0'; pName += length) {
			for (iSyl = 0; (length = strlen(syl_table[iSyl].old)); iSyl++) {
				if (!str_prefix(syl_table[iSyl].old, pName)) {
					strnzcat(buf, sizeof(buf), syl_table[iSyl].new);
					break;
				}
			}
			if (length == 0)
				length = 1;
		}
	}

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch)
			continue;
		show_bits = ASSF_NONE;

		if (IS_SET(type, SPT_VERBAL)
		&& !rch->daze
		&& !IS_AFFECTED(rch, AFF_SCREAM | AFF_STUN | AFF_WEAK_STUN)
		&& rch->in_room->sector_type != SECT_UNDER_WATER)
		{
			SET_BIT(show_bits, ASSF_VERBAL);
			if ((get_skill(rch, gsn_spell_craft) * 9) / 10
					< number_percent())
			{
				str_verb = buf;
				check_improve(rch, gsn_spell_craft, FALSE, 5);
			} else  {
				str_verb = sk->name;
				check_improve(rch, gsn_spell_craft, TRUE, 5);
				if (victim == rch)
					ret = TRUE;
			}
		}

		if (comp->nused > 0)
		{
			sflag = 0;
			for (iSyl = 0; iSyl < comp->nused; iSyl++)
				SET_BIT(sflag, ((struct spell_component *) VARR_GET(comp, iSyl))->flags);

			if (!IS_AFFECTED(rch, AFF_BLIND)
			&& (IS_SET(sflag, SPC_NEED_GROUND | SPC_NEED_EQ)
				|| get_skill(rch, gsn_peek) > number_percent()))
			{
				SET_BIT(show_bits, ASSF_COMPONENT);
				if (victim == rch)
					ret = TRUE;
			}
		}

		if (IS_SET(type, SPT_SOMATIC)
		&& !IS_AFFECTED(rch, AFF_BLIND))
		{
			SET_BIT(show_bits, ASSF_SOMATIC);
			if (victim == rch)
				ret = TRUE;
		}

		if (show_bits == ASSF_NONE)
			continue;
		for (sflag = 0; spell_show_table[sflag].name; sflag++) {
			if (spell_show_table[sflag].bit != show_bits)
				continue;
			act_puts3(spell_show_table[sflag].name,
				ch, str_verb, rch, NULL, TO_VICT, POS_RESTING);
		}
	}
	return ret;
}

/* find min level of the skill for char */
int skill_level(CHAR_DATA *ch, int sn)
{
	int slevel = MAX_LEVEL+1;
	skill_t *sk;
	class_t *cl;
	race_t *r;
	pcskill_t       *ps;

/* noone can use ill-defined skills */
/* broken chars can't use any skills */
	if (IS_NPC(ch)
	||  (sk = skill_lookup(sn)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL
	||  (r = race_lookup(ch->race)) == NULL
	||  !r->pcdata)
		return slevel;

	if ((ps = pcskill_lookup(ch, sn)) == NULL)
		return MAX_LEVEL+1;
	if ( ps->percent == 0)
		return MAX_LEVEL+1;
	return ps->level;

}
 
/*
int skill_level_own(CHAR_DATA *ch, int sn)
{
 int i;
 pcskill_t       *ps;

    if (!ch->pcdata->remort)
	return MAX_LEVEL+1;
 for (i = 0; i < ch->pcdata->remort->skills.nused; i++) {
        ps = VARR_GET(&ch->pcdata->remort->skills, i);
        if ( (ps->sn) == sn ) {
                if ( ps->percent == 0) break;
                return ps->level;
        }
 }
 return MAX_LEVEL+1;
}
 */        

/*
 * assumes !IS_NPC(ch) && ch->level >= skill_level(ch, sn)
 */
int mana_cost(CHAR_DATA *ch, int sn)
{
	skill_t *sk;

	if ((sk = skill_lookup(sn)) == NULL)
		return 0;

	return UMAX(sk->min_mana, 100 / (2 + ch->level - skill_level(ch, sn)));
}
