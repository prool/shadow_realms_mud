/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*-
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
 * $Id: olc.c,v 1.33 2007/01/27 22:52:13 rem Exp $
 */

/***************************************************************************
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was freely distributed with the The Isles 1.1 source code,   *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "lang.h"

/*
 * The version info.  Please use this info when reporting bugs.
 * It is displayed in the game by typing 'version' while editing.
 * Do not remove these from the code - by request of Jason Dinkel
 */
#define VERSION	"ILAB Online Creation [Beta 1.0, ROM 2.3 modified]\n" \
		"     Port a ROM 2.4 v1.7 [-> ROM 2.4b4a]\n" \
		"     Modified for use with SoG" \
		"     Modified for SR [v1.1]\n"

#define AUTHOR	"     By Jason(jdinkel@mines.colorado.edu)\n" \
                "     By Hans Birkeland (hansbi@ifi.uio.no)\n" \
                "     Por Ivan Toledo (pvillanu@choapa.cic.userena.cl)\n" \
		"     Farmer Joe (fjoe@iclub.nsu.ru)\n" \
		"     Xorader (xorader@mail.ru)\n"

#define DATE	"     (Apr. 7, 1995 - ROM mod, Apr 16, 1995)\n" \
		"     (Port a ROM 2.4 - Nov 2, 1996)\n" \
		"     Version actual : 1.71 - Mar 22, 1998\n" \
		"     SR update: v1.1 - Feb 2002\n"

#define CREDITS "     Original by Surreality(cxw197@psu.edu) and Locke(locke@lm.com)"

const char ED_AREA[]	= "area";
const char ED_ROOM[]	= "room";
const char ED_OBJ[]	= "object";
const char ED_MOB[]	= "mobile";
const char ED_MPCODE[]	= "mpcode";
const char ED_HELP[]	= "help";
//const char ED_CLAN[]	= "clan";
const char ED_MSG[]	= "msgdb";
const char ED_LANG[]	= "language";
const char ED_IMPL[]	= "implicit";
const char ED_EXPL[]	= "explicit";
const char ED_SOC[]	= "social";
const char ED_REL[]	= "religion";
const char ED_SKILL[]	= "skill";
/*const char ED_CLASS[]	= "class";*/

olced_t olced_table[] = {
	{ ED_AREA,	"AreaEd",	olc_cmds_area	},
	{ ED_ROOM,	"RoomEd",	olc_cmds_room	},
	{ ED_OBJ,	"ObjEd",	olc_cmds_obj	},
	{ ED_MOB,	"MobEd",	olc_cmds_mob	},
	{ ED_MPCODE,	"MPEd",		olc_cmds_mpcode	},
	{ ED_HELP,	"HelpEd",	olc_cmds_help	},
	{ ED_MSG,	"MsgEd",	olc_cmds_msg	},
//	{ ED_CLAN,	"ClanEd",	olc_cmds_clan	},
	{ ED_LANG,	"LangEd",	olc_cmds_lang	},
	{ ED_IMPL,	"ImplRuleEd",	olc_cmds_impl	},
	{ ED_EXPL,	"ExplRuleEd",	olc_cmds_expl	},
	{ ED_SOC,	"SocEd",	olc_cmds_soc	},
	{ ED_REL,	"RelEd",	olc_cmds_rel	},
	{ ED_SKILL,	"SkillEd",	olc_cmds_skill	},
/*	{ ED_CLASS,	"ClassEd",	olc_cmds_class	}, */
	{ NULL }
};

static olc_cmd_t *	cmd_lookup(olc_cmd_t *cmd_table, const char *name);

static void do_olc(CHAR_DATA *ch, const char *argument, int fun);

/* Executed from comm.c.  Minimizes compiling when changes are made. */
bool run_olc_editor(DESCRIPTOR_DATA_MUDDY *d)
{
	char command[MAX_INPUT_LENGTH];
	olc_cmd_t *cmd;
	const char *argument;
	olced_t *olced = d->olced;

	if ((olced = d->olced) == NULL)
		return FALSE;

	argument = one_argument(/*d->incomm*/(char*)d->client->inarray,
			command, sizeof(command));
	//d->client->inlength = strlen(d->client->inarray);
			/* ���! �������� ���� ������ !!!!!!!!! */

	if (command[0] == '\0') {
		olced->cmd_table[FUN_SHOW].olc_fun(d->character, argument,
						   olced->cmd_table+FUN_SHOW);
		return TRUE;
	}
	
	if (!str_cmp(command, "done")) {
		edit_done(d);
		return TRUE;
	}

	if ((cmd = cmd_lookup(olced->cmd_table+FUN_FIRST, command)) == NULL
	||  cmd->olc_fun == NULL)
		return FALSE;

	/* log */
	if (d->character)
	{
		char dlog[MAX_INPUT_LENGTH];

		snprintf(dlog, sizeof(dlog), "[%s] %s %s",
				olced->name, cmd->name, argument);
		log_printf("Log_olc %s: %s", d->character->name,
				dlog);
		wiznet("Log_olc $N: $t", d->character, dlog, WIZ_DEBUGOLC,
				0, d->character->level);
	}


	if (cmd->olc_fun(d->character, argument, cmd))
		olced->cmd_table[FUN_TOUCH].olc_fun(d->character, str_empty,
						    olced->cmd_table+FUN_TOUCH);

	return TRUE;
}

void do_create(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_CREATE);
}

void do_edit(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_EDIT);
}

void do_alist(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_LIST);
}

void do_ashow(CHAR_DATA *ch, const char *argument)
{
	do_olc(ch, argument, FUN_SHOW);
}

/*
 * olced_busy -- returns TRUE if there is another character
 *		 is using the same OLC editor
 */
bool olced_busy(CHAR_DATA *ch, const char *id, void *edit, void *edit2)
{
	DESCRIPTOR_DATA_MUDDY *d;

	for (d = descriptor_list_muddy; d; d = d->next)
	{
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (vch != ch
		&&  d->olced
		&&  d->olced->id == id
		&&  (!edit || d->pEdit == edit)
		&&  (!edit2 || d->pEdit2 == edit2)) {
			char_printf(ch, "%s: %s is locking this editor "
					"right now.\n",
				    d->olced->name,
				    vch->name);
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Generic OLC editor functions.
 * All functions assume !IS_NPC(ch).
 */
OLC_FUN(olced_spell_out)
{
	char_puts("Spell it out.\n", ch);
	return FALSE;
}

OLC_FUN(olced_dummy)
{
	return FALSE;
}

bool olced_number(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t* cmd, int *pInt)
{
	int val;
	char *endptr;
	char arg[MAX_STRING_LENGTH];
	VALIDATE_FUN *validator;

	one_argument(argument, arg, sizeof(arg));
	val = strtol(arg, &endptr, 0);
	if (*arg == '\0' || *endptr != '\0') {
		char_printf(ch, "Syntax: %s number\n", cmd->name);
		return FALSE;
	}

	if ((validator = cmd->arg1) && !validator(ch, &val))
		return FALSE;

	*pInt = val;
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_name(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, const char **pStr)
{
	VALIDATE_FUN *validator;
	bool changed;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_printf(ch, "Syntax: %s string\n", cmd->name);
		return FALSE;
	}

	if ((validator = cmd->arg1) && !validator(ch, argument))
		return FALSE;

	changed = FALSE;
	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if (!str_cmp(arg, "all")) {
			char_printf(ch, "%s: %s: Illegal name.\n",
				    OLCED(ch)->name, arg);
			continue;
		}
		changed = TRUE;
		name_toggle(pStr, arg, ch, OLCED(ch)->name);
	}

	return changed;
}

bool olced_str(CHAR_DATA *ch, const char *argument,
	       olc_cmd_t *cmd, const char **pStr)
{
	VALIDATE_FUN *validator;

	if (IS_NULLSTR(argument)) {
		char_printf(ch, "Syntax: %s string\n", cmd->name);
		return FALSE;
	}

	if ((validator = cmd->arg1) && !validator(ch, argument))
		return FALSE;

	free_string(*pStr);
	*pStr = str_dup(argument);
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_str_text(CHAR_DATA *ch, const char *argument,
		    olc_cmd_t *cmd, const char **pStr)
{
	if (argument[0] =='\0') {
		string_append(ch, pStr);
		return FALSE;
	}

	char_printf(ch, "Syntax: %s\n", cmd->name);
	return FALSE;
}

bool olced_mlstr(CHAR_DATA *ch, const char *argument,
		 olc_cmd_t *cmd, mlstring **pmlstr)
{
	if (!mlstr_edit(pmlstr, argument)) {
		char_printf(ch, "Syntax: %s lang string\n", cmd->name);
		return FALSE;
	}
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_mlstrnl(CHAR_DATA *ch, const char *argument,
		   olc_cmd_t *cmd, mlstring **pmlstr)
{
	if (!mlstr_editnl(pmlstr, argument)) {
		char_printf(ch, "Syntax: %s lang string\n", cmd->name);
		return FALSE;
	}
	char_puts("Ok.\n", ch);
	return TRUE;
}

bool olced_mlstr_text(CHAR_DATA *ch, const char *argument,
		      olc_cmd_t *cmd, mlstring **pmlstr)
{
	if (!mlstr_append(ch, pmlstr, argument)) {
		char_printf(ch, "Syntax: %s lang\n", cmd->name);
		return FALSE;
	}
	return FALSE;
}

static void cb_format(int lang, const char **p, void *arg)
{
	*p = format_string(*p);
}

bool olced_exd(CHAR_DATA *ch, const char* argument,
	       olc_cmd_t *cmd, ED_DATA **ped)
{
	ED_DATA *ed;
	char command[MAX_INPUT_LENGTH];
	char keyword[MAX_INPUT_LENGTH];
	char str_slang[MAX_INPUT_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	int slang;

	argument = one_argument(argument, command, sizeof(command));
	argument = one_argument(argument, keyword, sizeof(keyword));
	argument = one_argument(argument, str_slang, sizeof(str_slang));
	argument = one_argument(argument, arg, sizeof(arg));

	if (command[0] == '\0' || keyword[0] == '\0') {
		// do_help(ch, "'OLC EXD'");
		// return;
		goto do_help_exd;	// see bellow
	}

	if (!str_cmp(command, "add")) {
		
		if ((slang = slang_lookup(str_slang)) == -1)
		{
			if (!str_cmp(str_slang, "none"))
				slang = 0;
			else {
				char_puts("Unknown slang.\n", ch);
				return FALSE;
			}
		}

		ed		= ed_new();
		ed->keyword	= str_dup(keyword);
		ed->slang	= slang;

		if (!mlstr_append(ch, &ed->description, arg)) {
			ed_free(ed);
			// do_help(ch, "'OLC EXD'");
			// return FALSE;
			goto do_help_exd;
		}

		ed->next	= *ped;
		*ped		= ed;
		char_puts("Extra description added.\n", ch);
		return TRUE;
	}

	if (!str_cmp(command, "name")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		if (!str_cmp(arg, "none")
		||  !str_cmp(arg, "all")) {
			char_printf(ch, "%s: %s: Illegal keyword.\n",
				    OLCED(ch)->name, arg);
			return FALSE;
		}
		name_toggle(&ed->keyword, arg, ch, OLCED(ch)->name);
		return TRUE;
	}

	if (!str_cmp(command, "edit")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		if ((slang = slang_lookup(str_slang)) == -1) {
			if (!str_cmp(str_slang, "none"))
			{
				if (ed->slang) {
					char_puts("Remove slang.\n", ch);
					ed->slang = 0;
				}
			} else
				char_puts("Unknown slang. Not change current.\n", ch);
			// slang = ed->slang;
		} else if (ed->slang != slang) {
			ed->slang = slang;
			char_printf(ch, "Slang of ED set '%s'\n", SKILL(slang)->name);
		}

		if (!mlstr_append(ch, &ed->description, arg)) {
			// do_help(ch, "'OLC EXD'");
			// return FALSE;
			goto do_help_exd;
		}
		return TRUE;
	}

	if (!str_cmp(command, "delete")) {
		ED_DATA *prev = NULL;

		for (ed = *ped; ed; ed = ed->next) {
			if (is_name(keyword, ed->keyword))
				break;
			prev = ed;
		}

		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		if (prev == NULL)
			*ped = ed->next;
		else
			prev->next = ed->next;

		ed->next = NULL;
		ed_free(ed);

		char_puts("Extra description deleted.\n", ch);
		return TRUE;
	}

	if (!str_cmp(command, "show")) {
		BUFFER *output;

		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		output = buf_new(-1);
		buf_printf(output, "Keyword:     [%s]\n", ed->keyword);
		buf_printf(output, "Slang:       [%s]\n",
				ed->slang ? SKILL(ed->slang)->name: "none");
		mlstr_dump(output, "Description: ", ed->description);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if (!str_cmp(command, "format")) {
		ed = ed_lookup(keyword, *ped);
		if (ed == NULL) {
			char_printf(ch, "%s: Extra description keyword not found.\n", OLCED(ch)->name);
			return FALSE;
		}

		mlstr_for_each(&ed->description, NULL, cb_format);
		char_puts("Extra description formatted.\n", ch);
		return TRUE;
	}

do_help_exd:
	char_puts(	"Syntax: exd add keyword {{slang|none} lang\n"
			"        exd edit keyword {{slang|none} lang\n"
			"        exd delete keyword\n"
			"        exd show keyword\n"
			"        exd format keyword\n"
			"        exd name <toggle name>\n",
		ch);
//	do_help(ch, "'OLC EXD'");
	return FALSE;
}

bool olced_flag64(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t* cmd, flag64_t *pflag)
{
	const flag_t *flag64_table;
	const flag_t *f;
	flag64_t ttype;
	const char *tname;

	if (!cmd->arg1) {
		char_printf(ch, "%s: %s: Table of values undefined (report it to implementors).\n", OLCED(ch)->name, cmd->name);
		return FALSE;
	}

	if (!str_cmp(argument, "?")) {
		show_flags(ch, cmd->arg1);
		return FALSE;
	}

	flag64_table = cmd->arg1;
	tname = flag64_table->name;
	ttype = flag64_table->bit;
	flag64_table++;

	switch (ttype) {
	case TABLE_BITVAL: {
		flag64_t marked = 0;

		/*
		 * Accept multiple flags.
		 */
		for (;;) {
			char word[MAX_INPUT_LENGTH];
	
			argument = one_argument(argument, word, sizeof(word));
	
			if (word[0] == '\0')
				break;
	
			if ((f = flag_lookup(cmd->arg1, word)) == NULL) {
				char_printf(ch, "Syntax: %s flag...\n"
						"Type '%s ?' for a list of "
						"acceptable flags.\n",
						cmd->name, cmd->name);
				return FALSE;
			}
			if (!f->settable) {
				char_printf(ch, "%s: %s: '%s': flag is not "
						"settable.\n",
					    OLCED(ch)->name,
					    cmd->name, f->name);
				continue;
			}
			SET_BIT(marked, f->bit);
		}
	
		if (marked) {
			TOGGLE_BIT(*pflag, marked);
			char_printf(ch, "%s: %s: '%s': flag(s) toggled.\n",
				    OLCED(ch)->name, cmd->name,
				    flag_string(cmd->arg1, marked));
			return TRUE;
		}
		return FALSE;

		/* NOT REACHED */
	}

	case TABLE_INTVAL:
		if ((f = flag_lookup(cmd->arg1, argument)) == NULL) {
			char_printf(ch, "Syntax: %s value\n"
					"Type '%s ?' for a list of "
					"acceptable values.\n",
					cmd->name, cmd->name);
			return FALSE;
		}
		if (!f->settable) {
			char_printf(ch, "%s: %s: '%s': value is not settable.\n",
				    OLCED(ch)->name, cmd->name, f->name);
			return FALSE;
		}
		*pflag = f->bit;
		char_printf(ch, "%s: %s: '%s': Ok.\n",
			    OLCED(ch)->name, cmd->name, f->name);
		return TRUE;
		/* NOT REACHED */

	default:
		char_printf(ch, "%s: %s: %s: table type %d unknown (report it to implementors).\n", OLCED(ch)->name, cmd->name, tname, ttype);
		return FALSE;
		/* NOT REACHED */
	}
}

bool olced_flag32(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t *cmd, flag32_t *psflag)
{
	flag64_t flag = (flag64_t) (*psflag);
	bool retval = olced_flag64(ch, argument, cmd, &flag);
	if (retval)
		*psflag = (flag32_t) flag;
	return retval;
}

bool olced_dice(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, int *dice)
{
	int num, type, bonus;
	char* p;

	if (argument[0] == '\0')
		goto bail_out;
	
	num = strtol(argument, &p, 0);
	if (num < 1 || *p != 'd')
		goto bail_out;

	type = strtol(p+1, &p, 0);
	if (type < 1 || *p != '+')
		goto bail_out;
	
	bonus = strtol(p+1, &p, 0);
	if (bonus < 0 || *p != '\0')
		goto bail_out;

	dice[DICE_NUMBER] = num;
	dice[DICE_TYPE]   = type;
	dice[DICE_BONUS]  = bonus;

	char_printf(ch, "%s set to %dd%d+%d.\n", cmd->name, num, type, bonus);
	return TRUE;

bail_out:
	char_printf(ch, "Syntax: %s <number>d<type>+<bonus>\n", cmd->name);
	return FALSE;
}

/*
bool olced_clan(CHAR_DATA *ch, const char *argument,
		olc_cmd_t *cmd, int *vnum)
{
	int cln;

	if (IS_NULLSTR(argument)) {
		char_printf(ch, "Syntax: %s clan\n"
				"Use 'clan ?' for list of valid clans.\n"
				"Use 'clan none' to reset clan.\n",
			    cmd->name);
		return FALSE;
	}

	if (!str_cmp(argument, "none")) {
		*vnum = 0;
		return TRUE;
	}

	if ((cln = cln_lookup(argument)) < 0) {
		char_printf(ch, "'%s': unknown clan.\n", argument);
		return FALSE;
	}

	*vnum = cln;
	return TRUE;
}
*/

bool olced_rulecl(CHAR_DATA *ch, const char *argument,
		  olc_cmd_t *cmd, lang_t *l)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int rulecl;

	argument = one_argument(argument, arg, sizeof(arg));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (argument[0] == '\0') {
		do_help(ch, "'OLC RULECLASS'");
		return FALSE;
	}

	if ((rulecl = flag_value(rulecl_names, arg)) < 0) {
		char_printf(ch, "%s: %s: unknown rule class\n",
			    OLCED(ch)->name, arg);
		return FALSE;
	}

	if (!str_prefix(arg2, "implicit")) {
		cmd->arg1 = validate_filename;
		return olced_str(ch, argument, cmd,
				 &l->rules[rulecl].file_impl);
	}

	if (!str_prefix(arg2, "explicit")) {
		cmd->arg1 = validate_filename;
		return olced_str(ch, argument, cmd,
				 &l->rules[rulecl].file_expl);
	}

	if (!str_prefix(arg2, "flags")) {
		cmd->arg1 = rulecl_flags;
		return olced_flag32(ch, argument, cmd, &l->rules[rulecl].flags);
	}

	do_help(ch, "'OLC RULECLASS'");
	return FALSE;
}

bool olced_vform_add(CHAR_DATA *ch, const char *argument,
		     olc_cmd_t *cmd, rule_t *r)
{
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (argument[0] == '\0' || !is_number(arg)) {
		do_help(ch, "'OLC VFORM'");
		return FALSE;
	}

	vform_add(r->f, atoi(arg), argument);
	char_puts("Form added.\n", ch);
	return TRUE;
}

bool olced_vform_del(CHAR_DATA *ch, const char *argument,
		     olc_cmd_t *cmd, rule_t *r)
{
	char arg[MAX_STRING_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		do_help(ch, "'OLC FORM'");
		return FALSE;
	}

	vform_del(r->f, atoi(arg));
	char_puts("Form deleted.\n", ch);
	return TRUE;
}

VALIDATE_FUN(validate_filename)
{
	if (strpbrk(arg, "/")) {
		char_printf(ch, "%s: Invalid characters in file name.\n",
			    OLCED(ch)->name);
		return FALSE;
	}
	return TRUE;
}

VALIDATE_FUN(validate_room_vnum)
{
	int vnum = *(int*) arg;

	if (vnum && get_room_index(vnum) == NULL) {
		char_printf(ch, "OLC: %d: no such room.\n", vnum);
		return FALSE;
	}

	return TRUE;
}

/*****************************************************************************
 Name:		show_commands
 Purpose:	Display all olc commands.
 Called by:	olc interpreters.
 ****************************************************************************/
OLC_FUN(show_commands)
{
	BUFFER *	output;
	int		col;

	output = buf_new(-1); 

	col = 0;
	for (cmd = OLCED(ch)->cmd_table+FUN_FIRST; cmd->name; cmd++) {
		buf_printf(output, "%-15.15s", cmd->name);
		if (++col % 5 == 0)
			buf_add(output, "\n");
	}
	if (col % 5 != 0)
		buf_add(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(show_version)
{
	char_puts(VERSION	"\n"
		  AUTHOR	"\n"
		  DATE		"\n"
		  CREDITS	"\n", ch);

	return FALSE;
}    

AREA_DATA *get_edited_area(CHAR_DATA *ch)
{
	int vnum;
	olced_t *olced = OLCED(ch);
	void *p = ch->desc->pEdit;

	if (!olced)
		return NULL;

	if (olced->id == ED_AREA)
		return p;

	if (olced->id == ED_HELP)
		return ((HELP_DATA*) p)->area;

	if (olced->id == ED_ROOM)
		return ch->in_room->area;

	if (olced->id == ED_OBJ)
		vnum = ((OBJ_INDEX_DATA*) p)->vnum;
	else if (olced->id == ED_MOB)
		vnum = ((MOB_INDEX_DATA*) p)->vnum;
	else if (olced->id == ED_MPCODE)
		vnum = ((MPCODE*) p)->vnum;
	else
		return NULL;
		
	return area_vnum_lookup(vnum);
}

bool touch_area(AREA_DATA *pArea)
{
	if (pArea)
		SET_BIT(pArea->flags, AREA_CHANGED);
	return FALSE;
}

bool touch_vnum(int vnum)
{
	return touch_area(area_vnum_lookup(vnum));
}

void edit_done(DESCRIPTOR_DATA_MUDDY *d)
{
	d->pEdit = NULL;
	d->olced = NULL;
}

/* Local functions */

/* lookup OLC editor by id */
olced_t *olced_lookup(const char * id)
{
	olced_t *olced;

	if (IS_NULLSTR(id))
		return NULL;

	for (olced = olced_table; olced->id; olced++)
		if (!str_prefix(id, olced->id))
			return olced;
	return NULL;
}

/* lookup cmd function by name */
static olc_cmd_t *cmd_lookup(olc_cmd_t *cmd_table, const char *name)
{
	for (; cmd_table->name; cmd_table++)
		if (!str_prefix(name, cmd_table->name))
			return cmd_table;
	return NULL;
}

char* help_topics[FUN_MAX] =
{
	"'OLC CREATE'",
	"'OLC EDIT'",
	str_empty,
	"'OLC ASHOW'",
	"'OLC ALIST'"
};

static void do_olc(CHAR_DATA *ch, const char *argument, int fun)
{
	char command[MAX_INPUT_LENGTH];
	olced_t *olced;

	if (IS_NPC(ch))
		return;

	argument = one_argument(argument, command, sizeof(command));
	if ((olced = olced_lookup(command)) == NULL) {
        	do_help(ch, help_topics[fun]);
        	return;
	}

	olced->cmd_table[fun].olc_fun(ch, argument, olced->cmd_table+fun);
}

const char* msgtoa(const char *argument)
{
	static char buf[MAX_STRING_LENGTH];
	const char *i;
	int o;

	for (o = 0, i = argument; o < sizeof(buf)-2 && *i; i++, o++) {
		switch (*i) {
		case '\a':
			buf[o++] = '\\';
			buf[o] = 'a';
			continue;
		case '\n':
			buf[o++] = '\\';
			buf[o] = 'n';
			continue;
		case '\r':
			buf[o++] = '\\';
			buf[o] = 'r';
			continue;
		case '\\':
			buf[o++] = '\\';
			break;
		case '{':
			buf[o++] = *i;
			break;
		}
		buf[o] = *i;
	}
	buf[o] = '\0';

	return buf;
}
