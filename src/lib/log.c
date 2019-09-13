/*
 * $Id: log.c,v 1.3 2003/05/17 15:33:38 xor Exp $
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

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "libtypedef.h"
#include "libconst.h"
#include "log.h"
#include "util.h"

//extern FILE *logfile;
FILE *logfile;
#ifdef SUNOS
#	include "compat/compat.h"
#endif
#ifdef WIN32
#	include <string.h>
#	define vsnprintf	_vsnprintf
#endif

bool initlogfile(const char *path, const char *filename)
{
	if (!(logfile = dfopen(path, filename, "w")) )
		return FALSE;
	/* turn of bufferize */
	setvbuf(logfile, NULL, _IONBF, 0);
	return TRUE;
}

void shutdownlog(void)
{
	if (logfile)
		fclose(logfile);
}
/*
 * Writes a string to the log.
 */
void log_printf(const char *format, ...)
{
	time_t current_time;
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);

	time(&current_time);
	fprintf(stderr, "%s :: %s\n", strtime(current_time), buf);
	if (logfile)
		fprintf(logfile, "%s :: %s\n", strtime(current_time), buf);
/*
#if defined (WIN32)
	logfile = fopen("sog.log", "a+b");
	if (logfile) {
		fprintf(logfile, "%s :: %s\n", strtime(current_time), buf);
		fclose(logfile);
	}
#else
	if (logfile)
		fprintf(logfile, "%s :: %s\n", strtime(current_time), buf);
#endif
*/

}


/*
 * Reports a bug.
 */
void bug(const char *str, int param)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];

	snprintf(buf2, sizeof(buf2), str, param);
	snprintf(buf, sizeof(buf), "[*****] BUG: %s", buf2);
	log(buf);
}

