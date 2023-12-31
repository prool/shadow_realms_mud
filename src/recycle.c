/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: recycle.c,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

/***************************************************************************
 *     ANATOLIA 2.1 is copyright 1996-1997 Serdar BULUT, Ibrahim CANPUNAR  *
 *     ANATOLIA has been brought to you by ANATOLIA consortium		   *
 *	 Serdar BULUT {Chronos}		bulut@rorqual.cc.metu.edu.tr       *
 *	 Ibrahim Canpunar  {Asena}	canpunar@rorqual.cc.metu.edu.tr    *	
 *	 Murat BICER  {KIO}		mbicer@rorqual.cc.metu.edu.tr	   *	
 *	 D.Baris ACAR {Powerman}	dbacar@rorqual.cc.metu.edu.tr	   *	
 *     By using this code, you have agreed to follow the terms of the      *
 *     ANATOLIA license, in the file Anatolia/anatolia.licence             *	
 ***************************************************************************/

/***************************************************************************
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,        *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *                                                                         *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Much time and thought has gone into this software and you are          *
 *  benefitting.  We hope that you share your changes too.  What goes      *
 *  around, comes around.                                                  *
 ***************************************************************************/
 
/***************************************************************************
*	ROM 2.4 is copyright 1993-1995 Russ Taylor			   *
*	ROM has been brought to you by the ROM consortium		   *
*	    Russ Taylor (rtaylor@pacinfo.com)				   *
*	    Gabrielle Taylor (gtaylor@pacinfo.com)			   *
*	    Brian Moore (rom@rom.efn.org)				   *
*	By using this code, you have agreed to follow the terms of the	   *
*	ROM license, in the file Rom24/doc/rom.license			   *
***************************************************************************/

#include <sys/types.h>
#include <sys/time.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"
#include "db.h"

/* stuff for recycling extended descs */
extern int top_ed;
extern void crush_mud(void);

ED_DATA *ed_new(void)
{
	ED_DATA *ed;
	ed = calloc(1, sizeof(*ed));
	if (ed == NULL)
		crush_mud();
	top_ed++;
	return ed;
}

ED_DATA *ed_new2(const ED_DATA *ed, const char* name)
{
	ED_DATA *ed2		= ed_new();
	ed2->keyword		= str_qdup(ed->keyword);
	ed2->description	= mlstr_printf(ed->description, name);
	return ed2;
}

ED_DATA *ed_dup(const ED_DATA *ed)
{
	ED_DATA *ned = NULL;
	ED_DATA **ped = &ned;

	for (; ed; ed = ed->next) {
		*ped = ed_new();
		(*ped)->keyword		= str_qdup(ed->keyword);
		(*ped)->description	= mlstr_dup(ed->description);
		ped = &(*ped)->next;
	}

	return ned;
}

void ed_free(ED_DATA *ed)
{
	ED_DATA *ed_next;

	for (; ed; ed = ed_next) {
		ed_next = ed->next;

		free_string(ed->keyword);
		mlstr_free(ed->description);
		free(ed);
		top_ed--;
	}
}

void ed_fread(FILE *fp, ED_DATA **edp)
{
	ED_DATA *ed	= ed_new();
	char	*str;
	int	sn;
	
	str = fread_word(fp);

	if (IS_NULLSTR(str)
	|| !str_cmp(str, "none"))
		sn = 0;
	else {
		sn = slang_lookup(str);
		if (sn < 0
		|| !IS_SET(SKILL(sn)->group, GROUP_SLANG))
		{
			log_printf("Warning! Unkown slang in ED_DATA: '%s' [%s: line %d]",
				str, filename, line_number);
			sn = 0;
		}
	}
	ed->slang       = sn;
	ed->keyword	= fread_string(fp);
	ed->description	= mlstr_fread(fp);
	SLIST_ADD(ED_DATA, *edp, ed);
}

void ed_fwrite(FILE *fp, ED_DATA *ed, const char *icon)
{
       	fprintf(fp, "%s '%s'\n%s~\n",
       		IS_NULLSTR(icon) ? "E" : icon,
       		ed->slang && skill_lookup(ed->slang)
       			? SKILL(ed->slang)->name : str_empty,
       		fix_string(ed->keyword));
	mlstr_fwrite(fp, NULL, ed->description);
	
}

AFFECT_DATA *aff_new(void)
{
	void *tmp;
	
	tmp = calloc(1, sizeof(AFFECT_DATA));
	if (tmp)
		return tmp;
	log_printf("Cann't alloc memory (aff_new) for new affect.");
	crush_mud();
	return NULL;
}

AFFECT_DATA *aff_dup(const AFFECT_DATA *paf)
{
	AFFECT_DATA *naf = aff_new();
	naf->where	= paf->where;
	naf->type	= paf->type;
	naf->level	= paf->level;
	naf->duration	= paf->duration;
	naf->location	= paf->location;
	naf->modifier	= paf->modifier;
	naf->bitvector	= paf->bitvector;
	return naf;
}

void aff_free(AFFECT_DATA *af)
{
	free(af);
}

OBJ_DATA *free_obj_list;

extern int obj_count;
extern int obj_free_count;

OBJ_DATA *new_obj(void)
{
	OBJ_DATA *obj;

	if (free_obj_list) {
		obj = free_obj_list;
		free_obj_list = free_obj_list->next;
		memset(obj, '\0', sizeof(*obj));
		obj_free_count--;
	}
	else {
		obj = calloc(1, sizeof(*obj));
		if (obj == NULL)
			crush_mud();
		obj_count++;
	}

	return obj;
}

void free_obj(OBJ_DATA *obj)
{
	AFFECT_DATA *paf, *paf_next;

	if (!obj)
		return;

	for (paf = obj->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		aff_free(paf);
    	}
	obj->affected = NULL;

	ed_free(obj->ed);
	obj->ed = NULL;
   
	free_string(obj->name);
	obj->name = NULL;

	mlstr_free(obj->description);
	obj->description = NULL;

	mlstr_free(obj->short_descr);
	obj->short_descr = NULL;

	mlstr_free(obj->owner);
	obj->owner = NULL;

//	free_string(obj->material);
//	obj->material = NULL;

	obj->next = free_obj_list;
	free_obj_list = obj;

	obj_free_count++;
}

CHAR_DATA *free_char_list;

extern int mob_count;
extern int mob_free_count;

CHAR_DATA *new_char (void)
{
	CHAR_DATA *ch;
	int i;

	if (free_char_list) {
		ch = free_char_list;
		free_char_list = free_char_list->next;
		//	memset(ch, '\0', sizeof(*ch));
		memset(ch, 0, sizeof(CHAR_DATA));
		mob_free_count--;
	}
	else {
		ch = calloc(1, sizeof(*ch));
		if (ch == NULL)
			crush_mud();
		mob_count++;
	}
	REMOVE_BIT(ch->form, FORM_EXTRACTED);

	RESET_FIGHT_TIME(ch);
	ch->last_death_time	= -1;

	ch->hit			= 20;
	ch->max_hit		= 20;
	ch->mana		= 100;
	ch->max_mana		= 100;
	ch->move		= 50;
	ch->max_move		= 50;
	ch->dam_type		= 17;      /* punch */
	ch->position		= POS_STANDING;
	for (i = 0; i < 4; i++)
		ch->armor[i]	= -100;
	for (i = 0; i < MAX_STATS; i ++)
		ch->perm_stat[i] = 13;
	ch->slang		= gsn_common_slang;
	ch->nsp_spool		= -1;
	return ch;
}

void free_char(CHAR_DATA *ch)
{
	OBJ_DATA *obj;
	OBJ_DATA *obj_next;
	AFFECT_DATA *paf;
	AFFECT_DATA *paf_next;

	if (!ch)
		return;

	nuke_pets(ch);
	break_horse(ch);

	for (obj = ch->carrying; obj; obj = obj_next) {
		obj_next = obj->next_content;
		extract_obj(obj, XO_F_NOCOUNT);
	}

	for (paf = ch->affected; paf; paf = paf_next) {
		paf_next = paf->next;
		affect_remove(ch,paf);
	}
	ch->affected = NULL;

	free_string(ch->name);
	ch->name = NULL;

	mlstr_free(ch->short_descr);
	ch->short_descr = NULL;

	mlstr_free(ch->long_descr);
	ch->long_descr = NULL;

	mlstr_free(ch->description);
	ch->description = NULL;


//	free_string(ch->material);
//	ch->material = NULL;

	free_string(ch->in_mind);
	ch->in_mind = NULL;

	free_pcdata(ch->pcdata);
	ch->pcdata = NULL;

	ch->next = free_char_list;
	free_char_list = ch;

	SET_BIT(ch->form, FORM_EXTRACTED);	// !!!
	mob_free_count++;
}

PC_DATA *new_pcdata(void)
{
	PC_DATA *pcdata;
	pcdata = calloc(1, sizeof(*pcdata));
	if (pcdata == NULL)
		crush_mud();
	pcdata->buffer = buf_new(-1);
	pcdata->learned.nsize = sizeof(pcskill_t);
	pcdata->learned.nstep = 8;
	pcdata->pwd = str_empty;
	pcdata->bamfin = str_empty;
	pcdata->bamfout = str_empty;
	pcdata->title = str_empty;
	pcdata->twitlist = str_empty;
	pcdata->granted = str_empty;
	pcdata->last_host = str_empty;
	pcdata->prefix		= str_empty;
	pcdata->lines		= PAGELEN;
	pcdata->logon		= current_time;

	return pcdata;
}

void free_pcdata(PC_DATA *pcdata)
{
	int alias;

	if (!pcdata)
		return;

	varr_free(&pcdata->learned);
	free_string(pcdata->pwd);
	free_string(pcdata->bamfin);
	free_string(pcdata->bamfout);
	free_string(pcdata->title);
	free_string(pcdata->twitlist);
	free_string(pcdata->granted);
	free_string(pcdata->prompt);
	free_string(pcdata->prefix);
	free_string(pcdata->last_host);
	free_remort(pcdata->remort);
	pcdata->remort = NULL;
	free_religion_char(pcdata->religion);
	pcdata->religion = NULL;
	free_quest_char(pcdata->quest);
	pcdata->quest = NULL;

	buf_free(pcdata->buffer);
    
	for (alias = 0; alias < MAX_ALIAS; alias++) {
		free_string(pcdata->alias[alias]);
		free_string(pcdata->alias_sub[alias]);
	}
	free(pcdata);
}

QUEST_CHAR	*new_quest_char(void)
{
	QUEST_CHAR *quest;
	
	quest = calloc(1, sizeof(*quest));
	if (!quest)
		crush_mud();
	return quest;
}

void		free_quest_char(QUEST_CHAR *quest)
{
	if (!quest)
		return;
	free(quest);
}

RELIGION_CHAR	*new_religion_char(void)
{
	RELIGION_CHAR	*religion;
	religion = calloc(1, sizeof(*religion));
	if (!religion)
		crush_mud();
	return religion;
}

void		free_religion_char(RELIGION_CHAR *relig)
{
	if (!relig)
		return;
	free(relig);
}

REMORT_DATA	*new_remort(void)
{
	REMORT_DATA *remort;
	remort = calloc(1, sizeof(*remort));
	if (remort == NULL)
		crush_mud();
	remort->skills.nsize = sizeof(pcskill_t);
	remort->skills.nstep = 2;
	remort->shop = NULL;
	remort->points = START_RP;
	
	return remort;
}
	
void	free_remort(REMORT_DATA *remort)
{
	if (!remort)
		return;
	varr_free(&remort->skills);
	free_rshop(remort->shop);
	free(remort);
}

RSHOP_DATA	*new_rshop(void)
{
	RSHOP_DATA	*shop;
	
	shop = calloc(1, sizeof(*shop));
	if (shop == NULL)
		crush_mud();
	shop->skills.nsize = sizeof(pcskill_t);
	shop->skills.nstep = 1;
	shop->ethos = -1;
	shop->align = -9999;
	shop->race = -1;
	shop->sex = -1;
	shop->class = -1;
	return shop;
}

void	free_rshop(RSHOP_DATA *shop)
{
	if (!shop)
		return;
	varr_free(&shop->skills);
	free(shop);
}

/* stuff for setting ids */
long	last_pc_id;
long	last_mob_id;

long get_pc_id(void)
{
	return last_pc_id = (current_time <= last_pc_id) ?
			    last_pc_id + 1 : current_time;
}

long get_mob_id(void)
{
	last_mob_id++;
	return last_mob_id;
}
    
MPTRIG *mptrig_new(int type, const char *phrase, int vnum)
{
	const char *p;
	MPTRIG *mptrig;

	mptrig = calloc(1, sizeof(*mptrig));
	if (mptrig == NULL)
		crush_mud();
	mptrig->type	= type;
	mptrig->vnum	= vnum;
	mptrig->phrase	= str_dup(phrase);

	for (p = mptrig->phrase; *p; p++)
		if (ISUPPER(*p)) {
			SET_BIT(mptrig->flags, TRIG_CASEDEP);
			break;
		}

	if ((type == TRIG_ACT || type == TRIG_SPEECH) && phrase[0] == '*') {
		int errcode;
		int cflags = REG_EXTENDED | REG_NOSUB;

		SET_BIT(mptrig->flags, TRIG_REGEXP);
		if (!IS_SET(mptrig->flags, TRIG_CASEDEP))
			cflags |= REG_ICASE;

		mptrig->extra = malloc(sizeof(regex_t));
		if (mptrig->extra == NULL)
			crush_mud();
		errcode = regcomp(mptrig->extra, phrase+1, cflags);
		if (errcode) {
			char buf[MAX_STRING_LENGTH];

			regerror(errcode, mptrig->extra, buf, sizeof(buf));
			log_printf("bad trigger for vnum %d (phrase '%s'): %s",
				   vnum, phrase, buf);
		}
	}
		
	return mptrig;
}

void mptrig_add(MOB_INDEX_DATA *mob, MPTRIG *mptrig)
{
	SET_BIT(mob->mptrig_types, mptrig->type);
	SLIST_ADD(MPTRIG, mob->mptrig_list, mptrig);
}

void mptrig_fix(MOB_INDEX_DATA *mob)
{
	MPTRIG *mptrig;

	for (mptrig = mob->mptrig_list; mptrig; mptrig = mptrig->next)
		SET_BIT(mob->mptrig_types, mptrig->type);
}

void mptrig_free(MPTRIG *mp)
{
	if (!mp)
		return;

	if (IS_SET(mp->flags, TRIG_REGEXP)) {
		regfree(mp->extra);
		free(mp->extra);
	}

	free_string(mp->phrase);
	free(mp);
}
