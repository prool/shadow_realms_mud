/*
 * $Id: comm_control.c,v 1.32 2007/01/27 22:52:13 rem Exp $
 */

#include <sys/types.h>
#if	!defined (WIN32)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#else
#	include <winsock.h>
#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "merc.h"
#include "comm_control.h"
#include "comm_colors.h"

extern int	max_on;

CONTROL_DESC *	ctrl_list;
int		top_id;

static CONTROL_DESC *	id_free_list;

static CONTROL_DESC *	control_desc_new(DESCRIPTOR_DATA_CLIENT *client);
//static void		control_desc_free(CONTROL_DESC *id);

CONTROL_DESC * control_newconn(DESCRIPTOR_DATA_CLIENT *client)
{
//	int fd;
//	struct sockaddr_in sock;
//	int size = sizeof(sock);
	CONTROL_DESC *id;
//	int i;

#if 0
	getsockname(infofd, (struct sockaddr*) &sock, &size);
	if ((fd = accept(infofd, (struct sockaddr*) &sock, &size)) < 0) {
		log_printf("info_newconn: accept: %s", strerror(errno));
		return;
	}

	if (getpeername(fd, (struct sockaddr *) &sock, &size) < 0) {
		log_printf("info_newconn: getpeername: %s", strerror(errno));
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		return;
	}

	log_printf("info_newconn: sock.sin_addr: %s", inet_ntoa(sock.sin_addr));

	for (i = 0; i < info_trusted.nused; i++) {
		struct in_addr* in_addr = VARR_GET(&info_trusted, i);
		if (!memcmp(in_addr, &sock.sin_addr, sizeof(struct in_addr)))
			break;
	}

	if (i >= info_trusted.nused) {
		log_printf("info_newconn: incoming connection refused");
#ifdef WIN32
		closesocket(fd);
#else
		close(fd);
#endif
		return;
	}

#if !defined (WIN32)
	if (fcntl(fd, F_SETFL, FNDELAY) < 0) {
		log_printf("info_newconn: fcntl: FNDELAY: %s", strerror(errno));
		close(fd);
		return;
	}
#endif
#endif

	id = control_desc_new(client);
	id->next = ctrl_list;
	ctrl_list = id;
	REMOVE_BIT(client->flags, DDCF_NEWCONNECT);	// !!!
	return id;
}

void control_process_cmd(CONTROL_DESC *id, const char * argument)
{
	BUFFER *output;
	char * p_tmp;
	char buf[MAX_STRING_LENGTH * 2];
	int format;
	//char *p;
	//char *q;
	DESCRIPTOR_DATA_MUDDY *d;
	//int nread;

#if 0
#if !defined (WIN32)
	if ((nread = read(id->fd, buf, sizeof(buf))) < 0)
	 {
		if (errno == EWOULDBLOCK)
#else
	if ((nread = recv(id->fd, buf, sizeof(buf), 0)) < 0)
	 {
        if ( WSAGetLastError() == WSAEWOULDBLOCK)
#endif
			return;
		log_printf("info_input: read: %s", strerror(errno));
		goto bail_out;
	}
	buf[nread] = '\0';

	for (p = buf; *p && strchr(" \n\r\t", *p); p++)
		;

	if ((q = strpbrk(p, " \n\r\t")))
		*q = '\0';
#endif
	log_printf("process_who: output format requested: '%s'", argument);
	format = format_lookup(argument);

	output = buf_new(-1);

	buf_printf(output, "%d\n", max_on);
#if 0
	p = buf_string(output);
#if !defined (WIN32)
	write(id->fd, p, strlen(p));
#else
	send(id->fd, p, strlen(p), 0);
#endif
#endif
	p_tmp = buf_string(output);
	if (!write_to_buffer_client(id->client, p_tmp, 0))
	{
		close_descriptor_control(id);
		return;
	}

	for (d = descriptor_list_muddy; d; d = d->next)
	{
		CHAR_DATA *wch = d->original ? d->original : d->character;

		if (!wch
		||  d->connected != CON_PLAYING
		||  wch->pcdata == NULL
		||  wch->pcdata->invis_level
		||  wch->pcdata->incog_level
		||  IS_AFFECTED(wch, AFF_FADE | AFF_HIDE | AFF_CAMOUFLAGE | 
				     AFF_INVIS | AFF_IMP_INVIS))
			continue;

		buf_clear(output);
		do_who_raw(NULL, wch, output);
		parse_colors(buf_string(output), buf, sizeof(buf), format);
/*
#if !defined (WIN32)
		write(id->fd, buf, strlen(buf));
#else
		send(id->fd, buf, strlen(buf), 0);
#endif
*/
		if (!write_to_buffer_client(id->client, buf, 0))
		{
			close_descriptor_control(id);
			return;
		}
	}
	buf_free(output);

//bail_out:
	close_descriptor_control(id);
}

void close_descriptor_control(CONTROL_DESC *id)
{
	if (id->client && id->client->connect)
	{
		close_descriptor_client(id->client, NULL);
	}
}

static CONTROL_DESC *control_desc_new(DESCRIPTOR_DATA_CLIENT *client)
{
	CONTROL_DESC *id;

	if (id_free_list) {
		id = id_free_list;
		id_free_list = id_free_list->next;
	}
	else {
		top_id++;
		id = malloc(sizeof(*id));
		if (id == NULL)
			crush_mud();
	}
	memset(id, 0, sizeof(*id));

	id->client = client;

	return id;
}

void control_desc_fulldestroy(CONTROL_DESC *id)
{
	if (id == ctrl_list)
		ctrl_list = ctrl_list->next;
	else {
		CONTROL_DESC *prev;

		for (prev = ctrl_list; prev; prev = prev->next)
			if (prev->next == id)
				break;

		if (!prev) {
			log("info_desc_free: descriptor not found");
			return;
		}

		prev->next = id->next;
	}

/*
#ifdef WIN32
	closesocket(id->fd);
#else
	close(id->fd);
#endif
*/
	if (id->client)
	{
		destroy_descriptor_client(id->client);
		id->client = NULL;
	}

	id->next = id_free_list;
	id_free_list = id;
}

