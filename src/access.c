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

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#if	!defined (WIN32)
#include <unistd.h>
#endif
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "merc.h"
#include "access.h"
#include "db.h"
#include "fight.h"

ban_t*	new_ban	(void);
void	free_ban(ban_t *ban);

struct ban_t
{
	ban_t*		next;
	flag32_t	ban_flags;
	int		level;
	const char*	name;
};

ban_t *ban_list;
ban_t *ban_free;

ban_t *new_ban(void)
{
    static ban_t ban_zero;
    ban_t *ban;

    if (ban_free == NULL)
	ban = alloc_perm(sizeof(*ban));
    else
    {
	ban = ban_free;
	ban_free = ban_free->next;
    }

    *ban = ban_zero;
    ban->name = str_empty;
    return ban;
}

void free_ban(ban_t *ban)
{
    free_string(ban->name);

    ban->next = ban_free;
    ban_free = ban;
}

void save_bans(void)
{
    ban_t *pban;
    FILE *fp;
    bool found = FALSE;

    if ((fp = dfopen(ETC_PATH, BAN_FILE, "w")) == NULL)
		return;

    for (pban = ban_list; pban != NULL; pban = pban->next)
    {
	if (IS_SET(pban->ban_flags,BAN_PERMANENT))
	{
	    found = TRUE;
	    log_printf("%-20s %-2d %s\n", pban->name, pban->level,
			format_flags(pban->ban_flags));
	    fprintf(fp,"%-20s %-2d %s\n",pban->name,pban->level,
		format_flags(pban->ban_flags));
	}
     }

     fclose(fp);

     if (!found)
	dunlink(ETC_PATH, BAN_FILE);
}

void load_bans(void)
{
    FILE *fp;
    ban_t *ban_last;
 
	if (!dfexist(ETC_PATH, BAN_FILE))
		return;

    if ((fp = dfopen(ETC_PATH, BAN_FILE, "r")) == NULL)
        return;
 
    ban_last = NULL;
    for (; ;)
    {
        ban_t *pban;
        if (feof(fp))
        {
            fclose(fp);
            return;
        }
 
        pban = new_ban();
 
        pban->name = str_dup(fread_word(fp));
	pban->level = fread_number(fp);
	pban->ban_flags = fread_flags(fp);
	fread_to_eol(fp);

        if (ban_list == NULL)
	    ban_list = pban;
	else
	    ban_last->next = pban;
	ban_last = pban;
    }
}

bool check_ban(const char *site, int type)
{
    ban_t *pban;
    char host[MAX_STRING_LENGTH];

    strnzcpy(host, sizeof(host), capitalize(site));
    host[0] = LOWER(host[0]);

    for (pban = ban_list; pban != NULL; pban = pban->next) 
    {
	if(!IS_SET(pban->ban_flags,type))
	    continue;

	if (!IS_SET(pban->ban_flags,BAN_PREFIX|BAN_SUFFIX)
	&& !str_cmp(pban->name,host))
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_PREFIX) 
	&&  IS_SET(pban->ban_flags,BAN_SUFFIX)  
	&&  strstr(pban->name,host) != NULL)
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_PREFIX)
	&&  !str_suffix(pban->name,host))
	    return TRUE;

	if (IS_SET(pban->ban_flags,BAN_SUFFIX)
	&&  !str_prefix(pban->name,host))
	    return TRUE;
    }

    return FALSE;
}


void ban_site(CHAR_DATA *ch, const char *argument, bool fPerm)
{
    char arg1[MAX_INPUT_LENGTH], arg2[MAX_INPUT_LENGTH];
    char *name;
    BUFFER *buffer;
    ban_t *pban, *prev;
    bool prefix = FALSE,suffix = FALSE;
    int type;

    argument = one_argument(argument, arg1, sizeof(arg1));
    argument = one_argument(argument, arg2, sizeof(arg2));

	if (arg1[0] == '\0') {
		if (ban_list == NULL) {
			char_puts("No sites banned at this time.\n",ch);
			return;
  		}

		buffer = buf_new(-1);

        	buf_add(buffer, "Banned sites  level  type     status\n");
		for (pban = ban_list;pban != NULL;pban = pban->next) {
			char buf2[MAX_STRING_LENGTH];

			snprintf(buf2, sizeof(buf2), "%s%s%s",
				IS_SET(pban->ban_flags,BAN_PREFIX) ? "*" : str_empty,
				pban->name,
				IS_SET(pban->ban_flags,BAN_SUFFIX) ? "*" : str_empty);

			buf_printf(buffer,"%-12s    %-3d  %-7s  %s\n",
				buf2, pban->level,
				IS_SET(pban->ban_flags,BAN_NEWBIES) ?
					"newbies" :
				IS_SET(pban->ban_flags,BAN_PLAYER)  ?
					"player" :
				IS_SET(pban->ban_flags,BAN_PERMIT)  ?
					"permit" :
				IS_SET(pban->ban_flags,BAN_ALL)     ?
					"all"	: str_empty,
	    			IS_SET(pban->ban_flags,BAN_PERMANENT) ?
					"perm" : "temp");
		}

        	page_to_char(buf_string(buffer), ch);
		buf_free(buffer);
        	return;
	}

    /* find out what type of ban */
    if (arg2[0] == '\0' || !str_prefix(arg2,"all"))
	type = BAN_ALL;
    else if (!str_prefix(arg2,"newbies"))
	type = BAN_NEWBIES;
    else if (!str_prefix(arg2,"player"))
	type = BAN_PLAYER;
    else if (!str_prefix(arg2,"permit"))
	type = BAN_PERMIT;
    else
    {
	char_puts("Acceptable ban types are all, newbies, player, and permit.\n",
	    ch); 
	return;
    }

    name = arg1;

    if (name[0] == '*')
    {
	prefix = TRUE;
	name++;
    }

    if (name[strlen(name) - 1] == '*')
    {
	suffix = TRUE;
	name[strlen(name) - 1] = '\0';
    }

    if (strlen(name) == 0)
    {
	char_puts("You have to ban SOMETHING.\n",ch);
	return;
    }

    prev = NULL;
    for (pban = ban_list; pban != NULL; prev = pban, pban = pban->next)
    {
        if (!str_cmp(name,pban->name))
        {
	    if (pban->level > ch->level)
	    {
            	char_puts("That ban was set by a higher power.\n", ch);
            	return;
	    }
	    else
	    {
		if (prev == NULL)
		    ban_list = pban->next;
		else
		    prev->next = pban->next;
		free_ban(pban);
	    }
        }
    }

    pban = new_ban();
    pban->name = str_dup(name);
    pban->level = ch->level;

    /* set ban type */
    pban->ban_flags = type;

    if (prefix)
	SET_BIT(pban->ban_flags,BAN_PREFIX);
    if (suffix)
	SET_BIT(pban->ban_flags,BAN_SUFFIX);
    if (fPerm)
	SET_BIT(pban->ban_flags,BAN_PERMANENT);

    pban->next  = ban_list;
    ban_list    = pban;
    save_bans();
    char_printf(ch, "%s has been banned.\n",pban->name);
    return;
}

void do_ban(CHAR_DATA *ch, const char *argument)
{
    ban_site(ch,argument,FALSE);
}

void do_permban(CHAR_DATA *ch, const char *argument)
{
    ban_site(ch,argument,TRUE);
}

void do_allow(CHAR_DATA *ch, const char *argument)                        
{
    char arg[MAX_INPUT_LENGTH];
    ban_t *prev;
    ban_t *curr;

    one_argument(argument, arg, sizeof(arg));

    if (arg[0] == '\0')
    {
        char_puts("Remove which site from the ban list?\n", ch);
        return;
    }

    prev = NULL;
    for (curr = ban_list; curr != NULL; prev = curr, curr = curr->next)
    {
        if (!str_cmp(arg, curr->name))
        {
	    if (curr->level > ch->level)
	    {
		char_puts(
		   "You are not powerful enough to lift that ban.\n",ch);
		return;
	    }
            if (prev == NULL)
                ban_list   = ban_list->next;
            else
                prev->next = curr->next;

            free_ban(curr);
	    char_printf(ch,"Ban on %s lifted.\n",arg);
	    save_bans();
            return;
        }
    }

    char_puts("Site is not banned.\n", ch);
    return;
}

void do_deny(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Deny whom?\n", ch);
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

	SET_BIT(victim->pcdata->plr_flags, PLR_DENY);
	char_puts("You are denied access!\n", victim);
	wiznet("$N denies access to $i",
		ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	char_puts("Ok.\n", ch);
	save_char_obj(victim, FALSE);
	stop_fighting(victim, TRUE);
	quit_char(victim, 0);
}

void do_delchar(CHAR_DATA *ch, const char *argument)
{
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *victim;
	const char *name;

	one_argument(argument, arg, sizeof(arg));
	if (arg[0] == '\0') {
		char_puts("Deny whom?\n", ch);
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
/*
	SET_BIT(victim->pcdata->plr_flags, PLR_DENY);
	char_puts("You are denied access!\n", victim);
	wiznet("$N denies access to $i",
		ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
	char_puts("Ok.\n", ch);
	save_char_obj(victim, FALSE);
	stop_fighting(victim, TRUE);
	quit_char(victim, 0);
*/
	char_puts("You are permanent annihilated!\n", victim);
	char_printf(ch, "%s annihilated!\n", victim->name);
	wiznet("$N deletes pfile of $i",
		ch, victim, WIZ_PENALTIES, WIZ_SECURE, 0);
//	char_puts("Ok.\n", ch);
	stop_fighting(victim, TRUE);
	
	name = capitalize(victim->name);
	if (victim->desc)
		close_descriptor_muddy(victim->desc);	/* disconnect */
	quit_char(victim, 0);
	snprintf(arg, sizeof(arg), "_%s", name);
	d2rename(PLAYER_PATH, name, TMP_PATH, arg);
}

bool check_ban_name(const char *name)
{
	FILE		*fp;
	char		arg[MAX_STRING_LENGTH];
	const char	*word;
	bool		prefix, suffix;
	int		length;
	bool		found = FALSE;

	if ((fp = dfopen(ETC_PATH, DENY_FILE, "r")) == NULL)
		return FALSE;
	for (; ;)
	{
		if (feof(fp))
			break;
		word = fread_word(fp);
		if (word == NULL || word[0] == '\0')
			continue;
		if (word[0] == '$')
			break;

		length = strlen(word);
		if (word[length-1] == '*')
		{
			prefix = TRUE;
			length--;
		} else
			prefix = FALSE;

		if (word[0] == '*')
		{
			suffix = TRUE;
			length--;
		} else
			suffix = FALSE;

		if (length <= 0)
			continue;

		strnzcpy(arg, UMIN(sizeof(arg), length+1), suffix ? word+1 : word);
		arg[length] = '\0';
		if (prefix && suffix)
		{
			if (!str_infix(arg, name))
			{
				found = TRUE;
				break;
			}
		} else if (prefix)
		{
			if (!str_prefix(arg, name))
			{
				found = TRUE;
				break;
			}
		} else if (suffix)
		{
			if (!str_suffix(arg, name))
			{
				found = TRUE;
				break;
			}
		} else
		{
			if (!str_cmp(arg, name))
			{
				found = TRUE;
				break;
			}
		}
	}

	fclose(fp);
	if (found)
		return TRUE;
	return FALSE;
}

void do_denyname(CHAR_DATA *ch, const char *argument)
{
	const char	*tmp_file = "_deny.list";
	FILE		*fpr, *fpw;
	char		arg[MAX_STRING_LENGTH];
	const char	*word;
	bool		prefix, suffix;
	int		length;
	int		remove;
	bool		found = FALSE;

	argument = one_argument(argument, arg, sizeof(arg));

	if (arg[0] == '-')
	{
		argument = one_argument(argument, arg, sizeof(arg));
		remove = TRUE;
	} else
		remove = FALSE;

	if (arg[0] == '\0' || arg[0] == '?')
	{
		BUFFER *output;
		bool	pattern;
		int	col = 0, count = 0;
		/* List denied names (in 'argument' grep string) */

		if ((pattern = (arg[0] == '?')) && argument[0] == '\0')
		{
			char_puts(	"Syntax: 'dename <deny name>'\n"
					"        'dename - <undeny name>'\n"
					"        'dename [? <pattern>]' - get list denied\n", ch);
			return;
		}
		if ((fpr = dfopen(ETC_PATH, DENY_FILE, "r")) == NULL)
		{
			char_printf(ch, "Can't open %s: %s\n",
					DENY_FILE, strerror(errno));
			return;
		}

		output = buf_new(-1);
		for (; ;)
		{
			if (feof(fpr))
				break;
			word = fread_word(fpr);
			if (word == NULL || word[0] == '\0')
				continue;
			if (word[0] == '$')
				break;
			if (pattern && str_infix(argument, word))
				continue;
			buf_printf(output, "%-18s", word);
			if (++col % 4 == 0)
				buf_add(output, "\n");
			count++;
		}
		if (col % 4)
			buf_add(output, "\n");
		buf_printf(output, "Counter: %d\n", count);
		page_to_char(buf_string(output), ch);
		buf_free(output);
		fclose(fpr);
		return;
	}

	if ((fpr = dfopen(ETC_PATH, DENY_FILE, "r")) == NULL)
	{
		char_printf(ch, "Can't open %s: %s\n",
				DENY_FILE, strerror(errno));
		return;
	}
	if ((fpw = dfopen(TMP_PATH, tmp_file, "w")) == NULL)
	{
		char_printf(ch, "Can't create %s: %s\n",
				tmp_file, strerror(errno));
		fclose(fpr);
		return;
	}

	for (; ;)
	{
		if (feof(fpr))
			break;
		word = fread_word(fpr);
		if (word == NULL || word[0] == '\0')
			continue;
		if (word[0] == '$')
			break;

		length = strlen(word);
		if (word[length-1] == '*')
		{
			prefix = TRUE;
			length--;
		} else
			prefix = FALSE;

		if (word[0] == '*')
		{
			suffix = TRUE;
			length--;
		} else
			suffix = FALSE;

		if (length <= 0)
			continue;

		//strnzcpy(arg, UMIN(sizeof(arg), length+1), suffix ? word+1 : word);
		///arg[length] = '\0';
		if (!str_cmp(arg, word))
		{
			if (remove)
			{
				char_printf(ch, "Remove '%s'.\n",
						word);
				found = TRUE;
				continue;
			} else {
				char_printf(ch, "'%s' already exists.\n",
						word);
				fclose(fpr);
				fclose(fpw);
				dunlink(TMP_PATH, tmp_file);
				return;
			}
			break;
		}
		fprintf(fpw, "%s\n", word);
	}

	if (!remove)
	{
		fprintf(fpw, "%s\n", arg);
		char_printf(ch, "'%s' added in DENIED list.\n", arg);
	} else {
		if (!found)
			char_printf(ch, "'%s' not found in DENIED list.\n",
				arg);
	}

	fprintf(fpw, "$\nGenerated by muddy\n");
	fclose(fpr);
	fclose(fpw);

	d2rename(TMP_PATH, tmp_file, ETC_PATH, DENY_FILE);
}

