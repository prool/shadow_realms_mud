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
 * $Id: skills.h,v 1.31 2003/04/22 07:35:23 xor Exp $
 */

#ifndef _SKILLS_H_
#define _SKILLS_H_

/*----------------------------------------------------------------------
 * skills stuff
 */

#define SKILL_CLAN		(A)
#define SKILL_RANGE		(B)
#define SKILL_AREA_ATTACK	(C)
#define SKILL_QUESTIONABLE	(D)
#define SKILL_NOREMORT		(E)
#define SKILL_CLERIC		(F)
#define SKILL_CREATED		(G)

struct skill_t {
	const char *	name;			/* skill name */
	int *		pgsn;			/* pointer to gsn */
	int		beats;			/* waiting time after use */
	mlstring *	noun_damage;		/* damage message */
	mlstring *	msg_off;		/* wear off message */
	mlstring *	msg_obj;		/* wear off message for obj */
	flag32_t	flags;			/* skill flags */
	// const char *	restrict_race;		/* race restrictions */
	flag32_t	group;			/* skill group */
	varr		sell;			/* Sell skills (sellskill_t) */
	int		min_level;		/* Special for Remort ;) */
	int		remort_cost;		/* in RP (remort points) */
	int		min_mana;		/* min mana used */
	struct spell_pt *	spell;		/* pointer on spell_pt */
};

#define SPC_SPEND		(A)	// component extract
#define SPC_SUCCESSFULL		(B)	// extract only if successfull cast and if set SPC_SPEND
#define SPC_NEED_EQ		(C)	// component need in equipment caster
#define SPC_NEED_GROUND		(D)	// component need in room caster
#define SPC_CHECK_AFTER_DELAY	(E)	// default before
#define SPC_SHOW_HELP		(F)	// print exact that need for cast
#define SPC_NOTNEED_SEE		(G)	// component need not see to caster
#define SPC_SUFFICIENT		(H)	// enougth only this component OR other, but above components check before!

struct spell_component {
	int		vnum;
	flag32_t	flags;
};

// (A) -(6 bit)- (F)		for door
#define SPT_DOOR_BITS	(63)	// (A)(B)(C)(D)(E)(F)  <- hide
#define	SPT_VERBAL	(G)	// say
#define SPT_SOMATIC	(H)	// action
//#define SPT_COMPONENT	(I)	// some thing (see components bellow)


#define SPT_TARG_LOCATE	(O)	// target - locate  (fireball, sand storm, holy world)
#define SPT_TARG_OBJECT (P)	// target - object (bless, create water, pwk)
#define SPT_MANA_AFTER	(Q)	// reduce mana after cast
#define SPT_CHECKPOS_AFTER	(R)
#define SPT_CHECKPOS_BEFORE	(S)

		// hide flags
#define SPT_CAST_FAR		(V)
//#define SPT_MISSING_TARGET	(W)
#define SPT_OFFENSIVE		(X)
#define SPT_VICTSEEACTION	(Y)
#define SPT_NOWAIT		(Z)

/*
 *	Verbal + Somatic + Component(Material)				+ delay
 *	[flag]   [flag]    [vnum or component (in future) - 2 int]	[int]
 */
struct spell_pt {
	SPELL_FUN *	spell_fun;		/* spell function */
	flag32_t	minimum_position;	/* position for caster */
	flag32_t	target;			/* legal target */
	flag32_t	mschool;		/* Magic School	in irv_bits */
	int		delay;			/* delay before cast */
	flag32_t	type;			/* SPT_... */
	varr		components;		/* varr of need components */
/*	int		slot;			   slot for #OBJOLD loading */
};

/*
 *	Magic schools...
 */
enum Number_of_MSchool {
	NONE,		//0
	MENTAL,		//1
	EARTH,
	FIRE,
	AIR,
	WATER,
	LIFE		//6
};

typedef struct magic_school_t magic_school_t;
struct magic_school_t {
	flag32_t		bit_force;
	enum Number_of_MSchool	anti_magic;
	int			apply_saves;
	int			vnum_teacher[3];
};
extern magic_school_t magic_schools[];

int smagic_apply_lookup(int location);
int get_magic_rank(CHAR_DATA * ch, int i);
int get_char_msc(CHAR_DATA *ch, int n_msc, int improve); // get magic school

/*
 *	end of "Magic schools..."
 */

#define SSF_NONE	0
#define SSF_RACE	1
#define SSF_CLASS	2
#define SSF_ALL		3
#define SSF_SEX		4
#define SSF_ALIGN	5
#define SSF_ETHOS	6
	
typedef struct sellskill_t sellskill_t;
struct sellskill_t {
	int		flag;			/* Usege flags SSF_ */
	const char *	name;			/* Name class, race... */
	int		vnum_seller;		/* Vnum mob who sell this skill*/
	int		minlev;			/* Min Level for sell */
	int		cost;			/* in silver */
};
sellskill_t *	sellskill_lookup(CHAR_DATA *ch, int sn);

extern varr skills;

#define HAS_SKILL(ch, sn)	(skill_level(ch, sn) < MAX_LEVEL)

#define SKILL(sn)		((skill_t*) VARR_GET(&skills, sn))
#define skill_lookup(sn)	((skill_t*) varr_get(&skills, sn))

const char *	skill_name	(int sn);
int		sn_lookup	(const char *name);

/* lookup skill by name in skill list */
void *		skill_vlookup	(varr *v, const char *name);

int		get_weapon_sn	(OBJ_DATA *obj);
int		get_weapon_skill(CHAR_DATA *ch, int sn);

int		get_skill	(CHAR_DATA *ch, int sn);
int		get_maxskill	(CHAR_DATA *ch, int sn);
void		set_skill	(CHAR_DATA *ch, int sn, int value, int level, int mp, int hard);
void		set_skill_raw	(CHAR_DATA *ch, int sn, int value, bool repl, int level, int mp, int hard);
//void            set_skill_own       (CHAR_DATA *ch, int sn, int value, int level, int mp, int hard);
//void            set_skill_own_raw   (CHAR_DATA *ch, int sn, int value, bool repl, int level, int mp, int hard);

int             skill_level     (CHAR_DATA *ch, int sn);
//int		skill_level_own	(CHAR_DATA *ch, int sn);

void		remove_skills	(CHAR_DATA *ch);
void		update_skills	(CHAR_DATA *ch);

int		mana_cost	(CHAR_DATA *ch, int sn);
//void		say_spell	(CHAR_DATA *ch, int sn);
bool		action_spell	(CHAR_DATA *ch, skill_t *sk, CHAR_DATA *victim);

int		base_exp	(CHAR_DATA *ch);
int		exp_to_level	(CHAR_DATA *ch);
int		exp_for_level	(CHAR_DATA *ch, int level);
void		check_improve	(CHAR_DATA *ch, int sn, bool success, int mult);
int		group_lookup	(const char *name);
int		slang_lookup(const char *name);

#endif
