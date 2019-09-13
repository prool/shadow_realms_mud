/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include "compat.h"

/*
 * $Id: snprintf.c,v 1.31 2003/04/22 07:35:23 xor Exp $
 */
/*                                                                             
 * For AIX, Win etc.                                                           
 */

int snprintf(char* buf, size_t size, const char* fmt, ...)
{
	int res;
	va_list ap;

	va_start(ap, fmt);
	res = vsnprintf(buf, size, fmt, ap);
	va_end(ap);

	return res;
}
