/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 *	Shadow Realms
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "olc.h"
#include "religion.h"

#define EDIT_REL(ch, rel)	(rel = (RELIGION_DATA *) ch->desc->pEdit)

DECLARE_OLC_FUN(reled_create		);
DECLARE_OLC_FUN(reled_edit		);
DECLARE_OLC_FUN(reled_touch		);
DECLARE_OLC_FUN(reled_show		);
DECLARE_OLC_FUN(reled_list		);

DECLARE_OLC_FUN(reled_name		);

static DECLARE_VALIDATE_FUN(validate_name);

olc_cmd_t olc_cmds_rel[] =
{
	{ "create",	reled_create			},
	{ "edit",	reled_edit			},
	{ "touch",	olced_dummy			},
	{ "show",	reled_show			},
	{ "list",	reled_list			},
	
	{ "name",	reled_name,	validate_name	},
	{ "commands",	show_commands			},
	
	{ NULL }
};

OLC_FUN(reled_create)
{
	return FALSE;
}

OLC_FUN(reled_edit)
{
	return FALSE;
}

OLC_FUN(reled_show)
{
	RELIGION_DATA	*rel;
	char arg[MAX_STRING_LENGTH];
	BUFFER *output;
	int vn, i;
	
	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		if (IS_EDIT(ch, ED_REL))
			EDIT_REL(ch, rel);
		else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	} else {
		if (is_number(arg))
			vn = atoi(arg);
		else
			vn = rel_lookup(arg);
		if (vn < 0 || (rel = RELIGION(vn)) == NULL) {
			char_printf(ch, "RelEd: %s: No such religion.\n", arg);
			return FALSE;
		}
	}
	output = buf_new(-1);
	
	buf_printf(output, "Name:         [%s]\n", rel->name);
	mlstr_dump(output, "Desc:         ",  rel->desc);
	buf_printf(output, "Altar:        [%d]room [%d]pit\n",
				rel->altar.room->vnum, rel->altar.pit->vnum);
	buf_printf(output, "Ghost timer:  [%d] + [%d] * level\n",
				rel->ghost_timer_default,
				rel->ghost_timer_plevel);
	buf_printf(output, "Cost:         [%d]qp + [%d]gold\n",
				rel->cost_qp,
				rel->cost_gold);
	buf_printf(output, "God:          [%d] in room [%d]\n",
			rel->god ? rel->god->pIndexData->vnum : -1,
			rel->god ? rel->godroom : -1);
	buf_printf(output, "Templeman:    [%d] vnum mob\n", rel->vnum_templeman);
	buf_printf(output, "Tattoo vnum:  [%d]\n", rel->vnum_tattoo);
	buf_printf(output, "Recall cexp:  [%d]%%\n", rel->recall_change_exp);
	buf_printf(output, "Death cexp:   [%d]%%\n", rel->change_exp_death);
	buf_printf(output, "Change buy:   [%d]%%\n", rel->change_buy);
	buf_printf(output, "Change sell:  [%d]%%\n", rel->change_sell);
	buf_printf(output, "Change sacr:  [%d]%%\n", rel->change_sacrifice);
	
	if (rel->allow_align)
		buf_printf(output,
			   "Allow align:  [%s]\n", flag_string(ralign_names, rel->allow_align));
	if (rel->flags)
		buf_printf(output,
			   "Flags:        [%s]\n", flag_string(religion_flags, rel->flags));

	if (rel->change_flags) for(i = 0; i < rel->change_faith.nused; i++)
	{
		religion_faith_change *rfc = VARR_GET(&rel->change_faith, i);
		if (!i)
			buf_add(output, "Faith change:\n");
		buf_printf(output,      "            %-25s %-7s %3d\n",
			flag_string(religion_faith_flags, rfc->that),
			rfc->is_like ? "like" : "dislike",
			rfc->value);
		
	}
	for(i = 0; i < rel->add_skills.nused; i++)
	{
		religion_add_skill *ras = VARR_GET(&rel->add_skills, i);
		if (!i)
			buf_add(output, "Add skills:\n");
		buf_printf(output, 	"            %-25s %2d %3d %3d %2d\n",
			skill_name(ras->sn), ras->level, ras->add_percent,
			ras->max_percent, ras->hard);
			
	}
	
	for(i = 0; i < rel->fight_spells.nused; i++)
	{
		religion_fight_spell *rfs = VARR_GET(&rel->fight_spells, i);
		if (!i)
			buf_add(output, "Fight spells:\n");
		buf_printf(output,	"            %-25s %3d %s %s\n",
			skill_name(rfs->sn), rfs->percent, rfs->is_for_honorable ? "honor   " : "dishonor",
			rfs->to_char ? "to_char" : "to_victim");
	}
	
//	buf_add(output, "This is may be not all information (see religion.conf for full)\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(reled_list)
{
	int i;
	
	for (i = 0; i < religions.nused; i++)
		if (argument[0] == '\0'
		|| strstr(RELIGION(i)->name, argument))
			char_printf(ch, "[%2d] %s\n", i, RELIGION(i)->name);
	return FALSE;
}

OLC_FUN(reled_name)
{
	return FALSE;
}

static VALIDATE_FUN(validate_name)
{
	int i;
	RELIGION_DATA *rel;
	
	EDIT_REL(ch, rel);
	for (i = 0; i < religions.nused; i++) {
		if (RELIGION(i) != rel
		&& !str_cmp(RELIGION(i)->name, arg)) {
			char_printf(ch, "RelEd: %s: duplicate religion name.\n",
					arg);
			return FALSE;
		}
	}
	return TRUE;
}
