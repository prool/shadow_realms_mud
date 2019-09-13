/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*-
 * Changed by Xor <xorader@mail.ru> in 2002 for SR
 * 
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
 */

#include <stdio.h>

#include "typedef.h"
#include "const.h"
#include "namedp.h"
#include "str.h"
#include "log.h"
#include "buffer.h"

namedp_t *namedp_lookup(namedp_t *table, const char *name)
{
	for (; table->name; table++)
		if (!str_cmp(table->name, name))
			return table;
	return NULL;
}

const char *namedp_name(const namedp_t *table, void *p)
{
	for (; table->name; table++)
		if (table->p == p)
			return table->name;
	return NULL;
}

void namedp_check(const namedp_t *np)
{
	for (; np->name; np++)
		if (!np->touched) 
			log_printf("%s: not touched", np->name);
}

void page_to_char(const char *txt, CHAR_DATA *ch);
void show_namedp_chbuf(CHAR_DATA *ch, const namedp_t *table)
{
	BUFFER *output;
	int  col = 0;

	output = buf_new(-1);
	for (; table->name; table++) {
		buf_printf(output, "%-39.38s", table->name);
		if (++col % 2 == 0)
			buf_add(output, "\n");
	}

	if (col % 2 != 0)
		buf_add(output, "\n");

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

