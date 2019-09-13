/*
 *	mnet.h for libmnet.so
 *	Lib for make connect with clients aka players(users!)
 */

#ifndef _LIBMNET_H_
#define _LIBMNET_H_

#if !defined(WIN32)
#define __USE_XOPEN_EXTENDED
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/telnet.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#ifdef MCCP
#include <zlib.h>
#endif

//#include "libtypedef.h"
//#include "varr.h"
//#include "libconst.h"
#include "libmuddy.h"
#include "libconst.h"
#include "charset.h"
#include "resolver.h"

typedef struct descriptor_data_client	DESCRIPTOR_DATA_CLIENT;
typedef struct descriptor_data_tunnel	DESCRIPTOR_DATA_TUNNEL;
typedef struct descriptor_data_main	DESCRIPTOR_DATA_MAIN;

/* For listening sockets */
typedef struct insockets_t {
	int			socket;
//	int			port;
	struct sockaddr_in	listen;
	flag32_t		flags;
	varr			trusted_addr;	/* struct in_addr
						 * but it's temporaly
						 */
	int			idtun;
} insockets_t;

/*
 * Socket and TCP/IP stuff.
 */
#if defined (WIN32)
#include <winsock.h>

void    gettimeofday    args( ( struct timeval *tp, void *tzp ) );

/*  Definitions for the TELNET protocol. Copied from telnet.h */

#define IAC		255	/* interpret as command: */
#define DONT	254	/* you are not to use option */
#define DO		253	/* please, you use option */
#define WONT	252	/* I won't use option */
#define WILL	251	/* I will use option */
#define SB		250	/* interpret as subnegotiation */
#define GA		249	/* you may reverse the line */
#define EL		248	/* erase the current line */
#define EC		247	/* erase the current character */
#define AYT		246	/* are you there */
#define AO		245	/* abort output--but let prog finish */
#define IP		244	/* interrupt process--permanently */
#define BREAK	243	/* break */
#define DM		242	/* data mark--for connect. cleaning */
#define NOP		241	/* nop */
#define SE		240	/* end sub negotiation */
#define EOR		239 /* end of record (transparent mode) */
#define SYNCH	242	/* for telfunc calls */

#define TELOPT_ECHO	1	/* echo */
#endif



#define MAX_HEADER_LENGTH		255	/* for tunnel packet */
#define CURR_TUNNEL_HEADER_VERS		1
#define CURR_TUNNEL_HEADER_LENGTH	11

#define	DEFAULT_LISTTUN_PORT		4002
#define DEFAULT_LISTCLI_PORT		4003

#define MAX_TUNNEL_BUF_OUT      (MAX_STRING_LENGTH * 4) /* from mud to mgaterd */
#define MAX_TUNNEL_BUF_IN       (MAX_STRING_LENGTH * 4) /* from mgaterd to mud */

#define MAX_CLIENT_BUF_OUT	(MAX_STRING_LENGTH * 4)
#define MAX_CLIENT_BUF_IN	MAX_STRING_LENGTH

#define MAX_BUF_OUT		MAX_TUNNEL_BUF_OUT
#define MAX_BUF_IN		MAX_TUNNEL_BUF_IN


/* Descriptor Data Client Flags */
#define	DDCF_NONE		(0)
#define DDCF_NOTELNET		(A)	// ignore IAC
#define DDCF_NOIAC		(B)
//#define DDCF_TELNET_GA	(C)	// Initial \n\r if needed
#define DDCF_NALLOW_EMPT_LINE	(D)	// skip '^[\n\r]*$' (don't allow simple <ENTER>)
#define DDCF_WAIT_CTL_IAC	(E)
#define DDCF_WAIT_FOR_SE	(F)	// for parsing telnet protocol (IAC)
#define DDCF_NEWCONNECT		(G)	/* just connected client */
#define DDCF_NEEDCLOSE		(H)	/* client prepared for close connection */
#define DDCF_CAN_OUT		(I)	/* can send out data */
#define DDCF_CHECK_CR		(J)	/* check carridge return ('\r') */
#define DDCF_CHECK_LF		(K)	/* check LF ('\n') */
//#define DDCF_TUNNEL		()
//#define DDCF_HOLDOVER		()	// attempt hold connect if buffers overflow


/* IAC replacement if DDUF_NOIAC is set */
/* DDUF_NOIAC is useful to map 'Ñ' (IAC) to 'ñ' when using win1251 codepage */
#define IAC_REPL        223

/*
 * 	for understand:
 *		struct sockaddr_in  {
 *			short sin_family;		// address family
 *			u_short sin_port;		// port
 *			struct in_addr sin_addr;	// IP
 *			char sin_data[8];		// unused
 *		};
 *		// Internet address.  
 *		typedef uint32_t in_addr_t;
 *		struct in_addr
 *		{
 *			in_addr_t s_addr;
 *		};
 *	htonl,  htons,  ntohl, ntohs - convert values between host
 *		and network byte order (use man)
 */

struct descriptor_data_main
{
	DESCRIPTOR_DATA_MAIN	* next;
//	const char *		hostname;
	struct sockaddr_in	addr;
	uint			socket;
//	flag32_t		flags;		// DDMF_*

	/* Input - get from socket (old: from client to tunneli->mud) */
	unsigned char *		inbuf;
	uint			intop;
	uint			insize;
	/* Private */
	//...

	/* Output - put in socket (to client from mud->tunnel) */
	unsigned char *		outbuf;
	uint			outtop;
	uint			outsize;
};

/*
 *	Descriptor of user(s) connect
 */
struct descriptor_data_client
{
	int			cid;			// ID of connect (one stream, one client)
	int			hid;			// host ID (may be from remote daemon by IP)
					/*  local hid or hid recieved
					 *  from remote mgaterd (tunnel_hid)
					 */
				/*
				 * Often remote_id_client == user->id_client
				 * but if local daemon already have this id,
				 * user->id_client will have been changed on
				 * other uniq ID.
				 */
	const char *		hostname;
	/* Tunnel */
	//DESCRIPTOR_DATA_TUNNEL	* tunnel;
	int			idtun;	/* from that tunnel */


	DESCRIPTOR_DATA_CLIENT	* next;
//	DESCRIPTOR_DATA_USER *	snoop_by;
//	CHAR_DATA *		character;
//	CHAR_DATA *		original;
//	LINK_CLIENT_TUNNEL	* ltunnel;	// if user from some tunnel


	flag32_t		flags;	// DDCF_*
	DESCRIPTOR_DATA_MAIN	* connect;	/* buffers and other connect data */

	/* Input */
	unsigned char		inarray		[MAX_INPUT_LENGTH];
	uint			inlength;	// if (>0) - ready string (need get data from here and set zero this field)
	uint			infillcount;	// some reading string from d->connect->inbuf
	uint			needskip;	// skip some control bytes (after IAC...)

	/* wtf ? rtfm! */
	struct codepage*	codepage;

//	int			connected;	// connect status
	
	/* Output */
//	int 			repeat;
//	char *			outbuf;
//	uint 			outsize;	/* Max size of outbuf */
//	uint 			outtop;		/* Size elements(bytes) in outbuf */
#ifdef MCCP
	
	z_stream *		out_compress;	/* mccp: support data */
	unsigned char *		out_compress_buf;
#endif

	/* for Page buffer */
//	const char *		showstr_head;
//	const char *		showstr_point;

/* OLC stuff */
//	olced_t	*		olced;
//	void *             	pEdit;		/* edited obj	*/
//	void *             	pEdit2;		/* edited obj 2	*/

/* string editor stuff */
//	const char **		pString;	/* edited string	*/
//	const char *		backup;		/* backup		*/

};

#define DDTF_RAW_DATA		(A)		/* not control char with all clients */
#define DDTF_RAW_REQDESTROY	(B)		/* require call destroy_tunnel() */
#define DDTF_CAN_OUT		(C)

struct descriptor_data_tunnel {
	int			idtun;			/* ID of tunnel */
//	DESCRIPTOR_DATA_TUNNEL	* next;
	DESCRIPTOR_DATA_CLIENT	* clients;		/* connected clients */

//	struct sockaddr_in	addr;
//	varr			associated_ports;	/* array of int's */
	varr			listening_sockets;	/* array of insockets_t */
	flag32_t		flags;			/* DDTF_ */

	DESCRIPTOR_DATA_MAIN	* from_connect;	/* if NULL, then local mud !!!
						 * from that host connected
						 * this tunnel
						 */
	int		hid;

	int		counter_hoop;	/*  how much mgaterd's use this connect
					 *  0 - local (not from tunnel)
					 *  1 - one hoop mgaterd (standart)
					 *  more - difficult control user ;)
					 */
};

//#define DESC_INIT_FUN(fun)	

/* mnet.c */
DESCRIPTOR_DATA_MAIN *new_descriptor_main(void);
void destroy_descriptor_main(DESCRIPTOR_DATA_MAIN *dclose);
void free_descriptor_main(DESCRIPTOR_DATA_MAIN *d);
bool process_output_main(DESCRIPTOR_DATA_MAIN *d);
DESCRIPTOR_DATA_MAIN * init_descriptor_main (insockets_t *insock);
const char * strid_dm(DESCRIPTOR_DATA_MAIN *d);
int read_from_descriptor_main(DESCRIPTOR_DATA_MAIN *d);
int write_to_buffer_main(DESCRIPTOR_DATA_MAIN *d, const char *txt, uint length);
bool write_to_descriptor_main(DESCRIPTOR_DATA_MAIN *d, const char *txt,
		uint length);
int init_socket(int port, struct sockaddr_in * fulladdr, bool sf_REUSEADDR, bool sf_LINGER);
bool accept_socket(int fd, int idtun);
bool add_to_trusted_addr_str(insockets_t *lsock, const char * saddr);
void add_to_trusted_addr(insockets_t *lsock, struct in_addr *in_addr);

/* clients.c */
DESCRIPTOR_DATA_CLIENT	* init_local_descriptor_client(insockets_t *insock);
DESCRIPTOR_DATA_CLIENT	* new_descriptor_client(void);
void	free_descriptor_client(DESCRIPTOR_DATA_CLIENT *d);
void	destroy_descriptor_client(DESCRIPTOR_DATA_CLIENT *dclose);
const char		* strid_dc(DESCRIPTOR_DATA_CLIENT *d);
int	get_idsock_connect(DESCRIPTOR_DATA_CLIENT * client);
const char * get_hostname(DESCRIPTOR_DATA_CLIENT * client);
struct in_addr * get_ip_client(DESCRIPTOR_DATA_CLIENT * client);
bool	check_out_ready_client(DESCRIPTOR_DATA_CLIENT * client);
int hostcmp(DESCRIPTOR_DATA_CLIENT * c1, DESCRIPTOR_DATA_CLIENT * c2);
void	close_descriptor_client(DESCRIPTOR_DATA_CLIENT *dclose,
		DESCRIPTOR_DATA_CLIENT **d_next);
bool	read_from_descriptor_client(DESCRIPTOR_DATA_CLIENT *d);
bool raw_write_to_buffer_client(DESCRIPTOR_DATA_CLIENT *d, const char *txt,
			uint length);
bool write_to_buffer_client(DESCRIPTOR_DATA_CLIENT *d, const char *txt,
			uint length);
void echo_off_string(DESCRIPTOR_DATA_CLIENT * d);
void echo_on_string(DESCRIPTOR_DATA_CLIENT * d);
void send_ga_string(DESCRIPTOR_DATA_CLIENT * d);
bool inithid(const char *dir, const char * filedata);
uint gethid(struct in_addr addr, bool addnew);

/* tunnels.c */
extern varr tunnel_list;

void	add_simpleport(varr * addrs, int port);
int	init_local_clients_tunnel(varr *localports, const char * msg);
bool	begin_work_with_tunnel(int idtun);
bool	shutdown_tunnel(int idtun);
bool	process_in_all_tunnels(void);
bool	process_out_all_tunnels(void);
void	tunnel_add_to_trusted_addr(int idtun, struct in_addr *in_addr);
bool	init_tunnels_listening(int port, struct sockaddr_in * bind_addr);
bool	begin_work_with_ltunnels(void);
//int check_assoc_port(int port);

#endif

