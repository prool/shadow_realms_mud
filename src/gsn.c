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
 * $Id: gsn.c,v 1.33 2007/01/27 22:52:13 rem Exp $
 */

#include <stdio.h>

#include "typedef.h"
#include "gsn.h"
#include "namedp.h"

/* gsns */
int gsn_backstab;
int gsn_dodge;
int gsn_envenom;
int gsn_hide;
int gsn_peek;
int gsn_sneak;
int gsn_disarm;
int gsn_enhanced_damage;
int gsn_kick;
int gsn_parry;
int gsn_mentalblock;
int gsn_rescue;
int gsn_second_attack;
int gsn_third_attack;
int gsn_extra_attack;
int gsn_blindness;
int gsn_charm_person;
int gsn_curse;
int gsn_anathema;
int gsn_remove_curse;
int gsn_invisibility;
int gsn_mass_invis;
int gsn_poison;
int gsn_plague;
int gsn_sleep;
int gsn_sanctuary;
int gsn_black_shroud;
int gsn_demand;
int gsn_fly;
int gsn_fourth_attack;
int gsn_dual_backstab;
int gsn_cleave;
int gsn_counter;
int gsn_ambush;
int gsn_circle;
int gsn_nerve;
int gsn_endure;
int gsn_quiet_movement;
int gsn_tame;
int gsn_assassinate;
int gsn_caltrops;
int gsn_throw;
int gsn_strangle;
int gsn_blackjack;
int gsn_bloodthirst;
int gsn_spellbane;
int gsn_resistance;
int gsn_deathblow;
int gsn_fade;
int gsn_garble;
int gsn_confuse;
int gsn_track;
int gsn_hunt;
int gsn_chaos_blade;
int gsn_wrath;
int gsn_disintegrate;
int gsn_stalker;
int gsn_tesseract;
int gsn_randomizer;
int gsn_trophy;
int gsn_truesight;
int gsn_brew;
int gsn_shadowlife;
int gsn_ruler_badge;
int gsn_remove_badge;
int gsn_dragon_strength;
int gsn_dragon_breath;
int gsn_warcry;
int gsn_dragonsword;
int gsn_guard;
int gsn_guard_call;
int gsn_love_potion;
int gsn_deafen;
int gsn_protective_shield;
int gsn_trance;
int gsn_demon_summon;
int gsn_nightwalker;
int gsn_squire;
int gsn_lightning_bolt;
int gsn_disperse;
int gsn_bless;
int gsn_weaken;
int gsn_haste;
int gsn_cure_critical;
int gsn_cure_serious;
int gsn_burning_hands;
int gsn_acid_blast;
int gsn_ray_of_truth;
int gsn_spell_craft;
int gsn_improved_maladiction;
int gsn_improved_benediction;
int gsn_improved_attack;
int gsn_improved_combat;
int gsn_improved_fight;
int gsn_improved_weapon;
int gsn_improved_curative;
int gsn_improved_beguiling;
int gsn_improved_protective;
int gsn_giant_strength;
int gsn_dragon_wit;
int gsn_trollish_vigor;
int gsn_pray;
int gsn_elven_beauty;
int gsn_sagacity;
int gsn_explode;
int gsn_acid_breath;
int gsn_fire_breath;
int gsn_frost_breath;
int gsn_gas_breath;
int gsn_lightning_breath;
int gsn_cure_light;
int gsn_magic_missile;
int gsn_demonfire;
int gsn_faerie_fire;
int gsn_shield;
int gsn_chill_touch;
int gsn_corruption;
int gsn_second_weapon;
int gsn_second_dagger;
int gsn_target;
int gsn_sand_storm;
int gsn_scream;
int gsn_tiger_power;
int gsn_hara_kiri;
int gsn_enhanced_armor;
int gsn_vampire;
int gsn_vampiric_bite;
int gsn_light_resistance;
int gsn_blink;
int gsn_path_find;
int gsn_search;
int gsn_dig;
int gsn_critical;
int gsn_piercing_pain;
int gsn_detect_sneak;
int gsn_mend;
int gsn_shielding;
int gsn_blind_fighting;
int gsn_swimming;
int gsn_aquabreath;
int gsn_drive_boat;
int gsn_camouflage_move;
int gsn_protection_heat;
int gsn_protection_cold;
int gsn_teleport;
int gsn_witch_curse;
int gsn_kassandra;
int gsn_sebat;
int gsn_matandra;
int gsn_armor_use;
int gsn_cure_poison;
int gsn_fire_shield;
int gsn_fear;
int gsn_settraps;
int gsn_mental_attack;
int gsn_secondary_attack;
int gsn_mortal_strike;
int gsn_shield_cleave;
int gsn_weapon_cleave;
int gsn_slow;
int gsn_improved_invis;
int gsn_tail;
int gsn_power_word_stun;
int gsn_grip;
int gsn_concentrate;
int gsn_mastering_sword;
int gsn_master_hand;
int gsn_mastering_pound;
int gsn_fifth_attack;
int gsn_area_attack;
int gsn_reserved;
int gsn_add_skill;
int gsn_bandage;
int gsn_web;
int gsn_bow;
int gsn_mastering_bow;
int gsn_bash_door;
int gsn_katana;
int gsn_bluefire;
int gsn_crush;
int gsn_charge;
int gsn_perception;
int gsn_deadly_venom;
int gsn_cursed_lands;
int gsn_lethargic_mist;
int gsn_black_death;
int gsn_mysterious_dream;
int gsn_sense_life;
int gsn_arrow;
int gsn_lance;
int gsn_evil_spirit;
int gsn_blindness_dust;
int gsn_poison_smoke;
int gsn_mastering_spell;

/* new_gsns */

int gsn_axe;
int gsn_dagger;
int gsn_flail;
int gsn_mace;
int gsn_polearm;
int gsn_shield_block;
int gsn_spear;
int gsn_sword;
int gsn_whip;
int gsn_staves;
 
int gsn_bash;
int gsn_berserk;
int gsn_dirt;
int gsn_hand_to_hand;
int gsn_trip;
 
int gsn_fast_healing;
int gsn_haggle;
int gsn_meditation;
 
int gsn_frenzy;
int gsn_riding;
int gsn_riding_fight;
int gsn_thumbling;
int gsn_pick;
int gsn_hand_block;
int gsn_forest_fighting;

int gsn_doppelganger;
int gsn_mirror;
int gsn_knife;

int gsn_wrath_of_gods;

int gsn_wands;
int gsn_brandish;
int gsn_scrolls;

int gsn_common_slang;
int gsn_feebling;

namedp_t real_gsn_table[] =
{
	{ "gsn_backstab",		&gsn_backstab			},
	{ "gsn_dodge",			&gsn_dodge			},
	{ "gsn_envenom",		&gsn_envenom			},
	{ "gsn_hide",			&gsn_hide			},
	{ "gsn_peek",			&gsn_peek			},
	{ "gsn_sneak",			&gsn_sneak			},
	{ "gsn_disarm",			&gsn_disarm			},
	{ "gsn_enhanced_damage",	&gsn_enhanced_damage		},
	{ "gsn_kick",			&gsn_kick			},
	{ "gsn_parry",			&gsn_parry			},
	{ "gsn_mentalblock",		&gsn_mentalblock		},
	{ "gsn_rescue",			&gsn_rescue			},
	{ "gsn_second_attack",		&gsn_second_attack		},
	{ "gsn_third_attack",		&gsn_third_attack		},
	{ "gsn_extra_attack",		&gsn_extra_attack		},
	{ "gsn_blindness",		&gsn_blindness			},
	{ "gsn_charm_person",		&gsn_charm_person		},
	{ "gsn_curse",			&gsn_curse			},
	{ "gsn_anathema",		&gsn_anathema			},
	{ "gsn_remove_curse",		&gsn_remove_curse		},
	{ "gsn_invisibility",		&gsn_invisibility		},
	{ "gsn_mass_invis",		&gsn_mass_invis			},
	{ "gsn_poison",			&gsn_poison			},
	{ "gsn_plague",			&gsn_plague			},
	{ "gsn_sleep",			&gsn_sleep			},
	{ "gsn_sanctuary",		&gsn_sanctuary			},
	{ "gsn_black_shroud",		&gsn_black_shroud		},
	{ "gsn_demand", 		&gsn_demand			},
	{ "gsn_fly",			&gsn_fly			},
	{ "gsn_fourth_attack",		&gsn_fourth_attack		},
	{ "gsn_dual_backstab",		&gsn_dual_backstab		},
	{ "gsn_cleave",			&gsn_cleave			},
	{ "gsn_counter",		&gsn_counter			},
	{ "gsn_ambush",			&gsn_ambush			},
	{ "gsn_circle",			&gsn_circle			},
	{ "gsn_nerve",			&gsn_nerve			},
	{ "gsn_endure",			&gsn_endure			},
	{ "gsn_quiet_movement",		&gsn_quiet_movement		},
	{ "gsn_tame",			&gsn_tame			},
	{ "gsn_assassinate",		&gsn_assassinate		},
	{ "gsn_caltrops",		&gsn_caltrops			},
	{ "gsn_throw",			&gsn_throw			},
	{ "gsn_strangle",		&gsn_strangle			},
	{ "gsn_blackjack",		&gsn_blackjack			},
	{ "gsn_bloodthirst",		&gsn_bloodthirst		},
	{ "gsn_spellbane",		&gsn_spellbane			},
	{ "gsn_resistance",		&gsn_resistance			},
	{ "gsn_deathblow",		&gsn_deathblow			},
	{ "gsn_fade",			&gsn_fade			},
	{ "gsn_garble",			&gsn_garble			},
	{ "gsn_confuse",		&gsn_confuse			},
	{ "gsn_track",			&gsn_track			},
	{ "gsn_hunt",			&gsn_hunt			},
	{ "gsn_chaos_blade",		&gsn_chaos_blade		},
	{ "gsn_wrath",			&gsn_wrath			},
	{ "gsn_disintegrate",		&gsn_disintegrate		},
	{ "gsn_stalker",		&gsn_stalker			},
	{ "gsn_tesseract",		&gsn_tesseract			},
	{ "gsn_randomizer",		&gsn_randomizer			},
	{ "gsn_trophy",			&gsn_trophy			},
	{ "gsn_truesight",		&gsn_truesight			},
	{ "gsn_brew",			&gsn_brew			},
	{ "gsn_shadowlife",		&gsn_shadowlife			},
	{ "gsn_ruler_badge",		&gsn_ruler_badge		},
	{ "gsn_remove_badge",		&gsn_remove_badge		},
	{ "gsn_dragon_strength",	&gsn_dragon_strength		},
	{ "gsn_dragon_breath",		&gsn_dragon_breath		},
	{ "gsn_warcry",			&gsn_warcry			},
	{ "gsn_dragonsword",		&gsn_dragonsword		},
	{ "gsn_guard",			&gsn_guard			},
	{ "gsn_guard_call",		&gsn_guard_call			},
	{ "gsn_love_potion",		&gsn_love_potion		},
	{ "gsn_deafen",			&gsn_deafen			},
	{ "gsn_protective_shield",	&gsn_protective_shield		},
	{ "gsn_trance",			&gsn_trance			},
	{ "gsn_demon_summon",		&gsn_demon_summon		},
	{ "gsn_nightwalker",		&gsn_nightwalker		},
	{ "gsn_squire",			&gsn_squire			},
	{ "gsn_lightning_bolt",		&gsn_lightning_bolt		},
	{ "gsn_disperse",		&gsn_disperse			},
	{ "gsn_bless",			&gsn_bless			},
	{ "gsn_weaken",			&gsn_weaken			},
	{ "gsn_haste",			&gsn_haste			},
	{ "gsn_cure_critical",		&gsn_cure_critical		},
	{ "gsn_cure_serious",		&gsn_cure_serious		},
	{ "gsn_burning_hands",		&gsn_burning_hands		},
	{ "gsn_acid_blast",		&gsn_acid_blast			},
	{ "gsn_ray_of_truth",		&gsn_ray_of_truth		},
	{ "gsn_spell_craft",		&gsn_spell_craft		},
	{ "gsn_improved_maladiction",	&gsn_improved_maladiction	},
	{ "gsn_improved_benediction",	&gsn_improved_benediction	},
	{ "gsn_improved_attack",	&gsn_improved_attack		},
	{ "gsn_improved_combat",	&gsn_improved_combat		},
	{ "gsn_improved_fight",		&gsn_improved_fight		},
	{ "gsn_improved_weapon",	&gsn_improved_weapon		},
	{ "gsn_improved_curative",	&gsn_improved_curative		},
	{ "gsn_improved_beguiling",	&gsn_improved_beguiling		},
	{ "gsn_improved_protective",	&gsn_improved_protective	},
	{ "gsn_giant_strength",		&gsn_giant_strength		},
	{ "gsn_pray",			&gsn_pray			},
	{ "gsn_trollish_vigor",		&gsn_trollish_vigor		},
	{ "gsn_dragon_wit",		&gsn_dragon_wit			},
	{ "gsn_elven_beauty",		&gsn_elven_beauty		},
	{ "gsn_sagacity",		&gsn_sagacity			},
	{ "gsn_explode",		&gsn_explode			},
	{ "gsn_acid_breath",		&gsn_acid_breath		},
	{ "gsn_fire_breath",		&gsn_fire_breath		},
	{ "gsn_frost_breath",		&gsn_frost_breath		},
	{ "gsn_gas_breath",		&gsn_gas_breath			},
	{ "gsn_lightning_breath",	&gsn_lightning_breath		},
	{ "gsn_cure_light",		&gsn_cure_light			},
	{ "gsn_magic_missile",		&gsn_magic_missile		},
	{ "gsn_demonfire",		&gsn_demonfire			},
	{ "gsn_faerie_fire",		&gsn_faerie_fire		},
	{ "gsn_shield",			&gsn_shield			},
	{ "gsn_chill_touch",		&gsn_chill_touch		},
	{ "gsn_corruption",		&gsn_corruption			},
	{ "gsn_second_weapon",		&gsn_second_weapon		},
	{ "gsn_second_dagger",		&gsn_second_dagger		},
	{ "gsn_target",			&gsn_target			},
	{ "gsn_sand_storm",		&gsn_sand_storm			},
	{ "gsn_scream",			&gsn_scream			},
	{ "gsn_tiger_power",		&gsn_tiger_power		},
	{ "gsn_hara_kiri",		&gsn_hara_kiri			},
	{ "gsn_enhanced_armor",		&gsn_enhanced_armor		},
	{ "gsn_vampire",		&gsn_vampire			},
	{ "gsn_vampiric_bite",		&gsn_vampiric_bite		},
	{ "gsn_light_resistance",	&gsn_light_resistance		},
	{ "gsn_blink",			&gsn_blink			},
	{ "gsn_path_find",		&gsn_path_find			},
	{ "gsn_search",			&gsn_search			},  
	{ "gsn_dig",			&gsn_dig			},
	{ "gsn_critical",		&gsn_critical			},
	{ "gsn_piercing_pain",		&gsn_piercing_pain		},
	{ "gsn_detect_sneak",		&gsn_detect_sneak		},
	{ "gsn_mend",			&gsn_mend			},
	{ "gsn_shielding",		&gsn_shielding			},
	{ "gsn_blind_fighting",		&gsn_blind_fighting		},
	{ "gsn_swimming",		&gsn_swimming			},
	{ "gsn_aquabreath",		&gsn_aquabreath			},
	{ "gsn_drive_boat",		&gsn_drive_boat			},
	{ "gsn_camouflage_move",	&gsn_camouflage_move		},
	{ "gsn_protection_heat",	&gsn_protection_heat		},
	{ "gsn_protection_cold",	&gsn_protection_cold		},
	{ "gsn_teleport",		&gsn_teleport			},
	{ "gsn_witch_curse",		&gsn_witch_curse		},
	{ "gsn_kassandra",		&gsn_kassandra			},
	{ "gsn_sebat",			&gsn_sebat			},
	{ "gsn_matandra",		&gsn_matandra			},
	{ "gsn_armor_use",		&gsn_armor_use			},
	{ "gsn_cure_poison",		&gsn_cure_poison		},
	{ "gsn_fire_shield",		&gsn_fire_shield		},
	{ "gsn_fear",			&gsn_fear			},
	{ "gsn_settraps",		&gsn_settraps			},
	{ "gsn_mental_attack",		&gsn_mental_attack		},
	{ "gsn_secondary_attack",	&gsn_secondary_attack		},
	{ "gsn_mortal_strike",		&gsn_mortal_strike		},
	{ "gsn_shield_cleave",		&gsn_shield_cleave		},
	{ "gsn_weapon_cleave",		&gsn_weapon_cleave		},
	{ "gsn_slow",			&gsn_slow			},
	{ "gsn_improved_invis",		&gsn_improved_invis		},
	{ "gsn_tail",			&gsn_tail			},
	{ "gsn_power_word_stun",	&gsn_power_word_stun		},
	{ "gsn_grip",			&gsn_grip			},
	{ "gsn_concentrate",		&gsn_concentrate		},
	{ "gsn_mastering_sword",	&gsn_mastering_sword		},
	{ "gsn_master_hand",		&gsn_master_hand		},
	{ "gsn_mastering_pound",	&gsn_mastering_pound		},
	{ "gsn_fifth_attack",		&gsn_fifth_attack		},
	{ "gsn_area_attack",		&gsn_area_attack		},
	{ "gsn_reserved",		&gsn_reserved			},
	{ "gsn_add_skill",		&gsn_add_skill			},
	{ "gsn_bandage",		&gsn_bandage			},
	{ "gsn_web",			&gsn_web			},
	{ "gsn_bow",			&gsn_bow			},
	{ "gsn_mastering_bow",		&gsn_mastering_bow		},
	{ "gsn_bash_door",		&gsn_bash_door			},
	{ "gsn_katana",			&gsn_katana			},
	{ "gsn_bluefire",		&gsn_bluefire			},
	{ "gsn_crush",			&gsn_crush			},
	{ "gsn_charge",			&gsn_charge			},
	{ "gsn_perception",		&gsn_perception			},
	{ "gsn_deadly_venom",		&gsn_deadly_venom		},
	{ "gsn_cursed_lands",		&gsn_cursed_lands		},
	{ "gsn_lethargic_mist",		&gsn_lethargic_mist		},
	{ "gsn_black_death",		&gsn_black_death		},
	{ "gsn_mysterious_dream",	&gsn_mysterious_dream		},
	{ "gsn_sense_life",		&gsn_sense_life			},
	{ "gsn_arrow",			&gsn_arrow			},
	{ "gsn_lance",			&gsn_lance			},
	{ "gsn_evil_spirit",		&gsn_evil_spirit		},
	{ "gsn_blindness_dust",		&gsn_blindness_dust		},
	{ "gsn_poison_smoke",		&gsn_poison_smoke		},
	{ "gsn_mastering_spell",	&gsn_mastering_spell		},
	{ "gsn_axe",			&gsn_axe			},
	{ "gsn_dagger",			&gsn_dagger			},
	{ "gsn_staves",			&gsn_staves			},
	{ "gsn_flail",			&gsn_flail			},
	{ "gsn_mace",			&gsn_mace			},
	{ "gsn_polearm",		&gsn_polearm			},
	{ "gsn_shield_block",		&gsn_shield_block		},
	{ "gsn_spear",			&gsn_spear			},
	{ "gsn_sword",			&gsn_sword			},
	{ "gsn_whip",			&gsn_whip			},
	{ "gsn_bash",			&gsn_bash			},
	{ "gsn_berserk",		&gsn_berserk			},
	{ "gsn_dirt",			&gsn_dirt			},
	{ "gsn_hand_to_hand",		&gsn_hand_to_hand		},
	{ "gsn_trip",			&gsn_trip			},
	{ "gsn_fast_healing",		&gsn_fast_healing		},
	{ "gsn_haggle",			&gsn_haggle			},
	{ "gsn_meditation",		&gsn_meditation			},
	{ "gsn_frenzy",			&gsn_frenzy			},
	{ "gsn_riding",			&gsn_riding			},
	{ "gsn_riding_fight",		&gsn_riding_fight		},
	{ "gsn_thumbling",		&gsn_thumbling			},
	{ "gsn_forest_fighting",	&gsn_forest_fighting		},
	{ "gsn_pick",			&gsn_pick			},
	{ "gsn_doppelganger",		&gsn_doppelganger		},
	{ "gsn_mirror",			&gsn_mirror			},
	{ "gsn_hand_block",		&gsn_hand_block			},
	{ "gsn_knife",			&gsn_knife			},
	{ "gsn_wrath_of_gods",		&gsn_wrath_of_gods		},
	{ "gsn_wands",			&gsn_wands			},
	{ "gsn_brandish",		&gsn_brandish			},
	{ "gsn_scrolls",		&gsn_scrolls			},
	{ "gsn_common_slang",		&gsn_common_slang		},
	{ "gsn_feebling",		&gsn_feebling			},

	{ NULL }
};

namedp_t *gsn_table = real_gsn_table;
