/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "merc.h"
#include "auction.h"

AUCTION_DATA auction = { NULL };

/***************************************************************************
 *  This snippet was orginally written by Erwin S. Andreasen.              *
 *	erwin@pip.dknet.dk, http://pip.dknet.dk/~pip1773/                  *
 *  Adopted to Anatolia MUD by chronos.                                    *
 ***************************************************************************/

void talk_auction(const char *fmt, ...)
{
	va_list ap;
	char buf[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA_MUDDY *d;
	CHAR_DATA *original;

	va_start(ap, fmt);
	vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	for (d = descriptor_list_muddy; d; d = d->next)
	{
		/* if switched */
		original = d->original ? d->original : d->character;
		if (d->connected == CON_PLAYING
		&&  !IS_SET(original->comm, COMM_NOAUCTION))
			act_puts("{YAUCTION{x: $t",
				 original, buf, NULL, TO_CHAR | ACT_TRANS, POS_DEAD);
	}
}


/*
  This function allows the following kinds of bets to be made:

  Absolute bet
  ============

  bet 14k, bet 50m66, bet 100k

  Relative bet
  ============

  These bets are calculated relative to the current bet. The '+' symbol adds
  a certain number of percent to the current bet. The default is 25, so
  with a current bet of 1000, bet + gives 1250, bet +50 gives 1500 etc.
  Please note that the number must follow exactly after the +, without any
  spaces!

  The '*' or 'x' bet multiplies the current bet by the number specified,
  defaulting to 2. If the current bet is 1000, bet x  gives 2000, bet x10
  gives 10,000 etc.

*/

int advatoi (const char *s)
/*
  util function, converts an 'advanced' ASCII-number-string into a number.
  Used by parsebet() but could also be used by do_give or do_wimpy.

  Advanced strings can contain 'k' (or 'K') and 'm' ('M') in them, not just
  numbers. The letters multiply whatever is left of them by 1,000 and
  1,000,000 respectively. Example:

  14k = 14 * 1,000 = 14,000
  23m = 23 * 1,000,0000 = 23,000,000

  If any digits follow the 'k' or 'm', the are also added, but the number
  which they are multiplied is divided by ten, each time we get one left. This
  is best illustrated in an example :)

  14k42 = 14 * 1000 + 14 * 100 + 2 * 10 = 14420

  Of course, it only pays off to use that notation when you can skip many 0's.
  There is not much point in writing 66k666 instead of 66666, except maybe
  when you want to make sure that you get 66,666.

  More than 3 (in case of 'k') or 6 ('m') digits after 'k'/'m' are automatically
  disregarded. Example:

  14k1234 = 14,123

  If the number contains any other characters than digits, 'k' or 'm', the
  function returns 0. It also returns 0 if 'k' or 'm' appear more than
  once.

*/

{

/* the pointer to buffer stuff is not really necessary, but originally I
   modified the buffer, so I had to make a copy of it. What the hell, it 
   works:) (read: it seems to work:)
*/

  char string[MAX_INPUT_LENGTH]; /* a buffer to hold a copy of the argument */
  char *stringptr = string; /* a pointer to the buffer so we can move around */
  char tempstring[2];       /* a small temp buffer to pass to atoi*/
  int number = 0;           /* number to be returned */
  int multiplier = 0;       /* multiplier used to get the extra digits right */


  strnzcpy(string, sizeof(string), s);        /* working copy */

  while (isdigit (*stringptr)) /* as long as the current character is a digit */
  {
	  strncpy (tempstring,stringptr,1);           /* copy first digit */
	  number = (number * 10) + atoi (tempstring); /* add to current number */
	  stringptr++;                                /* advance */
  }

  switch (UPPER(*stringptr)) {
	  case 'K'  : multiplier = 1000;    number *= multiplier; stringptr++; break;
	  case 'M'  : multiplier = 1000000; number *= multiplier; stringptr++; break;
	  case '\0' : break;
	  default   : return 0; /* not k nor m nor NUL - return 0! */
  }

  while (isdigit (*stringptr) && (multiplier > 1)) /* if any digits follow k/m, add those too */
  {
	  strncpy (tempstring,stringptr,1);           /* copy first digit */
	  multiplier = multiplier / 10;  /* the further we get to right, the less are the digit 'worth' */
	  number = number + (atoi (tempstring) * multiplier);
	  stringptr++;
  }

  if (*stringptr != '\0' && !isdigit(*stringptr)) /* a non-digit character was found, other than NUL */
	return 0; /* If a digit is found, it means the multiplier is 1 - i.e. extra
	             digits that just have to be ignore, liked 14k4443 -> 3 is ignored */


  return (number);
}


int parsebet (const int currentbet, const char *argument)
{
  int newbet = 0;               /* a variable to temporarily hold the new bet */
  char string[MAX_INPUT_LENGTH];/* a buffer to modify the bet string */
  char *stringptr = string;     /* a pointer we can move around */

				/* make a work copy of argument */
  strnzcpy(string, sizeof(string), argument);

  if (*stringptr)               /* check for an empty string */
  {

	if (isdigit (*stringptr)) /* first char is a digit assume e.g. 433k */
	  newbet = advatoi (stringptr); /* parse and set newbet to that value */

	else
	  if (*stringptr == '+') /* add ?? percent */
	  {
	    if (strlen (stringptr) == 1) /* only + specified, assume default */
	      newbet = (currentbet * 125) / 100; /* default: add 25% */
	    else
	      newbet = (currentbet * (100 + atoi (++stringptr))) / 100; /* cut off the first char */
	  }
	  else
	  {
	    if ((*stringptr == '*') || (*stringptr == 'x')) /* multiply */
	    {
	      if (strlen (stringptr) == 1) /* only x specified, assume default */
	        newbet = currentbet * 2 ; /* default: twice */
	      else /* user specified a number */
	        newbet = currentbet * atoi (++stringptr); /* cut off the first char */
	    }
	  }
  }

  return newbet;        /* return the calculated bet */
}


void auction_give_obj(CHAR_DATA* victim, OBJ_DATA *obj)
{
	act("The auctioneer appears before you in a puff of smoke "
	    "and hands you $p.", victim, obj, NULL, TO_CHAR);
	act("The auctioneer appears before $n and hands $m $p.",
	    victim, obj, NULL, TO_ROOM);

	if (victim->carry_weight + get_obj_weight(obj) >
	    can_carry_w(victim)) {
		act("$p is too heavy for you to carry.",
		    victim, obj, NULL, TO_CHAR);
		act("$n is carrying too much to carry $p and $e drops it.",
		    victim, obj, NULL, TO_ROOM);
		obj_to_room (obj, victim->in_room);
	}
	else
		obj_to_char (obj, victim);
}

void auction_update (void)
{
	if (auction.item == NULL)
		return;

	if (--auction.pulse > 0)
		return;

	auction.pulse = PULSE_AUCTION;
	switch (++auction.going) { /* increase the going state */
	case 1 : /* going once */
	case 2 : /* going twice */
	        if (auction.bet > 0)
			/* XXX */
			talk_auction("%s: going %s for %d.",
				fix_short(mlstr_mval(auction.item->short_descr)),
	                	((auction.going == 1) ? "once" : "twice"),
				auction.bet);
	        else 
			/* XXX */
	        	talk_auction("%s: going %s, starting price %d.",
				     fix_short(mlstr_mval(auction.item->short_descr)),
	                	     ((auction.going == 1) ? "once" : "twice"),
				     auction.starting);
	        break;

	 case 3 : /* SOLD! */
	        if (auction.bet > 0) {
			int tax;
			int pay;
			bool auc_free = GET_CHAR_RELIGION(auction.buyer)
				&& IS_SET(GET_CHAR_RELIGION(auction.buyer)->flags, RELIG_AUC_FREE);

			/* XXX */
	        	talk_auction("%s: sold to %s for %d.",
				     fix_short(mlstr_mval(auction.item->short_descr)),
				     (IS_PC(auction.buyer) && auction.buyer->pcdata->invis_level >= LEVEL_HERO) ?
				     "someone" : auction.buyer->name,
				     auction.bet);

			auction_give_obj(auction.buyer, auction.item);

			tax = auction.bet * 0.1;
			/* pay = auction.bet * 90 / 100; */
			pay = auction.bet - tax;

			 /* give him the money */
			if (tax
			&& !auc_free)
				char_printf(auction.seller,
				    "The auctioneer pays you %d gold, "
				    "charging an auction fee of %d(10%%).\n",
				    pay, tax);
			else
				char_printf(auction.seller,
				    "The auctioneer pays you %d gold.\n",
				    pay);
			
			auction.seller->gold += pay + (auc_free ? tax : 0);
		}
	        else { /* not sold */
			/* XXX */
	        	talk_auction("No bets received for %s.",
				     fix_short(mlstr_mval(auction.item->short_descr)));
			talk_auction("object has been removed from auction.");
			auction_give_obj(auction.seller, auction.item);
	        }
		auction.item = NULL; /* reset item */
        }
} 


void do_auction(CHAR_DATA *ch, const char *argument)
{
	int tax;
	OBJ_DATA *obj;
	char arg1[MAX_INPUT_LENGTH];
	char starting[MAX_INPUT_LENGTH];

	argument = one_argument(argument, arg1, sizeof(arg1));

	if (IS_NPC(ch))    /* NPC can't auction cos the can be extracted ! */
		return;

	if (IS_SET(ch->comm, COMM_NOAUCTION)) {
		if (!str_cmp(arg1, "on")) {
			char_puts("Auction channel is now ON.\n",ch);
			REMOVE_BIT(ch->comm,COMM_NOAUCTION);
			return;
		}
		else {
			char_puts("Your auction channel is OFF.\n",ch);
			char_puts("You must first change auction channel ON.\n",ch);
			return;
		}
	}

	if (arg1[0] == '\0') {
		if (auction.item != NULL) {
			spell_spool_t sspell;
			/* show item data here */
			if (IS_IMMORTAL(ch)){
				char_printf(ch, "Seller is {W%s{x.\n", auction.seller->name);
				if (auction.bet > 0)
					char_printf(ch, "Buyer is {W%s{x.\n", auction.buyer->name);
			}
			if (auction.bet > 0)
				char_printf(ch, "Current bet on this item is "
						"%d gold.\n",auction.bet);
			else
				char_printf(ch,
					    "Starting price for this item is "
					    "%d gold.\n"
					    "No bets on this item have been "
					    "received.\n", auction.starting);
			sspell.sn	= 0;
			sspell.level	= 0;
			sspell.target	= 0;
			sspell.ch	= ch;
			sspell.vo	= auction.item;
			sspell.percent	= 100;
			spell_identify(&sspell);
			return;
		}
		else {	
			char_puts("Auction WHAT?\n", ch);
			return;
		}
	}

	if (!str_cmp(arg1, "off")) {
		char_puts("Auction channel is now OFF.\n",ch);
		SET_BIT(ch->comm,COMM_NOAUCTION);
		return;
	}

	if (IS_IMMORTAL(ch) && !str_cmp(arg1, "stop")) {
		if (auction.item == NULL) {
			char_puts("There is no auction going on "
				  "you can stop.\n",ch);
			return;
		}
		else { /* stop the auction */
			talk_auction("Sale of %s has been stopped "
				     "by an Immortal.",
				     fix_short(mlstr_mval(auction.item->short_descr)));
			auction_give_obj(auction.seller, auction.item);
			auction.item = NULL;

			/* return money to the buyer */
			if (auction.buyer != NULL) {
				auction.buyer->gold += auction.bet;
				char_puts("Your money has been returned.\n",
					  auction.buyer);
	    		}

			/* return money to the seller */
			if (auction.seller != NULL) {
				auction.seller->gold +=
					((auction.starting * 10) / 100) == 0 ? 1 :
					 (auction.starting * 10) / 100;
				char_puts("Your money has been returned.\n",
					  auction.seller);
			}
	    		return;
		}
	}

	if (!str_cmp(arg1, "bet")) {
	        int newbet;

		if (auction.item == NULL) {
	        	char_puts ("There isn't anything being auctioned "
				      "right now.\n",ch);
	        	return;
		}

		if (ch == auction.seller) {
			char_puts("You cannot bet on your own "
				     "equipment...:)\n",ch);
			return;
		}

	        /* make - perhaps - a bet now */
	        if (argument[0] == '\0') {
			char_puts ("Bet how much?\n",ch);
			return;
	        }

		newbet = parsebet (auction.bet, argument);

	        if (newbet > ch->gold) {
	        	char_puts("You don't have that much money!\n", ch);
	        	return;
	        }

		if (auction.bet > 0) {
			if (newbet < auction.bet + 1) {
				char_puts("You must bid at least 1 gold "
					  "over the current bet.\n", ch);
	        		return;
	        	}
		}
		else {
			if (newbet < auction.starting) {
				char_puts("You cannot bid less than the "
					  "starting price.\n", ch);
				return;
			}
		}

	        /* the actual bet is "Ok.\n"! */

	        /* return the gold to the last buyer, if one exists */
	        if (auction.buyer != NULL)
	        	auction.buyer->gold += auction.bet;

	        ch->gold -= newbet; /* substract the gold - important :) */
	        auction.buyer = ch;
	        auction.bet   = newbet;
	        auction.going = 0;
	        auction.pulse = PULSE_AUCTION; /* start the auction over again */

	        talk_auction("A bet of %d gold has been received on %s.",
			     newbet,
			     fix_short(mlstr_mval(auction.item->short_descr)));
	        return;
	}

	/* finally... */

	obj = get_obj_carry (ch, arg1); /* does char have the item ? */ 

	if (obj == NULL) {
		char_puts("You aren't carrying that.\n",ch);
		return;
	}

	if (obj->timer > 0) {
		char_puts("You cannot auction decaying objects.\n", ch);
		return;
	}

	if (auction.item != NULL) {
		act ("Try again later - $p is being auctioned right now!",
		     ch, auction.item, NULL, TO_CHAR);
		return;
	}

	argument = one_argument(argument, starting, sizeof(starting));
	if (starting[0] == '\0')
		auction.starting = MIN_START_PRICE;
	else if ((auction.starting = atoi(starting)) < MIN_START_PRICE) {
		char_printf(ch, "You must specify the starting price (at least %d gold).\n", MIN_START_PRICE);
		return;
	}

	switch (obj->pIndexData->item_type) {
	default:
		act_puts("You cannot auction $T.",
			 ch, NULL,
			 flag_string(item_types, obj->pIndexData->item_type),
			 TO_CHAR, POS_SLEEPING);
		break;

	case ITEM_LIGHT:
	case ITEM_WEAPON:
	case ITEM_ARMOR:
	case ITEM_STAFF:
	case ITEM_WAND:
	case ITEM_GEM:
	case ITEM_TREASURE:
	case ITEM_JEWELRY:
	case ITEM_FURNITURE:
	case ITEM_FOOD:
	case ITEM_POTION:
	case ITEM_PILL:
		if (!(GET_CHAR_RELIGION(ch)
			&& IS_SET(GET_CHAR_RELIGION(ch)->flags, RELIG_AUC_FREE)))
		{
			tax = auction.starting * 0.05;
			tax = (tax == 0 ? 1 : tax);
			if (ch->gold < tax) {
				char_printf(ch, "You do not have enough gold to pay "
						"an auction fee of %d gold.\n", tax);
				return;
			}

			char_printf(ch, "The auctioneer charges you an auction fee "
					"of %d gold (10%%).\n", tax);
			ch->gold -= tax;
		}

		obj_from_char(obj);
		auction.item = obj;
		auction.bet = 0; 	/* obj->cost / 100 */
		auction.buyer = NULL;
		auction.seller = ch;
		auction.pulse = PULSE_AUCTION;
		auction.going = 0;

		talk_auction("A new item has been received: {Y%s{x.",
			     fix_short(mlstr_mval(obj->short_descr)));
		break;
	} /* switch */
}
