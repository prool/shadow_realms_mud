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
 * $Id: db_skills.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_skill);

DBFUN dbfun_skills[] =
{
	{ "SKILL",	load_skill	},
	{ NULL }
};

DBDATA db_skills = { dbfun_skills };

DBLOAD_FUN(load_skill)
{
	skill_t *skill;

	skill = varr_enew(&skills);
	skill->sell.nsize = sizeof(sellskill_t);
	skill->sell.nstep = 1;
	skill->spell = NULL;	//!!!

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'B':
			KEY("Beats", skill->beats, fread_number(fp));
			break;
		case 'C':
			KEY("Cost", skill->remort_cost, fread_number(fp));
			if (!str_cmp(word, "Component"))
			{
				int vnum = fread_number(fp);
				flag32_t flags = fread_fstring(componets_flags_table, fp);

				if (skill->spell) {
					struct spell_component *comp
						= varr_enew(&(skill->spell->components));
					comp->vnum = vnum;
					comp->flags = flags;
				}
				fMatch = TRUE;
			}
			break;
		case 'D':
			if (!str_cmp(word, "Delay"))
			{
				int ddd = fread_number(fp);
				if (skill->spell)
					skill->spell->delay = ddd;
				fMatch = TRUE;
			}
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				if (!skill->remort_cost)
					skill->remort_cost = DEFAULT_PRICE_SKILL;
				if (IS_NULLSTR(skill->name)) {
					db_error("load_skill",
						 "skill name undefined");
					skills.nused--;
				}
				if (skill->spell)
				{
					if (!skill->spell->mschool
					|| !skill->spell->spell_fun)
						log_printf("Attention. '%s' hasn't some fields!",
						skill->name);
					if (!skill->spell->type)
						SET_BIT(skill->spell->type, SPT_TARG_OBJECT);
					if (!IS_SET(skill->spell->type,
							SPT_CHECKPOS_AFTER | SPT_CHECKPOS_BEFORE))
						SET_BIT(skill->spell->type, SPT_CHECKPOS_BEFORE);
				}
				return;
			}
			break;
		case 'F':
			KEY("Flags", skill->flags,
			    fread_fstring(skill_flags, fp));
			break;
		case 'G':
			KEY("Group", skill->group,
			    fread_fword(skill_groups, fp));
			if (!str_cmp(word, "Gsn")) {
				skill->pgsn = fread_namedp(gsn_table, fp);
				*skill->pgsn = skills.nused - 1;
				fMatch = TRUE;
			}
			break;
		case 'M':
			KEY("MinMana", skill->min_mana, fread_number(fp));
//			KEY("MinPos", skill->minimum_position,
//			    fread_fword(position_table, fp));
			if (!str_cmp(word, "MinPos"))
			{
				flag32_t ttt = fread_fword(position_table, fp);
				if (skill->spell)
					skill->spell->minimum_position = ttt;
				fMatch = TRUE;
			}
			KEY("MinLev", skill->min_level,fread_number(fp));
//			KEY("Msc", skill->mschool,
//					fread_fstring(irv_flags, fp));
			if (!str_cmp(word, "Msc") || !str_cmp(word, "Mschool"))
			{
				flag32_t ttt = fread_fstring(irv_flags, fp);
				if (skill->spell)
					skill->spell->mschool = ttt;
				fMatch = TRUE;
			}
//			KEY("Mschool", skill->mschool,
//					fread_fstring(irv_flags, fp));
			break;
		case 'N':
			SKEY("Name", skill->name);
			MLSKEY("NounDamage", skill->noun_damage);
			break;
		case 'O':
			MLSKEY("ObjWearOff", skill->msg_obj);
			break;
		case 'S':
			if (!str_cmp(word, "Sell") || !str_cmp(word, "SellSkill")) {
				//char *str;
				sellskill_t *sskill;

				sskill = varr_enew(&skill->sell);
				free_string(sskill->name);
				sskill->flag = flag_value(sell_skill_type, fread_word(fp));
				if (sskill->flag == -1)
				{
					if (fBootDb)
						db_error("load_skills", "incorrect SellSkill, use: Sell {race|class|sex|align|ethos|all} list_names~ minlev cost_in_silver vnum_seller");
					else
						log_printf("[load_skills] incorrect SellSkill");
					exit(1);

				}
				sskill->name = fread_string(fp);
				sskill->minlev = fread_number(fp);
				sskill->cost = fread_number(fp);
				sskill->vnum_seller = fread_number(fp);
				
				fMatch = TRUE;
				break;
			}
/*
 *			KEY("Slot", skill->slot, fread_number(fp));	for OLDAREA data
 */
			if (!str_cmp(word, "Slot")) {
				/* Skip this value */
				fread_number(fp);
				fMatch = TRUE;
				break;
			}
//			KEY("SpellFun", skill->spell_fun,
//			    fread_namedp(spellfn_table, fp));
			if (!str_cmp(word, "SpellFlags"))
			{
				flag32_t spf = fread_fstring(spell_flags_table, fp);

				if (skill->spell)
					skill->spell->type = spf;
				fMatch = TRUE;
				break;
			}

			if (!str_cmp(word, "SpellFun"))
			{
				SPELL_FUN *ttt = fread_namedp(spellfn_table, fp);
				if (skill->spell)
					skill->spell->spell_fun = ttt;
				fMatch = TRUE; 
			}
			break;
		case 'T':
//			KEY("Type", word, fread_word(fp));	/* just skip */
			if (!str_cmp(word, "Type"))
			{
				const char *str;

				str = fread_word(fp);
				if (!str_cmp(str, "spell"))
				{
					if (!skill->spell)
						skill->spell = calloc(1, sizeof(*(skill->spell)));
					skill->spell->components.nsize = sizeof(struct spell_component);
					skill->spell->components.nstep = 1;
				}
				fMatch = TRUE;
			}
//			KEY("Target", skill->target,
//			    fread_fword(skill_targets, fp));
			if (!str_cmp(word, "Target"))
			{
				flag32_t ttt = fread_fword(skill_targets, fp);
				if (skill->spell)
					skill->spell->target = ttt;
				fMatch = TRUE;
			}
			break;
		case 'W':
			MLSKEY("WearOff", skill->msg_off);
			break;
		}

		if (!fMatch)
			db_error("load_skill", "%s: Unknown keyword", word);
	}
}

