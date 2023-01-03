/*
 * $Id: str.h,v 1.3 2003/05/18 23:40:46 xor Exp $
 */

#ifndef _STR_H_
#define _STR_H_

#include "libtypedef.h"

const char *	str_dup		(const char *str);
const char *	str_qdup	(const char *str);
void		free_string	(const char *str);
const char *	str_printf	(const char *format,...);

extern char	str_empty[1];
	
char *	strnzcpy(char *dest, size_t len, const char *src);
#define strnzncpy(dest, len, src, count) \
		strnzcpy((dest), UMIN((len), (count)+1), (src))
char *	strnzcat(char *dest, size_t len, const char *src);
char *	strnzncat(char *dest, size_t len, const char *src, size_t count);
#if !defined (WIN32)
//char *	strlwr(const char *s); // prool for cygwin
#endif

int	str_cmp		(const char *astr, const char *bstr);
int	str_ncmp	(const char *astr, const char *bstr, size_t len);
bool	str_prefix	(const char *astr, const char *bstr);
bool	str_infix	(const char *astr, const char *bstr);
bool	str_suffix	(const char *astr, const char *bstr);

int hashstr(const char *s, int maxn, int hashs);
int hashistr(const char *s, int maxn, int hashs);

int cmpstr(const void*, const void*);

uint	number_argument (const char *argument, char *arg, size_t len);
uint	mult_argument	(const char *argument, char *arg, size_t len);
const char *	one_argument	(const char *argument, char *arg_first, size_t);
const char *	first_arg	(const char *argument, char *arg_first, size_t,
				 bool fCase);

#endif
