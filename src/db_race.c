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
 * $Id: db_race.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#include <stdio.h>
#include <string.h>

#include "merc.h"
#include "db.h"

DECLARE_DBLOAD_FUN(load_race);
DECLARE_DBLOAD_FUN(load_pcrace);

DECLARE_DBINIT_FUN(init_race);

int slang_lookup(const char *name);

DBFUN dbfun_races[] =
{
	{ "RACE",	load_race	},
	{ "PCRACE",	load_pcrace	},
	{ NULL }
};

DBDATA db_races = { dbfun_races, init_race };

DBINIT_FUN(init_race)
{
	db_set_arg(dbdata, "PCRACE", NULL);
}

DBLOAD_FUN(load_race)
{
	race_t *race = race_new();
	race->file_name = get_filename(filename);
	db_set_arg(dbdata, "PCRACE", race);

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'A':
			KEY("Aff", race->aff,
			    fread_fstring(affect_flags, fp));
			KEY("Act", race->act,
			    fread_fstring(act_flags, fp));
			break;
		case 'E':
			if (!str_cmp(word, "End")) {
				if (race->slang == 0)
					race->slang = gsn_common_slang;

				if (IS_NULLSTR(race->name)) {
					db_error("load_race",
						 "race name undefined");
					race_free(race);
					races.nused--;
				}
				return;
			}
			break;
		case 'F':
			KEY("Form", race->form,
			    fread_fstring(form_flags, fp));
			KEY("Flags", race->flags,
			    fread_fstring(race_flags, fp));
			break;
		case 'I':
			KEY("Imm", race->imm,
			    fread_fstring(irv_flags, fp));
			break;
		case 'N':
			SKEY("Name", race->name);
			break;
		case 'O':
			KEY("Off", race->off,
			    fread_fstring(off_flags, fp));
			break;
		case 'P':
			KEY("Parts", race->parts,
			    fread_fstring(part_flags, fp));
			break;
		case 'R':
			KEY("Res", race->res,
			    fread_fstring(irv_flags, fp));
			break;
		case 'S':
			if (!str_cmp(word, "Slang")) {	
				const char *p = fread_word(fp);
				int slang;
				
				if ((slang = slang_lookup(p)) <= 0)
				{
					db_error("load_pcrace", "error slang");
					return;
				}
				race->slang = slang;
				fMatch = TRUE;
			}
			break;
		case 'V':
			KEY("Vuln", race->vuln,
			    fread_fstring(irv_flags, fp));
			break;
		}

		if (!fMatch)
			db_error("load_race", "%s: Unknown keyword", word);
	}
}

DBLOAD_FUN(load_pcrace)
{
	race_t *race = arg;
	pcrace_t *pcr;

	if (!race) {
		db_error("load_pcrace", "#PCRACE before #RACE");
		return;
	}

	pcr = race->pcdata = pcrace_new();

	for (;;) {
		int i;
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch(UPPER(word[0])) {
		case 'B':
			SKEY("BonusSkills", pcr->bonus_skills);
			break;

		case 'C':
			if (!str_cmp(word, "Class")) {
				rclass_t *rcl;

				rcl = varr_enew(&pcr->classes);
				rcl->name = str_dup(fread_word(fp));
				rcl->mult = fread_number(fp);
				fMatch = TRUE;
			}
			break;

		case 'E':
			if (!str_cmp(word, "End")) {
				if (pcr->who_name[0] == '\0') {
					db_error("load_pcrace",
						 "race who_name undefined");
					pcrace_free(pcr);
					race->pcdata = NULL;
				}
				
				varr_qsort(&pcr->classes, cmpstr);
				varr_qsort(&pcr->skills, cmpint);
				return;
			}
			break;

		case 'H':
			KEY("HPBonus", pcr->hp_bonus, fread_number(fp));
			break;

		case 'M':
			KEY("ManaBonus", pcr->mana_bonus, fread_number(fp));
			if (!str_cmp(word, "MaxStats")) {
				for (i = 0; i < MAX_STATS; i++)
					pcr->max_stats[i] = fread_number(fp);
				fMatch = TRUE;
			}

		case 'P':
			KEY("Points", pcr->points, fread_number(fp));
			KEY("PracBonus", pcr->prac_bonus, fread_number(fp));
			break;

		case 'R':
			KEY("RestrictAlign", pcr->allow_align,
			    fread_fstring(ralign_names, fp));
			KEY("RestrictEthos", pcr->allow_ethos,	/* Not used :) */
			    fread_fstring(ethos_table, fp));
			break;

		case 'S':
			KEY("Size", pcr->size, fread_fword(size_table, fp));
			if (!str_cmp(word, "ShortName")) {
				const char *p = fread_string(fp);
				strnzcpy(pcr->who_name, sizeof(pcr->who_name),
					 p);
				free_string(p);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Skill")) {
				rskill_t *rsk;

				rsk = varr_enew(&pcr->skills);
				rsk->sn = sn_lookup(fread_word(fp));
				rsk->level = fread_number(fp);
				fMatch = TRUE;
				break;
			}
			if (!str_cmp(word, "Stats")) {
				for (i = 0; i < MAX_STATS; i++)
					pcr->stats[i] = fread_number(fp);
				fMatch = TRUE;
			}
			break;
		}

		if (!fMatch)
			db_error("load_pcrace", "%s: Unknown keyword", word);
	}
}
