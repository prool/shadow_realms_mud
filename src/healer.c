/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: healer.c,v 1.31 2003/04/22 07:35:22 xor Exp $
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

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "merc.h"

DECLARE_DO_FUN(	do_say	);

void do_heal(CHAR_DATA *ch, const char *argument)
{
    CHAR_DATA *mob;
    char arg[MAX_INPUT_LENGTH];
    char arg2[MAX_INPUT_LENGTH];
    int sn;
    int cost;
    SPELL_FUN *spell;
    void *vo = NULL;
    char *words;
    RELIGION_DATA *rel;

    /* check for healer */
	for (mob = ch->in_room->people; mob; mob = mob->next_in_room)
		if (IS_NPC(mob) && IS_SET(mob->pIndexData->act, ACT_HEALER)
		/*&&  (!mob->clan || mob->clan == ch->clan)*/)
		 	break;
 
    if (mob == NULL) {
        char_puts("You can't do that here.\n", ch);
        return;
    }

    if (HAS_SKILL(ch, gsn_spellbane)) {
	char_puts("You are Battle Rager, not the filthy magician\n",ch);
	return;
    }

    argument = one_argument(argument, arg, sizeof(arg));
    argument = one_argument(argument, arg2, sizeof(arg2));

    if (arg[0] == '\0') {
        /* display price list */
	act("Healer offers the following spells.",ch,NULL,mob,TO_CHAR);
	char_puts("  light   : cure light wounds     10 silver\n",ch);
	char_puts("  serious : cure serious wounds   15 silver\n",ch);
	char_puts("  critic  : cure critical wounds  25 silver\n",ch);
	char_puts("  heal    : healing spell         50 silver\n",ch);
	char_puts("  blind   : cure blindness        20 silver\n",ch);
	char_puts("  disease : cure disease          25 silver\n",ch);
	char_puts("  poison  : cure poison           10 silver\n",ch); 
	char_puts("  uncurse : remove curse          50 silver\n",ch);
	char_puts("  refresh : restore movement       5 silver\n",ch);
	char_puts("  mana    : restore mana          10 silver\n",ch);
	char_puts("  master  : master heal spell      1 gold\n",ch);
	char_puts("  energize: restore 300 mana       1 gold\n",ch);
	char_puts("  haste   : haste spell            3 gold\n",ch);
	char_puts("  giant   : giant strength         2 gold\n",ch);
	char_puts("  wit     : dragon wit           2.5 gold\n",ch);
	char_puts("  sagacity: sagacity               2 gold\n",ch);
	char_puts("  vigor   : trollish vigor         4 gold\n",ch);
	char_puts("  beauty  : elven beauty         1.5 gold\n",ch);
	char_puts("  pray    : pray                 5.5 gold\n",ch);
	char_puts(" Type heal <type> to be healed.\n",ch);
	return;
    }

    if (!str_prefix(arg,"light"))
    {
        spell = spell_cure_light;
	sn    = sn_lookup("cure light");
	words = "judicandus dies";
	 cost  = 10;
    }

    else if (!str_prefix(arg,"serious"))
    {
	spell = spell_cure_serious;
	sn    = sn_lookup("cure serious");
	words = "judicandus gzfuajg";
	cost  = 15;
    }

    else if (!str_prefix(arg,"critical"))
    {
	spell = spell_cure_critical;
	sn    = sn_lookup("cure critical");
	words = "judicandus qfuhuqar";
	cost  = 25;
    }

    else if (!str_prefix(arg,"heal"))
    {
	spell = spell_heal;
	sn = sn_lookup("heal");
	words = "pzar";
	cost  = 50;
    }

    else if (!str_prefix(arg,"blindness"))
    {
	spell = spell_cure_blindness;
	sn    = sn_lookup("cure blindness");
      	words = "judicandus noselacri";		
        cost  = 20;
    }

    else if (!str_prefix(arg,"disease") || !str_prefix(arg, "plague"))
    {
	spell = spell_cure_disease;
	sn    = sn_lookup("cure disease");
	words = "judicandus eugzagz";
	cost = 25;
    }

    else if (!str_prefix(arg,"poison"))
    {
	spell = spell_cure_poison;
	sn    = sn_lookup("cure poison");
	words = "judicandus sausabru";
	cost  = 10;
    }
	
    else if (!str_prefix(arg,"uncurse") || !str_prefix(arg,"curse"))
    {
	spell = spell_remove_curse; 
	sn    = sn_lookup("remove curse");
	words = "candussido judifgz";
	cost  = 50;
	if (arg2[0] != '\0') {
		if ((vo = get_obj_here(ch, arg2)) == NULL) {
			char_printf(ch, "You don't see '%s' here.\n", arg2);
			return;
		}
	}
    }

    else if (!str_prefix(arg,"mana"))
    {
        spell = NULL;
        sn = -3;
        words = "candamira";
        cost = 10;
    }

	
    else if (!str_prefix(arg,"refresh") || !str_prefix(arg,"moves"))
    {
	spell =  spell_refresh;
	sn    = sn_lookup("refresh");
	words = "candusima"; 
	cost  = 5;
    }

    else if (!str_prefix(arg,"master"))
    {
	spell =  spell_master_healing;
	sn    = sn_lookup("master healing");
	words = "candastra nikazubra"; 
	cost  = 100;
    }

    else if (!str_prefix(arg,"energize"))
    {
	spell =  NULL;
	sn    = -2;
	words = "energizer"; 
	cost  = 100;
    }
    
    else if (!str_prefix(arg, "haste"))
    {
    	spell	= spell_haste;
    	sn	= sn_lookup("haste");
    	words	= "paghz";
    	cost	= 300;
    }
    
    else if (!str_prefix(arg, "giant"))
    {
	spell	= spell_giant_strength;
	sn	= sn_lookup("giant strength");
	words	= "ouaih ghcandusiohp";
	cost	= 200;
    }
    
    
    else if (!str_prefix(arg, "wit"))
    {
	spell	= spell_dragon_wit;
	sn	= sn_lookup("dragon wit");
	words	= "egruoai xuh";
	cost	= 250;
    }
    else if (!str_prefix(arg, "sagacity"))
    {
	spell	= spell_sagacity;
	sn	= sn_lookup("sagacity");
	words	= "gaoaquhl";
	cost	= 200;
    }
    else if (!str_prefix(arg, "vigor"))
    {
	spell	= spell_trollish_vigor;
	sn	= sn_lookup("trollish vigor");
	words	= "hfarrugp zuoaf";
	cost	= 400;
    }
    else if (!str_prefix(arg, "beauty"))
    {
	spell	= spell_elven_beauty;
	sn	= sn_lookup("elven beauty");
	words	= "zrnofo bzkadahl";
	cost	= 150;
    }
    
    else if (!str_prefix(arg, "pray"))
    {
	spell	= spell_pray;
	sn	= sn_lookup("pray");
	words	= "sgrul";
	cost	= 550;
    }

    else 
    {
	act("Healer does not offer that spell.  Type 'heal' for a list.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }
    
    if ((rel = GET_CHAR_RELIGION(ch))
    && IS_SET(rel->flags, RELIG_CHEAP_HEALER))
    	cost = UMAX(1, cost / 1.5);

    if (cost > (ch->gold * 100 + ch->silver))
    {
	act("You do not have that much money.",
	    ch,NULL,mob,TO_CHAR);
	return;
    }

    WAIT_STATE(ch, PULSE_VIOLENCE);

    deduct_cost(ch, cost);

    act("$n utters the words, '$t'.", mob, words, NULL, TO_ROOM);
    if (sn == -2)
     {
	ch->mana += 300;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	char_puts("A warm glow passes through you.\n",ch);
     }
    if (sn == -3)
    {
	ch->mana += dice(2,8) + mob->level / 3;
	ch->mana = UMIN(ch->mana,ch->max_mana);
	char_puts("A warm glow passes through you.\n",ch);
    }
  
     if (sn < 0)
	return;
     {
	spell_spool_t sspell;

	sspell.sn		= sn;
	sspell.level	= mob->level;
	sspell.ch		= mob;
	sspell.target	= TARGET_OBJ;
	sspell.percent	= 100;
	if (spell == spell_remove_curse && vo)
	{
		sspell.vo	= vo;
		spell(&sspell);
	} else {
		sspell.vo	= ch;
		spell(&sspell);
	}
     }
}
