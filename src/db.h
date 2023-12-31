/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

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
 * $Id: db.h,v 1.32 2003/05/18 23:46:25 xor Exp $
 */

#ifndef _DB_H_
#define _DB_H_

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
 *  Original Diku Mud copyright (C) 1990, 1991 by Sebastian Hammer,	   *
 *  Michael Seifert, Hans Henrik St{rfeldt, Tom Madsen, and Katja Nyboe.   *
 *									   *
 *  Merc Diku Mud improvments copyright (C) 1992, 1993 by Michael	   *
 *  Chastain, Michael Quan, and Mitchell Tse.				   *
 *									   *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc	   *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.						   *
 *									   *
 *  Much time and thought has gone into this software and you are	   *
 *  benefitting.  We hope that you share your changes too.  What goes	   *
 *  around, comes around.						   *
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

typedef struct dbdata DBDATA;

typedef void DBLOAD_FUN(DBDATA *dbdata, FILE *fp, void *arg);
#define DECLARE_DBLOAD_FUN(fun) DBLOAD_FUN fun
#define DBLOAD_FUN(fun) void fun(DBDATA *dbdata, FILE *fp, void *arg)

typedef void DBINIT_FUN(DBDATA *dbdata);
#define DECLARE_DBINIT_FUN(fun) DBINIT_FUN fun
#define DBINIT_FUN(fun) void fun(DBDATA *dbdata)

struct dbfun {
	char *		name;
	DBLOAD_FUN *	fun;
	void *		arg;
};
typedef struct dbfun DBFUN;

struct dbdata {
	DBFUN *		fun_tab;	/* table of parsing functions	*/
	DBINIT_FUN *	dbinit;		/* init function		*/
	size_t		tab_sz;		/* table size			*/
};

extern DBDATA db_areas;
//extern DBDATA db_clans;
extern DBDATA db_classes;
extern DBDATA db_hometowns;
extern DBDATA db_langs;
extern DBDATA db_races;
extern DBDATA db_skills;
extern DBDATA db_socials;
extern DBDATA db_system;
extern DBDATA db_religion;
extern DBDATA db_damages;

void db_load_file(DBDATA *, const char *path, const char *file);
void db_set_arg(DBDATA *, const char* name, void *arg);


void crush_mud(void);
void reboot_mud(void);

void	load_oldmsgdb	(void);
void	load_msgdb	(void);
void	load_notes	(void);
void	load_bans	(void);

void	vnum_check	(AREA_DATA *area, int vnum);

void	convert_objects	(void);
void	convert_object	(OBJ_INDEX_DATA *pObjIndex);

void	reset_area      (AREA_DATA * pArea);
void	reset_room	(ROOM_INDEX_DATA *pRoom);

void *		fread_namedp	(namedp_t *table, FILE *fp);
//int		fread_clan	(FILE *fp);


//extern varr	list_damages_msg;

#define SLIST_ADD(type, list, item)					\
	{								\
		if ((list) == NULL)					\
			(list) = (item);				\
		else {							\
			type *p;					\
									\
			for (p = (list); p->next != NULL; p = p->next)	\
				;					\
			p->next = (item);				\
		}							\
		(item)->next = NULL;					\
	}

#define KEY(literal, field, value)			\
		if (!str_cmp(word, literal)) {		\
			field  = value;			\
			fMatch = TRUE;			\
			break;				\
		}

#define SKEY(string, field)				\
		if (!str_cmp(word, string)) {		\
			free_string(field);		\
			field = fread_string(fp);	\
			fMatch = TRUE;			\
			break;				\
		}

#define MLSKEY(string, field)				\
		if (!str_cmp(word, string)) {		\
			mlstr_free(field);		\
			field = mlstr_fread(fp);	\
			fMatch = TRUE;			\
			break;				\
		}

extern int		newmobs;
extern int		newobjs;
extern MOB_INDEX_DATA *	mob_index_hash	[MAX_KEY_HASH];
extern OBJ_INDEX_DATA *	obj_index_hash	[MAX_KEY_HASH];
extern ROOM_INDEX_DATA *room_index_hash [MAX_KEY_HASH];
extern int		top_mob_index;
extern int		top_obj_index;
extern int		top_vnum_mob;
extern int		top_vnum_obj;
extern int		top_vnum_room;
extern int  		top_affect;
extern int		top_ed; 
extern int		top_area;
extern int		top_exit;
extern int		top_help;
extern int		top_reset;
extern int		top_room;
extern int		top_shop;
extern int		social_count;
extern AREA_DATA *	area_first;
extern AREA_DATA *	area_last;
extern AREA_DATA *	area_current;
extern SHOP_DATA *	shop_last;


/* mud server options (etc/system.conf) */
#define OPT_ASCII_ONLY_NAMES	(A)
extern flag32_t		mud_options;

#endif

