/*
 * mccp.c + mgater
 * (c) Shadow Realms     1999 - 2003 year (Xor) ver 0.01
 * Copyrights see doc/sr.license
 *	overview of mgater see in MGaterD: mgaterd.c
 */

/*
 *	Requiare zlib (http://www.cdrom.com/pub/infozip/zlib/)
 *		(zlib-devel*.rpm for Linux Red Hat)
 *	Xor: "I use Rot14-mccp.patch for create SR MCCP support"
 *		<URL:http://homepages.ihug.co.nz/~icecube/compress/>
 */

#ifndef LINUX
#define     ENOSR           63      /* Out of streams resources */
#endif

#include <ctype.h>
//#include <limits.h>
//#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/telnet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include "libmgater.h"
//#include "merc.h"


DESCRIPTOR_DATA_MUD *tunnel_list = NULL;
varr listen_mgater_list = { sizeof(LISTEN_MGATER), 1 };
varr mgater_sockets = { sizeof(int), 2 };

/* mccp: write_to_descriptor wrapper */
bool	write_to_descriptor(DESCRIPTOR_DATA *d, const char *txt, uint length)
{
//#ifndef NOMGATER
	if (d->out_compress)
		return writeCompressed(d, txt, length);
	else
//#endif
		return write_to_descriptor_nz(d->descriptor, txt, length);
}

/*
 * Lowest level output function.
 * Write a block of text to the file descriptor.
 * If this gives errors on very long blocks (like 'ofind all'),
 *   try lowering the max block size.
 */
/*	without compression (NotZlib) */
bool	write_to_descriptor_nz(int desc, const char *txt, uint length)
{
	uint iStart;
	uint nWrite;
	uint nBlock;

	if (!length)
		length = strlen(txt);
	for (iStart = 0; iStart < length; iStart += nWrite) {
		nBlock = UMIN(length - iStart, 4096);
#if !defined( WIN32 )
		if ((nWrite = write(desc, txt + iStart, nBlock)) < 0) {
#else
		if ((nWrite = send(desc, txt + iStart, nBlock, 0)) < 0) {
#endif
			log_printf("write_to_descriptor: %s", strerror(errno));
			return FALSE;
		}
	}
	return TRUE;
}

 
//int	init_socket(int port);	// comm.c
bool init_mgater_port(LISTEN_MGATER *lmgater)
{
	int *p;

	if (!lmgater->port)
		return FALSE;
	тут хрень
	if ((lmgater->socket = init_socket(lmgater->port)) < 0)
		return FALSE;
	p = varr_enew(&mgater_sockets);
	*p = lmgater->socket;
	log_printf("mgater listen on port %d", lmgater->port);
	return TRUE;
}

LISTEN_MGATER *find_mgater(int socket, int port)
{
	int i;
	LISTEN_MGATER *lmgater;

	for (i = listen_mgater_list.nused; i < listen_mgater_list.nused; i++)
	{
		lmgater = VARR_GET(&listen_mgater_list, i);

		if (lmgater->socket == socket || lmgater->port == port)
			return lmgater;
	}

	return NULL;
}

DESCRIPTOR_DATA_MUD *new_tunnel(void)
{
	DESCRIPTOR_DATA_MUD *dnew;

	if ((dnew = calloc(1, sizeof(*dnew))) == NULL)
	{
		log_printf("new_tunnel: nomem");
		exit(1);
	}

	dnew->outsize		= 4000;
	dnew->outbuf		= malloc(dnew->outsize);
	dnew->insize		= 4000;
	dnew->inbuf		= malloc(dnew->insize);
	//dnew->wait_for_se	= 0;

	dnew->next	= tunnel_list;
	tunnel_list	= dnew;

	return dnew;
}

void init_tunnel(int mgater_socket)
{
	int socket;
	int size;
	struct sockaddr_in addr;
	LISTEN_MGATER *lmgater;
	DESCRIPTOR_DATA_MUD *tunnel;

	size = sizeof(addr);

	if ((lmgater = find_mgater(mgater_socket, 0)) == NULL)
	{
		log_printf("***BUG: init_tunnel: unknown listen socket %d",
				mgater_socket);
		return;
	}
	//getsockname(mgater_socket, (struct sockaddr*) &addr, &size);
	if ((socket = accept(mgater_socket, (struct sockaddr*) &addr, &size)) < 0)
	{
		log_printf("init_tunnel: port %d: accept: %s",
			lmgater->port,
			strerror(errno));
		return;
	}

#if !defined (WIN32)
	if (fcntl(socket, F_SETFL, FNDELAY) < 0) {
		log_printf("init_tunnel: fcntl: FNDELAY: %s",
				strerror(errno));
		close(socket);
		return;
	}
#endif

	log_printf("init_tunnel: port %d: sock.sin_addr: %s",
			inet_ntoa(addr.sin_addr));

	/* check trusted here */
	//....
	
	tunnel = new_tunnel();

	tunnel->socket = socket;
	tunnel->listen = lmgater;
	tunnel->addr = addr;
}

bool check_data_socket(int socket)
{
	fd_set in_set;
	static struct timeval null_time = {0, 0};
	
	FD_ZERO(&in_set);
	FD_SET(socket, &in_set);

	if (select(socket+1, &in_set, NULL, NULL, &null_time) < 0)
	{
		log_printf("check_data_socket: select: %s", strerror(errno));
		exit(1);
	}

	if (FD_ISSET(socket, &in_set))
		return TRUE;

	return FALSE;
}


/*
 *	recive data from tunnel stream
 *	return	>0 if recive new data
 *		0 - no data
 *		-1 - some errors (for close tunnel)
 *			FD_SET(dt->socket, &exc_set);
 */
int read_from_tunnel_socket(DESCRIPTOR_DATA_MUD *tunnel)
{
#define MINSTEPSIZE		(2 + CURR_HEADER_LENGTH + 10)
	int iStart, iOld;

	
	iOld = iStart = tunnel->outtop;

	for (; ;) {
		int nRead;

		if (tunnel->outsize < MAX_BUFTUN_LENGTH)
		{
			if (tunnel->outsize < tunnel->outtop + MINSTEPSIZE)
			{
				/* incraise outsize */
				char *outbuf;
				int size = UMIN(MAX_BUFTUN_LENGTH, 2 * tunnel->outsize);
				outbuf = malloc(size);
				strncpy(outbuf, tunnel->outbuf, tunnel->outtop);
				free(tunnel->outbuf);
				tunnel->outbuf = outbuf;
				tunnel->outsize = size;
			}
		} else {
			if (tunnel->outsize < tunnel->outtop + MINSTEPSIZE)
				return iStart - iOld;
		}

#if !defined (WIN32)
		nRead = read(tunnel->socket, tunnel->outbuf + iStart,
				tunnel->outsize - iStart );
#else
		nRead = recv(tunnel->socket, tunnel->outbuf + iStart,
				tunnel->outsize - iStart , 0 );
#endif

		if (nRead > 0) {
			iStart += nRead;
			tunnel->outtop += nRead;

			/* check more data */
			if (check_data_socket(tunnel->socket))
				continue;

			return iStart - iOld;
/*			if (d->inarray[iStart-1] == '\n'
			||  d->inarray[iStart-1] == '\r')
				break;
*/
		} else if (nRead == 0) {
			log_printf("EOF encountered on read mgater(%s).",
				inet_ntoa(tunnel->addr.sin_addr));
			// close_tunnel(tunnel);
			return -1;
		}
#if !defined (WIN32)
		else if (errno == EWOULDBLOCK
		|| errno ==  EINTR
		|| errno == EAGAIN)
		{
			return iStart - iOld;
		}
#else
		else if ( WSAGetLastError() == WSAEWOULDBLOCK)
		{
			return iStart - iOld;
		}
#endif
		else {
			log_printf("read_from_tunnel_socket: %s: %s",
					inet_ntoa(tunnel->addr.sin_addr),
					strerror(errno));
			// close_tunnel(tunnel);
			return -2;
		}
	}

	return -3; // unsepction exit ! ! BUG
}

bool transfer_cluster_toclbuf(CLUSTER_DATA *cluster,
		DESCRIPTOR_CLIENT_FTUN *d_cft)
{
	return TRUE;
}

void read_tunnel_cluster_v1(DESCRIPTOR_DATA_MUD *tunnel, uint currp,
		CLUSTER_DATA *cluster)
{
	cluster->header_length
		= *((short unsigned int *) (tunnel->outbuf + currp));
	cluster->header_ver
		= *((short int *) (tunnel->outbuf + currp + 2));
	cluster->flags
		= *((short int *) (tunnel->outbuf + currp + 4));
	cluster->id_connect
		= *((short int *) (tunnel->outbuf + currp + 6));
	cluster->id_client
		= *((short int *) (tunnel->outbuf + currp + 8));
	cluster->data_len
		= *((short unsigned int *) (tunnel->outbuf + currp + 10));

	if (cluster->data_len)
		cluster->data = tunnel->outbuf + currp + 12;
	else
		cluster->data = NULL;
}

bool parse_outtunnel_buffer(DESCRIPTOR_DATA_MUD *tunnel)
{
	uint curr_cluster;
	//int cluster_count = 0;

	if (!tunnel->outtop)
		return TRUE;

	curr_cluster = 0;
	for (; curr_cluster + 2 <= tunnel->outtop;)
	{
		short unsigned int header_length;
		short int header_ver;
		CLUSTER_DATA cluster;
		DESCRIPTOR_CLIENT_FTUN * desc_cl;

		header_length = *(short unsigned int *) (tunnel->outbuf + curr_cluster);
		if (2 + header_length > tunnel->outtop - curr_cluster)
		{
			if (curr_cluster == 0
			&& tunnel->outsize == MAX_BUFTUN_LENGTH)
			{
				/* so big cluster!
				 * kick tunnel nah
				 */
				log_printf("parse_outtunnel_buffer: very big header");
				return FALSE;
			}
			goto commit_data;
		}

		header_ver = *(short int *) (tunnel->outbuf + curr_cluster + 2);
		
		switch(header_ver)
		{
			case 1:
				read_tunnel_cluster_v1(tunnel, curr_cluster, &cluster);
				break;
			default:
				log_printf("parse_outtunnel_buffer: Unknown cluster version");
				return FALSE;
		}

		if (cluster.data_len
		&& 2 + header_length + cluster.data_len
			> tunnel->outtop - curr_cluster )
		{
			if (curr_cluster == 0
			&& tunnel->outsize == MAX_BUFTUN_LENGTH)
			{
				/* so big cluster!
				 * kick tunnel nah
				 */
				log_printf("parse_outtunnel_buffer: very big cluster");
				return FALSE;
			}
			goto commit_data;
		}

		/*
		 *	Ok. Now we have filled cluster and ready to
		 *	work with it.
		 */

		if (IS_SET(cluster.flags, FP_INITCONNECT))
		{
			//desc_cl = init_descriptor_from_tunnel(tunnel, &cluster);
		} else
			//desc_cl = lookup_dftunnel(tunnel, cluster.id_connect);

		if (desc_cl)
		{
			if (IS_SET(cluster.flags, FP_CLOSECONNECT))
			{
				//close_descriptor_from_tunnel(tunnel, desc_cl);
			} else {
				if (!transfer_cluster_toclbuf(&cluster, desc_cl))
				{
					write_to_buffer(desc_cl->desc, "\n\rMUD:o-oops\n\r", 0);
					//close_descriptor_from_tunnel(tunnel, desc_cl);
				}
			}
				/*
				 * Кароче бабка... нужно делать общую библиотеку
				 * функций mgater'а и не вы`быватца :)
				 */
		} else {
			log_printf("parse_outtunnel_buffer: unknow client %d [%d]",
					cluster.id_connect, cluster.id_client);
		}

		/*
		 *	Go check next cluster.
		 */
		curr_cluster += 2 + header_length + cluster.data_len;
	}

	commit_data:

	if (curr_cluster)
	{
		if (curr_cluster < tunnel->outtop)
			memmove(tunnel->outbuf,
				tunnel->outbuf + curr_cluster,
				tunnel->outtop - curr_cluster);
		tunnel->outtop -= curr_cluster;
	}

	return TRUE;
}

bool process_input_tunnel(DESCRIPTOR_DATA_MUD *tunnel)
{
	int result;

	while((result = read_from_tunnel_socket(tunnel)) > 0)
	{
		if (!parse_outtunnel_buffer(tunnel))
			return FALSE;
	}

	if (result < 0)
		return FALSE;

	return parse_outtunnel_buffer(tunnel);
}

