/*
 * Copyright (c) 1999, Molchanov Alexander (Xorader),
 * as 'Shadow Realms' program software.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the conditions specified in
 * a file doc/SR/sr.license are met!
 */

#ifndef _COMM_H_
#define _COMM_H_

#include <libmnet.h>

extern DESCRIPTOR_DATA_MUDDY * descriptor_list_muddy;

struct descriptor_data_muddy
{
	DESCRIPTOR_DATA_CLIENT	* client;
	int			connected;
	CHAR_DATA		* character;
	CHAR_DATA		* original;
	DESCRIPTOR_DATA_MUDDY	* snoop_by;
	DESCRIPTOR_DATA_MUDDY	* next;

	bool			needPrompt;

	/* OLC stuff */
	olced_t			* olced;
	void			* pEdit;	/* edited obj	*/
	void			* pEdit2;	/* edited obj 2	*/

	/* for Page buffer */
	const char *		showstr_head;
	const char *		showstr_point;

	/* string editor stuff */
	const char **		pString;	/* edited string	*/
	const char *		backup;		/* backup		*/
};

//#define		FDDF_NONE		(0)
//#define		FDDF_ALL		(A)
//#define		FDDF_NEWCONNECT		(B)
//#define		FDDF_NEEDCLOSE		(C)
//#define	FDDF_ALL_CHECK		(D)	/* init/close new connections ! */

/*
typedef struct find_descriptor_data
{
	DESCRIPTOR_DATA_CLIENT	* curr_client;
	DESCRIPTOR_DATA_TUNNEL	* curr_tunnel;
	flag32_t		flags;
} FIND_DESCRIPTOR_DATA;
*/

void	show_string	(DESCRIPTOR_DATA_MUDDY *d, char *input);
//void	will_close_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *dclose);
void	write_to_buffer_muddy(DESCRIPTOR_DATA_MUDDY *d, const char *txt, unsigned int length);
//bool	write_to_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *d, const char *txt, int length);
DESCRIPTOR_DATA_MUDDY *new_descriptor_muddy(void);
//void	free_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *d);
void	close_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *dclose);


//FIND_DESCRIPTOR_DATA * init_desc_find(FIND_DESCRIPTOR_DATA * find_d, flag32_t flags);
//DESCRIPTOR_DATA_MUDDY * get_first_desc_find(FIND_DESCRIPTOR_DATA * find_d);
//DESCRIPTOR_DATA_MUDDY * get_next_desc_find(FIND_DESCRIPTOR_DATA * find_d);

void	char_puts(const char *txt, CHAR_DATA *ch);
#define char_nputs(msgid, ch) char_puts(msg(msgid, ch), ch)
#define char_mlputs(m, ch) char_puts(mlstr_cval(m, ch), ch)
void	char_printf(CHAR_DATA *ch, const char *format, ...);

void	send_to_char(const char *txt, CHAR_DATA *ch);
void	page_to_char( const char *txt, CHAR_DATA *ch);

void 	class_table_show(CHAR_DATA *ch);
void 	race_table_show(CHAR_DATA *ch);
bool	show_last(BUFFER *output, CHAR_DATA *ch);

void telnet_flags_check(CHAR_DATA *ch, DESCRIPTOR_DATA_CLIENT * client);

/*
 * configuration parameters
 */
extern varr client_sockets;
extern varr mcontrol_sockets;
extern varr control_trusted;

/* mud server options (etc/system.conf) */
//#define OPT_ASCII_ONLY_NAMES	(A)

//extern flag32_t mud_options;


#endif

