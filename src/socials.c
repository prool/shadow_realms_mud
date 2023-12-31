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
 * $Id: socials.c,v 1.31 2003/04/22 07:35:23 xor Exp $
 */

#include <stdio.h>

#include "typedef.h"
#include "varr.h"
#include "socials.h"
#include "str.h"
#include "mlstring.h"

varr socials = { sizeof(social_t), 8 };

social_t *social_new()
{
	return varr_enew(&socials);
}

void social_free(social_t *soc)
{
	free_string(soc->name);

	mlstr_free(soc->found_char);
	mlstr_free(soc->found_vict);
	mlstr_free(soc->found_notvict);

	mlstr_free(soc->noarg_char);
	mlstr_free(soc->noarg_room);

	mlstr_free(soc->self_char);
	mlstr_free(soc->self_room);

	mlstr_free(soc->notfound_char);
}

social_t *social_lookup(const char *name,
			int (*cmpfun)(const char *s1, const char *s2))
{
	int i;

	for (i = 0; i < socials.nused; i++) {
		social_t *soc = VARR_GET(&socials, i);
		if (!cmpfun(name, soc->name))
			return soc;
	}

	return NULL;
}
