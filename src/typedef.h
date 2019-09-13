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
 * $Id: typedef.h,v 1.32 2003/05/18 23:46:25 xor Exp $
 */

#ifndef _TYPEDEF_H_
#define _TYPEDEF_H_

#include <sys/types.h>
#include <libtypedef.h>

typedef struct affect_data		AFFECT_DATA;
typedef struct area_data		AREA_DATA;
typedef struct buf_data			BUFFER;
typedef struct char_data		CHAR_DATA;
typedef struct descriptor_data_muddy	DESCRIPTOR_DATA_MUDDY;
//typedef struct listen_mgater		LISTEN_MGATER;
//typedef struct descriptor_data_mud	DESCRIPTOR_DATA_MUD;
//typedef struct descriptor_client_ftun	DESCRIPTOR_CLIENT_FTUN;
typedef struct cluster_data		CLUSTER_DATA;
typedef struct exit_data		EXIT_DATA;
typedef struct ed_data			ED_DATA;
typedef struct help_data		HELP_DATA;
typedef struct mob_index_data		MOB_INDEX_DATA;
typedef struct obj_data			OBJ_DATA;
typedef struct obj_index_data		OBJ_INDEX_DATA;
typedef struct pc_data 			PC_DATA;
typedef struct religion_char		RELIGION_CHAR;
typedef struct quest_char		QUEST_CHAR;
typedef struct religion_type		RELIGION_DATA;
typedef struct remort_data		REMORT_DATA;
typedef struct rshop_data		RSHOP_DATA;
typedef struct reset_data		RESET_DATA;
typedef struct room_index_data 		ROOM_INDEX_DATA;
typedef struct shop_data		SHOP_DATA;
typedef struct time_info_data		TIME_INFO_DATA;
typedef struct weather_data		WEATHER_DATA;
typedef struct room_history_data	ROOM_HISTORY_DATA;
typedef struct mptrig			MPTRIG;
typedef struct mpcode			MPCODE;
typedef struct qtrouble_t		qtrouble_t;
//typedef struct flag_t			flag_t; 
typedef struct mlstring			mlstring;

typedef struct class_t			class_t;
typedef struct race_t			race_t;
//typedef struct clan_t			clan_t;
typedef struct pcrace_t			pcrace_t;
typedef struct rclass_t			rclass_t;

typedef struct skill_t			skill_t;
typedef struct cskill_t			cskill_t;
typedef struct rskill_t			rskill_t;
typedef struct clskill_t		clskill_t;
typedef struct pcskill_t		pcskill_t;

typedef struct where_t			where_t;
typedef struct namedp_t			namedp_t;
typedef struct lang_t			lang_t;
typedef struct cmd_t			cmd_t;
typedef struct pose_t			pose_t;
typedef struct rulecl_t			rulecl_t;
typedef struct olced_t 			olced_t;	
typedef struct rule_t			rule_t;
typedef union vo_t			vo_t;
typedef struct altar_t			altar_t;
typedef struct hometown_t		hometown_t;
typedef struct note_t			note_t;
typedef struct ban_t			ban_t;

typedef struct damages_msg 		DAMAGES_MSG;

union vo_t {
	int			vnum;
	ROOM_INDEX_DATA *	r;
	OBJ_INDEX_DATA *	o;
	MOB_INDEX_DATA *	m;
};

typedef struct spell_spool_t spell_spool_t;

typedef void	DO_FUN		(CHAR_DATA *ch, const char *argument);
typedef bool	SPEC_FUN	(CHAR_DATA *ch);
//typedef bool	SPELL_FUN	(int sn, int level, CHAR_DATA *ch, void *vo,
//				 int target, int percent);
typedef	bool	SPELL_FUN	(const spell_spool_t *sspell);
typedef int	OPROG_FUN	(OBJ_DATA *obj, CHAR_DATA *ch, const void *arg);

#define args(a) a
#define DECLARE_DO_FUN(fun)	DO_FUN	  fun
#define DECLARE_SPEC_FUN(fun) 	SPEC_FUN  fun
#define DECLARE_SPELL_FUN(fun)	SPELL_FUN fun
#define DECLARE_OPROG_FUN(fun)	OPROG_FUN fun

#define DO_FUN(fun)	void	fun(CHAR_DATA *ch, const char *argument)
#define SPEC_FUN(fun)	bool	fun(CHAR_DATA *ch)
#define SPELL_FUN(fun)	void	fun(int sn, int level, CHAR_DATA *ch, void *vo, int target, int percent)
#define OPROG_FUN(fun)	int	fun(OBJ_DATA *obj, CHAR_DATA *ch, void *arg);

/* WIN32 Microsoft specific definitions */
//#if defined (WIN32)
//#	define vsnprintf	_vsnprintf
//#	define snprintf		_snprintf
//#	define vsnprintf	_vsnprintf
//#endif
 
#endif

