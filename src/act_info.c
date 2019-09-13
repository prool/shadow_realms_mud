/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
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
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>

#if !defined(WIN32)
#	include <unistd.h>
#endif
#include <ctype.h>

#include "merc.h"
#include "update.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"
#include "comm.h"
#include "lang.h"

#if (defined(SUNOS) || defined(SVR4) || defined(LINUX)) && defined(CRYPT)
#	include <crypt.h>
#endif

/* command procedures needed */
DECLARE_DO_FUN(do_exits		);
DECLARE_DO_FUN(do_look		);
DECLARE_DO_FUN(do_help		);
DECLARE_DO_FUN(do_affects	);
DECLARE_DO_FUN(do_murder	);
DECLARE_DO_FUN(do_say		);
DECLARE_DO_FUN(do_alist		);

static int show_order[] = {
	WEAR_LIGHT,
	WEAR_FINGER_L,
	WEAR_FINGER_R,
	WEAR_NECK,
	WEAR_SHIRT,
	WEAR_BODY,
	WEAR_HEAD,
	WEAR_LEGS,
	WEAR_FEET,
	WEAR_HANDS,
	WEAR_ARMS,
	WEAR_SHIELD,
	WEAR_ABOUT,
	WEAR_WAIST,
	WEAR_WRIST_L,
	WEAR_WRIST_R,
	WEAR_WIELD,
	WEAR_SECOND_WIELD,
	WEAR_HOLD,
	WEAR_FLOAT,
	WEAR_TATTOO,
	WEAR_CLANMARK,
	WEAR_BACK,
	WEAR_GLASSES,
	WEAR_EARRING_L,
	WEAR_EARRING_R,	
	-1
};

/* for do_count */
int max_on = 0;

/*
 * Local functions.
 */
char *	format_obj_to_char	(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort);

#define SL_CH_SHORT		(A)
#define SL_CH_SHOWNOT		(B)
#define SL_CH_ON_NEAR		(C)
#define SL_CH_UNDER_NEAR 	(D)
#define SL_CH_CHECK_HIDE	(E)	// check hidden objects
void	show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch, flag32_t flags,
				OBJ_DATA *near);
//void	show_list_to_char	(OBJ_DATA *list, CHAR_DATA *ch,
//				 bool fShort, bool fShowNothing);


void	show_char_to_char_0	(CHAR_DATA *victim, CHAR_DATA *ch);
void	show_char_to_char_1	(CHAR_DATA *victim, CHAR_DATA *ch);
void	show_char_to_char	(CHAR_DATA *list, CHAR_DATA *ch);
static void	printf_nothing_find(CHAR_DATA* ch);

char *obj_name(OBJ_DATA *obj, CHAR_DATA *ch)
{
	static char buf[MAX_STRING_LENGTH];
	const char *name;
	
	if ((	IS_AFFECTED(ch, AFF_BLIND)
		|| (IS_SET(obj->extra_flags, ITEM_INVIS) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
		|| IS_SET(obj->hidden_flags, OHIDE_HIDDEN | OHIDE_BURYED))
	&& (!IS_NPC(ch) && !IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT)))
	{
		if (obj->pIndexData->item_type == ITEM_POTION)
			name = GETMSG("some potion", GET_LANG(ch));
		else
			name = GETMSG("something", GET_LANG(ch));
		
		strnzcpy(buf, sizeof(buf), name);	
		return buf;
	}
	name = fix_short(mlstr_cval(obj->short_descr, ch));

	if (!IS_SET(ch->comm, COMM_NOENG)
		&& name != mlstr_mval(obj->short_descr))
	{
		char engname[MAX_STRING_LENGTH];
		one_argument(obj->name, engname, sizeof(engname));
		snprintf(buf, sizeof(buf), "{y(%s){x ", engname);
	} else
		buf[0] = '\0';

	strnzcat(buf, sizeof(buf), name);
	return buf;
}

char *long_obj_name(OBJ_DATA *obj, CHAR_DATA *ch)
{
	static char buf[MAX_STRING_LENGTH];
	const char *name;

	if ((	IS_AFFECTED(ch, AFF_BLIND)
		|| (IS_SET(obj->extra_flags, ITEM_INVIS) && !IS_AFFECTED(ch, AFF_DETECT_INVIS))
		|| IS_SET(obj->hidden_flags, OHIDE_HIDDEN | OHIDE_BURYED))
	&& (!IS_NPC(ch) && !IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT)))
	{
		if (IS_AFFECTED(ch, AFF_BLIND))
			name = GETMSG("Something lies here.", GET_LANG(ch));
		else if (IS_SET(obj->hidden_flags, OHIDE_BURYED))
			name = GETMSG("Something buried here.", GET_LANG(ch));
		else if (IS_SET(obj->hidden_flags, OHIDE_HIDDEN))
			name = GETMSG("Something hidden here.", GET_LANG(ch));
		else
			name = GETMSG("Something lies on the ground.", GET_LANG(ch));
		
		strnzcpy(buf, sizeof(buf), name);	
		return buf;
	}

	name = mlstr_cval(obj->description, ch);

	if (!IS_SET(ch->comm, COMM_NOENG)
		&& name != mlstr_mval(obj->description))
	{
		char engname[MAX_STRING_LENGTH];
		one_argument(obj->name, engname, sizeof(engname));
		snprintf(buf, sizeof(buf), "{y( %s ){x ", engname);
	} else
		buf[0] = '\0';

	strnzcat(buf, sizeof(buf), name);
	return buf;
}

char *char_name(CHAR_DATA *victim, CHAR_DATA *ch)
{
	static char buf[MAX_STRING_LENGTH];
	const char *name;
	/* May use format_short() in handler.c !!! */

	name = mlstr_cval(victim->long_descr, ch);

	if (!IS_SET(ch->comm, COMM_NOENG)
		&& name != mlstr_mval(victim->long_descr))
	{
		char engname[MAX_STRING_LENGTH];
		one_argument(victim->name, engname, sizeof(engname));
		snprintf(buf, sizeof(buf), "{w( %s ){x ", engname);
	} else
		buf[0] = '\0';

	strnzcat(buf, sizeof(buf), name);
	return buf;
}

static void printf_nothing_find(CHAR_DATA* ch)
{
	char_printf(ch, "%sNothing.\n",
		(IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) ?
		"     " : str_empty);
}

char *format_obj_to_char(OBJ_DATA *obj, CHAR_DATA *ch, bool fShort)
{
	static char buf[MAX_STRING_LENGTH];

	if ((fShort && mlstr_null(obj->short_descr))
	||  mlstr_null(obj->description))
		return str_empty;

	buf[0] = '\0';

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_OBJ_STAT(obj, ITEM_INVIS))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({yInvis{x) ", GET_LANG(ch)));
		if (IS_OBJ_STAT(obj, ITEM_DARK))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({DDark{x) ", GET_LANG(ch)));
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL)
		&&  IS_OBJ_STAT(obj, ITEM_EVIL))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({RRed Aura{x) ", GET_LANG(ch)));
		if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
		&&  IS_OBJ_STAT(obj, ITEM_BLESS))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({BBlue Aura{x) ", GET_LANG(ch)));
		if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
		&&  IS_OBJ_STAT(obj, ITEM_MAGIC))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({MMagical{x) ", ch->lang));
		if (IS_OBJ_STAT(obj, ITEM_GLOW))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({WGlowing{x) ", ch->lang));
		if (IS_OBJ_STAT(obj, ITEM_HUM))
			strnzcat(buf, sizeof(buf),
				 GETMSG("({YHumming{x) ", ch->lang));
	}
	else {
		static char flag_tS[] = "{x[{y.{D.{R.{B.{M.{W.{Y.{x] ";
		strnzcpy(buf, sizeof(buf), flag_tS);
		if (IS_OBJ_STAT(obj, ITEM_INVIS)	)   buf[5] = 'I';
		if (IS_OBJ_STAT(obj, ITEM_DARK)		)   buf[8] = 'D';
		if (IS_AFFECTED(ch, AFF_DETECT_EVIL)
		&& IS_OBJ_STAT(obj, ITEM_EVIL)		)   buf[11] = 'E';
		if (IS_AFFECTED(ch, AFF_DETECT_GOOD)
		&&  IS_OBJ_STAT(obj,ITEM_BLESS)		)   buf[14] = 'B';
		if (IS_AFFECTED(ch, AFF_DETECT_MAGIC)
		&& IS_OBJ_STAT(obj, ITEM_MAGIC)		)   buf[17] = 'M';
		if (IS_OBJ_STAT(obj, ITEM_GLOW)		)   buf[20] = 'G';
		if (IS_OBJ_STAT(obj, ITEM_HUM)		)   buf[23] = 'H';
		if (strcmp(buf, flag_tS) == 0)
			buf[0] = '\0';
	}
	
	if (IS_OBJ_STAT(obj, ITEM_QUEST)
	&& IS_PC(ch) && ch->pcdata->quest
	&& (ch->pcdata->quest->target1 == obj
	   || ch->pcdata->quest->target2 == obj))
		strnzcat(buf, sizeof(buf),
			GETMSG("{r[{RTARGET{r]{x ", ch->lang));

	if (fShort) {
		strnzcat(buf, sizeof(buf), obj_name(obj, ch));
	/*	strnzcat(buf, sizeof(buf),
	 *	format_short(obj->short_descr, obj->name, ch));
	 */
		if (obj->pIndexData->vnum > 5 /* not money, gold, etc */
		&&  (obj->condition < COND_EXCELLENT
		|| !IS_SET(ch->comm, COMM_NOVERBOSE))){
			strnzcat(buf, sizeof(buf), " [{g");
			strnzcat(buf, sizeof(buf), GETMSG(get_cond_alias(obj), ch->lang));
			strnzcat(buf, sizeof(buf), "{x]");
		}
	 
		return buf;
	}

	if (obj->in_room && (IS_WATER(obj->in_room)
			|| (obj->on && can_see_obj(ch, obj->on)))) {
/*		char* p;
 *		p = strchr(buf, '\0');
 *		strnzcat(buf, sizeof(buf),
 *			 format_short(obj->short_descr, obj->name, ch));
 *		p[0] = UPPER(p[0]);
 */
		strnzcat(buf, sizeof(buf), obj_name(obj, ch));
		
		if (IS_WATER(obj->in_room))
			switch(number_range(1, 3)) {
		case 1:
			strnzcat(buf, sizeof(buf),
				 " is floating gently on the water.");
			break;
		case 2:
			strnzcat(buf, sizeof(buf),
				 " is making it's way on the water.");
			break;
		case 3:
			strnzcat(buf, sizeof(buf),
				 " is getting wet by the water.");
			break;
		} 
		else
		{
			strnzcat(buf, sizeof(buf), " is lies on ");
			if (ch->lang)	// > 0
				strnzcat(buf, sizeof(buf), word_form(
					mlstr_cval(obj->on->short_descr, ch),
					5, ch->lang, RULES_CASE));
			else
				strnzcat(buf, sizeof(buf),
					mlstr_mval(obj->on->short_descr));
			strnzcat(buf, sizeof(buf), ".");
		}
	}
	else
		strnzcat(buf, sizeof(buf), long_obj_name(obj, ch));
	return buf;
}

/*
 * Show a list to a character.
 * Can coalesce duplicated items.
 */
void show_list_to_char(OBJ_DATA *list, CHAR_DATA *ch,
			flag32_t flags, OBJ_DATA *near)
{
	BUFFER *output;
	const char **prgpstrShow;
	int *prgnShow;
	char *pstrShow;
	OBJ_DATA *obj;
	int nShow;
	int iShow;
	int count;
	bool fCombine;
	flag32_t flags_cansee_obj;

	if (ch->desc == NULL)
		return;

	/*
	 * Alloc space for output lines.
	 */
	output = buf_new(-1);

	count = 0;
	for (obj = list; obj; obj = obj->next_content)
		count++;
	if (count > MAX_LIST_OBJ){
		char_puts("Warning! Is much list object.\n", ch);
		log_printf("Warning! %s looking more MAX_LIST_OBJ(%d): %d",
			ch->name, MAX_LIST_OBJ, count);
//		return;
	}
	prgpstrShow = malloc(count * sizeof(char *));
	prgnShow    = malloc(count * sizeof(int)  );
	if (prgpstrShow == NULL || prgnShow == NULL)
		crush_mud();	
	nShow	= 0;
	flags_cansee_obj = IS_SET(flags, SL_CH_CHECK_HIDE) ?
		FCSO_SEE_HIDE : FCSO_NONE;

	/*
	 * Format the list of objects.
	 */
	for (obj = list; obj; obj = obj->next_content)
	{
		if (near && IS_SET(flags, SL_CH_ON_NEAR | SL_CH_UNDER_NEAR))
		{
			if (IS_SET(flags, SL_CH_ON_NEAR | SL_CH_UNDER_NEAR)
				== (SL_CH_ON_NEAR | SL_CH_UNDER_NEAR))
			{
				if ((	!is_obj_on_near(obj, near)
					&& !is_obj_under_near(obj, near))
				|| can_see_obj_raw(ch, obj, near,
				  FCSO_ON_NEAR | FCSO_UNDER_NEAR | flags_cansee_obj) == FALSE) 
					continue;
			} else if (IS_SET(flags, SL_CH_ON_NEAR))
			{
				if (!is_obj_on_near(obj, near)
				|| can_see_obj_raw(ch, obj, near,
				  FCSO_ON_NEAR | flags_cansee_obj) == FALSE)
				 	continue;
			} else if (IS_SET(flags, SL_CH_UNDER_NEAR))
			{
				if (!is_obj_under_near(obj, near)
				|| can_see_obj_raw(ch, obj, near,
				  FCSO_UNDER_NEAR | flags_cansee_obj) == FALSE)
					continue;
			}
		} else if (!can_see_obj_raw(ch, obj, NULL, flags_cansee_obj))
			continue;
		
		if (IS_SET(obj->hidden_flags, OHIDE_HIDDEN)
		&& IS_SET(flags, SL_CH_CHECK_HIDE))
		{
			if ((number_range(1, 140) - 10) <
				get_skill(ch, gsn_search))
			{
				REMOVE_BIT(obj->hidden_flags, OHIDE_HIDDEN);

				act("$n has found $p.", ch, obj, NULL, TO_ROOM);
				act("You has found $p.", ch, obj, NULL, TO_CHAR);
				check_improve(ch, gsn_search, TRUE, 24);
			} else
				continue;
		}

		if (obj->wear_loc == WEAR_NONE)
		{
			pstrShow = format_obj_to_char(obj, ch,
					IS_SET(flags, SL_CH_SHORT));

			fCombine = FALSE;

			if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
				/*
				 * Look for duplicates, case sensitive.
				 * Matches tend to be near end so run loop
				 * backwords.
				 */
				for (iShow = nShow - 1; iShow >= 0; iShow--) {
					if (!strcmp(prgpstrShow[iShow],
						    pstrShow)) {
						prgnShow[iShow]++;
						fCombine = TRUE;
						break;
					}
				}
			}

			/*
			 * Couldn't combine, or didn't want to.
			 */
			if (!fCombine) {
				prgpstrShow [nShow] = str_dup(pstrShow);
				prgnShow    [nShow] = 1;
				nShow++;
			}
		}
	}

	/*
	 * Output the formatted list.
	 */
	for (iShow = 0; iShow < nShow; iShow++) {
		if (prgpstrShow[iShow][0] == '\0')
			continue;

		if (IS_NPC(ch) || IS_SET(ch->comm, COMM_COMBINE)) {
			if (prgnShow[iShow] != 1) 
				buf_printf(output, "(%2d) ", prgnShow[iShow]);
			else
				buf_add(output,"     ");
		}

		buf_add(output, prgpstrShow[iShow]);
		buf_add(output,"\n");
		free_string(prgpstrShow[iShow]);
	}

	if (IS_SET(flags, SL_CH_SHOWNOT) && nShow == 0) {
		printf_nothing_find(ch);
	}

	page_to_char(buf_string(output),ch);

	/*
	 * Clean up.
	 */
	buf_free(output);
	free(prgpstrShow);
	free(prgnShow);
}

#define flag_t_SET(pos, c, exp) (flag_tS[pos] = (exp) ? (flags = TRUE, c) : '.')

void show_char_to_char_0(CHAR_DATA *victim, CHAR_DATA *ch)
{
	const char *msg = str_empty;
	const void *arg = NULL;
	const void *arg3 = NULL;
	flag32_t flags = 0;

	if (is_affected(victim, gsn_doppelganger)
	&&  (IS_NPC(ch) || !IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT)))
		victim = victim->doppel;

	if (IS_NPC(victim)) {
		/* if (IS_PC(ch) && ch->pcdata->questmob > 0 */

		if (IS_PC(ch)
		&&  victim->hunter == ch)
			char_puts("{r[{RTARGET{r]{x ", ch);
	}
	else {
		if (IS_SET(victim->pcdata->plr_flags, PLR_WANTED))
			char_puts("({RWanted{x) ", ch);

		if (IS_SET(victim->comm, COMM_AFK))
			char_puts("{c[AFK]{x ", ch);
	}

	if (IS_SET(ch->comm, COMM_LONG)) {
		if (IS_AFFECTED(victim, AFF_INVIS))
			char_puts("({yInvis{x) ", ch);
		if (IS_AFFECTED(victim, AFF_HIDE)) 
			char_puts("({DHidden{x) ", ch);
		if (IS_AFFECTED(victim, AFF_CHARM)) 
			char_puts("({mCharmed{x) ", ch);
		if (IS_AFFECTED(victim, AFF_PASS_DOOR)) 
			char_puts("({cTranslucent{x) ", ch);
		if (IS_AFFECTED(victim, AFF_FAERIE_FIRE)) 
			char_puts("({MPink Aura{x) ", ch);
		if (IS_NPC(victim)
		&&  IS_SET(victim->pIndexData->act, ACT_UNDEAD)
		&&  IS_AFFECTED(ch, AFF_DETECT_UNDEAD))
			char_puts("({DUndead{x) ", ch);
		if (RIDDEN(victim))
			char_puts("({GRidden{x) ", ch);
		if (IS_AFFECTED(victim,AFF_IMP_INVIS))
			char_puts("({bImproved{x) ", ch);
		if (IS_EVIL(victim) && IS_AFFECTED(ch, AFF_DETECT_EVIL))
			char_puts("({RRed Aura{x) ", ch);
		if (IS_GOOD(victim) && IS_AFFECTED(ch, AFF_DETECT_GOOD))
			char_puts("({YGolden Aura{x) ", ch);
		if (IS_AFFECTED(victim, AFF_SANCTUARY))
			char_puts("({WWhite Aura{x) ", ch);
		if (IS_AFFECTED(victim, AFF_BLACK_SHROUD))
			char_puts("({DBlack Aura{x) ", ch);
		if (IS_AFFECTED(victim, AFF_FADE)) 
			char_puts("({yFade{x) ", ch);
		if (IS_AFFECTED(victim, AFF_CAMOUFLAGE)) 
			char_puts("({gCamf{x) ", ch);
	}
	else {
		static char flag_tS[] = "{x[{y.{D.{m.{c.{M.{D.{G.{b.{R.{Y.{W.{y.{g.{x] ";
		bool flags = FALSE;

		flag_t_SET( 5, 'I', IS_AFFECTED(victim, AFF_INVIS));
		flag_t_SET( 8, 'H', IS_AFFECTED(victim, AFF_HIDE));
		flag_t_SET(11, 'C', IS_AFFECTED(victim, AFF_CHARM));
		flag_t_SET(14, 'T', IS_AFFECTED(victim, AFF_PASS_DOOR));
		flag_t_SET(17, 'P', IS_AFFECTED(victim, AFF_FAERIE_FIRE));
		flag_t_SET(20, 'U', IS_NPC(victim) &&
				  IS_SET(victim->pIndexData->act, ACT_UNDEAD) &&
				  IS_AFFECTED(ch, AFF_DETECT_UNDEAD));
		flag_t_SET(23, 'R', RIDDEN(victim));
		flag_t_SET(26, 'I', IS_AFFECTED(victim, AFF_IMP_INVIS));
		flag_t_SET(29, 'E', IS_EVIL(victim) &&
				  IS_AFFECTED(ch, AFF_DETECT_EVIL));
		flag_t_SET(32, 'G', IS_GOOD(victim) &&
				  IS_AFFECTED(ch, AFF_DETECT_GOOD));
		flag_t_SET(35, 'S', IS_AFFECTED(victim, AFF_SANCTUARY));
		flag_t_SET(34, 'W', IS_AFFECTED(victim, AFF_SANCTUARY));

		if (IS_AFFECTED(victim, AFF_BLACK_SHROUD)) {
			flag_t_SET(35, 'B', TRUE);
			flag_t_SET(34, 'D', TRUE);
		}

		flag_t_SET(38, 'C', IS_AFFECTED(victim, AFF_CAMOUFLAGE));
		flag_t_SET(41, 'F', IS_AFFECTED(victim, AFF_FADE));

		if (flags)
			char_puts(flag_tS, ch);
	}
	
	if (!IS_NPC(victim)) {
		if (victim->pcdata->invis_level >= LEVEL_HERO)
			char_puts("[{WWizi{x] ", ch);
		if (victim->pcdata->incog_level >= LEVEL_HERO)
			char_puts("[{DIncog{x] ", ch);
	}

	if (IS_NPC(victim) && victim->position == victim->start_pos) {
		
		/* XT */
		char_puts(char_name(victim, ch), ch);
		return;
	}

	if (IS_IMMORTAL(victim))
		char_puts("{W", ch);
	else
		char_puts("{x", ch);

	switch (victim->position) {
	case POS_DEAD:
		msg = "$N {xis DEAD!!";
		break;
	
	case POS_MORTAL:
		msg = "$N {xis mortally wounded.";
		break;
	
	case POS_INCAP:
		msg = "$N {xis incapacitated.";
		break;
	
	case POS_STUNNED:
		msg = "$N {xis lying here stunned.";
		break;
	
	case POS_SLEEPING:
		if (victim->on == NULL) {
			msg = "$N {xis sleeping here.";
			break;
		}
	
		arg = victim->on;
		if (IS_SET(victim->on->value[2], SLEEP_AT))
			msg = "$N {xis sleeping at $p.";
		else if (IS_SET(victim->on->value[2], SLEEP_ON))
			msg = "$N {xis sleeping on $p.";
		else
			msg = "$N {xis sleeping in $p.";
		break;
	
	case POS_RESTING:
		if (victim->on == NULL) {
			msg = "$N {xis resting here.";
			break;
		}

		arg = victim->on;
		if (IS_SET(victim->on->value[2], REST_AT))
			msg = "$N {xis resting at $p.";
		else if (IS_SET(victim->on->value[2], REST_ON))
			msg = "$N {xis resting on $p.";
		else
			msg = "$N {xis resting in $p.";
		break;
	
	case POS_SITTING:
		if (victim->on == NULL) {
			msg = "$N {xis sitting here.";
			break;
		}
	
		arg = victim->on;
		if (IS_SET(victim->on->value[2], SIT_AT))
			msg = "$N {xis sitting at $p.";
		else if (IS_SET(victim->on->value[2], SIT_ON))
			msg = "$N {xis sitting on $p.";
		else
			msg = "$N {xis sitting in $p.";
		break;
	
	case POS_STANDING:
		if (victim->on == NULL) {
			if (!IS_NPC(victim)
			&&  !IS_SET(ch->comm, COMM_BRIEF))
				arg = victim->pcdata->title;
	
			if (MOUNTED(victim)) {
				arg3 = MOUNTED(victim);
				msg = "$N{x$t {xis here, riding $I.";
			}
			else
				msg = "$N{x$t {xis here.";
			break;
		}
	
		arg = victim->on;
		if (IS_SET(victim->on->value[2],STAND_AT))
			msg = "$N {xis standing at $p.";
		else if (IS_SET(victim->on->value[2],STAND_ON))
			msg = "$N {xis standing on $p.";
		else
			msg = "$N {xis standing in $p.";
		break;
	
	case POS_FIGHTING:
		if (victim->fighting == NULL) {
			arg = "thin air??";
			flags = ACT_TRANS;
			msg = "$N {xis here, fighting with $t.";
		}
		else if (victim->fighting == ch) {
			arg = "YOU!";
			flags = ACT_TRANS;
			msg = "$N {xis here, fighting with $t.";
		}
		else if (victim->in_room == victim->fighting->in_room) {
			arg = victim->fighting;
			msg = "$N {xis here, fighting with $i.";
		}
		else {
			arg = "someone who left??";
			flags = ACT_TRANS;
			msg = "$N {xis here, fighting with $t.";
		}
		break;
	}

	act_puts3(msg, ch, arg, victim, arg3,
		  TO_CHAR | ACT_FORMSH | flags, POS_DEAD);
}

char* wear_loc_names[] =
{
	"<used as light>     $t",
	"<worn on finger>    $t",
	"<worn on finger>    $t",
	"<worn around neck>  $t",
	"<adjoins to torso>  $t",
	"<worn on torso>     $t",
	"<worn on head>      $t",
	"<worn on legs>      $t",
	"<worn on feet>      $t",
	"<worn on hands>     $t",
	"<worn on arms>      $t",
	"<worn as shield>    $t",
	"<worn about body>   $t",
	"<worn about waist>  $t",
	"<worn about wrist>  $t",
	"<worn about wrist>  $t",
	"<wielded>           $t",
	"<held>              $t",
	"<floating nearby>   $t",
	"<scratched tattoo>  $t",
	"<dual wielded>      $t",
	"<clan mark>         $t",
	"<worn back>         $t",
	"<worn face>         $t",
	"<worn on left ear>  $t",
	"<worn on right ear> $t",
	"<stuck in>          $t",
};

void show_obj_to_char(CHAR_DATA *ch, OBJ_DATA *obj, flag32_t wear_loc)
{
	if (obj){
		bool can_see = can_see_obj(ch, obj);
		act(wear_loc_names[wear_loc], ch,
			can_see ? format_obj_to_char(obj, ch, TRUE) : "something",
			NULL, TO_CHAR | (can_see ? 0 : ACT_TRANS));
	}
	else
		act(wear_loc_names[wear_loc], ch,
			"[empty]",
			NULL, TO_CHAR);
}

void show_char_to_char_1(CHAR_DATA *victim, CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	int i;
	int percent;
	bool found;
	char *msg;
	const char *desc;
	CHAR_DATA *doppel = victim;
	CHAR_DATA *mirror = victim;
	char buf[MAX_STRING_LENGTH];

	if (is_affected(victim, gsn_doppelganger)) {
		if (IS_NPC(ch) || !IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT)) {
			doppel = victim->doppel;
			if (is_affected(victim, gsn_mirror))
				mirror = victim->doppel;
		}
	}

	if (can_see(victim, ch)) {
		if (ch == victim)
			act("$n looks at $mself.",
			    ch, NULL, NULL, TO_ROOM);
		else {
			act_puts("$n looks at you.",
				 ch, NULL, victim, TO_VICT, POS_RESTING);
			act("$n looks at $N.",
			    ch, NULL, victim, TO_NOTVICT);
		}
	}

	if (IS_NPC(doppel))
		desc = mlstr_cval(doppel->description, ch);
	else
		desc = mlstr_mval(doppel->description);

	if (!IS_NULLSTR(desc))
		char_printf(ch, "%s{x", desc);
	else
		act_puts("You see nothing special about $m.",
			 victim, NULL, ch, TO_VICT, POS_DEAD);

	if (MOUNTED(victim))
		act_puts("$N is riding $i.",
			 ch, MOUNTED(victim), victim, TO_CHAR, POS_DEAD);
	if (RIDDEN(victim))
		act_puts("$N is being ridden by $i.",
			 ch, RIDDEN(victim), victim, TO_CHAR, POS_DEAD);

	if (victim->max_hit > 0)
		percent = (100 * victim->hit) / victim->max_hit;
	else
		percent = -1;

	if (percent >= 100)
		msg = "{Cis in perfect health{x.";
	else if (percent >= 90)
		msg = "{bhas a few scratches{x.";
	else if (percent >= 75)
		msg = "{Bhas some small but disgusting cuts{x.";
	else if (percent >= 50)
		msg = "{Gis covered with bleeding wounds{x.";
	else if (percent >= 30)
		msg = "{Yis gushing blood{x.";
	else if (percent >= 15)
		msg = "{Mis writhing in agony{x.";
	else if (percent >= 0)
		msg = "{Ris convulsing on the ground{x.";
	else
		msg = "{Ris nearly dead{x.";

	/* vampire ... */
	if (percent < 90 && HAS_SKILL(ch, gsn_vampire))
		gain_condition(ch, COND_BLOODLUST, -1);

	if (!IS_IMMORTAL(doppel)) {
		char_printf(ch, "(%s) ", race_name(doppel->race));
//		if (!IS_NPC(doppel)) 
//			char_printf(ch, "(%s) ", class_name(doppel));
		char_printf(ch, "(%s) ", flag_string(sex_table, doppel->sex));
	}

	strnzcpy(buf, sizeof(buf), fix_short(PERS(victim, ch)));
	buf[0] = UPPER(buf[0]);
	char_printf(ch, "%s %s%s%s %s\n",
			get_stat_alias(victim, STAT_CHA),
			IS_IMMORTAL(victim) ? "{W" : str_empty,
			buf,
			IS_IMMORTAL(victim) ? "{x" : str_empty,
			GETMSG(msg, ch->lang));

	found = FALSE;
	for (i = 0; show_order[i] != -1; i++)
		if ((obj = get_eq_char(mirror, show_order[i]))
		&&  can_see_obj(ch, obj)) {
			if (get_eq_char(mirror, WEAR_ABOUT) && (
				    show_order[i] == WEAR_CLANMARK
				 || show_order[i] == WEAR_TATTOO))
						continue;
			if (!found) {
				char_puts("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}


			show_obj_to_char(ch, obj, show_order[i]);
		}

	for (obj = mirror->carrying; obj; obj = obj->next_content)
		if (obj->wear_loc == WEAR_STUCK_IN
		&&  can_see_obj(ch, obj)) {
			if (!found) {
				char_puts("\n", ch);
				act("$N is using:", ch, NULL, victim, TO_CHAR);
				found = TRUE;
			}

			show_obj_to_char(ch, obj, WEAR_STUCK_IN);
		}

	if (victim != ch
	&&  (!IS_IMMORTAL(victim) || IS_IMMORTAL(ch))
	&&  !IS_NPC(ch)
	&&  number_percent() < get_skill(ch, gsn_peek) / (get_eq_char(mirror, WEAR_ABOUT) ? (IS_IMMORTAL(ch) ? 1 : 2) : 1)) {
		char_puts("\nYou peek at the inventory:\n", ch);
		if (get_skill(ch, gsn_peek) > 75 && number_percent() > 50)
			char_printf(ch, "Victim has %d gold and %d silver.\n",
				(mirror->gold * number_range(85, 115)) / 100,
				(mirror->silver * number_range(75, 125)) / 100);
		check_improve(ch, gsn_peek, TRUE, 4);
		show_list_to_char(mirror->carrying, ch, SL_CH_SHORT | SL_CH_SHOWNOT, NULL);
	}
}

void show_char_to_char(CHAR_DATA *list, CHAR_DATA *ch)
{
	CHAR_DATA *rch;
	int life_count = 0;

	for (rch = list; rch; rch = rch->next_in_room) {
		if (rch == ch
		||  (IS_PC(rch) &&
		     !IS_TRUSTED(ch, rch->pcdata->incog_level) &&
		     ch->in_room != rch->in_room))
			continue;
			
		if (!IS_NPC(rch)
		&& !IS_TRUSTED(ch, rch->pcdata->invis_level)) {
			AREA_DATA *pArea;

			if (!IS_NPC(rch))
				continue;

			pArea = area_vnum_lookup(rch->pIndexData->vnum);
			if (pArea == NULL
			||  !IS_BUILDER(ch, pArea))
				continue;
		}

		if (can_see(ch, rch))
			show_char_to_char_0(rch, ch);
		else {
			if (room_is_dark(ch) && IS_AFFECTED(rch, AFF_INFRARED))
				char_puts("You see {rglowing red eyes{x watching YOU!\n", ch);
			life_count++;
		}
	}

	if (list && list->in_room == ch->in_room
	&&  life_count
	&&  IS_AFFECTED(ch, AFF_DETECT_LIFE))
		act_puts("You feel $j more life $qj{forms} in the room.",
			 ch, (const void*) life_count, NULL,
			 TO_CHAR, POS_DEAD);
}

void do_clear(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch))
		char_puts("\033[0;0H\033[2J", ch);
}

/* changes your scroll */
DO_FUN(do_scroll)
{
	char arg[MAX_INPUT_LENGTH];
	int lines;

	if (IS_NPC(ch))
		return;
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_printf(ch, "You currently display %d lines per "
				"page.\n", ch->pcdata->lines + 2);
		return;
	}

	if (!is_number(arg)) {
		char_puts("You must provide a number.\n",ch);
		return;
	}

	lines = atoi(arg);
	if (lines < SCROLL_MIN || lines > SCROLL_MAX) {
		char_printf(ch, "Valid scroll range is %d..%d.\n",
			    SCROLL_MIN, SCROLL_MAX);
		return;
	}

	char_printf(ch, "Scroll set to %d lines.\n", lines);
	ch->pcdata->lines = lines - 2;
}

void do_version(CHAR_DATA *ch, const char *argument)
{
	char_printf(ch, "Muddy compiled in %s.\n", __DATE__);
}

/* RT does socials */
void do_socials(CHAR_DATA *ch, const char *argument)
{
	do_alist(ch, "social");
}

/* RT Commands to replace news, motd, imotd, etc from ROM */
void do_motd(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "motd");
}

void do_classtable(CHAR_DATA *ch, const char *argument)
{
	class_table_show(ch);
}

void do_racetable(CHAR_DATA *ch, const char *argument)
{
	race_table_show(ch);
}

void do_imotd(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "imotd");
}

void do_rules(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "rules");
}

void do_wizlist(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "wizlist");
}

/* RT this following section holds all the auto commands from ROM, as well as
   replacements for config */
#define do_print_sw(ch, swname, sw) \
		char_printf(ch, "%-16s %s\n", swname, sw ? "ON" : "OFF");

void do_autolist(CHAR_DATA *ch, const char *argument)
{
	/* lists most player flags */
	if (IS_NPC(ch))
		return;

	char_puts("action         status\n",ch);
	char_puts("---------------------\n",ch);
	do_print_sw(ch, "autoassist", IS_SET(ch->pcdata->plr_flags, PLR_AUTOASSIST));
	do_print_sw(ch, "autoexit", IS_SET(ch->pcdata->plr_flags, PLR_AUTOEXIT));
	do_print_sw(ch, "autogold", IS_SET(ch->pcdata->plr_flags, PLR_AUTOGOLD));
	do_print_sw(ch, "autolook", IS_SET(ch->pcdata->plr_flags, PLR_AUTOLOOK));
	do_print_sw(ch, "autoloot", IS_SET(ch->pcdata->plr_flags, PLR_AUTOLOOT));
	do_print_sw(ch, "autosac", IS_SET(ch->pcdata->plr_flags, PLR_AUTOSAC));
	do_print_sw(ch, "autosplit", IS_SET(ch->pcdata->plr_flags, PLR_AUTOSPLIT));
	do_print_sw(ch, "autoscalp", IS_SET(ch->pcdata->plr_flags, PLR_AUTOSCALP));

	if (IS_SET(ch->pcdata->plr_flags, PLR_MENTALBLOCK))
		char_puts("You ability of mentalblock is on.\n", ch);
	else
		char_puts("You ability of mentalblock is off!\n",ch);

	if (IS_SET(ch->pcdata->plr_flags, PLR_NOFOLLOW))
		char_puts("You do not welcome followers.\n",ch);
	else
		char_puts("You accept followers.\n",ch);
}

void do_autoassist(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOASSIST);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOASSIST))
		char_puts("You will now assist when needed.\n",ch);
	else
		char_puts("Autoassist removed.\n",ch);
}

void do_autoexit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOEXIT);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOEXIT))
		char_puts("Exits will now be displayed.\n",ch);
	else 
		char_puts("Exits will no longer be displayed.\n",ch);
}

void do_autogold(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOGOLD);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOGOLD))
		char_puts("Automatic gold looting set.\n",ch);
	else 
		char_puts("Autogold removed.\n",ch);
}

DO_FUN(do_autolook)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOLOOK);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOLOOK))
		char_puts("Automatic corpse examination set.\n", ch);
	else
		char_puts("Autolooking removed.\n", ch);
}

void do_autoloot(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOLOOT);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOLOOT))
		char_puts("Automatic corpse looting set.\n", ch);
	else
		char_puts("Autolooting removed.\n", ch);
}

void do_autosac(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOSAC);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOSAC))
		char_puts("Automatic corpse sacrificing set.\n",ch);
	else
		char_puts("Autosacrificing removed.\n",ch);
}

void do_autoscalp(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}
	
	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOSCALP);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOSCALP))
		char_puts("Automatic corpse scalping set.\n",ch);
	else
		char_puts("Autoscalping removed.\n",ch);
}

void do_autosplit(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_AUTOSPLIT);
	if (IS_SET(ch->pcdata->plr_flags, PLR_AUTOSPLIT))
		char_puts("Automatic gold splitting set.\n",ch);
	else
		char_puts("Autosplitting removed.\n",ch);
}

void do_prompt(CHAR_DATA *ch, const char *argument)
{
	const char *prompt;

	if (IS_NPC(ch))
		return;
	if (argument[0] == '\0'
	||  !str_prefix(argument, "show")) {
		char_printf(ch, "Current prompt is '%s'.\n", ch->pcdata->prompt);
		return;
	}

	if (!str_cmp(argument, "all") || !str_cmp(argument, "default"))
		prompt = str_dup(DEFAULT_PROMPT);
	else
		prompt = str_printf("%s ", argument);

	free_string(ch->pcdata->prompt);
	ch->pcdata->prompt = prompt;
	char_printf(ch, "Prompt set to '%s'.\n", ch->pcdata->prompt);
}

void do_bprompt(CHAR_DATA *ch, const char *argument)
{
//	const char *prompt;

	if (IS_NPC(ch))
		return;
	if (argument[0] == '\0'
	||  !str_prefix(argument, "show")) {
		char_printf(ch, "Current bprompt is '%s'.\n", ch->pcdata->bprompt);
		return;
	}

	if ( !str_cmp(argument, "all") || !str_cmp(argument, "default") ){
		free_string(ch->pcdata->bprompt);
		ch->pcdata->bprompt = NULL;
	} else {
		if (ch->pcdata->bprompt != NULL)
			free_string(ch->pcdata->bprompt);
		ch->pcdata->bprompt = str_printf("%s ", argument);
	}
	
	char_printf(ch, "Battle prompt set to '%s'.\n", ch->pcdata->bprompt);
}

void do_nofollow(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_NOFOLLOW);
	if (IS_SET(ch->pcdata->plr_flags,PLR_NOFOLLOW)) {
		char_puts("You no longer accept followers.\n", ch);
		die_follower(ch);
	}
	else
		char_puts("You now accept followers.\n", ch);
}

void do_mentalblock(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;
	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_MENTALBLOCK);
	if (IS_SET(ch->pcdata->plr_flags,PLR_MENTALBLOCK))
		char_puts("You turn on your mental block.\n", ch);
	else 
		char_puts("You turn off your mental block.\n", ch);
}

void do_look_in_raw(CHAR_DATA* ch, const char *argument, bool check_hidden)
{
	OBJ_DATA *obj;

	if ((obj = get_obj_here(ch, argument)) == NULL) {
		char_puts("You don't see that here.\n", ch);
		return;
	}

	switch (obj->pIndexData->item_type) {
	default:
		char_puts("That is not a container.\n", ch);
		break;

	case ITEM_DRINK_CON:
		if (obj->value[1] <= 0) {
			char_puts("It is empty.\n", ch);
			break;
		}

		act_puts("It's $tfilled with a $T liquid.",
			 ch,
			 obj->value[1] < obj->value[0] / 4 ?
				"less than half-" :
			 obj->value[1] < 3 * obj->value[0] / 4 ?
			 	"about half-" :
			 	"more than half-",
			 liq_table[obj->value[2]].liq_color,
			 TO_CHAR | ACT_TRANS, POS_DEAD);
		break;

	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_SCALP_CORPSE_NPC:
	case ITEM_SCALP_CORPSE_PC:
		if (IS_SET(obj->value[1], CONT_CLOSED) 
		/*&& (!ch->clan ||
		clan_lookup(ch->clan)->altar_ptr != obj)*/) {
			char_puts("It is closed.\n", ch);
			break;
		}

		act_puts("$p holds:", ch, obj, NULL, TO_CHAR, POS_DEAD);
		show_list_to_char(obj->contains, ch,
			SL_CH_SHORT | SL_CH_SHOWNOT |
			(check_hidden ? SL_CH_CHECK_HIDE: 0), NULL);
		break;
	}
}

/*inline*/ void do_look_in(CHAR_DATA* ch, const char *argument) // inline delete by prool
{
	do_look_in_raw(ch, argument, FALSE);
}

#define FLN_NONE	(0)
#define FLN_ON		(A)
#define FLN_UNDER	(B)
#define FLN_CHECK_HIDE	(C)	// check hiddne objects

void do_look_near(CHAR_DATA* ch, const char *argument, flag32_t flags)
{
	OBJ_DATA *obj;
	flag32_t flags_showlist;

	if ((obj = get_obj_here(ch, argument)) == NULL
	|| (flags == FLN_NONE)) {
		char_puts("You don't see that here.\n", ch);
		return;
	}
	
	flags_showlist = SL_CH_SHOWNOT |
		(IS_SET(flags, FLN_CHECK_HIDE) ? SL_CH_CHECK_HIDE : 0);
	
	if (obj->in_room == NULL
	|| (IS_SET(flags, FLN_ON) && obj->under == NULL && !IS_SET(flags, FLN_UNDER))
	|| (IS_SET(flags, FLN_UNDER) && obj->on == NULL && (IS_SET(flags, FLN_ON) && obj->under) == FALSE)) {
		if (IS_SET(flags, FLN_ON))
		{
			act("On $p you see:", ch, obj, NULL, TO_CHAR);
			printf_nothing_find(ch);
		}
		if (IS_SET(flags, FLN_UNDER))
		{
			act("Under $p you see:", ch, obj, NULL, TO_CHAR);
			printf_nothing_find(ch);
		}
		return;
	}

	if (IS_SET(flags, FLN_ON))
	{
		act("On $p you see:", ch, obj, NULL, TO_CHAR);
		show_list_to_char(obj->in_room->contents, ch,
			SL_CH_ON_NEAR | flags_showlist, obj);
	}
	
	if (IS_SET(flags, FLN_UNDER))
	{
		act("Under $p you see:", ch, obj, NULL, TO_CHAR);
		show_list_to_char(obj->in_room->contents, ch,
			SL_CH_UNDER_NEAR | flags_showlist, obj);
	}
}

int check_exit(const char *argument);

void do_look(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	EXIT_DATA *pexit;
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	ED_DATA *ed;
	int door;
	int number,count;

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING) {
		char_puts("You can't see anything but stars!\n", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_puts("You can't see anything, you're sleeping!\n", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	number = number_argument(arg1, arg3, sizeof(arg3));
	count = 0;

	if (arg1[0] == '\0' || !str_cmp(arg1, "auto")) {

		/* 'look' or 'look auto' */

		if (!room_is_dark(ch) && check_blind_raw(ch)) {
			const char *name;
			const char *engname;

			name = mlstr_cval(ch->in_room->name, ch);
			engname = mlstr_mval(ch->in_room->name);
			char_printf(ch, "{W%s", name);
			if (ch->lang && name != engname)
				char_printf(ch, " (%s){x", engname);
			else
				char_puts("{x", ch);
		
			if (IS_IMMORTAL(ch)
			||  IS_BUILDER(ch, ch->in_room->area))
				char_printf(ch, " [Room %d]",ch->in_room->vnum);

			char_puts("\n", ch);

			if (arg1[0] == '\0'
			||  (!IS_NPC(ch) && !IS_SET(ch->comm, COMM_BRIEF)))
				char_printf(ch, "  %s",
					    mlstr_cval(ch->in_room->description, ch));

			if (!IS_NPC(ch) && IS_SET(ch->pcdata->plr_flags, PLR_AUTOEXIT)) {
				char_puts("\n", ch);
				do_exits(ch, "auto");
			}
		}
		else 
			char_puts("It is pitch black...\n", ch);

		show_list_to_char(ch->in_room->contents, ch, 0, NULL);
		show_char_to_char(ch->in_room->people, ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (!str_cmp(arg1, "i")
	||  !str_cmp(arg1, "in")) {
		/* 'look in' */
		if (arg2[0] == '\0') {
			char_puts("Look in what?\n", ch);
			return;
		}

		do_look_in(ch, arg2);
		return;
	}
	
	if (!str_cmp(arg1, "o")
	|| !str_cmp(arg1, "on")) {
		/* 'look on' */
		if (arg2[0] == '\0') {
			char_puts("Look on what?\n", ch);
			return;
		}

		do_look_near(ch, arg2, FLN_ON);
		return;
	}

	if (!str_cmp(arg1, "u")
	|| !str_cmp(arg1, "under")) {
		/* 'look under' */
		if (arg2[0] == '\0') {
			char_puts("Look under what?\n", ch);
			return;
		}

		do_look_near(ch, arg2, FLN_UNDER);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) != NULL) {
		show_char_to_char_1(victim, ch);

		/* Love potion */
		if (is_affected(ch, gsn_love_potion) && (victim != ch)) {
			AFFECT_DATA af;

			affect_strip(ch, gsn_love_potion);

			if (ch->master)
				stop_follower(ch);
			add_follower(ch, victim);
			ch->leader = victim;

			af.where = TO_AFFECTS;
			af.type = gsn_charm_person;
			af.level = ch->level;
			af.duration =  number_fuzzy(victim->level / 4);
			af.bitvector = AFF_CHARM;
			af.modifier = 0;
			af.location = 0;
			affect_to_char(ch, &af);

			act("Isn't $n just so nice?",
			    victim, NULL, ch, TO_VICT);
			act("$N looks at you with adoring eyes.",
			    victim, NULL, ch, TO_CHAR);
			act("$N looks at $n with adoring eyes.",
			    victim, NULL, ch, TO_NOTVICT);
		}

		return;
	}

	for (obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			/* player can see object */
			ed = ed_lookup(arg3, obj->ed);
			if (ed) {
				if (++count == number) {
					show_ed(ch, ed, TRUE);
					return;
				}
				else
					continue;
			}

			ed = ed_lookup(arg3, obj->pIndexData->ed);

			if (ed) {
				if (++count == number) {
					show_ed(ch, ed, TRUE);
					return;
				}
				else
					continue;
			}

			if (is_name(arg3, obj->name))
				if (++count == number) {
					char_puts("You see nothing special about it.\n", ch);
					return;
				}
		}
	}

	for (obj = ch->in_room->contents;
	     obj != NULL; obj = obj->next_content) {
		if (can_see_obj(ch, obj)) {
			ed = ed_lookup(arg3, obj->ed);
			if (ed != NULL)
				if (++count == number) {
					show_ed(ch, ed, TRUE);
					return;
				}

			ed = ed_lookup(arg3, obj->pIndexData->ed);
			if (ed != NULL)
				if (++count == number) {
					show_ed(ch, ed, TRUE);
					return;
				}

			if (is_name(arg3, obj->name))
				if (++count == number) {
					char_puts(format_descr(obj->description, ch),
						ch);
					char_puts("\n", ch);
					return;
				}
		}
	}

	ed = ed_lookup(arg3, ch->in_room->ed);
	if (ed != NULL) {
		if (++count == number) {
			show_ed(ch, ed, TRUE);
			return;
		}
	}

	if (count > 0 && count != number) {
		if (count == 1)
			act_puts("You only see one $t here.",
				 ch, arg3, NULL, TO_CHAR, POS_DEAD);
		else
			act_puts("You only see $j of those here.",
				 ch, (const void*) count, NULL,
				 TO_CHAR, POS_DEAD);
		return;
	}

	if ((door = check_exit(arg1)) < 0) {
		char_puts("You don't see that here.\n", ch);
		return;
	}

	/* 'look direction' */
	if ((pexit = ch->in_room->exit[door]) == NULL
	|| (IS_SET(pexit->exit_info, EX_BURYED)
		&& number_percent() > get_skill(ch, gsn_perception)))
	{
		char_puts("Nothing special there.\n", ch);
		return;
	}
	
	if (IS_SET(pexit->exit_info, EX_BURYED))
	{
		check_improve(ch, gsn_perception, TRUE, 16);
		char_puts("You see that in this direction rock is friable.\n", ch);
		return;
	}

	if (IS_SET(pexit->exit_info, EX_HIDDEN))
	{
		char_puts("After you attentively seeing in this direction you observe exit.\n", ch);
	}

	if (!IS_NULLSTR(mlstr_mval(pexit->description)))
		char_mlputs(pexit->description, ch);
	else
		char_puts("Nothing special there.\n", ch);

	if (pexit->keyword    != NULL
	&&  pexit->keyword[0] != '\0'
	&&  pexit->keyword[0] != ' ') {
		if (IS_SET(pexit->exit_info, EX_CLOSED)) {
			act_puts("The $d is closed.",
				 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);
		}
		else if (IS_SET(pexit->exit_info, EX_ISDOOR))
			act_puts("The $d is open.",
				 ch, NULL, pexit->keyword, TO_CHAR, POS_DEAD);
	}
}

void do_search(CHAR_DATA * ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];

	WAIT_STATE(ch, SKILL(gsn_search)->beats);

	act("$n searches something.", ch, NULL, NULL, TO_ROOM);

	if (argument[0] == '\0')
	{
		act("You search items in room.", ch, NULL, NULL, TO_CHAR);
		show_list_to_char(ch->in_room->contents, ch, SL_CH_CHECK_HIDE | SL_CH_SHOWNOT, NULL);
		return;
	}
	
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (!str_cmp(arg1, "o")
	|| !str_cmp(arg1, "on")) {
		if (arg2[0] == '\0') {
			char_puts("Search on what?\n", ch);
			return;
		}
		act("You search items on '$u'.", ch, arg2, NULL, TO_CHAR);
		do_look_near(ch, arg2, FLN_ON | FLN_CHECK_HIDE);
		return;
	}
	
	if (!str_cmp(arg1, "u")
	|| !str_cmp(arg1, "under")) {
		if (arg2[0] == '\0') {
			char_puts("Search under what?\n", ch);
		} else {
			act("You search items under '$u'.", ch, arg2, NULL, TO_CHAR);
			do_look_near(ch, arg2, FLN_UNDER | FLN_CHECK_HIDE);
		}
		return;
	}
	
	if (!str_cmp(arg1, "i")
	|| !str_cmp(arg1, "in")) {
		if (arg2[0] == '\0') {
			char_puts("Search in what?\n", ch);
		} else {
			act("You search items in '$u'", ch, arg2, NULL, TO_CHAR);
			do_look_in_raw(ch, arg2, TRUE);
		}
		return;
	}
	
	char_puts("Syntax: search [o[n]|u[nder]|i[n] <obj>]", ch);
}

void do_examine(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;

	one_argument(argument, arg, sizeof(arg));

	if (ch->desc == NULL)
		return;

	if (ch->position < POS_SLEEPING) {
		char_puts("You can't see anything but stars!\n", ch);
		return;
	}

	if (ch->position == POS_SLEEPING) {
		char_puts("You can't see anything, you're sleeping!\n", ch);
		return;
	}

	if (!check_blind(ch))
		return;

	if (arg[0] == '\0') {
		char_puts("Examine what?\n", ch);
		return;
	}

	do_look(ch, arg);

	if ((obj = get_obj_here(ch, arg)) == NULL)
		return;

	WAIT_STATE(ch, PULSE_VIOLENCE / 2);

	switch (obj->pIndexData->item_type) {
	case ITEM_MONEY: {
		const char *msg;

		if (obj->value[0] == 0) {
			if (obj->value[1] == 0)
				msg = "Odd...there's no coins in the pile.";
			else if (obj->value[1] == 1)
				msg = "Wow. One gold coin.";
			else
				msg = "There are $J $qJ{gold coins} in this pile.";
		}
		else if (obj->value[1] == 0) {
			if (obj->value[0] == 1)
				msg = "Wow. One silver coin.";
			else
				msg = "There are $j $qj{silver coins} in the pile.";
		}
		else {
			msg = "There are $J gold and $j $qj{silver coins} in the pile."; 
		}
		act_puts3(msg, ch,
			  (const void*) obj->value[0], NULL,
			  (const void*) obj->value[1],
			  TO_CHAR, POS_DEAD);
		break;
	}

	case ITEM_DRINK_CON:
	case ITEM_CONTAINER:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_SCALP_CORPSE_NPC:
	case ITEM_SCALP_CORPSE_PC:
		do_look_in(ch, argument);
		break;
	}
	
	if (IS_SET(ch->comm, COMM_NOT_OU_EXA) == FALSE
	&& obj->in_room)
	{
		do_look_near(ch, argument, FLN_ON | FLN_UNDER);
	}
}

/*
 * Thanks to Zrin for auto-exit part.
 */
void do_exits(CHAR_DATA *ch, const char *argument)
{
	EXIT_DATA *pexit;
	bool found;
	bool fAuto;
	int door;

	fAuto  = !str_cmp(argument, "auto");

	if (fAuto)
		char_puts("{C[Exits:", ch);
	else if (IS_IMMORTAL(ch) || IS_BUILDER(ch, ch->in_room->area))
		char_printf(ch, "Obvious exits from room %d:\n",
			    ch->in_room->vnum);
	else
		char_puts("Obvious exits:\n", ch);

	found = FALSE;
	for (door = 0; door < MAX_DIR; door++) {
		if ((pexit = ch->in_room->exit[door]) != NULL)
		{
			bool closed;
			int chance;
		
			if (!can_see_exit(ch, pexit))
				continue;

			if (IS_SET(pexit->exit_info, EX_HIDDEN))
			{
				
				if ((chance = get_skill(ch, gsn_perception))
				&& number_percent() < chance)
				{
					if (!fAuto)
						check_improve(ch, gsn_perception, TRUE, 12);
				} else
					continue;
			}

			found = TRUE;
			closed = IS_SET(pexit->exit_info, EX_CLOSED | EX_HIDDEN | EX_BURYED);

			if (fAuto)
				char_printf(ch, " %s%s", dir_name[door],
					    closed ? "*" : str_empty);
			else {
				char_printf(ch, "{C%-5s%s{x - %s",
					    capitalize(dir_name[door]),
					    closed ? "*" : str_empty,
					    room_dark(pexit->to_room.r) ?
					    GETMSG("Too dark to tell", ch->lang) :
					    mlstr_cval(pexit->to_room.r->name,
							ch));
				if (IS_IMMORTAL(ch)
				||  IS_BUILDER(ch, pexit->to_room.r->area))
					char_printf(ch, " (room %d)",
						    pexit->to_room.r->vnum);
				char_puts("\n", ch);
			}
		}
	}

	if (!found)
		char_puts(fAuto ? " none" : "None.\n", ch);

	if (fAuto)
		char_puts("]{x\n", ch);
}

void do_worth(CHAR_DATA *ch, const char *argument)
{
	char_printf(ch, "You have %d gold, %d silver", ch->gold, ch->silver);
	if (IS_NPC(ch))
		return;
	if (ch->level < LEVEL_HERO)
		char_printf(ch, ", and %d experience (%d exp to level)",
			    ch->pcdata->exp, exp_to_level(ch));
	char_puts(".\n", ch);

/*
	act_puts("You have killed $j $T",
		 ch, (const void*) ch->pcdata->has_killed,
		 IS_GOOD(ch) ? "non-goods" :
		 IS_EVIL(ch) ? "non-evils" : 
			       "non-neutrals",
		 TO_CHAR | ACT_NOLF | ACT_TRANS, POS_DEAD);
	act_puts(" and $j $T.",
		 ch, (const void*) ch->pcdata->anti_killed,
		 IS_GOOD(ch) ? "goods" :
		 IS_EVIL(ch) ? "evils" : 
			       "neutrals",
		 TO_CHAR | ACT_TRANS, POS_DEAD);
 */
	act_puts("You have killed $j goods.", ch, 
		(const void*)ch->pcdata->good_killed, NULL, 
		TO_CHAR | ACT_TRANS, POS_DEAD );
	act_puts("You have killed $j evils.", ch, 
		(const void*)ch->pcdata->evil_killed, NULL, 
		TO_CHAR | ACT_TRANS, POS_DEAD );
	act_puts("You have killed $j neutrals.", ch, 
		(const void*)ch->pcdata->neutral_killed, NULL,
		TO_CHAR | ACT_TRANS, POS_DEAD );
}

char *	const	day_name	[] =
{
	"the Moon", "the Bull", "Deception", "Thunder", "the Horror",
	"the Kingdom", "Rancour", "the Crypt", "the Tutor",
	"Tranquility", "the Great Gods", "Freedom"
};

char *	const	month_name	[] =
{
	"the Winter Wolf", "the Frost Giant",
	"the Spring", "Nature", "Futility", "the Dragon",
	"the Sun", "the Heat", "the Dark Shades",
	"the Long Shadows", "the Ancient Darkness", "the Great Evil"
};

void do_time(CHAR_DATA *ch, const char *argument)
{
	extern char str_boot_time[];
	char *suf;
	int day;

	day	= time_info.day + 1;

	     if (day > 4 && day <  20) suf = "th";
	else if (day % 10 ==  1      ) suf = "st";
	else if (day % 10 ==  2      ) suf = "nd";
	else if (day % 10 ==  3      ) suf = "rd";
	else			       suf = "th";

	char_printf(ch,
		    "It is %d o'clock %s, Day of %s, %d%s the Month of %s.\n",
		    (time_info.hour % 12 == 0) ? 12 : time_info.hour %12,
		    time_info.hour >= 12 ? "pm" : "am",
		    day_name[time_info.day % 12],
		    day, suf, month_name[time_info.month]);

	if (!IS_SET(ch->in_room->room_flags, ROOM_INDOORS) || IS_IMMORTAL(ch))
		act_puts("It's $T.", ch, NULL,
			(time_info.hour>=5 && time_info.hour<9) ?   "dawn"    :
			(time_info.hour>=9 && time_info.hour<12) ?  "morning" :
			(time_info.hour>=12 && time_info.hour<18) ? "mid-day" :
			(time_info.hour>=18 && time_info.hour<21) ? "evening" :
								    "night",
			TO_CHAR | ACT_TRANS, POS_DEAD);

	if (!IS_IMMORTAL(ch))
		return;

	char_printf(ch, "\nSR started up at %s\n"
			"The system time is %s.\n",
			str_boot_time, strtime(time(NULL)));
}

DO_FUN(do_date)
{
	char *suf;
	int day;

	day	= time_info.day + 1;

	     if (day > 4 && day <  20) suf = "th";
	else if (day % 10 ==  1      ) suf = "st";
	else if (day % 10 ==  2      ) suf = "nd";
	else if (day % 10 ==  3      ) suf = "rd";
	else			       suf = "th";

	char_printf(ch,
		"Day of %s, %d%s the Month of %s. Year of %d.\n",
			day_name[time_info.day % 12],
			day, suf, month_name[time_info.month],
			time_info.year);
		
	if (!IS_IMMORTAL(ch))
		return;
	char_printf(ch, "Real date: %s\n", strtime(time(NULL)));
}

void do_weather(CHAR_DATA *ch, const char *argument)
{
	static char * const sky_look[5] = {
		"cloudless",
		"cloudy",
		"rainy",
		"lit by flashes of lightning",
		"snowing"
	};

	if (!IS_OUTSIDE(ch)) {
		char_puts("You can't see the weather indoors.\n", ch);
		return;
	}

	char_printf(ch, "The sky is %s and %s.\n",
		    sky_look[weather_info.sky],
		    weather_info.change_t >= 0 ?
		    "a warm southerly breeze blows" :
		    "a cold northern gust blows");
	if (IS_IMMORTAL(ch)) {
		char_printf(ch, "Temperature is %d(%d)C and waterfull is %d%%.\n",
			weather_info.temperature / 10, weather_info.temperature,
			weather_info.waterfull);
		char_printf(ch, "change_t = %d and change_w = %d.\n",
			weather_info.change_t, weather_info.change_w);
	}
}

void do_help(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	output = buf_new(ch->lang);
	help_show(ch, output, argument);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_who_raw(CHAR_DATA* ch, CHAR_DATA *wch, BUFFER* output)
{
	//clan_t *clan;
	class_t *cl;
	race_t *r;

	if ((cl = class_lookup(wch->class)) == NULL
	||  (r = race_lookup(wch->race)) == NULL
	||  !r->pcdata)
		return;

	buf_add(output, "{x[");
	if ( (ch && (IS_IMMORTAL(ch) || ch == wch || (LEVEL(ch) - LEVEL(wch)) > 9) )
	&& wch->level < LEVEL_HERO )
		buf_printf(output, "%3d ", wch->level);
	else
		buf_add(output, "    ");

	if (wch->level >= LEVEL_HERO) {
		if (ch && IS_IMMORTAL(ch))
			buf_add(output, "  ");
		buf_add(output, "{G");
		switch (wch->level) {
		case HIGH:		buf_add(output, " {WHIGH "); break;
		case IMPLEMENTOR:	buf_add(output, "  IMP "); break;
		case CREATOR:		buf_add(output, "  CRE "); break;
		case SUPREME:		buf_add(output, "  SUP "); break;
		case DEITY:		buf_add(output, "  DEI "); break;
		case GOD:		buf_add(output, "  GOD "); break;
		case IMMORTAL:		buf_add(output, "  IMM "); break;
		case DEMI:		buf_add(output, "  DEM "); break;
		case ANGEL:		buf_add(output, "  ANG "); break;
		case AVATAR:		buf_add(output, "  AVA "); break;
		case LEGEND:		buf_add(output, "{YLEGEND"); break;
		case HERO:		buf_add(output, " {YHERO "); break;
		}
		buf_add(output, "{x");
		if (ch && IS_IMMORTAL(ch))
			buf_add(output, "  ");
	}
	else {
		buf_printf(output, "%5.5s ", r->pcdata->who_name);

		if (ch && IS_IMMORTAL(ch))
			buf_printf(output, " %3.3s", cl->who_name);
	}
	buf_add(output, "] ");

	// Display Other flags.

	buf_add(output, "{X(");
	if (IS_ATTACKER(wch))
		buf_add(output, "{RA");
	else
		buf_add(output, "{GP");
	if (ch && IS_TRUSTED(ch, LEVEL_LEGEND))
	{

		if (IS_SET(wch->pcdata->otherf, OTHERF_NOEXP))
			buf_add(output, "{YX");
		else
			buf_add(output, "{Y.");

		if (IS_SET(wch->pcdata->otherf, OTHERF_CONFIRM_DESC))
			buf_add(output, "{MD");
		else
			buf_add(output, "{M.");

		if (IS_SET(wch->pcdata->otherf, OTHERF_RENAME))
			buf_add(output, "{BR");
		else
			buf_add(output, "{B.");

		if (IS_SET(wch->pcdata->plr_flags, PLR_GHOST))
			buf_add(output, "{CG");
		else
			buf_add(output, "{C.");
	}

	buf_add(output, "{x) ");
	
		
/*
	if (IS_ATTACKER(wch))
		buf_add(output, "{X({RA{x) ");
	else
		buf_add(output, "{X({GP{x) ");
*/


	/*
	if (wch->clan
	&&  (clan = clan_lookup(wch->clan))
	&&  (!IS_SET(clan->flags, CLAN_HIDDEN) ||
	     (ch && (wch->clan == ch->clan || IS_IMMORTAL(ch)))))
		buf_printf(output, "[{c%s{x] ", clan->name);
	*/

	if (IS_SET(wch->comm, COMM_AFK))
		buf_add(output, "{c[AFK]{x ");

	if (wch->pcdata->invis_level > 0)
		buf_printf(output, "[{WWizin: %d{x] ", wch->pcdata->invis_level);
	if (wch->pcdata->incog_level >0 )
		buf_printf(output, "[{DIncog: %d{x] ", wch->pcdata->incog_level);

	if (ch && in_PK(ch, wch) && !IS_IMMORTAL(ch) && !IS_IMMORTAL(wch))
		buf_add(output, "{r[{RPK{r]{x ");

	if (IS_SET(wch->pcdata->plr_flags, PLR_WANTED))
		buf_add(output, "{R(WANTED){x ");

	if (IS_IMMORTAL(wch))
		buf_printf(output, "{W%s{x", 
			ch ? mlstr_cval(wch->short_descr, ch)
			: mlstr_mval(wch->short_descr));
	else
		buf_add(output, ch ? mlstr_cval(wch->short_descr, ch)
			: mlstr_mval(wch->short_descr));

	buf_add(output, wch->pcdata->title);

	buf_add(output, "\n");
}

#define WHO_F_IMM	(A)		/* imm only			*/
#define WHO_F_PK	(B)		/* PK only			*/
#define WHO_F_TATTOO	(C)		/* same tattoo only		*/
#define WHO_F_CLAN	(D)		/* clan only			*/
#define WHO_F_RCLAN	(E)		/* specified clans only		*/
#define WHO_F_RRACE	(F)		/* specified races only		*/
#define WHO_F_RCLASS	(G)		/* specified classes only	*/

DO_FUN(do_who)
{
	BUFFER *output;
	DESCRIPTOR_DATA_MUDDY *d;

	flag32_t flags = 0;
	flag32_t ralign = 0;
	flag32_t rethos = 0;

	int iLevelLower = 0;
	int iLevelUpper = MAX_LEVEL;

	int tattoo_vnum = 0;	/* who tattoo data */
	OBJ_DATA *obj;

	int nNumber;
	int nMatch = 0;
	int count = 0;
	int rvn;

	const char *clan_names = str_empty;
	const char *race_names = str_empty;
	const char *class_names = str_empty;
	char *p;

	/*
	 * Parse arguments.
	 */
	nNumber = 0;
	for (;;) {
		int i;
		char arg[MAX_INPUT_LENGTH];

		argument = one_argument(argument, arg, sizeof(arg));
		if (arg[0] == '\0')
			break;

		if (!str_prefix(arg, "immortals")) {
			SET_BIT(flags, WHO_F_IMM);
			continue;
		}

		if (!str_cmp(arg, "pk")) {
			SET_BIT(flags, WHO_F_PK);
			continue;
		}

		if (!str_cmp(arg, "tattoo")) {
			if ((obj = get_eq_char(ch, WEAR_TATTOO)) == NULL) {
				char_puts("You haven't got a tattoo yet!\n", ch);
				goto bail_out;
			}
			SET_BIT(flags, WHO_F_TATTOO);
			tattoo_vnum = obj->pIndexData->vnum;
			continue;
		}

		if (!str_cmp(arg, "clan")) {
			SET_BIT(flags, WHO_F_CLAN);
			continue;
		}

		/*
		if ((i = cln_lookup(arg)) > 0) {
			name_add(&clan_names, CLAN(i)->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RCLAN);
			continue;
		}
		*/

		if ((i = rn_lookup(arg)) > 0 && RACE(i)->pcdata) {
			name_add(&race_names, RACE(i)->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RRACE);
			continue;
		}

		if (!IS_IMMORTAL(ch))
			continue;

		if ((i = cn_lookup(arg)) >= 0) {
			name_add(&class_names, CLASS(i)->name, NULL, NULL);
			SET_BIT(flags, WHO_F_RCLASS);
			continue;
		}

		if ((p = strchr(arg, '-'))) {
			*p++ = '\0';
			if (arg[0]) {
				if ((i = flag_value(ethos_table, arg)))
					SET_BIT(rethos, i);
				else
					char_printf(ch, "%s: unknown ethos.\n", arg);
			}
			if (*p) {
				if ((i = flag_value(ralign_names, p)))
					SET_BIT(ralign, i);
				else
					char_printf(ch, "%s: unknown align.\n", p);
			}
			continue;
		}

		if (is_number(arg)) {
			switch (++nNumber) {
			case 1:
				iLevelLower = atoi(arg);
				break;
			case 2:
				iLevelUpper = atoi(arg);
				break;
			default:
				char_printf(ch,
					    "%s: explicit argument (skipped)\n",
					    arg);
				break;
			}
			continue;
		}
	}

	/*
	 * Now show matching chars.
	 */
	output = buf_new(ch->lang);
	for (d = descriptor_list_muddy; d; d = d->next)
	{
		CHAR_DATA *wch;

		//clan_t *clan;
		race_t *race;
		class_t *class;

		if (d->connected != CON_PLAYING)
			continue;
		count++;

		wch = d->original ? d->original : d->character;
		if (!wch || !can_see(ch, wch) || !wch->pcdata)
			continue;

		if (is_affected(wch, gsn_vampire)
		&&  !IS_IMMORTAL(ch) && ch != wch)
			continue;

		if (wch->level < iLevelLower || wch->level > iLevelUpper
		||  (IS_SET(flags, WHO_F_IMM) && wch->level < LEVEL_IMMORTAL)
		||  (IS_SET(flags, WHO_F_PK) && !in_PK(ch, wch))
		//||  (IS_SET(flags, WHO_F_CLAN) && !wch->clan)
		||  (ralign && ((RALIGN(wch) & ralign) == 0))
		||  (rethos && ((wch->pcdata->ethos & rethos) == 0)))
			continue;

		if (IS_SET(flags, WHO_F_TATTOO)) {
			if ((obj = get_eq_char(wch, WEAR_TATTOO)) == NULL
			||  tattoo_vnum != obj->pIndexData->vnum)
				continue;
		}

		/*
		if (IS_SET(flags, WHO_F_RCLAN)) {
			if (!wch->clan
			||  (clan = clan_lookup(wch->clan)) == NULL
			||  !is_name(clan->name, clan_names))
				continue;
		}
		*/

		if (IS_SET(flags, WHO_F_RRACE)) {
			if ((race = race_lookup(wch->race)) == NULL
			||  !is_name(race->name, race_names))
				continue;
		}

		if (IS_SET(flags, WHO_F_RCLASS)) {
			if ((class = class_lookup(wch->class)) == NULL
			||  !is_name(class->name, class_names))
				continue;
		}

		nMatch++;
		do_who_raw(ch, wch, output);
	}

	max_on = UMAX(count, max_on);
	buf_printf(output, "{x\nPlayers found: %d. Most so far today: %d.\n",
		   nMatch, max_on);

			/* Religion gods in online */
	if (!flags && ch)
	{
		CHAR_DATA *vch;
		
		nMatch = 0;
		for (vch = char_list; vch && IS_PC(vch); vch = vch->next)
			if ((rvn = vch->pcdata->religion_vn))
			{
				if (!nMatch++)
					buf_add(output, "You are filling in world next gods: ");
				buf_printf(output, " %s", RELIGION(rvn)->name);
			} 
		if (nMatch)
			buf_printf(output, "  {W[{x%d{W]{x\n", nMatch);
	}

	page_to_char(buf_string(output), ch);
	buf_free(output);

bail_out:
	free_string(clan_names);
	free_string(class_names);
	free_string(race_names);
}

/* whois command */
void do_whois(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	BUFFER *output = NULL;
	DESCRIPTOR_DATA_MUDDY *d;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("You must provide a name.\n", ch);
		return;
	}

	for (d = descriptor_list_muddy; d; d = d->next)
	{
		CHAR_DATA *wch;

		if (d->connected != CON_PLAYING || !can_see(ch,d->character))
				continue;

		if (d->connected != CON_PLAYING
		||  (is_affected(d->character, gsn_vampire) &&
		     !IS_IMMORTAL(ch) && (ch != d->character)))
			continue;

		wch = (d->original != NULL) ? d->original : d->character;

		if (!can_see(ch,wch))
			continue;

		if (!str_prefix(arg,wch->name)) {
			if (output == NULL)
				output = buf_new(-1);
			do_who_raw(ch, wch, output);
		}
	}

	if (output == NULL) {
		char_puts("No one of that name is playing.\n", ch);
		return;
	}

	buf_add(output, "{x");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_count(CHAR_DATA *ch, const char *argument)
{
	int count;
	DESCRIPTOR_DATA_MUDDY *d;

	count = 0;

	for (d = descriptor_list_muddy; d; d = d->next)
		if (d->connected == CON_PLAYING && can_see(ch, d->character))
			count++;

	max_on = UMAX(count,max_on);

	char_printf(ch, "There are %d characters on, ", count);
	if (max_on == count)
		char_puts("the most so far today", ch);
	else
		char_printf(ch, "the most on today was %d", max_on);
	char_puts(".\n", ch);
}

void do_inventory(CHAR_DATA *ch, const char *argument)
{
	char_puts("You are carrying:\n", ch);
	show_list_to_char(ch->carrying, ch, SL_CH_SHORT | SL_CH_SHOWNOT, NULL);
}

void do_equipment(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	int i;
	bool found;
	bool not_all = strcmp(argument, "all");

	char_puts("You are using:\n", ch);
	found = FALSE;
	for (i = 0; show_order[i] >= 0; i++) {
		if ((obj = get_eq_char(ch, show_order[i])) == NULL)
		{
			bool cont = FALSE;

			if (not_all)
				continue;
			switch(show_order[i]){
				case WEAR_SHIELD:
				case WEAR_HOLD:
						if (CAN_WEAR_SH(ch, get_eq_char(ch, WEAR_WIELD)))
							break;
						cont = TRUE;
						break;
				case WEAR_SECOND_WIELD:
						if (IS_EMPTY_SEC_HAND(ch, get_eq_char(ch, WEAR_WIELD)))
							break;
				case WEAR_CLANMARK:
				case WEAR_TATTOO:
				case WEAR_FLOAT:
				case WEAR_GLASSES:
				case WEAR_EARRING_L:
				case WEAR_EARRING_R:
								cont = TRUE;
								break;
			}
			if (cont)
				continue;
		}

		show_obj_to_char(ch, obj, show_order[i]);
		if (obj)
			found = TRUE;
	}

	for(obj = ch->carrying; obj != NULL; obj = obj->next_content) {
		if (obj->wear_loc != WEAR_STUCK_IN)
			continue;

		show_obj_to_char(ch, obj, WEAR_STUCK_IN);
		found = TRUE;
	}

	if (!found)
		char_puts("Nothing.\n", ch);
}

void do_compare(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	OBJ_DATA *obj1;
	OBJ_DATA *obj2;
	int value1;
	int value2;
	char *cmsg;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0') {
		char_puts("Compare what to what?\n", ch);
		return;
	}

	if ((obj1 = get_obj_carry(ch, arg1)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	}

	if (arg2[0] == '\0') {
		for (obj2 = ch->carrying;
		     obj2 != NULL; obj2 = obj2->next_content)
			if (obj2->wear_loc != WEAR_NONE
			&&  can_see_obj(ch,obj2)
			&&  obj1->pIndexData->item_type == obj2->pIndexData->item_type
			&&  (obj1->wear_flags & obj2->wear_flags & ~ITEM_TAKE))
				break;

		if (obj2 == NULL) {
			char_puts("You aren't wearing anything comparable.\n", ch);
			return;
		}
	}
	else if ((obj2 = get_obj_carry(ch,arg2)) == NULL) {
		char_puts("You do not have that item.\n", ch);
		return;
	}

	cmsg		= NULL;
	value1	= 0;
	value2	= 0;

	if (obj1 == obj2)
		cmsg = "You compare $p to itself.  It looks about the same.";
	else if (obj1->pIndexData->item_type != obj2->pIndexData->item_type)
		cmsg = "You can't compare $p and $P.";
	else {
		switch (obj1->pIndexData->item_type) {
		default:
			cmsg = "You can't compare $p and $P.";
			break;

		case ITEM_ARMOR:
			value1 = obj1->value[0]+obj1->value[1]+obj1->value[2];
			value2 = obj2->value[0]+obj2->value[1]+obj2->value[2];
			break;

		case ITEM_WEAPON:
			value1 = (1 + obj1->value[2]) * obj1->value[1];
			value2 = (1 + obj2->value[2]) * obj2->value[1];
			break;
		}
	}

	if (cmsg == NULL) {
		if (value1 == value2)
			cmsg = "$p and $P look about the same.";
		else if (value1  > value2)
			cmsg = "$p looks better than $P.";
		else
			cmsg = "$p looks worse than $P.";
	}

	act(cmsg, ch, obj1, obj2, TO_CHAR);
}

void do_credits(CHAR_DATA *ch, const char *argument)
{
	do_help(ch, "SHADOW REALMS");
}

typedef struct record {
	ROOM_INDEX_DATA *room;
	int direction;
} record;

#define MAX_N_O_V	50
#define MAX_RANGE	10

static CHAR_DATA *parse_room(ROOM_INDEX_DATA *pRoom, CHAR_DATA *ch, const char *name_x, CHAR_DATA **list_chars,
	int *nov, int *num_of_vict, bool ugly)
{
	CHAR_DATA *victim;
	CHAR_DATA *vch;
	int i;
	
	for (victim = pRoom->people; victim; victim = victim->next_in_room) {
		if (!can_see(ch, victim))
			continue;
		
		if (ugly &&  is_affected(victim, gsn_vampire))
		{
			goto find_point;
		}
		
		vch = (is_affected(victim, gsn_doppelganger) &&
			(IS_NPC(ch) || !IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT))) ?
			victim->doppel : victim;
		if (name_x[0] && !is_name(name_x, vch->name))
			continue;
		
		find_point:
					
		for (i = 0;  i < *nov; i++)
		{
			if (list_chars[i] == victim)
				break;	// next victim in room
		}
		
		if (i < *nov)
			continue;	// next victim in room
		
		if (--(*num_of_vict) > 0)
		{
			if (*nov >= MAX_N_O_V)
				return NULL;
			list_chars[(*nov)++] = victim;
		} else
			return victim;
	}
	return NULL;
}

/*
 *	This function search num_of_vict.victim by 'CHAR_DATA *victim' or 'const char *name' if victim == NULL.
 */
CHAR_DATA *is_near_char(CHAR_DATA *ch, CHAR_DATA *victim, int range, const char *name, int num_of_vict)
{
	record mas[MAX_RANGE + 1];
	int r = 0;
	EXIT_DATA *texit;
	bool has_name;
	uint number;
	char name_x[MAX_STRING_LENGTH];
	CHAR_DATA *list_chars[MAX_N_O_V];
	int nov = 0;
	bool ugly = FALSE;
	
	has_name = !victim;
	if (!ch->in_room || !ch->in_room->area || (!has_name && (!victim->in_room || !victim->in_room->area))) {
		log_printf("BUG [is_near_char]! Char not in area.", 0);
		return NULL;
	}
	
	if (range > MAX_RANGE) {
		log_printf("Warning [is_near_char]!!! Range > MAX_RANGE");
		range = MAX_RANGE;
	}
	
	if (has_name) {
		if (IS_NULLSTR(name))
			return NULL;
		number = number_argument(name, name_x, sizeof(name_x));
		if (number == 0)
			return NULL;
		if (number > 1)
			num_of_vict = number;

		if (num_of_vict > MAX_N_O_V)
			return NULL;
		ugly = !str_cmp(name, "ugly");
		if ((victim = parse_room(ch->in_room, ch, name_x, list_chars, &nov, &num_of_vict, ugly)) != NULL)
			return victim;
		if (nov >= MAX_N_O_V)
			return NULL;
	} else if (ch->in_room == victim->in_room)
		return victim;
	
	mas[0].room = ch->in_room;
	mas[0].direction = -1;
	
	for (;;)
	{
		if (++mas[r].direction >= MAX_DIR || r >= range) {
			if (--r < 0)
				return NULL;
			continue;
		}
		if ((texit = mas[r].room->exit[mas[r].direction]) == NULL)
		{
			continue;
		} else {
			
			if (++r <= range)
				mas[r].direction = -1;
			if ((mas[r].room = texit->to_room.r) == NULL || r > range) {
				r--;
				continue;
			}
			
			if (has_name) {
				if ((victim = parse_room(mas[r].room, ch, name_x, list_chars, &nov, &num_of_vict, ugly)) != NULL)
					return victim;
				if (nov >= MAX_N_O_V)
					return NULL;
			} else if (victim->in_room == mas[r].room)
				return victim;
		}
	}
	
	return NULL;
}

static int char_list_printf(CHAR_DATA *ch, CHAR_DATA *victim, CHAR_DATA **list_chars,
	int n, const char *name_x, BUFFER *output)
{
	int i;
	int find;
	ROOM_INDEX_DATA *room;
	
	if ((find = n + (victim ? 1 : 0)) == 0)
	{
		buf_printf(output, GETMSG("You didn't find any '%s'.\n", ch->lang), name_x);
		return 0;
	}
	buf_printf(output, GETMSG("You find %d victim(s) with prefix '%s'.\n", ch->lang), find, name_x);
	for (i = 0; i < n; i++)
		if (list_chars[i] && list_chars[i]->in_room)
			buf_printf(output, "%-25s %s({G%s{x)\n",
				PERS2(list_chars[i], ch, ACT_FORMSH),
				mlstr_mval(list_chars[i]->in_room->name),
				((room = list_chars[i]->in_room) && can_see_room(ch, room) &&
					room->area) ? room->area->name : "???");
	if (victim && victim->in_room)
		buf_printf(output, "%-25s %s({G%s{x)\n",
			PERS2(victim, ch, ACT_FORMSH),
			mlstr_mval(victim->in_room->name),
			((room = victim->in_room) && can_see_room(ch, room) &&
				room->area) ? room->area->name : "???");
	return find;
}

int is_near_char_printf(CHAR_DATA *ch, int range, int num_of_vict, const char *name, BUFFER *output)
{
	CHAR_DATA *list_chars[MAX_N_O_V];
	CHAR_DATA *victim;
	EXIT_DATA *texit;
	char name_x[MAX_STRING_LENGTH];
	int nov = 0;
	int r = 0;
	int number;
	record mas[MAX_RANGE + 1];
	bool ugly = FALSE;

	if (range > MAX_RANGE) {
		log_printf("Warning [is_near_char]!!! Range > MAX_RANGE");
		range = MAX_RANGE;
	}
	
	if (!ch->in_room || !ch->in_room->area) {
		log_printf("BUG [is_near_char_printf]! Char not in area.", 0);
		return 0;
	}
	
	if (IS_NULLSTR(name))
		return 0;
	
	number = number_argument(name, name_x, sizeof(name_x));
	if (number == 0)
		return 0;
	if (number > 1)
		num_of_vict = number;
	if (num_of_vict > MAX_N_O_V)
		return 0;
	ugly = !str_cmp(name, "ugly");

	if ((victim = parse_room(ch->in_room, ch, name_x, list_chars, &nov, &num_of_vict, ugly)) != NULL
	  || nov >= MAX_N_O_V)
		return char_list_printf(ch, victim, list_chars, nov, name_x, output);
	
	mas[0].room = ch->in_room;
	mas[0].direction = -1;
	
	for (;;)
	{
		if (++mas[r].direction >= MAX_DIR || r >= range) {
			if (--r < 0)
				return char_list_printf(ch, NULL, list_chars, nov, name_x, output);
			continue;
		}
		if ((texit = mas[r].room->exit[mas[r].direction]) == NULL)
		{
			continue;
		} else {
			
			if (++r <= range)
				mas[r].direction = -1;
			if ((mas[r].room = texit->to_room.r) == NULL || r > range) {
				r--;
				continue;
			}
			
			if ((victim = parse_room(mas[r].room, ch, name_x, list_chars, &nov, &num_of_vict, ugly)) != NULL
			  || nov >= MAX_N_O_V)
				return char_list_printf(ch, victim, list_chars, nov, name_x, output);
		}
	}
	return char_list_printf(ch, NULL, list_chars, nov, name_x, output);
}


void do_where(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA_MUDDY *d;
	bool fPKonly = FALSE;

	if (!ch->in_room || !ch->in_room->area){
		log_printf("BUG [do_where]! Char not in area.", 0);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (!check_blind(ch))
		return;

	if (room_is_dark(ch)) {
		char_puts("It's too dark to see.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_NOWHERE))
	{
		char_puts("You can not understand, where you are.\n", ch);
		return;
	}

	if (!str_cmp(arg,"pk"))
		fPKonly = TRUE;

	WAIT_STATE(ch, PULSE_VIOLENCE);
	if (arg[0] == '\0' || fPKonly) {
		bool found = FALSE;

		char_printf(ch, "Players in your area ({G%s{x):\n", ch->in_room->area->name);
		for (d = descriptor_list_muddy; d; d = d->next)
		{
			if (d->connected == CON_PLAYING
			&&  (victim = d->character) != NULL
			&&  !IS_NPC(victim)
			&&  (!fPKonly || in_PK(ch, victim))
			&&  victim->in_room != NULL
			&&  victim->in_room->area == ch->in_room->area
			&&  can_see(ch, victim)) {
				CHAR_DATA *doppel;
				found = TRUE;

				if (is_affected(victim, gsn_doppelganger)
				&&  (IS_NPC(ch) || !IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT)))
					doppel = victim->doppel;
				else
					doppel = victim;

				char_printf(ch, "%s%-28s %s\n",
					(in_PK(ch, doppel) &&
					!IS_IMMORTAL(ch)) ?
					"{r[{RPK{r]{x " : "     ",
					PERS(victim, ch),
					is_near_char(ch, victim, 4, str_empty, 0) ? mlstr_mval(victim->in_room->name) : "(far)");
			}
		}
		if (!found)
			char_puts("None.\n", ch);
	}
	else {
		BUFFER *output;
		
		output = buf_new(-1);
		is_near_char_printf(ch, 5 + LEVEL(ch) / 30, 20, arg, output);
		page_to_char(buf_string(output), ch);
		buf_free(output);
	}
}

void do_consider(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	char *cmsg;
//	char *align;
	int diff;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Consider killing whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Suicide is against your way.\n", ch);
		return;
	}

	if (!in_PK(ch, victim)) {
		char_puts("Don't even think about it.\n", ch);
		return;
	}

	diff = victim->level - ch->level;

	     if (diff <= -10) cmsg = "You can kill $N naked and weaponless.";
	else if (diff <=  -5) cmsg = "$N is no match for you.";
	else if (diff <=  -2) cmsg = "$N looks like an easy kill.";
	else if (diff <=   2) cmsg = "The perfect match!";
	else if (diff <=   7) cmsg = "$N says '{GDo you feel lucky, punk?{x'.";
	else if (diff <=   12) cmsg = "$N laughs at you mercilessly.";
	else		      cmsg = "Death will thank you for your gift.";

	act(cmsg, ch, NULL, victim, TO_CHAR);

/*	if (IS_EVIL(ch) && IS_EVIL(victim))
		align = "$N grins evilly with you.";
	else if (IS_GOOD(victim) && IS_GOOD(ch))
		align = "$N greets you warmly.";
	else if (IS_GOOD(victim) && IS_EVIL(ch))
		align = "$N smiles at you, hoping you will turn from your evil path.";
	else if (IS_EVIL(victim) && IS_GOOD(ch))
		align = "$N grins evilly at you.";
	else if (IS_NEUTRAL(ch) && IS_EVIL(victim))
		align = "$N grins evilly.";
	else if (IS_NEUTRAL(ch) && IS_GOOD(victim))
		align = "$N smiles happily.";
	else if (IS_NEUTRAL(ch) && IS_NEUTRAL(victim))
		align = "$N looks just as disinterested as you.";
	else
		align = "$N looks very disinterested.";
	act(align, ch, NULL, victim, TO_CHAR);
 */
}

void set_title(CHAR_DATA *ch, const char *title)
{
	char buf[MAX_TITLE_LENGTH];
	static char nospace[] = "-.,!?':";

	buf[0] = '\0';

	if (title) {
		if (strchr(nospace, *cstrfirst(title)) == NULL) {
			buf[0] = ' ';
			buf[1] = '\0';
		}

		strnzcat(buf, sizeof(buf), title);
	}

	free_string(ch->pcdata->title);
	ch->pcdata->title = str_dup(buf);
}

void do_title(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	if (IS_SET(ch->pcdata->plr_flags, PLR_NOTITLE)) {
		char_puts("You can't change your title.\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		char_puts("Change your title to what?\n", ch);
		return;
	}

	if (strstr(argument, "{\\")) {
		char_puts("Illegal characters in title.\n", ch);
		return;
	}
		
	set_title(ch, argument);
	char_puts("Ok.\n", ch);
}

void do_description(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_STRING_LENGTH];

	if (IS_NPC(ch)) {
		char_puts("Huh?\n", ch);
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (str_cmp(arg, "edit") == 0) {
		string_append(ch, mlstr_convert(&ch->description, -1));
		if (IS_SET(ch->pcdata->otherf, OTHERF_CONFIRM_DESC)) {
			REMOVE_BIT(ch->pcdata->otherf, OTHERF_CONFIRM_DESC);
			char_puts("{RYour descriptions has been denied."
					"{x\n", ch);
		}
		return;
	}

	char_printf(ch, "Your description is:\n"
			 "%s\n"
			 "Use 'desc edit' to edit your description.\n",
		    mlstr_mval(ch->description));
}

void do_report(CHAR_DATA *ch, const char *argument)
{
	doprintf(do_say, ch, "I have %d/%d hp %d/%d mana %d/%d mv.",
		 ch->hit, ch->max_hit,
		 ch->mana, ch->max_mana,
		 ch->move, ch->max_move);
}

/*
 * 'Wimpy' originally by Dionysos.
 */
void do_wimpy(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	int wimpy;
	class_t *cl;

	if (IS_NPC(ch))
		return;
	if ((cl = class_lookup(ch->class))
	&&  !CAN_FLEE(ch, cl)) {
		char_printf(ch, "You don't deal with wimpies, "
				"or such feary things.\n");
		if (ch->pcdata->wimpy)
			ch->pcdata->wimpy = 0;
		return;
	}

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0')
		wimpy = ch->max_hit / 5;
	else
		wimpy = atoi(arg);

	if (wimpy < 0) {
		char_puts("Your courage exceeds your wisdom.\n", ch);
		return;
	}

	if (wimpy > ch->max_hit/2) {
		char_puts("Such cowardice ill becomes you.\n", ch);
		return;
	}

	ch->pcdata->wimpy	= wimpy;

	char_printf(ch, "Wimpy set to %d hit points.\n", wimpy);
	return;
}

bool check_password(CHAR_DATA *ch, const char *pswd, bool show_txt_wait)
{
#if !defined(CRYPT)
	if ( (ch->pcdata->pwd[0] != 0) && strcmp(pswd, ch->pcdata->pwd)) {
		if (show_txt_wait)
		{
			WAIT_STATE(ch, 10 * PULSE_PER_SECOND);
			char_puts("Wrong password.  Wait 10 seconds.\n", ch);
		}
		return FALSE;
	}
#else
	if (strcmp(crypt(arg1, ch->pcdata->pwd), ch->pcdata->pwd) && ch->pcdata->pwd[0] != 0) {
		if (show_txt_wait)
			WAIT_STATE(ch, 10 * PULSE_PER_SECOND);
			char_puts("Wrong password.  Wait 10 seconds.\n", ch);
		}
		return FALSE;
	}
#endif
	return TRUE;
}

void do_password(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char *pwdnew;

	if (IS_NPC(ch))
		return;

	argument = first_arg(argument, arg1, sizeof(arg1), FALSE);
	argument = first_arg(argument, arg2, sizeof(arg2), FALSE);

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Syntax: config password <old> <new>\n", ch);
		return;
	}

	if (!check_password(ch, arg1, TRUE))
		return;

	if (strlen(arg2) < 5) {
		char_puts("New password must be at least "
			     "five characters long.\n", ch);
		return;
	}

	/*
	 * No tilde allowed because of player file format.
	 */
#if !defined(CRYPT)
	pwdnew = arg2;
#else
	pwdnew = crypt(arg2, ch->name);
#endif
	if (strchr(pwdnew, '~') != NULL) {
		char_puts("New password not acceptable, "
			     "try again.\n", ch);
		return;
	}

	free_string(ch->pcdata->pwd);
	ch->pcdata->pwd = str_dup(pwdnew);
	save_char_obj(ch, FALSE);
	char_puts("Ok.\n", ch);
}

void scan_list(ROOM_INDEX_DATA *scan_room, CHAR_DATA *ch, 
		int depth, int door)
{
	CHAR_DATA *rch;

	if (scan_room == NULL) 
		return;

	for (rch = scan_room->people; rch; rch = rch->next_in_room) {
		if (rch == ch || !can_see(ch, rch))
			continue;
		char_printf(ch, "	%s.\n",
				PERS2(rch, ch, ACT_FORMSH));
//				format_short(rch->short_descr, rch->name, ch));
	}
}

void do_scan2(CHAR_DATA *ch, const char *argument)
{
	EXIT_DATA *pExit;
	int door;
	int chance;

	act("$n looks all around.", ch, NULL, NULL, TO_ROOM);
	if (!check_blind(ch))
		return;

	char_puts("Looking around you see:\n", ch);

	char_puts("{Chere{x:\n", ch);
	scan_list(ch->in_room, ch, 0, -1);
	for (door = 0; door < 6; door++) {
		pExit = ch->in_room->exit[door];

		if (!can_see_exit(ch, pExit)
		|| IS_SET(pExit->exit_info, EX_BURYED))
			continue;

		if (IS_SET(pExit->exit_info, EX_HIDDEN))
		{
			if ((chance = get_skill(ch, gsn_perception))
			&& number_percent() < chance)
				check_improve(ch, gsn_perception, TRUE, 12);
			else
				continue;
		}
		
		if (IS_SET(pExit->exit_info, EX_CLOSED))
		{
			act_puts("{C$t{x:    the $d is closed.", ch,
				dir_name[door], pExit->keyword,
				TO_CHAR, POS_DEAD);
			continue;
		}

		char_printf(ch, "{C%s{x:\n", dir_name[door]);
		scan_list(pExit->to_room.r, ch, 1, door);
	}
}

void do_scan(CHAR_DATA *ch, const char *argument)
{
	char dir[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *in_room;
	ROOM_INDEX_DATA *to_room;
	EXIT_DATA *exit;	/* pExit */
	int door;
	int range;
	int i;
	CHAR_DATA *person;
	int numpeople;

	one_argument(argument, dir, sizeof(dir));
	WAIT_STATE(ch, PULSE_VIOLENCE);		// use 'exits' for quick search

	if (dir[0] == '\0') {
		do_scan2(ch, str_empty);
		return;
	}

	switch (dir[0]) {
	case 'N':
	case 'n':
		door = 0;
		break;
	case 'E':
	case 'e':
		door = 1;
		break;
	case 'S':
	case 's':
		door = 2;
		break;
	case 'W':
	case 'w':
		door = 3;
		break;
	case 'U':
	case 'u':
		door = 4;
		break;
	case 'D':
	case 'd':
		door = 5;
		break;
	default:
		char_puts("Wrong direction.\n", ch);
		return;
	}

	act("$n scans $t.", ch, dir_name[door], NULL, TO_ROOM | ACT_TRANS);
	if (!check_blind(ch))
		return;

	act_puts("You scan $t.", ch, dir_name[door], NULL, TO_CHAR | ACT_TRANS,
		 POS_DEAD);

	range = 1 + ch->level/10;

	in_room = ch->in_room;
	for (i = 1; i <= range; i++) {
		if (!can_look_in_exit(ch, (exit = in_room->exit[door])))
		{
			if (can_see_exit(ch, exit)
			&& IS_SET(exit->exit_info, EX_CLOSED))
			{
				char_printf(ch, "***** Range %d *****\n", i);
				char_puts("     You see closed door.\n", ch);
			}
			return;
		}

		to_room = exit->to_room.r;

		for (numpeople = 0, person = to_room->people; person != NULL;
		     person = person->next_in_room)
			if (can_see(ch,person)) {
				numpeople++;
				break;
			}

		if (numpeople) {
			char_printf(ch, "***** Range %d *****\n", i);
			show_char_to_char(to_room->people, ch);
			char_puts("\n", ch);
		}
		in_room = to_room;
	}
}

void do_request(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	AFFECT_DATA af;

	if (is_affected(ch, gsn_reserved)) {
		char_puts("Wait for a while to request again.\n", ch);
		return;
	}

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (IS_NPC(ch))
		return;

	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Request what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("Why don't you just ask the player?\n", ch);
		return;
	}

	if (!IS_GOOD(ch)) {
		do_say(victim,
		       "I will not give anything to someone so impure.");
		return;
	}

	if (ch->move < (50 + ch->level)) {
		do_say(victim, "You look rather tired, "
			       "why don't you rest a bit first?");
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);
	ch->move -= 10;
	ch->move = UMAX(ch->move, 0);

	if (((obj = get_obj_carry(victim , arg1)) == NULL
	&&  (obj = get_obj_wear(victim, arg1)) == NULL)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}
	
	if (IS_SET(obj->hidden_flags, OHIDE_INVENTORY)) {
		do_say(victim, "Sorry, but this obj is only for trade.");
		return;
	}

	if (!IS_GOOD(victim)) {
		do_say(victim, "I'm not about to give you anything!");
		do_murder(victim, ch->name);
		return;
	}

	if (obj->wear_loc != WEAR_NONE)
		unequip_char(victim, obj);

	if (!can_drop_obj(ch, obj)) {
		do_say(victim, "Sorry, I can't let go of it.  It's cursed.");
		return;
	}

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		char_puts("Your hands are full.\n", ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		char_puts("You can't carry that much weight.\n", ch);
		return;
	}

	if (!can_see_obj(ch, obj)) {
		act("You don't see that.", ch, NULL, victim, TO_CHAR);
		return;
	}

	if (IS_SET(obj->extra_flags, ITEM_QUIT_DROP)) {
		do_say(victim, "Sorry, I must keep it myself.");
		return;
	}

	if (obj->level >= ch->level + ch->alignment / 100) {
		do_say(victim, "In good time, my child");
		return;
	}	
	
	ch->alignment -= obj->level;
	
	obj_from_char(obj);
	obj_to_char(obj, ch);
	act("$n requests $p from $N.", ch, obj, victim, TO_NOTVICT);
	act("You request $p from $N.",	 ch, obj, victim, TO_CHAR);
	act("$n requests $p from you.", ch, obj, victim, TO_VICT);

	oprog_call(OPROG_GIVE, obj, ch, victim);

	ch->move -= (50 + ch->level);
	ch->move = UMAX(ch->move, 0);
	ch->hit -= 3 * (ch->level / 2);
	ch->hit = UMAX(ch->hit, 0);

	act("You feel grateful for the trust of $N.", ch, NULL, victim,
	    TO_CHAR);
//	char_puts("and for the goodness you have seen in the world.\n",ch);

	af.type = gsn_reserved;
	af.where = TO_AFFECTS;
	af.level = ch->level;
	af.duration = ch->level / 10 -
		(get_curr_stat(ch, STAT_CHA) / 2 - 18);
	af.location = APPLY_NONE;
	af.modifier = 0;
	af.bitvector = 0;
	affect_to_char(ch, &af);
}

void do_hometown(CHAR_DATA *ch, const char *argument)
{
	int amount;
	int htn;
	race_t *r;
	class_t *cl;

	if (IS_NPC(ch)) {
		act_puts("You can't change your hometown!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

	if (!IS_SET(ch->in_room->room_flags, ROOM_REGISTRY)) {
		act_puts("You have to be in the Registry "
			 "to change your hometown.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if ((htn = hometown_permanent(ch)) >= 0) {
		act_puts("Your hometown is $t, permanently. "
			 "You can't change your hometown.",
			 ch, hometown_name(htn), NULL,
			 TO_CHAR | ACT_TRANS, POS_DEAD);
		return;
	}

	amount = (ch->level * 250) + 1000;

	if (argument[0] == '\0') {
		act_puts("The change of hometown will cost you $j gold.",
			 ch, (const void*) amount, NULL, TO_CHAR, POS_DEAD);
		char_puts("Choose from: ", ch);
		hometown_print_avail(ch);
		char_puts(".\n", ch);
		return;
	}

	if ((htn = htn_lookup(argument)) < 0) {
		act_puts("That's not a valid hometown.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (htn == ch->pcdata->hometown) {
		act_puts("But you already live in $t!",
			 ch, hometown_name(htn), NULL,
			 TO_CHAR | ACT_TRANS, POS_DEAD);
		return;
	}

	if (hometown_restrict(HOMETOWN(htn), ch)) {
		act_puts("You are not allowed there.",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	if (ch->pcdata->bank_g < amount) {
		act_puts("You don't have enough money in bank "
			 "to change hometowns!",
			 ch, NULL, NULL, TO_CHAR, POS_DEAD);
		return;
	}

	ch->pcdata->hometown = htn;
	act_puts("Now your hometown is $t.",
		 ch, hometown_name(ch->pcdata->hometown),
		 NULL, TO_CHAR | ACT_TRANS, POS_DEAD);
}

void do_detect_hidden(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA	af;
	int		chance;
	int		sn;

	if ((sn = sn_lookup("detect hide")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (IS_AFFECTED(ch, AFF_DETECT_HIDDEN)) {
		char_puts("You are already as alert as you can be. \n",ch);
		return;
	}

	if (number_percent() > chance) {
		char_puts("You peer intently at the shadows "
			     "but they are unrevealing.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}
	
	WAIT_STATE(ch, PULSE_VIOLENCE);

	af.where     = TO_AFFECTS;
	af.type      = sn;
	af.level     = ch->level;
	af.duration  = ch->level;
	af.location  = APPLY_NONE;
	af.modifier  = 0;
	af.bitvector = AFF_DETECT_HIDDEN;
	affect_to_char(ch, &af);
	char_puts("Your awareness improves.\n", ch);
	check_improve(ch, sn, TRUE, 1);
}

void do_bear_call(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	bear;
	CHAR_DATA *	bear2;
	AFFECT_DATA	af;
	int		i;
	int		chance;
	int		sn;
	int		mana;

	if ((sn = sn_lookup("bear call")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	char_puts("You call for bears help you.\n",ch);
	act("$n shouts a bear call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		char_puts("You cannot summon the strength to handle "
			     "more bears right now.\n", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch, AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pIndexData->vnum == MOB_VNUM_BEAR) {
			char_puts("What's wrong with the bear you've got?",
				     ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&&  IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No bears listen you.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_PEACE |
					    ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL && ch->in_room->exit[1] == NULL
	&&   ch->in_room->exit[2] == NULL && ch->in_room->exit[3] == NULL
	&&   ch->in_room->exit[4] == NULL && ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		char_puts("No bears come to your rescue.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana "
			     "to shout a bear call.\n", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		char_puts("No bears listen you.\n", ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}

	check_improve(ch, sn, TRUE, 1);
	bear = create_mob(get_mob_index(MOB_VNUM_BEAR));

	for (i=0;i < MAX_STATS; i++)
		bear->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	bear->max_hit = IS_NPC(ch) ? ch->max_hit : ch->pcdata->perm_hit;
	bear->hit = bear->max_hit;
	bear->max_mana = IS_NPC(ch) ? ch->max_mana : ch->pcdata->perm_mana;
	bear->mana = bear->max_mana;
	bear->alignment = ch->alignment;
	bear->level = UMIN(100, 1 * ch->level-2);
	for (i=0; i < 3; i++)
		bear->armor[i] = interpolate(bear->level, -100, 100);
	bear->armor[3] = interpolate(bear->level, -100, 0);
	bear->sex = ch->sex;
	bear->gold = 0;

	bear2 = create_mob(bear->pIndexData);
	clone_mob(bear, bear2);

	SET_BIT(bear->affected_by, AFF_CHARM);
	SET_BIT(bear2->affected_by, AFF_CHARM);
	bear->master = bear2->master = ch;
	bear->leader = bear2->leader = ch;

	char_puts("Two bears come to your rescue!\n",ch);
	act("Two bears come to $n's rescue!", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= ch->level;
	af.duration	= SKILL(sn)->beats;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);

	char_to_room(bear, ch->in_room);
	char_to_room(bear2, ch->in_room);
}

void do_attacker(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *rch;
	char	tmp[MAX_STRING_LENGTH];
	
	if (IS_NPC(ch))
		return;
	
	if (argument[0] == '\0')
	{
		char_printf(ch, "You are %s\n", IS_ATTACKER(ch) ?
					"{RAttacker{x." : "{GPeaceful{x.");
		if (!IS_ATTACKER(ch))
			char_printf(ch, "If you want become attacker, type 'help attacker'.\n");
		return;
	}
	
	if (strcmp(argument, "become"))
	{
		char_puts("Syntax: attacker\n", ch);
		char_puts("Syntax: attacker become\n\n", ch);
		do_attacker(ch, str_empty);
		return;
	}
	
	if (!ch->in_room)
		return;
	
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && IS_SET(rch->pIndexData->act, ACT_SHERIFF))
			break;

	if (!rch) {
		char_puts("You must find Sheriff to become an attacker.\n", ch);
		return;
	}
	
	if (IS_ATTACKER(ch)){
		do_say(rch, "You are already attacker.");
		return;
	}
	
	snprintf(tmp, sizeof(tmp), "Congratulation, %s!!! You have become attacker.",
		ch->name);
	do_say(rch, tmp);
	interpret(rch, "giggle");
	save_char_obj(ch, FALSE);
	
	SET_BIT(ch->pcdata->otherf, OTHERF_ATTACKER);
}

/*  Sages say: level, stats, AFFECTED, Immune, Resist, Vulnerable, Svs(+slovami)
 *             damroll, hitroll
 */
void identify_self(CHAR_DATA *ch, CHAR_DATA *rch){
	char tmp[MAX_STRING_LENGTH];
	int cost;
		
	if (IS_NPC(ch)){
		do_say(rch, "This job I do only for PC players.");
		return;
	}
	
	cost = ch->level * 2;
	
	if (ch->gold * 100 + ch->silver < cost) {
		snprintf(tmp, sizeof(tmp), "You need at least %i silver.\n", cost);
		char_puts(tmp, ch);
		return;
	}
	else {
		deduct_cost(ch, cost);
		char_puts("Your purse feels lighter.\n", ch);
	}

	snprintf(tmp, sizeof(tmp), "%s, I can say about you next:", PERS(ch, rch));
	do_say(rch, tmp);
	snprintf(tmp, sizeof(tmp), "Your level is %i(%i).", ch->level, LEVEL(ch));
	do_say(rch, tmp);
	snprintf(tmp, sizeof(tmp), "Your stats: Str %i(%i) Int %i(%i) Wis %i(%i)",
		ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
		ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
		ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS));
	do_say(rch, tmp);
	snprintf(tmp, sizeof(tmp), "            Dex %i(%i) Con %i(%i) Cha %i(%i)",
		ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
		ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON),
		ch->perm_stat[STAT_CHA], get_curr_stat(ch, STAT_CHA));
	do_say(rch, tmp);
	snprintf(tmp, sizeof(tmp), "You have hitroll %i and damroll %i.",
		GET_HITROLL(ch), GET_DAMROLL(ch));
	do_say(rch, tmp);
	if (ch->affected_by){
		snprintf(tmp, sizeof(tmp), "You affected by %s.",
			flag_string(affect_flags, ch->affected_by));
		do_say(rch, tmp);
	}
	if (ch->imm_flags){
		snprintf(tmp, sizeof(tmp), "You immuned from %s.",
			flag_string(irv_flags, ch->imm_flags));
		do_say(rch, tmp);
	}
	if (ch->res_flags){
		snprintf(tmp, sizeof(tmp), "You resisted from %s.",
			flag_string(irv_flags, ch->res_flags));
		do_say(rch, tmp);
	}
	if (ch->vuln_flags){
		snprintf(tmp, sizeof(tmp), "You vulnerabled to %s.",
			flag_string(irv_flags, ch->vuln_flags));
		do_say(rch, tmp);
	}
	snprintf(tmp, sizeof(tmp), "Your align is %i.", ch->alignment);
	do_say(rch, tmp);
	
	snprintf(tmp, sizeof(tmp), "Your svs is %i and you have %i%% chance to save from spell your level.",
		get_char_svs(ch, 0, FALSE), percent_saves_spell_simply(ch->level, ch, DAM_NONE));
	do_say(rch, tmp);
	if (!IS_NPC(ch)) {
		snprintf(tmp, sizeof(tmp), "Your pit location is in general area of {W%s{z for {W%s{z.",
			get_altar(ch)->room->area->name,
			mlstr_cval(get_altar(ch)->room->name, ch));
		do_say(rch, tmp);
		if (ch->pcdata->ethos != ETHOS_LAWFUL) {
			snprintf(tmp, sizeof(tmp), "You may give {Y%d{z gold to sheriff for incrase your ethos.",
				GET_COST_ETHOS(ch));
			do_say(rch, tmp);
		}
		if (ch->pcdata->remort && ch->pcdata->remort->remorts) {
			snprintf(tmp, sizeof(tmp), "You remorted {W%d{z time.",
				ch->pcdata->remort->remorts);
			do_say(rch, tmp);
		}
	}
}

void do_identify(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	int cost;
	char str[MAX_STRING_LENGTH];

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_NPC(rch) && IS_SET(rch->pIndexData->act, ACT_SAGE))
			break;

	if (!rch) {
		 char_puts("No one here seems to know much "
			      "about that.\n", ch);
		 return;
	}
	
	if (!str_cmp(argument, "self")){
		identify_self(ch, rch);
		return;
	}
	
	if ((obj = get_obj_carry(ch, argument)) == NULL) {
		 char_puts("You are not carrying that.\n", ch);
		 return;
	}

	cost = obj->level;
	
	if (IS_IMMORTAL(ch))
		act("$n looks at you!", rch, obj, ch, TO_VICT);
	else if (ch->gold * 100 + ch->silver < cost) {
		act("$n resumes to identify by looking at $p.",
		       rch, obj, 0, TO_ROOM);
		snprintf(str, sizeof(str), "You need at least %i silver.\n", cost);
		char_puts(str, ch);
		return;
	}
	else {
		deduct_cost(ch, cost);
		char_puts("Your purse feels lighter.\n", ch);
	}

	act("$n gives a wise look at $p.", rch, obj, 0, TO_ROOM);
	{
		spell_spool_t sspell;

		sspell.ch	= ch;
		sspell.vo	= obj;
		sspell.percent	= 100;
		sspell.arg	= str_empty;

		spell_identify(&sspell);
	}
}

static void format_stat(char *buf, size_t len, CHAR_DATA *ch, int stat)
{
	if (ch->level >= 20 || IS_NPC(ch))
		snprintf(buf, len, "%2d (%2d)",
			 ch->perm_stat[stat],
			 get_curr_stat(ch, stat));
	else
		strnzcpy(buf, len, get_stat_alias(ch, stat));
}

void do_score(CHAR_DATA *ch, const char *argument)
{
	char buf2[MAX_INPUT_LENGTH];
	char title[MAX_STRING_LENGTH];
	const char *name;
	int ekle = 0;
	int delta;
	class_t *cl;
	BUFFER *output;
	bool can_flee;

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	output = buf_new(ch->lang);
	buf_add(output, "\n      {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/~~\\{x\n");

	strnzcpy(title, sizeof(title),
		 IS_NPC(ch) ? " Believer of Chronos." : ch->pcdata->title);
	name = IS_NPC(ch) ? capitalize(mlstr_val(ch->short_descr, ch->lang)) :
			    mlstr_val(ch->short_descr, ch->lang);
	delta = strlen(title) - cstrlen(title) + MAX_CHAR_NAME - strlen(name);
	title[32+delta] = '\0';
	snprintf(buf2, sizeof(buf2), "     {G|{x   %%s%%-%ds {Y%%3d years old   {G|____|{x\n", 33+delta);
	buf_printf(output, buf2, name, title, get_age(ch));

	buf_add(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n");

	format_stat(buf2, sizeof(buf2), ch, STAT_STR);
	buf_printf(output, "     {G| {RLevel: {x%-3d (%+3d)    {C| {RStr: {x%-11.11s {C| {RReligion  : {x%-10.10s {G|{x\n",
		   ch->level,
		   ch->drain_level,
		   buf2,
		   (IS_NPC(ch) || !ch->pcdata->religion) ? str_empty :
		   	ch->pcdata->religion->data->name);

	format_stat(buf2, sizeof(buf2), ch, STAT_INT);
	buf_printf(output,
"     {G| {RRace : {x%-11.11s  {C| {RInt: {x%-11.11s {C| {RPractice  : {x%-3d        {G|{x\n",
		race_name(ch->race),
		buf2,
		IS_NPC(ch) ? 0 : ch->pcdata->practice);

	format_stat(buf2, sizeof(buf2), ch, STAT_WIS);
	buf_printf(output,
"     {G| {RSex  : {x%-11.11s  {C| {RWis: {x%-11.11s {C| {RTrain     : {x%-3d        {G|{x\n",
		   ch->sex == 0 ?	"sexless" :
		   ch->sex == 1 ?	"male" :
					"female",
		   buf2,
		   IS_NPC(ch) ? 0 : ch->pcdata->train);

	format_stat(buf2, sizeof(buf2), ch, STAT_DEX);
	buf_printf(output,
"     {G| {RClass: {x%-12.12s {C| {RDex: {x%-11.11s {C| {RQuest Pnts: {x%-10d {G|{x\n",
		IS_NPC(ch) ? "mobile" : cl->name,
		buf2,
		IS_NPC(ch) ? 0 : ch->pcdata->questpoints);

	format_stat(buf2, sizeof(buf2), ch, STAT_CON);
	buf_printf(output,
"     {G| {RAlign: {x%-12.12s {C| {RCon: {x%-11.11s {C| {R%-10.10s: {x%-3d        {G|{x\n",
		flag_string(align_names, NALIGN(ch)),
		buf2,
		IS_NPC(ch) ? "Quest?" : (GET_QUEST_TYPE(ch) ? "Quest Time" : "Next Quest"),
		IS_NPC(ch) ? 0 : abs(GET_QUEST_TIME(ch)));
	can_flee = CAN_FLEE(ch, cl);
	format_stat(buf2, sizeof(buf2), ch, STAT_CHA);
	buf_printf(output,
"     {G| {REthos: {x%-12.12s {C| {RCha: {x%-11.11s {C| {R%s     : {x%-10d {G|{x\n",
		IS_NPC(ch) ? "mobile" : flag_string(ethos_table, ch->pcdata->ethos),
		buf2,
		can_flee ? "Wimpy" : "Death",
		can_flee ? GET_WIMPY(ch) : ch->pcdata->death);

	snprintf(buf2, sizeof(buf2), "%s %s.",
		 GETMSG("You are", ch->lang),
		 GETMSG(flag_string(position_names, ch->position), ch->lang));
	buf_printf(output, "     {G| {RHome : {x%-31.31s {C|{x %-22.22s {G|{x\n",
		IS_NPC(ch) ? "Midgaard" : hometown_name(ch->pcdata->hometown),
		buf2);

	buf_add(output, "     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x{x\n");

	if (ch->guarding != NULL) {
		ekle = 1;
		buf_printf(output,
"     {G| {GYou are guarding: {x%-12.12s                                  {G|{x\n",
			    ch->guarding->name);
	}

	if (ch->guarded_by != NULL) {
		ekle = 1;
		buf_printf(output,
"     {G| {GYou are guarded by: {x%-12.12s                                {G|{x\n",
			    ch->guarded_by->name);
	}

	if (!IS_NPC(ch)) {
		if (ch->pcdata->condition[COND_DRUNK] > 10) {
			ekle = 1;
			buf_printf(output,
"     {G| {GYou are drunk.                                                  {G|{x\n");
		}

		if (ch->pcdata->condition[COND_THIRST] <= 0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are thirsty.                                                {G|{x\n");
		}
/*		if (ch->pcdata->condition[COND_FULL]   ==	0) */
		if (ch->pcdata->condition[COND_HUNGER] <= 0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are hungry.                                                 {G|{x\n");
		}

		if (IS_GHOST(ch)) {
			ekle = 1;
			buf_add(output,
"     {G| {cYou are ghost.                                                  {G|{x\n");
		}

		if (ch->pcdata->condition[COND_BLOODLUST] <= 0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are hungry for blood.                                       {G|{x\n");
		}

		if (ch->pcdata->condition[COND_DESIRE] <=  0) {
			ekle = 1;
			buf_printf(output,
"     {G| {YYou are desiring your home.                                     {G|{x\n");
		}
	}

	if (!IS_IMMORTAL(ch) && IS_PUMPED(ch)) {
		ekle = 1;
		buf_printf(output,
"     {G| {RYour adrenalin is gushing!                                      {G|{x\n");
	}

	if (ekle)
		buf_add(output,
"     {G|{C+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+{G|{x\n");

	buf_printf(output,
"     {G| {RItems Carried :   {x%4d/%-5d        {RArmor vs exotic : {x%-10d{G|{x\n",
		ch->carry_number, can_carry_n(ch),
		GET_AC(ch,AC_EXOTIC));

	buf_printf(output,
"     {G| {RWeight Carried: {x%7d/%-10d  {RArmor vs bash   : {x%-10d{G|{x\n",
	get_carry_weight(ch), can_carry_w(ch), GET_AC(ch,AC_BASH));
	
	buf_printf(output,
"     {G| {RGold          :   {Y%-16d  {RArmor vs pierce : {x%-10d{G|{x\n",
		 ch->gold,GET_AC(ch,AC_PIERCE));

	buf_printf(output,
"     {G| {RSilver        :   {W%-17d {RArmor vs slash  : {x%-10d{G|{x\n",
		 ch->silver,GET_AC(ch,AC_SLASH));

	buf_printf(output,
"     {G| {RCurrent exp   :   {x%-14d    {RSaves vs Spell  : {x%-10d{G|{x\n",
		IS_NPC(ch) ? 0 : ch->pcdata->exp, get_char_svs(ch, 0, FALSE));

	buf_printf(output,
"     {G| {RExp to level  :   {x%-14d    {RHitP: {x%10d/%-10d {G|{x\n",
		IS_NPC(ch) ? 0 : exp_to_level(ch), ch->hit, ch->max_hit);

if (ch->level >= 15){
	buf_printf(output,
"     {G| {RHitroll       :   {x%-14d    {RMana: {x%10d/%-10d {G|{x\n",
		   GET_HITROLL(ch),ch->mana, ch->max_mana);
	buf_printf(output,
"     {G| {RDamroll       :   {x%-14d    {RMove: {x%10d/%-10d {G|{x\n",
		    GET_DAMROLL(ch), ch->move, ch->max_move);
} else {
	buf_printf(output,
"     {G|               {R:                     Mana: {x%10d/%-10d {G|{x\n",
				   ch->mana, ch->max_mana);
	buf_printf(output,
"     {G|               {R:                     Move: {x%10d/%-10d {G|{x\n",
				     ch->move, ch->max_move);
}
	buf_add(output, "  {G/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~/   |{x\n");
	buf_add(output, "  {G\\________________________________________________________________\\__/{x\n");

	if (IS_SET(ch->comm, COMM_SHOWAFF))
		show_affects(ch, output);
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_oscore)
{
	class_t *cl;
	char buf2[MAX_STRING_LENGTH];
	int i, k;
	BUFFER *output;

	if ((cl = class_lookup(ch->class)) == NULL)
		return;

	output = buf_new(ch->lang);

	buf_printf(output, "%s %s%s\n{x",
		GETMSG("You are", ch->lang),
		IS_NPC(ch) ? capitalize(mlstr_val(ch->short_descr, ch->lang)) :
			     ch->name,
		IS_NPC(ch) ? " The Believer of Chronos." : ch->pcdata->title);

	buf_printf(output, "Level {c%d(%+d){x, {c%d{x years old (%d hours).\n",
		ch->level, ch->drain_level, get_age(ch),
		IS_NPC(ch) ? 20 : (ch->pcdata->played + (int) (current_time - ch->pcdata->logon)) / 3600);

	buf_printf(output,
		"Race: {c%s{x  Sex: {c%s{x  Class: {c%s{x  "
		"Hometown: {c%s{x\n",
		race_name(ch->race),
		ch->sex == 0 ? "sexless" : ch->sex == 1 ? "male" : "female",
		IS_NPC(ch) ? "mobile" : cl->name,
		IS_NPC(ch) ? "Midgaard" : hometown_name(ch->pcdata->hometown));

	buf_printf(output,
		"You have {c%d{x/{c%d{x hit, {c%d{x/{c%d{x mana, "
		"{c%d{x/{c%d{x movement.\n",
		ch->hit, ch->max_hit, ch->mana, ch->max_mana,
		ch->move, ch->max_move);

	buf_printf(output,
		"You have {c%d{x practices and "
		"{c%d{x training sessions.\n",
		IS_NPC(ch) ? 0 : ch->pcdata->practice, IS_NPC(ch) ? 0 : ch->pcdata->train);

	buf_printf(output, "You are carrying {c%d{x/{c%d{x items "
		"with weight {c%ld{x/{c%d{x pounds.\n",
		ch->carry_number, can_carry_n(ch),
		get_carry_weight(ch), can_carry_w(ch));

	if (ch->level > 20 || IS_NPC(ch))
		buf_printf(output,
			"Str: {c%d{x({c%d{x)  Int: {c%d{x({c%d{x)  "
			"Wis: {c%d{x({c%d{x)  Dex: {c%d{x({c%d{x)  "
			"Con: {c%d{x({c%d{x)  Cha: {c%d{x({c%d{x)\n",
			ch->perm_stat[STAT_STR], get_curr_stat(ch, STAT_STR),
			ch->perm_stat[STAT_INT], get_curr_stat(ch, STAT_INT),
			ch->perm_stat[STAT_WIS], get_curr_stat(ch, STAT_WIS),
			ch->perm_stat[STAT_DEX], get_curr_stat(ch, STAT_DEX),
			ch->perm_stat[STAT_CON], get_curr_stat(ch, STAT_CON),
			ch->perm_stat[STAT_CHA], get_curr_stat(ch, STAT_CHA));
	else
		buf_printf(output,
			"Str: {c%-9s{x Wis: {c%-9s{x Con: {c%-9s{x\n"
			"Int: {c%-9s{x Dex: {c%-9s{x Cha: {c%-11s{x\n",
			get_stat_alias(ch, STAT_STR),
			get_stat_alias(ch, STAT_WIS),
			get_stat_alias(ch, STAT_CON),
			get_stat_alias(ch, STAT_INT),
			get_stat_alias(ch, STAT_DEX),
			get_stat_alias(ch, STAT_CHA));

	snprintf(buf2, sizeof(buf2),
		 "You have scored {c%d{x exp, and have %s%s%s.\n",
		 IS_NPC(ch) ? 0 : ch->pcdata->exp,
		 ch->gold + ch->silver == 0 ? "no money" :
					      ch->gold ? "{Y%ld gold{x " : str_empty,
		 ch->silver ? "{W%ld silver{x " : str_empty,
		 ch->gold + ch->silver ? ch->gold + ch->silver == 1 ?
					"coin" : "coins" : str_empty);
	if (ch->gold)
		buf_printf(output, buf2, ch->gold, ch->silver);
	else
		buf_printf(output, buf2, ch->silver);

	/* KIO shows exp to level */
	if (!IS_NPC(ch) && ch->level < LEVEL_HERO)
		buf_printf(output, "You need {c%d{x exp to level.\n",
			exp_to_level(ch));

	if (!IS_NPC(ch))
		buf_printf(output,
			"Quest Points: {c%d{x.  "
			"%s: {c%d{x.\n",
			ch->pcdata->questpoints, 
			IS_NPC(ch) ? "Quest?" : (GET_QUEST_TYPE(ch) ? 
					"Quest Time" : "Next Quest"),
			IS_NPC(ch) ? 0 : abs(GET_QUEST_TIME(ch)));

	if (CAN_FLEE(ch, cl))
		buf_printf(output, "Wimpy set to {c%d{x hit points.",
			   GET_WIMPY(ch));
	else
		buf_printf(output, "Total {c%d{x deaths up to now.",
			   ch->pcdata->death);

	if (ch->guarding)
		buf_printf(output, "  You are guarding: {W%s{x",
			   ch->guarding->name);

	if (ch->guarded_by)
		buf_printf(output, "  You are guarded by: {W%s{x",
			   ch->guarded_by->name);
	buf_add(output, "\n");

	if (!IS_NPC(ch)) {
		if (ch->pcdata->condition[COND_DRUNK] > 10)
			buf_add(output, "You are {cdrunk{x.\n");

		if (ch->pcdata->condition[COND_THIRST] <= 0)
			buf_add(output, "You are {rthirsty{x.\n");

/*		if (ch->pcdata->condition[COND_FULL] == 0) */
		if (ch->pcdata->condition[COND_HUNGER] <= 0)
			buf_add(output, "You are {rhungry{x.\n");
		if (ch->pcdata->condition[COND_BLOODLUST] <= 0)
			buf_add(output, "You are {rhungry for {Rblood{x.\n");
		if (ch->pcdata->condition[COND_DESIRE] <= 0)
			buf_add(output, "You are {rdesiring your home{x.\n");
		if (IS_SET(ch->pcdata->plr_flags, PLR_GHOST))
			buf_add(output, "You are {cghost{x.\n");
	}

	buf_printf(output, "You are %s.\n",
		   GETMSG(flag_string(position_names, ch->position), ch->lang));
	buf_printf(output, "You size is %s.\n",
		flag_string(size_table, ch->size));

	if ((ch->position == POS_SLEEPING || ch->position == POS_RESTING ||
	     ch->position == POS_FIGHTING || ch->position == POS_STANDING)
	&& !IS_IMMORTAL(ch) && IS_PUMPED(ch))
		buf_add(output, "Your {radrenalin is gushing{x!\n");

	/* print AC condition */
	for (i = 0; i < 4; i++) {
		static char* ac_name[4] = {
			"{cpiercing{x",
			"{cbashing{x",
			"{cslashing{x",
			"{cexotic{x"
		};

		buf_add(output, "You are ");
		if (get_ac_condition(ch, i, ch->level) == -100)
			buf_printf(output,
				   "{chopelessly vulnerable{x to %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= -70)
			buf_printf(output,
				   "{cdefenseless against{x %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= -40)
			buf_printf(output, "{cbarely protected{x from %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= -20)
			buf_printf(output, "{cslightly armored{x against %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= 0)
			buf_printf(output, "{csomewhat armored{x against %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= 20)
			buf_printf(output, "{carmored{x against %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= 40)
			buf_printf(output, "{cwell-armored{x against %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= 60)
			buf_printf(output, "{cvery well-armored{x against %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) <= 80)
			buf_printf(output, "{cheavily armored{x against %s.\n",
				   ac_name[i]);
		else if (get_ac_condition(ch, i, ch->level) < 100)
			buf_printf(output, "{csuperbly armored{x against %s.\n",
				   ac_name[i]);
		else
			buf_printf(output, "{calmost invulnerable{x to %s.\n",
				ac_name[i]);
	}
	
	/* Diferent svs */
	for (k = 0; k < QUANTITY_MAGIC / 3; k++)
	{
		if (!k)
			buf_add(output, "You have saves vs:    ");
		else
			buf_add(output, "                      ");

		for (i = 0; i < 3; i++)
		{
			buf_printf(output, "%-7s {c%3d{x(%3d)   ",
				flag_string(irv_flags, magic_schools[1 + k * 3 + i].bit_force),
				ch->saving_throws[1 + k * 3 + i],
				get_char_svs(ch, 1 + k * 3 + i, FALSE));

			if (i + k * 3 == QUANTITY_MAGIC - 1)
				break;
		}
		buf_add(output, "\n");
	}

	/* RT wizinvis and holy light */
	if (IS_IMMORTAL(ch)) {
		buf_printf(output, "Holy Light: {c%s{x",
			   IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT) ?
			   "on" : "off");

		if (ch->pcdata->invis_level)
			buf_printf(output, "  Invisible: {clevel %d{x",
				ch->pcdata->invis_level);

		if (ch->pcdata->incog_level)
			buf_printf(output, "  Incognito: {clevel %d{x",
				ch->pcdata->incog_level);
		buf_add(output, "\n");
	}

	if (ch->level >= 15)
		buf_printf(output, "Hitroll: {c%d{x  Damroll: {c%d{x.\n",
			GET_HITROLL(ch), GET_DAMROLL(ch));

	buf_add(output, "You are ");
	if (IS_GOOD(ch))
		buf_add(output, "good.  ");
	else if (IS_EVIL(ch))
		buf_add(output, "evil.  ");
	else
		buf_add(output, "neutral.  ");

	if (ch->pcdata) switch(ch->pcdata->ethos) {
	case ETHOS_LAWFUL:
		buf_add(output, "You have a lawful ethos.\n");
		break;
	case ETHOS_NEUTRAL:
		buf_add(output, "You have a neutral ethos.\n");
		break;
	case ETHOS_CHAOTIC:
		buf_add(output, "You have a chaotic ethos.\n");
		break;
	default:
		buf_add(output, "You have no ethos");;
		if (!IS_NPC(ch))
			buf_add(output, ", report it to the gods!\n");
		else
			buf_add(output, ".\n");
	}

	if (IS_NPC(ch) || !ch->pcdata->religion)
		buf_add(output, "You don't believe any religion.\n");
	else
		buf_printf(output,"Your religion is the way of %s.\n",
			ch->pcdata->religion->data->name);
	show_last(output, ch);

	if (IS_SET(ch->comm, COMM_SHOWAFF))
		show_affects(ch, output);
	send_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_affects(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;

	output = buf_new(ch->lang);
	show_affects(ch, output);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_lion_call(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *	gch;
	CHAR_DATA *	lion;
	CHAR_DATA *	lion2;
	AFFECT_DATA	af;
	int		i;
	int		chance;
	int		sn;
	int		mana;

	if ((sn = sn_lookup("lion call")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	char_puts("You call for lions help you.\n",ch);
	act("$n shouts a lion call.",ch,NULL,NULL,TO_ROOM);

	if (is_affected(ch, sn)) {
		char_puts("You cannot summon the strength to handle "
			     "more lions right now.\n", ch);
		return;
	}

	for (gch = npc_list; gch; gch = gch->next) {
		if (IS_AFFECTED(gch,AFF_CHARM)
		&&  gch->master == ch
		&&  gch->pIndexData->vnum == MOB_VNUM_LION) {
			char_puts("What's wrong with the lion you've got?", ch);
			return;
		}
	}

	if (ch->in_room != NULL
	&& IS_SET(ch->in_room->room_flags, ROOM_NOMOB)) {
		char_puts("No lions can listen you.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_PEACE |
					    ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->exit[0] == NULL && ch->in_room->exit[1] == NULL
	&&   ch->in_room->exit[2] == NULL && ch->in_room->exit[3] == NULL
	&&   ch->in_room->exit[4] == NULL && ch->in_room->exit[5] == NULL)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		char_puts("No lions come to your rescue.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana "
			     "to shout a lion call.\n", ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		check_improve(ch, sn, FALSE, 1);
		char_puts("No lions listen you.\n", ch);
		return;
	}

	check_improve(ch, sn, TRUE, 1);
	lion = create_mob(get_mob_index(MOB_VNUM_LION));

	for (i=0;i < MAX_STATS; i++)
		lion->perm_stat[i] = UMIN(25,2 * ch->perm_stat[i]);

	lion->max_hit = IS_NPC(ch) ? ch->max_hit : ch->pcdata->perm_hit;
	lion->hit = lion->max_hit;
	lion->max_mana = IS_NPC(ch) ? ch->max_mana : ch->pcdata->perm_mana;
	lion->mana = lion->max_mana;
	lion->alignment = ch->alignment;
	lion->level = UMIN(100,1 * ch->level-2);
	for (i=0; i < 3; i++)
		lion->armor[i] = interpolate(lion->level,-100,100);
	lion->armor[3] = interpolate(lion->level,-100,0);
	lion->sex = ch->sex;
	lion->gold = 0;

	lion2 = create_mob(lion->pIndexData);
	clone_mob(lion,lion2);

	SET_BIT(lion->affected_by, AFF_CHARM);
	SET_BIT(lion2->affected_by, AFF_CHARM);
	lion->master = lion2->master = ch;
	lion->leader = lion2->leader = ch;

	char_puts("Two lions come to your rescue!\n",ch);
	act("Two lions come to $n's rescue!",ch,NULL,NULL,TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= ch->level;
	af.duration	= SKILL(sn)->beats;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);

	char_to_room(lion, ch->in_room);
	char_to_room(lion2, ch->in_room);
}

/* object condition aliases */
char *get_cond_alias(OBJ_DATA *obj)
{
	char *stat;
	int istat = obj->condition;

	     if	(istat >= COND_EXCELLENT)	stat = "excellent";
	else if (istat >= COND_FINE)		stat = "fine";
	else if (istat >= COND_GOOD)		stat = "good";
	else if (istat >= COND_AVERAGE)		stat = "average";
	else if (istat >= COND_POOR)		stat = "poor";
	else					stat = "fragile";

	return stat;
}

int get_max_quant_slang(CHAR_DATA *ch)
{
	int max = 1;
	int intel;
	
	intel = get_curr_stat(ch, STAT_INT);
	if (intel >= 10)
		max += 1 + (intel - 10) / 3;
	return max;
}
int get_curr_quant_slang(CHAR_DATA *ch)	/* return quantity of current slang which has char */
{
	int quant = 0;
	int i;
	pcskill_t	*ps;
	
	for (i = 0; i < ch->pcdata->learned.nused; i++) {
		ps = VARR_GET(&ch->pcdata->learned, i);
		if (IS_SET(SKILL(ps->sn)->group, GROUP_SLANG)
		&& ps->maxpercent > 0
		&& ps->level > 0
		&& ps->level <= MAX_LEVEL)
			quant++;
	}
	return quant;
}

/* new practice */
void do_practice(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA	*mob;
	int		sn;
	skill_t		*sk;
	pcskill_t	*ps;
	class_t		*cl;
	int		adept;
	bool		found;
	int		hard, prac;
	char		arg[MAX_STRING_LENGTH];
	int		curr_quant_slang;
	int		cost = 0;

	if (IS_NPC(ch))
		return;

	WAIT_STATE(ch, PULSE_VIOLENCE);
	if (argument[0] == '\0') {
		BUFFER *output;
		int col = 0;
		int i;

		output = buf_new(-1);

		for (i = 0; i < ch->pcdata->learned.nused; i++) {
			ps = VARR_GET(&ch->pcdata->learned, i);

			if (ps->percent == 0
			||  (sk = skill_lookup(ps->sn)) == NULL
			||  ps->level > ch->level
			|| sk->group == GROUP_SLANG)
				continue;

			buf_printf(output, "%-19s %s%3d{x%%  ",
				   sk->name, ps->percent <= 1 ?  "{R":
				   (ps->percent >= ps->maxpercent ? "{G" : str_empty),
				   ps->percent);
			if (++col % 3 == 0)
				buf_add(output, "\n");
		}

		if (col % 3)
			buf_add(output, "\n");

		buf_printf(output, "   Lang(s) [max is %d]:\n",
			get_max_quant_slang(ch));
		col = 0;
		for (i = 0; i < ch->pcdata->learned.nused; i++) {
			ps = VARR_GET(&ch->pcdata->learned, i);

			if (ps->percent == 0
			||  (sk = skill_lookup(ps->sn)) == NULL
			||  ps->level > ch->level
			|| sk->group != GROUP_SLANG)
				continue;

			buf_printf(output, "%-19s %3d%%  ",
				   sk->name, ps->percent);
			if (++col % 3 == 0)
				buf_add(output, "\n");
		}
		if (col % 3)
			buf_add(output, "\n");

		buf_printf(output, "You have %d practice sessions left.\n",
			   ch->pcdata->practice);

		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if ((cl = CLASS(ch->class)) == NULL) {
		log_printf("do_practice: %s: class %d: unknown",
			   ch->name, ch->class);
		return;
	}

	one_argument(argument, arg, sizeof(arg));
	sk = SKILL((sn = sn_lookup(arg)));
	ps = pcskill_lookup(ch, sn);
		//ps = (pcskill_t*) skill_vlookup(&ch->pcdata->learned, arg);

	if (!IS_SET(sk->group, GROUP_SLANG) &&
	(!ps || ps->percent == 0)) {
		char_puts("You can't practice that.\n", ch);
		return;
	}

	if (ps && ps->level > ch->level) {
		char_puts("You are inexperienced for it.\n", ch);
		return;
	}
	
	curr_quant_slang = get_curr_quant_slang(ch);

	if (IS_SET(sk->group, GROUP_SLANG))
		prac = curr_quant_slang + 1;
	else
		prac = ps ? ps->level / 30 + 1 : 1;

	if (ch->pcdata->practice < prac) {
		char_printf(ch, "You have no practice sessions left (need %d).\n",
			prac);
		return;
	}

	if (sn == gsn_vampire) {
		char_puts("You can't practice that, only available "
			  "at questor.\n", ch);
		return;
	}

	found = FALSE;
	for (mob = ch->in_room->people; mob != NULL; mob = mob->next_in_room) {
		if (!IS_NPC(mob) || !IS_SET(mob->pIndexData->act, ACT_PRACTICE))
			continue;

		found = TRUE;

		/*
		if (IS_SET(sk->flags, SKILL_CLAN)) {
			if (ch->clan == mob->clan)
				break;
			continue;
		}
		*/

		if ((mob->pIndexData->practicer == 0 &&
		    (sk->group == GROUP_NONE ||
		     IS_SET(sk->group,	GROUP_CREATION | GROUP_HARMFUL |
					GROUP_PROTECTIVE | GROUP_DETECTION |
					GROUP_WEATHER))) )
			break;
		if (IS_SET(mob->pIndexData->practicer, sk->group)) {
			if (IS_SET(sk->group, GROUP_SLANG)
			&& sn != mob->pIndexData->slang)
				continue;
			if (mob->pIndexData->vnum != MOB_VNUM_NEWB_GUILDMASTER)
				cost = 50 + (ps ?  (ps->level * 2) : 0);
			break;
		}
	}

	if (mob == NULL) {
		if (found)
			char_puts("You can't do that here. "
				  "Use 'slook skill', 'help practice' "
				  "for more info.\n", ch);
		else
			char_puts("You couldn't find anyone "
				  "who can teach you.\n", ch);
		return;
	}

	adept = ps ? (ps->maxpercent / 2) : 90;
	if (ps && (ps->percent >= adept)) {
		char_printf(ch, "You are already learned at %s.\n",
			    sk->name);
		return;
	}
	
	if (ch->gold * 100 + ch->silver < cost) {
		doprintf(interpret, mob, "say Learning this %s will cost you %d silver.",
			sk->spell ? "spell" : "skill", cost);
		return;
	}
	
	if (IS_SET(sk->group, GROUP_SLANG)
	&& curr_quant_slang >= get_max_quant_slang(ch))
	{
		do_say(mob, "Sorry, but you cann't learning slang more.");
		do_say(mob, "Incrase your intelect if you are in need of this.");
		return;
	}
	
	if (cost > 0) {
		deduct_cost(ch, cost);
		char_puts("Your purse feels lighter.\n", ch);
	}

	ch->pcdata->practice = ch->pcdata->practice - prac;
	if (ps == NULL)
	{
		set_skill(ch, sn, 1, 1, 100, 50);
		if ((ps = pcskill_lookup(ch, sn)) == NULL)
		{
			bug("do_practice: can't create pc skill", 0);
			return;
		}
	}

	hard = ps->hard;
	hard = hard == 0 ? 1 : hard;
	ps->percent += (int_app[get_curr_stat(ch,STAT_INT)].learn) / (((float)hard)/50*3);
	
	if (ps->percent < adept) {
		act("You practice $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n practices $T.", ch, NULL, sk->name, TO_ROOM);
	}
	else {
		ps->percent = adept;
		act("You are now learned at $T.", ch, NULL, sk->name, TO_CHAR);
		act("$n is now learned at $T.", ch, NULL, sk->name, TO_ROOM);
	}
	char_printf(ch, "You spend {C%i{x practice for it.\n", prac);
}

void do_camp(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA af;
	int sn;
	int chance;
	int mana;

	if ((sn = sn_lookup("camp")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (is_affected(ch, sn)) {
		char_puts("You don't have enough power to handle more "
			     "camp areas.\n", ch);
		return;
	}

	if (IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_PEACE |
					    ROOM_PRIVATE | ROOM_SOLITARY)
	||  (ch->in_room->sector_type != SECT_FIELD
	&&   ch->in_room->sector_type != SECT_FOREST
	&&   ch->in_room->sector_type != SECT_MOUNTAIN
	&&   ch->in_room->sector_type != SECT_HILLS)) {
		char_puts("There are not enough leaves to camp here.\n",
			     ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough mana to make a camp.\n",
			     ch);
		return;
	}
	ch->mana -= mana;

	if (number_percent() > chance) {
		char_puts("You failed to make your camp.\n", ch);
		check_improve(ch, sn, FALSE, 4);
		return;
	}

	check_improve(ch, sn, TRUE, 4);
	WAIT_STATE(ch, SKILL(sn)->beats);

	char_puts("You succeeded to make your camp.\n", ch);
	act("$n succeeded to make $s camp.", ch, NULL, NULL, TO_ROOM);

	af.where	= TO_AFFECTS;
	af.type 	= sn;
	af.level	= ch->level;
	af.duration	= 12;
	af.bitvector	= 0;
	af.modifier	= 0;
	af.location	= APPLY_NONE;
	affect_to_char(ch, &af);

	af.where	= TO_ROOM_CONST;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= ch->level / 20;
	af.bitvector	= 0;
	af.modifier	= 2 * ch->level;
	af.location	= APPLY_ROOM_HEAL;
	affect_to_room(ch->in_room, &af);

	af.modifier	= ch->level;
	af.location	= APPLY_ROOM_MANA;
	affect_to_room(ch->in_room, &af);
}

void do_demand(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA  *obj;
	int chance;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (IS_NPC(ch))
		return;


	if (arg1[0] == '\0' || arg2[0] == '\0') {
		char_puts("Demand what from whom?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg2)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim)) {
		char_puts("Why don't you just want that directly "
			     "from the player?\n", ch);
		return;
	}

	WAIT_STATE(ch, PULSE_VIOLENCE);

	chance = IS_EVIL(victim) ? 10 : IS_GOOD(victim) ? -5 : 0;
	chance += (get_curr_stat(ch,STAT_CHA) - 15) * 10;
	chance += ch->level - victim->level;

	chance = (get_skill(ch, gsn_demand))*chance/100;

	if (number_percent() > chance) {
		do_say(victim, "I'm not about to give you anything!");
		check_improve(ch, gsn_demand, FALSE, 1);
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch, gsn_demand, TRUE, 1);

	if (((obj = get_obj_carry(victim , arg1)) == NULL
	&&   (obj = get_obj_wear(victim, arg1)) == NULL)) {
		do_say(victim, "Sorry, I don't have that.");
		return;
	}
	
	if (IS_SET(obj->hidden_flags, OHIDE_INVENTORY) && number_percent() > chance / 10)
	{
		do_say(victim, "I'm not about to give you my goods!");
		do_murder(victim, ch->name);
		return;
	}

	if (IS_SET(obj->extra_flags, ITEM_QUIT_DROP)) {
		do_say(victim, "Forgive me, my master, I can't give it to anyone.");
		return;
	}


	if (obj->wear_loc != WEAR_NONE)
		unequip_char(victim, obj);

	if (!can_drop_obj(ch, obj)) {
		do_say(victim, "It's cursed so, I can't let go of it. "
			       "Forgive me, my master");
		return;
	}

	if (ch->carry_number + get_obj_number(obj) > can_carry_n(ch)) {
		char_puts("Your hands are full.\n", ch);
		return;
	}

	if (ch->carry_weight + get_obj_weight(obj) > can_carry_w(ch)) {
		char_puts("You can't carry that much weight.\n", ch);
		return;
	}

	if (!can_see_obj(ch, obj)) {
		act("You don't see that.", ch, NULL, victim, TO_CHAR);
		return;
	}

	obj_from_char(obj);
	obj_to_char(obj, ch);
	act("$n demands $p from $N.", ch, obj, victim, TO_NOTVICT);
	act("You demand $p from $N.",	ch, obj, victim, TO_CHAR  );
	act("$n demands $p from you.", ch, obj, victim, TO_VICT  );

	oprog_call(OPROG_GIVE, obj, ch, victim);
	char_puts("Your power makes all around the world shivering.\n",ch);
}

void do_control(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int chance;
	int sn;
	race_t *r;

	argument = one_argument(argument, arg, sizeof(arg));

	if ((sn = sn_lookup("control animal")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("Huh?\n", ch);
		return;
	}

	if (arg[0] == '\0') {
		char_puts("Charm what?\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if ((r = race_lookup(ORG_RACE(victim))) && r->pcdata) {
		char_puts("You should try this on monsters?\n", ch);
		return;
	}

	if (count_charmed(ch))
		return;

	if (is_safe(ch, victim))
		return;

	WAIT_STATE(ch, SKILL(sn)->beats);

	chance += (get_curr_stat(ch,STAT_CHA) - 20) * 5;
	chance += (ch->level - victim->level) * 3;
	chance +=
	(get_curr_stat(ch,STAT_INT) - get_curr_stat(victim,STAT_INT)) * 5;

	if (IS_AFFECTED(victim, AFF_CHARM)
	||  IS_AFFECTED(ch, AFF_CHARM)
	||  number_percent() > chance
	||  ch->level < (victim->level + 2)
	||  IS_SET(victim->imm_flags, MAGIC_MENTAL)
	||  (IS_NPC(victim) && victim->pIndexData->pShop != NULL)) {
		check_improve(ch, sn, FALSE, 2);
		do_say(victim,"I'm not about to follow you!");
		do_murder(victim, ch->name);
		return;
	}

	check_improve(ch, sn, TRUE, 2);

	if (victim->master)
		stop_follower(victim);
	SET_BIT(victim->affected_by, AFF_CHARM);
	victim->master = victim->leader = ch;

	act("Isn't $n just so nice?", ch, NULL, victim, TO_VICT);
	if (ch != victim)
		act("$N looks at you with adoring eyes.",
		    ch, NULL, victim, TO_NOTVICT);
}

void do_make_arrow(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *arrow;
	AFFECT_DATA af;
	OBJ_INDEX_DATA *pObjIndex;
	int count, mana, wait;
	char arg[MAX_INPUT_LENGTH];
	int chance;
	int color_chance = 100;
	int vnum = -1;
	int sn;
	int color = -1;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("make arrow")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know how to make arrows.\n", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You couldn't find enough wood.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	wait = SKILL(sn)->beats;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')
		vnum = OBJ_VNUM_WOODEN_ARROW;
	else if (!str_prefix(arg, "green")) {
		color = sn_lookup("green arrow");
		vnum = OBJ_VNUM_GREEN_ARROW;
	}
	else if (!str_prefix(arg, "red")) {
		color = sn_lookup("red arrow");
		vnum = OBJ_VNUM_RED_ARROW;
	}
	else if (!str_prefix(arg, "white")) {
		color = sn_lookup("white arrow");
		vnum = OBJ_VNUM_WHITE_ARROW;
	}
	else if (!str_prefix(arg, "blue")) {
		color = sn_lookup("blue arrow");
		vnum = OBJ_VNUM_BLUE_ARROW;
	}

	if (vnum < 0) {
		char_puts("You don't know how to make "
			  "that kind of arrow.\n", ch);
		return;
	}

	if (color > 0) {
		color_chance = get_skill(ch, color);
		mana += SKILL(color)->min_mana;
		wait += SKILL(color)->beats;
	}

	if (ch->mana < mana) {
		char_puts("You don't have enough energy "
			  "to make that kind of arrows.\n", ch);
		return;
	}

	ch->mana -= mana;
	WAIT_STATE(ch, wait);

	char_puts("You start to make arrows!\n",ch);
	act("$n starts to make arrows!", ch, NULL, NULL, TO_ROOM);
	pObjIndex = get_obj_index(vnum);
	for(count = 0; count < ch->level / 5; count++) {
		if (number_percent() > chance * color_chance / 100) {
			char_puts("You failed to make the arrow, "
				  "and broke it.\n", ch);
			check_improve(ch, sn, FALSE, 3);
			if (color > 0)
				check_improve(ch, color, FALSE, 3);
			continue;
		}

		check_improve(ch, sn, TRUE, 3);
		if (color > 0)
			check_improve(ch, color, TRUE, 3);

		arrow = create_obj(pObjIndex, 0);
		arrow->level = ch->level;
		arrow->value[1] = 4 + ch->level / 10;
		arrow->value[2] = 4 + ch->level / 10;

		af.where	 = TO_OBJECT;
		af.type		 = sn;
		af.level	 = ch->level;
		af.duration	 = -1;
		af.location	 = APPLY_HITROLL;
		af.modifier	 = ch->level / 10;
		af.bitvector 	 = 0;
		affect_to_obj(arrow, &af);

		af.where	= TO_OBJECT;
		af.type		= sn;
		af.level	= ch->level;
		af.duration	= -1;
		af.location	= APPLY_DAMROLL;
		af.modifier	= ch->level / 10;
		af.bitvector	= 0;
		affect_to_obj(arrow, &af);

		obj_to_char(arrow, ch);
		act_puts("You successfully make $p.",
			 ch, arrow, NULL, TO_CHAR, POS_DEAD);
	}
}

void do_make_bow(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *	bow;
	AFFECT_DATA	af;
	int		mana;
	int		sn;
	int		chance;

	if (IS_NPC(ch))
		return;

	if ((sn = sn_lookup("make bow")) < 0
	||  (chance = get_skill(ch, sn)) == 0) {
		char_puts("You don't know how to make bows.\n", ch);
		return;
	}

	if (ch->in_room->sector_type != SECT_FIELD
	&&  ch->in_room->sector_type != SECT_FOREST
	&&  ch->in_room->sector_type != SECT_HILLS) {
		char_puts("You couldn't find enough wood.\n", ch);
		return;
	}

	mana = SKILL(sn)->min_mana;
	if (ch->mana < mana) {
		char_puts("You don't have enough energy to make a bow.\n",
			     ch);
		return;
	}
	ch->mana -= mana;
	WAIT_STATE(ch, SKILL(sn)->beats);

	if (number_percent() > chance) {
		char_puts("You failed to make the bow, and broke it.\n",
			     ch);
		check_improve(ch, sn, FALSE, 1);
		return;
	}
	check_improve(ch, sn, TRUE, 1);

	bow = create_obj(get_obj_index(OBJ_VNUM_RANGER_BOW), 0);
	bow->level = ch->level;
	bow->value[1] = 4 + ch->level / 15;
	bow->value[2] = 4 + ch->level / 15;

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= -1;
	af.location	= APPLY_HITROLL;
	af.modifier	= ch->level / 10;
	af.bitvector 	= 0;
	affect_to_obj(bow, &af);

	af.where	= TO_OBJECT;
	af.type		= sn;
	af.level	= ch->level;
	af.duration	= -1;
	af.location	= APPLY_DAMROLL;
	af.modifier	= ch->level / 10;
	af.bitvector 	= 0;
	affect_to_obj(bow, &af);

	obj_to_char(bow, ch);
	act_puts("You successfully make $p.", ch, bow, NULL, TO_CHAR, POS_DEAD);
}

void do_make(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("You can make either bow or arrow.\n",ch);
		return;
	}

	if (!str_prefix(arg, "arrow"))
		do_make_arrow(ch, argument);
	else if (!str_prefix(arg, "bow"))
		do_make_bow(ch, argument);
	else
		do_make(ch, str_empty);
}

/*Added by Osya*/
void do_homepoint(CHAR_DATA *ch, const char *argument)
{
        AFFECT_DATA af;
        int sn;
        int chance;
        char arg[MAX_INPUT_LENGTH];

        if ((sn = sn_lookup("homepoint")) < 0
        ||  (chance = get_skill(ch, sn)) == 0) {
                char_puts("Huh?\n", ch);
                return;
        }

        if (is_affected(ch, sn)) {
                char_puts("You fatigue for searching new home.\n", ch) ;
                return;
        }

        if (IS_SET(ch->in_room->room_flags, ROOM_SAFE | ROOM_PEACE |
                                            ROOM_PRIVATE | ROOM_SOLITARY)
        ||  (ch->in_room->sector_type != SECT_FIELD
        &&   ch->in_room->sector_type != SECT_FOREST
        &&   ch->in_room->sector_type != SECT_MOUNTAIN
        &&   ch->in_room->sector_type != SECT_HILLS)) {
                char_puts("You are cannot set home here.\n",
                             ch);
                return;
        }

        if (ch->mana < ch->max_mana) {
                char_puts("You don't have strength to make a new home.\n",
                             ch);
                return;
        }

        ch->mana = 0 ;

        if (number_percent() > chance) {
                char_puts("You failed to make your homepoint.\n", ch);
                check_improve(ch, sn, FALSE, 4);
                return;
        }

        check_improve(ch, sn, TRUE, 4);
        WAIT_STATE(ch, SKILL(sn)->beats);

        char_puts("You succeeded to make your homepoint.\n", ch);
        act("$n succeeded to make $s homepoint. ", ch, NULL, NULL, TO_ROOM);

        af.where        = TO_AFFECTS;
        af.type         = sn;
        af.level        = ch->level;
        af.duration     = 100;
        af.bitvector    = 0;
        af.modifier     = 0;
        af.location     = APPLY_NONE;
        affect_to_char(ch, &af);

        argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] && !str_prefix(arg, "motherland"))
		ch->pcdata->homepoint = NULL;
        else 
		ch->pcdata->homepoint = ch->in_room; 
}
