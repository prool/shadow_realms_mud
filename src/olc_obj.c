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
 * $Id: olc_obj.c,v 1.35 2007/12/02 19:40:22 rem Exp $
 */

#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"
#include "olc.h"
#include "db.h"
#include "obj_prog.h"

#define EDIT_OBJ(ch, obj)	(obj = (OBJ_INDEX_DATA*) ch->desc->pEdit)

DECLARE_OLC_FUN(objed_create		);
DECLARE_OLC_FUN(objed_edit		);
DECLARE_OLC_FUN(objed_touch		);
DECLARE_OLC_FUN(objed_show		);
DECLARE_OLC_FUN(objed_list		);
DECLARE_OLC_FUN(objed_del		);

DECLARE_OLC_FUN(objed_name		);
DECLARE_OLC_FUN(objed_short		);
DECLARE_OLC_FUN(objed_long		);
DECLARE_OLC_FUN(objed_addaffect		);
DECLARE_OLC_FUN(objed_addapply		);
DECLARE_OLC_FUN(objed_delaffect		);
DECLARE_OLC_FUN(objed_value0		);
DECLARE_OLC_FUN(objed_value1		);
DECLARE_OLC_FUN(objed_value2		);
DECLARE_OLC_FUN(objed_value3		);
DECLARE_OLC_FUN(objed_value4		);
DECLARE_OLC_FUN(objed_weight		);
DECLARE_OLC_FUN(objed_limit		);
DECLARE_OLC_FUN(objed_cost		);
DECLARE_OLC_FUN(objed_exd		);

DECLARE_OLC_FUN(objed_extra		);
DECLARE_OLC_FUN(objed_wear		);
DECLARE_OLC_FUN(objed_type		);
//DECLARE_OLC_FUN(objed_affect		);
DECLARE_OLC_FUN(objed_material		);
DECLARE_OLC_FUN(objed_level		);
DECLARE_OLC_FUN(objed_condition		);
//DECLARE_OLC_FUN(objed_clan		);
DECLARE_OLC_FUN(objed_clone		);
DECLARE_OLC_FUN(objed_gender		);

DECLARE_VALIDATE_FUN(validate_condition);

olc_cmd_t olc_cmds_obj[] =
{
/*	{ command	function		arg			}, */

	{ "create",	objed_create					},
	{ "edit",	objed_edit					},
	{ "touch",	objed_touch					},
	{ "show",	objed_show					},
	{ "list",	objed_list					},
	{ "delete_ob",	olced_spell_out					},
	{ "delete_obj",	objed_del					},

	{ "addaffect",	objed_addaffect					},
	{ "addapply",	objed_addapply					},
	{ "cost",	objed_cost					},
	{ "delaffect",	objed_delaffect					},
	{ "exd",	objed_exd					},
	{ "long",	objed_long					},
	{ "name",	objed_name					},
	{ "short",	objed_short					},
	{ "v0",		objed_value0					},
	{ "v1",		objed_value1					},
	{ "v2",		objed_value2					},
	{ "v3",		objed_value3					},
	{ "v4",		objed_value4					},
	{ "weight",	objed_weight					},
	{ "limit",	objed_limit					},

	{ "extra",	objed_extra,		extra_flags		},
	{ "wear",	objed_wear,		wear_flags		},
	{ "type",	objed_type,		item_types		},
	{ "material",	objed_material 					},
	{ "level",	objed_level					},
	{ "condition",	objed_condition,	validate_condition	},
//	{ "clan",	objed_clan					},
	{ "clone",	objed_clone					},
	{ "gender",	objed_gender,		gender_table		},

	{ "version",	show_version					},
	{ "commands",	show_commands					},

	{ NULL }
};

void		show_obj_values	(BUFFER *output, OBJ_INDEX_DATA *pObj);
static int	set_obj_values	(BUFFER *output, OBJ_INDEX_DATA *pObj,
				 const char *argument, int value_num);
static void	show_spells	(BUFFER *output, int tar);
float		get_obj_vc(OBJ_INDEX_DATA *obj, int level);

OLC_FUN(objed_create)
{
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;
	int  value;
	int  iHash;
	char arg[MAX_STRING_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	value = atoi(arg);
	if (!value) {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	pArea = area_vnum_lookup(value);
	if (!pArea) {
		char_puts("ObjEd: That vnum is not assigned an area.\n", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("ObjEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (get_obj_index(value)) {
		char_puts("ObjEd: Object vnum already exists.\n", ch);
		return FALSE;
	}
		 
	pObj			= new_obj_index();
	pObj->vnum		= value;
		 
	if (value > top_vnum_obj)
		top_vnum_obj = value;

	iHash			= value % MAX_KEY_HASH;
	pObj->next		= obj_index_hash[iHash];
	obj_index_hash[iHash]	= pObj;

	ch->desc->pEdit		= (void *)pObj;
	OLCED(ch)		= olced_lookup(ED_OBJ);
	touch_area(pArea);
	char_puts("ObjEd: Object created.\n", ch);
	return FALSE;
}

OLC_FUN(objed_edit)
{
	char arg[MAX_INPUT_LENGTH];
	int value;
	OBJ_INDEX_DATA *pObj;
	AREA_DATA *pArea;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	value = atoi(arg);
	pObj = get_obj_index(value);
	if (!pObj) {
		char_puts("ObjEd: Vnum does not exist.\n", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pObj->vnum);
	if (!IS_BUILDER(ch, pArea)) {
		char_puts("ObjEd: Insufficient security.\n", ch);
	       	return FALSE;
	}

	ch->desc->pEdit = (void*) pObj;
	OLCED(ch)	= olced_lookup(ED_OBJ);
	return FALSE;
}

OLC_FUN(objed_touch)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return touch_vnum(pObj->vnum);
}

OLC_FUN(objed_show)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA	*pObj;
	AREA_DATA	*pArea;
	AFFECT_DATA *paf;
	int cnt;
	BUFFER *output;
	//clan_t *clan;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_OBJ))
			EDIT_OBJ(ch, pObj);
		else {
			do_help(ch, "'OLC EDIT'");
			return FALSE;
		}
	}
	else {
		int value = atoi(arg);
		pObj = get_obj_index(value);
		if (!pObj) {
			char_puts("ObjEd: Vnum does not exist.\n", ch);
			return FALSE;
		}
	}

	pArea = area_vnum_lookup(pObj->vnum);

	output = buf_new(-1);
	buf_printf(output, "Name:        [%s]\nArea:        [%5d] %s\n",
		pObj->name, pArea->vnum, pArea->name);

	buf_printf(output, "Vnum:        [%5d]  Gender: [%s]\n"
			   "Type:        [%s]\n",
		pObj->vnum,
		flag_string(gender_table, pObj->gender),
		flag_string(item_types, pObj->item_type));

	//if (pObj->clan && (clan = clan_lookup(pObj->clan))) 
	//	buf_printf(output, "Clan        : [%s]\n", clan->name);

	if (pObj->limit != -1)
		buf_printf(output, "Limit:       [%5d]\n", pObj->limit);
	else
		buf_add(output, "Limit:       [none]\n");

	buf_printf(output, "Level:       [%5d]  VC: [%.2f]\n",
		pObj->level, get_obj_vc(pObj, pObj->level));

	buf_printf(output, "Wear flags:  [%s]\n",
		flag_string(wear_flags, pObj->wear_flags));

	buf_printf(output, "Extra flags: [%s]\n",
		flag_string(extra_flags, pObj->extra_flags));

	buf_printf(output, "Material:    [%s]\n",                /* ROM */
		pObj->material);

	buf_printf(output, "Condition:   [%5d]\n",               /* ROM */
		pObj->condition);

	buf_printf(output, "Weight:      [%5d]\nCost:        [%5d]\n",
		pObj->weight, pObj->cost);

	if (pObj->ed) {
		ED_DATA *ed;

		buf_add(output, "Ex desc kwd: ");

		for (ed = pObj->ed; ed; ed = ed->next)
			buf_printf(output, "[%s]", ed->keyword);

		buf_add(output, "\n");
	}

	mlstr_dump(output, "Short desc: ", pObj->short_descr);
	mlstr_dump(output, "Long desc: ", pObj->description);
	
	if (pObj->oprogs)
	{
		int i;
		OPROG_DATA *p;

		for (i = 0; i < OPROG_MAX; i++)
		   if (pObj->oprogs[i])
		   	for (p = oprog_table; p->name != NULL; p++)
		   	   if (p->fn == pObj->oprogs[i])
				buf_printf(output, "ObjProg:     [%s]\n",
					p->name);
	}

	for (cnt = 0, paf = pObj->affected; paf; paf = paf->next) {
		where_t *w = where_lookup(paf->where);

		if (cnt == 0) {
			buf_add(output, "Number      Affects Modifier Affects Bitvector\n");
			buf_add(output, "------ ------------ -------- ------- -----------------------------------------\n");
		}
		buf_printf(output, "[%4d] %12.12s",
			   cnt, flag_string(apply_flags, paf->location));
		if (paf->location == APPLY_5_SKILL || paf->location == APPLY_10_SKILL)
			buf_printf(output, " %-8s ", skill_name(paf->modifier));
		else
			buf_printf(output, " %8d ", paf->modifier);
		buf_printf(output, "%7.7s %s\n",
			flag_string(apply_types, paf->where),
			w ? flag_string(w->table, paf->bitvector) : "none");
		cnt++;
	}

	//(1 + obj->value[2]) * obj->value[1] / 2
	if (pObj->item_type == ITEM_WEAPON)
		buf_printf(output,	"Average:     [%d]\n",
			(1 + pObj->value[2]) * pObj->value[1] / 2);

	show_obj_values(output, pObj);
	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(objed_list)
{
	OBJ_INDEX_DATA	*pObjIndex;
	AREA_DATA	*pArea;
	BUFFER		*buffer;
	char		arg  [MAX_INPUT_LENGTH];
	bool fAll, found;
	int vnum;
	int  col = 0;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC ALIST'");
		return FALSE;
	}

	if ((pArea = get_edited_area(ch)) == NULL)
		pArea = ch->in_room->area;

	buffer  = buf_new(-1);
	fAll    = !str_cmp(arg, "all");
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pObjIndex = get_obj_index(vnum))) {
			if (fAll || is_name(arg, pObjIndex->name)
			|| flag_value(item_types, arg) == pObjIndex->item_type) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %-17.16s{x",
					   pObjIndex->vnum,
					   mlstr_mval(pObjIndex->short_descr));
				if (++col % 3 == 0)
					buf_add(buffer, "\n");
			}
		}
	}

	if (!found)
		char_puts("Object(s) not found in this area.\n", ch);
	else {
		if (col % 3 != 0)
			buf_add(buffer, "\n");

		page_to_char(buf_string(buffer), ch);
	}

	buf_free(buffer);
	return FALSE;
}

OLC_FUN(objed_del)
{
	OBJ_INDEX_DATA *pObj;
	OBJ_DATA *obj, *obj_next;
	AREA_DATA *area;
	int i;
	bool error = FALSE;

	EDIT_OBJ(ch, pObj);

	if (olced_busy(ch, ED_OBJ, pObj, NULL))
		return FALSE;

/* check that pObj is not in resets */
	for (i = 0; i < MAX_KEY_HASH; i++) {
		ROOM_INDEX_DATA *room;

		for (room = room_index_hash[i]; room; room = room->next) {
			int j = 0;
			RESET_DATA *reset;

			for (reset = room->reset_first; reset;
							reset = reset->next) {
				bool found = FALSE;

				j++;
				switch (reset->command) {
				case 'P':
					if (reset->arg3 == pObj->vnum
					|| reset->arg1 == pObj->vnum)
						found = TRUE;
					break;

					/* FALLTHRU */

				case 'O':
				case 'G':
				case 'E':
					if (reset->arg1 == pObj->vnum)
						found = TRUE;
					break;
				}

				if (!found)
					continue;

				if (!error) {
					error = TRUE;
					char_puts("ObjEd: can't delete obj "
						  "index: delete the "
						  "following resets:\n", ch);
				}

				char_printf(ch, "ObjEd: room %d, reset %d\n",
					    room->vnum, j);
			}
		}
	}

	if (error)
		return FALSE;

/* delete all the instances of obj index */
	for (obj = object_list; obj; obj = obj_next) {
		obj_next = obj->next;

		if (obj->pIndexData == pObj)
			extract_obj(obj, XO_F_NORECURSE);
	}

	if ((area = area_vnum_lookup(pObj->vnum)))
		touch_area(area);

/* delete obj index itself */
	i = pObj->vnum % MAX_KEY_HASH;
	if (pObj == obj_index_hash[i])
		obj_index_hash[i] = pObj->next;
	else {
		OBJ_INDEX_DATA *prev;

		for (prev = obj_index_hash[i]; prev; prev = prev->next)
			if (prev->next == pObj)
				break;

		if (prev)
			prev->next = pObj->next;
	}

	free_obj_index(pObj);
	char_puts("ObjEd: Obj index deleted.\n", ch);
	edit_done(ch->desc);
	return FALSE;
}

/*
 * Need to issue warning if flag isn't valid. -- does so now -- Hugin.
 */
OLC_FUN(objed_addaffect)
{
	int location;
	int modifier;
	flag32_t where;
	flag64_t bitvector;
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	char loc[MAX_STRING_LENGTH];
	char mod[MAX_STRING_LENGTH];
	char wh[MAX_STRING_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, loc, sizeof(loc));
	argument = one_argument(argument, mod, sizeof(mod));
	argument = one_argument(argument, wh, sizeof(wh));

	if (loc[0] == '\0') {
		do_help(ch, "'OLC ADDAFFECT'");
		return FALSE;
	}

	if (!str_cmp(loc, "none")) {
		location = APPLY_NONE;
		modifier = 0;
	}
	else {
		if ((location = flag_value(apply_flags, loc)) < 0) {
			char_puts("Valid locations are:\n", ch);
			show_flags(ch, apply_flags);
			return FALSE;
		}
		
		if (location == APPLY_5_SKILL || location == APPLY_10_SKILL) {
			if ((modifier = sn_lookup(mod)) < 0) {
				char_puts("Unknown skill/spell.\n", ch);
				do_help(ch, "'OLC ADDAFFECT'");
				return FALSE;
			}
		} else {
			if (!is_number(mod)) {
				do_help(ch, "'OLC ADDAFFECT'");
				return FALSE;
			}
			modifier = atoi(mod);
		}
	}

	if (wh[0] == '\0') {
		where = -1;
		bitvector = 0;
	}
	else {
		where_t *w;

		if ((where = flag_value(apply_types, wh)) < 0) {
			char_puts("Valid bitaffect locations are:\n", ch);
			show_flags(ch, apply_types);
			return FALSE;
		}

		if ((w = where_lookup(where)) == NULL) {
			char_printf(ch, "%s: not in where_table.\n",
				    flag_string(apply_types, where));
			return FALSE;
		}

		if ((bitvector = flag_value(w->table, argument)) == 0) {
			char_printf(ch, "Valid '%s' bitaffect flags are:\n",
				    flag_string(apply_types, where));
			show_flags(ch, w->table);
			return FALSE;
		}
	}

	pAf             = aff_new();
	pAf->location   = location;
	pAf->modifier   = modifier;
	pAf->where	= where;
	pAf->type       = -1;
	pAf->duration   = -1;
	pAf->bitvector  = bitvector;
	pAf->level      = pObj->level;
	pAf->next       = pObj->affected;
	pObj->affected  = pAf;

	char_puts("Affect added.\n", ch);
	return TRUE;
}

OLC_FUN(objed_addapply)
{
	int location, bv, where, modifier;
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	where_t *wd;
	char loc[MAX_STRING_LENGTH];
	char mod[MAX_STRING_LENGTH];
	char type[MAX_STRING_LENGTH];
	char bvector[MAX_STRING_LENGTH];

	EDIT_OBJ(ch, pObj);

	argument = one_argument(argument, type, sizeof(type));
	argument = one_argument(argument, loc, sizeof(loc));
	argument = one_argument(argument, mod, sizeof(mod));
	one_argument(argument, bvector, sizeof(bvector));

	if ((where = flag_value(apply_types, type)) < 0) {
		char_puts("Invalid apply type. Valid apply types are:\n", ch);
		show_flags(ch, apply_types);
		return FALSE;
	}

	if ((location = flag_value(apply_flags, loc)) < 0) {
		char_puts("Valid applies are:\n", ch);
		show_flags(ch, apply_flags);
		return FALSE;
	}

	if ((wd = where_lookup(where)) == NULL) {
		char_puts("ObjEd: bit vector table undefined. "
			  "Report it to implementors.\n", ch);
		return FALSE;
	}

	if ((bv = flag_value(wd->table, bvector)) == 0) {
		char_puts("Valid bitvector types are:\n", ch);
		show_flags(ch, wd->table);
		return FALSE;
	}

	if (location == APPLY_5_SKILL || location == APPLY_10_SKILL) {
		if ((modifier = sn_lookup(mod)) < 0) {
			char_puts("Unknown skill/spell.\n", ch);
			char_puts("Syntax: addapply type location "
				"mod bitvector\n", ch);
			return FALSE;
		}
	} else {
		if (!is_number(mod)) {
			char_puts("Syntax: addapply type location "
				"mod bitvector\n", ch);
			return FALSE;
		}
		modifier = atoi(mod);
	}

	pAf             = aff_new();
	pAf->location   = location;
	pAf->modifier   = modifier;
	pAf->where	= where;
	pAf->type	= -1;
	pAf->duration   = -1;
	pAf->bitvector  = bv;
	pAf->level      = pObj->level;
	pAf->next       = pObj->affected;
	pObj->affected  = pAf;

	char_puts("Apply added.\n", ch);
	return TRUE;
}

/*
 * My thanks to Hans Hvidsten Birkeland and Noam Krendel(Walker)
 * for really teaching me how to manipulate pointers.
 */
OLC_FUN(objed_delaffect)
{
	OBJ_INDEX_DATA *pObj;
	AFFECT_DATA *pAf;
	AFFECT_DATA *pAf_next;
	char affect[MAX_STRING_LENGTH];
	int  value;
	int  cnt = 0;

	EDIT_OBJ(ch, pObj);

	one_argument(argument, affect, sizeof(affect));

	if (!is_number(affect) || affect[0] == '\0')
	{
		char_puts("Syntax:  delaffect [#xaffect]\n", ch);
		return FALSE;
	}

	value = atoi(affect);

	if (value < 0)
	{
		char_puts("Only non-negative affect-numbers allowed.\n", ch);
		return FALSE;
	}

	if (!(pAf = pObj->affected))
	{
		char_puts("ObjEd:  Non-existant affect.\n", ch);
		return FALSE;
	}

	if(value == 0)	/* First case: Remove first affect */
	{
		pAf = pObj->affected;
		pObj->affected = pAf->next;
		aff_free(pAf);
	}
	else		/* Affect to remove is not the first */
	{
		while ((pAf_next = pAf->next) && (++cnt < value))
			 pAf = pAf_next;

		if(pAf_next)		/* See if it's the next affect */
		{
			pAf->next = pAf_next->next;
			aff_free(pAf_next);
		}
		else                                 /* Doesn't exist */
		{
			 char_puts("No such affect.\n", ch);
			 return FALSE;
		}
	}

	char_puts("Affect removed.\n", ch);
	return TRUE;
}

OLC_FUN(objed_name)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_name(ch, argument, cmd, &pObj->name);
}

OLC_FUN(objed_short)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_mlstr(ch, argument, cmd, &pObj->short_descr);
}

OLC_FUN(objed_long)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_mlstr(ch, argument, cmd, &pObj->description);
}

/*****************************************************************************
 Name:		objed_values
 Purpose:	Finds the object and sets its value.
 Called by:	The four valueX functions below. (now five -- Hugin)
 ****************************************************************************/
bool objed_values(CHAR_DATA *ch, const char *argument, int value)
{
	BUFFER *output;
	int errcode = 1;
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);

	output = buf_new(-1);
	if (argument[0] == '\0'
	||  (errcode = set_obj_values(output, pObj, argument, value)) < 2)
		show_obj_values(output, pObj);
	page_to_char(buf_string(output), ch);
	buf_free(output);
	return !errcode;
}

OLC_FUN(objed_value0)
{
	return objed_values(ch, argument, 0);
}

OLC_FUN(objed_value1)
{
	return objed_values(ch, argument, 1);
}

OLC_FUN(objed_value2)
{
	return objed_values(ch, argument, 2);
}

OLC_FUN(objed_value3)
{
	return objed_values(ch, argument, 3);
}

OLC_FUN(objed_value4)
{
	return objed_values(ch, argument, 4);
}

OLC_FUN(objed_weight)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->weight);
}

OLC_FUN(objed_limit)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->limit);
}

OLC_FUN(objed_cost)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, (int*)&pObj->cost);
}

OLC_FUN(objed_exd)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_exd(ch, argument, cmd, &pObj->ed);
}

OLC_FUN(objed_extra)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag64(ch, argument, cmd, &pObj->extra_flags);
}

OLC_FUN(objed_wear)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag32(ch, argument, cmd, &pObj->wear_flags);
}

OLC_FUN(objed_type)
{
	bool changed;
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	changed = olced_flag32(ch, argument, cmd, &pObj->item_type);
	if (changed) {
		pObj->value[0] = 0;
		pObj->value[1] = 0;
		pObj->value[2] = 0;
		pObj->value[3] = 0;
		pObj->value[4] = 0;     /* ROM */
	}
	return changed;
}

OLC_FUN(objed_material)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_str(ch, argument, cmd, &pObj->material);
}

OLC_FUN(objed_level)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->level);
}

OLC_FUN(objed_condition)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_number(ch, argument, cmd, &pObj->condition);
}

/*
OLC_FUN(objed_clan)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_clan(ch, argument, cmd, &pObj->clan);
}
*/

OLC_FUN(objed_clone)
{
	OBJ_INDEX_DATA *pObj;
	OBJ_INDEX_DATA *pFrom;
	char arg[MAX_INPUT_LENGTH];
	int i;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;
	AFFECT_DATA **ppaf;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		char_puts("Syntax: clone <vnum>\n", ch);
		return FALSE;
	}

	i = atoi(arg);
	if ((pFrom = get_obj_index(i)) == NULL) {
		char_printf(ch, "ObjEd: %d: Vnum does not exist.\n", i);
		return FALSE;
	}

	EDIT_OBJ(ch, pObj);
	if (pObj == pFrom)
		return FALSE;

	free_string(pObj->name);
	pObj->name		= str_qdup(pFrom->name);
	free_string(pObj->material);
	pObj->material		= str_qdup(pFrom->material);
	mlstr_free(pObj->short_descr);
	pObj->short_descr	= mlstr_dup(pFrom->short_descr);
	mlstr_free(pObj->description);
	pObj->description	= mlstr_dup(pFrom->description);

	pObj->item_type		= pFrom->item_type;
	pObj->extra_flags	= pFrom->extra_flags;
	pObj->wear_flags	= pFrom->wear_flags;
	pObj->level		= pFrom->level;
	pObj->condition		= pFrom->condition;
	pObj->weight		= pFrom->weight;
	pObj->cost		= pFrom->cost;
	pObj->limit		= pFrom->limit;
	//pObj->clan		= pFrom->clan;

	for (i = 0; i < 5; i++)
		pObj->value[i]	= pFrom->value[i];

/* copy affects */
	for (paf = pObj->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
	}

	ppaf = &pObj->affected;
	for (paf = pFrom->affected; paf; paf = paf->next) {
		*ppaf = aff_dup(paf);
		ppaf = &(*ppaf)->next;
	}

/* copy extra descriptions */
	ed_free(pObj->ed);
	pObj->ed = ed_dup(pFrom->ed);

	return TRUE;
}

OLC_FUN(objed_gender)
{
	OBJ_INDEX_DATA *pObj;
	EDIT_OBJ(ch, pObj);
	return olced_flag32(ch, argument, cmd, &pObj->gender);
}

void show_obj_values(BUFFER *output, OBJ_INDEX_DATA *pObj)
{
	struct attack_type *atd;
	switch(pObj->item_type) {
	default:	/* No values. */
		buf_add(output, "Currently edited obj has unknown item type.\n");
		/* FALLTHRU */

	case ITEM_TREASURE:
	case ITEM_CLOTHING:
	case ITEM_TRASH:
	case ITEM_KEY:
	case ITEM_BOAT:
	case ITEM_CORPSE_NPC:
	case ITEM_CORPSE_PC:
	case ITEM_PROTECT:
	case ITEM_MAP:
	case ITEM_WARP_STONE :
	case ITEM_ROOM_KEY:
	case ITEM_GEM:
	case ITEM_JEWELRY:
	case ITEM_JUKEBOX:
	case ITEM_TATTOO:
		break;
		     
	case ITEM_LIGHT:
		if (pObj->value[2] == -1 || pObj->value[2] == 999) /* ROM OLC */
			buf_printf(output, "[v2] Light:  Infinite[-1]\n");
		else
			buf_printf(output, "[v2] Light:  [%d]\n", pObj->value[2]);
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		buf_printf(output,
			"[v0] Level:          [%d]\n"
			"[v1] Charges Total:  [%d]\n"
			"[v2] Charges Left:   [%d]\n"
			"[v3] Spell:          %s\n",
			pObj->value[0],
			pObj->value[1],
			pObj->value[2],
			skill_name(pObj->value[3]));
		break;

	case ITEM_PORTAL:
		buf_printf(output,
			    "[v0] Charges:        [%d]\n"
			    "[v1] Exit Flags:     %s\n"
			    "[v2] Portal Flags:   %s\n"
			    "[v3] Goes to (vnum): [%d]\n",
			    pObj->value[0],
			    flag_string(exit_flags, pObj->value[1]),
			    flag_string(portal_flags , pObj->value[2]),
			    pObj->value[3]);
		break;
			
	case ITEM_FURNITURE:          
		buf_printf(output,
			    "[v0] Max people:      [%d]\n"
			    "[v1] Max weight:      [%d]\n"
			    "[v2] Furniture Flags: %s\n"
			    "[v3] Heal bonus:      [%d]\n"
			    "[v4] Mana bonus:      [%d]\n",
			    pObj->value[0],
			    pObj->value[1],
			    flag_string(furniture_flags, pObj->value[2]),
			    pObj->value[3],
			    pObj->value[4]);
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		if (pObj->item_type == ITEM_SCROLL)
		{
			buf_printf(output, "Slang[see in first exd]: %s\n",
				(pObj->ed && pObj->ed->slang) ?
				SKILL(pObj->ed->slang)->name : "none");
		}
		buf_printf(output,
			"[v0] Level:  [%d]\n"
			"[v1] Spell:  %s\n"
			"[v2] Spell:  %s\n"
			"[v3] Spell:  %s\n"
			"[v4] Spell:  %s\n",
			pObj->value[0],
			skill_name(pObj->value[1]),
			skill_name(pObj->value[2]),
			skill_name(pObj->value[3]),
			skill_name(pObj->value[4]));
		break;

/* ARMOR for ROM */

	case ITEM_ARMOR:
		buf_printf(output,
			"[v0] Ac pierce       [%d]\n"
			"[v1] Ac bash         [%d]\n"
			"[v2] Ac slash        [%d]\n"
			"[v3] Ac exotic       [%d]\n",
			pObj->value[0],
			pObj->value[1],
			pObj->value[2],
			pObj->value[3]);
			break;

/* WEAPON changed in ROM: */
/* I had to split the output here, I have no idea why, but it helped -- Hugin */
/* It somehow fixed a bug in showing scroll/pill/potions too ?! */
	case ITEM_WEAPON:
		buf_printf(output, "[v0] Weapon class:   %s\n",
			    flag_string(weapon_class, pObj->value[0]));
		buf_printf(output, "[v1] Number of dice: [%d]\n", pObj->value[1]);
		buf_printf(output, "[v2] Type of dice:   [%d]\n", pObj->value[2]);
		buf_printf(output, "[v3] Type:           %s\n",
			    (atd = GET_ATTACK_T(pObj->value[3])) ?
			    mlstr_mval(atd->name) : "none");
		buf_printf(output, "[v4] Special type:   %s\n",
			    flag_string(weapon_type2,  pObj->value[4]));
		break;

	case ITEM_CONTAINER:
		buf_printf(output,
			"[v0] Weight:     [%d kg]\n"
			"[v1] Flags:      [%s]\n"
			"[v2] Key:     %s [%d]\n"
			"[v3] Capacity    [%d]\n"
			"[v4] Weight Mult [%d]\n",
			pObj->value[0],
			flag_string(cont_flags, pObj->value[1]),
		        get_obj_index(pObj->value[2]) ?
			mlstr_mval(get_obj_index(pObj->value[2])->short_descr) :
			"none",
		        pObj->value[2],
		        pObj->value[3],
		        pObj->value[4]);
		break;

	case ITEM_DRINK_CON:
		buf_printf(output,
			    "[v0] Liquid Total: [%d]\n"
			    "[v1] Liquid Left:  [%d]\n"
			    "[v2] Liquid:       %s\n"
			    "[v3] Poisoned:     %s\n",
			    pObj->value[0],
			    pObj->value[1],
			    liq_table[pObj->value[2]].liq_name,
			    pObj->value[3] ? "Yes" : "No");
		break;

	case ITEM_FOUNTAIN:
		buf_printf(output,
			    "[v0] Liquid Total: [%d]\n"
			    "[v1] Liquid Left:  [%d]\n"
			    "[v2] Liquid:	    %s\n",
			    pObj->value[0],
			    pObj->value[1],
			    liq_table[pObj->value[2]].liq_name);
		break;
			    
	case ITEM_FOOD:
		buf_printf(output,
			"[v0] Food hours: [%d]\n"
			"[v1] Full hours: [%d]\n"
			"[v3] Poisoned:   %s\n",
			pObj->value[0],
			pObj->value[1],
			pObj->value[3] ? "Yes" : "No");
		break;

	case ITEM_MONEY:
		buf_printf(output, "[v0] Silver: [%d]\n"
				   "[v1] Gold:   [%d]\n",
			   pObj->value[0], pObj->value[1]);
		break;

	case ITEM_HORSE_TICKET:
		buf_printf(output, "[v0] Vnum horse: [%d]\n"
				   "[v1] Max charge: [%d]\n"
				   "[v2] Current charge: [%d]\n",
			pObj->value[0], pObj->value[1], pObj->value[2]);
		break;
	case ITEM_DIG:
		buf_printf(output, "[v0] Dig percent: [%d]\n",
			pObj->value[0]);
		break;
	}
}

/*
 * Return values:
 *	0 - pObj was changed successfully
 *	1 - pObj was not changed
 *	2 - pObj was not changed, do not show obj values
 */
int set_obj_values(BUFFER *output, OBJ_INDEX_DATA *pObj,
		   const char *argument, int value_num)
{
	switch (pObj->item_type) {
		int	val;

	default:
		return 1;
		     
	case ITEM_LIGHT:
		switch (value_num) {
		default:
			return 1;
		case 2:
			buf_add(output, "HOURS OF LIGHT SET.\n\n");
			pObj->value[2] = atoi(argument);
			break;
		}
		break;

	case ITEM_WAND:
	case ITEM_STAFF:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "SPELL LEVEL SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "TOTAL NUMBER OF CHARGES SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "CURRENT NUMBER OF CHARGES SET.\n\n");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			if (!str_cmp(argument, "?")
			||  (val = sn_lookup(argument)) < 0) {
				show_spells(output, -1);
				return 2;
			}
			buf_add(output, "SPELL TYPE SET.\n");
			pObj->value[3] = val;
			break;
		}
		break;

	case ITEM_SCROLL:
	case ITEM_POTION:
	case ITEM_PILL:
		switch (value_num) {
		case 0:
			buf_add(output, "SPELL LEVEL SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
		case 2:
		case 3:
		case 4:
			if (!str_cmp(argument, "?")
			||  (val = sn_lookup(argument)) < 0) {
				show_spells(output, -1);
				return 2;
			}
			buf_printf(output, "SPELL TYPE %d SET.\n\n", value_num);
			pObj->value[value_num] = val;
			break;
 		}
		break;

/* ARMOR for ROM: */

	case ITEM_ARMOR:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "AC PIERCE SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "AC BASH SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "AC SLASH SET.\n\n");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "AC EXOTIC SET.\n\n");
			pObj->value[3] = atoi(argument);
			break;
		}
		break;

/* WEAPONS changed in ROM */

	case ITEM_WEAPON:
		switch (value_num) {
		case 0:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(weapon_class, argument)) < 0) {
				show_flags_buf(output, weapon_class);
				return 2;
			}
			buf_add(output, "WEAPON CLASS SET.\n\n");
			pObj->value[0] = val;
			break;
		case 1:
			buf_add(output, "NUMBER OF DICE SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "TYPE OF DICE SET.\n\n");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			if (!str_cmp(argument, "?")
			||  (val = attack_lookup(argument)) < 0) {
				show_attack_types(output);
				return 2;
			}
			buf_add(output, "WEAPON TYPE SET.\n\n");
			pObj->value[3] = val;
			break;
		case 4:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(weapon_type2, argument)) < 0) {
				show_flags_buf(output, weapon_type2);
				return 2;
			}
			buf_add(output, "SPECIAL WEAPON TYPE TOGGLED.\n\n");
			TOGGLE_BIT(pObj->value[4], val);
			break;
		}
		break;

	case ITEM_PORTAL:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "CHARGES SET.\n\n");
				     pObj->value[0] = atoi(argument);
			break;
		case 1:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(exit_flags, argument)) < 0) {
				show_flags_buf(output, exit_flags);
				return 2;
			}
			buf_add(output, "EXIT flag_tS TOGGLED.\n\n");
			TOGGLE_BIT(pObj->value[1], val);
			break;
		case 2:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(portal_flags, argument)) < 0) {
				show_flags_buf(output, portal_flags);
				return 2;
			}
			buf_add(output, "PORTAL flag_tS TOGGLED.\n\n");
			TOGGLE_BIT(pObj->value[2], val);
			break;
		case 3:
			buf_add(output, "EXIT VNUM SET.\n\n");
			pObj->value[3] = atoi(argument);
			break;
		}
		break;

	case ITEM_FURNITURE:
		switch (value_num) {
		case 0:
			buf_add(output, "NUMBER OF PEOPLE SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "MAX WEIGHT SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(furniture_flags, argument)) < 0) {
				show_flags_buf(output, furniture_flags);
				return 2;
			}
		        buf_add(output, "FURNITURE flag_tS TOGGLED.\n\n");
			TOGGLE_BIT(pObj->value[2], val);
			break;
		case 3:
			buf_add(output, "HEAL BONUS SET.\n\n");
			pObj->value[3] = atoi(argument);
			break;
		case 4:
			buf_add(output, "MANA BONUS SET.\n\n");
			pObj->value[4] = atoi(argument);
			break;
		}
		break;
		   
	case ITEM_CONTAINER:
		switch (value_num) {
		case 0:
			buf_add(output, "WEIGHT CAPACITY SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			if (!str_cmp(argument, "?")
			||  (val = flag_value(cont_flags, argument)) < 0) {
				show_flags_buf(output, cont_flags);
				return 2;
			}
			buf_add(output, "CONTAINER TYPE SET.\n\n");
			TOGGLE_BIT(pObj->value[1], val);
			break;
		case 2:
			if (atoi(argument) != 0) {
				if (!get_obj_index(atoi(argument))) {
					buf_add(output, "THERE IS NO SUCH ITEM.\n\n");
					return 1;
				}

				if (get_obj_index(atoi(argument))->item_type != ITEM_KEY) {
					buf_add(output, "THAT ITEM IS NOT A KEY.\n\n");
					return 1;
				}
			}
			buf_add(output, "CONTAINER KEY SET.\n\n");
			pObj->value[2] = atoi(argument);
			break;
		case 3:
			buf_add(output, "CONTAINER MAX WEIGHT SET.\n");
			pObj->value[3] = atoi(argument);
			break;
		case 4:
			buf_add(output, "WEIGHT MULTIPLIER SET.\n\n");
			pObj->value[4] = atoi(argument);
			break;
		}
		break;

	case ITEM_DRINK_CON:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			if (!str_cmp(argument, "?")
			||  (val = liq_lookup(argument)) < 0) {
				show_liq_types(output);
				return 2;
			}
			buf_add(output, "LIQUID TYPE SET.\n\n");
			pObj->value[2] = val;
			break;
		case 3:
			buf_add(output, "POISON VALUE TOGGLED.\n\n");
			pObj->value[3] = !pObj->value[3];
			break;
		}
		break;

	case ITEM_FOUNTAIN:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "MAXIMUM AMOUT OF LIQUID HOURS SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "CURRENT AMOUNT OF LIQUID HOURS SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			if (!str_cmp(argument, "?")
			||  (val = liq_lookup(argument)) < 0) {
				show_liq_types(output);
				return 2;
			}
			buf_add(output, "LIQUID TYPE SET.\n\n");
			pObj->value[2] = val;
			break;
		}
		break;
			    	
	case ITEM_FOOD:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "HOURS OF FOOD SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "HOURS OF FULL SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 3:
			buf_add(output, "POISON VALUE TOGGLED.\n\n");
			pObj->value[3] = !pObj->value[3];
			break;
		}
		break;

	case ITEM_MONEY:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "SILVER AMOUNT SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "GOLD AMOUNT SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		}
		break;
	case ITEM_HORSE_TICKET:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "HORSE VNUM SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		case 1:
			buf_add(output, "MAX CHARGE SET.\n\n");
			pObj->value[1] = atoi(argument);
			break;
		case 2:
			buf_add(output, "CURRENT CHARGE SET.\n\n");
			pObj->value[2] = atoi(argument);
			break;
		}
		break;
	case ITEM_DIG:
		switch (value_num) {
		default:
			return 1;
		case 0:
			buf_add(output, "PERCENT DIG SET.\n\n");
			pObj->value[0] = atoi(argument);
			break;
		}
		break;
	}

	return 0;
}

/*****************************************************************************
 Name:		show_spells
 Purpose:	Displays all spells.
 ***************************************************************************/
static void show_spells(BUFFER *output, int tar)
{
	int  sn;
	int  col;
 
	col = 0;
	for (sn = 0; sn < skills.nused; sn++) {
		skill_t *sk = SKILL(sn);

		if (!str_cmp(sk->name, "reserved") || sk->spell == NULL)
			continue;

		if (tar == -1 || sk->spell->target == tar) {
			buf_printf(output, "%-19.18s", sk->name);
			if (++col % 4 == 0)
				buf_add(output, "\n");
		}
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n");
}

void show_liqlist(CHAR_DATA *ch)
{
	int liq;
	BUFFER *buffer;
	
	buffer = buf_new(-1);
	
	for (liq = 0; liq_table[liq].liq_name != NULL; liq++) {
		if ((liq % 21) == 0)
			buf_add(buffer,"Name                 Color          Proof Full Thirst Food Ssize\n");

		buf_printf(buffer, "%-20s %-14s %5d %4d %6d %4d %5d\n",
			liq_table[liq].liq_name,liq_table[liq].liq_color,
			liq_table[liq].liq_affect[0],liq_table[liq].liq_affect[1],
			liq_table[liq].liq_affect[2],liq_table[liq].liq_affect[3],
			liq_table[liq].liq_affect[4]);
	}

	page_to_char(buf_string(buffer), ch);
	buf_free(buffer);
}

#if 0
	{ "type",	item_types,	"Types of objects."		},
	{ "extra",	extra_flags,	"Object attributes."		},
	{ "wear",	wear_flags,	"Where to wear object."		},
	{ "wear-loc",	wear_loc_flags,	"Where mobile wears object."	},
	{ "container",	cont_flags,"Container status."		},

/* ROM specific bits: */

	{ "armor",	ac_type,	"Ac for different attacks."	},
	{ "apply",	apply_flags,	"Apply flags"			},
	{ "wclass",     weapon_class,   "Weapon class."                }, 
	{ "wtype",      weapon_type2,   "Special weapon type."         },
	{ "portal",	portal_flags,	"Portal types."		},
	{ "furniture",	furniture_flags,"Furniture types."		},
	{ "liquid",	liq_table,	"Liquid types."		},
	{ "apptype",	apply_types,	"Apply types."			},
	{ "weapon",	attack_table,	"Weapon types."		},
	{ NULL,		NULL,		 NULL				}
};
#endif

VALIDATE_FUN(validate_condition)
{
	int val = *(int*) arg;

	if (val < 0 || val > 100) {
		char_puts("ObjEd: condition can range from 0 (ruined) "
			  "to 100 (perfect).\n", ch);
		return FALSE;
	}
	return TRUE;
}

/*
 *	System of VC (balance of objects power) - Virtual Cool
 *			[*** Xor ***   for Shadow Realms 2001]
 */

struct bits_cost_vc_t
{
	flag64_t	bit;
	float		cost;
};

struct bits_cost_vc_t bits_cost_vc_aff[] =	// affects
{
	{ AFF_BLIND,		-2	},
	{ AFF_INVIS,		1.5	},
	{ AFF_DETECT_EVIL,	0.4	},
	{ AFF_DETECT_INVIS,	0.8	},
	{ AFF_DETECT_MAGIC,	0.5	},
	{ AFF_DETECT_HIDDEN,	1.5	},
	{ AFF_DETECT_GOOD,	0.4	},
	{ AFF_SANCTUARY,	6.5	},
	{ AFF_FAERIE_FIRE,	-1	},
	{ AFF_INFRARED,		0.4	},
	{ AFF_CURSE,		-1.5	},
	{ AFF_CORRUPTION,	-2.5	},
	{ AFF_POISON,		-1.2	},
	{ AFF_PROTECT_EVIL,	2  	},
	{ AFF_PROTECT_GOOD,	2	},
	{ AFF_SNEAK,		0.7	},
	{ AFF_HIDE,		2	},
	{ AFF_SLEEP,		0	},
	{ AFF_CHARM,		0	},
	{ AFF_FLYING,		1.4	},
	{ AFF_PASS_DOOR,	1	},
	{ AFF_HASTE,		2.7	},
	{ AFF_CALM,		0	},
	{ AFF_PLAGUE,		-1.6	},
	{ AFF_WEAKEN,		-1.5	},
//	{ AFF_DARK_VISION,	0	},
	{ AFF_BERSERK,		1.2	},
//	{ AFF_SWIM,		0	},
	{ AFF_REGENERATION,	1	},
	{ AFF_SLOW,		-2	},
	{ AFF_CAMOUFLAGE,	1.2	},
	{ AFF_IMP_INVIS,	3	},
	{ AFF_FADE,		3.5	},
	{ AFF_SCREAM,		0	},
	{ AFF_BLOODTHIRST,	-1	},
	{ AFF_STUN,		-2.5	},
	{ AFF_WEAK_STUN,	0	},
	{ AFF_DETECT_IMP_INVIS,	2	},
	{ AFF_DETECT_FADE,	2.8	},
	{ AFF_DETECT_UNDEAD,	0.4	},
	{ AFF_DETECT_FEAR,	-1	},
//	{ AFF_DETECT_FORM_TREE,	0	},
//	{ AFF_DETECT_FORM_GRASS,0	},
	{ AFF_DETECT_WEB,	0	},
	{ AFF_DETECT_LIFE,	0.8	},
	{ AFF_ACUTE_VISION,	0.9  	},
	{ AFF_BLACK_SHROUD,	6	},
	{ AFF_BLESS,		1.7	},
	{ AFF_AQUABREATH,	1.1	},

	{ 0,			0 }
};

/*
 *	This is table for cost of resist bits, but
 *	immune is cost resist * 5, and vuln is resist * (-1)
 */
struct bits_cost_vc_t bits_cost_vc_irv[] =
{
	{ FORCE_MAGIC,		5	},
	{ MAGIC_WATER,		0.5	},
	{ MAGIC_AIR,		0.6	},
	{ MAGIC_EARTH,		0.5	},
	{ MAGIC_FIRE,		0.6	},
	{ FORCE_DEATH,		0.5	},	// MAGIC_LIFE
	{ MAGIC_MENTAL,		0.6	},

	{ FORCE_HOLY,		1	},
	{ FORCE_BASH,		1.75	},
	{ FORCE_PIERCE,		1.75	},
	{ FORCE_SLASH,		1.75	},
	{ FORCE_WEAPON,		7	},
	{ FORCE_WOOD,		1	},
	{ FORCE_SILVER,		1	},
	{ FORCE_IRON,		1	},
	{ FORCE_STEAL,		0.5	}, // :))
	{ FORCE_POISON,		0.4	},
	{ FORCE_COLD,		0.6	},
	{ FORCE_DISEASE,	1	},
	{ FORCE_LIGHT,		0.8	},
	{ FORCE_ENERGY,		1	},
	{ FORCE_ACID,		0.8	},
	{ FORCE_DROWNING,	0.7	},
	
	{ 0 /*for end cycle*/,	0	 }
};  

struct bits_cost_vc_t bits_cost_extra_flags[] =
{
	{ ITEM_GLOW,		0.01	},
	{ ITEM_HUM,		0.015	},
	{ ITEM_DARK,		0	},
	{ ITEM_LOCK,		0	},
	{ ITEM_EVIL,		0	},
	{ ITEM_INVIS,		0.1	},
	{ ITEM_MAGIC,		0.02	},
	{ ITEM_NODROP,		-0.03	},
	{ ITEM_BLESS,		0.1	},
	{ ITEM_ANTI_GOOD,	-0.05	},
	{ ITEM_ANTI_EVIL,	-0.05	},
	{ ITEM_ANTI_NEUTRAL,	-0.06	},
	{ ITEM_NOREMOVE,	0.01	},
//	{ ITEM_INVENTORY,	0	},
//	{ ITEM_INFINITY_SELL,	0	},
	{ ITEM_NOPURGE,		0	},
	{ ITEM_ROT_DEATH,	-0.02	},
	{ ITEM_VIS_DEATH,	0	},
	{ ITEM_NOSAC,		0	},
	{ ITEM_NONMETAL,	-0.02	},
	{ ITEM_NOLOCATE,	0.03	},
	{ ITEM_MELT_DROP,	-0.01	},
	{ ITEM_HAD_TIMER,	0	},
	{ ITEM_SELL_EXTRACT,	0	},
	{ ITEM_BURN_PROOF,	0.2	},
	{ ITEM_NOUNCURSE,	-0.03	},
	{ ITEM_NOSELL,		0	},
	{ ITEM_NOT_EDIBLE,	0	},
	//{ ITEM_CLAN,		0	},
	{ ITEM_QUIT_DROP,	-0.15	},
	{ ITEM_PIT,		0	},
//	{ ITEM_ENCHANTED,	0	},

	{ 0,			0	}
};

struct bits_cost_vc_t bits_cost_weapon_flags[] =
{
	{ WEAPON_FLAMING,	0.55	},
	{ WEAPON_FROST,		0.5	},
	{ WEAPON_VAMPIRIC,	0.4	},
	{ WEAPON_SHARP,		0.5	},
	{ WEAPON_VORPAL,	0.35	},
	{ WEAPON_TWO_HANDS,	-0.65	},
	{ WEAPON_SHOCKING,	0.5	},
	{ WEAPON_POISON,	0.4	},
	{ WEAPON_HOLY,		0.5	},
	{ WEAPON_KATANA,	-0.1	},

	{ 0,			0 }
};

struct bits_cost_vc_t ints_cost_damage_type[] =	/* Not bits !!! */
{
	{ DAM_NONE,	0.5	},	/* First cost not zero(0) !!! */
	{ DAM_BASH,	0	},
	{ DAM_PIERCE,	0	},
	{ DAM_SLASH,	0	},
	{ DAM_FIRE,	0.1	},
	{ DAM_COLD,	0.1	},
	{ DAM_LIGHTNING,0.1	},
	{ DAM_ACID,	0.1	},
	{ DAM_POISON,	0.05	},
	{ DAM_NEGATIVE,	0.1	},
	{ DAM_HOLY,	0.11	},
	{ DAM_ENERGY,	0.1	},
	{ DAM_MENTAL,	0.1	},
	{ DAM_DISEASE,	0.05	},
	{ DAM_DROWNING,	0.05	},
	{ DAM_LIGHT,	0.05	},
	{ DAM_OTHER,	0.02	},
	{ DAM_HARM,	0.02	},
	{ DAM_CHARM,	0.02	},
	{ DAM_SOUND,	0.02	},
	{ DAM_THIRST,	0.01	},
	{ DAM_HUNGER,	0.01	},
	{ DAM_LIGHT_V,	0.01	},
	{ DAM_TRAP_ROOM,0.01	},
	
	{ 0,		0 }
};

static float get_cost_bits(flag64_t bits, struct bits_cost_vc_t *table)
{
	float cost = 0;
	for (;table->bit && bits; table++)
	{
		if (IS_SET(bits, table->bit))
		{
			REMOVE_BIT(bits, table->bit);
		   	cost += table->cost;
		}
	}
	return cost;
}

float get_cost_ave_weapon(int level, int v1, int v2)	/* level isn't 0 !!! */
{
	float ave = (1 + v2) * v1 / 2;
	float vc;	// 1 - is ave for 1 VC
	/*
	 *	Before 20 level, average not top 16,
	 *	and after standart = objlevel * 0.75
	 *
	 *	Итак. Оружие 90 level'a с average 40 - 1.5 VC
	 *	Но! До 30 level'а VC расчитываеться по соотношению:
	 *		30 level: average 20 - 1.5 VC
	 *	Соотв-о HP у мобов порезать % на 35-40 (не в 2 раза! меньше)
	 */

	/*
	standart = (level > 20 || ave > 16) ?
		(float) ave / (level * 0.75) :
		(float) ave / UMAX(level * 0.85, 6);

	return standart > 1.3 ?
		1.2 * standart :
		standart;
	*/
	if (level > 30)
	{
		if (ave >= 20)
			vc = (float) 45 / level
			+ (float) (ave - 20) / (level * 0.22 /*(20/60)/1.5*/);
		else
			vc = (float) ave / (level * 0.45);
	} else {
		if (ave > 20)
			vc = 1.5 + (float) (ave - 20) / (level * 0.20);
		else if (ave < 6 && level < 5)
			vc = 0.3 * ave;
		else
			vc = (float) ave / (level * 0.45 /*(20/30)/1.5*/);
	}
	return vc;
}

float get_obj_vc(OBJ_INDEX_DATA *obj, int level)
{
	float vc, vc_tmp;
	int cnt;
	AFFECT_DATA *paf;
	flag64_t bits;
	int s_e, s_f, s_a, s_w, s_l, s_m;

	if (obj == NULL)
		return -1;
	vc = 0;
	level = URANGE(1, level, 100);	// !!!
	s_e = s_f = s_a = s_w = s_l = s_m = 0;

	switch(obj->item_type) {
		default:
			break;
		case ITEM_LIGHT:
			vc += obj->value[2] == -1 ? 5
				: (float) obj->value[2] / 168;
			break;
		case ITEM_WAND:
			vc += 0.1;
			break;
		case ITEM_STAFF:
			vc += 0.2;
			//vc += (float) obj->value[2] / 20;
			break;
		case ITEM_WEAPON:
			vc += get_cost_ave_weapon(level, obj->value[1], obj->value[2]);
			vc += get_cost_bits(obj->value[4], bits_cost_weapon_flags);
			switch (obj->value[0])	/* weapon type */
			{
				default:
					break;
				case WEAPON_SWORD:
					vc += 0.15;
					break;
				case WEAPON_DAGGER:
					vc += 0.1;
					break;
				case WEAPON_BOW:
				case WEAPON_MACE:
					vc += 0.05;
					break;
			}
			/*
			 *	Weapon damage type.
			 */
			{
				struct bits_cost_vc_t *table;

				for(table = ints_cost_damage_type;
				   table->bit || table->cost; table++)
					if (table->bit == obj->value[3])
					{
						vc += table->cost;
						break;
					}
			}
			break;
		case ITEM_CONTAINER:
			vc += 0.4;
			/*
			vc += (float) obj->value[3] / 200;
			vc += obj->value[4] > 50 ?
				(100 - UMIN(100, (float) obj->value[4])) / 100 :
				(50 - UMAX(0, (float) obj->value[4])) / 50 + 0.5;
			*/
			break;
		case ITEM_DRINK_CON:
			vc += 0.2;
			//vc += (float) obj->value[0] / 300;
			break;
 		case ITEM_ARMOR:
			for (cnt = 0; cnt < 3; cnt++)
				vc += obj->value[cnt] <= 0.5 * level ? ((float) obj->value[cnt] * 0.15) / level
					: 0.5 * 0.15 + (((float) obj->value[cnt] - 0.5 * (float) level) * 0.2) / level;
			vc += obj->value[3] <= 0.33 * level ? ((float) obj->value[3] * 0.17) / level
				: 0.33 * 0.17 + (((float) obj->value[3] - 0.33 * (float) level) * 0.25) / level;
			break;
	}
	
	if (!str_cmp(obj->material, "platinium")
	|| !str_cmp(obj->material, "titanium"))
		vc += 0.175;

	vc += get_cost_bits(obj->extra_flags, bits_cost_extra_flags);

		/* 20 weight = 1 KG */
	if (obj->weight <= 100)
		vc += (100 - (float) obj->weight) / 500;	/* change after foot -> kg */
	else
		vc -= ((float) obj->weight - 100) / 1500;

	for (cnt = 0, paf = obj->affected; paf; paf = paf->next) {
		if (paf->where != -1 && (bits = paf->bitvector))
		   switch(paf->where)
		   {
		   	default:
		   		break;
		   	case TO_AFFECTS:
		   	case TO_IMMUNE:
		   	case TO_RESIST:
		   	case TO_VULN:
		   		vc_tmp = get_cost_bits(bits, paf->where == TO_AFFECTS ? bits_cost_vc_aff : bits_cost_vc_irv);
		   		if (paf->where == TO_IMMUNE)
		   			vc_tmp *= 5;
		   		else if (paf->where == TO_VULN)
		   			vc_tmp = - vc_tmp;
		   		vc += vc_tmp;
		   		break;
		   	
		   }

		switch (paf->location)
		{
			case APPLY_STR:
			case APPLY_DEX:
			case APPLY_INT:
			case APPLY_WIS:
			case APPLY_CON:
				vc += paf->modifier;
				break;
			case APPLY_CHA:
				vc += 0.88 * paf->modifier;
				break;
			case APPLY_SEX:
				vc += 1.5;
				break;
			case APPLY_LEVEL:
				vc += paf->modifier;
				break;
			case APPLY_AGE:
				vc -= 0.2 * paf->modifier;
				break;
			case APPLY_MANA:
				vc += 0.33 * (float) paf->modifier / level;
				break;
			case APPLY_HIT:
				vc += 0.47 * (float) paf->modifier / level;
				break;
			case APPLY_MOVE:
				vc += 3.45 * (float) paf->modifier / level;
				break;
			case APPLY_GOLD:
				break;
			case APPLY_EXP:
				vc += 0.125 * paf->modifier;
				break;
			case APPLY_AC:
				vc += 0.7 * (float) paf->modifier / level;
				break;
			case APPLY_HITROLL:
				vc += 6.3 * (float) paf->modifier / level;
				break;
			case APPLY_DAMROLL:
				vc += 3.33 * (float) paf->modifier / level;
				break;
			case APPLY_SAVES:
				vc += 4 * (float) paf->modifier / level;
				break;
			case APPLY_SAVING_EARTH:
				s_e += paf->modifier;
				break;
			case APPLY_SAVING_FIRE:
				s_f += paf->modifier;
				break;
			case APPLY_SAVING_AIR:
				s_a += paf->modifier;
				break;
			case APPLY_SAVING_WATER:
				s_w += paf->modifier;
				break;
			case APPLY_SAVING_LIFE:
				s_l += paf->modifier;
				break;
			case APPLY_SAVING_MENTAL:
				s_m += paf->modifier;
				break;
			case APPLY_SPELL_AFFECT:
				vc += 0.8 * paf->modifier;
				break;
			case APPLY_SIZE:
				vc += 1.5 * paf->modifier;
				break;
			case APPLY_CLASS:
				vc += 1.5;
				break;
			case APPLY_RACE:
				vc += 1.5;
				break;
			case APPLY_10_SKILL:
				vc += 0.7 * paf->modifier;
				break;
			case APPLY_5_SKILL:
				vc += 0.4 * paf->modifier;
				break;
			default:
				break;
		}
	}
	
	if (s_e)
		vc += s_e <= 0.07 * level ? ((float) s_e * 0.65) / level
			: 0.07 * 0.65 + (((float) s_e - 0.07 * (float) level) * 0.9) / level;
	if (s_f)
		vc += s_f <= 0.07 * level ? ((float) s_f * 0.7) / level
			: 0.07 * 0.7 + (((float) s_f - 0.07 * (float) level) * 1) / level;
	if (s_a)
		vc += s_a <= 0.07 * level ? ((float) s_a * 0.7) / level
			: 0.07 * 0.7 + (((float) s_a - 0.07 * (float) level) * 1) / level;
	if (s_w)
		vc += s_w <= 0.07 * level ? ((float) s_w * 0.65) / level
			: 0.07 * 0.65 + (((float) s_w - 0.07 * (float) level) * 0.9) / level;
	if (s_l)
		vc += s_l <= 0.07 * level ? ((float) s_l * 0.65) / level
			: 0.07 * 0.65 + (((float) s_l - 0.07 * (float) level) * 0.9) / level;
	if (s_m)
		vc += s_m <= 0.07 * level ? ((float) s_m * 0.7) / level
			: 0.07 * 0.7 + (((float) s_m - 0.07 * (float) level) * 1) / level;
	return vc;
}

