/*
 * $Id: util.h,v 1.4 2003/10/07 21:37:45 xor Exp $
 */

#ifndef _UTIL_H_
#define _UTIL_H_

#include <stdio.h>
#include <linux/limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/telnet.h>
#include <arpa/inet.h>
#include "libtypedef.h"
#include "flag.h"

FILE *	dfopen	(const char *dir, const char *file, const char *mode);
int	dunlink	(const char *dir, const char *file);
int	d2rename(const char *dir1, const char *file1,
		 const char *dir2, const char *file2);
bool	dfexist	(const char *dir, const char *file);

const char *	get_filename(const char*);

int cmpint(const void *p1, const void *p2);

size_t		cstrlen		(const char* cstr);
const char*	cstrfirst	(const char *cstr);
const char *cutcolor(const char *str);

char*		strtime		(time_t);

extern unsigned char reverse_case[256];
extern unsigned char upper_case[256];
extern unsigned char lower_case[256];

unsigned char to_reverse_case (unsigned char c);
unsigned char to_upper_case (unsigned char c);
unsigned char to_lower_case (unsigned char c);

bool is_number(const char *argument);

extern int fBootDb;
extern char	filename	[PATH_MAX];
extern int	line_number;

void		db_error	(const char* fn, const char* fmt, ...);

int	xgetc	(FILE *fp);
void	xungetc	(int c, FILE *fp);

char *		fix_string	(const char *s);

const char *	fread_string	(FILE *fp);
void		fwrite_string	(FILE *fp, const char *name, const char *str);

char		fread_letter	(FILE *fp);
long int	fread_number	(FILE *fp);
flag64_t 	fread_flags	(FILE *fp);
void		fread_to_eol	(FILE *fp);
char *		fread_word	(FILE *fp);
flag64_t	fread_fword	(const flag_t *table, FILE *fp); 
flag64_t	fread_fstring	(const flag_t *table, FILE *fp);

flag64_t	flag_convert	(char letter);

bool		fread_addr(FILE *fp, struct sockaddr_in *addr);

#endif
