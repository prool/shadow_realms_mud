/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * Copyright (c) 1998 fjoe <fjoe@iclub.nsu.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: comm_act.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "merc.h"
#include "comm_colors.h"
#include "mob_prog.h"
#include "lang.h"

/*
 * static functions declarations
 */
static char *		translate(CHAR_DATA *ch, CHAR_DATA *victim,
				  const char *i);
static CHAR_DATA *	act_args(CHAR_DATA *ch, CHAR_DATA *vch, int flags);
static bool		act_skip(CHAR_DATA *ch, CHAR_DATA *vch, CHAR_DATA *to,
				 int flags, int min_pos);
static void		act_raw(CHAR_DATA *ch, CHAR_DATA *to,
				const void *arg1,
				const void *arg2,
				const void *arg3,
				const char *str, int flags);

void act_puts3(const char *format, CHAR_DATA *ch,
	       const void *arg1, const void *arg2, const void *arg3,
	       int flags, int min_pos)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;

	if (!format || (to = act_args(ch, vch, flags)) == NULL)
		return;

	if (IS_SET(flags, TO_CHAR | TO_VICT)) {
		if (!act_skip(ch, vch, to, flags, min_pos)) {
			act_raw(ch, to, arg1, arg2, arg3,
				GETMSG(format, to->lang), flags);
		}
		return;
	}
		
	for(; to; to = to->next_in_room) {
		if (act_skip(ch, vch, to, flags, min_pos)) 
			continue;
		act_raw(ch, to, arg1, arg2, arg3,
			GETMSG(format, to->lang), flags);
	}
}

void act_puts3_mlstr(const mlstring *format, CHAR_DATA *ch,
	       const void *arg1, const void *arg2, const void *arg3,
	       int flags, int min_pos)
{
	CHAR_DATA *to;
	CHAR_DATA *vch = (CHAR_DATA *) arg2;

	if (!format || (to = act_args(ch, vch, flags)) == NULL)
		return;

	if (IS_SET(flags, TO_CHAR | TO_VICT)) {
		if (!act_skip(ch, vch, to, flags, min_pos)) {
			act_raw(ch, to, arg1, arg2, arg3, mlstr_cval(format, to), flags);
		}
		return;
	}
		
	for(; to; to = to->next_in_room) {
		if (act_skip(ch, vch, to, flags, min_pos)) 
			continue;
		act_raw(ch, to, arg1, arg2, arg3, mlstr_cval(format, to), flags);
	}
}

/*----------------------------------------------------------------------------
 * static functions
 */

/* common and slang should have the same size */
char common[] =
	"aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ"
	"áÁâÂ÷×çÇäÄåÅ³£öÖúÚéÉêÊëËìÌíÍîÎïÏðÐòÒóÓôÔõÕæÆèÈãÃþÞûÛýÝÿßùÙøØüÜàÀñÑ";
char slang[] =
	"eEcCdDfFiIgGhHjJoOkKlLmMnNpPuUqQrRsStTvVyYwWxXzZaAbB"
	"õÕâÂâÂïÏâÂáÁùÙâÂâÂõÕâÂâÂâÂâÂéÉáÁâÂâÂòÒâÂùÙâÂâÂâÂâÂâÂâÂÿßõÕøØáÁõÕùÙ";

static int GET_SLANG_P(CHAR_DATA *ch, int slang)
{
	return IS_PC((ch)) ? get_skill(ch, slang) :
		(ch->pIndexData->slang == slang ? 100 : get_curr_stat(ch, STAT_WIS));
}

static char *slang_hash(const char *i, int percent, int gsn_slang, bool not_show_slang)
{
	static char trans[MAX_STRING_LENGTH];
	char *o;
	bool trans_letter;
	bool change_p;
	
	if (not_show_slang)
		trans[0] = '\0';
	else
		snprintf(trans, sizeof(trans), "[%s] ",
			number_percent() > percent ? "?" : skill_name(gsn_slang));

	o = strchr(trans, '\0');
	trans_letter = number_percent() > percent;
	change_p = FALSE;
	for (; *i && o-trans < sizeof(trans)-1; i++, o++) {
		char *p = strchr(common, *i);
		if (p)
		{
			if (trans_letter)
				*o = slang[p-common];
			else
				*o = *i;
			change_p = TRUE;
		} else {
			if (change_p)
			{
				trans_letter = number_percent() > percent;
				change_p = FALSE;
			}
			*o = *i;
		}
	}
	*o = '\0';

	return trans; 
}

void show_ed(CHAR_DATA *ch, ED_DATA *ed, bool wait)
{
	int slang = ed->slang;
	int percent;
	
	if (slang)
	{
		percent = get_skill(ch, slang);
		if (percent >= 100
		|| percent * 0.7 >= number_percent()
		|| (IS_PC(ch) && IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT)))
		{
			if (!IS_SET(ch->comm, COMM_NOT_VSLANG))
				char_printf(ch, "[%s] ",
					skill_name(slang));
			char_mlputs(ed->description, ch);
			check_improve(ch, slang, TRUE, 20);
		} else {
			char_puts(slang_hash(mlstr_cval(ed->description, ch),
					percent, slang,
					IS_SET(ch->comm, COMM_NOT_VSLANG) ? TRUE : FALSE),
				ch);
			if (wait)
				WAIT_STATE(ch, SKILL(slang)->beats);
			check_improve(ch, slang, FALSE, 15);
		}
	} else {
		char_mlputs(ed->description, ch);
	}
}

/* ch says, victim hears */
/* special modified for SR by Xor */
static char *translate(CHAR_DATA *ch, CHAR_DATA *victim, const char *i)
{
	static char trans[MAX_STRING_LENGTH];
	int percent, pch, pv;
//	race_t *r;
	
	if (*i == '\0'
	|| (ch == NULL)
	|| (victim == NULL)
	|| ch == victim)
	{
		strnzcpy(trans, sizeof(trans), i);
		return trans;
	}
	
	pch = GET_SLANG_P(ch, ch->slang);
	pv = GET_SLANG_P(victim, ch->slang);
	percent = UMIN(pch, pv) - (100 - URANGE(0, UMAX(pch, pv), 100)) / 5;
	percent = URANGE(0, percent, 100);
	
	if (percent >= number_percent()
	|| (IS_PC(victim) && IS_SET(victim->pcdata->plr_flags, PLR_HOLYLIGHT)))
	{
		if (IS_SET(victim->comm, COMM_NOT_VSLANG))
			strnzcpy(trans, sizeof(trans), i);
		else
			snprintf(trans, sizeof(trans), "[%s] %s",
				skill_name(ch->slang), i);
		check_improve(victim, ch->slang, TRUE, 20);
		check_improve(ch, ch->slang, TRUE, 32);
		return trans;
	}
	
	check_improve(victim, ch->slang, FALSE, 20);
	check_improve(ch, ch->slang, FALSE, 32);

	return slang_hash(i, percent, ch->slang,
		IS_SET(victim->comm, COMM_NOT_VSLANG) ? TRUE : FALSE);

/*	if (*i == '\0'
	||  (ch == NULL) || (victim == NULL)
	||  IS_NPC(ch) || IS_NPC(victim)
	||  IS_IMMORTAL(ch) || IS_IMMORTAL(victim)
	||  ch->slang == SLANG_COMMON
	||  ((r = race_lookup(ORG_RACE(victim))) &&
	     r->pcdata &&
	     ch->slang == r->pcdata->slang)) {

		if (IS_IMMORTAL(victim))
			snprintf(trans, sizeof(trans), "[%s] %s",
				 flag_string(slang_table, ch->slang), i);
		else

			strnzcpy(trans, sizeof(trans), i);
		return trans;
	}

	snprintf(trans, sizeof(trans), "[%s] ",
		 flag_string(slang_table, ch->slang));
	o = strchr(trans, '\0');
	for (; *i && o-trans < sizeof(trans)-1; i++, o++) {
		char *p = strchr(common, *i);
		*o = p ? slang[p-common] : *i;
	}
	*o = '\0';

	return trans;
*/
}

static CHAR_DATA *act_args(CHAR_DATA *ch, CHAR_DATA *vch, int flags)
{
	if (IS_SET(flags, TO_CHAR))
		return ch;

	if (IS_SET(flags, TO_VICT))
		return vch;

	if (ch == NULL || ch->in_room == NULL)
		return NULL;

	return ch->in_room->people;
}
 
static bool act_skip(CHAR_DATA *ch, CHAR_DATA *vch, CHAR_DATA *to,
		     int flags, int min_pos)
{
	if (to->position < min_pos)
		return TRUE;

	if (IS_SET(flags, TO_CHAR) && to != ch)
		return TRUE;
	if (IS_SET(flags, TO_VICT) && (to != vch || to == ch))
		return TRUE;
	if (IS_SET(flags, TO_ROOM) && to == ch)
		return TRUE;
	if (IS_SET(flags, TO_NOTVICT) && (to == ch || to == vch))
		return TRUE;

	if (IS_NPC(to)
	&&  to->desc == NULL
	&&  !HAS_TRIGGER(to, TRIG_ACT))
		return TRUE;
 
	if (IS_SET(flags, ACT_NOMORTAL) && !IS_NPC(to) && !IS_IMMORTAL(to))
		return TRUE;

/* twitlist handling */
	if (IS_SET(flags, ACT_NOTWIT)
	&&  !IS_NPC(to) && !IS_IMMORTAL(to)
	&&  !IS_NPC(ch) && !IS_IMMORTAL(ch)
	&&  is_name(ch->name, to->pcdata->twitlist))
		return TRUE;

/* check "deaf dumb blind" chars */
	if (IS_SET(flags, ACT_NODEAF) && is_affected(to, gsn_deafen))
		return TRUE;

/* skip TO_NOTVICT or TO_ROOM if char affected 'blind' */
	if (IS_SET(flags, ACT_NOBLIND)
	 && IS_SET(flags, TO_NOTVICT | TO_ROOM)
	 && (IS_AFFECTED(to, AFF_BLIND)
	     && !(IS_PC(to) && IS_SET(to->pcdata->plr_flags, PLR_HOLYLIGHT))
	    )
	   )
		return TRUE;

/* skip verbose messages */
	if (IS_SET(flags, ACT_VERBOSE)
	&&  IS_SET(to->comm, COMM_NOVERBOSE))
		return TRUE;

	return FALSE;
}

static char * const he_she  [] = { "it",  "he",  "she", "it",  "they" };
static char * const him_her [] = { "it",  "him", "her", "it",  "their" };
static char * const his_her [] = { "its", "his", "her", "its", "theirs"};
 
struct tdata {
	char	type;
	int	arg;
	char *	p;
};

#define TSTACK_SZ 5

static int SEX(CHAR_DATA *ch, CHAR_DATA *looker)
{
	if (is_affected(ch, gsn_doppelganger)
	&&  (IS_NPC(looker) || !IS_SET(looker->pcdata->plr_flags, PLR_HOLYLIGHT)))
		ch = ch->doppel;
	return URANGE(0, ch->sex, SEX_MAX-1);
}

static const char *
act_format_text(const void *text, CHAR_DATA *ch, CHAR_DATA *to, flag32_t flags)
{
	if (IS_SET(flags, ACT_FULLMLSTR))
	{
		text = mlstr_cval(text, to);
	} else {
		if (IS_SET(flags, ACT_TRANS))
			text = GETMSG(text, to->lang);
		if (IS_SET(flags, ACT_STRANS))
			text = translate(ch, to, text);
	}
	return text;
}
	
static /*inline*/ const char * // inline delete by prool
act_format_obj(OBJ_DATA *obj, CHAR_DATA *to, int sp, flag32_t flags)
{
	const char *descr;

	if (!can_see_obj(to, obj))
		return GETMSG("something", to->lang);

	if (sp < 0) {
		if (IS_SET(flags, ACT_FORMSH))
			return format_short(obj->short_descr, obj->name, to);

		return fix_short(mlstr_cval(obj->short_descr, to));
	}

	return descr = mlstr_cval(obj->short_descr, to);
}

/*
 * vch is (CHAR_DATA*) arg2
 * vch1 is (CHAR_DATA*) arg1
 * obj1 is (OBJ_DATA*) arg1
 * obj2 is (OBJ_DATA*) arg2
 *
 * Known act_xxx format codes are:
 *
 * a
 * A - $A{arg, word} - associate arg with word in genders
 *     function gets gender from word and apply it to arg
 * b
 * B
 * c - $cn{...} - case number ``n''
 * C
 * d - door name (arg2)
 * D
 * e - he_she(ch)
 * E - he_she(vch)
 * f
 * F
 * g - $gx{...} - gender form depending on sex of ``x'', where x is:
 *	c - char
 *	v - vch ((CHAR_DATA*) arg2)
 *	t - to (char the message is being printed to)
 * G
 * h - $h{...} - capitalize arguments
 * H
 * i - name(vch1)
 * I - name(vch3)
 * j - num(arg1)
 * J - num(arg3)
 * k
 * K
 * l
 * L
 * m - him_her(ch)
 * M - him_her(vch)
 * n - name(ch)
 * N - name(vch)
 * o
 * O
 * p - name(obj1)
 * P - name(obj2)
 * q - $qx{...} - numeric form depending on ``x'' where x is:
 *	j - num(arg1)
 *	J - num(arg2)
 * Q
 * r - room name (arg1)
 * R - room name (arg3)
 * s - his_her(ch)
 * S - his_her(vch)
 * t - text(arg1)
 * T - text(arg2)
 * u - text(arg1)
 * U - text(arg3)
 * v
 * V
 * w
 * W
 * x
 * X
 * y
 * Y
 * z
 * Z
 *
 */
static void act_raw(CHAR_DATA *ch, CHAR_DATA *to,
		    const void *arg1, const void *arg2, const void *arg3,
		    const char *str, int flags)
{
	CHAR_DATA *	vch = (CHAR_DATA*) arg2;
	CHAR_DATA *	vch1 = (CHAR_DATA*) arg1;
	CHAR_DATA *	vch3 = (CHAR_DATA*) arg3;
	int		num1 = (int) arg1;
	int		num3 = (int) arg3;
	ROOM_INDEX_DATA *room1 = (ROOM_INDEX_DATA*) arg1;
	ROOM_INDEX_DATA *room3 = (ROOM_INDEX_DATA*) arg3;
	OBJ_DATA *	obj1 = (OBJ_DATA*) arg1;
	OBJ_DATA *	obj2 = (OBJ_DATA*) arg2;
	char 		buf	[MAX_STRING_LENGTH];
	char 		tmp	[MAX_STRING_LENGTH];

	char *		point = buf;
	const char *	s = str;

	struct tdata	tstack[TSTACK_SZ];
	int		sp = -1;

	while(*s) {
		char		code;
		char		subcode;
		const char *	i;

		switch (*s) {
		default:
			*point++ = *s++;
			break;

		case '}':
			if (sp < 0) {
				*point++ = *s++;
				continue;
			}

			if (sp < TSTACK_SZ) {
				int rulecl = 0;

				switch (tstack[sp].type) {
				case 'g':
					rulecl = RULES_GENDER;
					break;
				case 'c':
					rulecl = RULES_CASE;
					break;
				case 'q':
					rulecl = RULES_QTY;
					break;
				case 'h':
					*point = '\0';
					strnzcpy(tstack[sp].p,
						sizeof(buf) - (tstack[sp].p - buf),
						capitalize(tstack[sp].p));
					point = strchr(tstack[sp].p, '\0');
					sp--;
					s++;
					continue;
				}

				*point = '\0';
				strnzcpy(tstack[sp].p, 
					 sizeof(buf) - (tstack[sp].p - buf),
					 word_form(tstack[sp].p, tstack[sp].arg,
						   to->lang, rulecl));
				point = strchr(tstack[sp].p, '\0');
			}

			sp--;
			s++;
			continue;

		case '{':
			if (*(s+1) == '}')
				s++;
			*point++ = *s++;
			continue;

		case '$':
			s++;

			switch (code = *s++) {
			default:  
				i = " <@@@> ";
				log_printf("act_raw: '%s': bad code $%c",
					   str, code);
				continue;
/* text arguments */
			case 't': 
			case 'u':
				i = act_format_text(arg1, ch, to, flags);
				break;

			case 'T':
				i = act_format_text(arg2, ch, to, flags);
				break;

			case 'U':
				i = act_format_text(arg3, ch, to, flags);
				break;

/* room arguments */
			case 'r':
				i = mlstr_mval(room1->name);
				break;

			case 'R':
				i = mlstr_mval(room3->name);
				break;

/* char arguments */
			case 'n':
				i = PERS2(ch, to,
					  (sp < 0) ? (flags | ACT_FIXSH) : 0);
				break;

			case 'N':
				i = PERS2(vch, to,
					  (sp < 0) ? (flags | ACT_FIXSH) : 0);
				break;

			case 'i':
				i = PERS2(vch1, to,
					  (sp < 0) ? (flags | ACT_FIXSH) : 0);
				break;

			case 'I':
				i = PERS2(vch3, to,
					  (sp < 0) ? (flags | ACT_FIXSH) : 0);
				break;

/* numeric arguments */
			case 'j':
				snprintf(tmp, sizeof(tmp), "%d", num1);
				i = tmp;
				break;

			case 'J':
				snprintf(tmp, sizeof(tmp), "%d", num3);
				i = tmp;
				break;

/* him/her arguments. obsolete. $gx{...} should be used instead */
			case 'e':
				i = he_she[SEX(ch, to)];
				break;
	
			case 'E':
				i = he_she[SEX(vch, to)];
				break;
	
			case 'm':
				i = him_her[SEX(ch, to)];
				break;
	
			case 'M':
				i = him_her[SEX(vch, to)];
				break;
	
			case 's':
				i = his_her[SEX(ch, to)];
				break;
	
			case 'S':
				i = his_her[SEX(vch, to)];
				break;

/* obj arguments */
			case 'p':
				i = act_format_obj(obj1, to, sp, flags);
				break;

			case 'P':
				i = act_format_obj(obj2, to, sp, flags);
				break;

/* door arguments */
			case 'd':
				if (IS_NULLSTR(arg2))
					i = GETMSG("door", to->lang);
				else {
					one_argument(arg2, tmp, sizeof(tmp));
					i = tmp;
				}
				break;
/* $h{...} capitalize arguments */
			case 'h':
				if (*s != '{') {
					log_printf("act_raw: '%s': "
						"syntax error", str);
					continue;
				}
				if (++sp >= TSTACK_SZ) {
					log_printf("act_raw: '%s': "
						"tstack overflow", str);
					continue;
				}
				tstack[sp].p = point;
				tstack[sp].type = code;
				s++;

				continue;
/* $gx{...}, $cx{...}, $qx{...} arguments */
			case 'g':
			case 'c':
			case 'q':
				if (*(s+1) != '{') {
					log_printf("act_raw: '%s': "
						   "syntax error", str);
					continue;
				}

				if (++sp >= TSTACK_SZ) {
					log_printf("act_raw: '%s': "
						   "tstack overflow", str);
					continue;
				}

				tstack[sp].p = point;
				tstack[sp].type = code;
				subcode = *s++;
				s++;

				switch (code) {
				case 'c':
					tstack[sp].arg = subcode - '0';
					break;

				case 'g':
					switch (subcode) {
					case 'N':
						tstack[sp].arg = vch->sex;
						break;

					case 'n':
						tstack[sp].arg = ch->sex;
						break;

					case 'i':
						tstack[sp].arg = vch1->sex;
						break;

					case 'I':
						tstack[sp].arg = vch3->sex;
						break;

					case 't':
						tstack[sp].arg = to->sex;
						break;

					case 'p':
						tstack[sp].arg = obj1->pIndexData->gender;
						break;

					case 'P':
						tstack[sp].arg = obj2->pIndexData->gender;
						break;

					default:
						log_printf("act_raw: '%s': "
							   "bad subcode '%c'",
							   str, subcode);
						sp--;
						break;
					}
					break;

				case 'q':
					switch(subcode) {
					case 'j':
						tstack[sp].arg = num1;
						break;

					case 'J':
						tstack[sp].arg = num3;
						break;
					default:
						log_printf("act_raw: '%s': "
							   "bad subcode '%c'",
							   str, subcode);
						sp--;
						break;
					}
					break;
				}
				continue;
			}
	
			if (i) {
				while ((*point++ = *i++));
				point--;
			}
			break;
		}
	}
 
	if (!IS_SET(flags, ACT_NOLF))
		*point++ = '\n';
	*point = '\0';

/* first non-control char is uppercased */
	point = (char*) cstrfirst(buf);
	*point = UPPER(*point);

	parse_colors(buf, tmp, sizeof(tmp), OUTPUT_FORMAT(to));

	if (!IS_NPC(to)) {
		if ((IS_SET(to->comm, COMM_AFK) || to->desc == NULL) &&
		     IS_SET(flags, ACT_TOBUF))
			buf_add(to->pcdata->buffer, tmp);
		else if (to->desc) {
			if (IS_SET(to->comm, COMM_QUIET_EDITOR)
			&&  to->desc->pString)
				buf_add(to->pcdata->buffer, tmp);
			else
				write_to_buffer_muddy(to->desc, tmp, 0);
		}
	}
	else {
		if (!IS_SET(flags, ACT_NOTRIG))
			mp_act_trigger(tmp, to, ch, arg1, arg2, TRIG_ACT);
		if (to->desc)
			write_to_buffer_muddy(to->desc, tmp, 0);
	}
}

