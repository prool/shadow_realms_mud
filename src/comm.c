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
 *  Merc Diku vMud improvments copyright (C) 1992, 1993 by Michael          *
 *  Chastain, Michael Quan, and Mitchell Tse.                              *
 *                                                                         *
 *  In order to use any part of this Merc Diku Mud, you must comply with   *
 *  both the original Diku license in 'license.doc' as well the Merc       *
 *  license in 'license.txt'.  In particular, you may not remove either of *
 *  these copyright notices.                                               *
 *                                                                         *
 *  Thanks to abaddon for proof-reading our comm.c and pointing out bugs.  *
 *  Any remaining bugs are, of course, our work, not his.  :)              *
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

/*
 * This file contains all of the OS-dependent stuff:
 *   startup, BSD sockets for tcp/ip, i/o, timing.
 *
 * The data flow for input is:
 *    Game_loop ---> Read_from_descriptor ---> Read
 *    Game_loop ---> Read_from_buffer
 *
 * The data flow for output is:
 *    Game_loop ---> Process_Output ---> Write_to_descriptor -> Write
 *
 * The OS-dependent functions are Read_from_descriptor and Write_to_descriptor.
 * -- Furey  26 Jan 1993
 */

#include <sys/types.h>
#if	!defined(WIN32)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/telnet.h>
#	include <arpa/inet.h>
#	define __USE_XOPEN_EXTENDED		// else: implicit declaration of function `getsid'
#	include <unistd.h>
#	include <netdb.h>
#	include <sys/wait.h>
#else
#	include <winsock.h>
#	include <sys/timeb.h>
#endif

#include <sys/time.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <locale.h>
#include <stdarg.h>   
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>

#if defined (USE_THREADS)
#include <pthread.h>
typedef void *(*thr_cb)(void *);
#endif

#if (defined(SUNOS) || defined(SVR4) || defined(LINUX)) && defined(CRYPT)
#	include <crypt.h>
#endif

#include "merc.h"
#include "interp.h"
#include "quest.h"
#include "update.h"
#include "access.h"
#include "charset.h"
//#include "mgater.h"

/*
#if !defined (USE_THREADS)
#include <resolver.h>
#endif
*/

#include "olc.h"
#include "comm_control.h"
#include "comm_colors.h"
#include "lang.h"

//DESCRIPTOR_DATA	*	new_descriptor	(void);
//void			free_descriptor	(DESCRIPTOR_DATA *d);

/*
struct codepage {
	char* name;
	unsigned char* from;
	unsigned char* to;
};

struct codepage codepages[] = {
	{ "koi8-r",		koi8_koi8,	koi8_koi8	},
	{ "alt (cp866)",	alt_koi8,	koi8_alt	},
	{ "win (cp1251)",	win_koi8,	koi8_win	},
	{ "iso (ISO-8859-5)",	iso_koi8,	koi8_iso	},
	{ "mac",		mac_koi8,	koi8_mac	},
	{ "translit",		koi8_koi8,	koi8_vola	},
};
#define NCODEPAGES (sizeof(codepages) / sizeof(struct codepage))
*/

//const char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
//const char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
//const char 	go_ahead_str	[] = { IAC, GA, '\0' };

/*
 * Global variables.
 */

DESCRIPTOR_DATA_MUDDY	* descriptor_list_muddy;	/* All open muddy descriptors	*/
//DESCRIPTOR_DATA *	d_next;		/* Next descriptor in loop	*/

int			muddy_clients;	/* ID Local clients of muddy */
int			muddy_control;	/* ID Local control sockets  */

bool			merc_down;		/* Shutdown		*/
bool			wizlock;		/* Game is wizlocked	*/
bool			newlock;		/* Game is newlocked	*/
char			str_boot_time[26];
time_t			current_time;	/* time of this pulse */	
int			iNumPlayers = 0; /* The number of players on */
//FILE			*logfile;
int			debug_level = 0;

//char			command[MAX_INPUT_LENGTH];

extern int		max_on;

/* Declare functions */
//int	init_socket		(int port);
void	process_who		(int port);

DESCRIPTOR_DATA_MUDDY	*init_descriptor_muddy(DESCRIPTOR_DATA_CLIENT *client);
void destroy_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *dclose);
bool process_output_muddy(DESCRIPTOR_DATA_MUDDY *d, bool fPrompt);

//bool	read_from_descriptor	(DESCRIPTOR_DATA *d);
	/* Search this in mgater.c */
//bool	write_to_descriptor	(int desc, char *txt, uint length);
void	game_loop_unix		(void);
#if !defined (USE_THREADS)
//void	resolv_check		(void);
#endif

/*
 * Other local functions (OS-independent).
 */
bool	check_reconnect		(DESCRIPTOR_DATA_MUDDY *d, /*const char *name,*/
				 bool fConn);
bool	check_playing		(DESCRIPTOR_DATA_MUDDY *d, const char *name);
int	main			(int argc, char **argv);
void	nanny			(DESCRIPTOR_DATA_MUDDY *d, const char *argument);
bool	process_output		(DESCRIPTOR_DATA_MUDDY *d, bool fPrompt);
//void	read_from_buffer	(DESCRIPTOR_DATA *d);
void	stop_idling		(CHAR_DATA *ch);
void    bust_a_prompt           (CHAR_DATA *ch);
void 	log_area_popularity	(void);

//char    *get_multi_command      args((DESCRIPTOR_DATA_MUDDY *d, char *argument, bool addincomm));

#if defined (USE_THREADS)
void *resolver(DESCRIPTOR_DATA *);
#endif


varr 	client_sockets = { sizeof(struct sockaddr_in), 2 };
varr	mcontrol_sockets = { sizeof(struct sockaddr_in), 2 };
varr	control_trusted = { sizeof(struct in_addr), 2 };

#if 0
void echo_off_string(CHAR_DATA *ch)
{
	/* may be without compression ?!*/
	write_to_descriptor(ch->desc, echo_off_str, 0);
}
#endif

static void usage(const char *name)
{
	fprintf(stderr, "Usage: %s [-i logfile] [-p port...] [-i port...]\n"
			"Where:\n"
			"\t-p <port>   -- listen port\n"
			"\t-c <cport>  -- control service port\n"
			"\t-l <file>   -- logfile\n"
			"\t-d <level>  -- turn on debug (tmp/debug.log)\n"
			"\t-u <user>   -- set user (and e_user) of proc\n"
			"\t-a          -- log all\n"
			"\t-t <tunnel> -- listening new tunnels\n",
		get_filename(name));
	exit(1);
}

#if 0
#define GETINT(v, i) (*(int*) VARR_GET(v, i))
static void open_sockets(varr *v, const char *logm)
{
	int i, j;

	for (i = 0, j = 0; i < v->nused; i++) {
		int port = GETINT(v, i);
		int sock;
		if ((sock = init_socket(port)) < 0)
			continue;
		log_printf(logm, port);
		GETINT(v, j++) = sock;
	}
	v->nused = j;
}

void close_sockets(varr *v)
{
	int i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
	}
}
#endif

int main(int argc, char **argv)
{
	struct timeval now_time;
	int ch;
	int check_info;
	int ltunnels = -1;

#if defined WIN32
	WORD	wVersionRequested = MAKEWORD(1, 1);
	WSADATA	wsaData;
	int err;
#else
	pid_t	pid;
	uid_t	uid;
	uid_t	muddy_uid = 0;
//	pid_t	gid;
//	pid_t	sid;
#endif

#if !defined(WIN32)
	setsid();

	pid = getpid();
/*	gid = getpgrp();
 *	sid = getsid(0);	// may be:   getsid(pid);
 */
	logfile = NULL;
	//initlogfile(path, filename);

	log_printf("Muddy pid is %d", pid);
	if (dfexist(TMP_PATH, PID_FILE))
	{
		log_printf("Error. Pid file already exits. Remove it before start muddy.");
		log_printf("%s/%s", TMP_PATH, PID_FILE);
		exit(1);
	} else {
		FILE *fp = dfopen(TMP_PATH, PID_FILE, "w");
		if (!fp) {
			log_printf("Error: cann't create pid file: %s.",
				strerror(errno));
		} else {
			fprintf(fp, "%d", pid);
			fclose(fp);
		}
	}

#endif
	/*
	 * Memory debugging if needed.
	 */
#if defined(MALLOC_DEBUG)
	malloc_debug(2);
#endif

	setlocale(LC_ALL, "");

	/*
	 * Init time.
	 */
	gettimeofday(&now_time, NULL);
	current_time 	= (time_t) now_time.tv_sec;
	strnzcpy(str_boot_time, sizeof(str_boot_time), strtime(current_time));

	/*
	 * Run the game.
	 */
	
#if defined (WIN32)
	srand((unsigned) time(NULL));
	err = WSAStartup(wVersionRequested, &wsaData); 
	if (err) {
		log_printf("winsock.dll: %s", strerror(errno));
		exit(1);
	}
#endif

	if (argc > 1) {
		opterr = 0;
		while ((ch = getopt(argc, argv, "ad:l:u:p:c:t:")) != -1) {
			//int *p;

			switch (ch) {
			case 'a':
				fLogAll = TRUE;
				break;
			case 'p':
				// command line parameters override configuration settings
				client_sockets.nused = 0;
				
				if (!is_number(optarg))
					usage(argv[0]);
				add_simpleport(&client_sockets, atoi(optarg));
				//p = varr_enew(&client_sockets);
				//*p = atoi(optarg);
				break;
			case 't':
				if (!is_number(optarg))
					usage(argv[0]);
				ltunnels = atoi(optarg);
				break;
			case 'c':
				mcontrol_sockets.nused = 0;
				if (!is_number(optarg))
					usage(argv[0]);
				add_simpleport(&mcontrol_sockets, atoi(optarg));
				//p = varr_enew(&mcontrol_sockets);
				//*p = atoi(optarg);
				break;
			case 'l':
				//logfile = dfopen(LOG_PATH, optarg, "w");
				//if (!logfile)
				if (!initlogfile(LOG_PATH, optarg))
					usage(argv[0]);
				break;
			case 'd':
				debug_level = atoi(optarg);
				break;
			case 'u':
				{
					struct passwd *spass;
					
					errno = 0;
					if ((spass = getpwnam(optarg)) == NULL
					|| (muddy_uid = spass->pw_uid) <= 0)
					{
						log_printf("some error user %s:"
							" %s", optarg,
							strerror(errno));
					}
				}
				break;
			default:
				usage(argv[0]);
			}
		}
		argc -= optind;
		argv += optind;
	}

#if !defined (WIN32) && !defined (USE_THREADS)
	resolver_init(muddy_uid);
#endif

	boot_db_system();

	init_debug_log();

	if (!client_sockets.nused) {
		log_printf("no client sockets defined");
		exit(1);
	}
	check_info = (!!mcontrol_sockets.nused);
	if (!inithid(ETC_PATH, HID_FILE))
	{
		log_printf("can't init hid db");
		exit(1);
	}
	
//	open_sockets(&control_sockets, "ready to rock on port %d");
//	open_sockets(&info_sockets, "info service started on port %d");

	muddy_clients = init_local_clients_tunnel(&client_sockets,
			"for muddy clients");
	muddy_control = init_local_clients_tunnel(&mcontrol_sockets,
			"control service");
	if (ltunnels > 0)
	{
		init_tunnels_listening(ltunnels, NULL);
	}

	if (muddy_clients < 0) {
		log_printf("no sockets for muddy clients could be opened.");
		exit(1);
	}

	if (check_info && muddy_control < 0) {
		log_printf("no control service sockets could be opened.");
		exit(1);
	}
	
//	if ((uid = geteuid()) == 0)	// root
	if (muddy_uid > 0)
	{
		uid = getuid();
		if (setreuid(muddy_uid, muddy_uid) < 0)
		{
			log_printf("Error to change uid(euid)[%d] from %d: %s",
				muddy_uid, uid, strerror(errno));
			exit(1);
		}
	}

	boot_db();

	if (muddy_control >= 0) {
		int i;
		DESCRIPTOR_DATA_TUNNEL *dt;
		for (i = 0; i < control_trusted.nused; i++)
		{
			struct in_addr* in_addr = VARR_GET(&control_trusted, i);
			tunnel_add_to_trusted_addr(muddy_control, in_addr);
		}

		/* disable some TELNET parsing */
		dt = VARR_GET(&tunnel_list, muddy_control);
		SET_BIT(dt->flags, DDTF_RAW_DATA);
	}

	if (!begin_work_with_tunnel(muddy_clients)
	|| (muddy_control >= 0 && !begin_work_with_tunnel(muddy_control))
	|| (ltunnels > 0 && !begin_work_with_ltunnels()))
	{
		exit(1);
	}

	game_loop_unix();

//	close_sockets(&control_sockets);
//	close_sockets(&info_sockets);
	shutdown_tunnel(muddy_clients);
	if (muddy_control >= 0)
		shutdown_tunnel(muddy_control);
	

#if defined (WIN32)
	WSACleanup();
#elif !defined (USE_THREADS)
	resolver_done();
#endif
	log_area_popularity();
	save_time();

	/*
	 * That's all, folks.
	 */
	log("Normal termination of game.");
	shutdownlog();
	if (dunlink(TMP_PATH, PID_FILE) < 0)
		log_printf("Cann't remove pid file: %s", strerror(errno));
	if (dunlink(TMP_PATH, DEBUG_LOG) < 0)
		log_printf("Cann't remove debug file '%s': %s",
		DEBUG_LOG, strerror(errno));
	done_debug_log();
	return 0;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA_MUDDY *descriptor_muddy_free;

DESCRIPTOR_DATA_MUDDY *new_descriptor_muddy(void)
{
	DESCRIPTOR_DATA_MUDDY *d;

	if (descriptor_muddy_free == NULL) {
		d = malloc(sizeof(*d));
		if (d == NULL)
			crush_mud();
	} else {
		d = descriptor_muddy_free;
		descriptor_muddy_free = descriptor_muddy_free->next;
	}

	memset(d, 0, sizeof(*d));
	return d;
}

void free_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *d)
{
	if (!d)
		return;

//	free_string(d->host);
//	free(d->outbuf);
	if (d->client)
	{
		close_descriptor_client(d->client, NULL);
		destroy_descriptor_client(d->client);
		d->client = NULL;
		//destroy_descriptor_client(d->client);
	}
	d->next = descriptor_muddy_free;
	descriptor_muddy_free = d;
}

#if 0
int init_socket(int port)
{
	static struct sockaddr_in sa_zero;
	struct sockaddr_in sa;
	struct linger ld;
	int x = 1;
	int fd;

	debug_printf(5, "init_socket[21]");  
#if defined (WIN32)
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
#else
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
		log_printf("init_socket(%d): socket: %s",
			   port, strerror(errno));
		return -1;
	}

	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
		       (char *) &x, sizeof(x)) < 0) {
		log_printf("init_socket(%d): setsockopt: SO_REUSEADDR: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	ld.l_onoff  = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
		       (char *) &ld, sizeof(ld)) < 0) {
		log_printf("init_socket(%d): setsockopt: SO_LINGER: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	sa		= sa_zero;
#if !defined (WIN32)
	sa.sin_family   = AF_INET;
#else
	sa.sin_family   = PF_INET;
#endif
	sa.sin_port	= htons(port);

	if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
		log_printf("init_socket(%d): bind: %s", port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	if (listen(fd, 3) < 0) {
		log_printf("init_socket(%d): listen: %s",
			   port, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return -1;
	}

	debug_printf(5, "end init_socket[21]");
	return fd;
}
#endif

#if 0
static void add_fds(varr *v, fd_set *in_set, int *maxdesc)
{
	int i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
		FD_SET(fd, in_set);
		if (*maxdesc < fd)
			*maxdesc = fd;
	}
}

static void check_fds(varr *v, fd_set *in_set, void (*new_conn_cb)(int))
{
	int i;

	for (i = 0; i < v->nused; i++) {
		int fd = GETINT(v, i);
		if (FD_ISSET(fd, in_set))
			new_conn_cb(fd);
	}
}
#endif

/*
 *	used in interp.c and here 
 */
char *get_multi_command( DESCRIPTOR_DATA_MUDDY *d, char *argument, bool addincomm)
{
    int counter, counter2;
    char leftover[MAX_INPUT_LENGTH];
    static char command[MAX_INPUT_LENGTH];

    command[0] = '\0';
    
    for ( counter = 0; argument[counter] != '\0' && counter < (MAX_INPUT_LENGTH - 1); counter++ ) {
        if ( argument[counter] == '|' &&  argument[counter+1] != '|' ) {
            command[counter] = '\0';
            counter++;
            for (counter2 = 0; argument[counter] != '\0'; counter2++,counter++)
                leftover[counter2] = argument[counter];
            leftover[counter2] = '\0';
            if (addincomm && d->client->inlength)
	    {
		strnzcat(leftover, sizeof(leftover), "|");
		strnzcat(leftover, sizeof(leftover), (char*)d->client->inarray);
            }
	    strnzcpy((char*)d->client->inarray, sizeof(d->client->inarray), leftover);
	    d->client->inlength = strlen((char*)d->client->inarray);
            return (command);
        } else {
            if (argument[counter] == '|' && argument[counter+1] == '|')
                for (counter2 = counter; argument[counter2] != '\0'; counter2++)
                    argument[counter2] = argument[counter2+1];
        }
        
        command[counter] = argument[counter];

    }
    command[counter] = '\0';
    if (!addincomm)
    {
	d->client->inlength = 0;
	//d->incomm[0] = '\0';
    }
    return (command);
}

void game_loop_unix(void)
{
//	static struct timeval null_time = {0, 0};
	struct timeval last_time;
	DESCRIPTOR_DATA_MUDDY	* dm, * dm_next;
//	DESCRIPTOR_DATA_TUNNEL	* dt;
	DESCRIPTOR_DATA_CLIENT	* client, * dc_next;
 
	gettimeofday(&last_time, NULL);
	current_time = (time_t) last_time.tv_sec;

	/* Main loop */
	while (!merc_down) {
		CONTROL_DESC * c_id, * c_id_next;

#if defined(MALLOC_DEBUG)
		if (malloc_verify() != 1)
			abort();
#endif

		if (!process_in_all_tunnels())
		{
			log_printf("game_loop_unix: network error (in)");
			exit(1);
		}


			/* resolving hostnames (see resolver.c other) */
		//resolv_check();

		/* check new connections of clients */
		for (client = ((DESCRIPTOR_DATA_TUNNEL *)
			VARR_GET(&tunnel_list, muddy_clients))->clients;
				client; client = dc_next)
		{
			dc_next = client->next;
			if (IS_SET(client->flags, DDCF_NEWCONNECT))
			{
				dm = init_descriptor_muddy(client);
			}
		}

		/* new control sockets process */
		if (muddy_control >= 0)
		{
			for (client = ((DESCRIPTOR_DATA_TUNNEL *)
			VARR_GET(&tunnel_list, muddy_control))->clients;
				client; client = dc_next)
			{
				dc_next = client->next;
				if (IS_SET(client->flags, DDCF_NEWCONNECT))
				{
					control_newconn(client);
				}
			}
		}

		/*
		 *	Work with control sockets
		 */
		for (c_id = ctrl_list; c_id; c_id = c_id_next)
		{
			c_id_next	= c_id->next;
			/*
			 *	Kick out the freaky folks (control)
			 */
			if (IS_SET(c_id->client->flags, DDCF_NEEDCLOSE)
			|| c_id->client->connect == NULL)
			{
				control_desc_fulldestroy(c_id);
				continue;
			}
			if (c_id->client->inlength)
			{
				control_process_cmd(c_id, (char*)c_id->client->inarray);
				c_id->client->inlength = 0;
			}
		}
		
		/*
		 * Process input.
		 */
		for (dm = descriptor_list_muddy; dm; dm = dm_next) {
			dm_next		= dm->next;
			//dm->fcommand	= FALSE;
			dm->needPrompt	= FALSE;
			
			/*
			 * Kick out the freaky folks (1).
			 */
			if (IS_SET(dm->client->flags, DDCF_NEEDCLOSE)
			|| dm->client->connect == NULL)
			{
				if (dm->character
				&& dm->character->level > 1)
					save_char_obj(dm->character, FALSE);
				//d->outtop = 0;
				destroy_descriptor_muddy(dm);
				continue;
			}
			/*
			if (FD_ISSET(d->descriptor, &in_set)) {
				if (d->character != NULL)
					d->character->timer = 0;

				if (!read_from_descriptor(d)) {
					FD_CLR(d->descriptor, &out_set);
					if (d->character != NULL
					&&  d->character->level > 1)
						save_char_obj(d->character,
							      FALSE);
					d->outtop = 0;
					close_descriptor(d);
					continue;
				}
			}
			*/

			/*	moved to: aggr_update() "update.c"
			if (dm->character && dm->character->daze > 0)
				--dm->character->daze;
			*/

			if (dm->character && dm->character->wait > 0)
			{
				--dm->character->wait;

				if (IS_SET(dm->character->comm, COMM_PROMPT)
				&& IS_SET(dm->character->comm, COMM_SYNCPROMPT)
				&& dm->character->wait == 0)
					dm->needPrompt = TRUE;
					
				continue;
			}

			
			//read_from_buffer(d);
			if (dm->connected == CON_RESOLV)
			{
				nanny(dm, str_empty);
			} else if (dm->client->inlength)
			{
				/*
				log_printf("inar: %s (%d: %d %d %d)",
						dm->client->inarray,
						dm->client->inlength,
						dm->client->inarray[0],
					dm->client->inlength > 0 ? dm->client->inarray[1] : -1,
					dm->client->inlength > 1 ? dm->client->inarray[2] : -1);
				*/
				dm->needPrompt = TRUE;
				if (dm->character)
				{
					dm->character->timer = 0;
					stop_idling(dm->character);
				}

				if (dm->showstr_point)
				{
					show_string(dm, (char*)dm->client->inarray);
					dm->client->inlength = 0;
				} else if (dm->pString)
				{
					string_add(dm->character,
						(char*)dm->client->inarray);
					dm->client->inlength = 0;
				} else if (dm->connected == CON_PLAYING)
				{
					if (!run_olc_editor(dm))
					{
						char *command2;

						command2 = get_multi_command(
							dm, (char*)dm->client->inarray,
							FALSE);
			    			substitute_alias(dm, command2);
			    			continue;
			    		} else
						dm->client->inlength = 0;
				} else {
					nanny(dm, (char*)dm->client->inarray);
					//dm->client->inlength = 0;
				}

				//if (d->connected != CON_RESOLV)
				//	d->incomm[0]	= '\0';
			}
		}

		/*
		 * Autonomous game motion.
		 */
		update_handler();
			/* Время уже приросло, так что все действия в
			 * эту пульсацию завершились !
			 */

		/*
		 * Output.
		 */
		for (dm = descriptor_list_muddy; dm; dm = dm_next)
		{
			dm_next = dm->next;
			if (check_out_ready_client(dm->client)
			|| dm->needPrompt)
				process_output_muddy( dm, TRUE );
		}
#if 0
		for (dm = descriptor_list_muddy; dm; dm = dm_next)
		{
			dm_next = dm->next;

			if ((d->fcommand || d->outtop > 0
#ifndef NOMGATER
			|| (d->out_compress
				&& d->out_compress->next_out != d->out_compress_buf)
#endif
			) &&  FD_ISSET(d->descriptor, &out_set))
			{
				if ( !process_output( d, TRUE ))
				{
#ifndef NOMGATER
					bool ok = TRUE;

					if ( d->fcommand || d->outtop > 0 )
						ok = process_output( d, TRUE );
					if (ok && d->out_compress)
						ok = processCompressed(d);
					
					if (!ok) {
#endif
						if (d->character != NULL
						&&  d->character->level > 1)
							save_char_obj(d->character, FALSE);
						d->outtop = 0;
						close_descriptor(d);
#ifndef NOMGATER
					}
#endif
				}
			}
		}
#endif
		if (!process_out_all_tunnels())
		{
			log_printf("game_loop_unix: network error (out)");
			exit(1);
		}

		/*
		 * Kick out the freaky folks (2).
		 */
		for (dm = descriptor_list_muddy; dm; dm = dm_next)
		{
			dm_next = dm->next;   
			if (IS_SET(dm->client->flags, DDCF_NEEDCLOSE)
			|| dm->client->connect == NULL)
			{
				if (dm->character
				&& dm->character->level > 1)
					save_char_obj(dm->character, FALSE);
				destroy_descriptor_muddy(dm);
			}
		}
			

	/*
	 * Synchronize to a clock.
	 * Sleep(last_time + 1/PULSE_PER_SCD - now).
	 * Careful here of signed versus unsigned arithmetic.
	 */
			/*
			 *	Есть идея сделать ускорение/замедление
			 *	времени при его отставании/убегании
			 *	от реальных секунд.
			 */

#if !defined (WIN32)
	 {
	    struct timeval now_time;
	    long secDelta;
	    long usecDelta;

	    gettimeofday(&now_time, NULL);
	    usecDelta	= ((int) last_time.tv_usec) - ((int) now_time.tv_usec)
			+ 1000000 / PULSE_PER_SCD;
	    secDelta	= ((int) last_time.tv_sec) - ((int) now_time.tv_sec);
	    while (usecDelta < 0) {
		usecDelta += 1000000;
		secDelta  -= 1;
	    }

	    while (usecDelta >= 1000000) {
		usecDelta -= 1000000;
		secDelta  += 1;
	    }

	    if (secDelta > 0 || (secDelta == 0 && usecDelta > 0)) {
		struct timeval stall_time;

		stall_time.tv_usec = usecDelta;
		stall_time.tv_sec  = secDelta;
		if (select(0, NULL, NULL, NULL, &stall_time) < 0) {
		    log_printf("game_loop: select: stall: %s", strerror(errno));
		    exit(1);
		}
	    }
	}
#else
	{
	    int times_up;
	    int nappy_time;
	    struct _timeb start_time;
	    struct _timeb end_time;
	    _ftime( &start_time );
	    times_up = 0;

	    while( times_up == 0 )
	    {
			_ftime( &end_time );
			if ( ( nappy_time = (int) ( 1000 * (double) ( ( end_time.time - start_time.time ) +
				       ( (double) ( end_time.millitm - start_time.millitm ) /
					1000.0 ) ) ) ) >= (double)( 1000 / PULSE_PER_SCD ) )
			  times_up = 1;
		else
		{
		    Sleep( (int) ( (double) ( 1000 / PULSE_PER_SECOND ) -
				  (double) nappy_time ) );
		    times_up = 1;
		}
	  }
	}
#endif
		gettimeofday(&last_time, NULL);
		current_time = (time_t) last_time.tv_sec;
	}
}

static void cp_print(DESCRIPTOR_DATA_MUDDY* d)
{
	char buf[MAX_STRING_LENGTH];
	int i;

	write_to_buffer_muddy(d, "\n\r", 0);
	for (i = 0; i < NCODEPAGES; i++) {
		snprintf(buf, sizeof(buf), "%s%d. %s",
			 i ? " " : "", i+1, codepages[i].name);
		write_to_buffer_muddy(d, buf, 0);
	}
	write_to_buffer_muddy(d, "\n\rSelect your codepage (non-russian players should choose translit): ", 0);
}

#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

//void init_descriptor(int control)
DESCRIPTOR_DATA_MUDDY *init_descriptor_muddy(DESCRIPTOR_DATA_CLIENT *client)
{
	DESCRIPTOR_DATA_MUDDY *dnew;
//	struct sockaddr_in sock;
//	int desc;
//	int size;
	HELP_DATA *greeting;

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor_muddy();

	dnew->client		= client;
	dnew->connected		= CON_GET_CODEPAGE;

	dnew->showstr_head	= NULL;
	dnew->showstr_point	= NULL;

	dnew->pString		= NULL;
	dnew->backup		= NULL;

	dnew->olced		= NULL;
	dnew->pEdit		= NULL;
	dnew->pEdit2		= NULL;

	//dnew->codepage	= codepages;
	//dnew->host		= NULL;

#if defined (USE_THREADS)
	pthread_mutex_init(&dnew->mutex, NULL);
#endif

//	log_printf("sock.sinaddr: %s", inet_ntoa(sock.sin_addr));

	dnew->next		= descriptor_list_muddy;
	descriptor_list_muddy	= dnew;

	/*
	 * Send the greeting.
	 */
	if ((greeting = help_lookup(1, "GREETING"))) {
		char buf[MAX_STRING_LENGTH];
		parse_colors(mlstr_mval(greeting->text), buf, sizeof(buf),
			     FORMAT_DUMB);
		write_to_buffer_muddy(dnew, buf + (buf[0] == '.'), 0);
	}
	cp_print(dnew);
	REMOVE_BIT(client->flags, DDCF_NEWCONNECT);
	return dnew;
}

void save_finger_char(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA_MUDDY *d;
	PC_DATA *pcdata;

	if (ch == NULL || (d = ch->desc) == NULL)
		return;
	pcdata = ch->pcdata;

	if (ch->desc->original)
		pcdata = ch->desc->original->pcdata;
	if (pcdata)
	{
		pcdata->last_logout = current_time;
		free_string(pcdata->last_host);
		if (d->client && d->client->hostname)
		{
			//pcdata->last_host = str_dup(get_hostname(d->client));
			pcdata->last_host = str_dup(d->client->hostname);
		}
	}

}

void close_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *dclose)
{
	if (dclose->client && dclose->client->connect)
	{
		close_descriptor_client(dclose->client, NULL);
	}
	dclose->connected = CON_NULL;
}

void destroy_descriptor_muddy(DESCRIPTOR_DATA_MUDDY *dclose)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA_MUDDY *d;

//	if (dclose->outtop > 0)
//		process_output(dclose, FALSE);

	if (dclose->snoop_by) 
		write_to_buffer_muddy(dclose->snoop_by,
				"Your victim has left the game.\n\r", 0);

	for (d = descriptor_list_muddy; d; d = d->next)
		if (d->snoop_by == dclose)
			d->snoop_by = NULL;

	if ((ch = dclose->character)) {
		if (dclose->connected != CON_NULLLOOP)
		{
			save_finger_char(ch);
			log_printf("Closing link to %s.", ch->name);
		}
		switch(dclose->connected)
		{
			case CON_PLAYING:
				act("$n has lost $s link.", ch, NULL, NULL, TO_ROOM);
				wiznet("Net death has claimed $N.", ch, NULL,
				       WIZ_LINKS, 0, 0);
				ch->desc = NULL;
				break;
			case CON_REMORT:
			case CON_DELETE:
				extract_char(dclose->character, 0);
				break;
			case CON_NULLLOOP:
				free_char(dclose->character);
				break;
			default:
				free_char(dclose->character);
				break;
		}
	}

//	if (d_next == dclose)
//		d_next = d_next->next;   

	if (dclose == descriptor_list_muddy)
		descriptor_list_muddy = descriptor_list_muddy->next;
	else {
		DESCRIPTOR_DATA_MUDDY *d;

		for (d = descriptor_list_muddy; d && d->next != dclose;
						d = d->next)
			;
		if (d)
			d->next = dclose->next;
		else
			bug("Close_socket: dclose not found.", 0);
	}

	if (dclose->client)
	{
		destroy_descriptor_client(dclose->client);
		dclose->client = NULL;
	}
	free_descriptor_muddy(dclose);
}

/*
 * Low level output function.
 */
void battle_prompt(CHAR_DATA *ch, CHAR_DATA *victim)
{
	int percent;
	char* msg;
 
        if (victim->max_hit > 0)
		percent = victim->hit * 100 / victim->max_hit;
        else
		percent = -1;
  
        if (percent >= 100)
		msg = "{Cis in perfect health{x.";
        else if (percent >= 90)
		msg = "{bhas a few scratches{x.";
        else if (percent >= 75)
		msg = "{Bhas some small but disgusting cuts{x.";
        else if (percent >= 50)
		msg = "{Gis covered with bleeding wounds{x.";
        else if (percent >= 30)
		msg = "{Yis gushing blood{x.";
        else if (percent >= 15)
		msg = "{Mis writhing in agony{x.";
        else if (percent >= 0)
		msg = "{Ris convulsing on the ground{x.";
        else
		msg = "{Ris nearly dead{x.";

	act_puts("$N $t", ch, msg, victim, TO_CHAR | ACT_TRANS, POS_DEAD);
}

/*
 * Some specials added by KIO 
 */
bool process_output_muddy(DESCRIPTOR_DATA_MUDDY *d, bool fPrompt)
{
	extern bool merc_down;
//	bool ga = FALSE;

	/*
	 * Bust a prompt.
	 */
	if (!merc_down) {
		CHAR_DATA *ch = d->character;

		if (d->showstr_point) {
			write_to_buffer_muddy(d, "[Hit Return to continue]",
					0);
			//ga = TRUE;
		}
		else if (fPrompt && d->connected == CON_PLAYING) {
			if (d->pString) {
				write_to_buffer_muddy(d, "  > ", 0);
				//ga = TRUE;
			}
			else if (ch) {
				CHAR_DATA *victim;

				/* battle prompt */
				if ((victim = ch->fighting) != NULL
				&&  can_see(ch,victim))
					battle_prompt(ch, victim);

				if (!IS_SET(ch->comm, COMM_COMPACT))
					write_to_buffer_muddy(d, "\n\r", 2);

				if (IS_SET(ch->comm, COMM_PROMPT) &&
						!(IS_SET(ch->comm, COMM_SYNCPROMPT)
						&& ch->wait > 0) )
					bust_a_prompt(d->character);
				//ga = TRUE;
			}
		}

		if ((ch == NULL || IS_SET(ch->comm, COMM_TELNET_GA))
		&& d->client)
			send_ga_string(d->client);

//		if (ch && ga && !IS_SET(ch->comm, COMM_TELNET_GA))
//			ga = FALSE;
	}

	return TRUE;
#if 0
	/*
	 * Short-circuit if nothing to write.
	 */
	if (d->outtop == 0)
		return TRUE;

	/*
	 * OS-dependent output.
	 */
	if (!write_to_descriptor(d, d->outbuf, d->outtop)) {
		d->outtop = 0;
		return FALSE;
	}
	else {
		if (ga)
			write_to_descriptor(d, go_ahead_str, 0);
		d->outtop = 0;
		return TRUE;
	}
#endif
}

void percent_hp(CHAR_DATA *ch, char buf[MAX_STRING_LENGTH])
{
	if (ch->hit > 0)
		snprintf(buf, sizeof(buf), "%d",
			 ((100 * ch->hit) / UMAX(1,ch->max_hit)));
	else
		strnzcpy(buf, sizeof(buf), "BAD!");
}

/*
 * Bust a prompt (player settable prompt)
 * coded by Morgenes for Aldara Mud
 * bust
 */
void bust_a_prompt(CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH];
	char buf2[MAX_STRING_LENGTH];
	const char *str;
	const char *i;
	char *point;
	CHAR_DATA *victim, *gch;
	EXIT_DATA *pexit;
	bool found;
	char tmpbuf[12];
	const char *dir_name[] = {"N","E","S","W","U","D"};
	int door;
 
	if (IS_SET(ch->comm, COMM_AFK)) {
		char_printf(ch, "{c<AFK>{x %s", IS_NPC(ch) ? "" : ch->pcdata->prefix);
		return;
	}

	point = buf;
	if (!IS_NPC(ch))
		if (ch->pcdata->bprompt != NULL && ch->fighting != NULL)
			str = ch->pcdata->bprompt;
		else
			str = ch->pcdata->prompt;
	else
		str = str_empty;
	
	if (IS_NULLSTR(str))
		str = DEFAULT_PROMPT;

	while (*str != '\0') {
		if (*str != '%') {
			*point++ = *str++;
			continue;
		}

		switch(*++str) {
		default:
	        	i = "";
			break;

		case 't':
			snprintf(buf2, sizeof(buf2),
				 "%d%s", (time_info.hour % 12 == 0) ? 
				 12 : time_info.hour % 12, 
				 time_info.hour >= 12 ? "pm" : "am");
			i = buf2;
			break;

		case 'e':
			found = FALSE;
			buf2[0] = '\0';
			for (door = 0; door < 6; door++)
				if ((pexit = ch->in_room->exit[door])
				&& can_see_exit(ch, pexit)
				&& (!IS_SET(pexit->exit_info, EX_HIDDEN)
					|| IS_IMMORTAL(ch))) {
					found = TRUE;
					strnzcat(buf2, sizeof(buf2),
						 dir_name[door]);
					if (IS_SET(pexit->exit_info, EX_CLOSED | EX_HIDDEN | EX_BURYED))
						strnzcat(buf2, sizeof(buf2),
							 "*");
				}
			if (buf2[0])
				strnzcat(buf2, sizeof(buf2), " ");
			i = buf2;
			break;

		case 'n':
			i = ch->name;
			break;

		case 'S':
			i = ch->sex == SEX_MALE ? "Male" :
			    ch->sex == SEX_FEMALE ? "Female" :
			    "None";
			break;

		case 'y':
			percent_hp(ch, buf2);
			strnzcat(buf2, sizeof(buf2), "%");
			i = buf2;
			break;

		case 'o':
			if ((victim = ch->fighting) != NULL) {
				if (can_see(ch, victim)) {
					percent_hp(victim, buf2);
					strnzcat(buf2, sizeof(buf2), "%");
					i = buf2;
				}
				else
					i = "???";
			}
			else
				i = "None";
			break;

		case 'h':
			snprintf(buf2, sizeof(buf2), "%d", ch->hit);
			i = buf2;
			break;

		case 'H':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_hit);
			i = buf2;
			break;

		case 'm':
			snprintf(buf2, sizeof(buf2), "%d", ch->mana);
			i = buf2;
			break;

		case 'M':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_mana);
			i = buf2;
			break;

		case 'v':
			snprintf(buf2, sizeof(buf2), "%d", ch->move);
			i = buf2;
			break;

		case 'V':
			snprintf(buf2, sizeof(buf2), "%d", ch->max_move);
			i = buf2;
			break;

		case 'A':
			if (IS_PC(ch))
			{
				if (IS_SET(ch->pcdata->plr_flags, PLR_GHOST))
					i = "{c@{z";
				else if (IS_SET(ch->pcdata->plr_flags, PLR_PUMPED))
					i = "{R${z";
				else
					i = "#";
			} else
				i = "#";
			break;

		case 'x':
			snprintf(buf2, sizeof(buf2), "%d", IS_NPC(ch) ? 0 : ch->pcdata->exp);
			i = buf2;
			break;

		case 'X':
			if (IS_PC(ch)) {
				snprintf(buf2, sizeof(buf2), "%d",
					 exp_to_level(ch));
				i = buf2;
			}
			else
				i = "-";
			break;

		case 'g':
			snprintf(buf2, sizeof(buf2), "%d", ch->gold);
			i = buf2;
			break;

		case 's':
			snprintf(buf2, sizeof(buf2), "%d", ch->silver);
			i = buf2;
			break;

		case 'a':
			i = IS_GOOD(ch) ? "good" :
			    IS_EVIL(ch) ? "evil" :
			    "neutral";
			break;

		case 'r':
			if (ch->in_room)
				i = (check_blind_raw(ch) && !room_is_dark(ch)) ?
				     mlstr_cval(ch->in_room->name, ch) :
				     "darkness";
			else
				i = "";
			break;
		case 'G':
			if (IS_AFFECTED(ch, AFF_BLIND)) {
				i = "?";
				break;
			}
			
			buf2[0]='\0';
			for (gch = char_list; gch != NULL; gch = gch->next)
			   if (is_same_group(gch, ch) && (gch != ch)) {
				tmpbuf[0] = '\0';
				snprintf(tmpbuf, sizeof(tmpbuf), "%d ", gch->hit);
				strnzcat(buf2, sizeof(buf2), tmpbuf);
			   }
			i = buf2;
			break;
		case 'q':
			snprintf(buf2, sizeof(buf2), "%d", GET_QUEST_TIME(ch));
			i = buf2; 
			break;
		
		case 'Q':
			snprintf(buf2, sizeof(buf2), "%d", ch->pcdata->questpoints);
			i = buf2; 
			break;

		case 'R':
			if (IS_IMMORTAL(ch) && ch->in_room) {
				snprintf(buf2, sizeof(buf2), "%d",
					 ch->in_room->vnum);
				i = buf2;
			}
			else
				i = "";
			break;

		case 'z':
			//	if (IS_IMMORTAL(ch) && ch->in_room != NULL)
			i = ch->in_room->area->name;
			/* else
			 *	i = "";
			 */
			break;

		case '%':
			i = "%";
			break;
		case 'f':
			if (IS_PC(ch) && ch->pcdata->religion)
			{
				snprintf(buf2, sizeof(buf2), "%d",
					ch->pcdata->religion->faith);
				i = buf2;
			} else {
				i = "-";
			}
			break;
		case 'l':
			snprintf(buf2, sizeof(buf2), "%-3.3s",
				skill_name(ch->slang));
			i = buf2;
			break;
		case 'E':
			i = OLCED(ch) ? OLCED(ch)->name : str_empty;
			if (!IS_NULLSTR(i)) {
				snprintf(buf2, sizeof(buf2), "%s ", i);
				i = buf2;
			}
			break;

		case 'W':
			if (ch->pcdata->invis_level) {
				snprintf(buf2, sizeof(buf2), "Wizi %d ",
					 ch->pcdata->invis_level);
				i = buf2;
			}
			else
				i = "";
			break;

		case 'I':
			if (ch->pcdata->incog_level) {
				snprintf(buf2, sizeof(buf2), "Incog %d ",
					 ch->pcdata->incog_level);
				i = buf2;
			}
			else
				i = "";
			break;
		}
		++str;
		while((*point = *i) != '\0')
			++point, ++i;
	}

	*point = '\0';
	send_to_char(buf, ch);

	if (IS_PC(ch) && ch->pcdata->prefix[0] != '\0')
		write_to_buffer_muddy(ch->desc, ch->pcdata->prefix, 0);
}

/*
 * Append onto an output buffer.
 */
void write_to_buffer_muddy(DESCRIPTOR_DATA_MUDDY *d, const char *txt,
					uint length)
{
	//uint size;
	//int i;
	//bool noiac = (d->connected == CON_PLAYING &&
	//	      d->character != NULL &&
	//	      IS_SET(d->character->comm, COMM_NOIAC));
	//char tmp_txt[MAX_STRING_LENGTH];
	
	//if (d->snoop_by)
	//	strncpy(tmp_txt, txt, sizeof(tmp_txt));
	/*
	 * Snoop-o-rama.
	 */
	if (d->snoop_by) {
		if (d->character)
			write_to_buffer_muddy(d->snoop_by, d->character->name,
							0);
		write_to_buffer_muddy(d->snoop_by, "> ", 2);
		write_to_buffer_muddy(d->snoop_by, txt, 0);
	}

	if (d->client->connect)
	{
		if (!write_to_buffer_client(d->client, txt, length))
		{
			close_descriptor_muddy(d);
			return;
		}
	}
	return;
}

int search_sockets(DESCRIPTOR_DATA_MUDDY *inp)
{
	DESCRIPTOR_DATA_MUDDY *d;

	if (IS_IMMORTAL(inp->character))
		return 0;

	for(d = descriptor_list_muddy; d; d = d->next) {
		if(!hostcmp(inp->client, d->client))
		{
			if (d->character && inp->character
			&&  !strcmp(inp->character->name, d->character->name)) 
				continue;
			return 1;
		}
	}
	return 0;
}
  
int align_restrict(CHAR_DATA *ch);
bool check_set_align(CHAR_DATA *ch, flag32_t align);
bool sex_restrict(CHAR_DATA *ch);
int ethos_check(CHAR_DATA *ch);

void advance(CHAR_DATA *victim, int level);

static void print_hometown(CHAR_DATA *ch)
{
	race_t *r;
	class_t *cl;
	int htn;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata
	||  (cl = class_lookup(ch->class)) == NULL
	||  !ch->pcdata) {
		char_puts("You should create your character anew.\n", ch);
		close_descriptor_muddy(ch->desc);
		return;
	}

	if ((htn = hometown_permanent(ch)) >= 0) {
		ch->pcdata->hometown = htn;
		char_printf(ch, "\nYour hometown is %s, permanently.\n"
				"[Hit Return to continue]\n",
			    hometown_name(htn));

/* XXX */
		ch->pcdata->endur = 100;
		ch->desc->connected = CON_GET_ETHOS;
		return;
	}

	char_puts("\n", ch);
	do_help(ch, "HOMETOWN");
	hometown_print_avail(ch);
	char_puts("? ", ch);
	ch->desc->connected = CON_PICK_HOMETOWN;
}

void class_table_show(CHAR_DATA *ch)
{
	BUFFER *output;
	int cl;
	class_t *class;
	char tmp[MAX_STRING_LENGTH];
	
	output = buf_new(-1);
	buf_add(output,
		"\n\rCLASS        ACRONYM  STR INT WIS DEX CON CHA  SEX EXTRA\n\r");
	buf_add(output,
		"-----          ---    --- --- --- --- --- ---  ---  ---\n\r");
	
	for(cl = 1; cl < classes.nused; cl++){
		class = class_lookup(cl);
		snprintf(tmp, sizeof(tmp), "%-14s %-3s   %3i %3i %3i %3i %3i %3i   %-3s  %3i",
			capitalize(class->name), class->who_name,
			class->stats[0], class->stats[1], class->stats[2],
			class->stats[3], class->stats[4], class->stats[5],
			class->restrict_sex == SEX_MALE ? " M " :
			  class->restrict_sex == SEX_FEMALE ? " F " : "ANY",
			class->points);
		buf_add(output, tmp);
		buf_add(output,"\n\r");
	}
	buf_add(output,"\n\r");
	
	page_to_char(buf_string(output), ch);
	buf_free(output);
}
void race_table_show(CHAR_DATA *ch)
{
	BUFFER *output;
	int r;
	race_t *race;
	char tmp[MAX_STRING_LENGTH];

	output = buf_new(-1);
	buf_add(output,
		"RACE       ACRONYM STR INT WIS DEX CON CHA EXTRA COMMENTS\n\r");
	buf_add(output,
		"----         ---   --- --- --- --- --- ---  ---  ------------------\n\r");
	for(r = 0; r < races.nused; r++){
		race = race_lookup(r);
		if (!race->pcdata)
			continue;
		snprintf(tmp, sizeof(tmp), "%-11s %-5s %3i %3i %3i %3i %3i %3i   %3i  ",
			capitalize(race->name), race->pcdata->who_name,
			race->pcdata->stats[0], race->pcdata->stats[1], race->pcdata->stats[2],
			race->pcdata->stats[3], race->pcdata->stats[4], race->pcdata->stats[5],
			race->pcdata->points);
		buf_add(output, tmp);
		if (race->pcdata->allow_align)
		{
			if (IS_SET(race->pcdata->allow_align, RA_GOOD))
				buf_add(output, "good; ");
			if (IS_SET(race->pcdata->allow_align, RA_EVIL))
				buf_add(output, "evil; ");
			if (IS_SET(race->pcdata->allow_align, RA_NEUTRAL))
				buf_add(output, "neutral; ");
		}
		
		if (race->pcdata->bonus_skills != NULL)
			buf_add(output,"bonus sk.; ");
		if (race->pcdata->skills.nused > 0)
			buf_add(output,"race sk.");
		
/*		tmp2 = race->pcdata->bonus_skills;
		for (;;) {
			char name[MAX_STRING_LENGTH];
			
			tmp2 = one_argument(tmp2, name, sizeof(name));
			if (name[0] == '\0')
				break;
			snprintf(tmp, sizeof(tmp), "\"%s\";", name);
			buf_add(output,
			"\n                                                       ");
			buf_add(output, tmp);
		}
 */
		
		buf_add(output,"\n\r");
		
	}
	
	page_to_char(buf_string(output), ch);
	buf_free(output);
}

bool show_last(BUFFER *output, CHAR_DATA *ch)
{
	char name[MAX_STRING_LENGTH];
	char str[MAX_STRING_LENGTH];
	struct stat stat_pfile;
	char str2[MAX_STRING_LENGTH];

	if (ch == NULL)
		return FALSE;
	if (ch->desc && ch->desc->original)
		ch = ch->desc->original;
	if (!IS_PC(ch))
		return FALSE;

	if (output)
		snprintf(name, sizeof(name), "%s's", ch->name);
	else
		snprintf(name, sizeof(name), "Your");

	snprintf(str2, sizeof(str2), "%s/%s", PLAYER_PATH, capitalize(ch->name));
	if (stat(str2, &stat_pfile))
	{
		snprintf(str, sizeof(str), "%s hasn't save file.\n",
				name);
	} else {
		snprintf(str, sizeof(str),
			"%s save file has %s of last data modification.\n",
			name, strtime(stat_pfile.st_mtime));
	}
	
	snprintf(str2, sizeof(str2), "%s%s last logout %s from '%s'.\n",
		str, name, strtime(ch->pcdata->last_logout), ch->pcdata->last_host);
	
	if (output == NULL)
		char_puts(str2, ch);
	else
		buf_add(output, str2);
	return TRUE;
}
/*
 * Deal with sockets that haven't logged in yet.
 */
void nanny(DESCRIPTOR_DATA_MUDDY *d, const char *argument)
{
	char buf[MAX_STRING_LENGTH];
//	DESCRIPTOR_DATA *d_old, *d_next;
//	char buf1[MAX_STRING_LENGTH];
	DESCRIPTOR_DATA_MUDDY *d_old, *d_next;
	char arg[MAX_INPUT_LENGTH];
	CHAR_DATA *ch;
	const char *pwdnew;
	int iClass,iRace,race,i;
	int nextquest = 0;
	race_t *r;
	class_t *cl;

	while (isspace(*argument))	// require, because <ENTER> is { ' ', '\0' }
		argument++;

	ch = d->character;

	switch (d->connected)
	{
	default:
		bug("Nanny: bad d->connected %d.", d->connected);
		close_descriptor_muddy(d);
		return;	/* return - not read data from d->client->inarray !!!
			 *	see bottom of function
			 */
		

	case CON_GET_CODEPAGE: {
		int num;

		if (argument[0] == '\0') {
			close_descriptor_muddy(d);
			break;
		}

		if (argument[1] != '\0'
		||  (num = argument[0] - '1') < 0
		||  num >= NCODEPAGES) {
			cp_print(d);
			break;
		}

		d->client->codepage = codepages + num;
		log_printf("'%s' codepage selected", d->client->codepage->name);
		d->connected = CON_GET_NAME;
		write_to_buffer_muddy(d, "By which name do you wish to be known? ", 0);
		//write_to_buffer_muddy(d, "\rBy which name do you wish to be known? ", 0);
		//set_binary(d);
		break;
	}

	case CON_GET_NAME:
		if (argument[0] == '\0') {
			close_descriptor_muddy(d);
			break;
		}

		if (!pc_name_ok(argument)) {
			write_to_buffer_muddy(d, "Illegal name, try another.\n\r"
					   "Name: ", 0);
			break;
		}

		log_printf("Searching %s.", capitalize(argument));
		//load_char_obj(d, argument);
		//ch   = d->character;
		ch = load_char_obj(NULL, argument);
		ch->desc = d;
		d->character = ch;

		telnet_flags_check(ch, ch->desc->client);
		d->connected = CON_RESOLV;

#if !defined (USE_THREADS)
		if (d->client->hostname == NULL)
		{
#if defined (WIN32)
			// проверить всю резолверку под MD!!!
			// check all resolver realisation on win32
			printf("%s@%s\n", ch->name, inet_ntoa(d->client->connect->addr.sin_addr));
#else
			/*
			fprintf(rfout, "%s@%s\n",
				ch->name, inet_ntoa(d->client->connect->addr.sin_addr));
			*/
			resolv_client_check_start(d->client->idtun,
				get_idsock_connect(d->client),
				ch->name, get_ip_client(d->client));
#endif
//			d->connected = CON_RESOLV;
/* wait until sock.sin_addr gets resolved */
			break;
		}

		/* FALLTHRU */

	case CON_RESOLV:
		if (d->client->hostname == NULL)
			return;	// data not read yet (not 'break' !)
#else
		/* NOT WORK !!!! */
	case CON_RESOLV:
		if (pthread_mutex_trylock(&d->mutex) == EBUSY)
			break;
		if (!d->host)
		{
			pthread_t resolver_thread;

//			d->connected = CON_RESOLV;
			pthread_create(&resolver_thread, NULL, (thr_cb)resolver, d);
			break;
		}
		pthread_mutex_unlock(&d->mutex);
#endif /* !USE_THREADS */

	/*
	 * Swiftest: I added the following to ban sites.  I don't
	 * endorse banning of sites, but Copper has few descriptors now
	 * and some people from certain sites keep abusing access by
	 * using automated 'autodialers' and leaving connections hanging.
	 *
	 * Furey: added suffix check by request of Nickel of HiddenWorlds.
	 */
	if (check_ban(get_hostname(d->client), BAN_ALL))
	{
		write_to_buffer_muddy(d, "Your site has been banned from this mud.\n\r", 0);
		close_descriptor_muddy(d);
		return;
	}

	if (!IS_IMMORTAL(ch))
	{
		if (check_ban(get_hostname(d->client), BAN_PLAYER))
		{
			write_to_buffer_muddy(d,"Your site has been banned for players.\n\r",0);
			close_descriptor_muddy(d);
			return;
	        }

#undef NO_PLAYING_TWICE
#ifdef NO_PLAYING_TWICE
		if(search_sockets(d))
		{
			write_to_buffer_muddy(d, "Playing twice is restricted...\n\r", 0);
			close_descriptor_muddy(d);
			return;
		} 
#endif
	  if (iNumPlayers > MAX_OLDIES && !IS_SET(ch->pcdata->plr_flags, PLR_NEW))
	  {
	     snprintf(buf, sizeof(buf),
	   "\n\rThere are currently %i players mudding out of a maximum of %i.\n\rPlease try again soon.\n\r",iNumPlayers - 1, MAX_OLDIES);
	     write_to_buffer_muddy(d, buf, 0);
	     close_descriptor_muddy(d);
	     return;
	  }
	  if (iNumPlayers > MAX_NEWBIES && IS_SET(ch->pcdata->plr_flags, PLR_NEW))
	  {
	     snprintf(buf, sizeof(buf),
	   "\n\rThere are currently %i players mudding. New player creation is \n\rlimited to when there are less than %i players. Please try again soon.\n\r",
		     iNumPlayers - 1, MAX_NEWBIES);
	     write_to_buffer_muddy(d, buf, 0);
	     close_descriptor_muddy(d);
	     return;
	  }
	}
	     
	if (IS_SET(ch->pcdata->plr_flags, PLR_DENY))
	{
	    log_printf("Denying access to %s@%s.", argument, get_hostname(d->client));
	    write_to_buffer_muddy(d, "You are denied access.\n\r", 0);
	    close_descriptor_muddy(d);
	    return;
	}

		if (check_reconnect(d,/* argument,*/ FALSE))
		{
			REMOVE_BIT(ch->pcdata->plr_flags, PLR_NEW);
		} else if (wizlock && !IS_HERO(ch))
		{
			write_to_buffer_muddy(d, "The game is wizlocked.\n\r", 0);
			close_descriptor_muddy(d);
			return;
		}

		if (!IS_SET(ch->pcdata->plr_flags, PLR_NEW))
		{
			/* Old player */
			//write_to_descriptor(d, echo_off_str, 0);
 			//echo_off_string(d->client);
			write_to_buffer_muddy(d, "Password: ", 0);
			echo_off_string(d->client);
			d->connected = CON_GET_OLD_PASSWORD;
			return;
		}
		else {
			/* New player */
 			if (newlock) {
				write_to_buffer_muddy(d, "The game is newlocked.\n\r", 0);
				close_descriptor_muddy(d);
				return;
			}

			if (check_ban(get_hostname(d->client), BAN_NEWBIES)) {
				write_to_buffer_muddy(d, "New players are not allowed from your site.\n\r", 0);
				close_descriptor_muddy(d);
				return;
			}
 	    
 	    		write_to_buffer_muddy(d, "Select your language ([R]ussian or [E]nglish): ", 0);
			d->connected = CON_GET_LANG;
			return;
		}
		break;

/* RT code for breaking link */
	case CON_BREAK_CONNECT:
		switch(*argument) {
		case 'y' : case 'Y':
			for (d_old = descriptor_list_muddy; d_old; d_old = d_next)
			{
				CHAR_DATA *rch;

				d_next = d_old->next;
				if (d_old == d || d_old->character == NULL)
					continue;

				rch = d_old->original ? d_old->original :
							d_old->character;
				if (str_cmp(ch->name, rch->name))
					continue;

				if (d_old->original)
					do_return(d_old->character, str_empty);
				close_descriptor_muddy(d_old);
			}

			if (check_reconnect(d/*, ch->name*/, TRUE))
				break;
			write_to_buffer_muddy(d,"Reconnect attempt failed.\n\r",0);

			/* FALLTHRU */

		case 'n' : case 'N':
	 		write_to_buffer_muddy(d,"Name: ",0);
			if (d->character != NULL)
			{
				free_char(d->character);
				d->character = NULL;
			}
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer_muddy(d, "Please type Y or N? ", 0);
			break;
		}
		break;
	case CON_GET_LANG:
		switch(*argument)
		{
		case 'R': case 'r':
			ch->lang = lang_lookup("rus");
			write_to_buffer_muddy(d, "Interface language is russian.\n\r", 0);
			break;
		case 'E': case 'e':
			ch->lang = lang_lookup("eng");
			write_to_buffer_muddy(d, "Interface language is english.\n\r", 0);
			break;
		default:
			write_to_buffer_muddy(d, "Please type [R]ussian or [E]nglish? ", 0);
			break;
		}

		write_to_buffer_muddy(d, "Select your color scheme ([A]nsi or [N]on): ", 0);
		d->connected = CON_GET_COLOR;
		break;

	case CON_GET_COLOR:
		switch(*argument)
		{
		case 'A': case 'a':
			SET_BIT(ch->comm, COMM_COLOR);
			char_puts("{BC{Ro{Yl{Co{Gr{x is now {RON{x, Way Cool!\n", ch);
			break;
		case 'N': case 'n':
			write_to_buffer_muddy(d, "Color is OFF, *sigh*.\n\r", 0);
			break;
		default:
			write_to_buffer_muddy(d, "Please type [A]nsi or [N]on? ", 0);
			break;
		}
		do_help(ch, "NAME");
		d->connected = CON_CONFIRM_NEW_NAME;
		break;
			
	case CON_CONFIRM_NEW_NAME:
		switch (*argument)
		{
		case 'y': case 'Y':
			if (check_ban_name(ch->name))
			{
				write_to_buffer_muddy(d, "Denied name, try another.\n\r"
	                                           "Name: ", 0);
				free_char(d->character);
				d->character = NULL;
				d->connected = CON_GET_NAME;
				break;
			}
			snprintf(buf, sizeof(buf),
				 "New character.\n\r"
				 "Give me a password for %s: ", ch->name);
			write_to_buffer_muddy(d, buf, 0);
			/* may be without compression ?!*/
			//write_to_descriptor_muddy(d, echo_off_str, 0);
			echo_off_string(d->client);
			d->connected = CON_GET_NEW_PASSWORD;
			break;

		case 'n': case 'N':
			write_to_buffer_muddy(d, "Ok, what IS it, then? ", 0);
			free_char(d->character);
			d->character = NULL;
			d->connected = CON_GET_NAME;
			break;

		default:
			write_to_buffer_muddy(d, "Please type Yes or No? ", 0);
			break;
		}
		break;

	case CON_GET_NEW_PASSWORD:
#if defined(unix)
		write_to_buffer_muddy( d, "\n\r", 2 );
#endif

		if (strlen(argument) < 5)
		{
			write_to_buffer_muddy(d, "Password must be at least five characters long.\n\rPassword: ", 0);
			break;
		}

#if !defined(CRYPT)
		pwdnew = argument;
#else
		pwdnew = crypt(argument, ch->name);
#endif
		free_string(ch->pcdata->pwd);
		ch->pcdata->pwd	= str_dup(pwdnew);
		write_to_buffer_muddy(d, "Please retype password: ", 0);
		d->connected = CON_CONFIRM_NEW_PASSWORD;
		break;

	case CON_CONFIRM_NEW_PASSWORD:
#if defined(unix)
	write_to_buffer_muddy( d, "\n\r", 2 );
#endif

#if !defined(CRYPT)
		if (strcmp(argument, ch->pcdata->pwd))
		{
			write_to_buffer_muddy(d, "Passwords don't match.\n\r"
					   "Retype password: ", 0);
			d->connected = CON_GET_NEW_PASSWORD;
			break;
		}

#else
		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd))
		{
			write_to_buffer_muddy(d, "Passwords don't match.\n\r"
					   "Retype password: ", 0);
			d->connected = CON_GET_NEW_PASSWORD;
			break;
		}
#endif

		/* may be without compression ?!*/
		echo_on_string(d->client);
//		write_toi_descriptor_muddy(d, (char *) echo_on_str, 0);

//		write_to_buffer(d, "The Shadow Realms is home for the following races:\n\r", 0);
	
	
//		do_help(ch,"class help");
		class_table_show(ch);

		strnzcpy(buf, sizeof(buf), "Select a class:\n\r[ ");


	        write_to_buffer_muddy(d,
		"\n\rWhat is your class ('help <class>' for more information)? ",0);
	    	d->connected = CON_GET_NEW_CLASS;
	    	break;
	

	case CON_GET_NEW_CLASS:
	iClass = cn_lookup(argument);
	argument = one_argument(argument, arg, sizeof(arg));

	if (!str_cmp(arg,"help"))
	  {
	    if (argument[0] == '\0')
//		do_help(ch,"class help");
		class_table_show(ch);
	    else
		do_help(ch,argument);
	        write_to_buffer_muddy(d,
		"What is your class ('help <class>' for more information)? ",0);
	    break;
	  }

	if (iClass <= 0)
	{
	    write_to_buffer_muddy(d,
		"That's not a class.\n\rWhat IS your class? ", 0);
	    break;
	}

		ch->class = iClass;
		ch->pcdata->points += CLASS(iClass)->points;
		act("You are now $t.", ch, CLASS(iClass)->name, NULL, TO_CHAR);

		for (i = 0; i < MAX_STATS; i++)
			ch->perm_stat[i] = number_range(10, get_max_train(ch, i));

//		snprintf(buf, sizeof(buf),
//			 "Str:%s  Int:%s  Wis:%s  Dex:%s  Con:%s  Cha:%s\n\rAccept (Y/N)? ",
//			 get_stat_alias(ch, STAT_STR),
//			 get_stat_alias(ch, STAT_INT),
//			 get_stat_alias(ch, STAT_WIS),
//			 get_stat_alias(ch, STAT_DEX),
//			 get_stat_alias(ch, STAT_CON),
//			 get_stat_alias(ch, STAT_CHA));

		
		
//		do_help(ch, "RACETABLE");
		race_table_show(ch);
		
		write_to_buffer_muddy(d, "\n\rThe following races are available:\n\r  ", 0);
		cl = class_lookup(ch->class);
		race = 1;
		for (iRace = 0; iRace < races.nused; iRace++)
		{
			race++;
			r = race_lookup(iRace);
			if ( (!r->pcdata) || (rclass_lookup(r, cl->name) == NULL) )
				continue;
			if (race == 8 || race == 14)
				write_to_buffer_muddy(d,"\n\r  ",0);
			write_to_buffer_muddy(d,"(",0);
			write_to_buffer_muddy(d, r->name, 0);
			write_to_buffer_muddy(d,") ",0);
		}
		write_to_buffer_muddy(d, "\n\r", 0);
		write_to_buffer_muddy(d, "What is your race? ('help <race>' for more information) ", 0);
				
		
		d->connected = CON_GET_NEW_RACE;
		break;



	case CON_GET_NEW_RACE:
		one_argument(argument, arg, sizeof(arg));

		if (!str_cmp(arg, "help"))
		{
			argument = one_argument(argument, arg, sizeof(arg));
			if (argument[0] == '\0')
			{
				write_to_buffer_muddy(d, "The Shadow Realms is the home for the following races:\n\r", 0);
	  			//do_help(ch,"RACETABLE");
				race_table_show(ch);
			} else {
				do_help(ch, argument);
				write_to_buffer_muddy(d, "What is your race? ('help <race>' for more information) ",0);
			}	
			break;
		}

		race = rn_lookup(argument);
		r = RACE(race);
		cl = class_lookup(ch->class);

		if (race == 0 || !r->pcdata || rclass_lookup(r, cl->name) == NULL)
		{
			write_to_buffer_muddy(d, "That is not a valid race.\n\r", 0);
			write_to_buffer_muddy(d, "The following races are available:\n\r  ", 0);
		
			race = 1;
			for (iRace = 0; iRace < races.nused; iRace++)
			{
				race++;
				r = race_lookup(iRace);
				if ( (!r->pcdata) || (rclass_lookup(r, cl->name) == NULL) )
				continue;
				if (race == 8 || race == 14)
					write_to_buffer_muddy(d,"\n\r  ",0);
				write_to_buffer_muddy(d,"(",0);
				write_to_buffer_muddy(d, r->name, 0);
				write_to_buffer_muddy(d,") ",0);
			}

			write_to_buffer_muddy(d, "\n\r", 0);
			write_to_buffer_muddy(d, "What is your race? ('help <race>' for more information) ", 0);
			break;
		}
		
		
		
		SET_ORG_RACE(ch, race);	// eq: ch->pcdata->race = race;
		ch->race = race;
		for (i=0; i < MAX_STATS;i++)
			ch->mod_stat[i] = 0;

		/* Add race stat modifiers 
		for (i = 0; i < MAX_STATS; i++)
			ch->mod_stat[i] += r->pcdata->stats[i];	*/

		/* Add race modifiers */
		ch->max_hit += r->pcdata->hp_bonus;
		ch->hit = ch->max_hit;
		ch->max_mana += r->pcdata->mana_bonus;
		ch->mana = ch->max_mana;
		ch->pcdata->practice = r->pcdata->prac_bonus;

		ch->affected_by = ch->affected_by| r->aff;
		ch->imm_flags	= ch->imm_flags| r->imm;
		ch->res_flags	= ch->res_flags| r->res;
		ch->vuln_flags	= ch->vuln_flags| r->vuln;
		ch->form	= r->form;
		ch->parts	= r->parts;

		/* add cost */
		ch->pcdata->points = r->pcdata->points;
		ch->size = r->pcdata->size;

		
		if (sex_restrict(ch))
		{
			d->connected = CON_ACCEPT_STATS;
			write_to_buffer_muddy(d, "[Hit Return to Continue]\n\r",0);
			break;
		}
		
		write_to_buffer_muddy(d, "What is your sex (M/F)? ", 0);
		d->connected = CON_GET_NEW_SEX;
		break;



	case CON_GET_NEW_SEX:
		switch (argument[0])
		{
			case 'm': case 'M': ch->sex = SEX_MALE;    
				    ch->pcdata->true_sex = SEX_MALE;
				    write_to_buffer_muddy(d, "Your sex is MALE.\n\r", 0);
				    break;
			case 'f': case 'F': ch->sex = SEX_FEMALE; 
				    ch->pcdata->true_sex = SEX_FEMALE;
				    write_to_buffer_muddy(d, "Your sex is FEMALE.\n\r", 0);
				    break;
			default:
			    write_to_buffer_muddy(d, "That's not a sex.\n\rWhat IS your sex? ", 0);
			    break;
		}
	

		write_to_buffer_muddy(d, "[Hit Return to Continue]\n\r",0);
		d->connected = CON_ACCEPT_STATS;
		break;
	
	
	
	case CON_ACCEPT_STATS:
	
		write_to_buffer_muddy(d,"\n\r",0);
		do_help(ch, "stats");
		//write_to_buffer(d, "\n\rYour stats is:\n\r", 0);
		for (i = 0; i < MAX_STATS; i++)
			ch->perm_stat[i] = get_max_train(ch, i)-7;

		write_to_buffer_muddy(d, "If your sex is female, you gain +1 CHA.\n\r", 0);
		write_to_buffer_muddy(d, "Your stats is 13 (+-) stat class and race:\n\r", 0);
		snprintf(buf, sizeof(buf),
			"Str:%s  Int:%s  Wis:%s  Dex:%s  Con:%s  Cha:%s\n\r",
			get_stat_alias(ch, STAT_STR),
			get_stat_alias(ch, STAT_INT),
			get_stat_alias(ch, STAT_WIS),
			get_stat_alias(ch, STAT_DEX),
			get_stat_alias(ch, STAT_CON),
			get_stat_alias(ch, STAT_CHA));	
	
		write_to_buffer_muddy(d, buf, 0);


		for (i=0; i < MAX_STATS;i++)
			ch->mod_stat[i] = 0;
		write_to_buffer_muddy(d, "\n\r", 2);
		if (!align_restrict(ch))
		{
			write_to_buffer_muddy(d, "You may be good, neutral, or evil.\n\r",0);
			write_to_buffer_muddy(d, "Which alignment (G/N/E)? ",0);
			d->connected = CON_GET_ALIGNMENT;
		} else {
			write_to_buffer_muddy(d, "[Hit Return to Continue]\n\r",0);
			print_hometown(ch);
		}
		break;		


/*	case CON_ACCEPT_STATS:
		switch(argument[0])
		  {
		  case 'H': case 'h': case '?':
		    do_help(ch,"stats");
		    break;
		  case 'y': case 'Y':	

		    for (i=0; i < MAX_STATS;i++)
		      ch->mod_stat[i] = 0;
		    write_to_buffer(d, "\n\r", 2);
		    if (!align_restrict(ch))
		    {
			    write_to_buffer(d, "You may be good, neutral, or evil.\n\r",0);
			    write_to_buffer(d, "Which alignment (G/N/E)? ",0);
			    d->connected = CON_GET_ALIGNMENT;
		    }
		    else {
			write_to_buffer(d, "[Hit Return to Continue]\n\r",0);
			print_hometown(ch);
		    }
		    break;
	    
		case 'n': case 'N':
			for (i = 0; i < MAX_STATS; i++)
				ch->perm_stat[i] = number_range(10, get_max_train(ch, i));

			snprintf(buf, sizeof(buf),
				 "Str:%s  Int:%s  Wis:%s  Dex:%s  Con:%s  Cha:%s\n\rAccept (Y/N)? ",
				 get_stat_alias(ch, STAT_STR),
				 get_stat_alias(ch, STAT_INT),
				 get_stat_alias(ch, STAT_WIS),
				 get_stat_alias(ch, STAT_DEX),
				 get_stat_alias(ch, STAT_CON),
				 get_stat_alias(ch, STAT_CHA));

			write_to_buffer(d, buf,0);
			d->connected = CON_ACCEPT_STATS;
			break;

		default:
			write_to_buffer(d,"Please answer (Y/N)? ",0);
			break;
		}
		break;
*/
	    
	case CON_GET_ALIGNMENT:
	switch(argument[0])
	  {
	  case 'g' : case 'G' : 
	  	if (!check_set_align(ch, RA_GOOD))
	  		break;
	        write_to_buffer_muddy(d, "Now your character is good.\n\r",0);
		break;
	  case 'n' : case 'N' : 
	  	if (!check_set_align(ch, RA_NEUTRAL))
	  		break;
	        write_to_buffer_muddy(d, "Now your character is neutral.\n\r",0);
		break;
	  case 'e' : case 'E' :
	  	if (!check_set_align(ch, RA_EVIL))
	  		break;
	        write_to_buffer_muddy(d, "Now your character is evil.\n\r",0);
		break;
	  default:
	    write_to_buffer_muddy(d,"That's not a valid alignment.\n\r",0);
	    write_to_buffer_muddy(d,"Which alignment (G/N/E)? ",0);
	    break;
	  }
		write_to_buffer_muddy(d, "\n\r[Hit Return to Continue]\n\r", 0);
		print_hometown(ch);
		break;
	
	case CON_PICK_HOMETOWN:
	{
		int htn;

		if (argument[0] == '\0'
		||  (htn = htn_lookup(argument)) < 0
		||  hometown_restrict(HOMETOWN(htn), ch))
		{
			char_puts("That's not a valid hometown.\n", ch);
			print_hometown(ch);
			break;
		}

		ch->pcdata->hometown = htn; 
		char_printf(ch, "\nNow your hometown is %s.\n"
				"[Hit Return to continue]\n",
			    hometown_name(htn));
		ch->pcdata->endur = 100;
		d->connected = CON_GET_ETHOS;
		break;
	}
	
	  case CON_GET_ETHOS:
	if (!ch->pcdata->endur)
	 {
	  switch(argument[0]) 
	      {
	   case 'H': case 'h': case '?': 
		do_help(ch, "alignment");
		goto end_parse_nanny;
	   case 'L': case 'l': 
	 	snprintf(buf, sizeof(buf), "\n\rNow you are lawful-%s.\n\r",
		   IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");
	        write_to_buffer_muddy(d, buf, 0);
		ch->pcdata->ethos = ETHOS_LAWFUL; 
		break;
	   case 'N': case 'n': 
	 	snprintf(buf, sizeof(buf), "\n\rNow you are neutral-%s.\n\r",
		   IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");
	        write_to_buffer_muddy(d, buf, 0);
		ch->pcdata->ethos = ETHOS_NEUTRAL; 
		break;
	   case 'C': case 'c': 
	 	snprintf(buf, sizeof(buf), "\n\rNow you are chaotic-%s.\n\r",
		   IS_GOOD(ch) ? "good" : IS_EVIL(ch) ? "evil" : "neutral");
	        write_to_buffer_muddy(d, buf, 0);
		ch->pcdata->ethos = ETHOS_CHAOTIC; 
		break;
	   default:
	    write_to_buffer_muddy(d, "\n\rThat is not a valid ethos.\n\r", 0);
	    write_to_buffer_muddy(d, "What ethos do you want, (L/N/C) <type help for more info>? ",0);
	    goto end_parse_nanny;
	   }
	} else {
	  ch->pcdata->endur = 0;
	  if (!ethos_check(ch)) {
		write_to_buffer_muddy(d, "What ethos do you want, (L/N/C) "
				   "<type help for more info> ?", 0);
		d->connected = CON_GET_ETHOS;
		goto end_parse_nanny;
	   } else
		ch->pcdata->ethos = ETHOS_NEUTRAL;
	 }
	     write_to_buffer_muddy(d, "\n\r[Hit Return to Continue]\n\r",0);
	     d->connected = CON_CREATE_DONE;
	     break;

	case CON_CREATE_DONE:
		log_printf("%s@%s new player.", ch->name, get_hostname(d->client));
		write_to_buffer_muddy(d, "\n\r", 2);
		do_help(ch, "motd");
		char_puts("[Press Enter to continue]", ch);
		d->connected = CON_READ_MOTD;
		break;

	case CON_GET_OLD_PASSWORD:
		write_to_buffer_muddy(d, "\n\r", 2);

#if !defined(CRYPT)
		if (strcmp(argument, ch->pcdata->pwd))
		{
			write_to_buffer_muddy(d, "Wrong password.\n\r", 0);
			log_printf("Wrong password by %s@%s",
				   ch->name, get_hostname(d->client));
			if (ch->pcdata->endur == 2)
				close_descriptor_muddy(d);
			else {
				/* may be without compression ?!*/
				//write_to_descriptor(d, (char *) echo_off_str, 0);
				//echo_off_string(d->client);
				write_to_buffer_muddy(d, "Password: ", 0);
				echo_off_string(d->client);
				d->connected = CON_GET_OLD_PASSWORD;
				ch->pcdata->endur++;
			}
			break;
		}
#else
		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd) && ch->pcdata->pwd[0] != '\0')
		{
			write_to_buffer_muddy(d, "Wrong password.\n\r", 0);
			log_printf("Wrong password by %s@%s",
				   ch->name, get_hostname(d->client));
			if (ch->pcdata->endur == 2)
				close_descriptor_muddy(d);
			else {
				/* may be without compression ?!*/
				//write_to_descriptor(d, (char *) echo_off_str, 0);
				//echo_off_string(d->client);
				write_to_buffer_muddy(d, "Password: ", 0);
				echo_off_string(d->client);
				d->connected = CON_GET_OLD_PASSWORD;
				ch->pcdata->endur++;
			}
			break;
		}
#endif
 
		if (ch->pcdata->pwd[0] == '\0')
		{
			write_to_buffer_muddy(d, "Warning! Null password!\n\r"
					   "Type 'password null <new password>'"
					   " to fix.\n\r", 0);
		}

		/* may be without compression ?!*/
		echo_on_string(d->client);
		//write_to_descriptor(d, (char *) echo_on_str, 0);

		if (check_playing(d, ch->name)
		||  check_reconnect(d/*, ch->name*/, TRUE))
			break;

		if (ch->level == 0)	// fix bug  (thx to Krelion)  :)
		{
			write_to_buffer_muddy(d, "Fail to create new char soul.\n\r", 0);
			close_descriptor_muddy(d);
			free_char(ch);
			break;
		}

		log_printf("%s@%s has connected.", ch->name, get_hostname(d->client));
		d->connected = CON_READ_IMOTD;

		/* FALL THRU */
		/* remove "break;" ! */

	case CON_READ_IMOTD:
		if (IS_HERO(ch))
			do_help(ch, "imotd");
		write_to_buffer_muddy(d,"\n\r",2);
		do_help(ch, "motd");
		d->connected = CON_READ_MOTD;

		/* FALL THRU */
		/* remove "break;" ! */

	case CON_READ_MOTD:
		update_skills(ch);
		write_to_buffer_muddy(d, 
		"\n\rWelcome to Shadow Realms Multi User Dungeon. Enjoy!!...\n\r",
		    0);
		
		add_pch_tolist(ch);
		d->connected	= CON_PLAYING;
		{
			DESCRIPTOR_DATA_MUDDY *dm;
			int count;
			FILE *max_on_file;
			int tmp = 0;
			count = 0;
			for (dm = descriptor_list_muddy; dm; dm = dm->next)
				if (dm->connected == CON_PLAYING)
			       		count++;
			max_on = UMAX(count, max_on);
			if ((max_on_file = dfopen(TMP_PATH, MAXON_FILE, "r")))
			{
				fscanf(max_on_file, "%d", &tmp);
				fclose(max_on_file);
			}
			if (tmp < max_on
			&&  (max_on_file = dfopen(TMP_PATH, MAXON_FILE, "w"))) {
				fprintf(max_on_file, "%d", max_on);
				log("Global max_on changed.");
				fclose(max_on_file);
			}
		}

		reset_char(ch);

		/* quest code */
		if (ch->pcdata->quest)
		{
			nextquest = -abs(ch->pcdata->quest->time);
			quest_cancel(ch);
			ch->pcdata->quest->time = nextquest;
			/* !quest code */
		}

		wiznet("{W$N{x has entered into SHADOW REALMS.",
			ch, NULL, WIZ_LOGINS, 0, ch->level);

		for (i = 0; i < MAX_STATS; i++) {
			int max_stat = get_max_train(ch, i);

			if (ch->perm_stat[i] > max_stat) {
				ch->pcdata->train += ch->perm_stat[i] - max_stat;
				ch->perm_stat[i] = max_stat;
			}
		}

		if (ch->gold > 6000 && !IS_IMMORTAL(ch)) {
			char_printf(ch, "You are taxed %d gold to pay for the Mayor's bar.\n\r", (ch->gold - 6000) / 2);
			ch->gold -= (ch->gold - 6000) / 2;
		}
	
/*		if (!IS_IMMORTAL(ch)) {
			for (i = 2; exp_for_level(ch, i) < ch->exp; i++)
				;

			if (i < ch->level) {
				int con;
				int wis;
				int inte;
				int dex;

				con = ch->perm_stat[STAT_CON];
				wis = ch->perm_stat[STAT_WIS];
				inte = ch->perm_stat[STAT_INT];
				dex = ch->perm_stat[STAT_DEX];
				ch->perm_stat[STAT_CON] = get_max_train(ch, STAT_CON);
				ch->perm_stat[STAT_WIS] = get_max_train(ch, STAT_WIS);
				ch->perm_stat[STAT_INT] = get_max_train(ch, STAT_INT);
				ch->perm_stat[STAT_DEX] = get_max_train(ch, STAT_DEX);
				do_remove(ch, "all");
				advance(ch, i-1);
		 		ch->perm_stat[STAT_CON] = con;
		 		ch->perm_stat[STAT_WIS] = wis;
		 		ch->perm_stat[STAT_INT] = inte;
		 		ch->perm_stat[STAT_DEX] = dex;
			}
		}
made by XT :) */

		if (ch->level == 0) {
			OBJ_DATA *wield;
			OBJ_INDEX_DATA *map;
			AFFECT_DATA af;
			

			ch->level	= 1;
			ch->pcdata->exp	= base_exp(ch);
			ch->hit		= ch->max_hit;
			ch->mana	= ch->max_mana;
			ch->move	= ch->max_move;
			ch->pcdata->train	= 3;
			ch->pcdata->practice	+= 5;
			ch->pcdata->death = 0;
			// ch->silver = 30;	players not understand this... *sigh*

			if (!can_speak(ch, ch->slang))
				ch->slang = get_default_speak(ch);

			set_title(ch, title_lookup(ch));

			do_outfit(ch, str_empty);

			obj_to_char(create_obj(get_obj_index(OBJ_VNUM_MAP), 0), ch);
			obj_to_char(create_obj(get_obj_index(OBJ_VNUM_NMAP1), 0), ch);
			obj_to_char(create_obj(get_obj_index(OBJ_VNUM_NMAP2), 0), ch);

			if ((map = get_map(ch)) != NULL)
				obj_to_char(create_obj(map, 0), ch);

			if ((wield = get_eq_char(ch, WEAR_WIELD)))
				set_skill_raw(ch, get_weapon_sn(wield),
					      40, FALSE, 1, 100, 50);

			char_puts("\n", ch);
			do_help(ch, "NEWBIE INFO");
			char_puts("\n", ch);
			char_to_room(ch, get_room_index(ROOM_VNUM_SCHOOL));
			
			af.where        = TO_AFFECTS;
			af.type         = gsn_pray;
			af.level        = 100;
			af.duration     = 72;
			af.bitvector	= AFF_BLESS;
			af.modifier	= 5;
			
			af.location     = APPLY_CON;
			affect_to_char(ch, &af);
			af.bitvector    = 0;

			af.location     = APPLY_INT;
			affect_to_char(ch, &af);

			af.location     = APPLY_DEX;
			affect_to_char(ch, &af);

			af.location     = APPLY_WIS;
			affect_to_char(ch, &af);
		}
		else {
			CHAR_DATA *pet;
			ROOM_INDEX_DATA *to_room;

			if (ch->in_room
			&&  (room_is_private(ch->in_room) /*||
			     (ch->in_room->area->clan &&
			      ch->in_room->area->clan != ch->clan)*/))
				ch->in_room = NULL;

			if (ch->in_room) 
				to_room = ch->in_room;
			else if (IS_IMMORTAL(ch))
				to_room = get_room_index(ROOM_VNUM_CHAT);
			else
				to_room = get_room_index(ROOM_VNUM_TEMPLE);

			pet = ch->pet;
			act("$N has entered the game.",
			    to_room->people, NULL, ch, TO_ALL);
			char_to_room(ch, to_room);

			if (pet) {
				act("$N has entered the game.",
				    to_room->people, NULL, pet, TO_ROOM);
				char_to_room(pet, to_room);
			}
		}
		
		show_last(NULL, ch);
		if (!JUST_KILLED(ch)) {
			do_look(ch, "auto");
			do_unread(ch, "login");  
		}

		break;	/* require "break;" ! */

	case CON_DELETE:
		echo_on_string(d->client);
		//write_to_descriptor(d, (char *) echo_on_str, 0);
#if !defined(CRYPT)
		if (strcmp(argument, ch->pcdata->pwd))
		{
			write_to_buffer_muddy(d, "Wrong password.\n\r", 0);
			WAIT_STATE(ch, 10 * PULSE_PER_SECOND);
			if (ch->pcdata->remort && ch->pcdata->remort->shop) {
				d->connected = CON_REMORT;
				write_to_buffer_muddy(ch->desc, "[remort]> ",0);
			} else
				d->connected = CON_PLAYING;
			break;
		}
#else
		if (strcmp(crypt(argument, ch->pcdata->pwd), ch->pcdata->pwd) &&
			ch->pcdata->pwd[0] != '\0')
		{
			write_to_buffer_muddy(d, "Wrong password.\n\r", 0);
			WAIT_STATE(ch, 10 * PULSE_PER_SECOND);
			if (ch->pcdata->remort && ch->pcdata->remort->shop) {
				d->connected = CON_REMORT;
				write_to_buffer_muddy(ch->desc, "[remort]> ",0);
			} else
				d->connected = CON_PLAYING;
			break;
		}
#endif
		
		if (ch->pcdata->remort && ch->pcdata->remort->shop) {
			remort_done(ch, TRUE);
		} else {
			wiznet("$N turns $Mself into line noise.", ch, NULL, 0, 0, 0);
			delete_player(ch, NULL);
		}
		break;
	case CON_REMORT:
		remort_loop(ch, argument);
		if (d->connected == CON_REMORT)
			write_to_buffer_muddy(d, "[remort]> ",0);
		break;
	case CON_NULLLOOP:
		break;
	}
	end_parse_nanny:
	if (d->client)		/* data/argument read(ed) */
		d->client->inlength = 0;
}

/*
 * look for link-dead player to reconnect.
 *
 * when fConn == FALSE then
 * simple copy password for newly [re]connected character
 * authentication
 *
 * otherwise reconnect attempt is made
 */
bool check_reconnect(DESCRIPTOR_DATA_MUDDY *d, /*const char *name,*/ bool fConn)
{
	CHAR_DATA *ch;
	DESCRIPTOR_DATA_MUDDY *d2;

	if (!fConn) {
		for (d2 = descriptor_list_muddy; d2; d2 = d2->next) {
			if (d2 == d)
				continue;
			ch = d2->original ? d2->original : d2->character;
			if (ch && !str_cmp(d->character->name, ch->name)) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_qdup(ch->pcdata->pwd);
				return TRUE;
			}
		}
	}

	for (ch = char_list; ch && IS_PC(ch); ch = ch->next) {
		if ((!fConn || ch->desc == NULL || ch->desc->client->connect == NULL)
		&& d->character != ch
		&& !str_cmp(d->character->name, ch->name)) {
			if (!fConn) {
				free_string(d->character->pcdata->pwd);
				d->character->pcdata->pwd = str_qdup(ch->pcdata->pwd);
			}
			else {

/*				if (ch->pcdata->remort
				&& ch->pcdata->remort->shop) {
					d->connected = CON_REMORT;
					write_to_buffer(ch->desc, "[remort]> ",0);
				}
*/

				d->character->in_room		= NULL;	// !!!
				if (ch->desc != NULL)
				{
					save_finger_char(ch);
					// хитрый финт ушами ;) aka for destroy_descriptor_muddy()
					ch->desc->connected	= CON_NULLLOOP;
						// move new char to old desciptor (prepared for destroy)
					ch->desc->character	= d->character;
					d->character->desc	= ch->desc;
				} else {
					free_char(d->character); // extract_char();
				}

				d->character		= ch;
				ch->desc		= d;
				ch->timer		= 0;

				if (ch->in_room == NULL)
				{
					close_descriptor_muddy(d);
					extract_char(ch, 0);
					return TRUE;
				}

				char_puts("Reconnecting. Type replay to see missed tells.\n", ch);
				act("$n has reconnected.",
				    ch, NULL, NULL, TO_ROOM);

				log_printf("%s@%s reconnected.",
					   ch->name, get_hostname(d->client));
				wiznet("$N groks the fullness of $S link.",
				       ch, NULL, WIZ_LINKS, 0, 0);
				d->connected = CON_PLAYING;
			}
			return TRUE;
		}
	}

	return FALSE;
}

/*
 * Check if already playing.
 */
bool check_playing(DESCRIPTOR_DATA_MUDDY *d, const char *name)
{
	DESCRIPTOR_DATA_MUDDY *dold;

	for (dold = descriptor_list_muddy; dold; dold = dold->next) {
		if (dold != d
		&&  dold->character
		&&  dold->connected != CON_GET_CODEPAGE
		&&  dold->connected != CON_GET_NAME
		&&  dold->connected != CON_RESOLV
		&&  dold->connected != CON_GET_OLD_PASSWORD
		&&  !str_cmp(name, dold->original ?  dold->original->name :
						     dold->character->name)) {
			write_to_buffer_muddy(d, "That character is already playing.\n\r",0);
			write_to_buffer_muddy(d, "Do you wish to connect anyway (Y/N)?",0);
			d->connected = CON_BREAK_CONNECT;
			return TRUE;
		}
	}

	return FALSE;
}

void stop_idling(CHAR_DATA *ch)
{
	if (ch == NULL
	||  ch->desc == NULL
	||  ch->desc->connected != CON_PLAYING
	||  IS_NPC(ch)
	||  !ch->pcdata->was_in_room
	||  ch->in_room->vnum != ROOM_VNUM_LIMBO)
		return;

	ch->timer = 0;
	char_from_room(ch);
	act("$N has returned from the void.",
	    ch->pcdata->was_in_room->people, NULL, ch, TO_ALL);
	char_to_room(ch, ch->pcdata->was_in_room);
	ch->pcdata->was_in_room	= NULL;
}

void char_puts(const char *txt, CHAR_DATA *ch)
{
	send_to_char(GETMSG(txt, ch->lang), ch);
}

void char_printf(CHAR_DATA *ch, const char *format, ...)
{
	char buf[MAX_STRING_LENGTH];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, sizeof(buf), GETMSG(format, ch->lang), ap);
	va_end(ap);
	send_to_char(buf, ch);
}

/*
 * Write to one char.
 */
void send_to_char(const char *txt, CHAR_DATA *ch)
{
	char buf[MAX_STRING_LENGTH*4];

	if (txt == NULL || ch->desc == NULL)
		return;

	parse_colors(txt, buf, sizeof(buf), OUTPUT_FORMAT(ch));
	write_to_buffer_muddy(ch->desc, buf, 0);
}

/*
 * Send a page to one char.
 */
void page_to_char(const char *txt, CHAR_DATA *ch)
{
	if (txt == NULL || ch->desc == NULL)
		return; /* ben yazdim ibrahim */

	if (IS_NPC(ch) || ch->pcdata->lines == 0) {
		send_to_char(txt, ch);
		return;
	}
	
	ch->desc->showstr_head = str_dup(txt);
	ch->desc->showstr_point = ch->desc->showstr_head;
	show_string(ch->desc, str_empty);
}

/* string pager */
void show_string(DESCRIPTOR_DATA_MUDDY *d, char *input)
{
	char buffer[4*MAX_STRING_LENGTH];
//	char buf[MAX_INPUT_LENGTH];
	char *scan;
	int lines = 0;
	int show_lines;

//	one_argument(input, buf, sizeof(buf));

	if (input[0] != '\0' && input[0] != ' ')
	{
		if (d->showstr_head) {
			free_string(d->showstr_head);
			d->showstr_head = NULL;
		}
		d->showstr_point  = NULL;
		return;
	}

	if (d->character)
		show_lines = IS_NPC(d->character) ? 0 : d->character->pcdata->lines;
	else
		show_lines = 0;

	for (scan = buffer; scan - buffer < sizeof(buffer)-2;
						scan++, d->showstr_point++) {
		/*
		 * simple copy if not eos and not eol
		 */
		if ((*scan = *d->showstr_point) && (*scan) != '\n') 
			continue;

		/*
		 * bamf out buffer if we reached eos or show_lines limit
		 */
		if (!*scan || (show_lines > 0 && ++lines >= show_lines)) {
			const char *chk;

			if (*scan)
				*++scan = '\0';
			send_to_char(buffer, d->character);

			for (chk = d->showstr_point; isspace(*chk); chk++)
				;
			if (!*chk) {
				if (d->showstr_head) {
					free_string(d->showstr_head);
					d->showstr_head = NULL;
				}
				d->showstr_point  = NULL;
			}
			return;
		}
	}
}

void log_area_popularity(void)
{
	FILE *fp;
	AREA_DATA *area;
	extern AREA_DATA *area_first;

	if ((fp = dfopen(TMP_PATH, AREASTAT_FILE, "w")) == NULL)
		return;
	fprintf(fp,"\nBooted %sArea popularity statistics (in char * ticks)\n",
	        str_boot_time);

	for (area = area_first; area != NULL; area = area->next)
		if (area->count >= 5000000)
			fprintf(fp,"%-60s overflow\n",area->name);
		else
			fprintf(fp,"%-60s %u\n",area->name,area->count);

	fclose(fp);
}

bool sex_restrict(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA_MUDDY *d = ch->desc;
	class_t *cl;
	
	cl = class_lookup(ch->class);
	if  (cl->restrict_sex >= 0){

		if (cl->restrict_sex == SEX_MALE){
			write_to_buffer_muddy(d, "Your sex is MALE.\n\r", 0);
			ch->sex = SEX_MALE;
			ch->pcdata->true_sex = SEX_MALE;
		} else {
			write_to_buffer_muddy(d, "Your sex is FEMALE.\n\r", 0);
			ch->pcdata->true_sex = SEX_FEMALE;
			ch->sex = SEX_FEMALE;
		}
		
		return TRUE;
	}
	
	return FALSE;
}

bool check_set_align(CHAR_DATA *ch, flag32_t align)
{
	race_t *r;
	
	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	|| !r->pcdata)
		return FALSE;

	if ((r->pcdata->allow_align
		&& !IS_SET(r->pcdata->allow_align, align))
	|| (CLASS(ch->class)->restrict_align
		&& !IS_SET(CLASS(ch->class)->restrict_align, align)))
	{
		char_puts("This alignment restrict for your race or class.\n", ch); 
		return FALSE;
	}
	
	switch (align)	
	{
		case RA_GOOD:
			ch->alignment = ALIGN_GOOD;
			break;
		case RA_NEUTRAL:
			ch->alignment = ALIGN_NEUTRAL;
			break;
		case RA_EVIL:
			ch->alignment = ALIGN_EVIL;
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

int align_restrict(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA_MUDDY *d = ch->desc;
	race_t *r;

	if ((r = race_lookup(ORG_RACE(ch))) == NULL
	||  !r->pcdata)
		return RA_NONE;

	if (r->pcdata->allow_align == RA_GOOD
	||  CLASS(ch->class)->restrict_align == RA_GOOD) {
		write_to_buffer_muddy(d, "Your character has good tendencies.\n\r",0);
		ch->alignment = ALIGN_GOOD;
		return RA_GOOD;
	}

	if (r->pcdata->allow_align == RA_NEUTRAL
	||  CLASS(ch->class)->restrict_align == RA_NEUTRAL) {
		write_to_buffer_muddy(d, "Your character has neutral tendencies.\n\r",0);
		ch->alignment = ALIGN_NEUTRAL;
		return RA_NEUTRAL;
	}

	if (r->pcdata->allow_align == RA_EVIL
	||  CLASS(ch->class)->restrict_align == RA_EVIL) {
		write_to_buffer_muddy(d, "Your character has evil tendencies.\n\r",0);
		ch->alignment = ALIGN_EVIL;
		return RA_EVIL;
	}		

	return RA_NONE;
}

int ethos_check(CHAR_DATA *ch)
{
	DESCRIPTOR_DATA_MUDDY *d = ch->desc;
	class_t *cl;

	if ((cl = class_lookup(ch->class))) {
		/*
		 * temporary workaround for paladins
		 */
		if (IS_SET(cl->restrict_ethos, ETHOS_LAWFUL)) {
			write_to_buffer_muddy(d, "You are Lawful.\n\r", 0);
			return 1;
		}
	}
	return 0;
}

void telnet_flags_check(CHAR_DATA *ch, DESCRIPTOR_DATA_CLIENT * client)
{
	if (ch == NULL || client == NULL)
		return;

	if (IS_SET(ch->comm, COMM_NOTELNET))
		SET_BIT(client->flags, DDCF_NOTELNET);
	else
		REMOVE_BIT(client->flags, DDCF_NOTELNET);

	if (IS_SET(ch->comm, COMM_NOIAC))
		SET_BIT(client->flags, DDCF_NOIAC);
	else
		REMOVE_BIT(client->flags, DDCF_NOIAC);
		
}

#if !defined(USE_THREADS)
// moved into libs :)
// 
#elif defined (USE_THREADS)

/* threaded resolver - Icct */
/* not work yet, becouse gethostbyaddr and other can't be threaded */
void *resolver(DESCRIPTOR_DATA *d)
{
    struct sockaddr_in sock;
    struct hostent *h;
    int size;

    pthread_mutex_unlock(&d->mutex);
    pthread_mutex_lock(&d->mutex);

    size = sizeof(sock);
    if (getpeername(d->descriptor, (struct sockaddr *)&sock, &size))
        d->host = str_dup("(unknown)");
    else {
        h = gethostbyaddr((char*)&sock.sin_addr, sizeof(sock.sin_addr), AF_INET);
        d->host = str_dup(h ? h->h_name: inet_ntoa(sock.sin_addr));
    }

    pthread_mutex_unlock(&d->mutex);
    pthread_exit(NULL);
    return NULL;	// for exclude warning: "control reaches end of non-void function"
}
#endif

/* Windows 95 and Windows NT support functions (copied from Envy) */
#if defined (WIN32)
void gettimeofday (struct timeval *tp, void *tzp)
{
    tp->tv_sec  = time( NULL );
    tp->tv_usec = 0;
}
#endif

