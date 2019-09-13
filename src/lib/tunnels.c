/*
 * tunnels.c		<MGater libs>
 */

#include <stdlib.h>
#include <errno.h>
#include <string.h>
//#define __need_timeval
//#undef _BITS_TIME_H
//#undef _STRUCT_TIMEVAL
//#include <bits/time.h>
#include <sys/time.h>
#include <time.h>
#include "libmnet.h"

varr		tunnel_list = { sizeof(DESCRIPTOR_DATA_TUNNEL), 2 };
insockets_t	tunnels_listening = { -1 } ;	/* socket for accept connect
						 *  new remote tunnels
						 */
varr		connect_list = { sizeof(DESCRIPTOR_DATA_MAIN), 2 };

/*
 *	return id of tunnel
 *	(-1) - error
 */

int new_tunnel(void)
{
	DESCRIPTOR_DATA_TUNNEL	* dnew = NULL;
	DESCRIPTOR_DATA_TUNNEL	* itun;
	int	i;
//	int	id = -1;

	
	for (i = 0; i < tunnel_list.nused; i++)
	{
		itun = VARR_GET(&tunnel_list, i);
		if (itun->idtun == -1)
		{
			/* has finded free slot */
			dnew = itun;
			memset(dnew, 0, sizeof(*dnew));
			dnew->idtun = i;
		}
	}

	if (!dnew)
	{
		dnew = varr_enew(&tunnel_list);
		dnew->idtun = tunnel_list.nused - 1;
	}
	/*
	if ((dnew = calloc(1, sizeof(*dnew))) == NULL)
	{
		log_printf("new_tunnel: nomem");
		return NULL;
	}
	*/
	dnew->listening_sockets.nsize	= sizeof(insockets_t);
	dnew->listening_sockets.nstep	= 1;
	dnew->from_connect		= NULL;
	dnew->clients			= NULL;
	

//	varr_qsort(&tunnel_list, cmpint);	// !!! NOT!
//	dnew->next			= tunnel_list;
//	tunnel_list			= dnew;

	return dnew->idtun;
}

void	free_tunnel(int idtun)
{
	DESCRIPTOR_DATA_TUNNEL	* dt = varr_get(&tunnel_list, idtun);

	if (!dt)
		return;
	if (dt->listening_sockets.p)
		varr_free(&(dt->listening_sockets));

	memset(dt, 0, sizeof(*dt));

	dt->idtun = -1;
	/* that's all :) */
}

void	add_simpleport(varr * addrs, int port)
{
	struct sockaddr_in * la;

	la = varr_enew(addrs);
#if !defined (WIN32)
	la->sin_family  = AF_INET;
//	la->sin_addr.s_addr=htonl(INADDR_ANY);
#else
	la->sin_family  = PF_INET;
	la->sin_addr.s_addr = INADDR_ANY;
#endif
	la->sin_port	= htons(port);
}

extern FILE * filedata_d;

int	init_remote_clients_tunnel(insockets_t * insock)
{
	DESCRIPTOR_DATA_TUNNEL	* d_tunnel;
	int			idtun;

	if (!filedata_d)
	{
		log_printf("not initialized hid db");
		return -1;
	}

	if ((idtun = new_tunnel()) == -1)
		return -1;
	d_tunnel = VARR_GET(&tunnel_list, idtun);

	if ((d_tunnel->from_connect = init_descriptor_main(insock)) == NULL)
	{
		return -1;
	}

	d_tunnel->hid = gethid(d_tunnel->from_connect->addr.sin_addr, TRUE);
	log_printf("init new tunnel %d: %s",
			idtun, strid_dm(d_tunnel->from_connect));

	return idtun;
}

/*
 * Init tunnel for clients(aka end users)
 *	localports - associated ports for this tunnel. Will bind in this func,
 *			varr of (struct sockaddr_in)'s.
 */
int	init_local_clients_tunnel(varr *localports, const char * msg)
{
	DESCRIPTOR_DATA_TUNNEL	* d_tunnel;
	int			i, idtun;
	int			socket;
	struct sockaddr_in	* bindaddr;
	insockets_t 		* listen_data;

	if (localports->nused <= 0)
	{
		return -1;
	}
	if (!filedata_d)
	{
		log_printf("not initialized hid db");
		return -1;
	}

	if ((idtun = new_tunnel()) == -1)
		return -1;

	d_tunnel = VARR_GET(&tunnel_list, idtun);

	d_tunnel->from_connect	= NULL;		// local tunnel
	d_tunnel->hid		= -1;

	for (i = 0; i < localports->nused; i++)
	{
		bindaddr = ((struct sockaddr_in *) VARR_GET(localports, i));

		if ((socket = init_socket(-1, bindaddr, TRUE, TRUE)) > 0)
		{
			listen_data = varr_enew(&(d_tunnel->listening_sockets));

			//listen_data->port		= ntohs(bindaddr->sin_port);
			memcpy(&(listen_data->listen), bindaddr, sizeof(*bindaddr));
			//listen_data->listen		= * bindaddr;
			listen_data->socket		= socket;
			listen_data->trusted_addr.nsize	= sizeof(struct in_addr);
			listen_data->trusted_addr.nstep	= 1;
			listen_data->idtun		= idtun;

			log_printf("%s: bind %d port success (tun: %d)",
					msg ? msg : str_empty,
					ntohs(bindaddr->sin_port),
					idtun);
		}
	}

	return idtun;
}

/*
 *	return TRUE if all ok
 */
bool init_tunnels_listening(int port, struct sockaddr_in * bind_addr)
{
	memset(&tunnels_listening, 0, sizeof(tunnels_listening));

	if ((tunnels_listening.socket
		= init_socket(port, bind_addr, TRUE,TRUE)) < 0)
	{
		log_printf("fail init listening socket for remote tunnels");
		return FALSE;
	}

	if (bind_addr)
	{
		tunnels_listening.listen	= * bind_addr;
	} else {
		memset(&(tunnels_listening.listen), 0,
				sizeof(tunnels_listening.listen));
#if !defined (WIN32)
		tunnels_listening.listen.sin_family	= AF_INET;
#else
		tunnels_listening.listen.sin_family	= PF_INET;
		tunnels_listening.listen.sin_addr.s_addr = INADDR_ANY;
#endif
		tunnels_listening.listen.sin_port	= htons(port);
	}
	tunnels_listening.trusted_addr.nsize	= sizeof(struct in_addr);
	tunnels_listening.trusted_addr.nstep	= 1;
	tunnels_listening.idtun			= -1; // ;)

	log_printf("bind listening %d port for remote tunnels success",
			ntohs(tunnels_listening.listen.sin_port));
			
	return TRUE;
}

bool begin_work_with_ltunnels(void)
{
	if (tunnels_listening.idtun != -1)
		return FALSE;
	if (!accept_socket(tunnels_listening.socket, -1))
	{
		return FALSE;
	}

	return TRUE;
}

bool begin_work_with_tunnel(int idtun)
{
	DESCRIPTOR_DATA_TUNNEL * d_tunnel;
	int i;
	insockets_t *lists;
	bool ret = TRUE;

	if ((d_tunnel = varr_get(&tunnel_list, idtun)) == NULL)
		return FALSE;
	
	for (i = 0; i < d_tunnel->listening_sockets.nused; i++)
	{
		lists = ((insockets_t *)
				VARR_GET((&(d_tunnel->listening_sockets)), i));
		if (!accept_socket(lists->socket, idtun))
			ret = FALSE;
	}
	log_printf("Begin work with tunnel '%d' started", idtun);
	return ret;
}

/*
int check_assoc_port(int port)
{
	DESCRIPTOR_DATA_TUNNEL *dm;
	int k, i;
	insockets_t *asport;

	//for (dm = tunnel_list; dm; dm = dm->next)
	for (k = 0; k < tunnel_list.nused; k++) {
	{
		dm = VARR_GET(&tunnel_list, k);
		for (i = 0; i < dm->listening_sockets.nused; i++)
		{
			asport = ((insockets_t *) VARR_GET((&(dm->listening_sockets)), i));
			if (asport->port == port)
				return dm->idtun;
		}
	}

	return -1;
}
*/


bool shutdown_tunnel(int idtun)
{
	DESCRIPTOR_DATA_TUNNEL	* d_tunnel = varr_get(&tunnel_list, idtun);
	DESCRIPTOR_DATA_CLIENT	* client;
	insockets_t		* lists;
	int			i;

	if (!d_tunnel || d_tunnel->idtun == -1)
		return FALSE;

	log_printf("shutdown tunnel: %d", d_tunnel->idtun);

	/*
	 *	close listening sockets
	 */
	for (i = 0; i < d_tunnel->listening_sockets.nused; i++)
	{
		lists = ((insockets_t *)
				VARR_GET((&(d_tunnel->listening_sockets)), i));
		if (lists->socket < 0)
			continue;
#if !defined( WIN32 )
		close(lists->socket);
#else
		closesocket(lists->socket);
#endif
		lists->socket = -1;
	}
	
	if (d_tunnel->from_connect)
	{
		/*
		 *	close remote connection
		 */
		destroy_descriptor_main(d_tunnel->from_connect);
		d_tunnel->from_connect = NULL;
	}
	
	/*
	 *	close all clients connects
	 */
	for (client = d_tunnel->clients; client; client = client->next)
	{
		if (!client->connect)
			continue;
		close_descriptor_client(client, NULL);
		REMOVE_BIT(client->flags, DDCF_CAN_OUT);
	}

	SET_BIT(d_tunnel->flags, DDTF_RAW_REQDESTROY);
	REMOVE_BIT(d_tunnel->flags, DDTF_CAN_OUT);

	return FALSE;
}

bool destroy_tunnel(int idtun)
{
	DESCRIPTOR_DATA_TUNNEL	* d_tunnel = varr_get(&tunnel_list, idtun);
	DESCRIPTOR_DATA_CLIENT	* client, * client_next;

	if (!d_tunnel || d_tunnel->idtun == -1)
		return FALSE;
	/*
	 *	destroy all clients connects
	 */
	for (client = d_tunnel->clients; client; client = client_next)
	{
		client_next = client->next;
	
		if (!client->connect)
			continue;
		destroy_descriptor_client(client);
	}

	free_tunnel(idtun);
	return TRUE;
}

/************************************************************************
 *
 *			Main networking LOOP function
 *
 ************************************************************************/

extern FILE * rfin;		/* from resolver.c */
/*
 *	return FALSE if network broken
 *		example, interface at shutdown
 */
bool process_in_all_tunnels(void)
{
	fd_set in_set;
	fd_set out_set;
	fd_set exc_set;
	DESCRIPTOR_DATA_TUNNEL	*dt;
	insockets_t		*lsock;
	DESCRIPTOR_DATA_CLIENT	*client;
	int maxdesc = 0;
	int itun, ils;
	int fd;
	int fd_rfin = rfin ? fileno(rfin) : -1;	// for resolver
	static struct timeval null_time_loop = {0, 0};
	
	FD_ZERO(&in_set	);
	FD_ZERO(&out_set);
	FD_ZERO(&exc_set);

	/*
	 * Poll all active descriptors.
	 */
	for (itun = 0; itun < tunnel_list.nused; itun++)
	{
		dt = VARR_GET(&tunnel_list, itun);

		if (dt->idtun == -1)
			continue;

		/* poll remote tunnels connection */
		if (dt->from_connect)
		{
			fd = dt->from_connect->socket;
				
			FD_SET(fd, &in_set);
			FD_SET(fd, &out_set);
			FD_SET(fd, &exc_set);
			maxdesc = UMAX(maxdesc, fd);
			continue;
		}

		for (ils = 0; ils < dt->listening_sockets.nused; ils++)
		{
			lsock = ((insockets_t *)
				VARR_GET((&(dt->listening_sockets)), ils));
			FD_SET(lsock->socket, &in_set);
			if (maxdesc < lsock->socket)
				maxdesc = lsock->socket;
		}
		for (client = dt->clients; client; client = client->next)
		{
			if (client->connect)
			{
				fd = client->connect->socket;
				
				FD_SET(fd, &in_set);
				FD_SET(fd, &out_set);
				FD_SET(fd, &exc_set);
				maxdesc = UMAX(maxdesc, fd);
			} else
				log_printf("***BUG:: check_conn_alltun: client"
						" %d not connect",
						client->cid);
		}
	}

	/* check new tunnels here */
	if (tunnels_listening.socket != -1)
	{
		FD_SET(tunnels_listening.socket, &in_set);
		maxdesc = UMAX(maxdesc, tunnels_listening.socket);
	}

	if (fd_rfin >= 0)
	{
		/* Poll resolver */
		FD_SET(fd_rfin, &in_set);
		maxdesc = UMAX(maxdesc, fd_rfin);
	}
	
	if (select(maxdesc+1,
			&in_set, &out_set, &exc_set, &null_time_loop) < 0)
	{
		log_printf("check_conn_alltun: select: %s", strerror(errno));
		log_printf("check_conn_alltun: null_time: tv_sec=%d tv_usec=%d",
				null_time_loop.tv_sec, null_time_loop.tv_usec);
		exit(1);
	}

	if (fd_rfin >= 0 && FD_ISSET(fd_rfin, &in_set))
		resolv_check();

	/* Check new connections and kick out with any errors,
	 * 	example: simple disconnecting
	 */
	for (itun = 0; itun < tunnel_list.nused; itun++)
	{
		dt = VARR_GET(&tunnel_list, itun);

		if (dt->idtun == -1)
			continue;

		/* check remote tunnels here */
		if (dt->from_connect)
		{
			/* check broken connections */
			fd = dt->from_connect->socket;
				
			if (FD_ISSET(fd, &exc_set))
			{
				FD_CLR(fd, &in_set );
				FD_CLR(fd, &out_set);
				dt->from_connect->outtop = 0;
				shutdown_tunnel(itun);
				continue;
			}
			/* check out stream */
			if (FD_ISSET(fd, &out_set))
			{
				SET_BIT(dt->flags, DDTF_CAN_OUT);
			} else {
				REMOVE_BIT(dt->flags, DDTF_CAN_OUT);
			}
			continue;
		}

		/* check new connections */
		for (ils = 0; ils < dt->listening_sockets.nused; ils++)
		{
			lsock = ((insockets_t *)
				VARR_GET((&(dt->listening_sockets)), ils));
			if (FD_ISSET(lsock->socket, &in_set))
			{
				DESCRIPTOR_DATA_CLIENT * newclient;
				if ((newclient = init_local_descriptor_client(lsock))
								== NULL)
				{
					continue;
				}
				SET_BIT(newclient->flags, DDCF_NEWCONNECT);
			}
		}

		/* check broken connections or some errors */
		for (client = dt->clients; client; client = client->next)
		{
			if (client->connect)
			{
				fd = client->connect->socket;
				
				if (FD_ISSET(fd, &exc_set))
				{
					FD_CLR(fd, &in_set );
	                                FD_CLR(fd, &out_set);
					if (client->connect)
						client->connect->outtop = 0;
					close_descriptor_client(client, NULL);
					REMOVE_BIT(client->flags, DDCF_CAN_OUT);
				} else {
					/* check out stream */
					if (FD_ISSET(fd, &out_set))
					{
						SET_BIT(client->flags, DDCF_CAN_OUT);
					} else {
						REMOVE_BIT(client->flags, DDCF_CAN_OUT);
					}
				}
			} else
				log_printf("***BUG:: check_conn_alltun: client"
						" %d not connect",
						client->cid);
		}
	}

	/*
	 *	Process input.
	 */
	for (itun = 0; itun < tunnel_list.nused; itun++)
	{
		dt = VARR_GET(&tunnel_list, itun);

		if (dt->idtun == -1)
			continue;

		if (dt->from_connect)
		{
			fd = dt->from_connect->socket;

			if (!FD_ISSET(fd, &in_set))
				continue;
			if (read_from_descriptor_main(dt->from_connect) < 0)
			{
				FD_CLR(fd, &out_set);
				REMOVE_BIT(dt->flags, DDTF_CAN_OUT);
				dt->from_connect->outtop = 0;
				shutdown_tunnel(itun);
				continue;
			}
			/*
			 *	Fill in-data remote clients here
			 */
			// ....
				// not clientdata read from real socket (all data from tunnel)
			continue;
		}

			// after check dt->from_connect !!! (check client local connections)
		for (client = dt->clients; client; client = client->next)
		{
			if (!client->connect)
				continue;
			fd = client->connect->socket;

			//if (!FD_ISSET(fd, &in_set))
			//	continue;
			if ((FD_ISSET(fd, &in_set) &&
				read_from_descriptor_main(client->connect) < 0
			)
			|| !read_from_descriptor_client(client))
			{
				FD_CLR(fd, &out_set);
				REMOVE_BIT(client->flags, DDCF_CAN_OUT);
				if (client->connect)
					client->connect->outtop = 0;
				close_descriptor_client(client, NULL);
				continue;
			}
		}
	}

	/* check new connection tunnels here */
	if (tunnels_listening.socket != -1
	&& FD_ISSET(tunnels_listening.socket, &in_set))
	{
		init_remote_clients_tunnel(&tunnels_listening);
	}

	return TRUE;
}

/*
 *	return FALSE if network broken
 *		example, interface at shutdown
 */
bool process_out_all_tunnels(void)
{
	int	itun;
	DESCRIPTOR_DATA_TUNNEL	*dt;
	DESCRIPTOR_DATA_CLIENT	*client;
	
	/* out tunnels here !!!! */
	//...
	/*
	 *	Process output.
	 */
	for (itun = 0; itun < tunnel_list.nused; itun++)
	{
		dt = VARR_GET(&tunnel_list, itun);

		if (dt->idtun == -1)
			continue;

		if (dt->from_connect)
		{
			if (!IS_SET(dt->flags, DDTF_CAN_OUT))
				continue;
			if (!process_output_main(dt->from_connect))
			{
				dt->from_connect->outtop = 0;
				shutdown_tunnel(itun);
				continue;
			}
			continue;
		}

		for (client = dt->clients; client; client = client->next)
		{
			if (!client->connect
			|| !IS_SET(client->flags, DDCF_CAN_OUT))
				continue;

			if (!process_output_main(client->connect))
			{
				if (client->connect)
					client->connect->outtop = 0;
				close_descriptor_client(client, NULL);
				continue;
			}
		}
	}
	return TRUE;
}

void tunnel_add_to_trusted_addr(int idtun, struct in_addr *in_addr)
{
	int i;
	insockets_t *lsock;
	DESCRIPTOR_DATA_TUNNEL  * dt = varr_get(&tunnel_list, idtun);
	
	if (!dt)
		return;
		
	for (i = 0; i < dt->listening_sockets.nused; i++)
	{
		lsock = ((insockets_t *)
			VARR_GET((&(dt->listening_sockets)), i));
		add_to_trusted_addr(lsock, in_addr);
	}
}

/*
 *****************************************************************************
 *	create connected tunnel
 *		target - target host.
 *		fromport_totarget - number of port which bind for
 *				create connect to target. If 0 or (-1) then
 *				port choose by system (OS).
 */

/*DESCRIPTOR_DATA_MAIN */int connect_to_muddy(struct sockaddr_in * fromhost,
		struct sockaddr_in *target)
{
	DESCRIPTOR_DATA_MAIN	* t_connect = NULL;
	int ct_socket;	// connecting to muddy socket (tunnel socket)
	const char * msg = "connect_to_muddy";
	int id = -1, i;

	if ((ct_socket = init_socket(-1,
			ntohs(fromhost->sin_port) > 0 ? fromhost : NULL,
			TRUE, TRUE)) < 0)
	{
		log_printf("%s: fail create socket", msg);
		return -1;
	}

	/* connect to target host (create tunnel) */
	log_printf("%s: attempt connect to host: %s %d", msg,
		inet_ntoa(target->sin_addr), ntohs(target->sin_port));
		
	if (connect(ct_socket, (struct sockaddr*) target,
			sizeof(struct sockaddr_in)) < 0)
	{
		log_printf("%s: failed connect to %s %d (%d): %s", msg,
			inet_ntoa(target->sin_addr), ntohs(target->sin_port),
			target->sin_family, strerror(errno));
		close(ct_socket);
		return -1;
	}

	for (i = 0; i < connect_list.nused; i++)
	{
		t_connect = VARR_GET(&connect_list, i);
		if (t_connect->socket == -1)
		{
			/* has finded free slot */
			memset(t_connect, 0, sizeof(*t_connect));
			id = i;
			break;
		}
	}

	if (i >= connect_list.nused)
	{
		id = connect_list.nused;
		t_connect = varr_enew(&connect_list);
	}

	t_connect->socket	= ct_socket;
	t_connect->outsize	= 8000;		/* to ... */
	t_connect->outbuf	= malloc(t_connect->outsize);
	t_connect->insize	= 8000;		/* in ... */
	t_connect->inbuf	= malloc(t_connect->insize);

//	t_connect->hostname	= str_dup(inet_ntoa(target->sin_addr));
//      May be the following line will be critical sometime
//	memcmp(&t_connect->addr, target, sizeof(t_connect->addr));

	return id/*t_connect*/;
}

void fill_clhead(unsigned char * cluster,
		short int sflags, short int cid, int hid)
{
	cluster[0]	= CURR_TUNNEL_HEADER_LENGTH;
	cluster[1]	= CURR_TUNNEL_HEADER_VERS;
	*((short int *) cluster	+ 2)	= sflags;
	*((short int *) cluster	+ 4)	= cid;
	*((int *) cluster	+ 6)	= hid;
	*((short int *) cluster	+ 10)	= 0;
}

void * create_cluster(void * buffer,
		short int sflags,
		short int cid, int hid,
		void * data, short int data_length)
{
	unsigned char * cluster;

	if (buffer)
		cluster = buffer;
	else {
		cluster = malloc(1 + CURR_TUNNEL_HEADER_LENGTH + data_length);
	}

	return cluster;
}

//тут (теперь обработка около select())
/*
bool process_tunneling_in(void)
{
	DESCRIPTOR_DATA_CMUDDY	* dtm, * dtm_next;

	for (dtm = descriptor_list_cmuddy; dtm; dtm = dtm_next)
	{
		dtm_next	= dtm->next;

		if (IS_SET(dtm->flags, DDCF_REQ_REMOTE_INIT))
		{
		}

		if (dtm->client->inarray)
		{
		}
	}

	return TRUE;
}

bool process_tunneling_out(void)
{
	DESCRIPTOR_DATA_CMUDDY	* dtm, * dtm_next;

	for (dtm = descriptor_list_cmuddy; dtm; dtm = dtm_next)
	{
		dtm_next	= dtm->next;

	}

	return TRUE;
}
*/

