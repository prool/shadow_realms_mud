/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * (c) Shadow Realms	1999 - 2000 year (Xor & Taelas)	ver 0.01
 *			2001 - 2002 year by Xor		ver 1.00
 *  make_drunk() make by Bellionore.... special thx him :)
 *	MUD ForEv'a
 */

#if defined( macintosh )
#include <types.h>
#else
#include <sys/types.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "const.h"

DECLARE_DO_FUN(do_help		);
void show_remort_data(CHAR_DATA *ch, bool show_buy);

void wrath_of_gods(CHAR_DATA *ch, int align)
{
	race_t *r;
	class_t *cl;
	flag32_t restrict_; // restrict renamed to restrict_ by prool
	int str;
	AFFECT_DATA af;
	const char * gdislike = "Gods dislike your action.\n";

	if (IS_NPC(ch) || align == 0
	|| (r = race_lookup(ORG_RACE(ch))) == NULL
	|| (cl = class_lookup(ch->class)) == NULL
	|| !r->pcdata)
		return;
	
	if ((restrict_ = r->pcdata->allow_align) == 0 // allow all
	 && (restrict_ = cl->restrict_align) == 0)
		return;
	
	str = 0;
	switch (restrict_){
		case RA_GOOD:
			if (IS_GOOD(ch) || align > 0)
				return;
			if (ch->alignment > 60) {
				char_puts(gdislike, ch);
				return;
			}
			str += 1 - (ch->alignment - 100) / 100;
			break;
		case RA_NEUTRAL:			
			if (IS_NEUTRAL(ch)
			|| (IS_GOOD(ch) && align < 0)
			|| (IS_EVIL(ch) && align > 0))
				return;
			if (ch->alignment > -140 || ch->alignment < 140) {
				char_puts(gdislike, ch);
				return;
			}
			str += abs(ch->alignment) / 100;
			break;
		case RA_EVIL:
			if (IS_EVIL(ch) || align < 0)
				return;
			if (ch->alignment < -60) {
				char_puts(gdislike, ch);
				return;
			}
			str += 1 + (ch->alignment + 100) / 100;
			break;
		default:
			bug("wrath_of_gods -> incorrect restrict !", 0);
			return;
	}
	
	str = URANGE(1, str, 10);
	
	af.where        = TO_AFFECTS;
	af.type         = gsn_wrath_of_gods;
	af.level        = 1000;
	af.duration     = -1;
	af.bitvector    = 0;
	af.modifier     = 0 - UMAX(1, str / 3);
	
	af.location     = APPLY_STR;
	affect_join(ch, &af);
	
	af.location     = APPLY_INT;
	affect_join(ch, &af);
	
	af.location     = APPLY_WIS;
	affect_join(ch, &af);
	
	af.location     = APPLY_DEX;
	affect_join(ch, &af);
	
	af.location     = APPLY_CON;
	affect_join(ch, &af);
	
	af.location     = APPLY_CHA;
	affect_join(ch, &af);
	
	if (str < 4){
		char_puts("{RGods is discontent.{x\n", ch);
		return;
	}
	
	af.location     = APPLY_NONE;
	af.modifier     = 0;
	if (!IS_AFFECTED(ch, AFF_CURSE)){
		af.bitvector    = AFF_CURSE;
		affect_to_char(ch, &af);
	}
	
	if (str < 7){
		char_puts("{RGods is wrath.{x\n", ch);
		return;
	}
	
	
	af.location     = APPLY_LEVEL;
	af.modifier     = -str;
	if (IS_AFFECTED(ch, AFF_BLIND))
		af.bitvector = 0;
	else
		af.bitvector    = AFF_BLIND;
	affect_join(ch, &af);
	
	char_puts("{RGods is fury.{x\n", ch);
}

const char *make_drunk (CHAR_DATA *ch, const char *string)
{
/* This structure defines all changes for a character */
  struct struck_drunk drunk[] =
  {
    {3, 10,
     {"a", "a", "a", "A", "aa", "ah", "Ah", "ao", "aw", "oa", "ahhhh"}},
    {8, 5,
     {"b", "b", "b", "B", "B", "vb"}},
    {3, 5,
     {"c", "c", "C", "cj", "sj", "zj"}},
    {5, 2,
     {"d", "d", "D"}},
    {3, 3,
     {"e", "e", "eh", "E"}},
    {4, 5,
     {"f", "f", "ff", "fff", "fFf", "F"}},
    {8, 2,
     {"g", "g", "G"}},
    {9, 6,
     {"h", "h", "hh", "hhh", "Hhh", "HhH", "H"}},
    {7, 6,
     {"i", "i", "Iii", "ii", "iI", "Ii", "I"}},
    {9, 5,
     {"j", "j", "jj", "Jj", "jJ", "J"}},
    {7, 2,
     {"k", "k", "K"}},
    {3, 2,
     {"l", "l", "L"}},
    {5, 8,
     {"m", "m", "mm", "mmm", "mmmm", "mmmmm", "MmM", "mM", "M"}},
    {6, 6,
     {"n", "n", "nn", "Nn", "nnn", "nNn", "N"}},
    {3, 6,
     {"o", "o", "ooo", "ao", "aOoo", "Ooo", "ooOo"}},
    {3, 2,
     {"p", "p", "P"}},
    {5, 5,
     {"q", "q", "Q", "ku", "ququ", "kukeleku"}},
    {4, 2,
     {"r", "r", "R"}},
    {2, 5,
     {"s", "ss", "zzZzssZ", "ZSssS", "sSzzsss", "sSss"}},
    {5, 2,
     {"t", "t", "T"}},
    {3, 6,
     {"u", "u", "uh", "Uh", "Uhuhhuh", "uhU", "uhhu"}},
    {4, 2,
     {"v", "v", "V"}},
    {4, 2,
     {"w", "w", "W"}},
    {5, 6,
     {"x", "x", "X", "ks", "iks", "kz", "xz"}},
    {3, 2,
     {"y", "y", "Y"}},
    {2, 9,
     {"z", "z", "ZzzZz", "Zzz", "Zsszzsz", "szz", "sZZz", "ZSz", "zZ", "Z"}}
  };
  
  struct struck_drunk drunkr[] =
  {
    {4, 3,
     {"À", "À", "à", "àÕÀ"}},
    {3, 9,
     {"Á", "Á", "á", "ÁÁ", "ÁÈ", "áÈ", "ÁÏ", "ÏÁ", "ÁÑ", "ÁÈÈÈÈ"}},
    {7, 10,
     {"Â", "Â", "â", "Ð", "ðÂ", "ÂÐ", "Ðâ", "ÐÂ", "Âð", "ÐÐ", "ÐÂÐ"}},
    {5, 7,
     {"Ã", "Ã", "ã", "ãÚ", "ÚÃ", "úÃ", "ÃÃ", "ÃÚ"}},
    {6, 2,
     {"Ä", "Ä", "ä"}},
    {3, 10,
     {"Å", "e", "eee", "Ee", "å", "Åx", "åÈ", "ÅÏ", "ÏÅ", "eê", "ÅÅÊÊÊ"}},
    {5, 7,
     {"Æ", "æ", "×", "÷", "ÆÆ", "æÆ", "ÆÅ", "Æ"}},
    {6, 2,
     {"Ç", "Ç", "ç"}},
    {4, 4,
     {"È", "È", "è", "èÅ", "ÈÈÈ"}},
    {2, 8,
     {"É", "É", "é", "ÉË", "éË", "Ê", "ÉÅ", "ÉÁ", "ÉÉÉÉÉÉ"}},
    {2, 6,
     {"Ê", "Ê", "ê", " ", "êË", "ÉË", "éË"}},
    {7, 4,
     {"Ë", "Ë", "ë", "ËÁ", "ËËËË"}},
    {3, 2,
     {"Ì", "Ì", "ì"}},
    {5, 7,
     {"Í", "Í", "í", "íÍ", "ÍÍÍ", "ÍÅÅ", "ÍíÍ", "íÍí"}},
    {6, 6,
     {"Î", "Î", "î", "îÎ", "ÎÅÅ", "ÎÎ", "ÎÎÎ"}},
    {3, 7,
     {"Ï", "Ï", "ï", "ÏÏÏ", "ïÁ", "ÏÁ", "ïÏÏ", "ÏïÏ"}},
    {3, 2,
     {"Ð", "Ð", "ð"}},
    {2, 8,
     {"Ñ", "Ñ", "ñ", "ÁÑ", "ÑÑÑ", "Á", "ÉËÁ", "ÊÁ", "ÁÁÁ"}},
    {4, 3,
     {"Ò", "Ò", "ò", "ÒÒ"}},
    {3, 10,
     {"Ó", "Ó", "ó", "ÓÓÓ", "ÓÓ", "óÓ", "Óó", "ÓÅÅÅ", "ÓÕÕ", "óóóóóÓÓÓÓÓ", "ÓÓÓÓÓÓÓÓ"}},
    {7, 2,
     {"Ô", "Ô", "ô"}},
    {3, 8,
     {"Õ", "Õ", "õ", "ÕÕ", "ÕÕÕÕ", "À", "õÕÕõ", "ÊÕÕ", "ÁÕÕÕ"}},
    {6, 3,
     {"Ö", "Ö", "ö", "ÖÖÖ"}},
    {4, 2,
     {"×", "×", "÷"}},
    {2, 2,
     {"Ø", "ø", "ø"}},
    {5, 5,
     {"Ù", "Ù", "ù", "ÙÙÙ", "ÙÊ", "Êù"}},
    {5, 3,
     {"Ú", "Ú", "ú", "ÚÙ"}},
    {4, 5,
     {"Û", "Û", "û", "ÛÅ", "ÛÜÜÜ", "ÛÛÛ"}},
    {2, 4,
     {"Ü", "Ü", "ü", "Ù", "ÕÕÕ"}},
    {3, 6,
     {"Ý", "Ý", "ý", "Û", "ÛÛÛ", "ÛÅÅ", "ÁÝ"}},
    {4, 5,
     {"Þ", "Þ", "þ", "Û", "ÞÅÅ", "ÜÜÜ"}},
    {4, 2,
     {"ß", "ß", "ÿ"}}
  };

  static char buf[MAX_STRING_LENGTH];
  char temp;
  int pos = 0;
  int drunklevel;
  int randomnum;

  /* Check how drunk a person is... */
  if (IS_NPC(ch))
        drunklevel = 0;
  else
        drunklevel = ch->pcdata->condition[COND_DRUNK] / 2;

  if (drunklevel > 0)
  {
	do {
		temp = to_lower_case (*string);
		if ((temp >= 'a') && (temp <= 'z'))
		{
			if (drunklevel > drunk[temp - 'a'].min_drunk_level) {
				randomnum = number_range (0, drunk[temp - 'a'].number_of_rep);
				strncpy (&buf[pos], drunk[temp - 'a'].replacement[randomnum], sizeof(buf));
				pos += strlen (drunk[temp - 'a'].replacement[randomnum]);
			} else
				buf[pos++] = *string;
		} else if (temp >= 'À' && temp <= 'ß') {
			if (drunklevel > drunkr[temp - 'À'].min_drunk_level) {
				randomnum = number_range (0, drunkr[temp - 'À'].number_of_rep);
				strncpy (&buf[pos], drunkr[temp - 'À'].replacement[randomnum], sizeof(buf));
				pos += strlen (drunkr[temp - 'À'].replacement[randomnum]);
			} else
				buf[pos++] = *string;
		} else 	if ((temp >= '0') && (temp <= '9')) {
			temp = '0' + number_range (0, 9);
			buf[pos++] = temp; 
		} else
			buf[pos++] = temp;
	} while (*string++ || pos <= MAX_STRING_LENGTH - 1);
      
	if (pos > MAX_STRING_LENGTH)
		pos = MAX_STRING_LENGTH;
	buf[pos] = '\0';          /* Mark end of the string... */
	return(buf);
   }
   return (string);
}

void do_ownlist(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;
	if (ch->pcdata->remort == NULL) {
		char_puts("You have not own parametrs.\n", ch);
		char_printf(ch, "You have %d remort points.\n", START_RP);
		return;
	}
	show_remort_data(ch, FALSE);
}

typedef struct type_remort_price type_remort_price;
struct  type_remort_price
{
	const char	*that;	// short name
	int             type;	// type of field
	int             price;	// started price
	const char      *info;	// long name
	flag32_t	flags;
};

#define RP_FLAG_NUMBER	(A)
#define RP_FLAG_SHORT	(B)
#define RP_FLAG_STRING	(C)
#define RP_FLAG_LONG	(D)

type_remort_price	remort_price[] =
{
	{"hitroll",	RP_HITROLL,	2,	"1 hitroll of hero level (growning price)",	RP_FLAG_NUMBER},
	{"damroll",	RP_DAMROLL,	3,	"1 damroll of hero level (growning price)",	RP_FLAG_NUMBER},
	{"hp",		RP_HIT,		1,	"5 hitpoint of hero level (growning price)",	RP_FLAG_NUMBER},
	{"mana",	RP_MANA,	1,	"5 mana of hero level (growning price)",	RP_FLAG_NUMBER},
	{"move",	RP_MOVE,	1,	"2 move of hero level (growning price)",	RP_FLAG_NUMBER},
	{"questp",	RP_QP,		1,	"50 questpoints",				RP_FLAG_NUMBER},
	{"train",	RP_TRAIN,	10,	"1 train",					RP_FLAG_NUMBER},
	{"ethos",	RP_ETHOS,	3,	"change your ethos",				RP_FLAG_STRING},
	{"align",	RP_ALIGN,	3,	"change your align",				RP_FLAG_STRING},
	{"race",	RP_RACE,	5,	"change your race",				RP_FLAG_STRING},
	{"sex",		RP_SEX,		10,	"change your sex",				RP_FLAG_STRING},
	{"prac",	RP_PRACTICE,	3,	"1 practice",					RP_FLAG_NUMBER},
	{"skill",	RP_SKILL,	DEFAULT_PRICE_SKILL,	"skill, that have your class (default price)",	RP_FLAG_STRING},
	{"lskill",	RP_LEVEL_SKILL, 2,			"decrease one level to skill, that you have",	RP_FLAG_LONG},
	{"pskill",	RP_PERC_SKILL,	1,			"add one max percent to skill, that you have",	RP_FLAG_LONG},
	{"rp",		RP_RP,		8,			"save any quantity remort points till next remort", RP_FLAG_NUMBER},
	{"class",	RP_CLASS,	5,			"change your class",				RP_FLAG_STRING},
	{NULL}
};

type_remort_price	*lookup_rp(int type)
{
	type_remort_price       *rp = remort_price;

	while(rp->that != NULL) {
		if (rp->type == type)
			return rp;
		rp++;
	}
	return NULL;
}

type_remort_price       *lookup_str_rp(const char *arg)
{
	type_remort_price       *rp = remort_price;
	
	while(rp->that != NULL) {
		if (!str_prefix(arg, rp->that))
			return rp;
		rp++;
	}
	return NULL;
}

int get_price_rp(CHAR_DATA *ch, int type, int x)
{
	int price, i, num;
	type_remort_price	*rp;
	REMORT_DATA		*remort;
	RSHOP_DATA		*shop;
	
	if ((rp = lookup_rp(type)) == NULL
	|| (remort = ch->pcdata->remort) == NULL
	|| (shop = ch->pcdata->remort->shop) == NULL)
		return -1;

	price = 0;
	switch (type)
	{
		case RP_HITROLL:
				num = shop->hitroll;
				for (i = 0; i < x; i++) {
					num++;
					price += rp->price * (20 + num) / 20;
				}
				break;
		case RP_DAMROLL:
				num = shop->damroll;
				for (i = 0; i < x; i++) {
					num++;
					price += rp->price * (40 + num) / 40;
				}
				break;
		case RP_HIT:
				num = shop->hit;
				i = x;
				while((i -= 5) > -5) {
					num += 5;
					price += rp->price * (75 + num) / 75;
				}
				break;
		case RP_MANA:
				num = shop->mana;
				i = x;
				while((i -= 5) > -5) {
					num += 5;
					price += rp->price * (100 + num) / 100;
				}
				break;
		case RP_MOVE:
				num = shop->move;
				i = x;
				while((i -= 2) > -2) {
					num += 2;
						price += rp->price * (50 + num) / 50;
				}
				break;
		case RP_QP:
				if (x > 0)
					price = rp->price * (x + 49) / 50;
				else
					price = 0;
				break;
		case RP_TRAIN:
		case RP_PRACTICE:
		case RP_LEVEL_SKILL:
		case RP_PERC_SKILL:
				price = rp->price * x;
				break;
		case RP_RP:
				price = (shop->rp > 0) ? x : rp->price + x;
				break;
		case RP_ETHOS:
		case RP_ALIGN:
		case RP_RACE:
		case RP_SEX:
		case RP_CLASS:
				price = rp->price;
				break;
		case RP_SKILL:
				if (IS_SET(skill_lookup(x)->flags, SKILL_NOREMORT))
					price = -1;
				else
					price = skill_lookup(x)->remort_cost;
				break;
		default:
				log_printf("[BUG] Unknow rp type(%s)[get_price_rp].", ch->name);
				price = -1;
				break;
	}
	return price;
}

void copy_remort_data(RSHOP_DATA *r_in, REMORT_DATA *r_out)
{
	int i;
	pcskill_t *ps_in, *ps_out;
	
	r_in->points = r_out->points;
	r_in->hitroll = r_out->hitroll;
	r_in->damroll = r_out->damroll;
	r_in->hit = r_out->hit;
	r_in->mana = r_out->mana;
	r_in->move = r_out->move;
	
	for (i = 0; i < r_out->skills.nused; i++) {
		ps_out = VARR_GET(&r_out->skills, i);
		ps_in = varr_enew(&r_in->skills);
		ps_in->sn = ps_out->sn;
		ps_in->percent = ps_out->percent;
		ps_in->level = ps_out->level;
		ps_in->maxpercent = ps_out->maxpercent;
		ps_in->hard = ps_out->hard;
	}

	varr_qsort(&r_in->skills, cmpint);
}

void show_price_rp(CHAR_DATA *ch)
{
	int i;
	type_remort_price	*rp = remort_price;
	BUFFER *output;
	const char *line =
		"=======================================================================\n";
	
	if (IS_NPC(ch))
		return;
	
	output = buf_new(-1);
	buf_add(output, line);
	buf_add(output,
		"|   That   |                    Info                          | Price |\n");
	buf_add(output, line);	
 	for (i = 0; i < NUMBER_RP; i++) {
 		rp = lookup_rp(i);
 		if (rp == NULL)
 			continue;
 		buf_printf(output, "| %-9s| %-49s| %5d |\n", rp->that, rp->info, get_price_rp(ch, i, 1));
 	}
	
	buf_add(output, line);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

int do_remort_buy(CHAR_DATA *ch, const char *argument, bool test)
{
/*
 *	If test == TRUE, show only price for this actions and return (value);
 *	else 'buy * * *'....
 */
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	type_remort_price       *rp;
	int num = 0;
	int price = -1;
	int tmp;
	RSHOP_DATA	*shop = ch->pcdata->remort->shop;
	
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	argument = one_argument(argument, arg3, sizeof(arg3));
	
	if (arg1[0] == '\0' || ((rp = lookup_str_rp(arg1)) == NULL)) {
		char_puts("Unknow type! Syntax:\n",ch);
		if (test)
			char_puts("value <type> [<that>] [<param>]\n", ch);
		else
			char_puts("buy <type> [<that>] [<param>]\n", ch);
		return price;
	}
	
	if ((IS_SET(rp->flags, RP_FLAG_NUMBER)
	  || IS_SET(rp->flags, RP_FLAG_STRING)
	  || IS_SET(rp->flags, RP_FLAG_LONG))
	&& arg2[0] == '\0')
	{
		char_puts("You must enter <that> for this type.\n", ch);
		return price;
	}
	
	if (rp->type == RP_SKILL) {
		if ((num = sn_lookup(arg2)) < 0) {
			char_puts("Unknown skill(spell).\n", ch);
			return -1;
		}
	}
	
	if (IS_SET(rp->flags, RP_FLAG_NUMBER)) {
		if (is_number(arg2))
			num = atoi(arg2);
		else {
			char_puts("<that> must be numeric for this type.\n", ch);
			return price;
		}
	}

	if (IS_SET(rp->flags, RP_FLAG_LONG))
	{
		if (arg3[0] == '\0' || !is_number(arg3)) {
			char_puts("You must enter numeric <param> for this type.\n", ch);
			return price;
		}
		num = atoi(arg3);
	}

	price = get_price_rp(ch, rp->type, num);
	
	if (test)
	{
		switch (rp->flags)
		{
			case RP_FLAG_NUMBER:
			case RP_FLAG_LONG:
				char_printf(ch, "You may buy '%s' number of %d by %d rp.\n", rp->that, num, price);
				break;
			default:
				if (rp->type == RP_SKILL) {
					if (price > 0)
						char_printf(ch, "You may buy '%s' by %d rp.\n", skill_lookup(num)->name, price);
					else
						char_printf(ch, "Skill(spell) '%s' don't get with remort.\n", skill_lookup(num)->name);
				} else
					char_printf(ch, "You may buy '%s' type by %d rp.\n", rp->that, price);
				break;
		}
		return price;
	}
	
	if (price > shop->points) {
		char_puts("You don't have enough points for buy this.\n", ch);
		return price;
	}
	
	if (rp->flags == RP_FLAG_NUMBER)
		char_printf(ch, "You buy '%s' number of %d by %d rp.\n", rp->that, num, price);

	switch (rp->type) {
		case RP_HITROLL:
				shop->hitroll += num;
				break;
		case RP_DAMROLL:
				shop->damroll += num;
				break;
		case RP_HIT:
				shop->hit += num;
				break;
		case RP_MANA:
				shop->mana += num;
				break;
		case RP_MOVE:
				shop->move += num;
				break;
		case RP_TRAIN:
				shop->train +=num;
				break;
		case RP_PRACTICE:
				shop->practice += num;
				break;
		case RP_RP:
				shop->rp += num;
				break;
		case RP_QP:
				shop->qp += num;
				break;
		case RP_ETHOS:
				num = flag_value(ethos_table, arg2);
				if (num <= 0) {
					char_puts("Unknown ethos.\n", ch);
					return -1;
				}
				char_printf(ch, "You change your ethos to %s.\n", flag_string(ethos_table, num));
				shop->ethos = num;
				break;
		case RP_ALIGN:
				num = flag_value(align_names, arg2);
				shop->align = num;
				char_printf(ch, "You change your align to %s.\n", flag_string(align_names, num));
				break;
		case RP_RACE:
				num = rn_lookup(arg2);
				if (num <= 0 || !RACE(num)->pcdata) {
					char_puts("That is not a valid race.\n",ch);
					return -1;
				}
				shop->race = num;
				char_printf(ch, "You change your race to %s.\n", race_name(num));
				break;
		case RP_SEX:
				num = flag_value(sex_table, arg2);
				if (num < 1 || num > 2)
				{
					char_puts("That is not a valid sex.\n", ch);
					return -1;
				}
				shop->sex = num;
				char_printf(ch, "You change your sex to %s.\n", flag_string(sex_table, num));
				break;
		case RP_CLASS:
				num = cn_lookup(arg2);
				if (num <= 0)
				{
					char_puts("That is not a valid class.\n", ch);
					return -1;
				}
				shop->class = num;
				char_printf(ch, "You change your class to %s.\n", class_lookup(num)->name);
				break;
		case RP_SKILL:
				{
				pcskill_t *ps;
				pcskill_t *ps_own;
				skill_t *sk = skill_lookup(num);

				if (cskill_lookup(class_lookup(ch->class), num) == NULL
				||  varr_bsearch(&shop->skills, &num, cmpint)
				|| (ps = pcskill_lookup(ch, num)) == NULL
				|| IS_SET(sk->flags, SKILL_NOREMORT))
				{
					char_puts("You can not choose this skill(spell).\n", ch);
					return -1;
				}
				ps_own = varr_enew(&shop->skills);
				ps_own->sn = num;
				ps_own->maxpercent = UMAX(5, ps->percent - 15);
				ps_own->percent = 1;
				ps_own->level = UMIN(90, ps->level + 10);
				ps_own->hard = UMIN(90, ps->hard);
				varr_qsort(&shop->skills, cmpint);
				char_printf(ch, "You add '%s' to your list of skills(spells).\n", SKILL(num)->name);
				}
				break;
		case RP_LEVEL_SKILL:
		case RP_PERC_SKILL:
				if ((tmp = sn_lookup(arg2)) >= 0)
				{
					pcskill_t *ps;
					skill_t *sk;
					
					if ((ps = varr_bsearch(&shop->skills, &tmp, cmpint)) == NULL)
					{
						char_puts("You have not this skill(spell) in your list of skills.\n", ch);
						return -1;
					}
					sk = skill_lookup(tmp);
					if (rp->type == RP_LEVEL_SKILL) {
						num = UMIN(num, ps->level - sk->min_level);
						ps->level = ps->level - num;
						char_printf(ch, "You decrease level of '%s' on %d in your list of skills(spells).\n", sk->name, num);
					} else {
						num = UMIN(num, 99 - ps->maxpercent);
						ps->maxpercent = ps->maxpercent + num;
						char_printf(ch, "You increase max percent of '%s' on %d in your list of skills(spells).\n", sk->name, num);
					}
					price = get_price_rp(ch, rp->type, num);
				} else {
					char_puts("Unknow skill(spell).\n", ch);
					return -1;
				}
				break;
		default:
			return price;
	}
	shop->points -= price;
	return price;
}

void show_remort_data(CHAR_DATA *ch, bool show_buy)
{
/*
 *  If show_buy == TRUE, show remort_data (ownlist),
 *  else (show_buy == FALSE), show rshop_data (addlist)
 */
	REMORT_DATA		*remort = ch->pcdata->remort;
	RSHOP_DATA		*shop = remort->shop;
	type_remort_price	*rp;
	BUFFER			*output;
	int 			i;
	char			str[MAX_STRING_LENGTH];
	const char 		*line =
		"====================";
	int			num;
	
	output = buf_new(-1);
	buf_add(output, line);
	if (show_buy)
		buf_add(output, "=== you bought ===");
	else
		buf_add(output, "== you own list ==");

	buf_printf(output, "%s\n", line);
	
	for (i = 0; i < NUMBER_RP; i++)
	{
		rp = lookup_rp(i);
		if (rp == NULL)
			continue;
		str[0] = '\0';
		num = -1;
		if (!show_buy)
		{
			switch(rp->type)
			{
				case RP_HITROLL:
					num = remort->hitroll;
					break;
				case RP_DAMROLL:
					num = remort->damroll;
					break;
				case RP_HIT:
					num = remort->hit;
					break;
				case RP_MANA:
					num = remort->mana;
					break;
				case RP_MOVE:
					num = remort->move;
					break;
				case RP_SKILL:
					for (num = 0; num < remort->skills.nused; num++) {
						pcskill_t *ps;
						skill_t *sk;
						
						if ((ps = varr_get(&remort->skills, num)) == NULL)
							continue;
						if ((sk = skill_lookup(ps->sn)) == NULL)
							continue;
						buf_printf(output, "| %-9s| %-25s [%3d%%][%3dl][%3dh]|\n", sk->spell ? "spell" : "skill",
							sk->name, ps->maxpercent, ps->level, ps->hard);
					}
					continue;
				default:
					continue;
			}
		} else {
			switch(rp->type)
			{
				case RP_HITROLL:
					num = shop->hitroll - remort->hitroll;
					break;
				case RP_DAMROLL:
					num = shop->damroll - remort->damroll;
					break;
				case RP_HIT:
					num = shop->hit - remort->hit;
					break;
				case RP_MANA:
					num = shop->mana - remort->mana;
					break;
				case RP_MOVE:
					num = shop->move - remort->move;
					break;
				case RP_QP:
					num = shop->qp;
					break;
				case RP_TRAIN:
					num = shop->train;
					break;
				case RP_PRACTICE:
					num = shop->practice;
					break;
				case RP_RP:
					num = shop->rp;
					break;
				case RP_ETHOS:
					if (shop->ethos >= 0)
						snprintf(str, sizeof(str), "%s", flag_string(ethos_table, shop->ethos));
					break;
				case RP_ALIGN:
					if (shop->align >= 0)
						snprintf(str, sizeof(str), "%s", flag_string(align_names, shop->align));
					break;
				case RP_RACE:
					if (shop->race >= 0)
						snprintf(str, sizeof(str), "%s", race_name(shop->race));
					break;
				case RP_SEX:
					if (shop->sex >= 0)
						snprintf(str, sizeof(str), "%s", flag_string(sex_table, shop->sex));
					break;
				case RP_CLASS:
					if (shop->class >= 0)
						snprintf(str, sizeof(str), "%s", class_lookup(shop->class)->name);
					break;
				case RP_SKILL:
					for (num = 0; num < shop->skills.nused; num++) {
						pcskill_t *ps, *pso;
						skill_t *sk;
						
						if ((ps = VARR_GET(&shop->skills, num)) == NULL)
							continue;
						if ((sk = skill_lookup(ps->sn)) == NULL)
							continue;
						if ((pso = pc_own_skill_lookup(ch, ps->sn)) != NULL
						&& ps->maxpercent == pso->maxpercent
						&& ps->level == pso->level)
							continue;
						buf_printf(output, "| %-9s| %-25s [%3d%%][%3dl][%3dh]|\n", sk->spell ? "spell" : "skill",
							sk->name, ps->maxpercent, ps->level, ps->hard);
					}
					continue;
				default:
					continue;
			}
		}
		if (rp->flags == RP_FLAG_NUMBER) {
			if (num > 0)
				buf_printf(output, "| %-9s| %-44d|\n", rp->that, num);
			else
				continue;
		} else {
			if (str[0] != '\0')
				buf_printf(output, "| %-9s| %-44s|\n", rp->that, str);
			else
				continue;
		}
	}
	
	if (show_buy)
		num = remort->shop->points;
	else
		num = remort->points;
	buf_printf(output, "%s====[%5d rp]====%s\n", line, num, line);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void race_change_check(CHAR_DATA *ch, race_t *rfrom, race_t *rto);
void remort_done(CHAR_DATA *ch, bool accept)
{
/*
 *	If accept == FALSE, prepare for confirm remort ('done', when passwd);
 *	else done remort.
 */
	REMORT_DATA	*remort = ch->pcdata->remort;
	RSHOP_DATA	*shop = ch->pcdata->remort->shop;
	class_t *cl;
	race_t *r;
	int align;
	int sex;
	int ethos;

	cl = (shop->class < 0) ? CLASS(ch->class) : CLASS(shop->class);
	r = (shop->race < 0) ? RACE(ch->race) : RACE(shop->race);
	align = (shop->align < 0) ?
		(ch->alignment >= ALIGN_GOOD ? ANUM_GOOD : (ch->alignment <= ALIGN_EVIL ? ANUM_EVIL : ANUM_NEUTRAL)) :
		shop->align;
	sex = (shop->sex < 0) ? ch->sex : shop->sex;
	ethos = (shop->ethos < 0) ? ch->pcdata->ethos : shop->ethos;

	if (!accept)
	{	
		if (rclass_lookup(r, cl->name) == NULL)
		{
			char_puts("Restrict class for current race.\n", ch);
			return;
		}
		
		if (cl->restrict_sex >= 0 && cl->restrict_sex != sex) {
			char_puts("Restrict sex for current class.\n", ch);
			return;
		}
		
		if (
		(r->pcdata->allow_align && (
			(align == ANUM_GOOD && !IS_SET(r->pcdata->allow_align, RA_GOOD))
		  ||	(align == ANUM_NEUTRAL && !IS_SET(r->pcdata->allow_align, RA_NEUTRAL))
		  ||	(align == ANUM_EVIL && !IS_SET(r->pcdata->allow_align, RA_EVIL))
		)) || (cl->restrict_align && (
			(cl->restrict_align == RA_GOOD && align != ANUM_GOOD)
		  ||	(cl->restrict_align == RA_NEUTRAL && align != ANUM_NEUTRAL)
		  ||	(cl->restrict_align == RA_EVIL && align != ANUM_EVIL)
		))) {
			char_puts("Restrict align for current race or class.\n", ch);
			return;
		}
		
		if (cl->restrict_ethos > 0 && cl->restrict_ethos != ethos) {
			char_puts("Restrict ethos for current class.\n", ch);
			return;
		}

		if (ch->desc && ch->desc->client)
			echo_off_string(ch->desc->client);
		wiznet("$N is contemplating remortion.", ch, NULL, 0, 0, ch->level);
		write_to_buffer_muddy(ch->desc, "Enter your password for confirm remort:", 0);
		ch->desc->connected = CON_DELETE;
		return;
	} else
	{
		int i, irace;
		pcskill_t *ps_in, *ps_out;
		OBJ_DATA *obj, *obj_next;
		AFFECT_DATA *paf, *paf_next;
		
		die_follower(ch);
		remort->remorts++;
		remort->points = shop->rp + UMAX(30, START_RP - remort->remorts * 5);
		remort->hitroll = shop->hitroll;
		remort->damroll = shop->damroll;
		remort->hit = shop->hit;
		remort->mana = shop->mana;
		remort->move = shop->move;
		varr_free(&remort->skills);
		remort->skills.nsize = sizeof(pcskill_t);
		remort->skills.nstep = 2;
		for (i = 0; i < shop->skills.nused; i++) {
			ps_out = VARR_GET(&shop->skills, i);
			ps_in = varr_enew(&remort->skills);
			ps_in->sn = ps_out->sn;
			ps_in->percent = 1;
			ps_in->level = ps_out->level;
			ps_in->maxpercent = ps_out->maxpercent;
			ps_in->hard = UMIN(90, ps_out->hard + 1);
		}
		varr_qsort(&remort->skills, cmpint);
		ch->pcdata->questpoints += shop->qp;
		ch->pcdata->train += shop->train;
		ch->pcdata->practice += shop->practice;
		free_rshop(shop);
		ch->pcdata->remort->shop = NULL;

		/* Start generation new char */
		ch->class = cn_lookup(cl->name);

				switch(align) {
			case ANUM_GOOD:
				ch->alignment = ALIGN_GOOD;
				break;
			case ANUM_EVIL:
				ch->alignment = ALIGN_EVIL;
				break;
			default:
				ch->alignment = ALIGN_NEUTRAL;
		}
		ch->pcdata->true_sex = ch->sex = sex;
		ch->pcdata->ethos = ethos;
		ch->pcdata->last_level = 0;
		ch->gold = ch->silver =0;
		ch->pcdata->bank_s = ch->pcdata->bank_g = 0;
		for (i = 0; i < MAX_STATS; i++) {
			int max_stat = get_max_train(ch, i);
			
			if (ch->perm_stat[i] > max_stat) {
				ch->pcdata->train += ch->perm_stat[i] - max_stat;
				ch->perm_stat[i] = max_stat;
			}
		}
		ch->level = 1;
		ch->pcdata->plevels = 0;
		ch->pcdata->exp = base_exp(ch);
		/* Remove all objects */
		for (obj = ch->carrying; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			extract_obj(obj, 0);
		}
		/* Remove all affects (not remove some affects) */
		for (paf = ch->affected; paf; paf = paf_next) {
			paf_next = paf->next;
			affect_remove(ch, paf);
		}
		while ((paf = ch->affected)) {
			ch->affected    = paf->next;
			aff_free(paf);
		}

		/* Remove all skills */
		varr_free(&ch->pcdata->learned);
		ch->pcdata->learned.nsize = sizeof(pcskill_t);
		ch->pcdata->learned.nstep = 20;
		/* Reset char */
		ch->pcdata->perm_hit = 10;
		ch->max_hit = ch->pcdata->perm_hit;
		ch->pcdata->perm_mana = 100;
		ch->max_mana = ch->pcdata->perm_mana;
		ch->pcdata->perm_move = 100;
		ch->max_move = ch->pcdata->perm_move;
		ch->pcdata->death = 0;	// !!!
		ch->pcdata->condition[COND_THIRST] = 48;
		ch->pcdata->condition[COND_FULL] = 48; 
		ch->pcdata->condition[COND_HUNGER] = 48;
		ch->pcdata->condition[COND_BLOODLUST] = 48;
		ch->pcdata->condition[COND_DESIRE] = 48;
		ch->pcdata->condition[COND_DRUNK] = 0;
		
		ch->affected_by = ch->imm_flags = ch->res_flags
			= ch->vuln_flags = 0;
		irace = rn_lookup(r->name);
		race_change_check(ch, race_lookup(ch->pcdata->race),
				race_lookup(irace));
		ch->pcdata->race = ch->race = irace;

		reset_char(ch);
		update_skills(ch);
		
		if (!( IS_SET(ch->comm, COMM_NO_AUTOTITLE) ||
		   IS_SET(ch->pcdata->plr_flags, PLR_NOTITLE)))
				set_title(ch, title_lookup(ch));
		/* Check hometown */
		if (hometown_restrict(HOMETOWN(ch->pcdata->hometown), ch))
		{
			int i;
			int hmt = -1;
			char_printf(ch, "Your hometown '%s' restrict for current settings.\n", hometown_name(ch->pcdata->hometown));
			for (i = 0; i < hometowns.nused; i++) {
				hometown_t *h = VARR_GET(&hometowns, i);
				
				if (hometown_restrict(h, ch))
					continue;
				hmt = i;
			}
			if (hmt < 0) {
				bug("[remort_done] No found needed hometown.", 0);
				hmt = 0;
			}
			ch->pcdata->hometown = hmt;
			char_printf(ch, "Your new hometown is '%s'.\n", hometown_name(hmt));
		}
		log_printf("%s remorted in %s[%s].", ch->name, cl->name, r->name);
		wiznet("$N has begun new life [remort].", ch, NULL, 0, 0, ch->level);
		ch->desc->connected = CON_PLAYING;
		add_pch_tolist(ch);
		char_to_room(ch, get_room_index(ROOM_VNUM_REMORT));
		if(IS_SET(ch->pcdata->otherf, OTHERF_CONFIRM_DESC))
			REMOVE_BIT(ch->pcdata->otherf, OTHERF_CONFIRM_DESC);
	}	
}

void remort_loop(CHAR_DATA *ch, const char *argument)
{
/*
 *	This function called by nanny() [see in comm.c] if
 *	ch->desc->connected == CON_REMORT.
 */
	char arg [MAX_INPUT_LENGTH];
	
	argument = one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0' || !str_prefix(arg,"help")) {
		if (arg[0] == '\0' || argument[0] == '\0')
			do_help(ch, "remort commands");
		else
			do_help(ch, argument);
		return;
 	}
 	
 	if (!str_prefix(arg,"score") || !str_prefix(arg,"practice")) {
 		doprintf(interpret, ch, arg);
 		return;
 	}
 	
 	if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell")
 	|| !str_prefix(arg,"racetable") || !str_prefix(arg,"classtable")) {
 		doprintf(interpret, ch, "%s %s", arg, argument);
 		return;
 	}
 	
 	if (!str_prefix(arg,"done") || !str_prefix(arg,"accept")) {
 		remort_done(ch, FALSE);
 		return;
 	}

 	if (!str_prefix(arg,"add") || !str_prefix(arg,"buy")) {
 		do_remort_buy(ch, argument, FALSE);
 		return;
 	}
 	
 	if (!str_prefix(arg,"list") || !str_prefix(arg,"prices")) {
 		show_price_rp(ch);
 		return;
 	}
 	
 	if (!str_prefix(arg,"cancel") || !str_prefix(arg,"quit")
 	|| !str_prefix(arg,"return"))
 	{
		free_rshop(ch->pcdata->remort->shop);
		ch->pcdata->remort->shop = NULL;
 		ch->desc->connected = CON_PLAYING;
 		add_pch_tolist(ch);
 		char_to_room(ch, get_room_index(ROOM_VNUM_REMORT));
 		return;
 	}
 	
 	if (!str_prefix(arg,"ownlist") || !str_prefix(arg,"ownparam"))
 	{
 		show_remort_data(ch, FALSE);
 		return;
 	}

	if (!str_prefix(arg,"buylist") || !str_prefix(arg,"addlist"))
	{
		show_remort_data(ch, TRUE);
		return;
	}
	
	if (!str_prefix(arg,"value") || !str_prefix(arg,"testbuy"))
	{
		do_remort_buy(ch, argument, TRUE);
		return;
	}
	
 	remort_loop(ch, str_empty); 
}

void do_remort(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;
	if (!IS_HERO(ch)) {
		char_puts("You must become hero for remort!\n", ch);
		return;
	}
	
	if (!ch->in_room || ch->in_room != get_room_index(ROOM_VNUM_REMORT)) {
		char_puts("You must be in special remort room!\n", ch);
		return;
	}
	
	if (!IS_SET(ch->pcdata->otherf, OTHERF_CONFIRM_DESC)) {
		char_puts("For remortion your description must be confirmed by immortals.\n", ch);
		return;
	}

	char_puts("WELCOME to remort!\n", ch);
	do_help(ch, "remort commands");
	if (!ch->pcdata->remort) {
		ch->pcdata->remort = new_remort();
	}
	if (ch->pcdata->remort->shop) {
		free_rshop(ch->pcdata->remort->shop);
		ch->pcdata->remort->shop = NULL;
		log_printf("[BUG] %s already has remort->shop [do_remort].", ch->name);
	}
	ch->pcdata->remort->shop = new_rshop();
	copy_remort_data(ch->pcdata->remort->shop, ch->pcdata->remort);

	ch->desc->connected = CON_REMORT;
	char_from_room(ch);

	remove_pch_fromlist(ch);
	
	write_to_buffer_muddy(ch->desc, "[remort]> ",0);
}

