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

DECLARE_DBLOAD_FUN(load_damages);

DBFUN dbfun_damages[] =
{
	{ "MSG",	load_damages	},
	{ NULL}
};

DBDATA db_damages = { dbfun_damages };

struct damages_data_t	damages_data;	

/*
void dm_free(DAMAGES_MSG *dm)
{
	mlstr_free(dm->msg_s);
	mlstr_free(dm->msg_p);
}
*/

flag_t damage_type_table[] =
{
	{ "",		TABLE_INTVAL		},
	
	{ "-1",			DAM_NONE,	TRUE	},
	{ "DAM_NONE",		DAM_NONE,	TRUE	},
	{ "DAM_BASH",		DAM_BASH,	TRUE	},
	{ "DAM_PIERCE",		DAM_PIERCE,	TRUE	},
	{ "DAM_SLASH",		DAM_SLASH,	TRUE	},
	{ "DAM_FIRE",		DAM_FIRE,	TRUE	},
	{ "DAM_COLD",		DAM_COLD,	TRUE	},
	{ "DAM_LIGHTNING",	DAM_LIGHTNING,	TRUE	},
	{ "DAM_ACID",		DAM_ACID,	TRUE	},
	{ "DAM_POISON",		DAM_POISON,	TRUE	},
	{ "DAM_NEGATIVE",	DAM_NEGATIVE,	TRUE	},
	{ "DAM_HOLY",		DAM_HOLY,	TRUE	},
	{ "DAM_ENERGY",		DAM_ENERGY,	TRUE	},
	{ "DAM_MENTAL",		DAM_MENTAL,	TRUE	},
	{ "DAM_DISEASE",	DAM_DISEASE,	TRUE	},
	{ "DAM_DROWNING",	DAM_DROWNING,	TRUE	},
	{ "DAM_LIGHT",		DAM_LIGHT,	TRUE	},
	{ "DAM_OTHER",		DAM_OTHER,	TRUE	},
	{ "DAM_HARM",		DAM_HARM,	TRUE	},
	{ "DAM_CHARM",		DAM_CHARM,	TRUE	},
	{ "DAM_SOUND",		DAM_SOUND,	TRUE	},
	{ "DAM_THIRST",		DAM_THIRST,	TRUE	},
	{ "DAM_HUNGER",		DAM_HUNGER,	TRUE	},
	{ "DAM_LIGHT_V",	DAM_LIGHT_V,	TRUE	},
	{ "DAM_TRAP_ROOM",	DAM_TRAP_ROOM,	TRUE	},
	
	{ NULL }
};

/*inline*/ bool test_to_m(mlstring * to_m) // inline delete by prool
{
	if (to_m &&
	(mlstr_mval(to_m)[0] == ' ' || mlstr_mval(to_m)[0] == '\0')) {
		mlstr_free(to_m);
		return TRUE;
	}
	return FALSE;
}

void fill_to(struct damages_msg_to *to, FILE *fp)
{
	if (test_to_m(to->msg_notvict = mlstr_fread(fp)))
		to->msg_notvict = NULL;
	if (test_to_m(to->msg_char = mlstr_fread(fp)))
		to->msg_char = NULL;
	if (test_to_m(to->msg_vict = mlstr_fread(fp)))
		to->msg_vict = NULL;
}

DBLOAD_FUN(load_damages)
{
	DAMAGES_MSG *dm = NULL;
	struct attack_type *atd = NULL;
	
	damages_data.list_damages_msg.nsize =
		sizeof(damages_data.list_damages_msg);
	damages_data.list_damages_msg.nstep = 2;
	damages_data.attack_table.nsize = sizeof(struct attack_type);
	damages_data.attack_table.nstep = 2;
	
	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;
		
		switch (UPPER(word[0])) {
			case 'A':
				if (!str_cmp(word, "attack")) {
					atd = varr_enew(&damages_data.attack_table);
					atd->name	= mlstr_fread(fp);
					atd->noun	= mlstr_fread(fp);
					atd->damage	= flag_lookup(damage_type_table, fread_word(fp))->bit;
					fMatch = TRUE;
				}
				break;
			case 'E':
				if (!str_cmp(word, "end")) {
					if (!dm || !dm->msg_p || !atd || !atd->noun) {
						db_error("load_damages",
							"abnormal termination define 'Msg' or 'Attack type'\n");
					}
					return;
				}
				break;
			case 'M':
				if (!str_cmp(word, "msg")) {
					dm = varr_enew(&damages_data.list_damages_msg);
					dm->min_dam	= fread_number(fp);
					dm->max_dam	= fread_number(fp);
					dm->msg_s	= mlstr_fread(fp);
					dm->msg_p	= mlstr_fread(fp);
					fMatch = TRUE;
				}
				break;
			case 'T':
				if (!str_cmp(word, "to_n_dt_cv_hunger")) {
					fill_to(&damages_data.to_n_dt_cv_hunger, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_n_dt_cv_thirst")) {
					fill_to(&damages_data.to_n_dt_cv_thirst, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_n_dt_cv_light")) {
					fill_to(&damages_data.to_n_dt_cv_light, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_n_dt_cv_trap")) {
					fill_to(&damages_data.to_n_dt_cv_trap, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_n_dt_cv")) {
					fill_to(&damages_data.to_n_dt_cv, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_n_dt")) {
					fill_to(&damages_data.to_n_dt, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_imm_cv")) {
					fill_to(&damages_data.to_imm_cv, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_imm")) {
					fill_to(&damages_data.to_imm, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_sp_cv")) {
					fill_to(&damages_data.to_sp_cv, fp);
					fMatch = TRUE;
				}
				if (!str_cmp(word, "to_sp")) {
					fill_to(&damages_data.to_sp, fp);
					fMatch = TRUE;
				}
				break;
			if (!fMatch) {
				db_error("load_damages", "%s: Unknown keyword", word);
			}
		}
	}
}
