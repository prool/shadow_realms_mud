/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
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

#if defined(macintosh)
#include <types.h>
#include <time.h>
#else
#include <sys/types.h>
#include <sys/time.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#if !defined (WIN32)
#include <unistd.h>
#endif
#include <limits.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <libmccp.h>

#include "merc.h"
//#include "interp.h"
#include "update.h"
#include "quest.h"
#include "obj_prog.h"
#include "fight.h"
#include "quest.h"
#include "cmd.h"
#include "db.h"
#include "olc.h"
#include "magic.h"

/* command procedures needed */
DECLARE_DO_FUN(do_rstat	);
DECLARE_DO_FUN(do_mstat	);
DECLARE_DO_FUN(do_dstat	);
DECLARE_DO_FUN(do_ostat	);
DECLARE_DO_FUN(do_rset	);
DECLARE_DO_FUN(do_mset	);
DECLARE_DO_FUN(do_oset	);
DECLARE_DO_FUN(do_sset	);
DECLARE_DO_FUN(do_mfind	);
DECLARE_DO_FUN(do_ofind	);
DECLARE_DO_FUN(do_mload	);
DECLARE_DO_FUN(do_oload	);
DECLARE_DO_FUN(do_save	);
DECLARE_DO_FUN(do_look	);
DECLARE_DO_FUN(do_stand	);
DECLARE_DO_FUN(do_help	);

qtrouble_t *qtrouble_lookup(CHAR_DATA *ch, int vnum);

//bool write_to_descriptor  (int desc, char *txt, int length);
extern int rebooter;

void show_obj_values(BUFFER *output, OBJ_INDEX_DATA *pObj);
float get_obj_vc(OBJ_INDEX_DATA *obj, int level);

void show_sspool(CHAR_DATA *, int);

void do_world(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0' || !IS_PC(ch) || argument[0] == '\0') {
		char_puts("Syntax:\n"
				"   world show sspool\n", ch);
		return;
	}

	if (!str_prefix(arg,"show")) {
		argument = one_argument(argument, arg, sizeof(arg));
		if (!str_prefix(arg,"sspool")) {
			show_sspool(ch, atoi(argument));
		}
	}
}

void brief_show_obj_index(OBJ_INDEX_DATA *pObj, BUFFER *buf)
{
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	int cnt;
	where_t *w;
	int i;
	ROOM_INDEX_DATA *room;
	RESET_DATA *reset;

	buf_printf(buf, "\n#Obj[%6d]: [%s] (%s)\n", pObj->vnum,
		mlstr_mval(pObj->short_descr), flag_string(item_types, pObj->item_type));
	if (pObj->limit != -1)
		snprintf(arg, sizeof(arg), "  Limited by %d", pObj->limit);
	else
		arg[0] = '\0';
	buf_printf(buf, "Level:       [%3d] VC[%.2f]%s\n",
		pObj->level, get_obj_vc(pObj, pObj->level), arg);
	buf_printf(buf, "Wear flags:  [%s]\n",
		flag_string(wear_flags, pObj->wear_flags));
	buf_printf(buf, "Extra flags: [%s]\n",
		flag_string(extra_flags, pObj->extra_flags));
	buf_printf(buf, "Condition:   [%3d]  Material:    [%s]\n",
		pObj->condition, pObj->material);
	buf_printf(buf, "Weight:      [%5d]\nCost:        [%5d]\n",
		pObj->weight, pObj->cost);
	for (cnt = 0, paf = pObj->affected; paf; paf = paf->next) {
		w = where_lookup(paf->where);
		if (cnt == 0) {
			buf_add(buf, "Number      Affects Modifier Affects Bitvector\n");
			buf_add(buf, "------ ------------ -------- ------- -----------------------------------------\n");
		}
		buf_printf(buf, "[%4d] %12.12s",
				cnt, flag_string(apply_flags, paf->location));
		if (paf->location == APPLY_5_SKILL || paf->location == APPLY_10_SKILL)
			buf_printf(buf, " %-8s ", skill_name(paf->modifier));
		else
			buf_printf(buf, " %8d ", paf->modifier);
		buf_printf(buf, "%7.7s %s\n",
			flag_string(apply_types, paf->where),
			w ? flag_string(w->table, paf->bitvector) : "none");
		cnt++;
	}
	show_obj_values(buf, pObj);
	cnt = 0;
	for (i = 0; i < MAX_KEY_HASH; i++)
		for (room = room_index_hash[i]; room; room = room->next)
			for (reset = room->reset_first; reset; reset = reset->next)
			{
				bool found = FALSE;
				
				switch(reset->command)
				{
					case 'P':
					case 'O':
					case 'G':
					case 'E':
						if (reset->arg1 == pObj->vnum)
							found = TRUE;
						break;
						
					default:
						break;;
				}
				if (!found)
					continue;
				if (!(cnt++))
					buf_add(buf, "Obj has resets in following rooms: ");
				buf_printf(buf, "%d ", room->vnum);
				break;
			}
	if (cnt)
		buf_add(buf, ".\n");
	else
		buf_add(buf, "This obj hasn't resets !\n"); 
}

void do_objlist(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	FILE *fp;
	OBJ_DATA *obj;
	BUFFER *buf;
	

	if ((fp = dfopen(TMP_PATH, "objlist.txt", "w+")) == NULL) {
	 	char_puts("File error.\n", ch);
	 	return;
	}

	buf = buf_new(-1);
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	
	if (arg1[0] == '\0') {
		fprintf(fp, "This is dump obj, not pIndexObj !!!\n");
		for(obj = object_list; obj != NULL; obj = obj->next) {
			if (obj->pIndexData->affected == NULL)
				continue;

			buf_clear(buf);
			buf_printf(buf, "\n#Obj: %s (Vnum : %d) \n",
			   mlstr_mval(obj->short_descr),
			   obj->pIndexData->vnum);
			format_obj(buf, obj);
			if (!IS_SET(obj->hidden_flags, OHIDE_ENCHANTED))
				format_obj_affects(buf, obj->pIndexData->affected,
					   FOA_F_NODURATION);
			format_obj_affects(buf, obj->affected, 0);
			fprintf(fp, "%s", fix_string(buf_string(buf)));
		}
		char_puts("tmp/objlist.txt created\n", ch);
	} else if (!str_prefix(arg1, "sort")) {
		OBJ_INDEX_DATA *pObj;
		int iLevel;
		int iType;
		int iHash;
		
		if (arg2[0] == '\0')
		{
			buf_free(buf);
			fclose(fp);
			do_objlist(ch, "?");
			return;
		} else if (!str_prefix(arg2, "level")) {
			for(iLevel = 0; iLevel < 101; iLevel++)
			{
			  buf_printf(buf, "\n\n          >>>>> Level %d <<<<<\n\n", iLevel);
			  for(iType = 0; item_types[iType].name; iType++)
			  {
			    if (!item_types[iType].settable)
			  	continue;
			    buf_printf(buf, "\n\n === Type %s ===\n", item_types[iType].name);
			    for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
			      for (pObj = obj_index_hash[iHash]; pObj; pObj = pObj->next)
				if (pObj->level == iLevel && pObj->item_type == item_types[iType].bit)
				{
					brief_show_obj_index(pObj, buf);
					fprintf(fp, "%s", fix_string(buf_string(buf)));
					buf_clear(buf);
				}
			  }
			}
		} else if (!str_prefix(arg2, "type")) {
			for(iType = 0; item_types[iType].name; iType++)
			{
				if (!item_types[iType].settable)
					continue;
				buf_printf(buf, "\n\n          >>>>> Type %s <<<<<\n\n", item_types[iType].name);
				for(iLevel = 0; iLevel < 101; iLevel++)
				{
					buf_printf(buf, "\n\n === Level %d ===\n", iLevel);
					for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
					 for (pObj = obj_index_hash[iHash]; pObj; pObj = pObj->next)
					 	if (pObj->level == iLevel && pObj->item_type == item_types[iType].bit)
					 	{
					 		brief_show_obj_index(pObj, buf);
					 		fprintf(fp, "%s", fix_string(buf_string(buf)));
					 		buf_clear(buf);
					 	}
				}
			}
		} else if (!str_prefix(arg2, "vc")) {
			ROOM_INDEX_DATA *room;
			AFFECT_DATA *paf;
			RESET_DATA *reset;
			int i, res, afs;
			flag32_t wear;
			
			fprintf(fp, "VC     Vnum    Level Cost   Type  Res Afs Lim Wear  Flags Short descr\n");
			fprintf(fp, "------ ------- ----- ------ ----- --- --- --- ----- ----- ---------------------\n");
			for (iHash = 0; iHash < MAX_KEY_HASH; iHash++)
				for (pObj = obj_index_hash[iHash]; pObj; pObj = pObj->next)
			{
				res = afs = 0;
				for (i = 0; i < MAX_KEY_HASH; i++)
				    for (room = room_index_hash[i]; room; room = room->next)
					for (reset = room->reset_first; reset; reset = reset->next)
						switch(reset->command)
						{
							case 'P': case 'O':
							case 'G': case 'E':
								if (reset->arg1 == pObj->vnum)
									res++;
								break;
							default:
								break;
						}
				for (paf = pObj->affected; paf; paf = paf->next)
					afs++;
				if (!(wear = (pObj->wear_flags & ~(ITEM_TAKE))))
					wear = pObj->wear_flags;
				fprintf(fp, "%6.2f %7d %5d %6d %-5.5s %3d %3d %3d %-5.5s %-5.5s %-16s\n",
					get_obj_vc(pObj, pObj->level),
					pObj->vnum,
					pObj->level,
					pObj->cost,
					flag_string(item_types, pObj->item_type),
					res,
					afs,
					pObj->limit,
					flag_string(wear_flags, wear),
					format_flags(pObj->extra_flags),
					cutcolor(mlstr_mval(pObj->short_descr)));
			}
			char_puts("Use in shell for sort by vc next:\n"
				"    sort -g -k 1,1 -o vc_objlist.txt objlist.txt\n",
				ch);
		} else {
			buf_free(buf);
			fclose(fp);
			do_objlist(ch, "?");
			return;
		}
		char_puts("Done.\n", ch);					
	} else {
		char_puts("Syntax: objlist [ sort {{level | type | vc}]\n", ch);
	}

	buf_free(buf);
	fclose(fp);
}

void do_limited(CHAR_DATA *ch, const char *argument)
{
	extern int top_obj_index;
	OBJ_DATA *obj;
	OBJ_INDEX_DATA *obj_index;
	int	lCount = 0;
	int	ingameCount;
	int 	nMatch;
	int 	vnum;

	if (argument[0] != '\0')  {
	obj_index = get_obj_index(atoi(argument));
	if (obj_index == NULL)  {
	  char_puts("Not found.\n", ch);
	  return;
	}
	if (obj_index->limit == -1)  {
	  char_puts("Thats not a limited item.\n", ch);
	  return;
	}
	nMatch = 0;
	      char_printf(ch, "%-35s [%5d]  Limit: %3d  Current: %3d\n", 
			   mlstr_mval(obj_index->short_descr), 
			   obj_index->vnum,
		           obj_index->limit, 
			   obj_index->count);
		  ingameCount = 0;
		  for (obj=object_list; obj != NULL; obj=obj->next)
		    if (obj->pIndexData->vnum == obj_index->vnum)  {
		      ingameCount++;
		      if (obj->carried_by != NULL) 
			char_printf(ch, "Carried by %-30s\n", obj->carried_by->name);
		      else if (obj->in_room != NULL) 
			char_printf(ch, "At %-20s [%d]\n",
				mlstr_cval(obj->in_room->name, ch),
				obj->in_room->vnum);
		      else if (obj->in_obj != NULL) 
			char_printf(ch, "In %-20s [%d] \n",
				mlstr_mval(obj->in_obj->short_descr),
				obj->in_obj->pIndexData->vnum);
		    }
		    char_printf(ch, "  %d found in game. %d should be in pFiles.\n", 
				ingameCount, obj_index->count-ingameCount);
		return;
	}

	nMatch = 0;
	for (vnum = 0; nMatch < top_obj_index; vnum++)
	  if ((obj_index = get_obj_index(vnum)) != NULL)
	  {
	    nMatch++;
		if (obj_index->limit != -1)  {
		  lCount++;
	      char_printf(ch, "%-37s [%5d]  Limit: %3d  Current: %3d\n", 
			   mlstr_mval(obj_index->short_descr), 
			   obj_index->vnum,
		           obj_index->limit, 
			   obj_index->count);
		}
	  }
	char_printf(ch, "\n%d of %d objects are limited.\n", lCount, nMatch);
}

void do_wiznet(CHAR_DATA *ch, const char *argument)
{
	int flag;

	if (argument[0] == '\0') {
		/* show wiznet status */
		BUFFER *output;

		output = buf_new(-1);
		buf_printf(output, "Wiznet status: %s\n",
			   IS_SET(ch->pcdata->wiznet, WIZ_ON) ? "ON" : "OFF");

		buf_add(output, "\nchannel    | status");
		buf_add(output, "\n-----------|-------\n");
		for (flag = 0; wiznet_table[flag].name != NULL; flag++)
			buf_printf(output, "%-11s|  %s\n",
				   wiznet_table[flag].name,
				   wiznet_table[flag].level > ch->level ?
				   "N/A" :
				   IS_SET(ch->pcdata->wiznet,
					  wiznet_table[flag].flag) ?
				   "ON" : "OFF");
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	if (!str_prefix(argument,"on")) {
		char_puts("Welcome to Wiznet!\n", ch);
		SET_BIT(ch->pcdata->wiznet, WIZ_ON);
		return;
	}

	if (!str_prefix(argument,"off")) {
		char_puts("Signing off of Wiznet.\n", ch);
		REMOVE_BIT(ch->pcdata->wiznet, WIZ_ON);
		return;
	}

	flag = wiznet_lookup(argument);

	if (flag == -1 || ch->level < wiznet_table[flag].level) {
		char_puts("No such option.\n", ch);
		return;
	}
	 
	TOGGLE_BIT(ch->pcdata->wiznet, wiznet_table[flag].flag);
	if (!IS_SET(ch->pcdata->wiznet, wiznet_table[flag].flag))
		char_printf(ch,"You will no longer see %s on wiznet.\n",
		        wiznet_table[flag].name);
	else
		char_printf(ch, "You will now see %s on wiznet.\n",
			    wiznet_table[flag].name);
}

void wiznet(const char *msg, CHAR_DATA *ch, const void *arg,
	    flag32_t flag, flag32_t flag_skip, int min_level)
{
	DESCRIPTOR_DATA_MUDDY *d;

	for (d = descriptor_list_muddy; d; d = d->next)
	{
		CHAR_DATA *vch = d->original ? d->original : d->character;

		if (d->connected != CON_PLAYING
		||  vch == NULL
		||  vch->level < LEVEL_IMMORTAL
		||  vch->pcdata == NULL
		||  !IS_SET(vch->pcdata->wiznet, WIZ_ON)
		||  (flag && !IS_SET(vch->pcdata->wiznet, flag))
		||  (flag_skip && IS_SET(vch->pcdata->wiznet, flag_skip))
		||  vch->level < min_level
		||  vch == ch)
			continue;

		if (IS_SET(vch->pcdata->wiznet, WIZ_PREFIX))
			act_puts("--> ", vch, NULL, NULL, TO_CHAR | ACT_NOLF,
				 POS_DEAD);
		act_puts(msg, vch, arg, ch, TO_CHAR, POS_DEAD);
	}
}

void do_tick(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	
	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0')  {
		char_puts("tick area : area update\n",ch);
		char_puts("tick char : char update\n",ch);
		char_puts("tick room : room update\n",ch);
		char_puts("tick track: track update\n",ch);
		char_puts("tick obj  : obj update\n",ch);
		return;
	}
	if (is_name(arg, "area"))  {
		area_update();
		char_puts("Area updated.\n", ch);
		return;
	}
	if (is_name(arg, "char player"))  {
		char_update();
		char_puts("Players updated.\n", ch);
		return;
	}
	if (is_name(arg, "room"))  {
		room_update();
		char_puts("Room updated.\n", ch);
		return;
	}
	if (is_name(arg, "track"))  {
		track_update();
		char_puts("Tracks updated.\n", ch);
		return;
	}
	if (is_name(arg, "obj"))  {
		obj_update();
		char_puts("Objects updated.\n", ch);
		return;
	}
	do_tick(ch,str_empty);
	return;
}

/* equips a character */
void do_outfit(CHAR_DATA *ch, const char *argument)
{
	OBJ_DATA *obj;
	class_t *cl = class_lookup(ch->class);
	int sn,vnum;

	if ((ch->level > 5 && !IS_IMMORTAL(ch))
	||  IS_NPC(ch) || cl == NULL) {
		char_puts("Find it yourself!\n",ch);
		return;
	}

	if ((obj = get_eq_char(ch, WEAR_LIGHT)) == NULL)
	{
	    obj = create_obj(get_obj_index(OBJ_VNUM_SCHOOL_BANNER), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_LIGHT);
	}
	
	if ((obj = get_eq_char(ch, WEAR_BODY)) == NULL)
	{
		obj = create_obj(get_obj_index(OBJ_VNUM_SCHOOL_VEST), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_BODY);
	}

	/* do the weapon thing */
	if ((obj = get_eq_char(ch,WEAR_WIELD)) == NULL
	&& (obj = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL) {
		sn = 0; 
		vnum = cl->weapon;
		obj = create_obj(get_obj_index(vnum),0);
		obj->cost = 0;
		obj->condition = 100;
	 	obj_to_char(obj,ch);
		equip_char(ch,obj,WEAR_WIELD);
	}

	if (((obj = get_eq_char(ch,WEAR_WIELD)) == NULL 
	||   !IS_WEAPON_STAT(obj,WEAPON_TWO_HANDS)) 
	&&  (obj = get_eq_char(ch, WEAR_SHIELD)) == NULL
	&& (obj = get_eq_char(ch,WEAR_SECOND_WIELD)) == NULL)
	{
	    obj = create_obj(get_obj_index(OBJ_VNUM_SCHOOL_SHIELD), 0);
		obj->cost = 0;
		obj->condition = 100;
	    obj_to_char(obj, ch);
	    equip_char(ch, obj, WEAR_SHIELD);
	}

	char_puts("You have been equipped by gods.\n",ch);
}

/* RT nochannels command, for those spammers */
void do_nochannels(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	
	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
	    char_puts("Nochannel whom?", ch);
	    return;
	}
	
	if ((victim = get_char_world(ch, arg)) == NULL) {
	    char_puts("They aren't here.\n", ch);
	    return;
	}
	
	if (!IS_NPC(victim) && victim->level >= ch->level) {
	    char_puts("You failed.\n", ch);
	    return;
	}
	
	if (IS_SET(victim->comm, COMM_NOCHANNELS)) {
	    REMOVE_BIT(victim->comm, COMM_NOCHANNELS);
	    char_puts("The gods have restored your channel priviliges.\n", 
			      victim);
	    char_puts("NOCHANNELS removed.\n", ch);
		wiznet("$N restores channels to $i",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
	    SET_BIT(victim->comm, COMM_NOCHANNELS);
	    char_puts("The gods have revoked your channel priviliges.\n", 
			       victim);
	    char_puts("NOCHANNELS set.\n", ch);
		wiznet("$N revokes $i's channels.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

/*
void do_smote(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *vch;
	const char *letter, *name;
	char last[MAX_INPUT_LENGTH], temp[MAX_STRING_LENGTH];
	int matches = 0;

	if (!IS_NPC(ch) && IS_SET(ch->comm, COMM_NOEMOTE)) {
	    char_puts("You can't show your emotions.\n", ch);
	    return;
	}

	if (argument[0] == '\0') {
	    char_puts("Emote what?\n", ch);
	    return;
	}

	if (strstr(argument,ch->name) == NULL) {
		char_puts("You must include your name in an smote.\n",ch);
		return;
	}

	char_printf(ch, "%s\n", argument);

	for (vch = ch->in_room->people; vch != NULL; vch = vch->next_in_room) {
		if (vch->desc == NULL || vch == ch)
			continue;

		if ((letter = strstr(argument,vch->name)) == NULL) {
			char_printf(ch, "%s\n", argument);
			continue;
		}

		strnzcpy(temp, sizeof(temp), argument);
		temp[strlen(argument) - strlen(letter)] = '\0';
		last[0] = '\0';
		name = vch->name;

		for (; *letter != '\0'; letter++) {
			if (*letter == '\'' && matches == strlen(vch->name)) {
				strnzcat(temp, sizeof(temp), "r");
				continue;
			}

			if (*letter == 's' && matches == strlen(vch->name)) {
				matches = 0;
				continue;
			}

			if (matches == strlen(vch->name))
				matches = 0;

			if (*letter == *name) {
				matches++;
				name++;
				if (matches == strlen(vch->name)) {
					strnzcat(temp, sizeof(temp), "you");
					last[0] = '\0';
					name = vch->name;
					continue;
				}
				strnzncat(last, sizeof(last), letter, 1);
				continue;
			}

			matches = 0;
			strnzcat(temp, sizeof(temp), last);
			strnzncat(temp, sizeof(temp), letter, 1);
			last[0] = '\0';
			name = vch->name;
		}
		char_printf(ch, "%s\n", temp);
	}
}	
*/

void do_bamfin(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch)) {
		if (argument[0] == '\0') {
			char_printf(ch, "Your poofin is '%s'\n",
				    ch->pcdata->bamfin);
			return;
		}

		if (strstr(argument, ch->name) == NULL) {
			char_puts("You must include your name.\n",ch);
			return;
		}
		     
		free_string(ch->pcdata->bamfin);
		ch->pcdata->bamfin = str_dup(argument);

		char_printf(ch, "Your poofin is now '%s'\n",
			    ch->pcdata->bamfin);
	}
}

void do_bamfout(CHAR_DATA *ch, const char *argument)
{
	if (!IS_NPC(ch)) {
		if (argument[0] == '\0') {
			char_printf(ch, "Your poofout is '%s'\n",
				    ch->pcdata->bamfout);
			return;
		}
	
		if (strstr(argument,ch->name) == NULL) {
			char_puts("You must include your name.\n", ch);
			return;
		}
	
		free_string(ch->pcdata->bamfout);
		ch->pcdata->bamfout = str_dup(argument);
	
		char_printf(ch, "Your poofout is now '%s'\n",
			    ch->pcdata->bamfout);
	}
}

void do_disconnect(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA_MUDDY *d;
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Disconnect whom?\n", ch);
		return;
	}

	if (is_number(arg)) {
		int desc;

		desc = atoi(arg);
		for (d = descriptor_list_muddy; d; d = d->next)
		{
		        if (/*d->descriptor*/
			get_idsock_connect(d->client) == desc) {
		        	close_descriptor_muddy(d);
	        		char_puts("Ok.\n", ch);
	        		return;
	        	}
		}
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->desc == NULL) {
		act("$N doesn't have a descriptor.", ch, NULL, victim, TO_CHAR);
		return;
	}

	for (d = descriptor_list_muddy; d; d = d->next)
		if (d == victim->desc) {
			close_descriptor_muddy(d);
			char_puts("Ok.\n", ch);
			return;
		}

	bug("Do_disconnect: desc not found.", 0);
	char_puts("Descriptor not found!\n", ch);
	return;
}

void do_echo(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA_MUDDY *d;
	
	if (argument[0] == '\0') {
		char_puts("Global echo what?\n", ch);
		return;
	}
	
	for (d = descriptor_list_muddy; d; d = d->next)
		if (d->connected == CON_PLAYING) {
			if (d->character->level >= ch->level)
				char_puts("global> ", d->character);
			char_printf(d->character, "%s\n", argument);
		}
}

void do_recho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA_MUDDY *d;
	
	if (argument[0] == '\0') {
		char_puts("Local echo what?\n", ch);
		return;
	}

	for (d = descriptor_list_muddy; d; d = d->next)
		if (d->connected == CON_PLAYING
		&&   d->character->in_room == ch->in_room) {
			if (d->character->level >= ch->level)
				char_puts("local> ",d->character);
			char_printf(d->character, "%s\n", argument);
		}
}

void do_zecho(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA_MUDDY *d;

	if (argument[0] == '\0') {
		char_puts("Zone echo what?\n",ch);
		return;
	}

	for (d = descriptor_list_muddy; d; d = d->next)
		if (d->connected == CON_PLAYING
		&&  d->character->in_room != NULL && ch->in_room != NULL
		&&  d->character->in_room->area == ch->in_room->area) {
			if (d->character->level >= ch->level)
				char_puts("zone> ", d->character);
			char_printf(d->character, "%s\n", argument);
		}
}

void do_pecho(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));
	
	if (argument[0] == '\0' || arg[0] == '\0') {
		char_puts("Personal echo what?\n", ch); 
		return;
	}
	 
	if  ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->level >= ch->level && ch->level != MAX_LEVEL)
		char_puts("personal> ", victim);

	char_printf(victim, "%s\n", argument);
	char_printf(ch, "personal> %s\n", argument);
}

ROOM_INDEX_DATA *find_location(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	if (is_number(argument))
		return get_room_index(atoi(argument));

	if ((victim = get_char_world(ch, argument)) != NULL)
		return victim->in_room;

	if ((obj = get_obj_world(ch, argument)) != NULL)
		return obj->in_room;

	return NULL;
}

void do_transfer(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	CHAR_DATA *victim;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0') {
		char_puts("Transfer whom (and where)?\n", ch);
		return;
	}

	if (!str_cmp(arg1, "all")) {
		DESCRIPTOR_DATA_MUDDY *d;

		for (d = descriptor_list_muddy; d; d = d->next)
		    if (d->connected == CON_PLAYING
		    &&   d->character != ch
		    &&   d->character->in_room != NULL
		    &&   can_see(ch, d->character))
			doprintf(do_transfer, ch,
				"%s %s", d->character->name, arg2);
		return;
	}

	/*
	 * Thanks to Grodyn for the optional location parameter.
	 */
	if (arg2[0]) {
		if ((location = find_location(ch, arg2)) == NULL) {
		    char_puts("No such location.\n", ch);
		    return;
		}

		if (room_is_private(location) && ch->level < MAX_LEVEL) {
		    char_puts("That room is private right now.\n", ch);
		    return;
		}
	} else
		location = ch->in_room;

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->in_room == NULL) {
		char_puts("They are in limbo.\n", ch);
		return;
	}

	if (victim->fighting != NULL)
		stop_fighting(victim, TRUE);
	act("$n disappears in a mushroom cloud.", victim, NULL, NULL, TO_ROOM);
	char_from_room(victim);

	act("$N arrives from a puff of smoke.",
	    location->people, NULL, victim, TO_ALL);
	if (ch != victim)
		act("$n has transferred you.", ch, NULL, victim, TO_VICT);
	char_puts("Ok.\n", ch);

	char_to_room(victim, location);
	if (JUST_KILLED(victim))
		return;

	do_look(victim, "auto");
}

void do_at(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_INDEX_DATA *original;
	OBJ_DATA *on;
	
	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("At where what?\n", ch);
		return;
	}

	if ((location = find_location(ch, arg)) == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	if (room_is_private(location) 	
	&&  ch->level < MAX_LEVEL) {
		char_puts("That room is private right now.\n", ch);
		return;
	}

	original = ch->in_room;
	on = ch->on;
	char_from_room(ch);

	char_to_room(ch, location);
	if (JUST_KILLED(ch))
		return;

	interpret(ch, argument);

	/* handle 'at xxx quit' */
	if (JUST_KILLED(ch) || IS_EXTRACTED(ch))
		return;

	char_from_room(ch);
	char_to_room(ch, original);
	if (JUST_KILLED(ch))
		return;
	ch->on = on;
}

void do_goto(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	CHAR_DATA *pet = NULL;

	if (argument[0] == '\0') {
		char_puts("Goto where?\n", ch);
		return;
	}

	if ((location = find_location(ch, argument)) == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	if (ch->level < LEVEL_IMMORTAL) {
		if (ch->fighting) {
			char_puts("No way! You are fighting.\n", ch);
			return;
		}

		if (IS_PUMPED(ch)) {
			act_puts("You are too pumped to goto now.",
				 ch, NULL, NULL, TO_CHAR, POS_DEAD);
			return;
		}
#if 0
		if (!IS_SET(ch->in_room->room_flags, ROOM_PEACE)) {
			char_puts("You must be in a safe place in order "
				  "to make a transportation.\n", ch);
			return;
		}

		if (!IS_BUILDER(ch, location->area)
		||  !IS_BUILDER(ch, ch->in_room->area)) {
			char_puts("You cannot transfer yourself there.\n", ch);
			return;
		}
#endif
	}

	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);

	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_TRUSTED(rch, ch->pcdata->invis_level)) {
			if (ch->pcdata != NULL
			&&  ch->pcdata->bamfout[0] != '\0')
				act("$t", ch, ch->pcdata->bamfout,
				    rch, TO_VICT);
			else
				act("$n leaves in a swirling mist.", ch, NULL,
				    rch, TO_VICT);
		}

	if (ch->pet && ch->in_room == ch->pet->in_room)
		pet = ch->pet;

	char_from_room(ch);

	for (rch = location->people; rch; rch = rch->next_in_room)
		if (IS_TRUSTED(rch, ch->pcdata->invis_level)) {
			if (ch->pcdata
			&&  ch->pcdata->bamfin[0])
				act("$t",
				    rch, ch->pcdata->bamfin, NULL, TO_CHAR);
			else
				act("$N appears in a swirling mist.",
				    rch, NULL, ch, TO_CHAR);
		}

	char_to_room(ch, location);
	if (JUST_KILLED(ch))
		return;
	do_look(ch, "auto");

	if (pet && !IS_AFFECTED(pet, AFF_SLEEP)) {
		if (ch->pet->position != POS_STANDING)
			do_stand(pet, str_empty);
		char_from_room(pet);
		char_to_room(pet, location);
	}
}

void do_violate(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *location;
	CHAR_DATA *rch;
	
	if (argument[0] == '\0') {
	    char_puts("Goto where?\n", ch);
	    return;
	}
	
	if ((location = find_location(ch, argument)) == NULL) {
	    char_puts("No such location.\n", ch);
	    return;
	}

	if (!room_is_private(location)) {
	    char_puts("That room isn't private, use goto.\n", ch);
	    return;
	}
	
	if (ch->fighting != NULL)
		stop_fighting(ch, TRUE);
	
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_TRUSTED(rch, ch->pcdata->invis_level)) {
			if (ch->pcdata != NULL
			&&  ch->pcdata->bamfout[0] != '\0')
				act("$t", ch, ch->pcdata->bamfout,
				    rch, TO_VICT);
			else
				act("$n leaves in a swirling mist.", ch, NULL,
				    rch, TO_VICT);
		}
	
	char_from_room(ch);
	
	for (rch = ch->in_room->people; rch != NULL; rch = rch->next_in_room)
		if (IS_TRUSTED(rch, ch->pcdata->invis_level)) {
			if (ch->pcdata && ch->pcdata->bamfin[0] != '\0')
				act("$t",
				    rch, ch->pcdata->bamfin, NULL, TO_CHAR);
			else
				act("$N appears in a swirling mist.",
				    rch, NULL, ch, TO_CHAR);
		}
	
	char_to_room(ch, location);
	if (JUST_KILLED(ch))
		return;
	do_look(ch, "auto");
}

/* RT to replace the 3 stat commands */

void do_stat(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	ROOM_INDEX_DATA *location;
	CHAR_DATA *victim;
	const char *string;

	string = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Syntax:\n", ch);
		char_puts("  stat <name>\n", ch);
		char_puts("  stat obj <name>\n", ch);
		char_puts("  stat mob <name>\n", ch);
		char_puts("  stat room <number>\n", ch);
		char_puts("  stat desc <number>\n", ch);
		return;
	}

	if (!str_cmp(arg, "room")) {
		do_rstat(ch, string);
		return;
	}
	
	if (!str_cmp(arg, "obj")) {
		do_ostat(ch, string);
		return;
	}

	if (!str_cmp(arg, "char") || !str_cmp(arg, "mob")) {
		do_mstat(ch, string);
		return;
	}
	 
	if (!str_cmp(arg, "desc")) {
		do_dstat(ch, string);
		return;
	}

	/* do it the old way */

	obj = get_obj_world(ch, argument);
	if (obj != NULL) {
		do_ostat(ch, argument);
	 	return;
	}

	victim = get_char_world(ch, argument);
	if (victim != NULL) {
		do_mstat(ch, argument);
		return;
	}

	location = find_location(ch, argument);
	if (location != NULL) {
		do_rstat(ch, argument);
		return;
	}

	char_puts("Nothing by that name found anywhere.\n",ch);
}

void do_rstat(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	ROOM_HISTORY_DATA *rh;
	OBJ_DATA *obj;
	CHAR_DATA *rch;
	BUFFER *output;
	int door;

	one_argument(argument, arg, sizeof(arg));
	location = (arg[0] == '\0') ? ch->in_room : find_location(ch, arg);
	if (location == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	if (ch->in_room != location 
	&&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR)) {
		char_puts("That room is private right now.\n", ch);
		return;
	}

	output = buf_new(-1);

	if (ch->in_room->affected_by)
		buf_printf(output, "Affected by %s\n", 
			   flag_string(raffect_flags, ch->in_room->affected_by));

	if (ch->in_room->room_flags)
		buf_printf(output, "Room Flags %s\n", 
			   flag_string(room_flags, ch->in_room->room_flags));

	mlstr_dump(output, "Name: ", location->name);
	buf_printf(output, "Area: '%s'\nOwner: '%s'\n",
		   location->area->name,
		   location->owner);

	buf_printf(output,
		   "Vnum: %d  Sector: %d  Light: %d  Healing: %d  Mana: %d\n",
		   location->vnum,
		   location->sector_type,
		   location->light,
		   location->heal_rate,
		   location->mana_rate);

	buf_printf(output, "Room flags: [%s].\n",
		   flag_string(room_flags, location->room_flags));
	buf_add(output, "Description:\n");
	mlstr_dump(output, str_empty, location->description);

	if (location->ed != NULL) {
		ED_DATA *ed;

		buf_add(output, "Extra description keywords: '");
		for (ed = location->ed; ed; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next != NULL)
				buf_add(output, " ");
		}
		buf_add(output, "'.\n");
	}

	buf_add(output, "Characters:");
	for (rch = location->people; rch; rch = rch->next_in_room) {
		if (can_see(ch,rch)) {
		    buf_add(output, " ");
		    one_argument(rch->name, buf, sizeof(buf));
		    buf_add(output, buf);
		}
	}

	buf_add(output, ".\nObjects:   ");
	for (obj = location->contents; obj; obj = obj->next_content) {
		buf_add(output, " ");
		one_argument(obj->name, buf, sizeof(buf));
		buf_add(output, buf);
	}
	buf_add(output, ".\n");

	for (door = 0; door <= 5; door++) {
		EXIT_DATA *pexit;

		if ((pexit = location->exit[door]) != NULL) {
			buf_printf(output, "Door: %d.  To: %d.  Key: %d.  Exit flags: %d.\nKeyword: '%s'.\n",
				door,
				pexit->to_room.r == NULL ?
				-1 : pexit->to_room.r->vnum,
		    		pexit->key,
		    		pexit->exit_info,
		    		pexit->keyword);
			mlstr_dump(output, "Description: ",
				     pexit->description);
		}
	}
	buf_add(output, "Tracks:\n");
	for (rh = location->history;rh != NULL;rh = rh->next)
		buf_printf(output,"%s took door %i.\n", rh->name, rh->went);

	send_to_char(buf_string(output), ch);
	buf_free(output);
}

char *obj_name(OBJ_DATA *obj, CHAR_DATA *ch);

void do_ostat(CHAR_DATA *ch, const char *argument)
{
	int i;
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH];
	OBJ_DATA *obj;
	struct attack_type *atd;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Stat what?\n", ch);
		return;
	}

	if ((obj = get_obj_world(ch, argument)) == NULL) {
		char_puts("Nothing like that in hell, heaven or earth.\n",ch);
		return;
	}

	output = buf_new(-1);
	buf_printf(output, "Name(s): %s\n", obj->name);
	if (!mlstr_null(obj->owner))
		buf_printf(output, "Owner: [%s]\n", mlstr_mval(obj->owner));
	buf_printf(output, "Vnum: %d  Type: %s  Resets: %d\n",
		obj->pIndexData->vnum,
		flag_string(item_types, obj->pIndexData->item_type),
		obj->pIndexData->reset_num);

	mlstr_dump(output, "Short description: ", obj->short_descr);
	mlstr_dump(output, "Long description: ", obj->description);

	buf_printf(output, "Wear bits: %s\n",
		   flag_string(wear_flags, obj->wear_flags));
	buf_printf(output, "Extra bits: %s\n",
		   flag_string(extra_flags, obj->extra_flags));
	buf_printf(output, "Number: %d/%d  Weight: %d/%d/%d (10th pounds)\n",
		1,           get_obj_number(obj),
		obj->weight, get_obj_weight(obj),get_true_weight(obj));

	buf_printf(output,
		  "Level: %d  Cost: %d  Condition: %d  Timer: %d Count: %d\n",
		  obj->level, obj->cost, obj->condition,
		  obj->timer, obj->pIndexData->count);

	buf_printf(output,
		"In room: %d  In object: %s  Carried by: %s  Wear_loc: %d\n",
		obj->in_room    == NULL    ?        0 : obj->in_room->vnum,
		obj->in_obj     == NULL    ? "(none)" : mlstr_mval(obj->in_obj->short_descr),
		obj->carried_by == NULL    ? "(none)" : 
		    can_see(ch,obj->carried_by) ? obj->carried_by->name
					 	: "someone",
		obj->wear_loc);

	if (obj->on || obj->under)
		buf_printf(output,
		"On obj: %s [%d]   Under obj: %s [%d]\n",
		obj->on ? obj_name(obj->on, ch) : str_empty,
		obj->on ? obj->on->pIndexData->vnum : -1,
		obj->under ? obj_name(obj->under, ch) : str_empty,
		obj->under ? obj->under->pIndexData->vnum : -1);

 	buf_printf(output,
		"Next: %s   Next_content: %s\n",
		obj->next	== NULL	   ? "(none)" : mlstr_mval(obj->next->short_descr),
		obj->next_content == NULL  ? "(none)" : mlstr_mval(obj->next_content->short_descr));
	buf_printf(output, "Values: %d %d %d %d %d\n",
		obj->value[0], obj->value[1], obj->value[2], obj->value[3],
		obj->value[4]);
	
	/* now give out vital statistics as per identify */
	
	switch (obj->pIndexData->item_type) {
		int i;
	case ITEM_SCROLL: 
	case ITEM_POTION:
	case ITEM_PILL:
		buf_printf(output, "Level %d spells of:", obj->value[0]);

		for (i = 1; i < 5; i++)
			if (obj->value[i] >= 0) 
				buf_printf(output, " '%s'",
					   skill_name(obj->value[i]));
		buf_add(output, ".\n");
		break;

	case ITEM_WAND: 
	case ITEM_STAFF: 
		buf_printf(output, "Has %d(%d) charges of level %d",
			   obj->value[1], obj->value[2], obj->value[0]);
	  
		if (obj->value[3] >= 0) 
			buf_printf(output, " '%s'",
				   skill_name(obj->value[3]));
		buf_add(output, ".\n");
		break;

	case ITEM_DRINK_CON:
		buf_printf(output, "It holds %s-colored %s.\n",
			   liq_table[obj->value[2]].liq_color,
			   liq_table[obj->value[2]].liq_name);
		break;
	  
	case ITEM_WEAPON:
		buf_printf(output, "%s\n",
			   flag_string(weapon_class, obj->value[0]));
		buf_printf(output,"Damage is %dd%d (average %d)\n",
			   obj->value[1],obj->value[2],
			   (1 + obj->value[2]) * obj->value[1] / 2);
		buf_printf(output, "Damage noun is %s.\n",
			   (atd = varr_get(&damages_data.attack_table, obj->value[3])) == NULL ? "none" : mlstr_mval(atd->noun));
		    
		if (obj->value[4])  /* weapon flags */
		        buf_printf(output,"Weapons flags: %s\n",
				   flag_string(weapon_type2, obj->value[4]));
		break;

	case ITEM_ARMOR:
		buf_printf(output, 
		    "Armor class is %d pierce, %d bash, %d slash, and %d vs. magic\n",
		        obj->value[0], obj->value[1], obj->value[2], obj->value[3]);
		break;

	case ITEM_CONTAINER:
	        buf_printf(output,"Capacity: %d#  Maximum weight: %d#  flags: %s\n",
	        	   obj->value[0], obj->value[3],
			   flag_string(cont_flags, obj->value[1]));
	        if (obj->value[4] != 100)
	        	buf_printf(output,"Weight multiplier: %d%%\n",
				   obj->value[4]);
		break;
	}

	if (obj->ed) {
		ED_DATA *ed;

		buf_add(output, "Extra description keywords: '");

		for (ed = obj->ed; ed; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next)
				buf_add(output, " ");
		}

		buf_add(output, "'\n");
	}

	if (obj->pIndexData->ed) {
		ED_DATA *ed;

		buf_add(output, "pIndexData extra description keywords: '");

		for (ed = obj->pIndexData->ed; ed; ed = ed->next) {
			buf_add(output, ed->keyword);
			if (ed->next)
				buf_add(output, " ");
		}

		buf_add(output, "'\n");
	}

	format_obj_affects(output, obj->affected, 0);
	if (!IS_SET(obj->hidden_flags, OHIDE_ENCHANTED)
	&& obj->pIndexData->affected) {
		buf_add(output, "pIndexAffect(s):\n");
		format_obj_affects(output, obj->pIndexData->affected,
				   FOA_F_NODURATION);
	}

	if (obj->pIndexData->oprogs) {
		buf_add(output, "Object progs:\n");
		for (i = 0; i < OPROG_MAX; i++)
			if (obj->pIndexData->oprogs[i] != NULL)
				buf_printf(output, "%s: %s\n",
					 optype_table[i],
					 oprog_name_lookup(obj->pIndexData->oprogs[i]));
	}
	buf_printf(output,"Damage condition : %d (%s)\n", obj->condition,
				get_cond_alias(obj));

	send_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_mstat(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	AFFECT_DATA *paf;
	CHAR_DATA *victim;
	BUFFER *output;
	struct attack_type *atd;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Stat whom?\n", ch);
		return;
	}
	
	if ((victim = get_char_room(ch, argument)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	output = buf_new(-1);

	buf_printf(output, "Name: [%s] Reset Zone: %s\n", victim->name,
		(IS_NPC(victim) &&victim->zone) ? victim->zone->name : "?");

	buf_printf(output, 
		"Vnum: %d  Race: %s (%s)  Group: %d  Sex: %s  Room: %d\n",
		IS_NPC(victim) ? victim->pIndexData->vnum : 0,
		race_name(victim->race), race_name(ORG_RACE(victim)),
		IS_NPC(victim) ? victim->pIndexData->group : 0, flag_string(sex_table,victim->sex),
		victim->in_room == NULL    ?        0 : victim->in_room->vnum);

	if (IS_NPC(victim))
		buf_printf(output,"Count: %d  Killed: %d  Religion: %s\n",
			victim->pIndexData->count, victim->pIndexData->killed,
			GET_CHAR_RELIGION(ch) ? GET_CHAR_RELIGION(ch)->name : "none");

	buf_printf(output, "Str: %d(%d)  Int: %d(%d)  Wis: %d(%d)  Dex: %d(%d)  Con: %d(%d) Cha: %d(%d)\n",
		victim->perm_stat[STAT_STR],
		get_curr_stat(victim,STAT_STR),
		victim->perm_stat[STAT_INT],
		get_curr_stat(victim,STAT_INT),
		victim->perm_stat[STAT_WIS],
		get_curr_stat(victim,STAT_WIS),
		victim->perm_stat[STAT_DEX],
		get_curr_stat(victim,STAT_DEX),
		victim->perm_stat[STAT_CON],
		get_curr_stat(victim,STAT_CON),
		victim->perm_stat[STAT_CHA],
		get_curr_stat(victim,STAT_CHA));

	buf_printf(output, "Hp: %d/%d  Mana: %d/%d  Move: %d/%d  Practices: %d  Trains: %d\n",
		victim->hit,         victim->max_hit,
		victim->mana,        victim->max_mana,
		victim->move,        victim->max_move,
		IS_NPC(victim) ? 0 : victim->pcdata->practice,
		IS_NPC(victim) ? 0 : victim->pcdata->train);
		
/*	buf_printf(output,"It belives the religion of %s.\n",
		(IS_NPC(victim) || !victim->pcdata->religion) ?
			str_empty : victim->pcdata->religion->data->name);
 */
	
	if (IS_NPC(victim))
		snprintf(buf, sizeof(buf), "%d", victim->alignment);
	else  {
		snprintf(buf, sizeof(buf), "%s-%s(%d)", 
			 flag_string(ethos_table, victim->pcdata->ethos),
			 flag_string(align_names, NALIGN(victim)),
			 victim->alignment);
	}

	buf_printf(output,
		"Lv: %d + %d  Class: %s  Align: %s Exp: %d\n",
		victim->level,
		victim->drain_level,
		class_name(victim),
		buf, IS_NPC(victim) ? 0 : victim->pcdata->exp);
	buf_printf(output,
		"Slevel: %d  Gold: %ld  Silver: %ld  BG: %ld BS: %ld\n",
			IS_NPC(victim) ? 0 : victim->pcdata->add_spell_level,
			victim->gold, victim->silver,
			IS_NPC(victim) ? 0 : victim->pcdata->bank_g,
			IS_NPC(victim) ? 0 : victim->pcdata->bank_s);

	buf_printf(output,"Armor: pierce: %d  bash: %d  slash: %d  magic: %d\n",
		    GET_AC(victim,AC_PIERCE), GET_AC(victim,AC_BASH),
		    GET_AC(victim,AC_SLASH),  GET_AC(victim,AC_EXOTIC));

	buf_printf(output, 
		"Hit: %d  Dam: %d  Saves: %d  Size: %s  Position: %s  Wimpy: %d\n",
		GET_HITROLL(victim), GET_DAMROLL(victim), victim->saving_throws[0],
		flag_string(size_table, victim->size),
		flag_string(position_table, victim->position),
		GET_WIMPY(victim));

	if (IS_NPC(victim))
		buf_printf(output, "Damage: %dd%d  Message:  %s\n",
			   victim->damage[DICE_NUMBER],
			   victim->damage[DICE_TYPE],
			   (atd = varr_get(&damages_data.attack_table, victim->dam_type)) == NULL ?
			   "none" : mlstr_mval(atd->noun));

	buf_printf(output, "Fighting: %s Deaths: %d Carry number: %d  Carry weight: %ld\n",
		   victim->fighting ? victim->fighting->name : "(none)" ,
		   IS_NPC(victim) ? 0 : victim->pcdata->death,
		   victim->carry_number, get_carry_weight(victim) / 10);

	if (!IS_NPC(victim)) {
		buf_printf(output,
			   "Thirst: %d  Hunger: %d  Full: %d  "
			   "Drunk: %d Bloodlust: %d Desire: %d\n",
			   victim->pcdata->condition[COND_THIRST],
			   victim->pcdata->condition[COND_HUNGER],
			   victim->pcdata->condition[COND_FULL],
			   victim->pcdata->condition[COND_DRUNK],
			   victim->pcdata->condition[COND_BLOODLUST],
			   victim->pcdata->condition[COND_DESIRE]);
		buf_printf(output, 
			   "Age: %d  Played: %d  Last Level: %d  Timer: %d\n",
			   get_age(victim), 
			   (int) (victim->pcdata->played + current_time - victim->pcdata->logon) / 3600, 
			   victim->pcdata->last_level, 
			   victim->timer);
	}

	if (!IS_NPC(victim))
		buf_printf(output, "Plr: [%s]\n",
			   flag_string(plr_flags, victim->pcdata->plr_flags));
	
	if (victim->comm)
		buf_printf(output, "Comm: [%s]\n",
			   flag_string(comm_flags, victim->comm));

	if (IS_NPC(victim) && victim->pIndexData->off_flags)
		buf_printf(output, "Offense: [%s]\n",
			   flag_string(off_flags,
				       victim->pIndexData->off_flags));

	if (victim->imm_flags)
		buf_printf(output, "Immune: [%s]\n",
			   flag_string(irv_flags, victim->imm_flags));
	
	if (victim->res_flags)
		buf_printf(output, "Resist: [%s]\n",
			   flag_string(irv_flags, victim->res_flags));

	if (victim->vuln_flags)
		buf_printf(output, "Vulnerable: [%s]\n",
			   flag_string(irv_flags, victim->vuln_flags));

	buf_printf(output, "Form: [%s]\n",
		   flag_string(form_flags, victim->form));
	buf_printf(output, "Parts: [%s]\n",
		   flag_string(part_flags, victim->parts));

	if (IS_PC(victim) && victim->pcdata->add_exp)
	{
		buf_printf(output, "Add_exp: [%d]\n",
				victim->pcdata->add_exp);
	}

	if (victim->affected_by)
		buf_printf(output, "Affected by %s\n", 
			   flag_string(affect_flags, victim->affected_by));

	buf_printf(output, "Master: %s  Leader: %s  Pet: %s\n",
		victim->master      ? victim->master->name   : "(none)",
		victim->leader      ? victim->leader->name   : "(none)",
		victim->pet 	    ? victim->pet->name	     : "(none)");

	/* OLC */
	if (IS_PC(victim))
		buf_printf(output, "Security: %d.\n",
			   victim->pcdata->security);

	mlstr_dump(output, "Short description: ", victim->short_descr);
	if (IS_NPC(victim))
		mlstr_dump(output, "Long description: ", victim->long_descr);

	if (IS_NPC(victim) && victim->spec_fun != 0)
		buf_printf(output, "Mobile has special procedure %s.\n",
			   spec_name(victim->spec_fun));

	for (paf = victim->affected; paf != NULL; paf = paf->next)
	{
		where_t *w = where_lookup(paf->where);
		char buf[MAX_STRING_LENGTH];
		
		snprintf(buf, sizeof(buf),
		    "Spell: '{c%s{x' modifies {c%s{x by {c%d{x for {c%d{x hours with "
		    "%s, level {c%d{x.\n",
			skill_name(paf->type),
		    flag_string(apply_flags, paf->location),
		    paf->modifier,
		    paf->duration,
		    w ? w->format: "none",
		    paf->level
		   );
 		buf_printf(output, buf,
 			w ? flag_string(w->table, paf->bitvector) : str_empty);
	}

	if (IS_PC(victim)) {
//		qtrouble_t *qt;

		if (GET_QUEST_TYPE(victim)) {
			buf_printf(output,
				   "Questgiver: [%d]  QuestPnts: [%d]  Questnext: [%d]\n",
				   victim->pcdata->quest->giver,
				   victim->pcdata->questpoints,
				   victim->pcdata->quest->time < 0 ?
					-victim->pcdata->quest->time : 0);
		 	buf_printf(output,
				   "QuestCntDown: [%d]  QuestObj:  [%d]  Questmob: [%d]  QuestType: %d\n",
				   victim->pcdata->quest->time > 0 ?
					victim->pcdata->quest->time : 0,
				   victim->pcdata->quest->target1,
				   victim->pcdata->quest->target2,
				   GET_QUEST_TYPE(victim));
		}
		else if (victim->pcdata->quest) {
			buf_printf(output,
				   "QuestPnts: [%d]  Questnext: [%d]  NOT QUESTING\n",
				   victim->pcdata->questpoints,
				   victim->pcdata->quest->time < 0 ?
					-victim->pcdata->quest->time : 0);
		}

/*		buf_add(output, "Quest Troubles: ");
		for (qt = victim->pcdata->qtrouble; qt; qt = qt->next)
			buf_printf(output, "[%d]-[%d] ", qt->vnum, qt->count-1);
		buf_add(output, "\n");
 */

		if (!IS_NULLSTR(victim->pcdata->twitlist))
			buf_printf(output, "Twitlist: [%s]\n",
				   victim->pcdata->twitlist);
		if (!IS_NULLSTR(victim->pcdata->granted))
			buf_printf(output, "Granted: [%s]\n",
				   victim->pcdata->granted);
		if (IS_SET(victim->pcdata->otherf, OTHERF_CONFIRM_DESC))
			buf_add(output, "Description confirmed.\n");
		show_last(output, victim);
		if (victim->pcdata->remort)
			buf_printf(output, "Remorted %d times [%d rp].",
				victim->pcdata->remort->remorts,
				victim->pcdata->remort->points);
	}

	buf_printf(output,
		   "Last fought: [%s], Last fight time: [%s]\n",
		   victim->last_fought ? victim->last_fought->name : "none", 
		   strtime(victim->last_fight_time));
	if (IS_PUMPED(victim))
		buf_add(output, "Adrenalin is gushing.\n");
	buf_printf(output, "In_mind: [%s], Target: [%s]\n", 
			victim->in_mind ? victim->in_mind : "none",
			victim->target ? victim->target->name : "none");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

DO_FUN(do_dstat)
{
	BUFFER *output;
	DESCRIPTOR_DATA_MUDDY *d;
	char arg[MAX_INPUT_LENGTH];
	int desc;

	one_argument(argument, arg, sizeof(arg));
	if (!is_number(arg)) {
		do_help(ch, "'WIZ STAT'");
		return;
	}

	desc = atoi(arg);

	for (d = descriptor_list_muddy; d; d = d->next)
		if (get_idsock_connect(d->client) == desc)
			break;
	if (!d || desc < 0) {
		char_puts("dstat: descriptor not found\n", ch);
		return;
	}

	output = buf_new(-1);

	buf_printf(output, "Desc: [%d]  Conn_stat: [%d]  "
			   "Outsize: [%d]  Outtop:  [%d]\n",
		   get_idsock_connect(d->client), d->connected,
		   d->client->connect->outsize, d->client->connect->outtop);
	buf_printf(output, "Inarray: [%s]\n", d->client->inarray);
	buf_printf(output, "Inlength: [%s]\n", d->client->inlength);
//	buf_printf(output, "Repeat: [%d]  Inlast: [%s]\n",
//		   d->repeat, d->inlast);
	if (d->character)
		buf_printf(output, "Ch: [%s]\n", d->character->name);
	if (d->original)
		buf_printf(output, "Original: [%s]\n", d->original->name);
	if (d->olced)
		buf_printf(output, "OlcEd: [%s]\n", d->olced->name);
	if (d->pString)
		buf_printf(output, "pString: [%s]\n", *d->pString);
	if (d->showstr_head)
		buf_printf(output, "showstr_head: [%s]\n", d->showstr_head);

	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_vnum(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *string;

	string = one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  vnum obj <name>\n",ch);
		char_puts("  vnum mob <name>\n",ch);
		return;
	}

	if (!str_cmp(arg, "obj")) {
		do_ofind(ch, string);
		return;
	}

	if (!str_cmp(arg, "mob") || !str_cmp(arg, "char")) { 
		do_mfind(ch, string);
		return;
	}

	/* do both */
	do_mfind(ch, argument);
	do_ofind(ch, argument);
}

void do_mfind(CHAR_DATA *ch, const char *argument)
{
	extern int top_mob_index;
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	int vnum;
	int nMatch;
	bool found;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Find whom?\n", ch);
		return;
	}

	found	= FALSE;
	nMatch	= 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_mob_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_mob_index; vnum++)
		if ((pMobIndex = get_mob_index(vnum)) != NULL) {
		    nMatch++;
		    if (is_name(argument, pMobIndex->name)) {
			found = TRUE;
			char_printf(ch, "[%5d] %s\n", pMobIndex->vnum,
				    mlstr_mval(pMobIndex->short_descr));
		    }
		}

	if (!found)
		char_puts("No mobiles by that name.\n", ch);
}

void do_ofind(CHAR_DATA *ch, const char *argument)
{
	extern int top_obj_index;
	char arg[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	int vnum;
	int nMatch;
	bool found;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Find what?\n", ch);
		return;
	}

	found	= FALSE;
	nMatch	= 0;

	/*
	 * Yeah, so iterating over all vnum's takes 10,000 loops.
	 * Get_obj_index is fast, and I don't feel like threading another link.
	 * Do you?
	 * -- Furey
	 */
	for (vnum = 0; nMatch < top_obj_index; vnum++)
		if ((pObjIndex = get_obj_index(vnum)) != NULL) {
		    nMatch++;
		    if (is_name(argument, pObjIndex->name)) {
			found = TRUE;
			char_printf(ch, "[%5d] %s\n", pObjIndex->vnum,
				    mlstr_mval(pObjIndex->short_descr));
		    }
		}

	if (!found)
		char_puts("No objects by that name.\n", ch);
}

void do_owhere(CHAR_DATA *ch, const char *argument)
{
	BUFFER *buffer = NULL;
	OBJ_DATA *obj;
	OBJ_DATA *in_obj;
	int number = 0, max_found = 200;
	int vnum = 0;

	if (argument[0] == '\0') {
		char_puts("Find what?\n",ch);
		return;
	}
	
	if (is_number(argument))
		vnum = atoi(argument);
	
	for (obj = object_list; obj != NULL; obj = obj->next) {
		if (!can_see_obj(ch, obj) || (!vnum && !is_name(argument, obj->name))
		|| (vnum && vnum != obj->pIndexData->vnum) || (ch->level < obj->level && !IS_IMMORTAL(ch)))
	        	continue;
	
		if (buffer == NULL)
			buffer = buf_new(-1);
		number++;
	
		for (in_obj = obj; in_obj->in_obj != NULL;
		     in_obj = in_obj->in_obj)
	        	;
	
		if (in_obj->carried_by != NULL
		&&  can_see(ch,in_obj->carried_by)
		&&  in_obj->carried_by->in_room != NULL)
			buf_printf(buffer,
				   "%3d) %s is carried by %s [Room %d]\n",
				number,
				mlstr_mval(obj->short_descr),
				fix_short(PERS(in_obj->carried_by, ch)),
				in_obj->carried_by->in_room->vnum);
		else if (in_obj->in_room != NULL
		     &&  can_see_room(ch, in_obj->in_room))
	        	buf_printf(buffer, "%3d) %s is in %s [Room %d]\n",
	        		number, mlstr_mval(obj->short_descr),
				mlstr_cval(in_obj->in_room->name, ch), 
				in_obj->in_room->vnum);
		else
			buf_printf(buffer, "%3d) %s is somewhere\n",number,
				mlstr_mval(obj->short_descr));
	
	    if (number >= max_found)
	        break;
	}
	
	if (buffer == NULL)
		char_puts("Nothing like that in heaven or earth.\n", ch);
	else {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
}

void do_mwhere(CHAR_DATA *ch, const char *argument)
{
	BUFFER *buffer;
	CHAR_DATA *victim;
	int count = 0, vnum = 0;

	if (argument[0] == '\0') {
		DESCRIPTOR_DATA_MUDDY *d;

		/* show characters logged */

		buffer = buf_new(-1);
		for (d = descriptor_list_muddy; d; d = d->next)
		{
		    if (d->character && d->connected == CON_PLAYING
		    &&  d->character->in_room != NULL && can_see(ch,d->character)
		    &&  can_see_room(ch,d->character->in_room))
		    {
			victim = d->character;
			count++;
			if (d->original != NULL)
			    buf_printf(buffer,"%3d) %s (in the body of %s) is in %s [%d]\n",
				count, d->original->name,
				mlstr_mval(victim->short_descr),
				mlstr_mval(victim->in_room->name),
				victim->in_room->vnum);
			else
			    buf_printf(buffer,"%3d) %s is in %s [%d]\n",
				count, victim->name,
				mlstr_mval(victim->in_room->name),
				victim->in_room->vnum);
		    }
		}

	    page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
		return;
	}

	buffer = NULL;
	if (is_number(argument))
		vnum = atoi(argument);
	
	for (victim = char_list; victim; victim = victim->next)
		if (victim->in_room
		&& ((!vnum && is_name(argument, victim->name))
		    || (vnum && IS_NPC(victim) && vnum == victim->pIndexData->vnum))) {
			if (buffer == NULL)
				buffer = buf_new(-1);

			count++;
			buf_printf(buffer, "%3d) [%5d] %-28s [%5d] %s\n",
			  count, IS_NPC(victim) ? victim->pIndexData->vnum : 0,
			  IS_NPC(victim) ?
				 mlstr_mval(victim->short_descr) : victim->name,
			  victim->in_room->vnum,
			  mlstr_mval(victim->in_room->name));
		}

	if (buffer) {
		page_to_char(buf_string(buffer),ch);
		buf_free(buffer);
	}
	else
		act("You didn't find any $T.", ch, NULL, argument, TO_CHAR);
}

void do_reboo(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to REBOOT, spell it out.\n", ch);
}

void do_shutdow(CHAR_DATA *ch, const char *argument)
{
	char_puts("If you want to SHUTDOWN, spell it out.\n", ch);
}

void do_shutdown(CHAR_DATA *ch, const char *argument)
{
	bool active;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "SHUTDOWN");
		char_printf(ch, "Syntax: shutdown <activate | deactivate | status>");
		return;
	}

	active = dfexist(TMP_PATH, SHUTDOWN_FILE);
		
	if (!str_prefix(arg, "status")) {
		char_printf(ch, "Shutdown status: %s\n",
			    active ? "active" : "inactive");
		return;
	}

	if (!str_prefix(arg, "activate")) {
		if (!active) {
			FILE *fp = dfopen(TMP_PATH, SHUTDOWN_FILE, "w");
			if (!fp) {
				char_printf(ch, "Error: %s.\n",
					    strerror(errno));
				return;
			}
			fclose(fp);
			wiznet("$N has activated shutdown", ch, NULL, 0, 0, 0);
			char_puts("Shutdown activated.\n", ch);
		}
		else
			char_puts("Shutdown already activated.\n", ch);
		return;
	}

	if (!str_prefix(arg, "deactivate") || !str_prefix(arg, "cancel")) {
		if (!active)
			char_puts("Shutdown already inactive.\n", ch);
		else {
			if (dunlink(TMP_PATH, SHUTDOWN_FILE) < 0) {
				char_printf(ch, "Error: %s.\n",
					    strerror(errno));
				return;
			}
			wiznet("$N has deactivated shutdown",
				ch, NULL, 0, 0, 0);
			char_puts("Shutdown deactivated.\n", ch);
		}
		return;
	}

	do_shutdown(ch, str_empty);
}

void do_protect(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
		char_puts("Protect whom from snooping?\n",ch);
		return;
	}

	if ((victim = get_char_world(ch,argument)) == NULL) {
		char_puts("You can't find them.\n",ch);
		return;
	}

	if (IS_SET(victim->comm,COMM_SNOOP_PROOF)) {
		act_puts("$N is no longer snoop-proof.", ch, NULL, victim,
			 TO_CHAR, POS_DEAD);
		char_puts("Your snoop-proofing was just removed.\n", victim);
		REMOVE_BIT(victim->comm, COMM_SNOOP_PROOF);
	}
	else {
		act_puts("$N is now snoop-proof.", ch, NULL, victim, TO_CHAR,
			 POS_DEAD);
		char_puts("You are now immune to snooping.\n", victim);
		SET_BIT(victim->comm, COMM_SNOOP_PROOF);
	}
}
	
void do_snoop(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	DESCRIPTOR_DATA_MUDDY *d;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Snoop whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim->desc == NULL) {
		char_puts("No descriptor to snoop.\n", ch);
		return;
	}

	if (victim == ch) {

		char_puts("Cancelling all snoops.\n", ch);
		wiznet("$N stops being such a snoop.", ch, NULL, WIZ_SNOOPS,
		       WIZ_SECURE, ch->level);
		for (d = descriptor_list_muddy; d; d = d->next)
			if (d->snoop_by == ch->desc)
				d->snoop_by = NULL;
		return;
	}

	if (victim->desc->snoop_by != NULL) {
		char_puts("Busy already.\n", ch);
		return;
	}

	if (!is_room_owner(ch,victim->in_room)
	&&  ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room)
	&&  !IS_TRUSTED(ch, IMPLEMENTOR)) {
		char_puts("That character is in a private room.\n",ch);
		return;
	}

	if (victim->level >= ch->level 
	||  IS_SET(victim->comm, COMM_SNOOP_PROOF)) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (ch->desc)
		for (d = ch->desc->snoop_by; d; d = d->snoop_by)
		    if (d->character == victim || d->original == victim) {
			char_puts("No snoop loops.\n", ch);
			return;
		    }

	victim->desc->snoop_by = ch->desc;
	wiznet("$N starts snooping on $i.",
		ch, victim, WIZ_SNOOPS, WIZ_SECURE, ch->level);
	char_puts("Ok.\n", ch);
}

void do_switch(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0') {
		char_puts("Switch into whom?\n", ch);
		return;
	}

	if (ch->desc == NULL)
		return;
	
	if (ch->desc->original != NULL) {
		char_puts("You are already switched.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (victim == ch) {
		char_puts("Ok.\n", ch);
		return;
	}

	if (!IS_NPC(victim) || IS_PC(victim)) {		// for be sure :)
		char_puts("You can only switch into mobiles.\n", ch);
		return;
	}

	if (!is_room_owner(ch,victim->in_room)
	&&  ch->in_room != victim->in_room 
	&&  room_is_private(victim->in_room)
	&&  !IS_TRUSTED(ch,IMPLEMENTOR)) {
		char_puts("That character is in a private room.\n", ch);
		return;
	}

	if (victim->desc != NULL) {
		char_puts("Character in use.\n", ch);
		return;
	}

	wiznet("$N switches into $i.",
		ch, victim, WIZ_SWITCHES, WIZ_SECURE, ch->level);

	ch->desc->character = victim;
	ch->desc->original  = ch;
	victim->desc        = ch->desc;
	ch->desc            = NULL;
	/* change communications to match */
/*	if (ch->prompt != NULL)
		victim->prompt = str_qdup(ch->prompt);
*/
	victim->comm = ch->comm;
/*	victim->lines = ch->lines; */
	char_puts("Ok.\n", victim);
}

void do_return(CHAR_DATA *ch, const char *argument)
{
	if (ch->desc == NULL)
		return;

	if (ch->desc->original == NULL)
		return;

	char_puts("You return to your original body.\n", ch);

/*	if (ch->prompt != NULL) {
		free_string(ch->prompt);
		ch->prompt = NULL;
	}
*/

	wiznet("$N returns from $i.",
		ch->desc->original, ch,
		WIZ_SWITCHES, WIZ_SECURE, ch->desc->original->level);
	ch->desc->character       = ch->desc->original;
	ch->desc->original        = NULL;
	ch->desc->character->desc = ch->desc; 
	ch->desc                  = NULL;

	do_replay(ch, str_empty);
}

/* trust levels for load and clone */
bool obj_check(CHAR_DATA *ch, OBJ_DATA *obj)
{
	if (IS_TRUSTED(ch, GOD)
	|| (IS_TRUSTED(ch, IMMORTAL) && obj->level <= 20 && obj->cost <= 1000)
	|| (IS_TRUSTED(ch, DEMI)     && obj->level <= 10 && obj->cost <= 500)
	|| (IS_TRUSTED(ch, ANGEL)    && obj->level <=  5 && obj->cost <= 250)
	|| (IS_TRUSTED(ch, AVATAR)   && obj->level ==  0 && obj->cost <= 100))
		return TRUE;
	else
		return FALSE;
}

bool mob_check(CHAR_DATA *ch, CHAR_DATA *mob)
{
	if ((mob->level > 20 && !IS_TRUSTED(ch, GOD))
	||  (mob->level > 10 && !IS_TRUSTED(ch, IMMORTAL))
	||  (mob->level >  5 && !IS_TRUSTED(ch, DEMI))
	||  (mob->level >  0 && !IS_TRUSTED(ch, ANGEL))
	||  !IS_TRUSTED(ch, AVATAR))
		return FALSE;
	else
		return TRUE;
}

/* for clone, to insure that cloning goes many levels deep */
void recursive_clone(CHAR_DATA *ch, OBJ_DATA *obj, OBJ_DATA *clone)
{
	OBJ_DATA *c_obj, *t_obj;
	for (c_obj = obj->contains; c_obj != NULL; c_obj = c_obj->next_content)
		if (obj_check(ch, c_obj)) {
			t_obj = create_obj(c_obj->pIndexData, 0);
			clone_obj(c_obj, t_obj);
			obj_to_obj(t_obj, clone);
			recursive_clone(ch, c_obj, t_obj);
		}
}

/* command that is similar to load */
void do_clone(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	const char *rest;
	CHAR_DATA *mob;
	OBJ_DATA  *obj;

	rest = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Clone what?\n",ch);
		return;
	}

	if (!str_prefix(arg,"object")) {
		mob = NULL;
		obj = get_obj_here(ch,rest);
		if (obj == NULL) {
		    char_puts("You don't see that here.\n",ch);
		    return;
		}
	} else if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")) {
		obj = NULL;
		mob = get_char_room(ch,rest);
		if (mob == NULL) {
		    char_puts("You don't see that here.\n",ch);
		    return;
		}
	} else { /* find both */
		mob = get_char_room(ch,argument);
		obj = get_obj_here(ch,argument);
		if (mob == NULL && obj == NULL) {
			char_puts("You don't see that here.\n",ch);
			return;
		}
	}

	/* clone an object */
	if (obj) {
		OBJ_DATA *clone;

		if (!obj_check(ch,obj)) {
			char_puts("You haven't enough power.\n", ch);
			return;
		}

		clone = create_obj(obj->pIndexData, 0); 
		clone_obj(obj, clone);
		if (obj->carried_by != NULL)
		    obj_to_char(clone, ch);
		else
		    obj_to_room(clone, ch->in_room);
		recursive_clone(ch, obj, clone);

		act("$n has created $p.", ch, clone, NULL, TO_ROOM);
		act("You clone $p.", ch, clone, NULL, TO_CHAR);
		wiznet("$N clones $p.",
			ch, clone, WIZ_LOAD, WIZ_SECURE, ch->level);
		return;
	} else if (mob != NULL) {
		CHAR_DATA *clone;
		OBJ_DATA *new_obj;

		if (!IS_NPC(mob)) {
		    char_puts("You can only clone mobiles.\n",ch);
		    return;
		}

		if (!mob_check(ch, mob)) {
			char_puts("You haven't enough power.\n", ch);
			return;
		}

		clone = create_mob(mob->pIndexData);
		clone_mob(mob,clone); 
		
		for (obj = mob->carrying; obj != NULL; obj = obj->next_content)
			if (obj_check(ch,obj)) {
				new_obj = create_obj(obj->pIndexData, 0);
				clone_obj(obj, new_obj);
				recursive_clone(ch, obj, new_obj);
				obj_to_char(new_obj, clone);
				new_obj->wear_loc = obj->wear_loc;
			}
		act("$n has created $N.", ch, NULL, clone, TO_ROOM);
		act("You clone $N.", ch, NULL, clone, TO_CHAR);
		wiznet("$N clones $i.",
			ch, clone, WIZ_LOAD, WIZ_SECURE, ch->level);
		char_to_room(clone, ch->in_room);
	}
}

/* RT to replace the two load commands */

void do_load(CHAR_DATA *ch, const char *argument)
{
	 char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Syntax:\n", ch);
		char_puts("  load mob <vnum>\n", ch);
		char_puts("  load obj <vnum> <level>\n", ch);
		return;
	}

	if (!str_cmp(arg,"mob") || !str_cmp(arg,"char")) {
		do_mload(ch, argument);
		return;
	}

	if (!str_cmp(arg,"obj")) {
		do_oload(ch, argument);
		return;
	}
	/* echo syntax */
	do_load(ch,str_empty);
}

void do_mload(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	MOB_INDEX_DATA *pMobIndex;
	CHAR_DATA *victim;
	int vnum;
	
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || !is_number(arg)) {
		char_puts("Syntax: load mob <vnum>.\n", ch);
		return;
	}

	if ((pMobIndex = get_mob_index(vnum = atoi(arg))) == NULL) {
		char_printf(ch, "%d: No mob has that vnum.\n", vnum);
		return;
	}

	victim = create_mob(pMobIndex);
	act("$n has created $N!", ch, NULL, victim, TO_ROOM);
	wiznet("$N loads $i.", ch, victim, WIZ_LOAD, WIZ_SECURE, ch->level);
	char_puts("Ok.\n", ch);
	char_to_room(victim, ch->in_room);
}

void do_oload(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH] ,arg2[MAX_INPUT_LENGTH];
	OBJ_INDEX_DATA *pObjIndex;
	OBJ_DATA *obj;
	int vnum;
	
	argument = one_argument(argument, arg1, sizeof(arg1));
	one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || !is_number(arg1)) {
		char_puts("Syntax: load obj <vnum>.\n", ch);
		return;
	}
	
	vnum = atoi(arg1);
	if ((pObjIndex = get_obj_index(vnum)) == NULL) {
		char_printf(ch, "%d: No objects with this vnum.\n", vnum);
		return;
	}

	obj = create_obj(pObjIndex, 0);
	if (CAN_WEAR(obj, ITEM_TAKE))
		obj_to_char(obj, ch);
	else
		obj_to_room(obj, ch->in_room);
	act("$n has created $p!", ch, obj, NULL, TO_ROOM);
	wiznet("$N loads $p.", ch, obj, WIZ_LOAD, WIZ_SECURE, ch->level);
	char_puts("Ok.\n", ch);
}

void do_purge(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;
	DESCRIPTOR_DATA_MUDDY *d;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		/* 'purge' */
		CHAR_DATA *vnext;
		OBJ_DATA  *obj_next;

		for (victim = ch->in_room->people; victim; victim = vnext) {
			vnext = victim->next_in_room;
			if (IS_NPC(victim)
			&&  !IS_SET(victim->pIndexData->act, ACT_NOPURGE) 
			&&  victim != ch /* safety precaution */)
				extract_char(victim, 0);
		}

		for (obj = ch->in_room->contents; obj != NULL; obj = obj_next) {
			obj_next = obj->next_content;
			if (!IS_OBJ_STAT(obj, ITEM_NOPURGE))
				extract_obj(obj, 0);
		}

		act("$n purges the room!", ch, NULL, NULL, TO_ROOM);
		char_puts("Ok.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_PC(victim)) {
		if (ch == victim) {
			char_puts("Ho ho ho.\n", ch);
			return;
		}

		if (ch->level <= victim->level) {
			char_puts("Maybe that wasn't a good idea...\n",ch);
			char_printf(ch,"%s tried to purge you!\n",ch->name);
			return;
		}

		act("$n disintegrates $N.", ch, 0, victim, TO_NOTVICT);

		if (victim->level > 1)
			save_char_obj(victim, FALSE);
		d = victim->desc;
		extract_char(victim, 0);
		if (d)
			close_descriptor_muddy(d);
		return;
	}

	act("$n purges $N.", ch, NULL, victim, TO_NOTVICT);
	extract_char(victim, 0);
}

void do_restore(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	CHAR_DATA *vch;
	DESCRIPTOR_DATA_MUDDY *d;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0' || !str_cmp(arg,"room")) {
	/* cure room */
		
	    for (vch = ch->in_room->people; vch; vch = vch->next_in_room) {
	        affect_strip(vch,gsn_plague);
	        affect_strip(vch,gsn_poison);
	        affect_strip(vch,gsn_blindness);
	        affect_strip(vch,gsn_sleep);
	        affect_strip(vch,gsn_curse);
	        
	        vch->hit 	= vch->max_hit;
	        vch->mana	= vch->max_mana;
	        vch->move	= vch->max_move;
	        update_pos(vch);
	        act("$n has restored you.",ch,NULL,vch,TO_VICT);
	    }

		wiznet("$N restored room $j.",
			ch, (const void*) ch->in_room->vnum,
			WIZ_RESTORE, WIZ_SECURE, ch->level);
	    
	    char_puts("Room restored.\n",ch);
	    return;

	}
	
	if (ch->level >=  MAX_LEVEL - 1 && !str_cmp(arg,"all")) {
	/* cure all */
	    for (d = descriptor_list_muddy; d; d = d->next)
	    {
		    victim = d->character;

		    if (victim == NULL || IS_NPC(victim))
			continue;
	            
	        affect_strip(victim,gsn_plague);
	        affect_strip(victim,gsn_poison);
	        affect_strip(victim,gsn_blindness);
	        affect_strip(victim,gsn_sleep);
	        affect_strip(victim,gsn_curse);
	        
	        victim->hit 	= victim->max_hit;
	        victim->mana	= victim->max_mana;
	        victim->move	= victim->max_move;
	        update_pos(victim);
		    if (victim->in_room != NULL)
	            act("$n has restored you.",ch,NULL,victim,TO_VICT);
	    }
		char_puts("All active players restored.\n",ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	affect_strip(victim, gsn_plague);
	affect_strip(victim, gsn_poison);
	affect_strip(victim, gsn_blindness);
	affect_strip(victim, gsn_sleep);
	affect_strip(victim, gsn_curse);
	victim->hit  = victim->max_hit;
	victim->mana = victim->max_mana;
	victim->move = victim->max_move;
	update_pos(victim);
	act("$n has restored you.", ch, NULL, victim, TO_VICT);
	wiznet("$N restored $i",
		ch, victim, WIZ_RESTORE, WIZ_SECURE, ch->level);
	char_puts("Ok.\n", ch);
}
		
void do_freeze(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Freeze whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (victim->level >= ch->level) {
		char_puts("You failed.\n", ch);
		return;
	}

	TOGGLE_BIT(victim->pcdata->plr_flags, PLR_FREEZE);
	if (!IS_SET(victim->pcdata->plr_flags, PLR_FREEZE)) {
		char_puts("You can play again.\n", victim);
		char_puts("FREEZE removed.\n", ch);
		wiznet("$N thaws $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else {
		char_puts("You can't do ANYthing!\n", victim);
		char_puts("FREEZE set.\n", ch);
		wiznet("$N puts $i in the deep freeze.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	save_char_obj(victim, FALSE);
}

void do_log(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Log whom?\n", ch);
		return;
	}

	if (!str_cmp(arg, "all")) {
		if (fLogAll) {
			fLogAll = FALSE;
			char_puts("Log ALL off.\n", ch);
		} else {
			fLogAll = TRUE;
			char_puts("Log ALL on.\n", ch);
		}
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	/*
	 * No level check, gods can log anyone.
	 */
	TOGGLE_BIT(victim->pcdata->plr_flags, PLR_LOG);
	if (!IS_SET(victim->pcdata->plr_flags, PLR_LOG))
		char_puts("LOG removed.\n", ch);
	else 
		char_puts("LOG set.\n", ch);
}

void do_noemote(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Noemote whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->level >= ch->level) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOEMOTE)) {
		REMOVE_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can emote again.\n", victim);
		char_puts("NOEMOTE removed.\n", ch);
		wiznet("$N restores emotes to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else {
		SET_BIT(victim->comm, COMM_NOEMOTE);
		char_puts("You can't emote!\n", victim);
		char_puts("NOEMOTE set.\n", ch);
		wiznet("$N revokes $i's emotes.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

void do_noshout(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Noshout whom?\n",ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (victim->level >= ch->level) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOSHOUT)) {
		REMOVE_BIT(victim->comm, COMM_NOSHOUT);
		char_puts("You can shout again.\n", victim);
		char_puts("NOSHOUT removed.\n", ch);
		wiznet("$N restores shouts to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	} else {
		SET_BIT(victim->comm, COMM_NOSHOUT);
		char_puts("You can't shout!\n", victim);
		char_puts("NOSHOUT set.\n", ch);
		wiznet("$N revokes $i's shouts.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

void do_nochat(CHAR_DATA *ch, const char *argument)
{
        char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;

//	one_argument(argument, arg);
	one_argument(argument, arg, sizeof(arg));

        if (arg[0] == '\0') {
                char_puts("Nochat whom?\n",ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL) {
                char_puts("They aren't here.\n", ch);
                return;
        }

        if (IS_NPC(victim)) {
                char_puts("Not on NPC's.\n", ch);
                return;
        }

        if (victim->level >= ch->level) {
                char_puts("You failed.\n", ch);
                return;
        }

        if (IS_SET(victim->comm, COMM_NOCHAT)) {
                REMOVE_BIT(victim->comm, COMM_NOCHAT);
                char_puts("You can chat again.\n", victim);
                char_puts("NOCHAT removed.\n", ch);
				snprintf(buf, sizeof(buf), "$N restores chats to %s.",victim->name);
                wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	} else {
                SET_BIT(victim->comm, COMM_NOCHAT);
                char_puts("You can't chat!\n", victim);
                char_puts("NOCHAT set.\n", ch);
				snprintf(buf, sizeof(buf), "$N revokes %s's chats.",victim->name);
                wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
        }
}

void do_nonote(CHAR_DATA *ch, const char *argument)
{
        char arg[MAX_INPUT_LENGTH],buf[MAX_STRING_LENGTH];
        CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

        if (arg[0] == '\0') {
                char_puts("Nonote whom?\n",ch);
                return;
        }

        if ((victim = get_char_world(ch, arg)) == NULL) {
                char_puts("They aren't here.\n", ch);
                return;
        }

        if (IS_NPC(victim)) {
                char_puts("Not on NPC's.\n", ch);
                return;
        }

        if (victim->level >= ch->level) {
                char_puts("You failed.\n", ch);
                return;
        }

        if (IS_SET(victim->pcdata->plr_flags, PLR_NONOTE)) {
                REMOVE_BIT(victim->pcdata->plr_flags, PLR_NONOTE);
                char_puts("You can write notes again.\n", victim);
                char_puts("NONOTE removed.\n", ch);
				snprintf(buf, sizeof(buf), "$N restores note channel to %s.",victim->name);
                wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
	} else {
                SET_BIT(victim->pcdata->plr_flags, PLR_NONOTE);
                char_puts("You can't write notes!\n", victim);
                char_puts("NONOTE set.\n", ch);
				snprintf(buf, sizeof(buf), "$N revokes %s's note channel.",victim->name);
                wiznet(buf,ch,NULL,WIZ_PENALTIES,WIZ_SECURE,0);
        }

}

void do_notell(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Notell whom?", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (!IS_NPC(victim) && victim->level >= ch->level) {
		char_puts("You failed.\n", ch);
		return;
	}

	if (IS_SET(victim->comm, COMM_NOTELL)) {
		REMOVE_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can tell again.\n", victim);
		char_puts("NOTELL removed.\n", ch);
		wiznet("$N restores tells to $i.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
	else {
		SET_BIT(victim->comm, COMM_NOTELL);
		char_puts("You can't tell!\n", victim);
		char_puts("NOTELL set.\n", ch);
		wiznet("$N revokes $i's tells.",
			ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	}
}

void do_peace(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *rch;

	for (rch = ch->in_room->people; rch; rch = rch->next_in_room) {
/*		if (!rch->fighting)
			continue;
 */
		stop_fighting(rch, TRUE);
		
		if (IS_NPC(rch)) {
			/*
			 * avoid aggressive mobs and hunting mobs attacks
			 */
			AFFECT_DATA af;

			rch->last_fought = NULL;
			
			af.where = TO_AFFECTS;
			af.type = gsn_reserved;
			af.level = MAX_LEVEL;
			af.duration = 15;
			af.location = APPLY_NONE;
			af.modifier = 0;
			af.bitvector = AFF_CALM | AFF_SCREAM;
			affect_to_char(rch, &af);
		}
	}

	char_puts("Ok.\n", ch);
}

void do_wizlock(CHAR_DATA *ch, const char *argument)
{
	extern bool wizlock;
	wizlock = !wizlock;

	if (wizlock) {
		wiznet("$N has wizlocked the game.", ch, NULL, 0, 0, 0);
		char_puts("Game wizlocked.\n", ch);
	}
	else {
		wiznet("$N removes wizlock.", ch, NULL, 0, 0, 0);
		char_puts("Game un-wizlocked.\n", ch);
	}
}

/* RT anti-newbie code */
void do_newlock(CHAR_DATA *ch, const char *argument)
{
	extern bool newlock;
	newlock = !newlock;
	
	if (newlock) {
		wiznet("$N locks out new characters.", ch, NULL, 0, 0, 0);
		char_puts("New characters have been locked out.\n", ch);
	}
	else {
		wiznet("$N allows new characters back in.", ch, NULL, 0, 0, 0);
		char_puts("Newlock removed.\n", ch);
	}
}

static CHAR_DATA *lookup_char_agr(CHAR_DATA *ch, const char *arg)
{
	CHAR_DATA *victim;
	
	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return NULL;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return NULL;
	}

	return victim;
}

static RELIGION_DATA *test_rp(CHAR_DATA *ch)
{
	if (ch->pcdata->religion_vn)
		return RELIGION(ch->pcdata->religion_vn);
	char_puts("You are not play some god now.\n", ch); 
	return NULL;
}

void do_rp(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	bool repeat;
	DESCRIPTOR_DATA_MUDDY *d;
	
	if (!IS_PC(ch))
		return;

	argument = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] == '\0') {
		char_puts("Syntax: rp <field> [<modifer/target>]\n"
			"  Fields  with <modifer> are: spgod rename confd\n"
			"    rp t[ell] <victim> <that>\n"
			"    rp s[hout] <that>\n"
			"    rp r[epeat] <that>\n",
			ch);
		return;
	}

	if (argument[0] == '\0') {
		char_puts("Input <modifer/target>.\n", ch);
		return;
	}
	
	if (!str_prefix(arg1, "repeat")) {
		victim = ch->pcdata->repeat;
		repeat = TRUE;
	} else {
		repeat = FALSE;
		victim = NULL;
	}

	if (!str_prefix(arg1, "tell") || repeat) {
		RELIGION_DATA *r;
		char arg3[MAX_INPUT_LENGTH];
		
		if (repeat == FALSE) {
			argument = one_argument(argument, arg2, sizeof(arg2));
			victim = get_char_world(ch, arg2);
		}

		if (victim == NULL)
		{
			char_puts("They aren't here.\n", ch);
			return;
		}
		
		if (argument[0] == '\0') {
			char_puts("Tell(repeat) what?\n", ch);
			return;
		}

		if (!(r = test_rp(ch)))
			return;
	
		snprintf(arg3, sizeof(arg3), "You <{m%s{x> tell $N '{G$t{x'",
			r->name);
		act_puts(arg3, ch, argument, victim, TO_CHAR | ACT_NODEAF, POS_DEAD);
	
		snprintf(arg3, sizeof(arg3), "%s tells you '{G$t{x'",
			r->name);
		act_puts(arg3, ch, argument, victim,
			TO_VICT | ACT_TOBUF | ACT_NODEAF, POS_DEAD);
	
		snprintf(arg3, sizeof(arg3), "$n <{m%s{x> tells to {W%s{x '{G$t{x'",
			r->name, victim->name);

		for (d = descriptor_list_muddy; d; d = d->next)
			if (d->connected == CON_PLAYING
			&& d->character
			&& IS_IMMORTAL(d->character)
			&& d->character != ch
			&& d->character != victim
			&& IS_SET(d->character->pcdata->wiznet, WIZ_RP))
			{
				act_puts(arg3, ch, argument, d->character,
					TO_VICT | ACT_NODEAF | ACT_TOBUF, POS_DEAD);
			} 

		if (IS_PC(victim))
			victim->reply = ch;
		ch->pcdata->repeat = victim;
		return;
	}

	if (!str_prefix(arg1, "shout")) {
		RELIGION_DATA *r;

		if (!(r = test_rp(ch)))
			return;
		act_puts("You <{m$T{x> shouts '{Y$t{x'",
			ch, argument, r->name, TO_CHAR | ACT_NODEAF, POS_DEAD);
		for (d = descriptor_list_muddy; d; d = d->next)
			if (d->connected == CON_PLAYING
			&& d->character
			&& d->character != ch
			&& !IS_SET(d->character->comm, COMM_NOSHOUT))
			{
				if (IS_PC(d->character)
				&& IS_SET(d->character->pcdata->wiznet, WIZ_RP))
					char_printf(d->character,
						"%s <{m%s{x> shouts '{Y%s{x'\n",
						ch->name, r->name, argument);
				else
					char_printf(d->character,
						"%s shouts '{Y%s{x'\n",
						r->name, argument);
			} 
		return;
	}
	
	argument = one_argument(argument, arg2, sizeof(arg2));
	
	if (!str_prefix(arg1, "spgod")) {
		CHAR_DATA *vch;
		int rv = rel_lookup(arg2);
		
		if (rv < 0 || !RELIGION(rv))
		{
			char_puts("Unknown religion.\n", ch);
			return;
		}

		if (rv) for (vch = char_list; vch && IS_PC(vch); vch = vch->next)
			if (IS_PC(vch)
			&& vch->pcdata->religion_vn == rv)
			{
				act("$N already playing this god.", ch, NULL, vch, TO_CHAR);
				return;
			}
		char_printf(ch, "Now you begin play by '%s'.\n", RELIGION(rv)->name);
		wiznet("$N begin speak by god - '$t'.", ch, RELIGION(rv)->name,
				WIZ_RP, 0, 0);
		ch->pcdata->religion_vn = rv;
			/* Clear reply from char at religion */
		if (rv == 0)
		{
			char_puts("Reset all tells (reply from other chars)!\n", ch);
			for (vch = char_list; vch && IS_PC(vch); vch = vch->next)
			{
				if (vch->reply == ch)
					vch->reply = NULL;
				if (vch->pcdata->repeat == ch)
					vch->pcdata->repeat = NULL;
			}
		}
		return;
 	}

	if (!str_prefix(arg1, "confd")) {
		if (!(victim = lookup_char_agr(ch, arg2)))
			return;
		TOGGLE_BIT(victim->pcdata->otherf, OTHERF_CONFIRM_DESC);
		if (IS_SET(victim->pcdata->otherf, OTHERF_CONFIRM_DESC)) {
			char_printf(ch, "You confirm %s's description.\n", victim->name);
			char_puts("{GYour descriptions has been confirmed by immortals.{x\n", victim);
		} else {
			char_printf(ch, "You remove confirm for %s's description.\n", victim->name);
			char_puts("{RYour descriptions has been denied by immortals.{x\n", victim);
		}
		wiznet("$N toggle of confirm description for $i.", ch, victim,
				WIZ_PENALTIES, 0, 0);
		return;
	}
	if (!str_prefix(arg1, "rename")){
		if (!(victim = lookup_char_agr(ch, arg2)))
			return;
		TOGGLE_BIT(victim->pcdata->otherf, OTHERF_RENAME);
		if (IS_SET(victim->pcdata->otherf, OTHERF_RENAME)) {
			char_printf(ch, "You set %s's rename flag.\n", victim->name);
			char_puts("{GYou must choose another name.{x\n", 
					victim);
			SET_BIT(victim->pcdata->otherf, OTHERF_NOEXP);
		} else {
			char_printf(ch, "You remove rename flag for %s.\n", 
					victim->name);
			char_puts("{RYour name is very nice. Good luck! :){x\n",
					victim);
			REMOVE_BIT(victim->pcdata->otherf, OTHERF_NOEXP);
		}
		wiznet("$N toggle bit of 'rename' for $i.", ch, victim,
				WIZ_PENALTIES, 0, 0);
		return;
	}

	do_rp(ch, str_empty);	
}

void do_add(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int modifer;

	argument = one_argument(argument, arg1, sizeof(arg1));
	if (arg1[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  add <char> <field> [<modifer>]\n", ch);
		char_puts("  Fields  with <modifer> are: questp, exp, gold, silver, prac\n", ch);
		return;
	}

	argument = one_argument(argument, arg2, sizeof(arg2));
	
	if (arg1[0] == '\0') {
		char_puts("Input char name.\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if (arg2[0] == '\0'){
		char_puts("  Fields are: questp, exp, gold, silver, prac, confd\n", ch);
		return;
	}	
	argument = one_argument(argument, arg3, sizeof(arg3));

	if (arg3[0] == '\0'){
		char_puts("  Input modifer.\n", ch);
		return;
	}

	if (!is_number(arg3)) {
		char_puts("Value must be numeric.\n", ch);
		return;
	}

	modifer = atoi(arg3);

	if (!str_prefix(arg2, "questp")){
		if (victim->pcdata->questpoints + modifer < 0){
			char_puts("Quest point's can't less 0.\n", ch);
			return;
		}
		victim->pcdata->questpoints += modifer;
		return;
	}

	if (!str_prefix(arg2, "exp")){
		gain_exp(victim, modifer, FALSE, TRUE);
		return;
	}

	if (!str_prefix(arg2, "gold")){
		if (get_carry_weight(victim) + modifer * 2 / 5 > can_carry_w(victim))
		{
			char_puts("Char can't carry added gold.\n", ch);
			return;
		}
		if (victim->gold + modifer < 0){
			char_puts("Gold can't less 0.\n",ch);
			return;
		}
		victim->gold += modifer;
		return;
	}

	if (!str_prefix(arg2, "silver")){
		if (get_carry_weight(victim) + modifer / 10 > can_carry_w(victim))
		{
			char_puts("Char can't carry added silver.\n", ch);
			return;
		}
		if (victim->silver + modifer < 0){
			char_puts("Silver can't less 0.\n",ch);
			return;
		}
		victim->silver += modifer;
		return;
	}

	if (!str_prefix(arg2, "prac")){
		if (victim->pcdata->practice + modifer < 0){
			char_puts("Practice can't less 0.\n", ch);
			return;
		}
		victim->pcdata->practice += modifer;
		return;
	}

	do_add(ch, str_empty);
}

/* RT set replaces sset, mset, oset, and rset */
void do_set(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  set mob   <name> <field> <value>\n",ch);
		char_puts("  set obj   <name> <field> <value>\n",ch);
		char_puts("  set room  <room> <field> <value>\n",ch);
		char_puts("  set skill <name> <spell or skill> <percent> <level> <maxlearn> <hard>\n",ch);
		return;
	}

	if (!str_prefix(arg,"mobile") || !str_prefix(arg,"character")) {
		do_mset(ch,argument);
		return;
	}

	if (!str_prefix(arg,"skill") || !str_prefix(arg,"spell")) {
		do_sset(ch,argument);
		return;
	}

	if (!str_prefix(arg,"object")) {
		do_oset(ch,argument);
		return;

	}

	if (!str_prefix(arg,"room")) {
		do_rset(ch,argument);
		return;
	}
	/* echo syntax */
	do_set(ch,str_empty);
}

void do_sset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	char arg5 [MAX_INPUT_LENGTH];
	char arg6 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value, v1, v2, v3;
	int sn, counter_sk = 0, counter_sp = 0;
	bool fAll;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	argument = one_argument(argument, arg3, sizeof(arg3));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  set skill <name> <spell or skill> <percent> <level> <mp> <hard>\n", ch);
		char_puts("  set skill <name> all <value>\n",ch);  
		char_puts("(use the name of the skill, not the number)\n",ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	fAll = !str_cmp(arg2, "all");
	sn   = 0;
	if (!fAll && (sn = sn_lookup(arg2)) < 0) {
		char_puts("No such skill or spell.\n", ch);
		return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		char_puts("Value must be numeric.\n", ch);
		return;
	}

	value = atoi(arg3);
	if (value < 0 || value > 100) {
		char_puts("Percent range is 0 to 100.\n", ch);
		return;
	}

	if (fAll){
		for (sn = 0; sn < skills.nused; sn++)
			if (skill_lookup(sn)->spell)
				set_skill_raw(victim, sn, value, TRUE, 1+ (counter_sp++) / 50, 100, 50);
			else
				set_skill_raw(victim, sn, value, TRUE, 1+ (counter_sk++) / 50, 100, 50);
		char_printf(ch, "Added %d skills and %d spells.\n",
				counter_sp, counter_sk);
		return;
	}
	
	argument = one_argument(argument, arg4, sizeof(arg4));
	argument = one_argument(argument, arg5, sizeof(arg5));
	argument = one_argument(argument, arg6, sizeof(arg6));
	
	if (arg4[0] == '\0' || arg5[0] == '\0' || arg6[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  set skill <name> <spell or skill> <percent> <level> <mp> <hard>\n", ch);
	}
	if (!is_number(arg4) || !is_number(arg5) || !is_number(arg6)) {
		char_puts("Value must be numeric.\n", ch);
		return;
	}
	v1 = atoi(arg4); v2 = atoi(arg5); v3 = atoi(arg6);
	
	if (v1 < 0 || v1 > MAX_LEVEL) {
		char_puts("Level range is 0 to MAX_LEVEL(100).\n", ch);
		return;
	}
	if (v2 < 0 || v2 > 100 || v2 < value) {
		char_puts("MaxPercent range is 0 to 100 and more or equal than percent.\n", ch);
		return;
	}
	if (v3 < 0 || v3 > 100) {
		char_puts("Hard range is 0 to 100.\n", ch);
		return;
	}
	
	set_skill_raw(victim, sn, value, TRUE, v1, v2, v3);
	char_puts("Ok.\n", ch);
}

void do_string(CHAR_DATA *ch, const char *argument)
{
	char type[MAX_INPUT_LENGTH];
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	OBJ_DATA *obj;

	argument = one_argument(argument, type, sizeof(type));
	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	strnzcpy(arg3, sizeof(arg3), argument);

	if (type[0] == '\0' || arg1[0] == '\0'
	||  arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  string char <name> <field> <string>\n",ch);
		char_puts("    fields: name short long desc title spec\n",ch);
		char_puts("  string obj  <name> <field> <string>\n",ch);
		char_puts("    fields: name short long extended\n",ch);
		return;
	}
	
	if (!str_prefix(type, "character") || !str_prefix(type, "mobile")) {
		if ((victim = get_char_room(ch, arg1)) == NULL) {
			char_puts("They aren't here.\n", ch);
			return;
		}

		/* clear zone for mobs */
		victim->zone = NULL;

		/* string something */

	 	if (!str_prefix(arg2, "name")) {
			if (!IS_NPC(victim)) {
				char_puts("Not on PC's.\n", ch);
				return;
			}
			free_string(victim->name);
			victim->name = str_dup(arg3);
			return;
		}
		
		if (!str_prefix(arg2, "short")) {
			if (!IS_NPC(victim)) {
				char_puts(" Not on PC's.\n", ch);
				return;
			}
			mlstr_edit(&victim->short_descr, arg3);
			return;
		}

		if (!str_prefix(arg2, "desc")) {
			mlstr_append(ch, &victim->description, arg3);
			return;
		}

		if (!str_prefix(arg2, "long")) {
			if (!IS_NPC(victim)) {
				char_puts("Not on PC's.\n", ch);
				return;
			}
			mlstr_editnl(&victim->long_descr, arg3);
			return;
		}

		if (!str_prefix(arg2, "title")) {
			if (IS_NPC(victim)) {
				char_puts("Not on NPC's.\n", ch);
				return;
			}

			set_title(victim, arg3);
			return;
		}
	}
	
	if (!str_prefix(type,"object")) {
		/* string an obj */
		
	 	if ((obj = get_obj_room(ch, arg1)) == NULL) {
			char_puts("Nothing like that in heaven or earth.\n",
				  ch);
			return;
		}
		
		if (obj->pIndexData->limit >= 0) {
			char_puts("You cannot string limited objs.\n", ch);
			return;
		}

		if (!str_prefix(arg2, "name")) {
			free_string(obj->name);
			obj->name = str_dup(arg3);
			return;
		}

		if (!str_prefix(arg2, "short")) {
			mlstr_edit(&obj->short_descr, arg3);
			return;
		}

		if (!str_prefix(arg2, "long")) {
			mlstr_edit(&obj->description, arg3);
			return;
		}

		if (!str_prefix(arg2, "ed")
		||  !str_prefix(arg2, "extended")
		||  !str_prefix(arg2, "exd")) {
			ED_DATA *ed;

			if (obj->carried_by != ch) {
				char_puts("Obj must be in your inventory.\n", ch);
				return;
			}

			argument = one_argument(argument, arg3, sizeof(arg3));
			if (argument == NULL) {
				char_puts("Syntax: oset <object> ed <keyword> "
					  "lang\n", ch);
				return;
			}

			ed = ed_new();
			ed->keyword	= str_dup(arg3);
			ed->next	= obj->ed;
			mlstr_append(ch, &ed->description, argument);
			obj->ed	= ed;
			return;
		}
	}
	
	/* echo bad use message */
	do_string(ch,str_empty);
}

void do_oset(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	char arg3[MAX_STRING_LENGTH];
	OBJ_DATA *obj;
	int value;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
		   one_argument(argument, arg3, sizeof(arg3));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  set obj <object> <field> <value>\n",ch);
		char_puts("Field being one of:\n", ch);
		char_puts("value0 value1 value2 value3 value4 (v1-v4)\n", ch);
		char_puts("owner level cost timer\n",	ch);
		return;
	}

	if ((obj = get_obj_world(ch, arg1)) == NULL) {
		char_puts("Nothing like that in heaven or earth.\n", ch);
		return;
	}

	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "value0") || !str_cmp(arg2, "v0")) {
		obj->value[0] = UMIN(50,value);
		return;
	}

	if (!str_cmp(arg2, "value1") || !str_cmp(arg2, "v1")) {
		obj->value[1] = value;
		return;
	}

	if (!str_cmp(arg2, "value2") || !str_cmp(arg2, "v2")) {
		obj->value[2] = value;
		return;
	}

	if (!str_cmp(arg2, "value3") || !str_cmp(arg2, "v3")) {
		obj->value[3] = value;
		return;
	}

	if (!str_cmp(arg2, "value4") || !str_cmp(arg2, "v4")) {
		obj->value[4] = value;
		return;
	}

	if (!str_prefix(arg2, "level")) {
		obj->level = value;
		return;
	}
		
	if (!str_prefix(arg2, "cost")) {
		obj->cost = value;
		return;
	}

	if (!str_prefix(arg2, "timer")) {
		obj->timer = value;
		return;
	}
		
	if (!str_prefix(arg2, "owner")) {
		mlstr_free(obj->owner);
		obj->owner = mlstr_new(arg3);
	}

	/*
	 * Generate usage message.
	 */
	do_oset(ch, str_empty);
	return;
}

void do_rset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	ROOM_INDEX_DATA *location;
	int value;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
		   one_argument(argument, arg3, sizeof(arg3));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  set room <location> <field> <value>\n",ch);
		char_puts("  Field being one of:\n",			ch);
		char_puts("    flags sector\n",				ch);
		return;
	}

	if ((location = find_location(ch, arg1)) == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	if (ch->in_room != location 
	&&  room_is_private(location) && !IS_TRUSTED(ch,IMPLEMENTOR)) {
	    char_puts("That room is private right now.\n",ch);
	    return;
	}

	/*
	 * Snarf the value.
	 */
	if (!is_number(arg3)) {
		char_puts("Value must be numeric.\n", ch);
		return;
	}
	value = atoi(arg3);

	/*
	 * Set something.
	 */
	if (!str_prefix(arg2, "flags")) {
		location->room_flags	= value;
		return;
	}

	if (!str_prefix(arg2, "sector")) {
		location->sector_type	= value;
		return;
	}

	/*
	 * Generate usage message.
	 */
	do_rset(ch, str_empty);
	return;
}

/* some ideas by Bellionore (Crazy Slash) */
void do_sockets(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	char arg[MAX_INPUT_LENGTH];
	DESCRIPTOR_DATA_MUDDY *d;

	CHAR_DATA *vch;
	int count;
	char            buf  [MAX_STRING_LENGTH];
	char *          st;
	char            s[100];
	char            idle[10];

	count = 0;
	output = buf_new(-1);

	one_argument(argument, arg, sizeof(arg));
	if (!str_ncmp(arg, "all", 4)){
		CHAR_DATA *wch;
		
		buf_printf(output, "All (!IS_NPC).\n");
		for (wch = char_list; wch && !IS_NPC(wch); wch = wch->next)
			buf_printf(output, "   %d.%s\n", ++count, wch->name);
		
		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	} else if (!str_ncmp(arg, "tun", 4))
	{
		DESCRIPTOR_DATA_TUNNEL	* dt;
		int			port;
		int			i;
		insockets_t		* listen_data;

		//for (dt = tunnel_list; dt; dt = dt->next) {
		for (i = 0; i < tunnel_list.nused; i++)
		{
			dt = VARR_GET(&tunnel_list, i);
			if (dt->idtun == -1)
				continue;
			if (dt->from_connect)
			{
				buf_printf(output, "    %d: remote %s tunnel %d\n",
						++count, strid_dm(dt->from_connect),
						dt->idtun);
				continue;
			}
			for (port = 0; port < dt->listening_sockets.nused;
					port++)
			{
				listen_data = VARR_GET(&(dt->listening_sockets), port);
				buf_printf(output, "    %d: local tunnel %d(%s) port %d\n",
					++count,
					dt->idtun,
					strid_dm(dt->from_connect),
					ntohs(listen_data->listen.sin_port));
			}
		}

		page_to_char(buf_string(output), ch);
		buf_free(output);
		return;
	}

	buf[0]      = '\0';

	buf_add(output, "\n[Connected_State Login@ Idl] Player Name Host [MCCP?]\n");
	buf_add(output, "--------------------------------------------------------------------------\n");

	for (d = descriptor_list_muddy; d; d = d->next)
	{
		if (d->character && can_see(ch, d->character)) {
			switch(d->connected) {
				case CON_PLAYING:              st = "    PLAYING    ";    break;
				case CON_DELETE:               st = "  DELETING!!!  ";    break;
				case CON_RESOLV:               st = "   Resolving   ";    break;
				case CON_GET_CODEPAGE:         st = " Get Codepage  ";    break;
				case CON_GET_ETHOS:            st = "   Get Ethos   ";    break;
				case CON_PICK_HOMETOWN:        st = " Picking Home  ";    break;
				case CON_CONFIRM_NEW_NAME:     st = "Confirm NewName";    break;
				case CON_ACCEPT_STATS:         st = "Accepting Stats";    break;
				case CON_CREATE_DONE:          st = " Creating Done ";    break;
				case CON_GET_NAME:             st = "   Get Name    ";    break;
				case CON_ROLL_STATS:           st = " Rolling Stats ";    break;
				case CON_GET_OLD_PASSWORD:     st = "Get Old Passwd ";    break;
				case CON_GET_NEW_PASSWORD:     st = "Get New Passwd ";    break;
				case CON_CONFIRM_NEW_PASSWORD: st = "Confirm Passwd ";    break;
				case CON_GET_NEW_RACE:         st = "  Get New Race ";    break;
				case CON_GET_NEW_SEX:          st = "  Get New Sex  ";    break;
				case CON_GET_NEW_CLASS:        st = " Get New Class ";    break;
				case CON_GET_ALIGNMENT:        st = " Get New Align ";    break;
				case CON_DEFAULT_CHOICE:       st = " Choosing Cust ";    break;
				case CON_GEN_GROUPS:           st = " Customization ";    break;
				case CON_PICK_WEAPON:          st = " Picking Weapon";    break;
				case CON_READ_IMOTD:           st = " Reading IMOTD ";    break;
				case CON_BREAK_CONNECT:        st = "   LINKDEAD    ";    break;
				case CON_READ_MOTD:		st = "  Reading MOTD ";	break;
				case CON_GET_LANG:		st = " Get Language  ";	break;
				case CON_REMORT:		st = "    REMORT     "; break;
				case CON_GET_COLOR:		st = "   Get Color   "; break;
				case CON_NULLLOOP:		st = "null nanny loop";	break;
				case CON_NULL:			st = "    null       ";	break;
				default:			st = "   !UNKNOWN!   ";	break;
			}
			count++;
           
			/* Format "login" value... */
			vch = d->original ? d->original : d->character;
			strftime( s, 100, "%I:%M%p", localtime(&vch->pcdata->logon));
           
			if (vch->timer > 0)
				snprintf(idle, sizeof(idle), "%-2d", vch->timer);
			else
				snprintf(idle, sizeof(idle), "  ");
           
			snprintf(buf, sizeof(buf), "[%s %7s %2s] %-12s %s \"%s\"%s\n\r",
				st, s, idle,
				(d->original) ? d->original->name : (d->character) 
					? d->character->name : "(None!)",
					strid_dc(d->client),
					get_hostname(d->client),
	#ifdef MCCP
					check_mccp(d->client) ? " [M]" :
	#endif
					str_empty
					);
              
			buf_add(output, buf);
		}
	}

	snprintf(buf, sizeof(buf), "\n\r%d user%s\n\r", count, count == 1 ? "" : "s");
	buf_add(output, buf);
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

/*
 * Thanks to Grodyn for pointing out bugs in this function.
 */
void do_force(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0' || argument[0] == '\0') {
		char_puts("Force whom to do what?\n", ch);
		return;
	}

	one_argument(argument, arg2, sizeof(arg2));
	
	if (!str_cmp(arg2,"delete") || !str_prefix(arg2,"mob")) {
		char_puts("That will NOT be done.\n",ch);
		return;
	}


	if (!str_cmp(arg, "all")) {
		CHAR_DATA *vch;
		CHAR_DATA *vch_next;

		if (ch->level < MAX_LEVEL) {
		    char_puts("Not at your level! (Only for HIGH)\n",ch);
		    return;
		}

		for (vch = char_list; vch && !IS_NPC(vch); vch = vch_next) {
		    vch_next = vch->next;

		    if (vch->level < ch->level) {
			act_puts("$n forces you to '$t'.",
				 ch, argument, vch, TO_VICT, POS_DEAD);
			interpret_raw(vch, argument, TRUE);
		    }
		}
	} else if (!str_cmp(arg,"players")) {
	    CHAR_DATA *vch;
	    CHAR_DATA *vch_next;
	
	    if (ch->level < LEVEL_IMMORTAL) {
	        char_puts("Not at your level! (Only for Imm)\n",ch);
	        return;
	    }
	
	    for (vch = char_list; vch && !IS_NPC(vch); vch = vch_next) {
	        vch_next = vch->next;
	
	        if (vch->level < ch->level && !IS_IMMORTAL(vch)) {
			act_puts("$n forces you to '$t'.",
				 ch, argument, vch, TO_VICT, POS_DEAD);
	            interpret(vch, argument);
	        }
	    }
	} else if (!str_cmp(arg,"gods")) {
	    CHAR_DATA *vch;
	    CHAR_DATA *vch_next;
	
	    if (ch->level < MAX_LEVEL) {
	        char_puts("Not at your level! (Only for HIGH)\n",ch);
	        return;
	    }
	
	    for (vch = char_list; vch && !IS_NPC(vch); vch = vch_next) {
	        vch_next = vch->next;
	
	        if (vch->level < ch->level && IS_IMMORTAL(vch)) {
			act_puts("$n forces you to '$t'.",
				 ch, argument, vch, TO_VICT, POS_DEAD);
	            interpret(vch, argument);
	        }
	    }
	} else {
		CHAR_DATA *victim;

		if ((victim = get_char_world(ch, arg)) == NULL)
		{
		    char_puts("They aren't here.\n", ch);
		    return;
		}

		if (victim == ch)
		{
		    char_puts("Aye aye, right away!\n", ch);
		    return;
		}

		if (!is_room_owner(ch,victim->in_room) 
		&&  ch->in_room != victim->in_room 
	    &&  room_is_private(victim->in_room) && !IS_TRUSTED(ch,IMPLEMENTOR))
		{
	        char_puts("That character is in a private room.\n",ch);
	        return;
	    }

		if (!IS_NPC(victim)) {
			if (victim->level >= ch->level) {
				char_puts("Victim has more big level that you!\n", ch);
				return;
			}

/*			if (ch->level < MAX_LEVEL -3) {
 *				char_puts("Not at your level!\n",ch);
 *				return;
 *			}
 */
		}

		act_puts("$n forces you to '$t'.",
			 ch, argument, victim, TO_VICT, POS_DEAD);
		interpret(victim, argument);
	}

	char_puts("Ok.\n", ch);
}

/*
 * New routines by Dionysos.
 */
void do_invis(CHAR_DATA *ch, const char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];

	if (IS_NPC(ch))
		return;
	/* RT code for taking a level argument */
	one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0') 
	/* take the default path */

	  if (ch->pcdata->invis_level)
	  {
		  ch->pcdata->invis_level = 0;
		  act("$n slowly fades into existence.", ch, NULL, NULL, TO_ROOM);
		  char_puts("You slowly fade back into existence.\n", ch);
	  }
	  else
	  {       if (ch->level >= LEVEL_IMMORTAL)
		  	ch->pcdata->invis_level = LEVEL_IMMORTAL;
		  else
		  	ch->pcdata->invis_level = ch->level;

		  act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
		  char_puts("You slowly vanish into thin air.\n", ch);
	  }
	else
	/* do the level thing */
	{
	  level = atoi(arg);
	  if (level < 2 || level > ch->level)
	  {
		char_puts("Invis level must be between 2 and your level.\n",ch);
	    return;
	  }
	  else
	  {
		  ch->reply = NULL;
	      ch->pcdata->invis_level = level;
	      act("$n slowly fades into thin air.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You slowly vanish into thin air.\n", ch);
	  }
	}
}

void do_incognito(CHAR_DATA *ch, const char *argument)
{
	int level;
	char arg[MAX_STRING_LENGTH];
	
	if (IS_NPC(ch))
		return;
	/* RT code for taking a level argument */
	one_argument(argument, arg, sizeof(arg));
	
	if (arg[0] == '\0')
	/* take the default path */
	
	  if (ch->pcdata->incog_level)
	  {
	      ch->pcdata->incog_level = 0;
	      act("$n is no longer cloaked.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You are no longer cloaked.\n", ch);
	  }
	  else
	  {   if (ch->level >= LEVEL_IMMORTAL)
	      	ch->pcdata->incog_level = LEVEL_IMMORTAL;
	      else 
	      	ch->pcdata->incog_level = ch->level;

	      act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You cloak your presence.\n", ch);
	  }
	else
	/* do the level thing */
	{
	  level = atoi(arg);
	  if (level < 2 || level > ch->level)
	  {
	    char_puts("Incog level must be between 2 and your level.\n",ch);
	    return;
	  }
	  else
	  {
	      ch->reply = NULL;
	      ch->pcdata->incog_level = level;
	      act("$n cloaks $s presence.", ch, NULL, NULL, TO_ROOM);
	      char_puts("You cloak your presence.\n", ch);
	  }
	}
}

void do_holylight(CHAR_DATA *ch, const char *argument)
{
	if (IS_NPC(ch))
		return;

	TOGGLE_BIT(ch->pcdata->plr_flags, PLR_HOLYLIGHT);
	char_printf(ch, "Holy light mode %s.\n",
		    IS_SET(ch->pcdata->plr_flags, PLR_HOLYLIGHT) ? "on" : "off");
}

/* prefix command: it will put the string typed on each line typed */

DO_FUN(do_prefi)
{
	char_puts("You cannot abbreviate the prefix command.\n", ch);
}

DO_FUN(do_prefix)
{
	if (IS_NPC(ch))
		return;

	if (argument[0] == '\0') {
		if (ch->pcdata->prefix[0] == '\0') {
			char_puts("You have no prefix to clear.\n",ch);
			return;
		}

		char_puts("Prefix removed.\n",ch);
		free_string(ch->pcdata->prefix);
		ch->pcdata->prefix = str_empty;
		return;
	}

	free_string(ch->pcdata->prefix);
	ch->pcdata->prefix = str_dup(argument);
	char_printf(ch, "Prefix set to '%s'.\n", argument);
}

void advance(CHAR_DATA *victim, int level)
{
	int iLevel;

/*	int tra;
 *	int pra;
 *
 *	tra = victim->train;
 *	pra = victim->practice;
 */
	victim->pcdata->plevels = 0;

	/*
	 * Lower level:
	 *   Reset to level 1.
	 *   Then raise again.
	 *   Currently, an imp can lower another imp.
	 *   -- Swiftest
	 */
	if (level <= victim->level) {
		int temp_prac;

		char_puts("**** OOOOHHHHHHHHHH  NNNNOOOO ****\n", victim);
		temp_prac = victim->pcdata->practice;
		victim->level		= 1;
		victim->pcdata->exp		= base_exp(victim);
		victim->max_hit		= 10;
		victim->max_mana	= 100;
		victim->max_move	= 100;
		victim->pcdata->practice	= 0;
		victim->hit		= victim->max_hit;
		victim->mana		= victim->max_mana;
		victim->move		= victim->max_move;
		victim->pcdata->perm_hit	= victim->max_hit;
		victim->pcdata->perm_mana	= victim->max_mana;
		victim->pcdata->perm_move	= victim->max_move;
		advance_level(victim, FALSE);
		victim->pcdata->practice	= temp_prac;
	}
	else 
		char_puts("**** OOOOHHHHHHHHHH  YYYYEEEESSS ****\n", victim);

	for (iLevel = victim->level; iLevel < level; iLevel++) {
		char_puts("{CYou raise a level!!{x ", victim);
		victim->pcdata->exp += exp_to_level(victim);
		victim->level++;
		advance_level(victim, FALSE);
	}
	victim->pcdata->exp_tl		= 0;
	
/*	victim->train		= tra;
 *	victim->practice	= pra;
 */
	
	save_char_obj(victim, FALSE);
}

void do_advance(CHAR_DATA *ch, const char *argument)
{
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int level;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0' || arg2[0] == '\0' || !is_number(arg2)) {
		char_puts("Syntax: advance <char> <level>.\n", ch);
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL) {
		char_puts("That player is not here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}

	if ((level = atoi(arg2)) < 1 || level > MAX_LEVEL) {
		char_puts("Level must be 1 to 100.\n", ch);
		return;
	}

	if (level > ch->level || victim->level >= ch->level) {
		char_puts("Limited to your level.\n", ch);
		return;
	}

	advance(victim, level);
}

void do_mset(CHAR_DATA *ch, const char *argument)
{
	char arg1 [MAX_INPUT_LENGTH];
	char arg2 [MAX_INPUT_LENGTH];
	char arg3 [MAX_INPUT_LENGTH];
	char arg4 [MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	int value, val2/*, i*/;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	argument = one_argument(argument, arg3, sizeof(arg3));
		   one_argument(argument, arg4, sizeof(arg4));

	if (arg1[0] == '\0' || arg2[0] == '\0' || arg3[0] == '\0') {
		char_puts("Syntax:\n",ch);
		char_puts("  set char <name> <field> {{<value>|-}\n",ch); 
		char_puts("  Field being one of:\n",			ch);
		char_puts("    str int wis dex con cha sex class level\n",ch);
		char_puts("    race gold hp mana move practice align\n", ch);
		char_puts("    train thirst drunk full hometown ethos\n", ch);
		char_puts("    noghost clan security\n", ch);
		char_puts("    questp questt relig bloodlust desire \n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	/*
	 * Snarf the value (which need not be numeric).
	 */
	value = is_number(arg3) ? atoi(arg3) : -1;
	val2  = is_number(arg4) ? atoi(arg4) : -1;

	/*
	 * Set something.
	 */
	if (!str_cmp(arg2, "str")) {
		if (value < 3 || value > get_max_train(victim,STAT_STR))
		{
		    char_printf(ch,
			"Strength range is 3 to %d\n.",
			get_max_train(victim,STAT_STR));
		    return;
		}

		victim->perm_stat[STAT_STR] = value;
		return;
	}

/*	if (!str_cmp(arg2, "trouble")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			return;
		}
		
		if (value == -1 || val2 == -1) {
			char_puts("Usage: set char <name> trouble "
				  "<vnum> <value>.\n", ch);
			return;
		}

		qtrouble_set(victim, value, val2+1);
		char_puts("Ok.\n", ch);
		return;
	}
*/

	if (!str_cmp(arg2, "security"))	{ /* OLC */
		if (IS_NPC(ch)) {
			char_puts("Si, claro.\n", ch);
			return;
		}

		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			return;
		}

		if (value > ch->pcdata->security || value < 0) {
			if (ch->pcdata->security != 0)
				char_printf(ch, "Valid security is 0-%d.\n",
					    ch->pcdata->security);
			else
				char_puts("Valid security is 0 only.\n", ch);
			return;
		}
		victim->pcdata->security = value;
		return;
	}

	if (!str_cmp(arg2, "int"))
	{
	    if (value < 3 || value > get_max_train(victim,STAT_INT))
	    {
	        char_printf(ch, "Intelligence range is 3 to %d.\n",
			get_max_train(victim,STAT_INT));
	        return;
	    }
	
	    victim->perm_stat[STAT_INT] = value;
	    return;
	}

	if (!str_cmp(arg2, "wis"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_WIS))
		{
		    char_printf(ch,
			"Wisdom range is 3 to %d.\n",get_max_train(victim,STAT_WIS));
		    return;
		}

		victim->perm_stat[STAT_WIS] = value;
		return;
	}
	if (!str_cmp(arg2, "questp"))
	{
		 if (value == -1) value = 0;
		 if (!IS_NPC(victim)) victim->pcdata->questpoints = value;
		return;
	}
	if (!str_cmp(arg2, "questt"))
	{
		 if (value == -1)
		 	value = 30;
		 if (IS_PC(victim) && victim->pcdata->quest)
		 	victim->pcdata->quest->time = value;
		return;
	}
	if (!str_cmp(arg2, "relig"))
	{
		if (IS_NPC(victim)) {
			char_puts("Not on NPC's.\n", ch);
			return;
		}
		if (!RELIGION(value) || !value) {
			char_puts("Unknown religion.\n", ch);
			return;
		}
		free_religion_char(victim->pcdata->religion);
		victim->pcdata->religion = new_religion_char();
		victim->pcdata->religion->data = RELIGION(value);
		return;
	}


	if (!str_cmp(arg2, "dex"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_DEX))
		{
		    char_printf(ch,
			"Dexterity ranges is 3 to %d.\n",
			get_max_train(victim,STAT_DEX));
		    return;
		}

		victim->perm_stat[STAT_DEX] = value;
		return;
	}

	if (!str_cmp(arg2, "con"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CON))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n",
			get_max_train(victim,STAT_CON));
		    return;
		}

		victim->perm_stat[STAT_CON] = value;
		return;
	}
	if (!str_cmp(arg2, "cha"))
	{
		if (value < 3 || value > get_max_train(victim,STAT_CHA))
		{
		    char_printf(ch,
			"Constitution range is 3 to %d.\n",
			get_max_train(victim,STAT_CHA));
		    return;
		}

		victim->perm_stat[STAT_CHA] = value;
		return;
	}

	if (!str_prefix(arg2, "sex"))
	{
		if (value < 0 || value > 2)
		{
		    char_puts("Sex range is 0 to 2.\n", ch);
		    return;
		}
/*		if ((victim->class == 0) || (victim->class == 8))
		{
		    char_puts("You can't change their sex.\n", ch);
		    return;
		}
		переделать с resctrict sex !!!
 */
		victim->sex = value;
		if (!IS_NPC(victim))
		    victim->pcdata->true_sex = value;
		return;
	}

	if (!str_prefix(arg2, "class")) {
		int cl;

		if (IS_NPC(victim)) {
			char_puts("Mobiles have no class.\n",ch);
			return;
		}

		cl = cn_lookup(arg3);
		if (cl <= 0) {
			BUFFER *output;

			output = buf_new(-1);

			buf_add(output, "Possible classes are: ");
	    		for (cl = 1; cl < classes.nused; cl++) {
	        		if (cl > 1)
	                		buf_add(output, " ");
	        		buf_add(output, CLASS(cl)->name);
	    		}
	        	buf_add(output, ".\n");

			send_to_char(buf_string(output), ch);
			buf_free(output);
			return;
		}

		victim->class = cl;
		victim->pcdata->exp = exp_for_level(victim, victim->level);
		victim->pcdata->exp_tl = 0;
		update_skills(victim);
		return;
	}

	if (!str_prefix(arg2, "level"))
	{
		if (!IS_NPC(victim))
		{
		    char_puts("Not on PC's.\n", ch);
		    return;
		}

		if (value < 0 || value > 100)
		{
		    char_puts("Level range is 0 to 100.\n", ch);
		    return;
		}
		victim->level = value;
		return;
	}

	if (!str_prefix(arg2, "gold"))
	{
		victim->gold = value;
		return;
	}

	if (!str_prefix(arg2, "hp"))
	{
		if (value < -10 || value > 30000)
		{
		    char_puts("Hp range is -10 to 30,000 hit points.\n", ch);
		    return;
		}
		victim->max_hit = value;
	    if (!IS_NPC(victim))
	        victim->pcdata->perm_hit = value;
		return;
	}

	if (!str_prefix(arg2, "mana"))
	{
		if (value < 0 || value > 60000)
		{
		    char_puts("Mana range is 0 to 60,000 mana points.\n", ch);
		    return;
		}
		victim->max_mana = value;
	    if (!IS_NPC(victim))
	        victim->pcdata->perm_mana = value;
		return;
	}

	if (!str_prefix(arg2, "move")) {
		if (value < 0 || value > 60000) {
			char_puts("Move range is 0 to 60,000 move points.\n", ch);
			return;
		}
		victim->max_move = value;

		if (!IS_NPC(victim))
		        victim->pcdata->perm_move = value;
		return;
	}

	if (!str_prefix(arg2, "practice")) {
		if (IS_NPC(victim)) {
			char_puts("Mobiles have no practice.\n",ch);
			return;
		}

		if (value < 0 || value > 250) {
		    char_puts("Practice range is 0 to 250 sessions.\n", ch);
		    return;
		}
		victim->pcdata->practice = value;
		return;
	}

	if (!str_prefix(arg2, "train")) {
		if (IS_NPC(victim)) {
			char_puts("Mobiles have no train.\n",ch);
			return;
		}

		if (value < 0 || value > 50) {
			char_puts("Training session range is 0 to 50 sessions.\n", ch);
			return;
		}
		victim->pcdata->train = value;
		return;
	}

	if (!str_prefix(arg2, "align")) {
		if (value < -1000 || value > 1000 || value == -1) {
			char_puts("Alignment range is -1000 to 1000.\n", ch);
			return;
		}
		victim->alignment = value;
		char_puts("Align set.\n", ch);
		char_puts("Remember to check their hometown.\n", ch);
		return;
	}

	if (!str_prefix(arg2, "ethos")) {
		int ethos;

		if (IS_NPC(victim)) {
			char_puts("Mobiles don't have an ethos.\n", ch);
			return;
		}

		ethos = flag_value(ethos_table, arg3);
		if (ethos < 0) {
			char_puts("%s: Unknown ethos.\n", ch);
			char_puts("Valid ethos types are:\n", ch);
			show_flags(ch, ethos_table);
			return;
		}

		victim->pcdata->ethos = ethos;
		return;
	}

	if (!str_prefix(arg2, "hometown"))
	{
	    if (IS_NPC(victim))
		{
		    char_puts("Mobiles don't have hometowns.\n", ch);
		    return;
		}
	    if (value < 0 || value > 4)
	    { 
	        char_puts("Please choose one of the following :.\n", ch);
	        char_puts("Town        Alignment       Value\n", ch);
	        char_puts("----        ---------       -----\n", ch);
	        char_puts("Midgaard     Any              0\n", ch);
	        char_puts("New Thalos   Any              1\n", ch);
	        char_puts("Titan        Any              2\n", ch);
	        char_puts("Ofcol        Neutral          3\n", ch);
	        char_puts("Old Midgaard Evil             4\n", ch);
	        return;
	    }

	    if ((value == 2 && !IS_GOOD(victim)) || (value == 3 &&
		!IS_NEUTRAL(victim)) || (value == 4 && !IS_EVIL(victim)))
	    { 
	        char_puts("The hometown doesn't match this character's alignment.\n", ch);
	        return;
	    }    
	    
	    victim->pcdata->hometown = value;
	    return;
	}

	if (!str_prefix(arg2, "thirst"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Thirst range is -1 to 100.\n", ch);
		    return;
		}

		victim->pcdata->condition[COND_THIRST] = value;
		return;
	}

	if (!str_prefix(arg2, "drunk"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Drunk range is -1 to 100.\n", ch);
		    return;
		}

		victim->pcdata->condition[COND_DRUNK] = value;
		return;
	}

	if (!str_prefix(arg2, "full"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    return;
		}

		victim->pcdata->condition[COND_FULL] = value;
		return;
	}

	if (!str_prefix(arg2, "hunger"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Hunger range is -1 to 100.\n", ch);
		    return;
		}

		victim->pcdata->condition[COND_HUNGER] = value;
		return;
	}

	if (!str_prefix(arg2, "bloodlust"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    return;
		}

		victim->pcdata->condition[COND_BLOODLUST] = value;
		return;
	}

	if (!str_prefix(arg2, "desire"))
	{
		if (IS_NPC(victim))
		{
		    char_puts("Not on NPC's.\n", ch);
		    return;
		}

		if (value < -1 || value > 100)
		{
		    char_puts("Full range is -1 to 100.\n", ch);
		    return;
		}

		victim->pcdata->condition[COND_DESIRE] = value;
		return;
	}

	if (!str_prefix(arg2, "race")) {
		int race;

		race = rn_lookup(arg3);

		if (race == 0) {
			char_puts("That is not a valid race.\n",ch);
			return;
		}

		if (!IS_NPC(victim) && !RACE(race)->pcdata) {
			char_puts("That is not a valid player race.\n",ch);
			return;
		}

		victim->race = race;
		SET_ORG_RACE(victim, race);
		update_skills(victim);
		if (!IS_NPC(victim)) {
			victim->pcdata->exp = exp_for_level(victim, victim->level);
			victim->pcdata->exp_tl = 0;
		}
		return;
	}

	if (!str_prefix(arg2, "noghost")) {
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}
		remove_ghost(victim);
		char_puts("Ok.\n", ch);
		return;
	}

	/*
	if (!str_prefix(arg2, "clan")) {
		int cn;

		char_puts("Clans are under construction !\n", ch);
		return;
		
		if (IS_NPC(victim)) {
			char_puts("Not on NPC.\n", ch);
			return;
		}

		if ((cn = cln_lookup(arg3)) < 0) {
			char_puts("Incorrect clan name.\n", ch);
			return;
		}

		if (cn != victim->clan) {
			clan_t *clan;

			if ((clan = clan_lookup(victim->clan)) &&
			(victim->clan)) {
				clan_update_lists(clan, victim, TRUE);
				clan_save(clan);
			}

			for (i = 0; i < clan->skills.nused; i++) {
                        	clskill_t *cs = VARR_GET(&clan->skills, i);
                        	set_skill_raw(victim, cs->sn, 0, TRUE, cs->level, 100, 50);
                        }

			victim->clan = cn;
			victim->pcdata->clan_status = CLAN_COMMONER;

			if (cn) {
				clan = CLAN(cn);
				name_add(&clan->member_list, victim->name,
					 NULL, NULL);
				clan_save(clan);
			}

			update_skills(victim);
		}

		char_puts("Ok.\n", ch);
		return;
	}
	*/

	/*
	 * Generate usage message.
	 */
	do_mset(ch, str_empty);
}


void do_desocket(CHAR_DATA *ch, const char *argument)
{
	DESCRIPTOR_DATA_MUDDY *d;
	int socket;
	char arg[MAX_INPUT_LENGTH];

	one_argument(argument, arg, sizeof(arg));

	if (!is_number(arg))

	{
	  char_puts("The argument must be a number.\n", ch);
	  return;
	}

	if (arg[0] == '\0')
	{
	  char_puts("Disconnect which socket?\n", ch);
	  return;
	}

	else
	{
	  socket = atoi(arg);
	  for (d = descriptor_list_muddy; d; d = d->next)
		{
		  if (get_idsock_connect(d->client) == socket)
		    {
		      if (d->character == ch)
			{
			  char_puts("It would be foolish to disconnect yourself.\n", ch);
			  return;
			}
		      if (d->connected == CON_PLAYING)
			{
			  char_puts("Why don't you just use disconnect?\n", ch);
			  return;
			}
		      //write_to_descriptor_muddy(d,
		      write_to_buffer_muddy(d,
			  "You are being disconnected by an immortal.",
					  0);
		      close_descriptor_muddy(d);
		      char_puts("Done.\n", ch);
		      return;
		    }
		}
	  char_puts("No such socket is connected.\n", ch);
	  return;
	}
}

void do_smite(CHAR_DATA *ch, const char *argument)
{
	CHAR_DATA *victim;

	if (argument[0] == '\0') {
	  char_puts("You are so frustrated you smite yourself!  OWW!\n", 
			ch);
	  return;
	}

	if ((victim = get_char_world(ch, argument)) == NULL) {
	  char_puts("You'll have to smite them some other day.\n", ch);
	  return;
	}

	if (IS_NPC(victim)) {
	  char_puts("That poor mob never did anything to you.\n", ch);
	  return;
	}

	if (victim->level > ch->level) {
	  char_puts("How dare you!\n", ch);
	  return;
	}

	if (victim->position < POS_SLEEPING) {
	  char_puts("Take pity on the poor thing.\n", ch);
	  return;
	}

	act("A bolt comes down out of the heavens and smites you!", victim, NULL,
		ch, TO_CHAR);
	act("You reach down and smite $n!", victim, NULL, ch, TO_VICT);
	act("A bolt from the heavens smites $n!", victim, NULL, ch, TO_NOTVICT);
	victim->hit = victim->hit / 2;
	return;
}

void do_popularity(CHAR_DATA *ch, const char *argument)
{
	BUFFER *output;
	AREA_DATA *area;
	extern AREA_DATA *area_first;
	int i;

	output = buf_new(-1);
	buf_add(output, "Area popularity statistics (in char * ticks)\n");

	for (area = area_first,i=0; area != NULL; area = area->next,i++) {
		if (i % 2 == 0) 
			buf_add(output, "\n");
		buf_printf(output, "%-20s %-8lu       ",
			   area->name, area->count);
	}
	buf_add(output, "\n\n");
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

void do_ititle(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0')  {
		char_puts("Change whose title to what?\n", ch);
		return;
	}

	victim = get_char_world(ch, arg);
	if (victim == NULL)  {
		char_puts("Nobody is playing with that name.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC's.\n", ch);
		return;
	}
	
	if (victim->level > ch->level){
		char_puts("You don't power for ititle him(her).\n", ch);
		return;
	}

	if (argument[0] == '\0') {
		char_puts("Change the title to what?\n", ch);
		return;
	}

	set_title(victim, argument);
	char_puts("Ok.\n", ch);
}

/*
 * .gz files are checked for too, just in case.
 */

void do_rename(CHAR_DATA* ch, const char *argument)
{
	char old_name[MAX_INPUT_LENGTH], 
	     new_name[MAX_INPUT_LENGTH];
	char strsave[PATH_MAX];
	char *file_name;

	CHAR_DATA *victim;
	//clan_t *clan;
		
	argument = first_arg(argument, old_name, sizeof(old_name), FALSE); 
		   first_arg(argument, new_name, sizeof(new_name), FALSE);
		
	if (!old_name[0]) {
		char_puts("Rename who?\n",ch);
		return;
	}
		
	victim = get_char_world(ch, old_name);
		
	if (!victim) {
		char_puts("There is no such a person online.\n",ch);
		return;
	}
		
	if (IS_NPC(victim)) {   
		char_puts("You cannot use Rename on NPCs.\n",ch);
		return;
	}

	if (victim != ch && victim->level >= ch->level) {
		char_puts("You failed.\n",ch);
		return;
	}
		
	if (!victim->desc || (victim->desc->connected != CON_PLAYING)) {
		char_puts("This player has lost his link or is inside a pager or the like.\n",ch);
		return;
	}

	if (!new_name[0]) {
		char_puts("Rename to what new name?\n",ch);
		return;
	}
		
	if (!pc_name_ok(new_name)) {
		char_puts("The new name is illegal.\n",ch);
		return;
	}

	/*
	if (victim->clan && (clan = clan_lookup(victim->clan))) {
		bool touched = FALSE;

		if (name_delete(&clan->member_list, old_name, NULL, NULL)) {
			touched = TRUE;
			name_add(&clan->member_list, new_name, NULL, NULL);
		}
		if (name_delete(&clan->leader_list, old_name, NULL, NULL)) {
			touched = TRUE;
			name_add(&clan->leader_list, new_name, NULL, NULL);
		}
		if (name_delete(&clan->second_list, old_name, NULL, NULL)) {
			touched = TRUE;
			name_add(&clan->second_list, new_name, NULL, NULL);
		}
		if (touched)
			clan_save(clan);
	}
	*/

/* delete old pfile */
	if (str_cmp(new_name, old_name)) {
		DESCRIPTOR_DATA_MUDDY *d;
		OBJ_DATA *obj;

		for (d = descriptor_list_muddy; d; d = d->next)
			if (d->character
			&&  !str_cmp(d->character->name, new_name)) {
				char_puts ("A player with the name you specified already exists!\n",ch);
				return;
			}

		/* check pfile */
		file_name = capitalize(new_name);
		if (dfexist(PLAYER_PATH, file_name)) {
			char_puts("A player with that name already exists!\n",
				  ch);
			return;		
		}

		/* check .gz pfile */
		snprintf(strsave, sizeof(strsave), "%s.gz", file_name);
		if (dfexist(PLAYER_PATH, strsave)) {
			char_puts ("A player with that name already exists in a compressed file!\n",ch);
			return;		
		}

		/* change object owners */
		for (obj = object_list; obj; obj = obj->next)
			if (obj->owner
			&&  !str_cmp(mlstr_mval(obj->owner), old_name)) {
				mlstr_free(obj->owner);
				obj->owner = mlstr_new(new_name);
			}

		dunlink(PLAYER_PATH, capitalize(old_name)); 
	}
/*
 * NOTE: Players who are level 1 do NOT get saved under a new name 
 */
	free_string(victim->name);
	victim->name = str_dup(new_name);
	mlstr_free(victim->short_descr);
	victim->short_descr = mlstr_new(new_name);
	save_char_obj(victim, FALSE);
		
	char_puts("Character renamed.\n", ch);
	act_puts("$n has renamed you to $N!",
		 ch, NULL, victim, TO_VICT, POS_DEAD);

	if (IS_SET(victim->pcdata->otherf, OTHERF_RENAME)) {
		char_printf(ch, "Rename flag removed.\n", victim->name);
		REMOVE_BIT(victim->pcdata->otherf, OTHERF_RENAME);
	}


} 

void do_notitle(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!IS_IMMORTAL(ch))
	    return;

	argument = one_argument(argument, arg, sizeof(arg));

	if ((victim = get_char_world(ch ,arg)) == NULL) {
		char_puts("He is not currently playing.\n", ch);
		return;
	}
	if (IS_NPC(victim))
		return;
	 
	TOGGLE_BIT(victim->pcdata->plr_flags, PLR_NOTITLE);
	if (!IS_SET(victim->pcdata->plr_flags, PLR_NOTITLE))
	 	char_puts("You can change your title again.\n", victim);
	else 
		char_puts("You won't be able to change your title anymore.\n",
			  victim);
	char_puts("Ok.\n", ch);
}
	   
void do_noaffect(CHAR_DATA *ch, const char *argument)
{
	AFFECT_DATA *paf,*paf_next;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	if (!IS_IMMORTAL(ch))
		return;

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Noaff whom?\n", ch);
		return;
	}

	if ((victim = get_char_world(ch, arg)) == NULL) {
		char_puts("He is not currently playing.\n", ch);
		return;
	}
	 
	for (paf = victim->affected; paf != NULL; paf = paf_next) {
		paf_next = paf->next;
		if (paf->duration >= 0 || paf->type == gsn_wrath_of_gods) {
			skill_t *sk;

			if ((sk = skill_lookup(paf->type))
			&&  !mlstr_null(sk->msg_off))
				act_puts3_mlstr(sk->msg_off, victim,
					NULL, NULL, NULL, TO_CHAR, POS_RESTING);
		  
			affect_remove(victim, paf);
		}
	}
}

void do_affrooms(CHAR_DATA *ch, const char *argument)
{
	ROOM_INDEX_DATA *room;
	ROOM_INDEX_DATA *room_next;
	int count = 0;

	if (!top_affected_room) 
		char_puts("No affected room.\n",ch);

	for (room = top_affected_room; room ; room = room_next) {
		room_next = room->aff_next;
		count++;
		char_printf(ch, "%d) [Vnum : %5d] %s\n",
			count, room->vnum , mlstr_cval(room->name, ch));
	}
}

void do_find(CHAR_DATA *ch, const char *argument)
{
	char* path;
	ROOM_INDEX_DATA *location;

	if (argument[0] == '\0') {
		char_puts("Ok. But what I should find?\n", ch);
		return;
	}

	if ((location = find_location(ch, argument)) == NULL) {
		char_puts("No such location.\n", ch);
		return;
	}

	path = find_way(ch, ch->in_room, location);
	char_printf(ch, "%s.\n", path);
	log_printf("From %d to %d: %s.\n",
		   ch->in_room->vnum, location->vnum, path);
	return;
}

void do_reboot(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));    

	if (arg[0] == '\0') {
		char_puts("Usage: reboot now\n"
			  "Usage: reboot <ticks to reboot>\n"
			  "Usage: reboot cancel\n"
			  "Usage: reboot status\n", ch);
		return;
	}

	if (is_name(arg,"cancel")) {
		reboot_counter = -1;
		char_puts("Reboot canceled.\n", ch);
		return;
	}

	if (is_name(arg, "now")) {
		reboot_mud();
		return;
	}

	if (is_name(arg, "status")) {
		if (reboot_counter == -1) 
			char_printf(ch, "Automatic rebooting is inactive.\n");
		else
			char_printf(ch, "Reboot in %i minutes.\n",
				    reboot_counter);
		return;
	}

	if (is_number(arg)) {
		reboot_counter = atoi(arg);
		rebooter = 1;
		char_printf(ch, "Muddy will reboot in %i ticks.\n",
			    reboot_counter);
		return;
	}

	do_reboot(ch, "");   
}

void msg_dump(BUFFER *buf, mlstring *ml);
DO_FUN(do_msgstat)
{
	varr *v;
	mlstring **mlp;
	int i;
	BUFFER *output;

	if (argument[0] == '\0') {
		for (i = 0; i < MAX_MSG_HASH; i++) {
			varr *v = msg_hash_table+i;
			char_printf(ch, "%3d: %d msgs\n", i, v->nused);
		}
		return;
	}

	if (!is_number(argument)) {
		do_help(ch, "MSGSTAT");
		return;
	}

	i = atoi(argument);
	if (i < 0 || i >= MAX_MSG_HASH) {
		char_printf(ch, "Valid hash key range is 0..%d\n",
			    MAX_KEY_HASH);
		return;
	}

	v = msg_hash_table+i;
	output = buf_new(-1);
	buf_printf(output, "Dumping msgs with hash #%d\n", i);
	for (i = 0; i < v->nused; i++) {
		mlp = VARR_GET(v, i);
		msg_dump(output, *mlp);
		buf_add(output, "\n");
	}
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

extern int str_count;
extern int str_real_count; /* XXX */

DO_FUN(do_strstat)
{
	char_printf(ch, "Strings: %d\n"
			"Allocated: %d\n",
		    str_count, str_real_count);
}

DO_FUN(do_grant)
{
	cmd_t *cmd;
	char arg1[MAX_INPUT_LENGTH];
	char arg2[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	argument = one_argument(argument, arg1, sizeof(arg1));
	argument = one_argument(argument, arg2, sizeof(arg2));
	if (arg1[0] == '\0') {
		do_help(ch, "'WIZ GRANT'");
		return;
	}

	if ((victim = get_char_room(ch, arg1)) == NULL) {
		char_puts("They aren't here.\n", ch);
		return;
	}

	if (IS_NPC(victim)) {
		char_puts("Not on NPC.\n", ch);
		return;
	}

	if (arg2[0] == '\0') {
		char_printf(ch, "Granted commands for %s: [%s]\n",
			    victim->name, victim->pcdata->granted);
		return;
	}

	if (is_number(arg2)) {
		int lev = atoi(arg2);

		if (lev < LEVEL_IMMORTAL) {
			char_printf(ch, "grant: granted level must be at least %d\n", LEVEL_HERO);
			return;
		}

		if (lev > ch->level) {
			char_puts("grant: granted level cannot be higher"
				  " than yours.\n", ch);
			return;
		}

		for (cmd = cmd_table; cmd->name; cmd++) {
			if (cmd->level < LEVEL_HERO
			||  cmd->level > lev)
				continue;

			name_add(&victim->pcdata->granted, cmd->name,
				 ch, "grant");
		}

		return;
	}

	for (; arg2[0]; argument = one_argument(argument, arg2, sizeof(arg2))) {
		if ((cmd = cmd_lookup(arg2)) == NULL
		&&  str_cmp(arg2, "none")
		&&  str_cmp(arg2, "all")) {
			char_printf(ch, "%s: command not found.\n", arg2);
			continue;
		}

		if (cmd && cmd->level < LEVEL_IMMORTAL) {
			char_printf(ch, "%s: not a wizard command.\n", arg2);
			continue;
		}
		name_toggle(&victim->pcdata->granted, arg2, ch, "grant");
	}
}

DO_FUN(do_disable)
{
	cmd_t *cmd;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ ENABLE DISABLE'");
		return;
	}

	if (!str_cmp(arg, "?")) {
		char_puts("Disabled commands:\n", ch);
		for (cmd = cmd_table; cmd->name; cmd++)
			if (IS_SET(cmd->flags, CMD_DISABLED))
				char_printf(ch, "%s\n", cmd->name);
		return;
	}

	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if ((cmd = cmd_lookup(arg)) == NULL) {
			char_printf(ch, "%s: command not found.\n", arg);
			continue;
		}

		if (!str_cmp(cmd->name, "enable")) {
			char_puts("'enable' command cannot be disabled.\n",
				  ch);
			return;
		}

		SET_BIT(cmd->flags, CMD_DISABLED);
		char_printf(ch, "%s: command disabled.\n", cmd->name);
	}
}

DO_FUN(do_enable)
{
	cmd_t *cmd;
	char arg[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		do_help(ch, "'WIZ ENABLE DISABLE'");
		return;
	}

	for (; arg[0]; argument = one_argument(argument, arg, sizeof(arg))) {
		if ((cmd = cmd_lookup(arg)) == NULL) {
			char_printf(ch, "%s: command not found.\n", arg);
			continue;
		}

		REMOVE_BIT(cmd->flags, CMD_DISABLED);
		char_printf(ch, "%s: command enabled.\n", cmd->name);
	}
}

void do_check(CHAR_DATA *ch, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
	char arg[MAX_INPUT_LENGTH];
	BUFFER *buffer;
	CHAR_DATA *victim;
	int count = 1;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '\0'|| !str_prefix(arg,"stats"))
	{
		buffer = buf_new(ch->lang);
		for (victim = char_list; victim != NULL; victim = victim->next)
		{
			if (IS_NPC(victim) || !can_see(ch,victim)) 
			continue;
    	    	
			if (victim->desc == NULL)
			{
				sprintf(buf,"%3d) %s is linkdead.\n\r", count, 
						victim->name);
				buf_add(buffer, buf);
				count++;
				continue;	    	
			}

			if (victim->desc->connected >= CON_GET_NEW_RACE
			 && victim->desc->connected <= CON_PICK_WEAPON)
			{	
				sprintf(buf,"%3d) %s is being created.\n\r",
					count, victim->name);
				buf_add(buffer, buf);
				count++;
				continue;
			}
	    
			if ( (victim->desc->connected == CON_GET_OLD_PASSWORD
			|| victim->desc->connected >= CON_READ_IMOTD))
			{
				sprintf(buf,"%3d) %s is connecting.\n\r",
						count, victim->name);
				buf_add(buffer, buf);
				count++;
				continue; 	    		 
			}

			if (victim->desc->connected == CON_PLAYING)
			{
				sprintf(buf,
	"%3d) %s, Level %d connected since %d hours (%d total hours)\n\r", 
				 count, victim->name,
				 victim->level, ((int)(current_time 
				 - victim->pcdata->logon)) /3600, 
				 (victim->pcdata->played + (int)(current_time 
				 - victim->pcdata->logon)) /3600 );
				buf_add(buffer, buf);
				if (arg[0]!='\0' && !str_prefix(arg,
					"stats")) {
					sprintf(buf,
"   %5d HP %5d M (%2d %2d %2d %2d %2d) %3d Tr %3d Pr %4d golds %d Qpts\n\r",
					 victim->max_hit, 
					 victim->max_mana,
					 victim->perm_stat[STAT_STR], 
					 victim->perm_stat[STAT_INT],
					 victim->perm_stat[STAT_WIS], 
					 victim->perm_stat[STAT_DEX],
					 victim->perm_stat[STAT_CON], 
					 victim->pcdata->train, 
					 victim->pcdata->practice, 
					 victim->gold 
					 + victim->silver/100, 
					 victim->pcdata->questpoints);
					buf_add(buffer, buf); 
				} 
			count++; 
			continue; 
			} 
			
			sprintf(buf,
			"%3d) BUGGY (Oops!)...please report to Implemetators: "
					"%s %d\n\r",
	    			count, victim->name, victim->desc->connected);
			buf_add(buffer, buf); 
			count++;
		} 
	
		page_to_char(buf_string(buffer),ch); 
		buf_free(buffer); 
		return; 
	} 
	
	if (!str_prefix(arg,"eq")) { 
		buffer = buf_new(ch->lang); 
		for (victim = char_list; victim != NULL; 
				victim = victim->next) { 
			if (IS_NPC(victim) 
			|| !victim->desc
			|| victim->desc->connected != CON_PLAYING 
			|| !can_see(ch,victim)) 
				continue; 
			
			sprintf(buf,
"%3d) %-10s, %2d items(weight %4d) Hit:%2d, Dam:%2d, AC:%3d %3d %3d %3d, Sav:%d\n\r", 
				count, victim->name, victim->carry_number, 
				victim->carry_weight, victim->hitroll, 
				victim->damroll, 
				victim->armor[AC_PIERCE], 
				victim->armor[AC_BASH], 
				victim->armor[AC_SLASH], 
				victim->armor[AC_EXOTIC],
				get_char_svs(victim, 0, FALSE));
			buf_add(buffer, buf); 
			count++;  
		} 
		page_to_char(buf_string(buffer),ch); 
		buf_free(buffer);    	
		return; 
	} 
	
	if (!str_prefix(arg,"snoop")) { 
		char bufsnoop [100]; 
		
		if(ch->level < MAX_LEVEL ) { 
			char_puts("You can't use this check option.\n\r",ch);
			return; 
		} 
		buffer = buf_new(ch->lang); 
		
		for (victim = char_list; victim != NULL; 
				victim = victim->next) { 
			if (IS_NPC(victim) 
			|| !victim->desc
			|| victim->desc->connected != CON_PLAYING
			|| !can_see(ch,victim)) 
				continue; 
			
			if(victim->desc->snoop_by != NULL) 
				sprintf(bufsnoop," %15s .",
				 victim->desc->snoop_by->character->name); 
			else 
				sprintf(bufsnoop,"     (none)      ." ); 
			
			sprintf(buf,"%3d %15s : %s \n\r",count,victim->name, 
					bufsnoop); 
			buf_add(buffer, buf); 
			count++; 
		} 
		page_to_char(buf_string(buffer),ch); 
		buf_free(buffer); 
		return; 
	} 
	
	char_puts("Syntax: 'check'       display info about players\n\r",ch);
	char_puts("        'check stats' display info and resume stats\n\r",ch); 
	char_puts("        'check eq'    resume eq of all players\n\r",ch); 
	char_puts("        'check snoop' display info about snoops\n\r",ch);
	char_puts("Use the stat command in case of doubt about someone...\n\r",
			ch); 
	return; 
}
