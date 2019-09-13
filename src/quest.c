/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*-
 * Changed for ^SR^ by Xor in 2002 year.
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
 * $Id: quest.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "merc.h"
#include "quest.h"
#include "update.h"

#ifdef SUNOS
#	include <stdarg.h>
#	include "compat/compat.h"
#endif

/*
 * quest items
 */
//#define QUEST_VNUM_GIRTH	94
//#define QUEST_VNUM_RING		95
#define QUEST_VNUM_RUG		50
//#define QUEST_VNUM_SONG		40
#define QUEST_VNUM_CANTEEN	501
#define QUEST_VNUM_FIAL		502

//#define TROUBLE_MAX 3

static void quest_tell(CHAR_DATA *ch, CHAR_DATA *questor, const char *fmt, ...);
static CHAR_DATA *questor_lookup(CHAR_DATA *ch);
// qtrouble_t *qtrouble_lookup(CHAR_DATA *ch, int vnum);

static void quest_points(CHAR_DATA *ch, const char *arg);
static void quest_info(CHAR_DATA *ch, const char *arg);
static void quest_time(CHAR_DATA *ch, const char *arg);
static void quest_list(CHAR_DATA *ch, const char *arg);
static void quest_buy(CHAR_DATA *ch, const char *arg);
static void quest_request(CHAR_DATA *ch, const char *arg);
static void quest_complete(CHAR_DATA *ch, const char *arg);
static void quest_control(CHAR_DATA *ch, const char *arg);
static void do_quest_cancel(CHAR_DATA *ch, const char *arg);
// static void quest_trouble(CHAR_DATA *ch, const char *arg);

static bool quest_give_item(CHAR_DATA *ch, CHAR_DATA *questor,
			    int item_vnum, int count_max);

static bool buy_gold(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_prac(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_tattoo(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_death(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_katana(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_vampire(CHAR_DATA *ch, CHAR_DATA *questor);
static bool buy_exp(CHAR_DATA *ch, CHAR_DATA *questor);
//static bool buy_train(CHAR_DATA *ch, CHAR_DATA *questor);

enum qitem_type {
	TYPE_ITEM,
	TYPE_OTHER
};

typedef struct qitem_data QITEM_DATA;
struct qitem_data {
	char		*name;
	int		price;
	const char	*restrict_class;
	int		vnum;
	bool		(*do_buy)(CHAR_DATA *ch, CHAR_DATA *questor);
};

struct qitem_data qitem_table[] = {
	{ "small magic rug",		 750, NULL,
	   QUEST_VNUM_RUG, NULL					},

	{ "50 gold pieces",		 500, NULL,
	   0, buy_gold						},

	{ "10 practices",		 500, NULL,
	   0, buy_prac						},

	{ "expirince (400 after 40 level)", 50, NULL,
	   0, buy_exp						},
	   
	{ "tattoo of your religion",	 200, NULL,
	   0, buy_tattoo					},

	{ "Decrease number of deaths",	  50, "samurai",
	   0, buy_death						},

	{ "Katana quest",		 100, "samurai",
	   0, buy_katana					},

	{ "Vampire skill",		 100, "vampire",
	   0, buy_vampire					},

	{ "Bottomless canteen with cranberry juice", 500, NULL,
	   QUEST_VNUM_CANTEEN, NULL				},
	
	{ "Infinity fial of light", 500, NULL,
	   QUEST_VNUM_FIAL, NULL				},

	{ NULL }
};

struct qcmd_data {
	char *name;
	void (*do_fn)(CHAR_DATA *ch, const char* arg);
	int min_position;
	int extra;
};
typedef struct qcmd_data Qcmd_t;

Qcmd_t qcmd_table[] = {
	{ "points",	quest_points,	POS_DEAD,	CMD_KEEP_HIDE},
	{ "info",	quest_info,	POS_DEAD,	CMD_KEEP_HIDE},
	{ "time",	quest_time,	POS_DEAD,	CMD_KEEP_HIDE},
	{ "list",	quest_list,	POS_RESTING,	0},
	{ "buy",	quest_buy,	POS_RESTING,	0},
	{ "request",	quest_request,	POS_RESTING,	0},
	{ "complete",	quest_complete,	POS_RESTING,	0},
	{ "control",	quest_control,	POS_RESTING,	0},
	{ "cancel",	do_quest_cancel, POS_RESTING,	CMD_KEEP_HIDE},
/*	{ "trouble",	quest_trouble,	POS_RESTING,	0}, */
	{ NULL}
};

/*
 * The main quest function
 */
void do_quest(CHAR_DATA *ch, const char *argument)
{
	char cmd[MAX_INPUT_LENGTH];
//	char arg[MAX_INPUT_LENGTH];
	Qcmd_t *qcmd;

	argument = one_argument(argument, cmd, sizeof(cmd));
//	argument = one_argument(argument, arg, sizeof(arg));

	if (IS_NPC(ch)) 
		return;
	
	if (!IS_SET(ch->pcdata->otherf, OTHERF_CONFIRM_DESC)) {
		char_puts("For quest commands your description must be confirmed by immortals.\n", ch);
		return;
	}
	
	if (IS_SET(ch->pcdata->otherf, OTHERF_RENAME)) {
		char_puts("For quest commands your must choose another name.\n",
				ch);
		return;
	}

	for (qcmd = qcmd_table; qcmd->name != NULL; qcmd++)
		if (str_prefix(cmd, qcmd->name) == 0) {
			if (ch->position < qcmd->min_position) {
				char_puts("In your dreams, or what?\n", ch);
				return;
			}
			if (!IS_SET(qcmd->extra, CMD_KEEP_HIDE)
			&&  IS_SET(ch->affected_by, AFF_HIDE | AFF_FADE)) { 
				REMOVE_BIT(ch->affected_by,
					   AFF_HIDE | AFF_FADE);
				act_puts("You step out of shadows.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				act("$n steps out of shadows.",
				    ch, NULL, NULL, TO_ROOM);
			}
			qcmd->do_fn(ch, argument);
			return;
		}
		
	char_puts("QUEST COMMANDS: points info time request list buy cancel\n", ch);
	char_puts("                complete [{{expirience|noexp} [<answer on quest>]]\n", ch);
	char_puts("                control {{ excort {{begin|end} | bringend }\n", ch);
	char_puts("For more information, type: help quests.\n", ch);
}

// ch may be NULL !!!
void quest_handle_death(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int qt;

	if (ch
	&& IS_NPC(ch)
	&&  IS_SET(ch->pIndexData->act, ACT_SUMMONED)
	&&  ch->master != NULL)
		ch = ch->master;

	if (victim->hunter) {
		if (victim->hunter == ch) {
			if ((qt = ch->pcdata->quest->type) == QUESTT_CASTMOB
			|| qt == QUESTT_ASK_WH_MOB
			|| qt == QUESTT_ASK_MOB_AT
			|| qt == QUESTT_EXCORT_TO_MOB
			|| qt == QUESTT_EXCORT_TO_ROOM
			|| qt == QUESTT_BRING_TO_MOB)
			{
				act_puts("You have killed your quest mob.",
					ch, NULL, NULL, TO_CHAR, POS_DEAD);
				quest_cancel(ch);
				ch->pcdata->quest->time = -number_range(5, 10);
				return;
			}
			act_puts("You have almost completed your QUEST!\n"
				 "Return to questmaster before your time "
				 "runs out!",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			victim->hunter = NULL;
			ch->pcdata->quest->target1 = NULL;
		}
		else {
			if (ch)
				act_puts("You have killed someone's quest mob.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			
			if (ch && is_same_group(victim->hunter, ch)
			&& victim->hunter->pcdata->quest->type == QUESTT_KILLMOB)
			{
				act_puts("Your group have killed your quest mob.",
					victim->hunter, NULL, NULL, TO_CHAR, POS_DEAD);
				quest_handle_death(victim->hunter, victim);
			} else {
				ch = victim->hunter;
				act_puts("Someone has killed you quest mob.",
					 ch, NULL, NULL, TO_CHAR, POS_DEAD);
				quest_cancel(ch);
				ch->pcdata->quest->time = -number_range(5, 10);
			}
		}
	}
}

void quest_cancel(CHAR_DATA *ch)
{
//	CHAR_DATA *fch;
	int qt;

	if (IS_NPC(ch)) {
		bug("quest_cancel: called for NPC", 0);
		return;
	}
	
	if (ch->pcdata->quest == NULL)
		return;

	/*
	 * remove mob->hunter
	 */
/*	for (fch = npc_list; fch; fch = fch->next)
		if (fch->hunter == ch) {
			fch->hunter = NULL;
			break;
		}
*/
	switch ((qt = ch->pcdata->quest->type))
	{
		default:
			break;
		case QUESTT_ASK_WH_MOB:
		case QUESTT_ASK_MOB_AT:
		case QUESTT_CASTMOB:
		case QUESTT_KILLMOB:
		case QUESTT_EXCORT_TO_MOB:
		case QUESTT_EXCORT_TO_ROOM:
			if (GET_QUEST_MOB1(ch)
			&& GET_QUEST_MOB1(ch)->hunter
			&& GET_QUEST_MOB1(ch)->hunter == ch)
				GET_QUEST_MOB1(ch)->hunter = NULL;
			break;
		case QUESTT_ASK_WH_OBJ:
		case QUESTT_ASK_OBJ_AT:
		case QUESTT_BRING_TO_MOB:
		case QUESTT_BRING:
			if (GET_QUEST_OBJ1(ch)
			&& IS_SET(GET_QUEST_OBJ1(ch)->extra_flags, ITEM_QUEST)
			&& !IS_SET(GET_QUEST_OBJ1(ch)->pIndexData->extra_flags, ITEM_QUEST))
				REMOVE_BIT(GET_QUEST_OBJ1(ch)->extra_flags, ITEM_QUEST);
			break;
	}
	
	switch (qt)	// check target2
	{
		default:
			break;
		case QUESTT_EXCORT_TO_MOB:
		case QUESTT_BRING_TO_MOB:
			if (GET_QUEST_MOB2(ch)
			&& GET_QUEST_MOB2(ch)->hunter
			&& GET_QUEST_MOB2(ch)->hunter == ch)
				GET_QUEST_MOB2(ch)->hunter = NULL;
			break;
	}
	ch->pcdata->quest->type = QUESTT_NONE;	// = 0;
	ch->pcdata->quest->time = 0;
	ch->pcdata->quest->giver = 0;
	ch->pcdata->quest->target1 = NULL;
	ch->pcdata->quest->target2 = NULL;
	ch->pcdata->quest->room = NULL;
	ch->pcdata->quest->gsn = 0;
}

static void do_quest_cancel(CHAR_DATA *ch, const char *arg)
{
	if (!GET_QUEST_TYPE(ch)
	|| ch->pcdata->quest->time < 10)
	{
		char_puts("You can't cancel your quest now.\n", ch);
		return;
	}

	char_puts("You cancel your current quest.\n", ch);
	quest_cancel(ch);
	ch->pcdata->quest->time = -number_range(10, 20);
}

/*
 * Called from update_handler() by pulse_area
 */
void quest_update(void)
{
	CHAR_DATA *ch, *ch_next;

	for (ch = char_list; ch && !IS_NPC(ch); ch = ch_next) {
		ch_next = ch->next;
		
		if (ch->pcdata->quest == NULL)
			continue;

		if (ch->pcdata->quest->time < 0) {
			if (++ch->pcdata->quest->time == 0) {
				char_puts("{*You may now quest again.\n", ch);
			}
		} else if (GET_QUEST_TYPE(ch)) {
			if (--ch->pcdata->quest->time == 0) {
				char_puts("You have run out of time for your quest!\n", ch);
				quest_cancel(ch);
				ch->pcdata->quest->time = -number_range(10, 16);
			} else if (ch->pcdata->quest->time < 6) {
				char_puts("Better hurry, you're almost out of time for your quest!\n", ch);
			}
		}
	}
}

/*
void qtrouble_set(CHAR_DATA *ch, int vnum, int count)
{
	qtrouble_t *qt;

	if ((qt = qtrouble_lookup(ch, vnum)) != NULL)
		qt->count = count;
	else {
		qt = malloc(sizeof(*qt));
		if (qt == NULL)
			crush_mud();
		qt->vnum = vnum;
		qt->count = count;
		qt->next = ch->pcdata->qtrouble;
		ch->pcdata->qtrouble = qt;
	}
}
*/

/*
 * local functions
 */

static void quest_tell(CHAR_DATA *ch, CHAR_DATA *questor, const char *fmt, ...)
{
	va_list ap;
	char buf[MAX_STRING_LENGTH];

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), GETMSG(fmt, ch->lang), ap);
	va_end(ap);

	do_tell_raw(questor, ch, buf);
}

static CHAR_DATA* questor_lookup(CHAR_DATA *ch)
{
	CHAR_DATA *vch;
	CHAR_DATA *questor = NULL;

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (!IS_NPC(vch)) 
			continue;
		if (IS_SET(vch->pIndexData->act, ACT_QUESTOR)) {
			questor = vch;
			break;
		}
	}

	if (questor == NULL) {
		char_puts("You can't do that here.\n", ch);
		return NULL;
	}

	if (questor->fighting != NULL) {
		char_puts("Wait until the fighting stops.\n", ch);
		return NULL;
	}

	return questor;
}

/*
qtrouble_t *qtrouble_lookup(CHAR_DATA *ch, int vnum)
{
	qtrouble_t *qt;

	for (qt = ch->pcdata->qtrouble; qt != NULL; qt = qt->next)
		if (qt->vnum == vnum)
			return qt;

	return NULL;
}
*/

/*
 * quest do functions
 */

static void quest_points(CHAR_DATA *ch, const char* arg)
{
	char_printf(ch, "You have {W%d{x quest points.\n",
		    ch->pcdata->questpoints);
}

static char *str_location =
	"That location is in general area of {W%s{x for {W%s{x.\n";
static char *str_location_area =
	"Last knowning location is in general area of {W%s{x.\n";
static char *str_unknown_location =
	"You cann't see target.\n";

static void print_location_quest(CHAR_DATA *ch)
{
	if (ch->pcdata->quest->room)
		char_printf(ch, str_location,
			ch->pcdata->quest->room->area->name, 
			mlstr_mval(ch->pcdata->quest->room->name));
}

static void print_location_area_quest(CHAR_DATA *ch)
{
	if (ch->pcdata->quest->room)
		char_printf(ch, str_location_area,
			ch->pcdata->quest->room->area->name);
}

static void print_attempts_quest(CHAR_DATA *ch)
{
	char_printf(ch, "You have %d%s attempt(s) to answer.\n",
		ch->pcdata->quest->gsn,
		ch->pcdata->quest->gsn == QUANTITY_ATTEMPTS ?
		"(max)": str_empty);
}

static ROOM_INDEX_DATA *get_location_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	OBJ_DATA *in_obj;

	if (obj)
	{
		for (in_obj = obj; in_obj->in_obj != NULL;
			in_obj = in_obj->in_obj)
				;

		if (in_obj->carried_by
		&& can_see(ch,in_obj->carried_by)
		&& in_obj->carried_by->in_room)
			return in_obj->carried_by->in_room;
		else if (in_obj->in_room
		&& can_see_room(ch, in_obj->in_room))
			return in_obj->in_room;
	}
	return NULL;
}

static void print_location_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	ROOM_INDEX_DATA *room = get_location_obj(ch, obj);
	
	if (room)
		char_printf(ch, str_location,
			room->area->name,
			mlstr_mval(room->name));
	else
		char_puts(str_unknown_location, ch);
}

static void print_location_area_obj(CHAR_DATA *ch, OBJ_DATA *obj)
{
	ROOM_INDEX_DATA *room = get_location_obj(ch, obj);
	
	if (room)
		char_printf(ch, str_location_area,
			room->area->name);
	else
		char_puts(str_unknown_location, ch);
}

static void quest_info(CHAR_DATA *ch, const char* arg)
{
	const char *str_quest_comp = "Your quest is ALMOST complete!\n"
			"Get back to questor before your time runs out!\n";

	switch (GET_QUEST_TYPE(ch))
	{
		default:
			char_puts("Unknown quest! Bug ?!\n", ch);
			break;
		case QUESTT_NONE:
			char_puts("You aren't currently on a quest.\n", ch);
			break;
		case QUESTT_KILLMOB:
			if (ch->pcdata->quest->target1)
			{
				CHAR_DATA *questinfo = ch->pcdata->quest->target1;

				char_printf(ch, "You are on a quest to slay the dreaded {W%s{x!\n",
					    mlstr_mval(questinfo->short_descr));
				print_location_quest(ch);
			} else {
				char_puts(str_quest_comp, ch);
			}
			break;
		case QUESTT_FINDOBJ:
			if (ch->pcdata->quest->target1)
			{
				OBJ_DATA *qinfoobj = ch->pcdata->quest->target1;

				char_printf(ch, "You are on a quest to recover the fabled {W%s{x!\n",
					    qinfoobj->name);
				print_location_quest(ch);
			}
			break;
		case QUESTT_CASTMOB:
			if (ch->pcdata->quest->target1
			&&  ch->pcdata->quest->gsn) {
				CHAR_DATA *questinfo = ch->pcdata->quest->target1;

				char_printf(ch, "You are on a quest to cast '%s' for {W%s{x!\n",
					    skill_lookup(ch->pcdata->quest->gsn) ? SKILL(ch->pcdata->quest->gsn)->name : "none",
					    mlstr_mval(questinfo->short_descr));
				print_location_quest(ch);
			} else {
				char_puts(str_quest_comp, ch);
			}
			break;
		case QUESTT_ASK_WH_MOB:	// ask "where is target mob ?"
			if (ch->pcdata->quest->target1)
			{
				CHAR_DATA *mob = ch->pcdata->quest->target1;

				char_printf(ch, "You are on a quest to answer on questions:\n"
					"   '{GWhere is {W%s{z(%s) ? What name of room this place ?{x'\n",
					mlstr_cval(mob->short_descr, ch),
					mob->name);

				print_attempts_quest(ch);

				if (ch->pcdata->quest->gsn < QUANTITY_ATTEMPTS)
					print_location_area_quest(ch);
			}
			break;
		case QUESTT_ASK_MOB_AT:	// ask "what mob at target place ?"
			if (ch->pcdata->quest->target1)
			{
				CHAR_DATA *mob = ch->pcdata->quest->target1;
				
				char_printf(ch, "You are on a quest to answer on questions:\n"
					"   '{GWhat name of mob in target location?{x'\n",
					mlstr_cval(mob->short_descr, ch),
					mob->name);

				print_attempts_quest(ch);
				if (mob->in_room)
					char_printf(ch, "That location is in general area of {W%s{x for {W%s{x.\n",
						mob->in_room->area->name,
						mlstr_mval(mob->in_room->name));
			}
			break;
		case QUESTT_ASK_WH_OBJ:	// ask "where is target obj ?"
			if (ch->pcdata->quest->target1)
			{
				OBJ_DATA *obj = ch->pcdata->quest->target1;
				
				char_printf(ch, "You are on a quest to answer on questions:\n"
					"   '{GWhere is {W%s{z(%s) ? What name of room this place ?{x'\n",
					mlstr_cval(obj->short_descr, ch),
					obj->name);

				print_attempts_quest(ch);
				if (ch->pcdata->quest->gsn < QUANTITY_ATTEMPTS)
					print_location_area_obj(ch, ch->pcdata->quest->target1);
			}
			break;
		case QUESTT_ASK_OBJ_AT:	// ask "what obj at target place ?"
			if (ch->pcdata->quest->target1)
			{
				char_printf(ch, "You are on a quest to answer on questions:\n"
					"   '{GWhat name of object in target location?{x'\n");

				print_attempts_quest(ch);
				print_location_obj(ch, ch->pcdata->quest->target1);
			}
			break;
		case QUESTT_EXCORT_TO_MOB:	// target1(M) -> target2(M)
			if (ch->pcdata->quest->target1
			&& ch->pcdata->quest->target2)
			{
				CHAR_DATA *mob1 = ch->pcdata->quest->target1;
				CHAR_DATA *mob2 = ch->pcdata->quest->target2;
				
				char_printf(ch, "You are on a quest to excort {W%s{x to {W%s{x.\n",
					mlstr_cval(mob1->short_descr, ch),
					mlstr_cval(mob2->short_descr, ch));
				if (mob1->in_room)
					char_printf(ch, "Location of {W%s{x in general area of {G%s{x.\n",
						mlstr_cval(mob1->short_descr, ch),
						mob1->in_room->area->name);
				char_printf(ch, "The place of {W%s{x in: ",
					mlstr_cval(mob2->short_descr, ch));
				print_location_quest(ch);
			} else
				char_puts(str_quest_comp, ch);
			break;
		case QUESTT_EXCORT_TO_ROOM:	// target1(M) -> target2(R)
			if (ch->pcdata->quest->target1)
			{
				CHAR_DATA *mob = ch->pcdata->quest->target1;
				ROOM_INDEX_DATA *room = ch->pcdata->quest->room;
				
				char_printf(ch, "You are on a quest to excort {W%s{x to room {W%s{x.\n",
					mlstr_cval(mob->short_descr, ch),
					mlstr_cval(room->name, ch));
				if (mob->in_room)
					char_printf(ch, "Location of {W%s{x in general area of {G%s{x.\n",
						mlstr_cval(mob->short_descr, ch),
						mob->in_room->area->name);
				char_printf(ch, "The place of room '{G%s{x' in: ",
					mlstr_cval(room->name, ch));
 				print_location_area_quest(ch);
			} else {
				char_puts(str_quest_comp, ch);
			}
			break;
		case QUESTT_BRING_TO_MOB:	// target1(O) -> target2(M)
			if (ch->pcdata->quest->target1
			&& ch->pcdata->quest->target2)
			{
				OBJ_DATA *obj = ch->pcdata->quest->target1;
				CHAR_DATA *mob = ch->pcdata->quest->target2;
				
				char_printf(ch, "You are on a quest to bring {W%s{x to {W%s{x.\n",
					mlstr_cval(obj->short_descr, ch),
					mlstr_cval(mob->short_descr, ch));
				print_location_quest(ch);
				if (mob->in_room)
					char_printf(ch, "Location of {W%s{x in general area of {G%s{x.\n",
						mlstr_cval(mob->short_descr, ch),
						mob->in_room->area->name);
			} else
				char_puts(str_quest_comp, ch);
			break;
		case QUESTT_BRING:		// target1(O) -> questor
			if (ch->pcdata->quest->target1)
			{
				OBJ_DATA *obj = ch->pcdata->quest->target1;
				
				char_printf(ch, "You are on a quest to bring {W%s{x to questor.\n",
					mlstr_cval(obj->short_descr, ch));
				print_location_quest(ch);
			}
			break;
	}
}

static CHAR_DATA *find_mob_in_room(CHAR_DATA *ch, CHAR_DATA* target, bool mess)
{
	CHAR_DATA *mob;
	
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
		if (mob == target)
		{
			if (can_see(ch, mob))
				return mob;
			break;
		}
	if (mess)
		char_printf(ch, "You cann't find %s here.\n",
			mlstr_cval(target->short_descr, ch));
	return NULL;
}

static OBJ_DATA *find_obj_inv(CHAR_DATA *ch, OBJ_DATA* target, bool mess)
{
	OBJ_DATA *obj;
	
	for (obj = ch->carrying; obj; obj = obj->next_content)
		if (obj == target)
		{
			if (can_see_obj(ch, obj))
				return obj;
			break;
		}
	if (mess)
		char_printf(ch, "You can't find %s in your inventory.\n",
			mlstr_cval(target->short_descr, ch));
	return NULL;
}

DECLARE_DO_FUN(do_stand		);

static void quest_control(CHAR_DATA *ch, const char* arg)
{
	char	arg1[MAX_INPUT_LENGTH];
	int 	qt = GET_QUEST_TYPE(ch);
	CHAR_DATA *mob;

	if (qt == QUESTT_NONE)
	{
		char_puts("You must request special quest for this command.\n", ch);
		return;
	}
	arg = one_argument(arg, arg1, sizeof(arg1));

	if (arg1[0] == '\0') {
		do_quest(ch, "?");
		return;
	}

	if (!str_prefix(arg1, "excort"))
	{
		char		arg2[MAX_INPUT_LENGTH];
		
		if ((qt != QUESTT_EXCORT_TO_MOB
		&& qt != QUESTT_EXCORT_TO_ROOM)
		|| GET_QUEST_MOB1(ch) == NULL
		|| (GET_QUEST_MOB2(ch) == NULL && qt == QUESTT_EXCORT_TO_MOB))
		{
			char_puts("Not for this quest type.\n", ch);
			return;
		}

		if ((mob = find_mob_in_room(ch, ch->pcdata->quest->target1, TRUE)) == NULL)
			return;
		
		arg = one_argument(arg, arg2, sizeof(arg2));
		if (arg2[0] == '\0') {
			quest_control(ch, str_empty);
		} else if (!str_prefix(arg2, "begin"))
		{
			if (mob->master)
			{
				char_printf(ch, "%s already follow somebody.\n",
					mlstr_cval(mob->short_descr, ch));
				return;
			}
			doprintf(interpret, mob, "say %s, excort me please...",
				PERS2(ch, mob, ACT_FORMSH));
			do_stand(mob, str_empty);
			add_follower(mob, ch);
		} else if (!str_prefix(arg2, "end"))
		{
			CHAR_DATA *mob2 = NULL;
			
			if (mob->master == NULL)
			{
				char_printf(ch, "%s isn't followed you.\n",
					mlstr_cval(mob->short_descr, ch));
				return;
			}

			if ((qt == QUESTT_EXCORT_TO_MOB
				&& (mob2 = find_mob_in_room(ch, ch->pcdata->quest->target2, TRUE)))
			|| (qt == QUESTT_EXCORT_TO_ROOM
				&& ch->pcdata->quest->room == ch->in_room))
			{
				doprintf(interpret, mob, "say %s, thank you! Now return to questmaster for prize.",
					PERS2(ch, mob, ACT_FORMSH));
				if (mob->hunter
				&& mob->hunter == ch)
					mob->hunter = NULL;
				if (mob2 && mob2->hunter
				&& mob2->hunter == ch)
					mob2->hunter = NULL;
				ch->pcdata->quest->target1 = NULL;
				ch->pcdata->quest->target2 = NULL;
			} else {
				doprintf(interpret, mob, "say Ok, %s, I wait you here...",
					PERS2(ch, mob, ACT_FORMSH));
			}
			stop_follower(mob);
		} else
			quest_control(ch, str_empty);
	} else if (!str_prefix(arg1, "bringend"))
	{
		OBJ_DATA *obj;
		
		if (qt != QUESTT_BRING_TO_MOB
		|| GET_QUEST_OBJ1(ch) == NULL
		|| GET_QUEST_MOB2(ch) == NULL)
		{
			char_puts("Not for this quest type.\n", ch);
			return;
		}
		
		if ((obj = find_obj_inv(ch, ch->pcdata->quest->target1, TRUE)) == NULL)
			return;
			
		if ((mob = find_mob_in_room(ch, ch->pcdata->quest->target2, TRUE)) == NULL)
			return;

		act_puts("You hand {W$p{x to $N.",
			ch, obj, mob, TO_CHAR, POS_DEAD);
		act("$n hands {W$p{x to $N.",
			ch, obj, mob, TO_ROOM);
		doprintf(interpret, mob, "say %s, thank you!",
			PERS2(ch, mob, ACT_FORMSH));
		ch->pcdata->quest->target1 = NULL;
		ch->pcdata->quest->target2 = NULL;
		if (mob->hunter
		&& mob->hunter == ch)
			mob->hunter = NULL;
		if (!IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST))
			REMOVE_BIT(obj->extra_flags, ITEM_QUEST);
		extract_obj(obj, 0);
		do_quest(ch, "info");
	} else
		quest_control(ch, str_empty);
}

static void quest_time(CHAR_DATA *ch, const char* arg)
{
	if (!GET_QUEST_TYPE(ch)) {
		char_puts("You aren't currently on a quest.\n", ch);
		if (!ch->pcdata->quest)
			return;
		if (ch->pcdata->quest->time < -1)
			char_printf(ch, "There are {W%d{x minutes remaining until you can go on another quest.\n",
				    -ch->pcdata->quest->time);
	    	else if (ch->pcdata->quest->time == -1)
			char_puts("There is less than a minute remaining until you can go on another quest.\n", ch);
	}
	else
		char_printf(ch, "Time left for current quest: {W%d{x.\n",
			    ch->pcdata->quest->time);
}

static void quest_list(CHAR_DATA *ch, const char *arg)
{
	CHAR_DATA *questor;
	QITEM_DATA *qitem;
	class_t *cl;

	if ((questor = questor_lookup(ch)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

	act("$n asks $N for list of quest items.", ch, NULL, questor, TO_ROOM);
	act_puts("You ask $N for list of quest items.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);

	char_puts("Current Quest Items available for Purchase:\n", ch);
	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->restrict_class != NULL
		&&  !is_name(cl->name, qitem->restrict_class))
			continue;

		if (arg[0] != '\0' && !is_name(arg, qitem->name))
			continue;

		char_printf(ch, "%5dqp...........%s\n",
			    qitem->price, qitem->name);
	}
	char_puts("To buy an item, type 'QUEST BUY <item>'.\n", ch);
}

static void quest_buy(CHAR_DATA *ch, const char *arg)
{
	CHAR_DATA *questor;
	QITEM_DATA *qitem;
	class_t *cl;

	if ((questor = questor_lookup(ch)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

	if (arg[0] == '\0') {
		char_puts("To buy an item, type 'QUEST BUY <item>'.\n", ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++)
		if (is_name(arg, qitem->name)) {
			bool buy_ok = FALSE;

			if (qitem->restrict_class != NULL
			&&  !is_name(cl->name, qitem->restrict_class))
				continue;

			if (ch->pcdata->questpoints < qitem->price) {
				quest_tell(ch, questor,
					   "Sorry, {W%s{z, but you don't have "
					   "enough quest points for that.",
					   ch->name);
				return;
			}

			if (qitem->vnum == 0)
				buy_ok = qitem->do_buy(ch, questor);
			else
				buy_ok = quest_give_item(ch, questor,
						qitem->vnum, 0);

			if (buy_ok) 
				ch->pcdata->questpoints -= qitem->price;
			return;
		}

	quest_tell(ch, questor, "I do not have that item, %s.", ch->name);
}

#define MAX_QMOB_COUNT 512
#define MAX_SPELL_COUNT 128

typedef struct quest_data_t
{
	int mindiff_50, maxdiff_50;
	int mindiff, maxdiff;
} quest_data_t;

quest_data_t quest_data[QUESTT_MAX] =
{
	{ -1,	4,	0,	6	},	//QUESTT_KILLMOB
	{ 0,	0,	0,	0	},	//QUESTT_FINDOBJ
	{ 2,	6,	4,	12	},	//QUESTT_CASTMOB
	{ 2,	8,	4,	14	},	//QUESTT_ASK_WH_MOB
	{ -1,	5,	0,	6	},	//QUESTT_ASK_WH_OBJ
	{ 2,	8,	4,	14	},	//QUESTT_ASK_MOB_AT
	{ -1,	5,	0,	6	},	//QUESTT_ASK_OBJ_AT
	{ 0,	7,	1,	9	},	//QUESTT_EXCORT_TO_MOB
	{ 0,	7,	1,	9	},	//QUESTT_EXCORT_TO_ROOM
	{ -2,	5,	-1,	6	},	//QUESTT_BRING_TO_MOB
	{ -2,	4,	-1,	5	}	//QUESTT_BRING
};

static void get_quest_failed(CHAR_DATA *ch, CHAR_DATA *questor)
{
	quest_tell(ch, questor, "I'm sorry, but i don't have any quests for you at this time.");
	quest_tell(ch, questor, "Try again later.");
	quest_cancel(ch);
	ch->pcdata->quest->time = -3;
}

static CHAR_DATA * get_random_mob(CHAR_DATA *ch, CHAR_DATA *questor,
	flag32_t questt, bool vict_evil, bool vict_good, AREA_DATA *notarea,
	bool aggrr)
{
	CHAR_DATA *mobs[MAX_QMOB_COUNT];
	CHAR_DATA *victim;
	size_t mob_count;
	int mindiff, maxdiff;
	
	if (ch->level < 51)
	{
		mindiff = quest_data[questt-1].mindiff_50;
		maxdiff = quest_data[questt-1].maxdiff_50;
	} else {
		mindiff = quest_data[questt-1].mindiff;
		maxdiff = quest_data[questt-1].maxdiff;
	}

	/*
	 * find MAX_QMOB_COUNT quest mobs and store their vnums in mob_buf
	 */
	mob_count = 0;
	for (victim = npc_list; victim; victim = victim->next) {
		int diff = victim->level - ch->level;

		if (IS_EXTRACTED(victim))
		{
			bug("get_random_mob: find extracted char[vnum: %d] in ch_list.",
				IS_NPC(victim) ? victim->pIndexData->vnum : -1);
			continue;
		}
		
		if (IS_PC(victim)
		||  (diff > maxdiff || diff < mindiff)
		||  victim->pIndexData->pShop
		||  victim->race == ch->race
		||  (IS_EVIL(victim) && vict_evil == FALSE)
		||  (IS_GOOD(victim) && vict_good == FALSE)
		||  victim->pIndexData->vnum < 100
		||  IS_SET(victim->pIndexData->act,
			   ACT_TRAIN | ACT_PRACTICE | ACT_HEALER |
			   ACT_NOTRACK | ACT_PET)
		||  questor->pIndexData == victim->pIndexData
		||  victim->in_room == NULL
		||  victim->in_room->area == notarea
		||  (IS_SET(victim->pIndexData->act, ACT_SENTINEL) &&
		     IS_SET(victim->in_room->room_flags,
			    ROOM_PRIVATE | ROOM_SOLITARY))
		|| (!aggrr && IS_SET(victim->pIndexData->act, ACT_AGGRESSIVE))
		||  !str_cmp(victim->in_room->area->name,
			     hometown_name(ch->pcdata->hometown))
		||  IS_SET(victim->in_room->area->flags,
			   AREA_CLOSED | AREA_NOQUEST))
			continue;
		mobs[mob_count++] = victim;
		if (mob_count >= MAX_QMOB_COUNT)
			break;
	}

	if (mob_count == 0) {
		get_quest_failed(ch, questor);
		return NULL;
	}

	return mobs[number_range(0, mob_count-1)];
}

static OBJ_DATA *get_random_obj(CHAR_DATA *ch, CHAR_DATA *questor,
	flag32_t questt)
{
	OBJ_DATA *objs[MAX_QMOB_COUNT];
	OBJ_DATA *obj;
	size_t obj_count = 0;
	ROOM_INDEX_DATA *room;
	int mindiff, maxdiff, diff;
	
	if (ch->level < 51)
	{
		mindiff = quest_data[questt-1].mindiff_50;
		maxdiff = quest_data[questt-1].maxdiff_50;
	} else {
		mindiff = quest_data[questt-1].mindiff;
		maxdiff = quest_data[questt-1].maxdiff;
	}
		
				
	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!CAN_WEAR(obj, ITEM_TAKE)
		|| obj->owner
		|| obj->timer
		|| IS_SET(obj->hidden_flags, OHIDE_EXTRACTED)
		|| obj->water_float > 0
		|| obj->pIndexData->limit > 0
		|| ((diff = obj->level - ch->level) > maxdiff || diff < mindiff)
		|| (IS_OBJ_STAT(obj, ITEM_ANTI_EVIL) && IS_EVIL(ch))
		|| (IS_OBJ_STAT(obj, ITEM_ANTI_GOOD) && IS_GOOD(ch))
		|| (IS_OBJ_STAT(obj, ITEM_ANTI_NEUTRAL) && IS_NEUTRAL(ch))
		|| !(	   obj->pIndexData->item_type == ITEM_LIGHT
			|| obj->pIndexData->item_type == ITEM_ARMOR
			|| obj->pIndexData->item_type == ITEM_WEAPON)
		|| obj->pIndexData->vnum < 100
		|| IS_SET(obj->extra_flags, ITEM_VIS_DEATH | ITEM_QUEST)
		|| (room = get_location_obj(ch, obj)) == NULL
		|| IS_SET(room->area->flags, AREA_CLOSED | AREA_NOQUEST)
		|| (obj->carried_by && !can_see(ch, obj->carried_by))
		|| (obj->in_room && !can_see_room(ch, obj->in_room)))
			continue;

		objs[obj_count++] = obj;

		if (obj_count >= MAX_QMOB_COUNT)
			break;
	}

	if (obj_count == 0)
		return NULL;

	return objs[number_range(0, obj_count-1)];;
}

static void quest_request(CHAR_DATA *ch, const char *arg)
{
	int i, chance;
	CHAR_DATA *victim = NULL;
	CHAR_DATA *questor;
	bool vict_evil = FALSE, vict_good = FALSE;
	OBJ_DATA *obj;
	int qt = QUESTT_NONE;
	bool many_time = FALSE;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act("$n asks $N for a quest.", ch, NULL, questor, TO_ROOM);
	act_puts("You ask $N for a quest.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);

	if (GET_QUEST_TYPE(ch)) {
    		quest_tell(ch, questor, "But you are already on a quest!");
    		return;
	} 
	
	if (!ch->pcdata->quest)
		ch->pcdata->quest = new_quest_char();

	if (ch->pcdata->quest->time < 0) {
		quest_tell(ch, questor,
			   "You're very brave, {W%s{z, but let someone else "
			   "have a chance.", ch->name);
		quest_tell(ch, questor, "Come back later.");
		return;
	}

	quest_tell(ch, questor, "Thank you, brave {W%s{z!", ch->name);

	if (chance(10)) { /* Quest to cast spell on mob */
		int spells[MAX_SPELL_COUNT];
		int count;
		pcskill_t *ps;
		skill_t *sk;
		
		count = 0;
		for (i = 0; i < ch->pcdata->learned.nused; i++) {
			ps = VARR_GET(&ch->pcdata->learned, i);
			
			if (ps->level > ch->level
			|| (sk = SKILL(ps->sn))->spell == NULL
			|| !(	   sk->spell->target == TAR_CHAR_OFFENSIVE
				|| sk->spell->target == TAR_CHAR_DEFENSIVE
				|| sk->spell->target == TAR_OBJ_CHAR_OFF
				|| sk->spell->target == TAR_OBJ_CHAR_DEF)
			|| IS_SET(sk->flags, SKILL_QUESTIONABLE)
			)
				continue;

			if (sk->spell->target == TAR_CHAR_DEFENSIVE
			&& sk->spell->target == TAR_OBJ_CHAR_DEF
			&& !IS_ATTACKER(ch))	// PEACEFULL can't cast this spell on attackers (mobs)!
				continue;

			spells[count] = ps->sn;
			if (++count >= MAX_SPELL_COUNT)
				break;
		}
		
		if (count)
		{
			ch->pcdata->quest->gsn = i
				= spells[number_range(1, count) - 1];
			if ((sk = SKILL(i))->spell->target == TAR_CHAR_DEFENSIVE
			|| sk->spell->target == TAR_OBJ_CHAR_DEF) {
				vict_evil = (IS_EVIL(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
				vict_good = (IS_GOOD(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
			}
			qt = ch->pcdata->quest->type = QUESTT_CASTMOB;
		};
	}
	
	if (!qt
	&& chance(20)
	&& (obj = get_random_obj(ch, questor,
		(qt = ch->pcdata->quest->type = number_bits(1) ?
		(number_bits(1) ? QUESTT_BRING_TO_MOB : QUESTT_BRING)
		: (number_bits(1) ? QUESTT_ASK_WH_OBJ : QUESTT_ASK_OBJ_AT))))
	)
	{
		ROOM_INDEX_DATA *room;

		ch->pcdata->quest->target1 = obj;
		SET_BIT(obj->extra_flags, ITEM_QUEST);
		room = ch->pcdata->quest->room = get_location_obj(ch, obj);

		if (room == NULL)
		{
			get_quest_failed(ch, questor);
			return;
		}

		switch(qt)
		{
		    case QUESTT_ASK_WH_OBJ:
			ch->pcdata->quest->gsn = QUANTITY_ATTEMPTS;
			quest_tell(ch, questor,
				"I lose sight of {W%s{z and be thankful "
				"if you answer where is it.",
				mlstr_cval(obj->short_descr, ch));
			quest_tell(ch, questor, "After your first error answer, "
				"I will try to remember name of area which contain searching obj.");
			break;
		    case QUESTT_ASK_OBJ_AT:
		    	ch->pcdata->quest->gsn = QUANTITY_ATTEMPTS;
			quest_tell(ch, questor,
				"My court wizardess, with her magic mirror, "
				"has finded new item, which no wtitten in her scrolls.");
			quest_tell(ch, questor,
				"I and she will be gratefull if you tell me that name "
				"of this item.");
			quest_tell(ch, questor,
				"Look in the general area of {W%s{z for {W%s{z!",
				room->area->name,
				mlstr_cval(room->name, ch));
			break;
		    case QUESTT_BRING_TO_MOB: /* Quest to bring r_obj to r_mob */
			if ((victim = get_random_mob(ch, questor, qt,
			IS_EVIL(ch) || IS_NEUTRAL(ch),
			IS_GOOD(ch) || IS_NEUTRAL(ch),
			room->area, FALSE)) == NULL)
				return;
			ch->pcdata->quest->target2 = victim;
			victim->hunter = ch;
			quest_tell(ch, questor,
				"My court friend {W%s{z lost %s {W%s{z "
				"and be gratefull if you bring this item %s.",
				mlstr_cval(victim->short_descr, ch),
				victim->sex == SEX_MALE ? "his" : "her",
				mlstr_cval(obj->short_descr, ch),
				victim->sex == SEX_MALE ? "him" : "her");
			quest_tell(ch, questor,
				"Look item in the general area of {W%s{z for {W%s{z!",
				room->area->name,
				mlstr_cval(room->name, ch));
			quest_tell(ch, questor,
				"Location of {W%s{x in general area of {G%s{x.",
				mlstr_cval(victim->short_descr, ch),
				victim->in_room->area->name);
			break;
		    case QUESTT_BRING:
			quest_tell(ch, questor,
				"I lost manuscript with identify of {W%s{z "
				"and be gratefull if you bring this item me.",
				mlstr_cval(obj->short_descr, ch));
			quest_tell(ch, questor,
				"Look in the general area of {W%s{z for {W%s{z!",
				room->area->name,
				mlstr_cval(room->name, ch));
			break;
		}
		goto last_steps;
	}

	if (!qt)
	{
		chance = number_range(1, 100);

		if (chance < 40) {
			qt = ch->pcdata->quest->type = QUESTT_KILLMOB;
			vict_evil = (IS_GOOD(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
			vict_good = (IS_EVIL(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
		} else if (chance < 75) {
			qt = ch->pcdata->quest->type = QUESTT_FINDOBJ;
			vict_evil = (IS_GOOD(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
			vict_good = (IS_EVIL(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
		} else if (chance < 90) {
			CHAR_DATA *victim2;
			
			qt = ch->pcdata->quest->type = number_bits(1) ?
				QUESTT_EXCORT_TO_MOB : QUESTT_EXCORT_TO_ROOM;

			vict_evil = (IS_EVIL(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
			vict_good = (IS_GOOD(ch) || IS_NEUTRAL(ch)) ? TRUE : FALSE;
			
			if ((victim = get_random_mob(ch, questor, qt,
			vict_evil, vict_good, NULL, FALSE)) == NULL)
				return;
			ch->pcdata->quest->target1 = victim;
			
			if ((victim2 = get_random_mob(ch, questor, qt,
					vict_evil, vict_good,
					victim->in_room->area, FALSE)) == NULL)
				return;
			quest_tell(ch, questor,
				"My friend lose one's way in general area of {W%s{z "
				"and be gratefull if you help %s.",
				victim->in_room->area->name,
				victim->sex == SEX_MALE ? "him" : "her");
			quest_tell(ch, questor,
				"{W%s{z is needed to excort in general area of {W%s{z.",
				mlstr_cval(victim->short_descr, ch),
				victim2->in_room->area->name);

			ch->pcdata->quest->room = victim2->in_room;
			victim->hunter = ch;

			if (qt == QUESTT_EXCORT_TO_MOB)
			{
				quest_tell(ch, questor,
					"In this area find {W%s{z and excort my friend to %s.",
						mlstr_cval(victim2->short_descr, ch),
						victim2->sex == SEX_MALE ? "him" : "her");
				ch->pcdata->quest->target2 = victim2;
				victim2->hunter = ch;
			} else
				quest_tell(ch, questor,
					"In this area find room '{W%s{z' and excort my friend to this place.",
					mlstr_cval(victim2->in_room->name, ch));
			many_time = TRUE;
			goto last_steps;
		} else {
			qt = ch->pcdata->quest->type = number_bits(1) ?
				QUESTT_ASK_WH_MOB : QUESTT_ASK_MOB_AT;
		
			vict_evil = vict_good = TRUE;
			ch->pcdata->quest->gsn = QUANTITY_ATTEMPTS;
		}
	}

	if ((victim = get_random_mob(ch, questor, qt,
	vict_evil, vict_good, NULL, TRUE)) == NULL)
		return;

	ch->pcdata->quest->room = victim->in_room;
	ch->pcdata->quest->target1 = victim;

	switch (qt)
	{
		case QUESTT_ASK_WH_MOB:	/* Quest to answer where mob */
		   {
			const char *h = victim->sex == SEX_MALE ? "he" : "she";

			quest_tell(ch, questor, "I lose sight of {W%s{z and be thankful "
						"if you answer where is %s.",
				mlstr_cval(victim->short_descr, ch),
				h);
			quest_tell(ch, questor, "After your first error answer, "
						"I will try to remember name of area which contain searching mob.");
			victim->hunter = ch;
		   }
		   break;
		case QUESTT_ASK_MOB_AT: /* Quest to answer what mob */
		   {
		   	/* "До меня поступили сведения, что там-то кто-то наводит террор
		   	 * " на местных жителей и я буду благодарен если ты сообщишь кто"
		   	 * "это."
		   	 */
		   	quest_tell(ch, questor, "In the general area of {W%s{z for {W%s{z "
		   				"somebody terror local citizen, "
		   				"and I be thankful if you answer "
		   				"who do it.",
		   				victim->in_room->area->name,
		   				mlstr_cval(victim->in_room->name, ch));
		   	victim->hunter = ch;
		   }
		   break;
		case QUESTT_CASTMOB:
		   {
			/* Quest to cast spell on mob */
			const char *h = victim->sex == SEX_MALE ? "him" : "her";  

			switch (SKILL(ch->pcdata->quest->gsn)->spell->target)
			{
				default:
					quest_tell(ch, questor,
						"{W%s{z has commited terrible crime and court of law decreed to cast '{w%s{z' for %s.",
						mlstr_cval(victim->short_descr, ch),
						SKILL(ch->pcdata->quest->gsn)->name,
						h);
					break;
				case TAR_OBJ_CHAR_DEF:
				case TAR_CHAR_DEFENSIVE:
					quest_tell(ch, questor,
						"{W%s{z got into trouble and for help need to cast '{w%s{z' for %s",
						mlstr_cval(victim->short_descr, ch),
						SKILL(ch->pcdata->quest->gsn)->name,
						h);
					break;
			}
			victim->hunter = ch;
		   }
		   break;
		case QUESTT_FINDOBJ:	/* Quest to find an obj */
		   {
			OBJ_DATA *eyed;
			int obj_vnum;

			if (IS_GOOD(ch))
				i = 0;
			else if (IS_EVIL(ch))
				i = 2;
			else
				i = 1;

			obj_vnum = number_range(QUEST_OBJ_FIRST, QUEST_OBJ_LAST);
			eyed = create_obj(get_obj_index(obj_vnum), 0);
			eyed->level = ch->level;
			eyed->owner = mlstr_dup(ch->short_descr);
			eyed->ed = ed_new2(eyed->pIndexData->ed, ch->name);
			eyed->cost = 0;
			eyed->timer = 30;

			obj_to_room(eyed, victim->in_room); // see bellow advanced puts ;)
			ch->pcdata->quest->target1 = eyed;

			quest_tell(ch, questor,
				"Vile pilferers have stolen {W%s{z "
				"from the royal treasury!",
				mlstr_mval(eyed->short_descr));
			quest_tell(ch, questor,
				"My court wizardess, with her magic mirror, "
				"has pinpointed its location.");
			quest_tell(ch, questor,
				"Look in the general area of {W%s{z for {W%s{z!",
				victim->in_room->area->name,
				mlstr_mval(victim->in_room->name));

			switch(number_bits(2))
			{
				default:	// only puts in room
					break;
				case 0:
					SET_BIT(eyed->hidden_flags, OHIDE_HIDDEN);
					break;
				case 1:
					SET_BIT(eyed->hidden_flags, OHIDE_BURYED);
					break;
			}
		   }
		   break;
		case QUESTT_KILLMOB:	/* Quest to kill a mob */
		   {
			if (IS_GOOD(ch)) {
				quest_tell(ch, questor,
					"Rune's most heinous criminal, {W%s{z, "
					"has escaped from the dungeon.",
					mlstr_mval(victim->short_descr));
				quest_tell(ch, questor,
					"Since the escape, {W%s{z has murdered {W%d{z civilians!",
					mlstr_mval(victim->short_descr),
					number_range(2, 20));
				quest_tell(ch, questor, "The penalty for this crime is death, and you are to deliver the sentence!");
			} else {
				quest_tell(ch, questor,
					"An enemy of mine, {W%s{z, "
					"is making vile threats against the crown.",
					mlstr_mval(victim->short_descr));
				quest_tell(ch, questor,
					"This threat must be eliminated!");
			}
			victim->hunter = ch;
		   }
		   break;
	}

	if (qt == QUESTT_CASTMOB
	|| qt == QUESTT_KILLMOB)
	{
		quest_tell(ch, questor,
			   "Seek {W%s{z out in the vicinity of {W%s{z!",
			   mlstr_mval(victim->short_descr),
			   mlstr_mval(victim->in_room->name));
		quest_tell(ch, questor,
			   "That location is in general area of {W%s{z.",
			   victim->in_room->area->name);
	}

last_steps:

	ch->pcdata->quest->giver = questor->pIndexData->vnum;
	ch->pcdata->quest->time = number_range(10, 20) + ch->level/10
		+ (many_time ? 20 : 0)
		+ (qt == QUESTT_BRING_TO_MOB ? 15 : 0);
	quest_tell(ch, questor,
		   "You have {W%d{z minutes to complete this quest.", 
		   ch->pcdata->quest->time);
	quest_tell(ch, questor, "May the gods go with you!");
}

static bool check_fail_ask_quest(CHAR_DATA *ch, CHAR_DATA *questor)
{
	if (--ch->pcdata->quest->gsn > 0) {
		quest_tell(ch, questor,
			"Incorrect answer. You have %d attempt(s).",
			ch->pcdata->quest->gsn);
		return FALSE;
	}
	
	quest_tell(ch, questor,
		"You used last attempt and failed quest.");
	quest_cancel(ch);
	ch->pcdata->quest->time = -number_range(6, 12);
	return TRUE;
}

static bool check_empty_arg(CHAR_DATA *ch, CHAR_DATA *questor, const char *arg)
{
	if (arg[0] == '\0')
	{
		quest_tell(ch, questor, "You must say your answer (quest complete <answer>)");
		return TRUE;
	}
	return FALSE;
}

static ROOM_INDEX_DATA *get_true_location_obj1(CHAR_DATA *ch)
{
	OBJ_DATA *in_obj;

	if (ch->pcdata->quest->target1 == NULL)
		return NULL;

	for (in_obj = ch->pcdata->quest->target1; in_obj->in_obj != NULL;
		in_obj = in_obj->in_obj)
			;

	if (in_obj->carried_by
	&& in_obj->carried_by->in_room)
		return in_obj->carried_by->in_room;
	else if (in_obj->in_room)
		return in_obj->in_room;
	
	return NULL;
}

static void quest_complete(CHAR_DATA *ch, const char *arg)
{
	bool complete = FALSE;
	bool exp;
	CHAR_DATA *questor;
	OBJ_DATA *obj;
//	OBJ_DATA *obj_next;

	int silver_reward = 0;
	int qp_reward = 0;
	int prac_reward = 0;

	if ((questor = questor_lookup(ch)) == NULL)
		return;

	act("$n informs $N $e has completed $s quest.",
	    ch, NULL, questor, TO_ROOM);
	act_puts("You inform $N you have completed your quest.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);

	if (!GET_QUEST_TYPE(ch)) {
		quest_tell(ch, questor, "You have to REQUEST a quest first, {W%s{z.",
			   ch->name); 
		return;
	}

	if (ch->pcdata->quest->giver != questor->pIndexData->vnum) {
		quest_tell(ch, questor,
			   "I never sent you on a quest! Perhaps you're "
			   "thinking of someone else.");
		return;
	}
	
	if (arg[0] != '\0') {
		char exp_str[MAX_INPUT_LENGTH];
		
		arg = one_argument(arg, exp_str, sizeof(exp_str));
		exp = !str_prefix(exp_str, "expirience");
	} else
		exp = FALSE;
	
	switch (ch->pcdata->quest->type)
	{
	   case QUESTT_EXCORT_TO_MOB:
	   case QUESTT_EXCORT_TO_ROOM:
	   case QUESTT_BRING_TO_MOB:
	     if (ch->pcdata->quest->target1 == NULL)
	     {
		quest_tell(ch, questor, "Good job. Thank you!");
		if (chance(5))
			prac_reward = number_range(1, 3);
		qp_reward = number_range(30, 60);
		silver_reward = 15 + number_range(5, 5 + ch->level / 8);
		complete = TRUE;
	     }
	     break;
	   case QUESTT_BRING:
	     if (ch->pcdata->quest->target1)
	     {
		if ((obj = find_obj_inv(ch, ch->pcdata->quest->target1, TRUE)))
		{
			act_puts("You hand {W$p{x to $N.",
				 ch, obj, questor, TO_CHAR, POS_DEAD);
			act("$n hands {W$p{x to $N.",
			    ch, obj, questor, TO_ROOM);
			if (!IS_SET(obj->pIndexData->extra_flags, ITEM_QUEST))
				REMOVE_BIT(obj->extra_flags, ITEM_QUEST);
			extract_obj(obj, 0);

			if (chance(2))
				prac_reward = number_range(1, 3);
			qp_reward = number_range(15, 35);
			silver_reward = 3 + number_range(ch->level / 10, ch->level / 5)
					+ obj->cost * 0.8;
			complete = TRUE;
			break;
		}
	     }
	     break;
	   case QUESTT_ASK_OBJ_AT:
	     {
		if (check_empty_arg(ch, questor, arg))
			return;
		
		if ((obj = ch->pcdata->quest->target1) == NULL)
			return;

		if (!str_cmp(arg, mlstr_mval(obj->short_descr))
		|| !strcmp(arg, mlstr_cval(obj->short_descr, ch))
		|| !str_cmp(arg, cutcolor(mlstr_mval(obj->short_descr)))
		/* || is_name(arg, obj->name) */ )
		{
			quest_tell(ch, questor, "Ok. Now I inform my court wizardess about %s. Thank you!",
				mlstr_cval(obj->short_descr, ch));
			qp_reward = number_range(10, 20 + (ch->pcdata->quest->gsn - 1) * 5);
			silver_reward = 10 + number_range(5, 5 + ch->level / 9);
			complete = TRUE;
		} else {
			check_fail_ask_quest(ch, questor);
			return;
		}
	     }
	     break;
	   case QUESTT_ASK_WH_OBJ:
	     {
		ROOM_INDEX_DATA *room;

		if (check_empty_arg(ch, questor, arg))
			return;

		if ((room = get_true_location_obj1(ch)) == NULL)
			return;

		if (!str_cmp(arg, mlstr_mval(room->name))
		|| !strcmp(arg, mlstr_cval(room->name, ch))
		|| !str_cmp(arg, cutcolor(mlstr_mval(room->name))))
		{
			quest_tell(ch, questor, "Ohh... Yes, now I can find %s. Thank you!",
				mlstr_cval(GET_QUEST_OBJ1(ch)->short_descr, ch));
			qp_reward = number_range(10, 25 + (ch->pcdata->quest->gsn - 1) * 5);
			silver_reward = 10 + number_range(5, 5 + ch->level / 8);
			complete = TRUE;
		} else {
			check_fail_ask_quest(ch, questor);
			return;
		}
	     }
	     break;
	   case QUESTT_ASK_WH_MOB:
	     {
		if (check_empty_arg(ch, questor, arg))
			return;

		if (ch->pcdata->quest->target1 && GET_QUEST_MOB1(ch)->in_room && (
			!str_cmp(arg, mlstr_mval(GET_QUEST_MOB1(ch)->in_room->name))
			|| !strcmp(arg, mlstr_cval(GET_QUEST_MOB1(ch)->in_room->name, ch))
			|| !str_cmp(arg, cutcolor(mlstr_mval(GET_QUEST_MOB1(ch)->in_room->name)))))
		{
			quest_tell(ch, questor, "Ohh... Yes, now I can find %s. Thank you!",
				mlstr_cval(GET_QUEST_MOB1(ch)->short_descr, ch));
			qp_reward = number_range(10, 20 + (ch->pcdata->quest->gsn - 1) * 5);
			silver_reward = 8 + number_range(5, 5 + ch->level / 10);
			complete = TRUE;
		} else {
			check_fail_ask_quest(ch, questor);
			return;
		}
	     }
	     break;
	   case QUESTT_ASK_MOB_AT:
	     {
		if (check_empty_arg(ch, questor, arg))
			return;

		if (!str_cmp(arg, mlstr_mval(GET_QUEST_MOB1(ch)->short_descr))
		|| !strcmp(arg, mlstr_cval(GET_QUEST_MOB1(ch)->short_descr, ch))
		|| !str_cmp(arg, cutcolor(mlstr_mval(GET_QUEST_MOB1(ch)->short_descr)))
		/* || is_name(arg, GET_QUEST_MOB1(ch)->name) */ )
		{
			quest_tell(ch, questor, "Ok. Now I inform rulers about %s. Thank you!",
				mlstr_cval(GET_QUEST_MOB1(ch)->short_descr, ch));
			qp_reward = number_range(10, 15 + (ch->pcdata->quest->gsn - 1) * 5);
			silver_reward = 10 + number_range(1, 1 + ch->level / 9);
			complete = TRUE;
		} else {
			check_fail_ask_quest(ch, questor);
			return;
		}
	     }
	     break;
	   case QUESTT_FINDOBJ:
	     if (ch->pcdata->quest->target1)
	     {
		if ((obj = find_obj_inv(ch, ch->pcdata->quest->target1, TRUE)))
		{
			act_puts("You hand {W$p{x to $N.",
				 ch, obj, questor, TO_CHAR, POS_DEAD);
			act("$n hands {W$p{x to $N.",
			    ch, obj, questor, TO_ROOM);
			extract_obj(obj, 0);

			if (chance(5))
				prac_reward = number_range(1, 3);
			qp_reward = number_range(15, 35);
			silver_reward = 7 + number_range(ch->level / 10, ch->level / 5);

			complete = TRUE;
			break;
		}
	     }
	     break;
	   case QUESTT_KILLMOB:
	     if (ch->pcdata->quest->target1 == NULL)
	     {
		if (chance(5))
			prac_reward = number_range( 1, 3);
			
		qp_reward = number_range(10, 40);
		silver_reward = 5 + dice(ch->level/10 + 1, 2);
		complete = TRUE;
	     }
	     break;
	   case QUESTT_CASTMOB:
	     if (ch->pcdata->quest->target1 == NULL)
	     {
		if (chance(1))
			prac_reward = 1;
		qp_reward = number_range(10, 25);
		silver_reward = 5 + number_range(5, 5 + ch->level / 10);
		complete = TRUE;
	     }
	     break;
	}

	if (!complete) {
		quest_tell(ch, questor,
			   "You haven't completed the quest yet, but there is "
			   "still time!");
		return;
	}

	quest_tell(ch, questor, "Congratulations on completing your quest!");
	if (!IS_ATTACKER(ch)) {
		if (exp)
			char_puts("Sorry, but convert quest points into expirience only for atackers.\n", ch);
		qp_reward = UMAX(1, qp_reward * 0.75);
		quest_tell(ch, questor,
			"As a reward, I am giving you %d quest points.",
			qp_reward);
		ch->pcdata->questpoints += qp_reward;
		quest_tell(ch, questor,
			"For get more quest points and money you must become ATTACKER.");
	} else {
		ch->silver += silver_reward;
		if (exp)
		{
			int exp_gain = qp_reward * URANGE(6, ch->level / 3, 12);
			quest_tell(ch, questor,
				"As a reward, I am giving you %d silver. "
				"I am glad that you think, that best prize is expirience.",
					silver_reward);
			gain_exp(ch, exp_gain, FALSE, TRUE);
		} else {
			ch->pcdata->questpoints += qp_reward;
			quest_tell(ch, questor,
				"As a reward, I am giving you %d quest points, "
				"and %d silver.",
				qp_reward, silver_reward);
		}

		if (prac_reward) {
			ch->pcdata->practice += prac_reward;
			quest_tell(ch, questor,
				   "You gain {C%d{G practices!\n", prac_reward);
		}
	}
	

	quest_cancel(ch);
	ch->pcdata->quest->time = -number_range(12, 20);
}

/*
static void quest_trouble(CHAR_DATA *ch, const char *arg)
{
	CHAR_DATA *questor;
	QITEM_DATA *qitem;
	class_t *cl;

	if ((questor = questor_lookup(ch)) == NULL
	||  (cl = class_lookup(ch->class)) == NULL)
		return;

	if (arg[0] == '\0') {
		char_puts("To correct a quest award's trouble, type: 'quest trouble <award>'.\n", ch);
		return;
	}

	for (qitem = qitem_table; qitem->name; qitem++) {
		if (qitem->restrict_class != NULL
		&&  !is_name(cl->name, qitem->restrict_class))
			continue;

		if (qitem->vnum && is_name(arg, qitem->name)) {
			quest_give_item(ch, questor, qitem->vnum, TROUBLE_MAX);
			return;
		}
	}

	quest_tell(ch, questor,
		   "Sorry, {W%s{z, but you haven't bought "
		   "that quest award, yet.\n",
		   ch->name);
}
*/

/*
 * quest buy functions
 */

static bool quest_give_item(CHAR_DATA *ch, CHAR_DATA *questor,
			    int item_vnum, int count_max)
{
	OBJ_DATA *reward;
//	qtrouble_t *qt;
	OBJ_INDEX_DATA *pObjIndex = get_obj_index(item_vnum);

	/*
	 * check quest trouble data
	 */

#if 0
	qt = qtrouble_lookup(ch, item_vnum);
	if (count_max) {
		/*
		 * 'quest trouble'
		 */

		if ((qt && qt->count > count_max)
		||  !IS_SET(pObjIndex->extra_flags, ITEM_QUEST)) {

			/* ch requested this item too many times	*
			 * or the item is not quest			*/

			quest_tell(ch, questor,
				   "This item is beyond the trouble option.");
			return FALSE;
		}
		else if (!qt) {
			/* ch has never bought this item, but requested it */
			quest_tell(ch, questor,
				   "Sorry, {W%s{z, but you haven't bought "
				   "that quest award, yet.\n",
				   ch->name);
			return FALSE;
		}
	}
	else {
		/*
		 * 'quest buy'
		 */

		if (qt && qt->count <= TROUBLE_MAX) {
			quest_tell(ch, questor,
				   "You have already bought this item.");
			return FALSE;
		}
	}
#endif

	reward = create_obj(pObjIndex, 0);
	if (get_wear_level(ch, reward) < reward->level) {
		quest_tell(ch, questor,
			   "This item is too powerful for you.\n");
		extract_obj(reward, 0);
		return FALSE;
	}

	/* update quest trouble data */

#if 0
	if (qt && count_max) {
		OBJ_DATA *obj;
		OBJ_DATA *obj_next;

		/* `quest trouble' */
		for (obj = object_list; obj != NULL; obj = obj_next) {
			obj_next = obj->next;
			if (obj->pIndexData->vnum == item_vnum 
			&&  IS_OWNER(ch, obj)) {
				extract_obj(obj, 0);
				break;
			}
		}

		quest_tell(ch, questor,
			   "This is the %i time that I am giving "
			   "that award back.",
			   qt->count);
		if (qt->count > count_max) 
			quest_tell(ch, questor,
				   "And I won't give you that again, "
				   "with trouble option.\n");
	}

	if (!qt && IS_SET(pObjIndex->extra_flags, ITEM_QUEST)) {
		qt = malloc(sizeof(*qt));
		if (qt == NULL)
			crush_mud();
		qt->vnum = item_vnum;
		qt->count = 0;
		qt->next = ch->pcdata->qtrouble;
		ch->pcdata->qtrouble = qt;
	}

	if (qt) {
		if (count_max)
			qt->count++;
		else
			qt->count = 1;
	}
#endif

	/* ok, give him requested item */

	if (IS_SET(pObjIndex->extra_flags, ITEM_QUEST)) {
		reward->owner = mlstr_dup(ch->short_descr);
		mlstr_free(reward->short_descr);
		reward->short_descr =
			mlstr_printf(reward->pIndexData->short_descr,
				     IS_GOOD(ch) ?	"holy" :
				     IS_NEUTRAL(ch) ?	"blue-green" : 
							"evil", 
				     ch->name);
	}

	obj_to_char(reward, ch);

	act("$N gives {W$p{x to $n.", ch, reward, questor, TO_ROOM);
	act_puts("$N gives you {W$p{x.",
		 ch, reward, questor, TO_CHAR, POS_DEAD);

	return TRUE;
}

static bool buy_gold(CHAR_DATA *ch, CHAR_DATA *questor)
{
	ch->pcdata->bank_g += 50;
	act("$N gives {Y50{x gold pieces to $n.", ch, NULL, questor, TO_ROOM);
	act("$N transfers {Y50{x gold pieces to your bank account.\n",ch, NULL, questor, TO_CHAR);
	return TRUE;
}

static bool buy_prac(CHAR_DATA *ch, CHAR_DATA *questor)
{

	ch->pcdata->practice += 10;
	act("$N gives 10 practices to $n.", ch, NULL, questor, TO_ROOM);
	act_puts("$N gives you 10 practices.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	return TRUE;
}

static bool buy_exp(CHAR_DATA *ch, CHAR_DATA *questor)
{
	act("$N convert some $n's quest points into expirience for $m.",
			ch, NULL, questor, TO_ROOM); 
	act_puts("$N convert some your quest points into expirience for you.",
			ch, NULL, questor, TO_CHAR, POS_DEAD);

	return gain_exp(ch, 40 * URANGE(5, ch->level / 4, 10), FALSE, TRUE);
}

/*static bool buy_train(CHAR_DATA *ch, CHAR_DATA *questor)
{
	ch->train++;
	act("$N gives 1 train to $n.", ch, NULL, questor, TO_ROOM);
	act_puts("$N gives you 1 train.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	return TRUE;
}
*/

static bool buy_tattoo(CHAR_DATA *ch, CHAR_DATA *questor)
{
	OBJ_DATA *tattoo;

	if (!GET_CHAR_RELIGION(ch)) {
		char_puts("You don't have a religion to have a tattoo.\n", ch);
		return FALSE;
	}

	tattoo = get_eq_char(ch, WEAR_TATTOO);
	if (tattoo != NULL) {
		char_puts("But you have already your tattoo!\n", ch);
		return FALSE;
	}

	tattoo = create_obj(get_obj_index(GET_CHAR_RELIGION(ch)->vnum_tattoo), 0);

	obj_to_char(tattoo, ch);
	equip_char(ch, tattoo, WEAR_TATTOO);
	act("$N tattoos $n with {W$p{x!", ch, tattoo, questor, TO_ROOM);
	act_puts("$N tattoos you with {W$p{x!",
		 ch, tattoo, questor, TO_CHAR, POS_DEAD);
	return TRUE;
}

static bool buy_death(CHAR_DATA *ch, CHAR_DATA *questor)
{
	if (ch->pcdata->death < 1) {
		quest_tell(ch, questor, 
			   "Sorry, {W%s{z, but you haven't got any deaths yet.",
			   ch->name);
		return FALSE;
	}

	ch->pcdata->death -= 1;
	return TRUE;
}

static bool buy_katana(CHAR_DATA *ch, CHAR_DATA *questor)
{
	AFFECT_DATA af;
	OBJ_DATA *katana;

	if ((katana = get_obj_list(ch, "katana", ch->carrying)) == NULL) {
		quest_tell(ch, questor, "Sorry, {W%s{z, but you don't have your katana with you.",
			   ch->name);
		return FALSE;
	}

	af.where	= TO_WEAPON;
	af.type 	= gsn_katana;
	af.level	= 100;
	af.duration	= -1;
	af.modifier	= 0;
	af.bitvector	= WEAPON_KATANA;
	af.location	= APPLY_NONE;
	affect_to_obj(katana, &af);
	quest_tell(ch, questor, "As you wield it, you will feel that its power will increase continuosly.");
	return TRUE;
}

static bool buy_vampire(CHAR_DATA *ch, CHAR_DATA *questor)
{
	pcskill_t *ps = pcskill_lookup(ch, gsn_vampire);

	if (ps && ps->percent >= 100)
	{
		char_puts("You already can be vampire.\n", ch);
		return FALSE;
	}

	set_skill(ch, gsn_vampire, 100, 10, 100, 50);
	act("$N gives secret of undead to $n.", ch, NULL, questor, TO_ROOM);
	act_puts("$N gives you SECRET of undead.",
		 ch, NULL, questor, TO_CHAR, POS_DEAD);
	act("Lightning flashes in the sky.", ch, NULL, NULL, TO_ALL);
	return TRUE;
}
