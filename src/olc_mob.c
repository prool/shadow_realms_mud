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
 * $Id: olc_mob.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "olc.h"
#include "db.h"

#define EDIT_MOB(ch, mob)	(mob = (MOB_INDEX_DATA*) ch->desc->pEdit)

/*
 * Mobile Editor Prototypes
 */
DECLARE_OLC_FUN(mobed_create		);
DECLARE_OLC_FUN(mobed_edit		);
DECLARE_OLC_FUN(mobed_touch		);
DECLARE_OLC_FUN(mobed_show		);
DECLARE_OLC_FUN(mobed_list		);

DECLARE_OLC_FUN(mobed_name		);
DECLARE_OLC_FUN(mobed_short		);
DECLARE_OLC_FUN(mobed_long		);
DECLARE_OLC_FUN(mobed_shop		);
DECLARE_OLC_FUN(mobed_desc		);
DECLARE_OLC_FUN(mobed_level		);
DECLARE_OLC_FUN(mobed_align		);
DECLARE_OLC_FUN(mobed_spec		);
DECLARE_OLC_FUN(mobed_del		);  /* SR */

DECLARE_OLC_FUN(mobed_sex		);  /* ROM */
DECLARE_OLC_FUN(mobed_act		);  /* ROM */
DECLARE_OLC_FUN(mobed_affect		);  /* ROM */
DECLARE_OLC_FUN(mobed_ac		);  /* ROM */
DECLARE_OLC_FUN(mobed_form		);  /* ROM */
DECLARE_OLC_FUN(mobed_part		);  /* ROM */
DECLARE_OLC_FUN(mobed_imm		);  /* ROM */
DECLARE_OLC_FUN(mobed_res		);  /* ROM */
DECLARE_OLC_FUN(mobed_vuln		);  /* ROM */
DECLARE_OLC_FUN(mobed_material		);  /* ROM */
DECLARE_OLC_FUN(mobed_off		);  /* ROM */
DECLARE_OLC_FUN(mobed_size		);  /* ROM */
DECLARE_OLC_FUN(mobed_hitdice		);  /* ROM */
DECLARE_OLC_FUN(mobed_manadice		);  /* ROM */
DECLARE_OLC_FUN(mobed_damdice		);  /* ROM */
DECLARE_OLC_FUN(mobed_race		);  /* ROM */
DECLARE_OLC_FUN(mobed_startpos		);  /* ROM */
DECLARE_OLC_FUN(mobed_defaultpos	);  /* ROM */
DECLARE_OLC_FUN(mobed_gold		);  /* ROM */
DECLARE_OLC_FUN(mobed_hitroll		);  /* ROM */
DECLARE_OLC_FUN(mobed_damtype		);  /* ROM */
DECLARE_OLC_FUN(mobed_group		);  /* ROM */
DECLARE_OLC_FUN(mobed_trigadd		);  /* ROM */
DECLARE_OLC_FUN(mobed_trigdel		);  /* ROM */
DECLARE_OLC_FUN(mobed_prac		); 
//DECLARE_OLC_FUN(mobed_clan		);
DECLARE_OLC_FUN(mobed_clone		);
DECLARE_OLC_FUN(mobed_invis		);
DECLARE_OLC_FUN(mobed_fvnum		);
DECLARE_OLC_FUN(mobed_slang		);  /* SR */
DECLARE_OLC_FUN(mobed_saves		);  /* SR */

DECLARE_VALIDATE_FUN(validate_fvnum	);

olc_cmd_t olc_cmds_mob[] =
{
/*	{ command	function		args		}, */
	{ "create",	mobed_create				},
	{ "edit",	mobed_edit				},
	{ "touch",	mobed_touch				},
	{ "show",	mobed_show				},
	{ "list",	mobed_list				},

	{ "alignment",	mobed_align				},
	{ "desc",	mobed_desc				},
	{ "level",	mobed_level				},
	{ "long",	mobed_long				},
	{ "name",	mobed_name				},
	{ "shop",	mobed_shop,		shop_flags	},
	{ "short",	mobed_short				},
	{ "spec",	mobed_spec				},
	{ "delete_mo",	olced_spell_out				},
	{ "delete_mob", mobed_del				},

	{ "sex",	mobed_sex,		sex_table	},
	{ "act",	mobed_act,		act_flags	},
	{ "affect",	mobed_affect,		affect_flags	},
	{ "prac",	mobed_prac,		skill_groups	},
	{ "armor",	mobed_ac				},
	{ "form",	mobed_form,		form_flags	},
	{ "part",	mobed_part,		part_flags	},
	{ "imm",	mobed_imm,		irv_flags	},
	{ "res",	mobed_res,		irv_flags	},
	{ "vuln",	mobed_vuln,		irv_flags	},
	{ "material",	mobed_material				},
	{ "off",	mobed_off,		off_flags	},
	{ "size",	mobed_size,		size_table	},
	{ "hitdice",	mobed_hitdice				},
	{ "manadice",	mobed_manadice				},
	{ "damdice",	mobed_damdice				},
	{ "race",	mobed_race				},
	{ "startpos",	mobed_startpos,		position_table	},
	{ "defaultpos",	mobed_defaultpos,	position_table	},
	{ "wealth",	mobed_gold				},
	{ "hitroll",	mobed_hitroll				},
	{ "damtype",	mobed_damtype				},
	{ "group",	mobed_group				},
//	{ "clan",	mobed_clan				},
	{ "trigadd",	mobed_trigadd				},
	{ "trigdel",	mobed_trigdel				},
	{ "clone",	mobed_clone				},
	{ "invis",	mobed_invis				},
	{ "fvnum",	mobed_fvnum,		validate_fvnum	},
	
	{ "saves",	mobed_saves,				},
	{ "slang",	mobed_slang				},

	{ "commands",	show_commands				},
	{ "version",	show_version				},

	{ NULL }
};

static void show_spec_cmds(CHAR_DATA *ch);

OLC_FUN(mobed_create)
{
	MOB_INDEX_DATA *pMob;
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
		char_puts("MobEd: That vnum is not assigned an area.\n", ch);
		return FALSE;
	}

	if (!IS_BUILDER(ch, pArea)) {
		char_puts("MobEd: Insufficient security.\n", ch);
		return FALSE;
	}

	if (get_mob_index(value)) {
		char_puts("MobEd: Mobile vnum already exists.\n", ch);
		return FALSE;
	}

	pMob			= new_mob_index();
	pMob->vnum		= value;
		 
	if (value > top_vnum_mob)
		top_vnum_mob = value;        

	pMob->act		= ACT_NPC;
	pMob->slang		= gsn_common_slang;
	iHash			= value % MAX_KEY_HASH;
	pMob->next		= mob_index_hash[iHash];
	mob_index_hash[iHash]	= pMob;

	ch->desc->pEdit		= (void*) pMob;
	OLCED(ch)		= olced_lookup(ED_MOB);
	touch_area(pArea);
	char_puts("MobEd: Mobile created.\n", ch);
	return FALSE;
}

OLC_FUN(mobed_edit)
{
	MOB_INDEX_DATA *pMob;
	AREA_DATA *pArea;
	int value;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	value = atoi(arg);
	if ((pMob = get_mob_index(value)) == NULL) {
		char_puts("MobEd: Vnum does not exist.\n", ch);
		return FALSE;
	}

	pArea = area_vnum_lookup(pMob->vnum);
	if (!IS_BUILDER(ch, pArea)) {
		char_puts("MobEd: Insufficient security.\n", ch);
	       	return FALSE;
	}

	ch->desc->pEdit	= (void*) pMob;
	OLCED(ch)	= olced_lookup(ED_MOB);
	return FALSE;
}

OLC_FUN(mobed_touch)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return touch_vnum(pMob->vnum);
}

OLC_FUN(mobed_show)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA	*pMob;
	AREA_DATA	*pArea;
	MPTRIG *mptrig;
	BUFFER *buf;
	//clan_t *clan;
	struct attack_type *atd;
	int i;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_MOB))
			EDIT_MOB(ch, pMob);
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	}
	else {
		int value = atoi(arg);
		if ((pMob = get_mob_index(value)) == NULL) {
			char_puts("MobEd: Vnum does not exist.\n", ch);
			return FALSE;
		}
	}

	buf = buf_new(-1);

	pArea = area_vnum_lookup(pMob->vnum);
	buf_printf(buf, "Name:        [%s]\nArea:        [%5d] %s\n",
		pMob->name, pArea->vnum, pArea->name);

	buf_printf(buf, "Act:         [%s]\n",
		flag_string(act_flags, pMob->act));

	buf_printf(buf, "Vnum:        [%5d] Sex:   [%s]   Race: [%s]\n",
		pMob->vnum,
		flag_string(sex_table, pMob->sex),
		race_name(pMob->race));

	/*if (pMob->clan && (clan = clan_lookup(pMob->clan))) 
		buf_printf(buf, "Clan:        [%s]\n", clan->name);
	*/

	buf_printf(buf, "Level:       [%2d]    Align: [%4d]      Hitroll: [%2d] Dam Type:    [%s]\n",
		pMob->level,	pMob->alignment,
		pMob->hitroll, (atd = GET_ATTACK_T(pMob->dam_type)) ?
				mlstr_mval(atd->name) : "none");

	if (pMob->group)
		buf_printf(buf, "Group:       [%5d]\n", pMob->group);

	buf_printf(buf, "Hit dice:    [%2dd%-3d+%4d] ",
			 pMob->hit[DICE_NUMBER],
			 pMob->hit[DICE_TYPE],
			 pMob->hit[DICE_BONUS]);

	buf_printf(buf, "Damage dice: [%2dd%-3d+%4d] ",
			 pMob->damage[DICE_NUMBER],
			 pMob->damage[DICE_TYPE],
			 pMob->damage[DICE_BONUS]);

	buf_printf(buf, "Mana dice:   [%2dd%-3d+%4d]\n",
			 pMob->mana[DICE_NUMBER],
			 pMob->mana[DICE_TYPE],
			 pMob->mana[DICE_BONUS]);

/* ROM values end */

	buf_printf(buf, "Affected by: [%s]\n",
		flag_string(affect_flags, pMob->affected_by));

/* ROM values: */

	buf_printf(buf, "Armor:       [pierce: %d  bash: %d  slash: %d  magic: %d]\n",
		pMob->ac[AC_PIERCE], pMob->ac[AC_BASH],
		pMob->ac[AC_SLASH], pMob->ac[AC_EXOTIC]);

	buf_add(buf, "Saves vs ( all ");
	for (i = 1; i < QUANTITY_MAGIC + 1; i++)
		buf_printf(buf, " %-3.3s", flag_string(irv_flags,
			magic_schools[i].bit_force));
	buf_add(buf, " ): [");
	for (i = 0; i < QUANTITY_MAGIC + 1; i++)
		buf_printf(buf, " %3d", pMob->saving_throws[i]);
	buf_add(buf, " ]\n");

	buf_printf(buf, "Form:        [%s]\n",
		flag_string(form_flags, pMob->form));

	buf_printf(buf, "Parts:       [%s]\n",
		flag_string(part_flags, pMob->parts));

	buf_printf(buf, "Imm:         [%s]\n",
		flag_string(irv_flags, pMob->imm_flags));

	buf_printf(buf, "Res:         [%s]\n",
		flag_string(irv_flags, pMob->res_flags));

	buf_printf(buf, "Vuln:        [%s]\n",
		flag_string(irv_flags, pMob->vuln_flags));

	buf_printf(buf, "Off:         [%s]\n",
		flag_string(off_flags,  pMob->off_flags));

	buf_printf(buf, "Size:        [%s]\n",
		flag_string(size_table, pMob->size));

	buf_printf(buf, "Material:    [%s]\n",
		 pMob->material);

	buf_printf(buf, "Start pos:   [%s]\n",
		flag_string(position_table, pMob->start_pos));

	buf_printf(buf, "Default pos: [%s]\n",
		flag_string(position_table, pMob->default_pos));

	buf_printf(buf, "Wealth:      [%5d]\n", pMob->wealth);

	if (pMob->invis_level)
		buf_printf(buf, "Invis level: [%d]\n", pMob->invis_level);

	if (pMob->fvnum)
		buf_printf(buf, "Female vnum: [%d]\n", pMob->fvnum);

	// if (pMob->slang != gsn_common_slang)
	buf_printf(buf, "Slang:       [%s]\n", skill_name(pMob->slang));

/* ROM values end */

	if (pMob->spec_fun)
		buf_printf(buf, "Spec fun:    [%s]\n",  spec_name(pMob->spec_fun));
	if (pMob->practicer)
		buf_printf(buf, "Practicer:   [%s]\n",
			flag_string(skill_groups, pMob->practicer));

	mlstr_dump(buf, "Short descr: ", pMob->short_descr);
	mlstr_dump(buf, "Long descr: ", pMob->long_descr);
	mlstr_dump(buf, "Description: ", pMob->description);

	if (pMob->pShop) {
		SHOP_DATA *pShop;
		int iTrade;
		int *ip;

		pShop = pMob->pShop;

		buf_printf(buf, "Shop data for [%5d]:\n"
				"  Markup for purchaser: %d%%\n"
				"  Markdown for seller:  %d%%\n",
			pShop->keeper, pShop->profit_buy, pShop->profit_sell);
		buf_printf(buf, "  Hours: %d to %d.\n",
			pShop->open_hour, pShop->close_hour);
		buf_printf(buf, "  Flags: [%s]\n",
			flag_string(shop_flags, pShop->flags));

		for (iTrade = 0; iTrade < MAX_TRADE; iTrade++) {
			if (pShop->buy_type[iTrade] != 0) {
			if (iTrade == 0) {
				buf_add(buf, "  Number Trades Type\n");
				buf_add(buf, "  ------ -----------\n");
			}
			buf_printf(buf, "  [%4d] %s\n", iTrade,
				flag_string(item_types, pShop->buy_type[iTrade]));
			}
		}
		if (pShop->vnums_of_buy.nused > 0)
		{
			buf_add(buf,	"  Vnums buy: ");
			for (iTrade = 0; iTrade < pShop->vnums_of_buy.nused; iTrade++) {
				if ((ip = VARR_GET(&pShop->vnums_of_buy, iTrade)) == NULL)
					continue;
				buf_printf(buf, " %d", *ip);
			}
			buf_add(buf,	"\n");
		}
	}

	if (pMob->mptrig_list) {
		int cnt = 0;

		buf_printf(buf, "\nMOBPrograms for [%5d]:\n", pMob->vnum);

		for (mptrig = pMob->mptrig_list; mptrig; mptrig = mptrig->next) {
			if (cnt ==0) {
				buf_add(buf, " Number Vnum Trigger Phrase [Flags]\n");
				buf_add(buf, " ------ ---- ------- ----------------------------------------------------------\n");
			}

			buf_printf(buf, "[%5d] %4d %7s %s [%s]\n", cnt,
			mptrig->vnum, flag_string(mptrig_types, mptrig->type),
			mptrig->phrase,
			flag_string(mptrig_flags, mptrig->flags));
			cnt++;
		}
	}

	page_to_char(buf_string(buf), ch);
	buf_free(buf);

	return FALSE;
}

OLC_FUN(mobed_list)
{
	MOB_INDEX_DATA	*pMobIndex;
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

	buffer = buf_new(-1);
	fAll    = !str_cmp(arg, "all");
	found   = FALSE;

	for (vnum = pArea->min_vnum; vnum <= pArea->max_vnum; vnum++) {
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
			if (fAll || is_name(arg, pMobIndex->name)) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %-17.16s",
					   pMobIndex->vnum,
					   mlstr_mval(pMobIndex->short_descr));
				if (++col % 3 == 0)
					buf_add(buffer, "\n");
			}
		}
	}

	if (!found) 
		char_puts("MobEd: No mobiles in this area.\n", ch);
	else {
		if (col % 3 != 0)
			buf_add(buffer, "\n");

		page_to_char(buf_string(buffer), ch);
	}

	buf_free(buffer);
	return FALSE;
}

OLC_FUN(mobed_del)
{
	MOB_INDEX_DATA *pMob;
	int i;
	bool error = FALSE;
	CHAR_DATA *mob, *mob_next;
	AREA_DATA *area;
	MPTRIG *mptrig;
	
	EDIT_MOB(ch, pMob);
	
	if (olced_busy(ch, ED_MOB, pMob, NULL))
		return FALSE;
	/* check that pMob is not in resets */
	for (i = 0; i < MAX_KEY_HASH; i++) {
		ROOM_INDEX_DATA *room;
		
		for (room = room_index_hash[i]; room; room = room->next) {
			int j = 0;
			RESET_DATA *reset;
			
			for (reset = room->reset_first; reset;
					reset = reset->next) {
				j++;
				if (reset->command == 'M' && reset->arg1 == pMob->vnum)
				{
					if (!error) {
						error = TRUE;
						char_puts("MobEd: can't delete mob "
							  "index: delete the following resets:\n", ch);
					}
					char_printf(ch, "MobEd: room %d, reset %d\n", 
						room->vnum, j);
				}
			}
		}
	}
	
	if (error)
		return FALSE;

	/* print warnings about mpcode */
	for(mptrig = pMob->mptrig_list; mptrig; mptrig = mptrig->next)
		if (mpcode_lookup(mptrig->vnum) != NULL) {
			if (!error) {
				error = TRUE;
				char_puts("MobEd: warning, mob index has folowing mptrig code vnum:\n", ch);
			}
			char_printf(ch, "MobEd: mpcode vnum %d\n", mptrig->vnum);
		}

	/* delete all the instances of mob index */
	for (mob = char_list; mob; mob = mob_next) {
		mob_next = mob->next;
		
		if (!IS_NPC(mob))
			continue;		// :)
		if (mob->pIndexData == pMob)
			extract_char(mob, 0);
	}
	if ((area = area_vnum_lookup(pMob->vnum)))
		touch_area(area);
	i = pMob->vnum % MAX_KEY_HASH;
	if (pMob == mob_index_hash[i]) {
		mob_index_hash[i] = pMob->next;
	} else {
		MOB_INDEX_DATA *prev;
		
		for (prev = mob_index_hash[i]; prev; prev = prev->next)
			if (prev->next == pMob)
				break;
		if (prev)
			prev->next = pMob->next;
	}
	free_mob_index(pMob);
	char_puts("MobEd: Mob index deleted.\n", ch);
	edit_done(ch->desc);
	return FALSE;
}

OLC_FUN(mobed_spec)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);

	if (argument[0] == '\0') {
		char_puts("Syntax:  spec [special function]\n", ch);
		return FALSE;
	}

	if (!str_cmp(argument, "?")) {
		show_spec_cmds(ch);
		return FALSE;
	}

	if (!str_cmp(argument, "none")) {
		 pMob->spec_fun = NULL;

		 char_puts("Spec removed.\n", ch);
		 return TRUE;
	}

	if (spec_lookup(argument)) {
		pMob->spec_fun = spec_lookup(argument);
		char_puts("Spec set.\n", ch);
		return TRUE;
	}

	char_puts("MobEd: No such special function.\n", ch);
	return FALSE;
}

OLC_FUN(mobed_damtype)
{
	char arg[MAX_INPUT_LENGTH];
	int dt;
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Syntax: damtype [damage message]\n", ch);
		char_puts("Syntax: damtype ?\n", ch);
		return FALSE;
	}

	if (!str_cmp(arg, "?")) {
		BUFFER *output = buf_new(-1);
		show_attack_types(output);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return FALSE;
	}

	if ((dt = attack_lookup(arg)) < 0) {
		char_printf(ch, "MobEd: %s: unknown damtype.\n", arg);
		return FALSE;
	}

	pMob->dam_type = dt;
	char_puts("Damage type set.\n", ch);
	return TRUE;
}

OLC_FUN(mobed_align)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->alignment);
}

OLC_FUN(mobed_level)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->level);
}

OLC_FUN(mobed_desc)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstr_text(ch, argument, cmd, &pMob->description);
}

OLC_FUN(mobed_long)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstrnl(ch, argument, cmd, &pMob->long_descr);
}

OLC_FUN(mobed_short)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_mlstr(ch, argument, cmd, &pMob->short_descr);
}

OLC_FUN(mobed_name)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_name(ch, argument, cmd, &pMob->name);
}

OLC_FUN(mobed_shop)
{
	MOB_INDEX_DATA *pMob;
	char command[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];

	argument = one_argument(argument, command, sizeof(command));
	argument = one_argument(argument, arg1, sizeof(arg1));

	EDIT_MOB(ch, pMob);

	if (command[0] == '\0')
	{
		char_puts("Syntax:  shop hours [#xopening] [#xclosing]\n", ch);
		char_puts("         shop profit [#xbuying%] [#xselling%]\n", ch);
		char_puts("         shop type [#x0-4] [item type]\n", ch);
		char_puts("         shop assign\n", ch);
		char_puts("         shop remove\n", ch);
		char_puts("         shop flags\n", ch);
		char_puts("         shop buy_vnums <vnum> [<vnum> [...]]\n", ch);
		char_puts("         shop del_vnums\n", ch);
		return FALSE;
	}

	if (!str_cmp(command, "flags")) {
		if (!pMob->pShop)
		{
			char_puts("MobEd:  Debes crear un shop primero (shop assign).\n", ch);
			return FALSE;
		}
		return olced_flag32(ch, arg1, cmd, &pMob->pShop->flags);
	}

	if (!str_cmp(command, "hours"))
	{
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument))
		{
			char_puts("Syntax:  shop hours [#xopening] [#xclosing]\n", ch);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			char_puts("MobEd:  Debes crear un shop primero (shop assign).\n", ch);
			return FALSE;
		}

		pMob->pShop->open_hour = atoi(arg1);
		pMob->pShop->close_hour = atoi(argument);

		char_puts("Shop hours set.\n", ch);
		return TRUE;
	}


	if (!str_cmp(command, "profit"))
	{
		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0' || !is_number(argument))
		{
			char_puts("Syntax:  shop profit [#xbuying%] [#xselling%]\n", ch);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			char_puts("MobEd:  Debes crear un shop primero (shop assign).\n", ch);
			return FALSE;
		}

		pMob->pShop->profit_buy     = atoi(arg1);
		pMob->pShop->profit_sell    = atoi(argument);

		char_puts("Shop profit set.\n", ch);
		return TRUE;
	}


	if (!str_cmp(command, "type"))
	{
		int value;

		if (arg1[0] == '\0' || !is_number(arg1)
		|| argument[0] == '\0')
		{
			char_puts("Syntax:  shop type [#x0-4] [item type]\n", ch);
			return FALSE;
		}

		if (atoi(arg1) >= MAX_TRADE)
		{
			char_printf(ch, "MobEd:  May sell %d items max.\n", MAX_TRADE);
			return FALSE;
		}

		if (!pMob->pShop)
		{
			char_puts("MobEd:  Debes crear un shop primero (shop assign).\n", ch);
			return FALSE;
		}

		if ((value = flag_value(item_types, argument)) == 0)
		{
			char_puts("MobEd:  That type of item is not known.\n", ch);
			return FALSE;
		}

		pMob->pShop->buy_type[atoi(arg1)] = value;

		char_puts("Shop type set.\n", ch);
		return TRUE;
	}
	
	if (!str_prefix(command, "buy_vnums"))
	{
		if (arg1[0] == '\0' || !is_number(arg1))
		{
			char_puts("Syntax:  shop buy_vnums <vnum> [<vnum> [...]]\n", ch);
			return FALSE;
		}
		
		if (!pMob->pShop)
		{
			char_puts("MobEd:  Debes crear un shop primero (shop assign).\n", ch);
			return FALSE;
		}
		
		for (;;) {
			if (arg1[0] == '\0')
				break;
			if (is_number(arg1))
			{
				int vn = atoi(arg1);
				int i;
				int *ip;
				
				for (i = 0; i < pMob->pShop->vnums_of_buy.nused; i++)
					if ((ip = VARR_GET(&pMob->pShop->vnums_of_buy, i))
					&& *ip == vn)
					{
						char_printf(ch, "'%d' already present\n", vn);
						vn = -1;
						break;
					}
				if (vn > 0) {
					ip = varr_enew(&pMob->pShop->vnums_of_buy);
					*ip = vn;
				}
			} else
				char_printf(ch, "'%s' must be numeric\n", arg1);
			argument = one_argument(argument, arg1, sizeof(arg1));
		}
		char_puts("Vnum(s) added.\n", ch);
		return TRUE;
	}
	
	if (!str_prefix(command, "del_vnums"))
	{
		if (!pMob->pShop)
		{
			char_puts("MobEd:  Debes crear un shop primero (shop assign).\n", ch);
			return FALSE;
		}
		
		if (!pMob->pShop->vnums_of_buy.p || !pMob->pShop->vnums_of_buy.nused)
		{
			char_puts("MobEd: Vnums list already removed.\n", ch); 
			return FALSE;
		}
		
		varr_free(&pMob->pShop->vnums_of_buy);
		pMob->pShop->vnums_of_buy.nsize = sizeof(int);
		pMob->pShop->vnums_of_buy.nstep = 1;
		char_puts("Vnum(s) removed.\n", ch);
		return TRUE;
	}

	/* shop assign && shop delete by Phoenix */

	if (!str_prefix(command, "assign"))
	{
		if (pMob->pShop)
		{
		 	char_puts("Mob already has a shop assigned to it.\n", ch);
		 	return FALSE;
		}

		pMob->pShop		= new_shop();
		pMob->pShop->keeper	= pMob->vnum;

		char_puts("New shop assigned to mobile.\n", ch);
		return TRUE;
	}

	if (!str_prefix(command, "remove"))
	{
		SHOP_DATA *pShop;

		pShop		= pMob->pShop;
		pMob->pShop	= NULL;
		free_shop(pShop);

		char_puts("Mobile is no longer a shopkeeper.\n", ch);
		return TRUE;
	}

	mobed_shop(ch, str_empty, cmd);
	return FALSE;
}

OLC_FUN(mobed_sex)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->sex);
}

OLC_FUN(mobed_act)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag64(ch, argument, cmd, &pMob->act);
}

OLC_FUN(mobed_affect)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag64(ch, argument, cmd, &pMob->affected_by);
}

OLC_FUN(mobed_prac) 
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->practicer);
}

int slang_lookup(const char *name);
OLC_FUN(mobed_slang)
{
	MOB_INDEX_DATA *pMob;
	int slang;  
	
	EDIT_MOB(ch, pMob);
	
	if ((slang = slang_lookup(argument)) < 0)
	{
		char_puts("Unknown slang.\n", ch);
		return FALSE;
	}
	
	pMob->slang = slang;
	return TRUE;
}

int lookup_msc_by_str(const char *arg, CHAR_DATA *ch_show);
OLC_FUN(mobed_saves)
{
	MOB_INDEX_DATA *pMob;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	int msc, set;
	bool has_msc = TRUE;
	
	EDIT_MOB(ch, pMob);
	
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	
	if (arg1[0] == '\0'
	|| arg1[0] == '?'
	|| ((msc = lookup_msc_by_str(arg1, ch)) <= 0
		&& (has_msc = str_cmp(arg1, "all")))
	|| arg2[0] == '\0'
	|| !is_number(arg2))
	{
		char_puts("Syntax: saves {{all|<mschool>} <value>\n", ch);
		return FALSE;
	}
	
	if (!has_msc)
		msc = 0;

	set = atoi (arg2);
	char_printf(ch, "Saves from %s set %d.\n",
		has_msc ? flag_string(irv_flags, magic_schools[msc].bit_force)
			: "all",
		set);
	pMob->saving_throws[msc] = set;
	return TRUE;
}

OLC_FUN(mobed_ac)
{
	MOB_INDEX_DATA *pMob;
	char arg[MAX_INPUT_LENGTH];
	int pierce, bash, slash, exotic;

	do   /* So that I can use break and send the syntax in one place */
	{
		if (argument[0] == '\0')  break;

		EDIT_MOB(ch, pMob);
		argument = one_argument(argument, arg, sizeof(arg));

		if (!is_number(arg))  break;
		pierce = atoi(arg);
		argument = one_argument(argument, arg, sizeof(arg));

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			bash = atoi(arg);
			argument = one_argument(argument, arg, sizeof(arg));
		}
		else
			bash = pMob->ac[AC_BASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			slash = atoi(arg);
			argument = one_argument(argument, arg, sizeof(arg));
		}
		else
			slash = pMob->ac[AC_SLASH];

		if (arg[0] != '\0')
		{
			if (!is_number(arg))  break;
			exotic = atoi(arg);
		}
		else
			exotic = pMob->ac[AC_EXOTIC];

		pMob->ac[AC_PIERCE] = pierce;
		pMob->ac[AC_BASH]   = bash;
		pMob->ac[AC_SLASH]  = slash;
		pMob->ac[AC_EXOTIC] = exotic;
		
		char_puts("Ac set.\n", ch);
		return TRUE;
	} while (FALSE);    /* Just do it once.. */

	char_puts("Syntax:  ac [ac-pierce [ac-bash [ac-slash [ac-exotic]]]]\n"
			  "help MOB_AC  gives a list of reasonable ac-values.\n", ch);
	return FALSE;
}

OLC_FUN(mobed_form)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->form);
}

OLC_FUN(mobed_part)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->parts);
}

OLC_FUN(mobed_imm)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->imm_flags);
}

OLC_FUN(mobed_res)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->res_flags);
}

OLC_FUN(mobed_vuln)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->vuln_flags);
}

OLC_FUN(mobed_material)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_str(ch, argument, cmd, &pMob->material);
}

OLC_FUN(mobed_off)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->off_flags);
}

OLC_FUN(mobed_size)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->size);
}

OLC_FUN(mobed_hitdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, cmd, pMob->hit);
}

OLC_FUN(mobed_manadice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, cmd, pMob->mana);
}

OLC_FUN(mobed_damdice)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_dice(ch, argument, cmd, pMob->damage);
}

OLC_FUN(mobed_race)
{
	MOB_INDEX_DATA *pMob;
	int race = 0;

	if (argument[0]
	&&  (!str_prefix(argument, "unique") || (race = rn_lookup(argument)))) {
		race_t *r;
		EDIT_MOB(ch, pMob);

		pMob->race = race;
		r = RACE(race);
		pMob->act	  = r->act;
		pMob->affected_by = r->aff;
		pMob->off_flags   = r->off;
		pMob->imm_flags   = r->imm;
		pMob->res_flags   = r->res;
		pMob->vuln_flags  = r->vuln;
		pMob->form        = r->form;
		pMob->parts       = r->parts;
		pMob->slang	  = r->slang; 

		char_puts("Race set.\n", ch);
		return TRUE;
	}

	if (argument[0] == '?') {
		char_puts("Available races are:", ch);

		for (race = 0; race < races.nused; race++) {
			if ((race % 3) == 0)
				char_puts("\n", ch);
			char_printf(ch, " %-15s", RACE(race)->name);
		}

		char_puts("\n", ch);
		return FALSE;
	}

	char_puts("Syntax:  race [race]\n"
		  "Type 'race ?' for a list of races.\n", ch);
	return FALSE;
}

OLC_FUN(mobed_startpos)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->start_pos);
}

OLC_FUN(mobed_defaultpos)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_flag32(ch, argument, cmd, &pMob->default_pos);
}

OLC_FUN(mobed_gold)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->wealth);
}

OLC_FUN(mobed_hitroll)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->hitroll);
}

OLC_FUN(mobed_group)
{
	MOB_INDEX_DATA *pMob;
	MOB_INDEX_DATA *pMTemp;
	char arg[MAX_STRING_LENGTH];
	int temp;
	BUFFER *buffer;
	bool found = FALSE;
	
	EDIT_MOB(ch, pMob);
	
	if (argument[0] == '\0') {
		char_puts("Syntax: group [number]\n", ch);
		char_puts("        group show [number]\n", ch);
		return FALSE;
	}
	
	if (is_number(argument))
	{
		pMob->group = atoi(argument);
		char_puts("Group set.\n", ch);
		return TRUE;
	}
	
	argument = one_argument(argument, arg, sizeof(arg));
	
	if (!strcmp(arg, "show") && is_number(argument)) {
		if (atoi(argument) == 0) {
			char_puts("Are you crazy?\n", ch);
			return FALSE;
		}

		buffer = buf_new(-1);

		for (temp = 0; temp < 65536; temp++) {
			pMTemp = get_mob_index(temp);
			if (pMTemp && (pMTemp->group == atoi(argument))) {
				found = TRUE;
				buf_printf(buffer, "[%5d] %s\n",
					   pMTemp->vnum, pMTemp->name);
			}
		}

		if (found)
			page_to_char(buf_string(buffer), ch);
		else
			char_puts("No mobs in that group.\n", ch);

		buf_free(buffer);
		return FALSE;
	}
	
	return FALSE;
}

/*
OLC_FUN(mobed_clan)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_clan(ch, argument, cmd, &pMob->clan);
}
*/

OLC_FUN(mobed_trigadd)
{
	int value;
	MOB_INDEX_DATA *pMob;
	MPTRIG *mptrig;
	MPCODE *mpcode;
	char trigger[MAX_STRING_LENGTH];
	char num[MAX_STRING_LENGTH];

	EDIT_MOB(ch, pMob);
	argument = one_argument(argument, num, sizeof(num));
	argument = one_argument(argument, trigger, sizeof(trigger));

	if (!str_cmp(num, "?")) {
		show_flags(ch, mptrig_types);
		return FALSE;
	}

	if (!is_number(num) || trigger[0] =='\0' || argument[0] =='\0') {
		 char_puts("Syntax: trigadd [vnum] [trigger] [phrase]\n",ch);
		 return FALSE;
	}

	if ((value = flag_value(mptrig_types, trigger)) < 0) {
		char_puts("Invalid trigger type.\n"
			  "Use 'trigadd ?' for list of triggers.\n", ch);
		return FALSE;
	}

	if ((mpcode = mpcode_lookup(atoi(num))) == NULL) {
		 char_puts("No such MOBProgram.\n", ch);
		 return FALSE;
	}

	mptrig = mptrig_new(value, argument, atoi(num));
	mptrig_add(pMob, mptrig);
	char_puts("Trigger added.\n",ch);
	return TRUE;
}

OLC_FUN(mobed_trigdel)
{
	MOB_INDEX_DATA *pMob;
	MPTRIG *mptrig;
	MPTRIG *mptrig_next;
	char mprog[MAX_STRING_LENGTH];
	int value;
	int cnt = 0;

	EDIT_MOB(ch, pMob);

	one_argument(argument, mprog, sizeof(mprog));
	if (!is_number(mprog) || mprog[0] == '\0') {
		char_puts("Syntax:  trigdel [#mprog]\n",ch);
		return FALSE;
	}

	value = atoi (mprog);

	if (value < 0) {
		 char_puts("Only non-negative mprog-numbers allowed.\n",ch);
		 return FALSE;
	}

	if (!(mptrig = pMob->mptrig_list)) {
		 char_puts("MobEd:  Nonexistent trigger.\n",ch);
		 return FALSE;
	}

	if (value == 0) {
		REMOVE_BIT(pMob->mptrig_types, pMob->mptrig_list->type);
		mptrig = pMob->mptrig_list;
		pMob->mptrig_list = mptrig->next;
		mptrig_free(mptrig);
	}
	else {
		while ((mptrig_next = mptrig->next) && (++cnt < value))
			mptrig = mptrig_next;

		if (mptrig_next) {
			REMOVE_BIT(pMob->mptrig_types, mptrig_next->type);
		        mptrig->next = mptrig_next->next;
			mptrig_free(mptrig_next);
		}
		else {
		        char_puts("No such trigger.\n",ch);
		        return FALSE;
		}
	}
	mptrig_fix(pMob);

	char_puts("Trigger removed.\n", ch);
	return TRUE;
}

OLC_FUN(mobed_clone)
{
	MOB_INDEX_DATA *pMob;
	MOB_INDEX_DATA *pFrom;
	char arg[MAX_INPUT_LENGTH];
	int i;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		char_puts("Syntax: clone <vnum>\n", ch);
		return FALSE;
	}

	i = atoi(arg);
	if ((pFrom = get_mob_index(i)) == NULL) {
		char_printf(ch, "MobEd: %d: Vnum does not exist.\n", i);
		return FALSE;
	}

	EDIT_MOB(ch, pMob);
	if (pMob == pFrom) 
		return FALSE;

	free_string(pMob->name);
	pMob->name		= str_qdup(pFrom->name);
	free_string(pMob->material);
	pMob->material		= str_qdup(pFrom->material);
	mlstr_free(pMob->short_descr);
	pMob->short_descr	= mlstr_dup(pFrom->short_descr);
	mlstr_free(pMob->long_descr);
	pMob->long_descr	= mlstr_dup(pFrom->long_descr);
	mlstr_free(pMob->description);
	pMob->description	= mlstr_dup(pFrom->description);

	pMob->spec_fun		= pFrom->spec_fun;
	pMob->group		= pFrom->group;
	pMob->act		= pFrom->act;
	pMob->affected_by	= pFrom->affected_by;
	pMob->alignment		= pFrom->alignment;
	pMob->level		= pFrom->level;
	pMob->hitroll		= pFrom->hitroll;
	pMob->dam_type		= pFrom->dam_type;
	pMob->off_flags		= pFrom->off_flags;
	pMob->imm_flags		= pFrom->imm_flags;
	pMob->res_flags		= pFrom->res_flags;
	pMob->vuln_flags	= pFrom->vuln_flags;
	pMob->start_pos		= pFrom->start_pos;
	pMob->default_pos	= pFrom->default_pos;
	pMob->sex		= pFrom->sex;
	pMob->race		= pFrom->race;
	pMob->wealth		= pFrom->wealth;
	pMob->form		= pFrom->form;
	pMob->parts		= pFrom->parts;
	pMob->size		= pFrom->size;
	pMob->practicer		= pFrom->practicer;
	//pMob->clan		= pFrom->clan;
	pMob->invis_level	= pFrom->invis_level;
	pMob->fvnum		= pFrom->fvnum;
	pMob->slang		= pFrom->slang;

	for (i = 0; i < QUANTITY_MAGIC + 1; i++)
		pMob->saving_throws[i] = pFrom->saving_throws[i];
	for (i = 0; i < 3; i++)
		pMob->hit[i]	= pFrom->hit[i];
	for (i = 0; i < 3; i++)
		pMob->mana[i]	= pFrom->mana[i];
	for (i = 0; i < 3; i++)
		pMob->damage[i]	= pFrom->damage[i];
	for (i = 0; i < 4; i++)
		pMob->ac[i]	= pFrom->ac[i];

	return TRUE;
}

OLC_FUN(mobed_invis)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->invis_level);
}

OLC_FUN(mobed_fvnum)
{
	MOB_INDEX_DATA *pMob;
	EDIT_MOB(ch, pMob);
	return olced_number(ch, argument, cmd, &pMob->fvnum);
}

/* Local functions */

static void show_spec_cmds(CHAR_DATA *ch)
{
	int  spec;
	int  col;
	BUFFER *output;

	output = buf_new(-1);
	col = 0;
	buf_add(output, "Preceed special functions with 'spec_'\n\n");
	for (spec = 0; spec_table[spec].function != NULL; spec++) {
		buf_printf(output, "%-19.18s", &spec_table[spec].name[5]);
		if (++col % 4 == 0)
			buf_add(output, "\n");
	}
 
	if (col % 4 != 0)
		buf_add(output, "\n");

	char_puts(buf_string(output), ch);
	buf_free(output);
}

VALIDATE_FUN(validate_fvnum)
{
	int fvnum = *(int*) arg;

	if (!get_mob_index(fvnum)) {
		char_printf(ch, "MobEd: %d: no such vnum.\n", fvnum);
		return FALSE;
	}

	return TRUE;
}
