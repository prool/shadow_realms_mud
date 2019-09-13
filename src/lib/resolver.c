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
 * $Id: resolver.c,v 1.7 2003/10/07 21:37:45 xor Exp $
 */

#if !defined (WIN32) && !defined (USE_THREADS)

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
//#include "typedef.h"
#include "log.h"
#include "resolver.h"
#include "libmnet.h"

FILE *	rfin = NULL;
FILE *	rfout = NULL;

static void	cleanup(int);
static void	resolver_loop(uid_t uid);

int	rpid;
int	fildes[4];

void resolver_init(uid_t uid)
{
	if (pipe(fildes) < 0 || pipe(fildes+2) < 0) {
		log_printf("resolver_init: pipe: %s", strerror(errno));
		exit(1);
	}

	signal(SIGPIPE, SIG_IGN);

	rpid = fork();
	if (rpid < 0) {
		log_printf("resolver_init: fork: %s", strerror(errno));
		exit(1);
	}

	if (rpid == 0)
		resolver_loop(uid);

	signal(SIGHUP, cleanup);
	signal(SIGQUIT, cleanup);
	signal(SIGILL, cleanup);
	signal(SIGABRT, cleanup);
	signal(SIGFPE, cleanup);
	signal(SIGBUS, cleanup);
	signal(SIGSEGV, cleanup);
	signal(SIGALRM, cleanup);
	signal(SIGTERM, cleanup);
	signal(SIGINT, cleanup);
	signal(SIGTRAP, cleanup);

#if !defined (LINUX)
	signal(SIGEMT, cleanup);
	signal(SIGSYS, cleanup);
#endif

	close(fildes[1]);
	close(fildes[2]);
	rfin = fdopen(fildes[0], "r");
	rfout = fdopen(fildes[3], "w");
	if (rfin == NULL || rfout == NULL) {
		log_printf("resolver_init: fdopen: %s", strerror(errno));
		exit(1);
	}

	setvbuf(rfin, NULL, _IOLBF, 0);
	setvbuf(rfout, NULL, _IOLBF, 0);

	fcntl(fileno(rfin), F_SETFL, O_NONBLOCK);
}

void donehid(void);
void resolver_done(void)
{
	donehid();
	fclose(rfin);
	fclose(rfout);
	kill(rpid, SIGTERM);
	wait(NULL);
}

/* local functions */
void shutdown_prepare(const char *message);
static void cleanup(int s)
{
	resolver_done();
	signal(s, SIG_DFL);
//	if (s == SIGTERM || s == SIGINT)
//	{
		log_printf("Cleanup and quit [%d]....", s);
		shutdown_prepare("Get kill signal.");
		sleep(1);
//	}
	raise(s);
}


/*
 *	format of input string for resolver:
 *		<number of tunnel>:<cid> <char name>@<hostname>
 *	@ - necessary char !!! (minimum string: some_ids@hostname)
 */
static void resolver_loop(uid_t uid)
{
	FILE *fin;
	FILE *fout;
	char buf[128];

	signal(SIGINT, SIG_IGN);
	signal(SIGTRAP, SIG_IGN);

	close(fildes[0]);
	close(fildes[3]);
	fin = fdopen(fildes[2], "r");
	fout = fdopen(fildes[1], "w");
	if (fin == NULL || fout == NULL) {
		log_printf("resolver_loop: fdopen: %s", strerror(errno));
		exit(1);
	}

	if (uid > 0)
	{
		if (setreuid(uid, uid) < 0)
		{
			log_printf("resolver_loop: error change uid to %d: %s",
					uid, strerror(errno));
		}
	}

	setvbuf(fin, NULL, _IOLBF, 0);
	setvbuf(fout, NULL, _IOLBF, 0);

	while(fgets(buf, sizeof(buf), fin)) {
		struct in_addr addr;
		struct hostent *hostent;
		char /* *p_id_connect, *p_charname, */ *p_addr, *p;
		/*int idtun, id_connect;
		DESCRIPTOR_DATA_TUNNEL	* d_tunnel;
		DESCRIPTOR_DATA_CLIENT	* client;
		*/

		if ((p = strchr(buf, '\n')) == NULL)
		{
			while(fgetc(fin) != '\n')
				;
			continue;
		}
		*p = '\0';		// create simple string with '\0' at end

		if ((p_addr = strchr(buf, '@')) == NULL)
			return;
		*p_addr++	= '\0';

		/* now buf is pointer of string "idtun:id_connect charname" */
		log_printf("resolver_loop: %s@%s", buf, p_addr);

		inet_aton(p_addr, &addr);
		hostent = gethostbyaddr((char*) &addr, sizeof(addr), AF_INET);
		fprintf(fout, "%s@%s\n",
			buf, hostent ? hostent->h_name : p_addr);
	}

	if (errno)
		log_printf("resolver_loop: %s", strerror(errno));
	fclose(fin);
	fclose(fout);
	exit(0);
}

void	resolv_client_check_start(int idtun, int cid,
		const char * charname, struct in_addr * ip_addr)
{
	fprintf(rfout, "%d:%d %s@%s\n",
			idtun, cid,
			charname, inet_ntoa(*ip_addr));
}

void	resolv_check()
{
	char buf[MAX_STRING_LENGTH];
	char *p;
	char *p_id_socket, *p_charname, *p_addr;
	int idtun, cid;
	DESCRIPTOR_DATA_TUNNEL	* d_tunnel;
	DESCRIPTOR_DATA_CLIENT	* client;

	while (fgets(buf, sizeof(buf), rfin))
	{
		if ((p = strchr(buf, '\n')) == NULL)
		{
			log_printf("resolv_check: rfin: line too long, skipping to '\\n'");
			while(fgetc(rfin) != '\n')
				;
			continue;
		}
		*p = '\0';
		if ((p_id_socket = strchr(buf, ':')) == NULL
		|| (p_charname = strchr(buf, ' ')) == NULL
		|| (p_addr = strchr(buf, '@')) == NULL)
			continue;

		*p_id_socket++	= '\0';
		*p_charname++	= '\0';
		*p_addr++	= '\0';
		/* now buf is pointer of string with idtun ... */

		idtun = atoi(buf);
		cid = atoi(p_id_socket);

			/* find tunnel */
		if ((d_tunnel = varr_get(&tunnel_list, idtun)) == NULL)
		{
			log_printf("resolv_check: uknown tunnel: %d: sock %d"
					" %s@%s", idtun,
				cid, p_charname, p_addr);
			continue;
		}
			/* find client */
		for (client = d_tunnel->clients; client; client = client->next)
			if (client->connect && get_idsock_connect(client) == cid)
				break;
		if (client == NULL)
		{
			log_printf("resolv_check: uknown client: %d: tid %d"
					" %s@%s", cid,
				idtun, p_charname, p_addr);
			continue;
		}

		if (client->connect == NULL)
		{
			log_printf("resolv_check: client can't connect: %s",
					strid_dc(client));
		}
		if (client->hostname)
		{
			log_printf("resolv_check: client already has hostname: %s",
					strid_dc(client));
			free_string(client->hostname);
			client->hostname = NULL;
		}

		client->hostname = str_dup(p_addr);
		log_printf("resolv_check: [ok] %s", strid_dc(client));
	}
}

#endif /* !USE_THREADS */

