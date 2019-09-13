/*
 * $Id: str.c,v 1.3 2003/05/18 23:40:46 xor Exp $
 */

#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>

#include "libtypedef.h"
#include "libconst.h"
#include "str.h"
#include "util.h"
//void crush_mud(void); /* be in db.c */

#if defined (WIN32)
#	define vsnprintf	_vsnprintf
int strncasecmp (const char *s1, const char *s2, size_t n);
int strcasecmp (const char *s1, const char *s2);
#endif

char str_empty[1];

int str_count;
int str_real_count;

#define strhash(s)	(hashstr(s, 64, MAX_STRING_HASH))

typedef struct str str;
struct str {
	int		ref;
	str *		next;
	char 		p[0];
};

str *hash_str[MAX_STRING_HASH];

static str *str_lookup(const char* p, int *hash);
static str *str_alloc(const char*, int hash);

const char *str_dup(const char *p)
{
	int hash;
	str *s;

	if (IS_NULLSTR(p))
		return str_empty;

	str_count++;
	if ((s = str_lookup(p, &hash)) == NULL)
		s = str_alloc(p, hash);
	s->ref++;
	return s->p;
}

const char *str_qdup(const char *p)
{
	str *s;

	if (IS_NULLSTR(p))
		return str_empty;

	str_count++;
	s = (str *)(p - sizeof(str));
	s->ref++;
	return p;
}

void free_string(const char *p)
{
	str *s, *q;
	int hash;

	if (p == NULL || p == str_empty)
		return;

	str_count--;
	hash = strhash(p);
	for (q = NULL, s = hash_str[hash]; s; s = s->next) {
		if (!strcmp(s->p, p))
			break;
		q = s;
	}

	if (!s || --s->ref)
		return;

	if (q)
		q->next = s->next;
	else
		hash_str[hash] = hash_str[hash]->next;
	str_real_count--;
	free(s);
}

/*
 * str_printf -- snprintf to string
 */
const char *str_printf(const char* format,...)
{
	va_list ap;
	char buf[MAX_STRING_LENGTH];

	if (format == NULL)
		return NULL;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	return str_dup(buf);
}

/*
 * strnzcpy - copy from dest to src and always append terminating '\0'.
 *            len MUST BE > 0
 */
char *strnzcpy(char *dest, size_t len, const char *src)
{
	strncpy(dest, src, len);
	dest[len-1] = '\0';
	return dest;
}

char *strnzcat(char *dest, size_t len, const char *src)
{
	size_t old_len;

	old_len = strlen(dest);
	if (old_len >= len - 1)
		return dest;

	strncat(dest, src, len - old_len - 1);
	return dest;
}

char *strnzncat(char *dest, size_t len, const char *src, size_t count)
{
	size_t old_len;

	old_len = strlen(dest);
	if (old_len >= len - 1)
		return dest;

	strncat(dest, src, UMIN(len - old_len - 1, count));
	return dest;
}

char *strlwr(const char *s)
{
	static char buf[MAX_STRING_LENGTH];
	char *p;

	if (s == NULL)
		return str_empty;

	for (p = buf; p-buf < sizeof(buf)-1 && *s; s++, p++)
		*p = LOWER(*s);
	*p = '\0';
	return buf;
}

/*
 * Compare strings, case insensitive.
 */
int str_cmp(const char *astr, const char *bstr)
{
	if (astr == NULL)
		return bstr == NULL ? 0 : -1;
	if (bstr == NULL)
		return 1;
	return strcasecmp(astr, bstr);
}

int str_ncmp(const char *astr, const char *bstr, size_t len)
{
	if (astr == NULL)
		return bstr == NULL ? 0 : -1;
	if (bstr == NULL)
		return 1;
	return strncasecmp(astr, bstr, len);
}

/*
 * Compare strings, case insensitive, for prefix matching.
 * Return TRUE if astr not a prefix of bstr
 *   (compatibility with historical functions).
 */
bool str_prefix(const char *astr, const char *bstr)
{
	if (astr == NULL)
		return TRUE;

	if (bstr == NULL)
		return TRUE;

	for (; *astr; astr++, bstr++) {
		if (LOWER(*astr) != LOWER(*bstr))
			return TRUE;
	}

	return FALSE;
}

/*
 * Compare strings, case insensitive, for match anywhere.
 * Returns TRUE is astr not part of bstr.
 *   (compatibility with historical functions).
 */
bool str_infix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;
	int ichar;
	char c0;

	if ((c0 = LOWER(astr[0])) == '\0')
		return FALSE;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);

	for (ichar = 0; ichar <= sstr2 - sstr1; ichar++) {
		if (c0 == LOWER(bstr[ichar]) && !str_prefix(astr, bstr + ichar))
		    return FALSE;
	}

	return TRUE;
}

/*
 * Compare strings, case insensitive, for suffix matching.
 * Return TRUE if astr not a suffix of bstr
 *   (compatibility with historical functions).
 */
bool str_suffix(const char *astr, const char *bstr)
{
	int sstr1;
	int sstr2;

	sstr1 = strlen(astr);
	sstr2 = strlen(bstr);
	if (sstr1 <= sstr2 && !str_cmp(astr, bstr + sstr2 - sstr1))
		return FALSE;
	else
		return TRUE;
}

/*
** A simple and fast generic string hasher based on Peter K. Pearson's
** article in CACM 33-6, pp. 677.
*/

static int TT[] = {
	1, 87, 49, 12, 176, 178, 102, 166, 121, 193, 6, 84, 249, 230, 44, 163,
	14, 197, 213, 181, 161, 85, 218, 80, 64, 239, 24, 226, 236, 142, 38, 200,
	110, 177, 104, 103, 141, 253, 255, 50, 77, 101, 81, 18, 45, 96, 31, 222,
	25, 107, 190, 70, 86, 237, 240, 34, 72, 242, 20, 214, 244, 227, 149, 235,
	97, 234, 57, 22, 60, 250, 82, 175, 208, 5, 127, 199, 111, 62, 135, 248,
	174, 169, 211, 58, 66, 154, 106, 195, 245, 171, 17, 187, 182, 179, 0, 243,
	132, 56, 148, 75, 128, 133, 158, 100, 130, 126, 91, 13, 153, 246, 216, 219,
	119, 68, 223, 78, 83, 88, 201, 99, 122, 11, 92, 32, 136, 114, 52, 10,
	138, 30, 48, 183, 156, 35, 61, 26, 143, 74, 251, 94, 129, 162, 63, 152,
	170, 7, 115, 167, 241, 206, 3, 150, 55, 59, 151, 220, 90, 53, 23, 131, 
	125, 173, 15, 238, 79, 95, 89, 16, 105, 137, 225, 224, 217, 160, 37, 123,
	118, 73, 2, 157, 46, 116, 9, 145, 134, 228, 207, 212, 202, 215, 69, 229,
	27, 188, 67, 124, 168, 252, 42, 4, 29, 108, 21, 247, 19, 205, 39, 203,
	233, 40, 186, 147, 198, 192, 155, 33, 164, 191, 98, 204, 165, 180, 117, 76,
	140, 36, 210, 172, 41, 54, 159, 8, 185, 232, 113, 196, 231, 47, 146, 120,
	51, 65, 28, 144, 254, 221, 93, 189, 194, 139, 112, 43, 71, 109, 184, 209,
};

int
hashstr(s, maxn, hashs)
const char *s;			/* string to hash */
int maxn;			/* maximum number of chars to consider */
int hashs;			/* hash table size. */
{
    register int h;
    register u_char *p;
    register int i;

    for(h = 0, i = 0, p = (u_char *)s; *p && i < maxn; i++, p++)
	h = TT[h ^ *p];
    if (hashs > 256 && *s) {
	int oh = h;
	for(i = 1, p = (u_char *)s, h = (*p++ + 1)&0xff; *p && i < maxn; i++, p++)
	    h = TT[h ^ *p];
	h += (oh << 8);
    }
    return h % hashs;		/* With 16 bit ints h has to be made positive first! */
}

/*
 * case insensitive version of hashstr, hashs must not be greater than 256
 */
int
hashistr(s, maxn, hashs)
const char *s;			/* string to hash */
int maxn;			/* maximum number of chars to consider */
int hashs;			/* hash table size. */
{
    register int h;
    register u_char *p;
    register int i;

    for(h = 0, i = 0, p = (u_char *)s; *p && i < maxn; i++, p++)
	h = TT[h ^ LOWER(*p)];
    return h % hashs;
}

int cmpstr(const void *p1, const void *p2)
{
	return -str_cmp(*(char**) p1, *(char**) p2);
}

/*----------------------------------------------------------------------------
 * static functions
 */

static str *str_alloc(const char *p, int hash)
{
	int len = strlen(p) + 1;
	str *s;
	str_real_count++;
	s = malloc(sizeof(*s) + len);
	if (s == NULL)
		return NULL;
		//crush_mud();
	strncpy(s->p, p, len);
	s->ref = 0;
	s->next = hash_str[hash];
	return hash_str[hash] = s;
}

static str *str_lookup(const char *p, int *hash)
{
	str *s;
	for (s = hash_str[*hash = strhash(p)]; s; s = s->next)
		if (!strcmp(s->p, p))
			return s;
	return NULL;
}

static uint x_argument(const char *argument, char c, char *arg, size_t len)
{
	char *p;
	char *q;
	int number;
    
	if (IS_NULLSTR(argument)) {
		arg[0] = '\0';
		return 0;
	}

	p = strchr(argument, c);
	if (p == NULL) {
		strnzcpy(arg, len, argument);
		return 1;
	}

	number = strtoul(argument, &q, 0);
	if (q == p)
		argument = p+1;
	else 
		number = 1;
	strnzcpy(arg, len, argument);
	return number;
}

/*
 * Given a string like 14.foo, return 14 and 'foo'
 */
uint number_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '.', arg, len);
}

/* 
 * Given a string like 14*foo, return 14 and 'foo'
 */
uint mult_argument(const char *argument, char *arg, size_t len)
{
	return x_argument(argument, '*', arg, len);
}

/*
 * Pick off one argument from a string and return the rest.
 * Understands quotes.
 */
const char *one_argument(const char *argument, char *arg_first, size_t len)
{
	return first_arg(argument, arg_first, len, TRUE);
}

/*****************************************************************************
 Name:		first_arg
 Purpose:	Pick off one argument from a string and return the rest.
 		Understands quotes, if fCase then arg_first will be lowercased
 Called by:	string_add(string.c)
 ****************************************************************************/
const char *first_arg(const char *argument, char *arg_first, size_t len,
		      bool fCase)
{
	char *q;
	char cEnd = '\0';

	if (IS_NULLSTR(argument)) {
		arg_first[0] = '\0';
		return argument;
	}

/* skip leading spaces */
	while (isspace(*argument))
		argument++;

/* check quotes */
	if (*argument == '\'' || *argument == '"')
        	cEnd = *argument++;

	for (q = arg_first; *argument && q - arg_first + 1 < len; argument++) {
		if ((!cEnd && isspace(*argument)) || *argument == cEnd) {
			argument++;
			break;
		}
		*q++ = fCase ? LOWER(*argument) : *argument;
	}
	*q = '\0';

	while (isspace(*argument))
		argument++;

	return argument;
}

