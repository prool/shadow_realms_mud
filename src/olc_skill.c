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
#include "skills.h"

#define EDIT_SKILL(ch, skill)	(skill = (skill_t *) ch->desc->pEdit)

DECLARE_OLC_FUN(skilled_create		);
DECLARE_OLC_FUN(skilled_edit		);
DECLARE_OLC_FUN(skilled_touch		);
DECLARE_OLC_FUN(skilled_show		);
DECLARE_OLC_FUN(skilled_list		);
DECLARE_OLC_FUN(skilled_touch		);
DECLARE_OLC_FUN(skilled_del		);

DECLARE_OLC_FUN(skilled_name		);
DECLARE_OLC_FUN(skilled_wait		);
DECLARE_OLC_FUN(skilled_dam_mess	);
DECLARE_OLC_FUN(skilled_off_mess	);
DECLARE_OLC_FUN(skilled_off_obj		);
DECLARE_OLC_FUN(skilled_flags		);
DECLARE_OLC_FUN(skilled_group		);
DECLARE_OLC_FUN(skilled_sell		);
DECLARE_OLC_FUN(skilled_minlev		);
DECLARE_OLC_FUN(skilled_minmana		);
DECLARE_OLC_FUN(skilled_spell		);
DECLARE_OLC_FUN(skilled_rpcost		);

static DECLARE_VALIDATE_FUN(validate_name);

#define SPELLED_FUN(fun)		bool fun(CHAR_DATA *ch, const char *argument, olc_cmd_t *cmd, struct spell_pt *spell)
#define DECLARE_SPELLED_FUN(fun)	SPELLED_FUN(fun)

DECLARE_SPELLED_FUN(spelled_fun		);
DECLARE_SPELLED_FUN(spelled_minpos	);
DECLARE_SPELLED_FUN(spelled_target	);
DECLARE_SPELLED_FUN(spelled_mschool	);
DECLARE_SPELLED_FUN(spelled_delay	);
DECLARE_SPELLED_FUN(spelled_type	);
DECLARE_SPELLED_FUN(spelled_component	);


olc_cmd_t olc_cmds_skill[] =
{
	{ "create",	skilled_create			},
	{ "edit",	skilled_edit			},
	{ "touch",	skilled_touch			},
	{ "show",	skilled_show			},
	{ "list",	skilled_list			},
	{ "delete_skil",olced_spell_out			},
	{ "delete_skill",skilled_del			},
	
	{ "name",	skilled_name,	validate_name	},
	{ "wait",	skilled_wait			},
	{ "dam_mess",	skilled_dam_mess		},
	{ "off_mess",	skilled_off_mess		},
	{ "off_obj",	skilled_off_obj			},
	{ "flags",	skilled_flags,	skill_flags	},
	{ "group",	skilled_group,	skill_groups	},
	{ "sell",	skilled_sell			},
	{ "levelmin",	skilled_minlev			},
	{ "manamin",	skilled_minmana			},
	{ "spell",	skilled_spell			},
	{ "remortcost",	skilled_rpcost			},
	
	{ "commands",	show_commands			},
	
	{ NULL }
};

bool touch_skill(skill_t *skill)
{
	SET_BIT(skill->flags, SKILL_CREATED);
	return FALSE;	
}

OLC_FUN(skilled_create)
{
	skill_t		*skill;
	int		sn;

	if (ch->pcdata->security < SECURITY_SKILL) {
		char_puts("SkillEd: perm den\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'OLC CREATE'");
		return FALSE;
	}

	if ((sn = sn_lookup(argument)) != -1)
	{
		char_printf(ch, "SkillEd: '%s' already present.\n", SKILL(sn)->name);
		return FALSE;
	}

	skill = varr_enew(&skills);
	skill->name = str_dup(argument);
	skill->sell.nsize = sizeof(sellskill_t);
	skill->sell.nstep = 1;
	skill->spell = NULL;
	skill->remort_cost = DEFAULT_PRICE_SKILL;
	
	ch->desc->pEdit = (void *) skill;
	OLCED(ch)	= olced_lookup(ED_SKILL);
	
	char_puts("Skill created.\n", ch);
	return FALSE;
}

OLC_FUN(skilled_edit)
{
	int		sn;

	if (ch->pcdata->security < SECURITY_SKILL) {
		char_puts("SkillEd: perm den.\n", ch);
		return FALSE;
	}

	if (argument[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}
	
	if (is_number(argument))
			sn = atoi(argument);
		else
			sn = sn_lookup(argument);
	
	if (sn < 0 || skill_lookup(sn) == NULL)
	{
		char_puts("SkillEd: skill not found.\n", ch);
		return FALSE;
	}
	
	ch->desc->pEdit = (void *) SKILL(sn);
	OLCED(ch)	= olced_lookup(ED_SKILL);
	return FALSE;
}

OLC_FUN(skilled_touch)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return touch_skill(skill);
}

OLC_FUN(skilled_show)
{
	skill_t		*skill;
//	char arg[MAX_STRING_LENGTH];
	BUFFER *output;
	int sn;
	
//	one_argument(argument, arg, sizeof(arg));
	
	if (argument[0] == '\0') {
		if (IS_EDIT(ch, ED_SKILL)) {
			EDIT_SKILL(ch, skill);
			sn = sn_lookup(skill->name);	// izvrat ;)
		} else {
			do_help(ch, "'OLC ASHOW'");
			return FALSE;
		}
	} else {
		if (is_number(argument))
			sn = atoi(argument);
		else
			sn = sn_lookup(argument);
		if (sn < 0 || (skill = skill_lookup(sn)) == NULL) {
			char_printf(ch, "SkillEd: %s: No such skill.\n", argument);
			return FALSE;
		}
	}
	output = buf_new(-1);
	
	buf_printf(output, "Name:          [%d] [%s]\n",	sn, skill->name);
	mlstr_dump(output, "Damage str:    ",		skill->noun_damage);
	mlstr_dump(output, "Off str:       ",		skill->msg_off);
	mlstr_dump(output, "Off obj str:   ",		skill->msg_obj);
	buf_printf(output, "Flags:         [%s]\n",
			flag_string(skill_flags, 	skill->flags));
	buf_printf(output, "Wait after:    [%d]\n",	skill->beats);
	//if (!IS_NULLSTR(skill->restrict_race))
	//	buf_printf(output, "Restrict race: [%s]\n",	skill->restrict_race);
	buf_printf(output, "Guild group:   [%s]\n",
			flag_string(skill_groups,	skill->group));
	buf_printf(output, "Min level:     [%d]\n",	skill->min_level);
	buf_printf(output, "Remort cost:   [%d]\n",	skill->remort_cost);
	buf_printf(output, "Mana(min):     [%d]\n",	skill->min_mana);

	if (skill->pgsn)
		buf_printf(output, "Gsn pointer:   [%s]\n",
			namedp_name(gsn_table,		skill->pgsn));
	else
		buf_printf(output, "Skill has NOT pointer to gsn.\n");
	
	if (skill->sell.nused > 0)
	{
		int iSell;
		sellskill_t *sell;

		buf_printf(output, "Research list:\n");
		for (iSell = 0; iSell < skill->sell.nused; iSell++) {
			sell = (sellskill_t *) varr_get(&skill->sell, iSell);
			buf_printf(output, "   %s VnumSeller: %6d  Cost: %6d  MinLev: %d [%s]\n",
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
	if (skill->spell)
	{
		struct spell_pt	*spell = skill->spell;
		int		i, counter = 0;
		struct spell_component *comp;
		OBJ_INDEX_DATA  *pObjIndex;
		
		buf_printf(output, "Spell data:\n");
		if (spell->spell_fun)
			buf_printf(output, "\tSpell Fun:   [%s]\n",
				namedp_name(spellfn_table,          spell->spell_fun));
		else
			buf_printf(output, "\tSpell has NOT spell function.\n");

		buf_printf(output, "\tMin pos:     [%s]\n",
			flag_string(position_table,	spell->minimum_position));
		buf_printf(output, "\tTarget:      [%s]\n",
			flag_string(skill_targets,	spell->target));
		buf_printf(output, "\tMSchool:     [%s]\n",
			flag_string(irv_flags,	spell->mschool));
		buf_printf(output, "\tWait before: [%d]\n", spell->delay);
		buf_printf(output, "\tType flags:  [%s]\n",
			flag_string(spell_flags_table,	spell->type));
		for (i = 0; i < spell->components.nused; i++)
		{
			comp = (struct spell_component *) varr_get(&spell->components, i);
			if (!(counter++))
				buf_printf(output, "\tComponent(s) need list:\n");
			pObjIndex = get_obj_index(comp->vnum);
			buf_printf(output, "\t   [%6d] [%s] %s\n",
					comp->vnum,
					flag_string(componets_flags_table, comp->flags),
					pObjIndex ? mlstr_mval(pObjIndex->short_descr): "(null)");
		}
		if (!counter)
			buf_printf(output, "\tNo need component(s)\n");
	} else {
		buf_printf(output, "This is simple skill.\n");
	}
	
	page_to_char(buf_string(output), ch);
	buf_free(output);

	return FALSE;
}

OLC_FUN(skilled_list)
{
	int i, count = 0;
	BUFFER *output;

	output = buf_new(-1);
	for (i = 0; i < skills.nused; i++)
		if (argument[0] == '\0'
		|| strstr(SKILL(i)->name, argument))
		{
			buf_printf(output, "[%3d] %s\n", i, SKILL(i)->name);
			count++;
		}

	if (!count)
		buf_printf(output, "No skills with that name found [%s].\n",
				argument);
	page_to_char(buf_string(output), ch);
	buf_free(output);
	return FALSE;
}

OLC_FUN(skilled_name)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_str(ch, argument, cmd, &skill->name);
}

OLC_FUN(skilled_wait)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_number(ch, argument, cmd, &skill->beats);
}

OLC_FUN(skilled_dam_mess)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_mlstr(ch, argument, cmd, &skill->noun_damage);
}

OLC_FUN(skilled_off_mess)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_mlstr(ch, argument, cmd, &skill->msg_off);
}

OLC_FUN(skilled_off_obj)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_mlstr(ch, argument, cmd, &skill->msg_obj);
}

OLC_FUN(skilled_flags)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_flag32(ch, argument, cmd, &skill->flags);
}

OLC_FUN(skilled_group)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_flag32(ch, argument, cmd, &skill->group);
}

OLC_FUN(skilled_minlev)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_number(ch, argument, cmd, &skill->min_level);
}

OLC_FUN(skilled_minmana)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_number(ch, argument, cmd, &skill->min_mana);
}

OLC_FUN(skilled_rpcost)
{
	skill_t *skill;
	EDIT_SKILL(ch, skill);
	return olced_number(ch, argument, cmd, &skill->remort_cost);
}

OLC_FUN(skilled_sell)
{
	char arg[MAX_INPUT_LENGTH];
	const char *errmsg = "Error. See 'sell help'\n";
	bool add;
	int type, minlevel, cost, vnum;
	sellskill_t *sskill;
	skill_t *skill;

	EDIT_SKILL(ch, skill);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if (!str_cmp(arg, "help")) {
		char_puts("Syntax: sell {{add|del|help} {{<type>|?} <minlevel> <cost silver> <vnum teacher> [<arg1>] [<arg2>] [...]\n", ch);
		return FALSE;
	}
	

	if (!str_cmp(arg, "del")) {
		add = FALSE;
	} else if (!str_cmp(arg, "add")){
		add = TRUE;
	} else {
		char_puts(errmsg, ch);
		return FALSE;
	}
	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '?')
	{
		show_flags(ch, sell_skill_type);
		return FALSE;
	} if ((type = flag_value(sell_skill_type, arg)) == -1)
	{
		char_puts("Error type\n", ch);
		return FALSE;
	}
	argument = one_argument(argument, arg, sizeof(arg));
	minlevel = atoi(arg);
	if (add && (minlevel < 0 || minlevel > MAX_LEVEL)) {
		char_puts(errmsg, ch);
		return FALSE;
	}
	argument = one_argument(argument, arg, sizeof(arg));
	cost = atoi(arg);
	if (add && (cost < 0 || cost > 30000)) {
		char_puts(errmsg, ch);
		return FALSE;
	}
	argument = one_argument(argument, arg, sizeof(arg));
	vnum = atoi(arg);
	if (add && get_mob_index(vnum) == NULL)
	{
		char_printf(ch, "No find mob for '%s'[%d]\n",
				arg, vnum);
		return FALSE;
	}
	
	if (add)
	{
		sskill = varr_enew(&skill->sell);

		free_string(sskill->name);

		sskill->flag = type;
		sskill->minlev = minlevel;
		sskill->cost = cost;
		sskill->vnum_seller = vnum;
		sskill->name = str_dup(argument);
	} else {
		int i;

		sskill = NULL;
		for (i = 0; i < skill->sell.nused; i++)
		{
			sskill = (sellskill_t *) varr_get(&skill->sell, i);

			if (sskill->flag == type
			&& sskill->minlev == minlevel
			&& sskill->cost == cost
			&& sskill->vnum_seller == vnum
			&& ((IS_NULLSTR(sskill->name) && IS_NULLSTR(argument))
				|| !str_cmp(sskill->name, argument)))
			{
				break;
			}
		}
		if (sskill == NULL || i >= skill->sell.nused)
		{
			char_puts("Can't find this sell_data for current skill.\n", ch);
			return FALSE;
		}
		varr_del(&(skill->sell), sskill);
	}
	
	char_puts("Ok\n", ch);
	return FALSE;
}

struct spell_pt *check_pres_spell(CHAR_DATA *ch, skill_t *skill)
{
	struct spell_pt *spell;

	if ((spell = skill->spell) == NULL)
	{
		char_puts("Spell data not present in this skill.\n", ch);
		return NULL;
	}
	return spell;
}

void show_list_mschool(CHAR_DATA *ch);
int lookup_msc_by_str(const char *arg, CHAR_DATA *ch_show);
OLC_FUN(skilled_spell)
{
	skill_t *skill;
	const char *errmsg = "Error. See 'spell help'\n";
	char arg[MAX_INPUT_LENGTH];
	bool retval = FALSE;
	struct spell_pt *spell;

	EDIT_SKILL(ch, skill);

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'OLC EDIT'");
		return FALSE;
	}

	if (!str_cmp(arg, "help")) {
		char_puts("Syntax: spell {{create|delete|fun|minpos|target|mschool|delay|type|component} [{{?|...}]\n", ch);
		return FALSE;
	} else if (!str_cmp(arg, "create")) {
		const namedp_t *np;
		
		if ((spell = skill->spell))
		{
			char_puts("Spell data already present.\n", ch);
			return FALSE;
		}
		if (argument[0] == '?')
		{
			char_puts("Syntax: spell create <spell fun>\n<Fun> list see by 'spell fun ?'\n", ch);
			return FALSE;
		} else if ((np = namedp_lookup(spellfn_table, argument))) {
			
			skill->spell = calloc(1, sizeof(*(skill->spell)));
			spell = skill->spell;
			spell->components.nsize = sizeof(struct spell_component);
			spell->components.nstep = 1;
			SET_BIT(spell->type, SPT_TARG_OBJECT);
			spell->spell_fun = np->p;
			retval = TRUE;
		}
	} else if (!str_cmp(arg, "delete")) {
		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;
		log_printf("Warning [spell_data del]:...");
		varr_free(&(spell->components));
		free(spell);
		skill->spell = NULL;
		retval = TRUE;
	} else if (!str_cmp(arg, "fun")) {
		const namedp_t *np;
		
		if (argument[0] == '?')
		{
			char_puts("<Fun> list:\n", ch);
			show_namedp_chbuf(ch, spellfn_table);
			return FALSE;
		}

		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;

		if ((np = namedp_lookup(spellfn_table, argument))) {
			spell->spell_fun = np->p;
			retval = TRUE;
		}
	} else if (!str_cmp(arg, "minpos")) {
		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;
		if (argument[0] == '?')
		{
			char_puts("MinPos list:\n", ch);
			show_flags(ch, position_table);
			return FALSE;
		}
		spell->minimum_position = flag_value(position_table, argument);
		retval = TRUE;
	} else if (!str_cmp(arg, "target")) {
		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;
		if (argument[0] == '?')
		{
			char_puts("SkillTargets list:\n", ch);
			show_flags(ch, skill_targets);
			return FALSE;
		}
		spell->target = flag_value(skill_targets, argument);
		retval = TRUE;
	} else if (!str_cmp(arg, "mschool")) {
		int imsc;
		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;
		if (argument[0] == '?')
		{
			char_puts("Mschool list:\n", ch);
			//show_flags(ch, irv_flags);
			show_list_mschool(ch);
			return FALSE;
		}
		if ((imsc = lookup_msc_by_str(argument, ch)) == 0)
		{
			return FALSE;
		}
		TOGGLE_BIT(spell->mschool, magic_schools[imsc].bit_force);
		char_puts("Toggled... ", ch);
		retval = TRUE;
	} else if (!str_cmp(arg, "type")) {
		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;
		if (argument[0] == '?')
		{
			char_puts("Spell flags list:\n", ch);
			show_flags(ch, spell_flags_table);
			return FALSE;
		}
		TOGGLE_BIT(spell->type, flag_value(spell_flags_table, argument));
		char_puts("Toggled... ", ch);
		retval = TRUE;
	} else if (!str_cmp(arg, "delay")) {
		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;
		spell->delay = atoi(argument);
		if (spell->delay < 0)
			spell->delay = 0;
		retval = TRUE;
	} else if (!str_cmp(arg, "component")) {
		bool add;
		int vnum, i;
		flag32_t flag;
		struct spell_component *scomp;

		if ((spell = check_pres_spell(ch, skill)) == NULL)
			return FALSE;
		argument = one_argument(argument, arg, sizeof(arg));
		if (!str_cmp(arg, "add"))
			add = TRUE;
		else if (!str_cmp(arg, "del"))
			add = FALSE;
		else /*if (argument[0] == '?' || arg[0] == '?')*/ {
			char_puts("Syntax: spell component {{add|del|?} <vnum component> <flag list>\t\tComponent flags:\n", ch);
			show_flags(ch, componets_flags_table);
			return FALSE;
		}
		argument = one_argument(argument, arg, sizeof(arg));
		vnum = atoi(arg);
		if (get_obj_index(vnum) == NULL)
		{
			char_printf(ch, "Unknow obj '%s'[%d]\n",
				arg, vnum);
			return FALSE;
		}
		flag = flag_value(componets_flags_table, argument);
		if (add)
		{
			scomp = varr_enew(&spell->components);

			scomp->vnum = vnum;
			scomp->flags = flag;
		} else {
			scomp = NULL;
			for (i = 0; i < spell->components.nused; i++)
			{
				scomp = (struct spell_component *) varr_get(&spell->components, i);
				
				if (scomp->vnum == vnum
				&& scomp->flags == flag)
					break;
			}
			if (scomp == NULL || i >= spell->components.nused)
			{
				char_puts("Can't find this comp_data for current spell.\n", ch);
				return FALSE;
			}
			varr_del(&(spell->components), scomp);
		}
		retval = TRUE;
	} else {
		char_puts(errmsg, ch);
		return FALSE;
	}

	if (retval)
		char_puts("Ok\n", ch);
	else
		char_puts("Fail\n", ch);
	return retval;
}

static VALIDATE_FUN(validate_name)
{
	int i;
	skill_t *skill;
	
	EDIT_SKILL(ch, skill);
	for (i = 0; i < skills.nused; i++) {
		if (SKILL(i) != skill
		&& !str_cmp(SKILL(i)->name, arg)) {
			char_printf(ch, "SkillEd: %s: duplicate skill name.\n",
					arg);
			return FALSE;
		}
	}
	return TRUE;
}

OLC_FUN(skilled_del)
{
	skill_t		*skill;

	EDIT_SKILL(ch, skill);
	if (olced_busy(ch, ED_SKILL, skill, NULL))
		return FALSE;
	log_printf("Warning [skill del]: may be crush if skill(spell) used by something.");

	varr_del(&skills, skill);
	char_puts("SkillEd: Skill deleted (Reboot recomended).\n", ch);
	edit_done(ch->desc);
	return FALSE;
}

