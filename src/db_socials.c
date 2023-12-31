/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*-
 * Copyright (c) 1999 fjoe <fjoe@iclub.nsu.ru>
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
 * $Id: db_socials.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#include <limits.h>
#include <stdio.h>
#include <ctype.h>

#include "merc.h"
#include "typedef.h"
#include "const.h"
#include "varr.h"
#include "socials.h"
#include "db.h"
#include "str.h"
#include "tables.h"
#include "util.h"

DECLARE_DBLOAD_FUN(load_social);

DBFUN dbfun_socials[] =
{
	{ "SOCIAL",	load_social	},
	{ NULL }
};

DBDATA db_socials = { dbfun_socials };

DBLOAD_FUN(load_social)
{
	social_t *soc = social_new();

	for (;;) {
		char *word = feof(fp) ? "End" : fread_word(fp);
		bool fMatch = FALSE;

		switch (UPPER(word[0])) {
		case 'E':
			if (!str_cmp(word, "end")) {
				if (IS_NULLSTR(soc->name)) {
					db_error("load_social",
						 "social name not defined");
					social_free(soc);
					socials.nused--;
				}
				return;
			}
			break;
		case 'F':
			MLSKEY("found_char", soc->found_char);
			MLSKEY("found_vict", soc->found_vict);
			MLSKEY("found_notvict", soc->found_notvict);
			break;
		case 'N':
			KEY("name", soc->name, str_dup(fread_word(fp)));
			MLSKEY("notfound_char", soc->notfound_char);
			MLSKEY("noarg_char", soc->noarg_char);
			MLSKEY("noarg_room", soc->noarg_room);
			break;
		case 'M':
			KEY("min_pos", soc->min_pos,
			    fread_fword(position_table, fp));
			break;
		case 'S':
			MLSKEY("self_char", soc->self_char);
			MLSKEY("self_room", soc->self_room);
			break;
		}

		if (!fMatch) 
			db_error("load_social", "%s: Unknown keyword", word);
	}
}
