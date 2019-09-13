/*
 * mnet.c			<MGater libs>
 * 	LIB for OS depend network working
 */

#include <sys/types.h>
#if !defined(WIN32)
//#define __USE_XOPEN_EXTENDED
//#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
//#include <arpa/telnet.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
#endif

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

//#include "clients.h"
//#include "tunnels.h"
#include "libmnet.h"
#include "libmuddy.h"

//#include "str.h"
//#include "mgater.h"

typedef struct hidcache_t {
	uint		hid;
	struct in_addr	addr;
} hidcache_t;
varr hid_cache = { sizeof (hidcache_t), 10 };
FILE * filedata_d;	// hid db file

/* Global varibales */
//DESCRIPTOR_DATA_CLIENT		* descriptor_list_cl;

const char * strid_dm(DESCRIPTOR_DATA_MAIN *d)
{
	static char	str[80];

	if (d)
		snprintf(str, sizeof(str), "%s(%d)",
			inet_ntoa(d->addr.sin_addr),
			d->socket);
	else
		strncpy(str, "(disc)", sizeof(str));
	return str;
}


/*
 * 	Expand buffer size if need
 * 	(check needing buffer size)
 *	return FALSE if buffer has not minimum free space
 */
bool check_in_buffer_size(DESCRIPTOR_DATA_MAIN *d)
{
#define MINSTEPSIZE		(2 + CURR_TUNNEL_HEADER_LENGTH + 10)

	if (d->insize < d->intop + MINSTEPSIZE)
	{
		if (d->insize < MAX_BUF_IN)
		{
			/* incraise insize */
			unsigned char * inbuf;
			uint size = UMIN(MAX_BUF_IN, 2 * d->outsize);

			inbuf = malloc(size);
			memcpy(inbuf, d->inbuf, d->intop);
			free(d->inbuf);
			d->inbuf = inbuf;
			d->insize = size;
			if (size == MAX_BUF_IN)
			{
				log_printf("%s - exced max buffer %d",
					strid_dm(d), MAX_BUF_IN);
				if (d->insize < d->intop + MINSTEPSIZE)
					return FALSE;
			}
		} else
			return FALSE;
			//return iStart - iOld;
	}
	return TRUE;
}
/*
 *	(old: client socket -> d->inarray)
 *	Get data from socket and put it on inbuf
 *	return	> 0	- if recive new data
 *		0	- no data
 *		< 0	- some errors (need close desc)
 *				? FD_SET(d->socket, &exc_set);
 */
int read_from_descriptor_main(DESCRIPTOR_DATA_MAIN *d)
{
	int iOld;
	int iStart;

	if (!d)
		return FALSE;

	iOld = iStart = d->intop;

	for (; ;) {
		int nRead;

		if (check_in_buffer_size(d) == FALSE)
			return iStart - iOld;
		
#if !defined (WIN32)
		nRead = read( d->socket, d->inbuf + iStart,
			d->insize - iStart );
#else
		nRead = recv( d->socket, d->inbuf + iStart,
			d->insize - iStart, 0 );
#endif
		if (nRead > 0) {
			iStart += nRead;
			d->intop += nRead;

			check_in_buffer_size(d);
			return iStart - iOld;
		}
		else if (nRead == 0) {
			log_printf("read_from_descriptor_main: "
				"EOF encountered on read %s.",
				strid_dm(d));
			return -1;
			//break;
		}
#if !defined (WIN32)
		else if (errno == EWOULDBLOCK
		|| errno ==  EINTR
		|| errno == EAGAIN )	// EWOULDBLOCK = EAGAIN - try again
		{
			return iStart - iOld;
		}
#else
		else if ( WSAGetLastError() == WSAEWOULDBLOCK)
		{
			return iStart - iOld;
			//break;
		}
#endif
		else {
			log_printf("read_from_descriptor_main: %s: %s",
					strid_dm(d), strerror(errno));
			return -2;
		}
	}

	return -3; // unsepction exit ! ! BUG

}


/*
 *	Append onto an output client/user buffer.
 *		return how much bytes not buffered
 *		if 0 - all ok
 */
int write_to_buffer_main(DESCRIPTOR_DATA_MAIN *d, const char *txt, uint length)
{
	/*
	 *	Find length in case caller didn't.
	 */
	if (length <= 0)
		length = strlen(txt);

	/*
	 *	Expand the buffer as needed.
	 */
	while (d->outtop + length >= d->outsize) {
		char *outbuf;

		if (d->outsize >= MAX_BUF_OUT) {
			log_printf("wr_to_buf_main: buffer overflow: %s",
					strid_dm(d));
			//close_descriptor_cl(d, NULL);
			return length;
		}
		outbuf = malloc(UMIN(MAX_BUF_OUT, 2 * d->outsize));
			// check mem here
		strncpy(outbuf, (char*)d->outbuf, d->outtop);
		free(d->outbuf);
		d->outbuf = (unsigned char *)outbuf;
		d->outsize *= 2;
	}

	/*
	 *	Copy
	 */
	if (length)
		memcpy(d->outbuf + d->outtop, txt, length);
	d->outtop += length;
	
	return 0;
}

/*
 *	Lowest level output function.
 *	Write a block of text to the file descriptor.
 *	If this gives errors on very long blocks (like 'ofind all'),
 *	try lowering the max block size.
 */
bool	write_to_descriptor_main_nz(int socket, const char *txt, uint length)
{
	uint iStart;
	uint nWrite;
	uint nBlock;

	if (!length)
		length = strlen(txt);

	// raw log
	/*
	fprintf(stderr, "\nrlomnz: ");
	*/
	for (iStart = 0; iStart < length; iStart += nWrite)
	{
		nBlock = UMIN(length - iStart, 4096);
#if !defined( WIN32 )
		if ((nWrite = write(socket, txt + iStart, nBlock)) < 0) {
#else
		if ((nWrite = send(socket, txt + iStart, nBlock, 0)) < 0) {
#endif
			log_printf("write_to_descriptor_main_nz: %s", strerror(errno));
			return FALSE;
		}
		/*
		{
			// raw log
			int i;
			for (i = 0; i < nWrite; i++)
			{
				fprintf(stderr, "(%d)", (unsigned char) *(txt + iStart + i));
			}
		}
		*/

	}
	// raw log
	/*
	fprintf(stderr, "\n");
	*/

	return TRUE;
}

bool write_to_descriptor_main(DESCRIPTOR_DATA_MAIN *d, const char *txt, uint length)
{
	return write_to_descriptor_main_nz(d->socket, txt, length);
}

int init_socket(int port, struct sockaddr_in * fulladdr, bool sf_REUSEADDR, bool sf_LINGER)
{
	static struct sockaddr_in sa_zero;
	//struct sockaddr_in sa;
	struct linger ld;
	int x = 1;
	int fd;

#if defined (WIN32)
	if ((fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
#else
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
#endif
		log_printf("init_socket(%d): socket: %s",
			   port, strerror(errno));
		return -1;
	}

	/*
	 *	attempt to reinit port
	 */
	if (sf_REUSEADDR)
	{
		if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,
			(char *) &x, sizeof(x)) < 0)
		{
			log_printf("init_socket(%d): setsockopt: SO_REUSEADDR: %s",
				   port, strerror(errno));
#if defined (WIN32)
			closesocket(fd);
#else
			close(fd);
#endif
			return -1;
		}
	}

	/* more raw connect */
	if (sf_LINGER)
	{
		ld.l_onoff  = 0;
		if (setsockopt(fd, SOL_SOCKET, SO_LINGER,
			(char *) &ld, sizeof(ld)) < 0)
		{
			log_printf("init_socket(%d): setsockopt: SO_LINGER: %s",
			   port, strerror(errno));
#if defined (WIN32)
			closesocket(fd);
#else
			close(fd);
#endif
			return -1;
		}
	}

	if (port > 0)
	{
		struct sockaddr_in sa;
		sa		= sa_zero;
#if !defined (WIN32)
		sa.sin_family   = AF_INET;
#else
		sa.sin_family   = PF_INET;
		sa.sin_addr.s_addr = INADDR_ANY;
#endif
		sa.sin_port	= htons(port);

		if (bind(fd, (struct sockaddr *) &sa, sizeof(sa)) < 0)
		{
			log_printf("init_socket(%d): bind: %s", port, strerror(errno));
#if defined (WIN32)
			closesocket(fd);
#else
			close(fd);
#endif
			return -1;
		}
	} else if (fulladdr)
	{
		if (bind(fd, (struct sockaddr *) fulladdr,
				sizeof(*fulladdr)) < 0)
		{
			log_printf("init_socket: bind: %s(%d): %s",
				inet_ntoa(fulladdr->sin_addr),
				ntohs(fulladdr->sin_port),
				strerror(errno));
#if defined (WIN32)
			closesocket(fd);
#else
			close(fd);
#endif
			return -1;
		}
	}

	return fd;
}

bool accept_socket(int fd, int idtun)
{
	if (listen(fd, 3) < 0) 
	{
		log_printf("init_socket(%d, %d - tun): listen: %s",
			   fd, idtun, strerror(errno));
#if defined (WIN32)
		closesocket(fd);
#else
		close(fd);
#endif
		return FALSE;
	}
	return TRUE;
}



#if !defined(FNDELAY)
#define FNDELAY O_NDELAY
#endif

	/* insock - listening socket */
DESCRIPTOR_DATA_MAIN * init_descriptor_main
		(insockets_t *insock/*, const char *greeting*/)
{
	DESCRIPTOR_DATA_MAIN	*dnew;
//	DESCRIPTOR_DATA_MAIN	*d;
	int sock;
	unsigned int size;
	int ret;
//	int id = 1;
	struct sockaddr_in addr;
//	HELP_DATA *greeting;

/*	size = sizeof(addr);
	if ((ret = getsockname(insock->socket,
		(struct sockaddr *) &addr, &size)) < 0
	|| size != sizeof(addr))
	{
		log_printf("init_desc_cl: getsockname%s: %s",
			size != sizeof(addr) ?
				"(size struct addr error)" : str_empty,
			ret < 0 ? strerror(errno) : str_empty);
		return;
	}
*/

	size = sizeof(addr);
	if ((sock = accept(insock->socket, (struct sockaddr *) &addr,
		&size)) < 0)
	{
		log_printf("init_descriptor_main: accept: %s", strerror(errno));
		return NULL;
	}

#if !defined (WIN32)
	if (fcntl(sock, F_SETFL, FNDELAY) < 0) {
		log_printf("init_descriptor_main: fcntl: FNDELAY: %s",
			   strerror(errno));
		return NULL;
	}
#endif

	/*
	 * Cons a new descriptor.
	 */
	dnew = new_descriptor_main();

	dnew->socket		= sock;

//	dnew->flags		= DDMF_NONE;	// 0
//	dnew->connected		= CON_GET_CODEPAGE;
//	dnew->showstr_head	= NULL;
//	dnew->showstr_point	= NULL;
//	dnew->pString		= NULL;
//	dnew->olced		= NULL;
//	dnew->pEdit		= NULL;
//	dnew->pEdit2		= NULL;

	dnew->outsize		= 2000;		/* to ... */
	dnew->outbuf		= malloc(dnew->outsize);
	dnew->insize		= 2000;
	dnew->inbuf		= malloc(dnew->insize);

//	if (dnew->outbuf == NULL)
//		crush_mud();
//	dnew->wait_for_se	= 0;
//	dnew->codepage		= codepages;
//	dnew->hostname		= NULL;

	size = sizeof(dnew->addr);
	if ((ret = getpeername(sock, (struct sockaddr *) &(dnew->addr), &size))
			< 0
	|| size != sizeof(dnew->addr))
	{
		log_printf("init_descriptor_main%s: getpeername: %s",
			size != sizeof(addr) ?
				"(size struct addr error)" : str_empty,
			ret < 0 ? strerror(errno) : str_empty);
		destroy_descriptor_main(dnew);
		return NULL;
	}
#if defined (WIN32)
	else {
		/* Copying from ROM 2.4b6 */
		int addr;
		struct hostent *from;

		addr = ntohl(dnew->addr.sin_addr.s_addr);
		from = gethostbyaddr((char *) &(dnew->addr).sin_addr,
				     sizeof(dnew->addr.sin_addr), AF_INET);
		dnew->hostname = str_dup(from ? from->h_name : "unknown");
	}
#endif
	/*
	 *	check trusted_addr
	 */
	if (insock->trusted_addr.nused)
	{
		struct in_addr * c_in_addr;
		int i;

		for(i = 0; i < insock->trusted_addr.nused; i++)
		{
			c_in_addr = VARR_GET(&(insock->trusted_addr), i);
			if (!memcmp(c_in_addr, &(dnew->addr.sin_addr),
					sizeof(dnew->addr.sin_addr)))
				break;	// find allowed IP address
		}
		
		if (i == insock->trusted_addr.nused)
		{
			/* IP adress not allowed */
			log_printf("init_descriptor_main: access denied to tun:%d port:%d: %s",
				insock->idtun, ntohs(insock->listen.sin_port),
				inet_ntoa(dnew->addr.sin_addr));
			destroy_descriptor_main(dnew);
			return NULL;
		}
	}

	return dnew;
}

/*
 *	(not remove from tunnel->clients list !) <- old comment
 */
void destroy_descriptor_main(DESCRIPTOR_DATA_MAIN *dclose)
{
//	if (dclose->outtop > 0)

	process_output_main(dclose);

/*
	if (dclose->snoop_by != NULL) 
		write_to_buffer(dclose->snoop_by,
				"Your victim has left the game.\n\r", 0);

	for (d = descriptor_list; d != NULL; d = d->next)
		if (d->snoop_by == dclose)
			d->snoop_by = NULL;


	if ((ch = dclose->character) != NULL) {
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

#if !defined( WIN32 )
	close(dclose->socket);
#else
	closesocket(dclose->socket);
#endif
	free_descriptor_main(dclose);
}

#if 0
void close_descriptor_cl(DESCRIPTOR_DATA_USER *dclose,
		DESCRIPTOR_DATA_USER **d_next)
{
	/* send to mud FP_CLOSECONNECT */
	/*
	if (dclose->tunnel
	&& !write_to_tunnel(dclose->tunnel, dclose, FP_CLOSECONNECT))
	{
		log_printf("close_descriptor_cl: can't send to '%s' FP_CLOSECONNECT",
				inet_ntoa(dclose->tunnel->addr.sin_addr));
	}
	*/

	if (d_next)
	{
		if (*d_next == dclose)
			*d_next = (*d_next)->next;
	}

	/*
	if (dclose == dclose->tunnel->clients)
		dclose->tunnel->clients = dclose->tunnel->clients->next;
	else {
		DESCRIPTOR_DATA_USER *d;

		for (d = dclose->tunnel->clients; d && d->next != dclose;
				d = d->next)
			;
		if (d != NULL)
			d->next = dclose->next;
		else
			log_printf("***BUG: Close_socket: dclose not found.");
	}
	log_printf("close client descriptor: %s -> %s",
			inet_ntoa(dclose->addr.sin_addr),
			dclose->tunnel ? inet_ntoa(dclose->tunnel->addr.sin_addr)
					: "?");
	*/

	destroy_descriptor_cl(dclose);
}
#endif

/* stuff for recycling descriptors */
DESCRIPTOR_DATA_MAIN *descriptor_free_main;

DESCRIPTOR_DATA_MAIN *new_descriptor_main(void)
{
	DESCRIPTOR_DATA_MAIN *d;

	if (descriptor_free_main == NULL) {
		d = malloc(sizeof(*d));
		if (d == NULL)
		{
			log_printf("nomem");
			return NULL;
			//mgater_done(ERRMG_NOMEM, str_empty);
		}
	} else {
		d = descriptor_free_main;
		descriptor_free_main = descriptor_free_main->next;
	}

	memset(d, 0, sizeof(*d));
	return d;
}

void free_descriptor_main(DESCRIPTOR_DATA_MAIN *d)
{
	if (!d)
		return;

//	if (d->hostname)
//		free_string(d->hostname);
	free(d->outbuf);
	free(d->inbuf);

	d->next = descriptor_free_main;
	descriptor_free_main = d;
}

bool process_output_main(DESCRIPTOR_DATA_MAIN *d)
{
	/*
	 *	Short-circuit if nothing to write.
	 */

	if (d->outtop == 0)
		return TRUE;

	/*
	 *	OS-dependent output.
	 */
	if (!write_to_descriptor_main(d, (char*)d->outbuf, d->outtop))
	{
		d->outtop = 0;
		return FALSE;
	}

//	if (IS_SET(d->flags, DDMF_TELNET_GA))
//		write_to_descriptor_main(d, go_ahead_str, 0);

	d->outtop = 0;
	return TRUE;
}

bool add_to_trusted_addr_str(insockets_t *lsock, const char * saddr)
{
	struct hostent *h = gethostbyname(saddr);

	if (!h)
	{
		log_printf("add_to_trusted_addr_str error: %s: %s",
				saddr, hstrerror(h_errno));
		return FALSE;
	} else {
		for (; *h->h_addr_list; h->h_addr_list++)
		{
			add_to_trusted_addr(lsock, (struct in_addr *) *h->h_addr_list);
		}
	}
	return TRUE;
}

void add_to_trusted_addr(insockets_t *lsock, struct in_addr *in_addr)
{
	struct in_addr * new_in_addr = varr_enew(&(lsock->trusted_addr));

	log_printf("'%s' added to trusted list of tun:%d port:%d",
			inet_ntoa(*in_addr),
			lsock->idtun, ntohs(lsock->listen.sin_port));
	memcpy(new_in_addr, in_addr, sizeof(*new_in_addr));
}

/************************************************************************
 * 
 *	Work with hid (client ip/host <-> id)
 * 
 ************************************************************************/

void addhidcache(uint hid, struct in_addr addr)
{
	hidcache_t * hidc;
//	struct in_addr *aac;

	hidc = varr_enew(&hid_cache);
	hidc->hid = hid;
	hidc->addr = addr;
	//memcpy(&(hidc->addr), &addr, sizeof(addr));
}

bool fillallcache(void)
{
	uint	k;
//	uint	hid = 1;
	const char * s;
	struct hostent *h;

//	if (!fseek(filedata_d, 1L, SEEK_SET))
//	{
//		log_printf("fillallcache: %s", strerror(errno));
//		return FALSE;
//	}
	rewind(filedata_d);

	for (;;) {
		if (feof(filedata_d))
		{
			return TRUE;
		}

		s = fread_word(filedata_d);
		k = fread_number(filedata_d);
		fread_to_eol(filedata_d);

		h = gethostbyname(s);

		if (!h)
		{
			log_printf("fillallcache: %s: gethostbyname: %s",
					s, hstrerror(h_errno));
			return FALSE;
		}

				// check repeating addr
		if (!gethid(*((struct in_addr *) h->h_addr), FALSE))
			addhidcache(k, *((struct in_addr *) (h->h_addr)));
	}
	return TRUE;
}

bool inithid(const char *dir, const char * filedata)
{
	char name[PATH_MAX];

	if ((filedata_d = dfopen(dir, filedata, "r+")) == NULL)
	{
		log_printf("%s: %s", name, strerror(errno));
		return FALSE;
	}

	if (!fillallcache())
	{
		return FALSE;
	}

	return TRUE;
}

/*
 *	save cache db in file
 *	require call at cleanup() !!!
 */ 
void donehid(void)
{
	uint	i;
	hidcache_t * hidc;
	//const char * s;
	//struct hostent *h;

//	if (!fseek(filedata_d, 0, SEEK_SET))
//	{
//		log_printf("donehid: %s", strerror(errno));
//	}

	if (!filedata_d)
		return;
	rewind(filedata_d);

	for (i = 0; i < hid_cache.nused; i++)
	{
		hidc = VARR_GET(&hid_cache, i);
		fprintf(filedata_d, "%s %d\n",
				inet_ntoa(hidc->addr), hidc->hid);
	}

	fclose(filedata_d);
}



// REQUIRE! optimized algoritm !!!
uint gethid(struct in_addr addr, bool addnew)
{
	uint	i;
	hidcache_t * hidc;
	uint	hid = 1;
	
	/*
	 *	find in cache
	 */
	for (i = 0; i < hid_cache.nused; i++)
	{
		hidc = VARR_GET(&hid_cache, i);
		if (!memcmp(&(hidc->addr), &addr, sizeof(addr)))
		{
			return hidc->hid;
		}
		if (hidc->hid >= hid)
			hid++;
	}
	/*
	 *	find in file-data
	 */
	/*		all IP in already cache (req choose algoritm with work big ip db ip)
	 */
#if 0
	if (!fseek(filedata_d, 0, SEEK_SET))
	{
		log_printf("gethid: %s", strerror(errno));
		return 0;
	}
	for (;;) {
		if (feof(filedata_d))
		{
			break;
		}
		s = fread_word(filedata_d);
		k = fread_number(filedata_d);
		h = gethostbyname(s);

		if (k >= hid)
			hid++;

		if (!h)
		{
			log_printf("gethid: %s: gethostbyname: %s",
					s, hstrerror(h_errno));
			continue;
		}

		if (!memcmp(h->h_addr, &addr, sizeof(addr)))
		{
			addhidcache(k, addr);
			return k;
		}
	}
#endif

	/*
	 *	add to cache
	 */
	if (addnew)
	{
		addhidcache(hid, addr);
		return hid;
	}
	return 0;
}

/*
struct in_addr getaddr_hid(unsigned int hid)
{
	struct in_addr tmp_hid;

	memset((char *)&tmp_hid, 0, sizeof(tmp_hid));
	return tmp_hid;
}
*/

