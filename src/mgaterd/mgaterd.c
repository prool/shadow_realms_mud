/*
 *	MGaterD (MUD Gater Daemon)
 *	(c) Shadow Realms	2003 (Xorader) ver 1.00
 *	Copyrights see doc/SR/sr.license
 */

/*
 *	About:
 *		Принимает "telnet" соединение(я) на
 *		указанный порт (по умолчанию на DEFAULT_LISTCLI_PORT)
 *		и тунелирует его на порт MUD'а (по умолчанию
 *		на localhost:DEFAULT_LISTTUN_PORT <- не в src, а в startup'е)
 *	Tunnel cluster (version 1):
 *		Header length:	1 byte - max MAX_HEADER_LENGTH bytes (255)
 *		Header:		<see above>
 *			Version:	1 byte
 *			Flags:		2 bytes (16 bits)
 *				Bits:
 *				0	- ...
 *			Id connect:	2 bytes (16 bits) - max 65535 connects
 *					if 0 then ... ?   --		cid
 *			Id client:	4 bytes (some address, by IP) -	hid
 *			Data length:	2 bytes - max MAX_BUF_LENGTH bytes
 *		Data:		XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
 *	Features:
 *	TODO:
 *		* pipe:MUD from localhost:MGaterD
 *		* WIN-OS support
 *		* Разрулить коды выхода (в #define'ы их)
 */

/*
 *	mgater TODO:
 *		- support common MCCP protocol
 *		- access mud list (by IP, by MAC)
 *		- access list for clients (by IP, by MAC, char name)
 *		- add internal IP
 *		- add internal virtual number
 *		- add SSL (may be compress by it!) !!!!
 *		- add control interface (with tcp_wrappers?)
 *		- logging (rotate logs)
 *		- control by *nix singals (SIGHUP, SIGTERM)
 *		- auto demonize (no tty, no group, chdir "/")
 *		- auto get other EUID (protect from ROOT permission)
 *		- safe shutdown (warning message and wait some time
 *				before shutdown)
 *		- resolver IP client (DNS) ?
 */

#include <sys/types.h>
#if !defined(WIN32)

//#define __USE_XOPEN_EXTENDED
#include <unistd.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <pwd.h>
#else
//#       include <winsock.h>
//#       include <sys/timeb.h>
#endif


//#include <ctype.h>
//#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//#include <fcntl.h>

#include <libmnet.h>
#include <libmuddy.h>

#include "const.h"
#include "mgaterd.h"

#include "control.h"
#include "tunneling.h"

//#include "descriptor_cl.h"
//#include "tunnel.h"

/* Global varibales */
//varr		mgater_sockets;
//varr		control_sockets;

			/* listening sockets for clients */
//varr			client_sockets = { sizeof(int), 2 };
varr			client_sockets = { sizeof(struct sockaddr_in), 2 };
			/* listening control sockets for admins */
//varr			control_sockets = { sizeof(int), 2 };

DESCRIPTOR_DATA_MAIN	* mtunnel;
		/* All open clients descriptors for tunnelize */
DESCRIPTOR_DATA_CMUDDY	* descriptor_list_cmuddy;

int			tunnelize_clients;
//int			admin_clients;

void initconf_mgater(struct sockaddr_in * fromhost, struct sockaddr_in *tohost);
void mgater_done(int exit_code, const char *str);
DESCRIPTOR_DATA_CMUDDY * init_descriptor_cmuddy(DESCRIPTOR_DATA_CLIENT *client);
void free_descriptor_cmuddy(DESCRIPTOR_DATA_CMUDDY *d);
DESCRIPTOR_DATA_CMUDDY *new_descriptor_cmuddy(void);
void write_to_buffer_cmuddy(DESCRIPTOR_DATA_CMUDDY *d, const char *txt, \
		uint length);
void close_descriptor_cmuddy(DESCRIPTOR_DATA_CMUDDY *dclose);
void destroy_descriptor_cmuddy(DESCRIPTOR_DATA_CMUDDY *dclose);

void mgater_work();

static void print_usage(const char *name)
{
	printf("Usage: %s [options]\n"
	       		"Where options:\n"
			"\t-h         -- this help\n"
			"\t-c <cport> -- control service port\n"
			"\t-p <port>  -- listen port (default %d)\n"
			"\t-f <port>  -- from that port send (otherwise system chose above 1024)\n"
			"\t-D         -- auto demonaiZe mgater\n"
			"\t-u <user>  -- set user (and e_user) of proc\n"
			"", get_filename(name),
			DEFAULT_LISTCLI_PORT);
}

int main(int argc, char **argv)
{
	bool			do_demon = FALSE;
	struct sockaddr_in	tohost;		/* tunneling to host */
	struct sockaddr_in	fromhost;
//	struct sockaddr_in	fromhost;	/* bind address */
				/* bind address - from which ip/ports
				 * connect 'tohost'
				 */
//	varr			fromhost = { sizeof(struct sockaddr_in),
//					2, NULL, 0, 0 };
//	int			fromport_totarget = -1;
//	FILE		*fp;
//	int		counter = 0;
//	char		from_port[MAX_INPUT_LENGTH];

#if !defined(WIN32)
	pid_t		pid;
	uid_t		uid;
	uid_t		mgaterd_uid = 0;
#endif

#if !defined(WIN32)
	logfile = NULL;

	if (dfexist(DIR_HOME, PID_FILE))
	{
		printf("Error. Pid file already exits. Remove it before start mgaterd:\n");
		printf("\t%s/%s\n", DIR_HOME, PID_FILE);
		exit(ERRMG_FIND_PID);
	}
#endif
	
	//setlocale(LC_ALL, "");
	initconf_mgater(&fromhost, &tohost);
	//from_port[0] = '\0';

	if (argc > 1)
	{
		int ch;

		opterr = 0;
		
		while ((ch = getopt(argc, argv, "p:f:u:c:Dh")) != -1) {
			switch (ch) {
			case 'u':
				{
					struct passwd *spass;

					errno = 0;
					if ((spass = getpwnam(optarg)) == NULL
					|| (mgaterd_uid = spass->pw_uid) <= 0)
					{
						log_printf("some error user %s:"
							" %s", optarg,
							strerror(errno));
						mgater_done(ERRMG_WRONG_ARGUMENTS,
							"Incorrect user.");
					}
				}
			case 'p':
				client_sockets.nused = 0;
				if (!is_number(optarg))
				{
					print_usage(argv[0]);
					mgater_done(ERRMG_WRONG_ARGUMENTS,
						"Incorrect listening port.");
				}
				add_simpleport(&client_sockets, atoi(optarg));
				break;
			case 'c':
				printf("Not released yet :(\n");
				break;
			case 'f':
				if (!is_number(optarg))
				{
					print_usage(argv[0]);
					mgater_done(ERRMG_WRONG_ARGUMENTS,
						"Incorrect 'from' port.");
				} else {
					memset(&fromhost, 0, sizeof(fromhost));
#if !defined (WIN32)
					fromhost.sin_family = AF_INET;
#else
					fromhost.sin_family = PF_INET;
					fromhost.sin_addr.s_addr = INADDR_ANY;
#endif
					fromhost.sin_port = htons(atoi(optarg));
				}
				break;
			case 'D':
				do_demon = TRUE;
				break;
			case 'h':
				print_usage(argv[0]);
				mgater_done(ERRMG_NONE, ".");
			default:
				print_usage(argv[0]);
				mgater_done(ERRMG_WRONG_ARGUMENTS,
					"Incorrect parametr(s).");
			}
		}
		argc -= optind;
		argv += optind;
	}
#if !defined(WIN32)
	if (do_demon)
	{
		setsid();
		setpgrp();
		if((pid = fork()) == 0) {
			log_printf("Started as daemon successful.");
		} else if (pid < 0) {
			printf("failed: %s\n", strerror(errno));
			mgater_done(ERRMG_FAIL_FORK, "Fork error!");
		} else
			exit(0);
	} else
		pid = getpid();
#endif

#if !defined (WIN32) && !defined (USE_THREADS)
	resolver_init(mgaterd_uid);
#endif

	if (client_sockets.nused == 0)
	{
		/* set default listening port*/
		add_simpleport(&client_sockets, DEFAULT_LISTCLI_PORT);
		log_printf("Set default listening port: %d",
				DEFAULT_LISTCLI_PORT);
	}

	//if (!add_tunnel_by_str(argv[1+counter], argv[2+counter], from_port, NULL))
	if (!inithid(DIR_HOME, HID_FILE))
	{
		mgater_done(ERRMG_FAIL_CREATE_TUNNEL, "can't init hid db");
	}
	
	tunnelize_clients = init_local_clients_tunnel(&client_sockets,
			"init listening tunnel for clients");
	if (tunnelize_clients < 0)
	{
		mgater_done(ERRMG_FAIL_CREATE_TUNNEL, "can't create listening tunnel");
	}

	if ((mtunnel = connect_to_muddy(&fromhost, &tohost)) == NULL)
	{
		mgater_done(ERRMG_FAIL_CREATE_TUNNEL, "can't connect to remote muddy");
	}

	if (mgaterd_uid > 0)
	{
		uid = getuid();
		if (setreuid(mgaterd_uid, mgaterd_uid) < 0)
		{
			log_printf("Error to change uid(euid)[%d] from %d: %s",
				mgaterd_uid, uid, strerror(errno));
			mgater_done(ERRMG_FAIL_REUID,
				"fail change uid");
		}
	}

#if !defined(WIN32)
	log_printf("MGaterD pid is %d", pid);
	{
		FILE *fp = dfopen(DIR_HOME, PID_FILE, "w");
		if (!fp) {
			log_printf("Error: cann't create pid file: %s.",
					strerror(errno));
		} else {
			fprintf(fp, "%d", pid);
			fclose(fp);
		}
	}
#endif

	if (!begin_work_with_tunnel(tunnelize_clients))
	{
		mgater_done(ERRMG_FAIL_INIT_TUNNEL, "can't init tunnel");
	}

	log_printf("Ready for work!");

	mgater_work();

	/*
	 *	That's all...
	 */

	//write(socket_to, "Done connect\n", strlen("Done connect\n")+1);
	//sleep(10);

	shutdown_tunnel(tunnelize_clients);

	mgater_done(ERRMG_NONE, str_empty);
	return ERRMG_UNKNOWN;
}

void mgater_work()
{
	bool			mg_work = TRUE;
	struct timeval		last_time;
	time_t			current_time;	/* time of this pulse */
	DESCRIPTOR_DATA_CMUDDY	* dtm, * dtm_next; 
	DESCRIPTOR_DATA_CLIENT	* client, * dc_next;

	gettimeofday(&last_time, NULL);
	current_time = (time_t) last_time.tv_sec;

	while (mg_work)
	{
		/*
		if (select(maxsock+1, &in_set, &out_set, &exc_set,
			&null_time) < 0)
		{
			log_printf("mgaterd_loop: select: %s", strerror(errno));
			log_printf("mgaterd_loop: null_time: tv_sec=%d tv_usec=%d",
				null_time.tv_sec, null_time.tv_usec);
			mgater_done(ERRMG_FAIL_SELECT, str_empty);
		}
		*/
		if (!process_in_all_tunnels())
		{
			mgater_done(ERRMG_FAIL_NETWORK, "mgater_work: "
					"network error");
		}

		/* check new connections of clients */
		for (client = ((DESCRIPTOR_DATA_TUNNEL *)
			VARR_GET(&tunnel_list, tunnelize_clients))->clients;
				client; client = dc_next)
		{
			dc_next = client->next;
			if (IS_SET(client->flags, DDCF_NEWCONNECT))
			{
				dtm = init_descriptor_cmuddy(client);
			}
		}
			
		/**********************************************************/
		/*
		 *	Process input from clients.
		 */
		for (dtm = descriptor_list_cmuddy; dtm; dtm = dtm_next)
		{
			dtm_next	= dtm->next;

			/*
			 *	Kick out the freaky folks (1).
			 */
			if (IS_SET(dtm->client->flags, DDCF_NEEDCLOSE)
			|| dtm->client->connect == NULL)
			{
				//d->outtop = 0;
				destroy_descriptor_cmuddy(dtm);
				continue;
			}
			// ... dm->client->inarray
		}

		/*
		 *	Autonomous Tgame motion.............
		 */
			// check_spam(d);
		
		/**********************************************************/
		/* Process output to muds from clients aka send to servers
		 *	Tunelize...
		 */
		// ...
		/*
		for (dtm = descriptor_list_cmuddy; dtm; dtm = dtm_next)
		{
			dtm_next = dtm->next;
			if (check_out_ready_client(dtm->client))
			//|| dm->needPrompt)
				process_output_tmuddy( dm, TRUE );
		}
		*/

		if (!process_tunneling_in())	// see tunneling.c
		{
			mgater_done(ERRMG_FAIL_NETWORK, "mgater_work: "
					"break connecting to muddy (in)");
		}
		
		/**********************************************************/
		/* Pause */
		// Synchronize to a clock. -> move here
		/*
		 *	Synchronize to a clock.
		 *	Sleep(last_time + 1/MG_PULSE_PER_SCD - now).
		 *	Careful here of signed versus unsigned arithmetic.
		 */
#if !defined (WIN32)
		{
			struct timeval now_time;
			long secDelta;
			long usecDelta;

			gettimeofday(&now_time, NULL);
			usecDelta	= ((int) last_time.tv_usec)
						- ((int) now_time.tv_usec)
						+ 1000000 / MG_PULSE_PER_SCD;
			secDelta	= ((int) last_time.tv_sec)
						- ((int) now_time.tv_sec);
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
				if (select(0, NULL, NULL, NULL, &stall_time) < 0)
				{
					log_printf("mgaterd_loop: select: stall: %s",
							strerror(errno));
					mgater_done(ERRMG_FAIL_SELECT_P, str_empty);
				}
			}
		}
#else
		/* Win Synchronize Not supported yet */
#pragma Not supported Win Synchronize

#endif
		gettimeofday(&last_time, NULL);
		current_time = (time_t) last_time.tv_sec;

		/**********************************************************/
		/* Receive from servers */

		//tunnel_out(tunnel);
		// собстно вставляем туннель в сам МАД :)

		if (!process_tunneling_out())	// see tunneling.c
		{
			mgater_done(ERRMG_FAIL_NETWORK, "mgater_work: "
					"break connecting to muddy (out)");
		}

		/**********************************************************/
		/*
		 *	Output to clients.
		 */
		if (!process_out_all_tunnels())
		{
			mgater_done(ERRMG_FAIL_NETWORK, "mgater_work: "
					"network error(2)");
		}

		/*
		 * Kick out the freaky folks (2).
		 */
		for (dtm = descriptor_list_cmuddy; dtm; dtm = dtm_next)
		{
			dtm_next	= dtm->next;

			if (IS_SET(dtm->client->flags, DDCF_NEEDCLOSE)
			|| dtm->client->connect == NULL)
			{
				//d->outtop = 0;
				destroy_descriptor_cmuddy(dtm);
				continue;
			}
		}
	}
}

void shutdown_prepare(const char *str)
{
	if (dfexist(DIR_HOME, PID_FILE)
	&& dunlink(DIR_HOME, PID_FILE) < 0)
		printf("Can't remove pid file: %s\n", strerror(errno));
}

/*
static void cleanup(int s)
{
	signal(s, SIG_DFL);
//	if (s == SIGTERM
//	|| s == SIGQUIT)
//	{
		// send_to_all("Get kill signal. Extra shutdown...\n");
		log_printf("Cleanup and quit [%d]....", s);
		shutdown_prepare(str_empty);
		sleep(1);
//	}
	raise(s);
}
*/

void mgater_done(int exit_code, const char *str)
{
	if (!IS_NULLSTR(str))
		printf("%s\n", str);
	resolver_done();
	shutdown_prepare(str_empty);
	exit(exit_code);
}

/*
 *	Load config(s) file and init some values
 */
void initconf_mgater(struct sockaddr_in * fromhost, struct sockaddr_in *tohost)
{
	char	filename[MAX_STRING_LENGTH];
	FILE	*fp;
	char	*word;
	bool	fMatch;

	snprintf(filename, sizeof(filename), "%s%c%s",
			DIR_HOME, PATH_SEPARATOR, CONF_FILE);
	log_printf("read config file: %s", CONF_FILE);
	if ((fp = dfopen(DIR_HOME, CONF_FILE, "r")) == NULL) {
		mgater_done(ERRMG_FAIL_READ_CONFIG, "Can't read config file");
	}

	*((int *) (&tohost->sin_addr))	= 0;
	//*((int *) (&fromhost->sin_addr))  = 0;
	memset(fromhost, 0, sizeof(*fromhost));
	line_number = 1;
	for (;;) {
		if (feof(fp) || (word = fread_word(fp)) == NULL)
			word = "End";
		fMatch = FALSE;
		switch(to_upper_case(word[0])) {
			case '*':
			case '#':
			case ';':
				fMatch = TRUE;
				fread_to_eol(fp);
				break;
			case 'F':
				if (!str_cmp(word, "From"))
				{
					if (fread_addr(fp, fromhost))
						fMatch = TRUE;
					break;
				}
			case 'E':
				if (! (*((int *) (&tohost->sin_addr)))
				/*||  ! (*((int *) (&fromhost->sin_addr)))*/)
				{
					mgater_done(ERRMG_FAIL_READ_CONFIG,
						"read cfg: Don't choose 'To' or 'From' address");
				}
				if (!str_cmp(word, "End"))
					return;
				break;
			case 'T':
				if (!str_cmp(word, "To"))
				{
					if (fread_addr(fp, tohost))
						fMatch = TRUE;
					break;
				}
				break;
		}
		if (!fMatch)
		{
			log_printf("File '%s', Line '%d', String '%s' <- error",
					filename, line_number, word);
			mgater_done(ERRMG_FAIL_READ_CONFIG, "Error in config file");
		}
	}
		
	fclose(fp);
}

DESCRIPTOR_DATA_CMUDDY *init_descriptor_cmuddy(DESCRIPTOR_DATA_CLIENT *client)
{
	DESCRIPTOR_DATA_CMUDDY *dnew;

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor_cmuddy();

	dnew->client	= client;
	dnew->next	= descriptor_list_cmuddy;
	descriptor_list_cmuddy = dnew;

	/*
	 *	Send the greeting.
	 */
	write_to_buffer_cmuddy(dnew, GREETING, sizeof(GREETING));

	REMOVE_BIT(client->flags, DDCF_NEWCONNECT);	// !

	SET_BIT(dnew->flags, DDCF_REQ_REMOTE_INIT);
	return dnew;
}

/* stuff for recycling descriptors */
DESCRIPTOR_DATA_CMUDDY *descriptor_tmuddy_free;

DESCRIPTOR_DATA_CMUDDY *new_descriptor_cmuddy(void)
{
	DESCRIPTOR_DATA_CMUDDY *d;

	if (descriptor_tmuddy_free == NULL) {
		d = malloc(sizeof(*d));
		if (d == NULL)
			mgater_done(ERRMG_NOMEM, "new_descriptor_cmuddy: nomem");
	} else {
		d = descriptor_tmuddy_free;
		descriptor_tmuddy_free = descriptor_tmuddy_free->next;
	}

	memset(d, 0, sizeof(*d));
	return d;
}

void free_descriptor_cmuddy(DESCRIPTOR_DATA_CMUDDY *d)
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
	d->next = descriptor_tmuddy_free;
	descriptor_tmuddy_free = d;
}

/*
 * Append onto an output buffer (output to client).
 */
void write_to_buffer_cmuddy(DESCRIPTOR_DATA_CMUDDY *d, const char *txt,
					uint length)
{
	if (d->client->connect)
	{
		if (!write_to_buffer_client(d->client, txt, length))
		{
			close_descriptor_cmuddy(d);
			return;
		}
	}
	return;
}

void close_descriptor_cmuddy(DESCRIPTOR_DATA_CMUDDY *dclose)
{
	if (dclose->client && dclose->client->connect)
	{
		close_descriptor_client(dclose->client, NULL);
	}
}

void destroy_descriptor_cmuddy(DESCRIPTOR_DATA_CMUDDY *dclose)
{
//	CHAR_DATA *ch;
//	DESCRIPTOR_DATA_CMUDDY *d;

	/*
	for (d = descriptor_list_cmuddy; d; d = d->next)
		if (d->snoop_by == dclose)
			d->snoop_by = NULL;

	if ((ch = dclose->character)) {
		save_finger_char(ch);
		log_printf("Closing link to %s.", ch->name);
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
			default:
				free_char(dclose->character);
				break;
		}
	}
	*/

	if (dclose == descriptor_list_cmuddy)
		descriptor_list_cmuddy = descriptor_list_cmuddy->next;
	else {
		DESCRIPTOR_DATA_CMUDDY *d;

		for (d = descriptor_list_cmuddy; d && d->next != dclose;
						d = d->next)
			;
		if (d)
			d->next = dclose->next;
		else
			bug("destroy_descriptor_cmuddy: dclose not found.", 0);
	}

	if (dclose->client)
	{
		destroy_descriptor_client(dclose->client);
		dclose->client = NULL;
	}
	free_descriptor_cmuddy(dclose);
}

