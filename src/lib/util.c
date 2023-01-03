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
 * $Id: util.c,v 1.4 2003/10/07 21:37:45 xor Exp $
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <netdb.h>
#if !defined (WIN32)
#include <unistd.h>
#endif

#include "libconst.h"
#include "log.h"
#include "libtypedef.h"
#include "str.h"
#include "util.h"

int	line_number;
bool	fBootDb;
char	filename[PATH_MAX];

#ifdef SUNOS
#	include "compat/compat.h"
#endif

#if defined(WIN32)
#define unlink	_unlink
#else
//#include <linux/limits.h> // prool for cygwin
#endif

/*
void doprintf(DO_FUN *fn, CHAR_DATA* ch, const char* fmt, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	fn(ch, buf);
	va_end(ap);
}
*/

FILE *dfopen(const char *dir, const char *file, const char *mode)
{
	char name[PATH_MAX];
	FILE *f;
	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	if ((f = fopen(name, mode)) == NULL)
		log_printf("%s: %s", name, strerror(errno));
	return f;
}

int dunlink(const char *dir, const char *file)
{
	char name[PATH_MAX];
	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	return unlink(name);
}

int d2rename(const char *dir1, const char *file1,
	     const char *dir2, const char *file2)
{
	int res;
	char name1[PATH_MAX];
	char name2[PATH_MAX];
	snprintf(name1, sizeof(name1), "%s%c%s", dir1, PATH_SEPARATOR, file1);
	snprintf(name2, sizeof(name2), "%s%c%s", dir2, PATH_SEPARATOR, file2);
#if defined (WIN32)
	res = unlink(name2);
	if (res == -1)
		log_printf("d2rename: can't delete file %s", name2);
#endif
	res = rename(name1, name2);
	if (res < 0)
		log_printf("d2rename: error renaming %s -> %s", name1, name2);
	return res;
}

bool dfexist(const char *dir, const char *file)
{
	struct stat sb;
	char name[PATH_MAX];
	snprintf(name, sizeof(name), "%s%c%s", dir, PATH_SEPARATOR, file);
	return (stat(name, &sb) >= 0);
}

const char *get_filename(const char *name)
{
	const char *p = (p = strrchr(name, PATH_SEPARATOR)) ? ++p : name;
	return str_dup(p);
}

int cmpint(const void *p1, const void *p2)
{
	return *(int*) p1 - *(int*) p2;
}

size_t cstrlen(const char *cstr)
{
	size_t res;

	if (cstr == NULL)
		return 0;

	res = strlen(cstr);
	while ((cstr = strchr(cstr, '{')) != NULL) {
		if (*(cstr+1) == '{')
			res--;
		else
			res -= 2;
		cstr += 2;
	}

	return res;
}

const char *cstrfirst(const char *cstr)
{
	if (cstr == NULL)
		return NULL;

	for (; *cstr == '{'; cstr++)
		if (*(cstr+1))
			cstr++;
	return cstr;
}

const char *cutcolor(const char *str)
{
	static char nstr[MAX_STRING_LENGTH];
	char *k;
	int i;
	
	if (!str)
		return NULL;
	
	for(k = nstr, i = 0; *str && i < MAX_STRING_LENGTH - 1; i++, str++)
	{
		if (*str == '{' && *(str+1))
			str++;
		else
			*k++= *str;
	}
	*k = '\0';

	return nstr;
}

char *strtime(time_t time)
{
	char *p = ctime(&time);
	p[24] = '\0';
	return p;
}

#define koi8_koi8 normal_case

unsigned char reverse_case[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122,  91,  92,  93,  94,  95,
	 96,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 179, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 163, 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191,
	224, 225, 226, 227, 228, 229, 230, 231,
	232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247,
	248, 249, 250, 251, 252, 253, 254, 255,
	192, 193, 194, 195, 196, 197, 198, 199,
	200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223 
};

unsigned char upper_case[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90,  91,  92,  93,  94,  95,
	 96,  65,  66,  67,  68,  69,  70,  71,
	 72,  73,  74,  75,  76,  77,  78,  79,
	 80,  81,  82,  83,  84,  85,  86,  87,
	 88,  89,  90, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 179, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 179, 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191,
	224, 225, 226, 227, 228, 229, 230, 231,
	232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247,
	248, 249, 250, 251, 252, 253, 254, 225,
	224, 225, 226, 227, 228, 229, 230, 231,
	232, 233, 234, 235, 236, 237, 238, 239,
	240, 241, 242, 243, 244, 245, 246, 247,
	248, 249, 250, 251, 252, 253, 254, 255
};

unsigned char lower_case[256] = {
	  0,   1,   2,   3,   4,   5,   6,   7,
	  8,   9,  10,  11,  12,  13,  14,  15,
	 16,  17,  18,  19,  20,  21,  22,  23,
	 24,  25,  26,  27,  28,  29,  30,  31,
	 32,  33,  34,  35,  36,  37,  38,  39,
	 40,  41,  42,  43,  44,  45,  46,  47,
	 48,  49,  50,  51,  52,  53,  54,  55,
	 56,  57,  58,  59,  60,  61,  62,  63,
	 64,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122,  91,  92,  93,  94,  95,
	 96,  97,  98,  99, 100, 101, 102, 103,
	104, 105, 106, 107, 108, 109, 110, 111,
	112, 113, 114, 115, 116, 117, 118, 119,
	120, 121, 122, 123, 124, 125, 126, 127,
	128, 129, 130, 131, 132, 133, 134, 135,
	136, 137, 138, 139, 140, 141, 142, 143,
	144, 145, 146, 147, 148, 149, 150, 151,
	152, 153, 154, 155, 156, 157, 158, 159,
	160, 161, 162, 163, 164, 165, 166, 167,
	168, 169, 170, 171, 172, 173, 174, 175,
	176, 177, 178, 163, 180, 181, 182, 183,
	184, 185, 186, 187, 188, 189, 190, 191,
	192, 193, 194, 195, 196, 197, 198, 199,
	200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223,
	192, 193, 194, 195, 196, 197, 198, 199,
	200, 201, 202, 203, 204, 205, 206, 207,
	208, 209, 210, 211, 212, 213, 214, 215,
	216, 217, 218, 219, 220, 221, 222, 223,
};

unsigned char to_reverse_case (unsigned char c) {
	return *(reverse_case+c);
}

unsigned char to_upper_case (unsigned char c) {
	return *(upper_case+c);
}

unsigned char to_lower_case (unsigned char c) {
	return *(lower_case+c);
}

/*
 *	Return true if an argument is completely numeric.
 */
bool is_number(const char *argument)
{
	if (IS_NULLSTR(argument))
		return FALSE;

	if (*argument == '+' || *argument == '-')
		argument++;

	for (; *argument != '\0'; argument++) {
		if (!isdigit(*argument))
			return FALSE;
	}

	return TRUE;
}

// from db.c

int xgetc(FILE *fp)
{
	int c = getc(fp);
	if (c == '\n')
		line_number++;
	return c;
}

void xungetc(int c, FILE *fp)
{
	if (c == '\n')
		line_number--;
	ungetc(c, fp);
}

/*
 * smash '\r', dup '~'
 */
char *fix_string(const char *s)
{
	static char buf[MAX_STRING_LENGTH * 2];
	char *p = buf;

	if (IS_NULLSTR(s))
		return str_empty;

	for (p = buf; *s && p-buf < sizeof(buf)-2; s++)
		switch (*s) {
		case '\r':
			break;

		case '~':
			*p++ = *s;
			/* FALLTHRU */

		default:
			*p++ = *s;
			break;
		}

	*p = '\0';
	return buf;
}

const char *fread_string(FILE *fp)
{
	char buf[MAX_STRING_LENGTH];
	char *plast;
	int c;

	plast = buf;

	/*
	 * Skip blanks.
	 * Read first char.
	 */
	do
		c = xgetc(fp);
	while (isspace(c));

	for (;;) {
		/*
		 * Back off the char type lookup,
		 *   it was too dirty for portability.
		 *   -- Furey
		 */

		if (plast - buf >= sizeof(buf) - 1) {
			bug("fread_string: line too long (truncated)", 0);
			buf[sizeof(buf)-1] = '\0';
			return str_dup(buf);
		}

		switch (c) {
		default:
			*plast++ = c;
			break;
 
		case EOF:
			db_error("fread_string", "EOF");
			return str_empty;
 
		case '\r':
			break;
 
		case '~':
			if ((c = xgetc(fp)) == '~') {
				*plast++ = c;
				break;
			}
			xungetc(c, fp);
			*plast = '\0';
			return str_dup(buf);
		}
		c = xgetc(fp);
	}
}

void fwrite_string(FILE *fp, const char *name, const char *str)
{
	if (IS_NULLSTR(name))
		fprintf(fp, "%s~\n", fix_string(str));
	else if (!IS_NULLSTR(str))
		fprintf(fp, "%s %s~\n", name, fix_string(str));
}

/*
 * Read a letter from a file.
 */
char fread_letter(FILE *fp)
{
	char c;

	do
		c = xgetc(fp);
	while (isspace(c));
	return c;
}

/*
 * Read a number from a file.
 */
long int fread_number(FILE *fp)
{
	long int number;
	bool sign;
	char c;

	do
		c = xgetc(fp);
	while (isspace(c));

	number = 0;

	sign   = FALSE;
	if (c == '+')
		c = xgetc(fp);
	else if (c == '-') {
		sign = TRUE;
		c = xgetc(fp);
	}

	if (!isdigit(c)) {
		if (fBootDb)
			db_error("fread_number", "bad format");
		log("fread_number: bad format");
		exit(1);
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c      = xgetc(fp);
	}

	if (sign)
		number = 0 - number;

	if (c == '|')
		number += fread_number(fp);
	else if (c != ' ')
		xungetc(c, fp);

	return number;
}

flag64_t fread_flags(FILE *fp)
{
	flag64_t number;
	char c;
	bool negative = FALSE;

	do
		c = xgetc(fp);
	while (isspace(c));

	if (c == '-') {
		negative = TRUE;
		c = xgetc(fp);
	}

	number = 0;

	if (!isdigit(c)) {
		while (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z')) {
			number += flag_convert(c);
			c = xgetc(fp);
		}
	}

	while (isdigit(c)) {
		number = number * 10 + c - '0';
		c = xgetc(fp);
	}

	if (c == '|')
		number += fread_flags(fp);
	else if (c != ' ')
		xungetc(c, fp);

	if (negative)
		return -number;

	return number;
}

flag64_t flag_convert(char letter)
{
	flag64_t bitsum = 0;
	char i;

	if ('A' <= letter && letter <= 'Z') {
		bitsum = A;
		for (i = letter; i > 'A'; i--)
			bitsum <<= 1;
	}
	else if ('a' <= letter && letter <= 'z') {
		bitsum = aa;
		for (i = letter; i > 'a'; i--)
			bitsum <<= 1;
	}

	return bitsum;
}

/*
 * Read to end of line (for comments).
 */
void fread_to_eol(FILE *fp)
{
	char c;

	do
		c = xgetc(fp);
	while (c != '\n' && c != '\r');

	do
		c = xgetc(fp);
	while (c == '\n' || c == '\r');

	xungetc(c, fp);
	return;
}

/*
 * Read one word (into static buffer).
 */
char *fread_word(FILE *fp)
{
	static char word[MAX_INPUT_LENGTH];
	char *pword;
	char cEnd;

	do
		cEnd = xgetc(fp);
	while (isspace(cEnd));

	if (cEnd == '\'' || cEnd == '"')
		pword   = word;
	else {
		word[0] = cEnd;
		pword   = word+1;
		cEnd    = ' ';
	}

	for (; pword < word + MAX_INPUT_LENGTH; pword++)
	{
		*pword = xgetc(fp);
		if (cEnd == ' ' ? isspace(*pword) : *pword == cEnd)
		{
		    if (cEnd == ' ')
			xungetc(*pword, fp);
		    *pword = '\0';
		    return word;
		}
	}
	db_error("fread_word" , "word too long");
	return NULL;
}

/*
 * Returns an initial-capped string.
 */
char *capitalize(const char *str)
{
	static char strcap[MAX_STRING_LENGTH];
	int i;

	for (i = 0; str[i] != '\0'; i++)
		strcap[i] = LOWER(str[i]);
	strcap[i] = '\0';
	strcap[0] = UPPER(strcap[0]);
	return strcap;
}

#define NBUF 5
#define NBITS 52

char *format_flags(flag64_t flags)
{
	static int cnt;
	static char buf[NBUF][NBITS+1];
	int count, pos = 0;

	cnt = (cnt + 1) % NBUF;

	for (count = 0; count < NBITS;  count++)
		if (IS_SET(flags, (flag64_t) 1 << count)) {
	        	if (count < 26)
	        		buf[cnt][pos] = 'A' + count;
	        	else
				buf[cnt][pos] = 'a' + (count - 26);
			pos++;
		}

	if (pos == 0) 
		buf[cnt][pos++] = '0';

	buf[cnt][pos] = '\0';
	return buf[cnt];
}

void db_error(const char* fn, const char* fmt,...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (fBootDb) {
		log_printf("%s: line %d: %s: %s",
			   filename, line_number, fn, buf);
		exit(1);
	}

	log_printf("%s: %s", fn, buf);
}

/*
 * read flag word (not f-word :)
 */
flag64_t fread_fword(const flag_t *table, FILE *fp)
{
	char *name = fread_word(fp);

	if (is_number(name))
		return atoi(name);

	return flag_value(table, name);
}

flag64_t fread_fstring(const flag_t *table, FILE *fp)
{
	const char *s = fread_string(fp);
	flag64_t val;

	if (is_number(s))
		val = atoi(s);
	else
		val = flag_value(table, s);

	free_string(s);
	return val;
}

bool fread_addr(FILE *fp, struct sockaddr_in *addr)
{
	struct hostent *h;
	const char *s = fread_string(fp);
	int port = fread_number(fp);

	memset(addr, 0, sizeof(*addr));
	if (IS_NULLSTR(s)
	|| !str_cmp(s, "def") || !str_cmp(s, "auto") || !str_cmp(s, "all"))
	{
#if !defined (WIN32)
		addr->sin_family = AF_INET;
#else
		addr->sin_family = PF_INET;
		addr->sin_addr.s_addr = INADDR_ANY;
#endif
	} else {
		h = gethostbyname(s);
		if (!h)
		{
			log_printf("fread_addr: gethostbyname: %s",
					hstrerror(h_errno));
			return FALSE;
		} else {
			memcpy(&(addr->sin_addr),
					h->h_addr,
					h->h_length);
			addr->sin_family = h->h_addrtype;
		}
	}

	addr->sin_port = htons(port);
	free_string(s);

	return TRUE;
}

