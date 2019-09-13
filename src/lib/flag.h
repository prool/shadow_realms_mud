/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: flag.h,v 1.1 2003/05/18 23:42:08 xor Exp $
 */

/***************************************************************************
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 *                                                                         *
 *  This code was written by Jason Dinkel and inspired by Russ Taylor,     *
 *  and has been used here for OLC - OLC would not be what it is without   *
 *  all the previous coders who released their source code.                *
 *                                                                         *
 ***************************************************************************/

#ifndef _ml_flag_t_H_
#define _ml_flag_t_H_

#include "libtypedef.h"
#include "libconst.h"

enum {
	TABLE_BITVAL,	/* table contains bit values		*/
	TABLE_INTVAL	/* table contains integer values	*/
};

typedef struct flag_t	flag_t;
struct flag_t
{
	const char *	name;
	flag64_t	bit;
	bool		settable;
};

const flag_t *	flag_lookup	(const flag_t *flag64_table, const char* name);
flag64_t	flag_value	(const flag_t *flag64_table, const char *argument);
const char *	flag_string	(const flag_t *flag64_table, flag64_t bits);

#endif	// _ml_flag_t_H_

