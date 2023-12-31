/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

/*
 * $Id: lookup.c,v 1.31 2003/04/22 07:35:22 xor Exp $
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
#include <stdio.h>
#include <time.h>
#include "merc.h"
#include "db.h"

/* returns material number */
int material_lookup(const char *name)
{
	return 0;
}

int liq_lookup(const char *name)
{
	int liq;

	for (liq = 0; liq_table[liq].liq_name; liq++) {
		if (LOWER(name[0]) == LOWER(liq_table[liq].liq_name[0])
		&& !str_prefix(name,liq_table[liq].liq_name))
		    return liq;
	}

	if (fBootDb)
		db_error("liq_lookup", "%s: Unknown liquid name", name);
	return -1;
}

void show_liq_types(BUFFER *output)
{
	int  liq;
	int  col;
 
	col = 0;
	for (liq = 0; liq_table[liq].liq_name; liq++) {
		buf_printf(output, "%-19.18s", liq_table[liq].liq_name);
		if (++col % 4 == 0)
			buf_add(output, "\n");
	}
 
	if (col % 4)
		buf_add(output, "\n");
}

int attack_lookup(const char *name)
{
	int att;
	struct attack_type * atd;

	for (att = 0; att < damages_data.attack_table.nused; att++) {
		if ((atd = varr_get(&damages_data.attack_table, att)) == NULL)
			continue;
		if (LOWER(name[0]) == LOWER(mlstr_mval(atd->name)[0])
		&&  !str_prefix(name, mlstr_mval(atd->name)) )
			return att;
	}

	if (fBootDb)
		db_error("attack_lookup", "%s: Unknown attack name", name);
	return -1;
}

void show_attack_types(BUFFER *output)
{
	int  att;
	int  col;
	struct attack_type *atd;
 
	col = 0;
	for (att = 0; att < damages_data.attack_table.nused; att++) {
		if ((atd = GET_ATTACK_T(att)) == NULL)
			continue;
		buf_printf(output, "%-19.18s", mlstr_mval(atd->name));
		if (++col % 4 == 0)
			buf_add(output, "\n");
	}
 
	if (col % 4)
		buf_add(output, "\n");
}

/* returns a flag for wiznet */
long wiznet_lookup(const char *name)
{
	int flag;

	for (flag = 0; wiznet_table[flag].name; flag++)
	{
		if (LOWER(name[0]) == LOWER(wiznet_table[flag].name[0])
		&& !str_prefix(name,wiznet_table[flag].name))
		    return flag;
	}

	return -1;
}
