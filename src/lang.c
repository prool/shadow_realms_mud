/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

#include <string.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

#include "merc.h"
#include "lang.h"
#include "db.h"

/*----------------------------------------------------------------------------
 * main language support functions
 */

static const char*
word_form_lookup(lang_t *l, rulecl_t *rcl, const char *word, int fnum)
{
	rule_t *rule;
	char **p;
	const char *q;
	char *k;
	static char buf[MAX_STRING_LENGTH];
	static char word_noncolor[MAX_STRING_LENGTH];

	if (!fnum || IS_NULLSTR(word))
		return word;

	/*
	 * variable part(s) of word can be specified by tildes
	 * (simple recursion)
 	 */
	if ((q = strchr(word, '~'))) {
		char buf2[MAX_STRING_LENGTH];
		char buf3[MAX_STRING_LENGTH];
		char *r;

		/* copy prefix */
		strnzncpy(buf2, sizeof(buf2), word, q-word);

		/*
		 * translate infix, translation must be done
		 * before copying the result to buf[] because buf is
		 * static
		 */
		r = strchr(q+1, '~');
		if (!r)
			r = strchr(q+1, '\0');
		strnzncpy(buf3, sizeof(buf3), q+1, *r ? r-q-1 : r-q);
		strnzcat(buf2, sizeof(buf2),
			 word_form_lookup(l, rcl, buf3, fnum));

		/* translate the rest */
		if (!*r) {
			strnzcpy(buf, sizeof(buf), buf2);
			return buf;
		}

		if (strchr(r+1, '~')) {
			strnzcpy(buf3, sizeof(buf3),
				 word_form_lookup(l, rcl, r+1, fnum));
			q = buf3;
		}
		else
			q = r+1;

		strnzcpy(buf, sizeof(buf), buf2);
		strnzcat(buf, sizeof(buf), q);

		return buf;
	}
	
	/*
	 *	Cut color from word.
	 */
	for(q = word, k = word_noncolor; *q; q++)
	{
		if (*q == '{' && *(q+1))
		{
			q++;
			continue;
		}
		*k++= *q;
	}
	*k = '\0';

	/*
	 * explicit rule lookup
	 */
	if ((rule = erule_lookup(rcl, word_noncolor)) == NULL) {
		rule_t e;

		/*
		 * implicit rule lookup
		 */
		if ((rule = irule_find(rcl, word_noncolor)) == NULL)
			return word;

		/*
		 * implicit rule found - create explicit rule and use it
		 */
		erule_create(&e, rule, word_noncolor);
		rule = erule_add(rcl, &e);
		SET_BIT(rcl->flags, RULES_EXPL_CHANGED);
	}

	if ((p = varr_get(&rule->f->v, fnum)) == NULL
	||  IS_NULLSTR(*p))
		return word_noncolor;

	if (rule->arg <= 0 || **p != '-')
		return *p;

	strnzcpy(buf, sizeof(buf), word_noncolor);
	strnzcpy(buf + rule->arg, sizeof(buf) - rule->arg, *p + 1);
	return buf;
}

const char *word_form(const char *word, int fnum, int lang, int rulecl)
{
	lang_t *l;

	if ((rulecl < 0 || rulecl >= MAX_RULECL)
	||  (l = varr_get(&langs, lang)) == NULL)
		return word;

	switch (rulecl) {
	case RULES_GENDER:
		fnum = (fnum + SEX_MAX - 1) % SEX_MAX;
		break;

	case RULES_QTY:
		fnum %= 100;
		if (fnum > 14)
			fnum %= 10;
	}

	return word_form_lookup(l, l->rules + rulecl, word, fnum);
}

/*----------------------------------------------------------------------------
 * vform_t functions
 */
vform_t *vform_new(void)
{
	vform_t *f = calloc(1, sizeof(*f));
	if (f == NULL)
		crush_mud();
	f->v.nsize = sizeof(char*);
	f->v.nstep = 4;
	f->ref = 1;
	return f;
}

vform_t *vform_dup(vform_t *f)
{
	f->ref++;
	return f;
}

void vform_free(vform_t *f)
{
	int i;

	if (--f->ref)
		return;
	for (i = 0; i < f->v.nused; i++)
		free_string(VARR_GET(&f->v, i));
	varr_free(&f->v);
	free(f);
}

void vform_add(vform_t *f, int fnum, const char *s)
{
	const char **p = varr_touch(&f->v, fnum);
	if (*p)
		free_string(*p);
	*p = str_dup(s);
}

void vform_del(vform_t *f, int fnum)
{
	const char **p = varr_get(&f->v, fnum);
	if (*p)
		free_string(*p);
	*p = NULL;
}

/*----------------------------------------------------------------------------
 * rule_t functions
 */

#define rulehash(s) hashistr(s, 16, MAX_RULE_HASH)

/* reverse order (otherwise word_del will not work) */
static int cmprule(const void *p1, const void *p2)
{
	return -str_cmp(((rule_t*) p1)->name, ((rule_t*) p2)->name);
}

void rule_init(rule_t *r)
{
	r->name = NULL;
	r->arg = 0;
	r->f = vform_new();
}

void rule_clear(rule_t *r)
{
	free_string(r->name);
	vform_free(r->f);
	r->name = NULL;
}

void erule_create(rule_t *expl, rule_t *impl, const char *word)
{
	rule_init(expl);
	expl->name = str_dup(word);
	expl->arg = strlen(word) + impl->arg;
	expl->f = vform_dup(impl->f);
}

/*----------------------------------------------------------------------------
 * implicit rules operations
 */
rule_t *irule_add(rulecl_t *rcl, rule_t *r)
{
	rule_t *rnew = varr_enew(&rcl->impl);
	*rnew = *r;
	return rnew;
}

rule_t *irule_insert(rulecl_t *rcl, size_t num, rule_t *r)
{
	rule_t *rnew;
	if (num > rcl->impl.nused)
		num = rcl->impl.nused;
	rnew = varr_insert(&rcl->impl, num);
	*rnew = *r;
	return rnew;
}

void irule_del(rulecl_t *rcl, rule_t *r)
{
	rule_clear(r);
	varr_del(&rcl->impl, r);
}

rule_t *irule_lookup(rulecl_t *rcl, const char *num)
{
	char *q;
	size_t i;

	if (IS_NULLSTR(num))
		return NULL;

	i = strtoul(num, &q, 0);
	if (*q || i >= rcl->impl.nused)
		return NULL;

	return VARR_GET(&rcl->impl, i);
}

rule_t *irule_find(rulecl_t *rcl, const char *word)
{
	int i;

	for (i = 0; i < rcl->impl.nused; i++) {
		rule_t *r = VARR_GET(&rcl->impl, i);
		if (r->name[0] == '-'
		&&  !strchr(word, ' ')
		&&  !str_suffix(r->name+1, word))
			return r;
	}

	return NULL;
}

/*----------------------------------------------------------------------------
 * explicit rules operations
 */
rule_t *erule_add(rulecl_t *rcl, rule_t *r)
{
	rule_t *rnew;
	varr *v;

	if (IS_NULLSTR(r->name))
		return NULL;

	v = rcl->expl + rulehash(r->name);
	if (varr_bsearch(v, r, cmprule))
		return NULL;

	rnew = varr_enew(v);
	*rnew = *r;
	varr_qsort(v, cmprule);
	return varr_bsearch(v, r, cmprule);
}

void erule_del(rulecl_t *rcl, rule_t *r)
{
	varr *v;

	v = rcl->expl + rulehash(r->name);
	rule_clear(r);
	varr_qsort(v, cmprule);
	v->nused--;
}

rule_t *erule_lookup(rulecl_t *rcl, const char *name)
{
	if (IS_NULLSTR(name))
		return NULL;
	return varr_bsearch(rcl->expl + rulehash(name), &name, cmprule);
}

/*----------------------------------------------------------------------------
 * rulecl_t functions
 */
static void rulecl_init(lang_t *l, int rulecl)
{
	int i;
	rulecl_t *rcl = l->rules + rulecl;

	rcl->rulecl = rulecl;
	for (i = 0; i < MAX_RULE_HASH; i++) {
		rcl->expl[i].nsize = sizeof(rule_t);
		rcl->expl[i].nstep = 4;
	}
	rcl->impl.nsize = sizeof(rule_t);
	rcl->impl.nstep = 4;
}

/*----------------------------------------------------------------------------
 * lang_t functions
 */
varr langs = { sizeof(lang_t), 2 };

lang_t *lang_new(void)
{
	int i;
	lang_t *l = varr_enew(&langs);

	l->slang_of = -1;
	l->vnum = langs.nused-1;

	for (i = 0; i < MAX_RULECL; i++)
		rulecl_init(l, i);

	return l;
}

int lang_lookup(const char *name)
{
	return lang_nlookup(name, strlen(name));
}

int lang_nlookup(const char *name, size_t len)
{
	int lang;

	if (IS_NULLSTR(name))
		return -1;

	for (lang = 0; lang < langs.nused; lang++) {
		lang_t *l = VARR_GET(&langs, lang);
		if (str_ncmp(l->name, name, len) == 0)
			return lang;
	}

	if (fBootDb)
		db_error("lang_lookup", "%s: unknown language", name);
	return -1;
}

