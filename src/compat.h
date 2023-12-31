/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

#ifndef _COMPAT_H_
#define _COMPAT_H_

/*
 * $Id: compat.h,v 1.31 2003/04/22 07:35:22 xor Exp $
 */
/*
 * For AIX, Win etc.
 */

char* strsep(char**, const char*);

#ifdef SUNOS
	int snprintf(char*, size_t, const char*, ...);
#	define vsnprintf(s, size, fmt, va) vsprintf(s, fmt, va)
#endif

/* Win32 stuff 	*/

#if defined (WIN32)
#include <windows.h>

void *bzero		(void *block, size_t size);
void *bcopy		(void *from, const void *to, size_t size);

typedef struct dirent
{
    WIN32_FIND_DATA FindData;
	HANDLE Data;
    char	d_name[ MAX_PATH + 1 ]; /* file's name */
	BOOL	d_firstread;			/* flag for 1st time */
} DIR;

DIR* opendir	(const char *dirname);
struct dirent *readdir	(DIR *dirstream);
int closedir	(DIR *dirstream);

#endif

#endif
