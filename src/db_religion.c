/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * Shadow Realms  2001 year (Xor) (c)
 */


#include <stdio.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_religion);

DBFUN dbfun_religion[] =
{
	{ "GOD",	load_religion	},
	{ NULL}
};

DBDATA db_religion = { dbfun_religion };

DBLOAD_FUN(load_religion)
{
	RELIGION_DATA *rel;
	
	rel = varr_enew(&religions);
	
	rel->add_skills.nsize	= sizeof(religion_add_skill);
	rel->add_skills.nstep	= 1;
	
	rel->fight_spells.nsize	= sizeof(religion_fight_spell);
	rel->fight_spells.nstep	= 1;
	
	rel->change_faith.nsize = sizeof(religion_faith_change);
	rel->change_faith.nstep = 2;
	
	rel->change_sacrifice	= 100;
	rel->recall_change_exp	= 100;
	rel->change_buy		= 100;
	rel->change_sell	= 100;
	rel->change_exp_death	= 100;
	
	rel->name	= fread_string(fp);
	rel->desc	= mlstr_fread(fp);
	rel->altar.room	= ((hometown_t *) varr_get(&hometowns, 0))->altar[0].room;
	rel->altar.pit	= ((hometown_t *) varr_get(&hometowns, 0))->altar[0].pit;
	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;
		switch (UPPER(word[0]))
		{
			case '*':
			case ';':
				fread_to_eol(fp);
				fMatch = TRUE;
				break;
			case 'A':
				KEY("AllowAlign", rel->allow_align,
					fread_fstring(ralign_names, fp));
				if (!str_cmp(word, "Altar")) {
					rel->altar.room	=
						get_room_index(fread_number(fp));
					rel->altar.pit	=
						get_obj_index(fread_number(fp));
					if (!rel->altar.room || !rel->altar.pit)
					{
						db_error("load_religion", "error altar");
						return;
					}
					fMatch = TRUE;
					break;
				}
				if (!str_cmp(word, "Addskill")) {
					religion_add_skill *rskill;
					int sn;
					
					if ((sn = sn_lookup(fread_word(fp))) < 0) {
						db_error("load_religion", "Unknown skill");
						return;
					}
					rskill = varr_enew(&rel->add_skills);
					rskill->sn		= sn;
					rskill->level		= fread_number(fp);
					rskill->add_percent	= fread_number(fp);
					rskill->max_percent	= fread_number(fp);
					rskill->hard		= fread_number(fp);
					fMatch = TRUE;
					break;
				}
				break;
			case 'C':
				KEY("Change_buy", rel->change_buy, fread_number(fp));
				KEY("Change_sell", rel->change_sell, fread_number(fp));
				KEY("Change_sacrifice", rel->change_sacrifice, fread_number(fp));
				KEY("Change_exp_death", rel->change_exp_death, fread_number(fp));
				KEY("Cost_gold", rel->cost_gold, fread_number(fp));
				KEY("Cost_qp", rel->cost_qp, fread_number(fp));
				break;
			case 'E':
				if (!str_cmp(word, "End")) {
					return;
				}
				break;
			case 'F':
				if (!str_cmp(word, "Faith")) {
					religion_faith_change *rf;
					char *str;
					
					rf = varr_enew(&rel->change_faith);
					
					str = fread_word(fp);
					
					if (!str_cmp(str, "+")
					|| !str_prefix(str, "incrase")
					|| !str_prefix(str, "like"))
						rf->is_like = TRUE;
					else
						rf->is_like = FALSE;

					rf->value = fread_number(fp);
					rf->that = fread_fstring(religion_faith_flags, fp);
					
					SET_BIT(rel->change_flags, rf->that);
					fMatch = TRUE;
					break;
				}
				/*
				 *	calculate automaticaly
				 * KEY("Faith_flags", rel->change_flags, fread_fstring(religion_faith_flags, fp));
				 */
				KEY("Flags", rel->flags, fread_fstring(religion_flags, fp));
				if (!str_cmp(word, "Fightcast")) {
					religion_fight_spell *fspell;
					int sn;
					
					if ((sn = sn_lookup(fread_word(fp))) < 0) {
						db_error("load_religion", "Unknown spell");
						return;
					}
					if (!skill_lookup(sn)->spell) {
						db_error("load_religion", "%s not spell", skill_name(sn));
						return;
					}
					fspell = varr_enew(&rel->fight_spells);
					
					fspell->sn		= sn;
					fspell->percent		= fread_number(fp);
					fspell->is_for_honorable= !str_prefix(fread_word(fp), "honor");
					fspell->to_char		= !str_prefix(fread_word(fp), "char");
					fspell->faith		= fread_number(fp);
					fMatch = TRUE;
					break;
				}
				break;
			case 'G':
				KEY("Ghost_timer_default", rel->ghost_timer_default, fread_number(fp));
				KEY("Ghost_timer_plevel", rel->ghost_timer_plevel, fread_number(fp));
				break;
			case 'R':
				KEY("Recall_exp", rel->recall_change_exp, fread_number(fp));
				break;
			case 'T':
				KEY("Tattoo", rel->vnum_tattoo, fread_number(fp));
				KEY("Templeman", rel->vnum_templeman, fread_number(fp));
				break;
			case 'V':
				if (!str_cmp(word, "Vgod")) {
					int vnum = fread_number(fp);
					/***********
					CHAR_DATA *victim;
					
					for (victim = char_list; victim; victim = victim->next)
						if (IS_NPC(victim)
						&& victim->pIndexData->vnum == vnum
						&& victim->in_room)
							break;
					if (!victim) {
						log_printf("[Attention] Can't find god-mob for religion: %s", rel->name);
						fMatch = TRUE;
						break;
					}
					rel->god = victim;
					rel->godroom = victim->in_room->vnum;
					SET_BIT(victim->pIndexData->act, ACT_GOD);
					*************/
					
					rel->vnum_god = vnum;
					fMatch = TRUE;
					break;
				}
				break;
		}
		if (!fMatch) {
			db_error("load_religion", "%s: Unknown keyword", word);
			return;
		}
	}
}
