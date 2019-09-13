/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * (c)Shadow Realms  2000 year (Xor & Taelas) ver. 0.000001
 *  make_drunk() make by Bellionore.... special thx to him :)
 */

#ifndef _CHAR_MUD_H_
#define _CHAR_MUD_H_

const char *make_drunk (CHAR_DATA *ch, const char *string);
void wrath_of_gods(CHAR_DATA *ch, int align);
void remort_loop(CHAR_DATA *ch, const char *argument);
void remort_done(CHAR_DATA *ch, bool accept);

#define RP_NONE		0
#define	RP_HITROLL	1
#define RP_DAMROLL	2
#define RP_HIT		3
#define	RP_MANA		4
#define	RP_MOVE		5
#define	RP_QP		6
#define	RP_TRAIN	7
#define	RP_ETHOS	8
#define	RP_ALIGN	9
#define	RP_RACE		10
#define RP_SEX		11
#define	RP_PRACTICE	12
#define RP_SKILL	13
#define	RP_LEVEL_SKILL	14
#define	RP_PERC_SKILL	15
#define	RP_RP		16
#define	RP_CLASS	17

#define NUMBER_RP	18

#define START_RP	85
#define DEFAULT_PRICE_SKILL	40

#endif
