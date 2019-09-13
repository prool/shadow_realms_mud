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
 * $Id: quest.h,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#ifndef _QUEST_H_
#define _QUEST_H_

/*
 * Quest obj vnums must take a continuous interval for proper quest generating.
 */
#define QUEST_OBJ_FIRST 84
#define QUEST_OBJ_LAST  87
#define QUANTITY_ATTEMPTS 3	//... in answer

/*
struct qtrouble_t {
	int vnum;
	int count;
	qtrouble_t *next;
};
*/

//#define IS_ON_QUEST(ch)	(ch->pcdata->questtime > 0)

#define GET_QUEST_TYPE(ch) (ch->pcdata->quest ? ch->pcdata->quest->type : QUESTT_NONE)
#define GET_QUEST_TIME(ch) (ch->pcdata->quest ? ch->pcdata->quest->time : 0)
#define GET_QUEST_MOB1(ch) (ch->pcdata->quest ? (CHAR_DATA *) ch->pcdata->quest->target1 : NULL)
#define GET_QUEST_MOB2(ch) (ch->pcdata->quest ? (CHAR_DATA *) ch->pcdata->quest->target2 : NULL)
#define GET_QUEST_OBJ1(ch) (ch->pcdata->quest ? (OBJ_DATA *) ch->pcdata->quest->target1 : NULL)

void quest_handle_death(CHAR_DATA *ch, CHAR_DATA *victim);
void quest_cancel(CHAR_DATA *ch);
void quest_update(void);

/*
int qtrouble_get(CHAR_DATA *ch, int vnum);
void qtrouble_set(CHAR_DATA *ch, int vnumi, int count);
*/

#endif
