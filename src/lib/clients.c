/*
 * clients.c			<MGater libs>
 * 	for working with simple clients (by telnet)
 */

#include "libmnet.h"

#include <errno.h>
#include <string.h>
#include <stdlib.h>

#ifdef MCCP
#include "libmccp.h"
#endif

const char	echo_off_str	[] = { IAC, WILL, TELOPT_ECHO, '\0' };
const char	echo_on_str	[] = { IAC, WONT, TELOPT_ECHO, '\0' };
const char	go_ahead_str	[] = { IAC, GA, '\0' };

const char * strid_dc(DESCRIPTOR_DATA_CLIENT *d)
{
	static char	str[80];

	snprintf(str, sizeof(str), "cid:%2d hid:%d t:%d",
			//get_hostname(d),
			//d->connect ? inet_ntoa(d->connect->addr.sin_addr)
			//	: "(disc)",
			get_idsock_connect(d),
			d->hid,
			d->idtun);
	return str;
}

/*
 *		insock - is 'listening_sockets' from associated tunnel
 */
DESCRIPTOR_DATA_CLIENT * init_local_descriptor_client(insockets_t *insock)
{
	DESCRIPTOR_DATA_CLIENT	* dnew;
	DESCRIPTOR_DATA_MAIN	* connect;
	DESCRIPTOR_DATA_TUNNEL	* tunnel;
//	int			hid = 1;
	static const char	binary_string[] =
	{
		(char) IAC,
		(char) DO,
		(char) TELOPT_BINARY,
		(char) 0
	};

	if ((connect = init_descriptor_main(insock)) == NULL)
	{
		return NULL;
	}

	dnew			= new_descriptor_client();
	dnew->connect		= connect;

	//if ((dnew->tunnel = check_assoc_port(insock->port)))
	if ((dnew->idtun = insock->idtun) > -1
	&& (tunnel = varr_get(&tunnel_list, insock->idtun)))
	{
		log_printf("sock.sinaddr: %s to %d",
				strid_dm(dnew->connect),
				insock->idtun);
	} else {
		log_printf("init_desc_cl: can't find tunnel %d for %s to %d port",
				insock->idtun,
				strid_dm(dnew->connect),
				ntohs(insock->listen.sin_port));
		destroy_descriptor_main(dnew->connect);
		dnew->connect = NULL;
		free_descriptor_client(dnew);
		return NULL;
	}

	dnew->hid		= gethid(dnew->connect->addr.sin_addr, TRUE);
	dnew->cid		= get_idsock_connect(dnew);
	dnew->hostname		= NULL;

//	dnew->tunnel		= NULL;
//	dnew->connected		= 0;
//	dnew->id_connect	= id;
//	dnew->remote_id_client	= 0;
//	dnew->showstr_head	= NULL;
//	dnew->showstr_point	= NULL;
//	dnew->pString		= NULL;
//	dnew->backup		= NULL;
	dnew->inlength		= 0;
	dnew->infillcount	= 0;
	dnew->needskip		= 0;
	dnew->codepage		= codepages;
	//dnew->flags		= DDCF_NALLOW_EMPT_LINE;

//	dnew->next		= descriptor_list_client;
//	descriptor_list_client	= dnew;

	dnew->next		= tunnel->clients;
	tunnel->clients		= dnew;

	
	if (IS_SET(tunnel->flags, DDTF_RAW_DATA))
	{
		dnew->flags	= DDCF_NOTELNET;
	} else {
		dnew->flags	= DDCF_NONE;	// 0
		/* set binary */
		raw_write_to_buffer_client( dnew, binary_string, 3);	// 3 chars !!!
#ifdef MCCP
		/* mccp: tell the client we support compression */
//		raw_write_to_buffer_client( dnew, compress_will, 0 );
#endif
	}
	
	/* send to mud FP_INITCONNECT */
	/*
	if (!write_to_tunnel(dnew->tunnel, dnew, FP_INITCONNECT))
	{
		close_descriptor_cl(dnew, NULL);
		return;
	}
	 */
	return dnew;
}

DESCRIPTOR_DATA_CLIENT * descriptor_free_client;

DESCRIPTOR_DATA_CLIENT * new_descriptor_client(void)
{
	DESCRIPTOR_DATA_CLIENT * d;

	if (descriptor_free_client == NULL) {
		d = malloc(sizeof(*d));
		if (d == NULL)
		{
			log_printf("new_descriptor_client: nomem");
			return NULL;
		}
	} else {
		d = descriptor_free_client;
		descriptor_free_client = descriptor_free_client->next;
	}
	memset(d, 0, sizeof(*d));
	return d;
}

void free_descriptor_client(DESCRIPTOR_DATA_CLIENT *d)
{
	if (!d)
		return;

	/* called from destroy_descriptor_main()
	if (d->connect)
		free_descriptor_main(d->connect);
	*/

#ifdef MCCP
	if (d->out_compress)
	{
		deflateEnd(d->out_compress);
		free(d->out_compress);
		d->out_compress = NULL;
	}
	if (d->out_compress_buf)
	{
		free(d->out_compress_buf);
		d->out_compress_buf = NULL;
	}
#endif
	if (d->hostname)
	{
		free_string(d->hostname);
		d->hostname = NULL;
	}

	d->next = descriptor_free_client;
	descriptor_free_client = d;
}

void destroy_descriptor_client(DESCRIPTOR_DATA_CLIENT *dclose)
{
	/* check close connect */
	if (dclose->connect)
	{
		log_printf("***BUG: destroy_descriptor_client: find not close connection: %s",
					strid_dc(dclose));
		dclose->connect->outtop = 0;
		destroy_descriptor_main(dclose->connect);
		dclose->connect = NULL;
	}

	//REMOVE_BIT(client->flags, DDCF_NEEDCLOSE);
	free_descriptor_client(dclose);
}

/*
 *	you must call destroy_descriptor_client(dclose) after this !!!
 *	elsethere memory lea(c)k!!
 */
void close_descriptor_client(DESCRIPTOR_DATA_CLIENT *dclose,
		DESCRIPTOR_DATA_CLIENT **d_next)
{
	DESCRIPTOR_DATA_TUNNEL * tunnel
		= varr_get(&tunnel_list, dclose->idtun);

	if (d_next && *d_next == dclose)
	{
		*d_next = (*d_next)->next;
	}
	
	/* Remove from tunnel->clients */
	if (dclose == tunnel->clients)
		tunnel->clients = tunnel->clients->next;
	else {
		DESCRIPTOR_DATA_CLIENT *d;

		for (d = tunnel->clients; d && d->next != dclose;
				d = d->next)
			;
		if (d)
			d->next = dclose->next;
		else
			log_printf("***BUG: close_d_cl: dclose not found: %s",
					strid_dc(dclose));
	}

#ifdef MCCP
	if (!compressEnd(dclose))
	{
		log_printf("close_descriptor_client: some error with compressEnd");
	}
#endif

	/* Close connect */
	if (dclose->connect)
	{
		destroy_descriptor_main(dclose->connect);
		dclose->connect = NULL;
	}

	SET_BIT(dclose->flags, DDCF_NEEDCLOSE);

	log_printf("close client descriptor: %s", strid_dc(dclose));

}

/*
 *	Transfer one line from input buffer(read_from_descriptor_main)
 *		to input line (d->inarray).
 *	return FALSE if error at connect
 */
bool read_from_descriptor_client(DESCRIPTOR_DATA_CLIENT *d)
{
	unsigned char *p, *ina;

	if (d->connect->intop == 0
	|| d->inlength)
		return TRUE;

	p = d->connect->inbuf;
	ina = d->inarray + d->infillcount;

		/* raw log */
	/*
	{
		int i;
		char str[2000];
		char str2[2000];
		snprintf(str, sizeof(str), "rl: ");
		for (i = 0; i < d->connect->intop; i++)
		{
			snprintf(str2, sizeof(str2), "%d ",
					d->connect->inbuf[i]);
			strncat(str, str2, sizeof(str));
		}
		strncat(str, "\n", sizeof(str));
		log_printf(str);
	}
	*/

			/* skip some control IAC */
	while (d->needskip)
	{
		skip_ctl_iac:
		--d->needskip;
		if (++p == d->connect->inbuf + d->connect->intop)
			goto finreadbuf;
	}

	if (IS_SET(d->flags, DDCF_WAIT_FOR_SE))
	{
		wait_ctl_se:		/* wait SE IAC */
		while (*p != SE)
		{
			if (++p == d->connect->inbuf + d->connect->intop)
				goto finreadbuf;
		}
		REMOVE_BIT(d->flags, DDCF_WAIT_FOR_SE);
		if (++p == d->connect->inbuf + d->connect->intop)
			goto finreadbuf;
	}

	if (IS_SET(d->flags, DDCF_WAIT_CTL_IAC))
	{
				/* wait control IAC */
		REMOVE_BIT(d->flags, DDCF_WAIT_CTL_IAC);
		wait_ctl_iac:

		switch (*p)
		{
		case DONT:
		case DO:
		case WONT:
		case WILL:
		//		q = p+3;
			d->needskip = 2;	// 3	(IAC + 2 control chars)
			break;
		case SB:
			SET_BIT(d->flags, DDCF_WAIT_FOR_SE);
			if (++p == d->connect->inbuf
					+ d->connect->intop)
				goto finreadbuf;
			goto wait_ctl_se;
		case IAC:
			goto fillnextc;
		default:
			d->needskip = 1;	// 2	(IAC + 1 control char)
			break;
		}
		goto skip_ctl_iac;
	}

	if (IS_SET(d->flags, (DDCF_CHECK_CR | DDCF_CHECK_LF)))
	{
		if (*p == '\r' && IS_SET(d->flags, DDCF_CHECK_CR))
		{
			REMOVE_BIT(d->flags, (DDCF_CHECK_CR | DDCF_CHECK_LF));
			if (++p == d->connect->inbuf
					+ d->connect->intop)
				goto finreadbuf;
		} else if (*p == '\n' && IS_SET(d->flags, DDCF_CHECK_LF))
		{
			REMOVE_BIT(d->flags, (DDCF_CHECK_CR | DDCF_CHECK_LF));
			if (++p == d->connect->inbuf
					+ d->connect->intop)
				goto finreadbuf;
		}
		REMOVE_BIT(d->flags, (DDCF_CHECK_CR | DDCF_CHECK_LF));
	}

	if (d->infillcount == 0)
	{
		if (IS_SET(d->flags, DDCF_NALLOW_EMPT_LINE))
		{
			/* skip simple <ENTER> */
			while (*p == '\n' || *p == '\r')
			{
				if (++p == d->connect->inbuf + d->connect->intop)
					goto finreadbuf;
			}
		}
	}

	for (;
		/* can't find end of string
		 * wait new data from read_from_descriptor_main()
		 */
		p != d->connect->inbuf + d->connect->intop;
	)
	{
		if (*p != IAC || IS_SET(d->flags, DDCF_NOTELNET))
		{
			if (*p == '\n' || *p == '\r')
			{
				uint skipc;
				unsigned char prevchar;

					/* simple <ENTER> */
				if (d->infillcount == 0)
				{
					*(ina++) = ' ';	// string is {' ', '\0'}
					d->infillcount++;
				}
				*ina = '\0';	// remark: // need check " "
				if (++d->infillcount == sizeof(d->inarray))
				{
					/* input len over */
					write_to_descriptor_main(d->connect, "Line too long - skip some characters...\n\r", 0);
				}
				d->inlength = d->infillcount - 1; // (-'\0')
				d->infillcount = 0;
				
				/* skip finalize '\n' and '\r' */
				prevchar = *p;
				if (++p == d->connect->inbuf
						+ d->connect->intop)
				{
					if (prevchar == '\n') {
						SET_BIT(d->flags, 
							DDCF_CHECK_CR);
					} else {	// prevchar == '\r'
						SET_BIT(d->flags,
							DDCF_CHECK_LF);
					}
					goto finreadbuf;
				}

				if ((*p == '\r' && prevchar == '\n')
				||  (*p == '\n' && prevchar == '\r'))
				{
					if (++p == d->connect->inbuf
							+ d->connect->intop)
						goto finreadbuf;
				}

				skipc = p - d->connect->inbuf;
				d->connect->intop -= skipc;
				memmove(d->connect->inbuf,
					d->connect->inbuf + skipc,
					d->connect->intop);
				return TRUE;
			}

			fillnextc:

			if (d->infillcount <= sizeof(d->inarray) - 1)
			{
				*(ina++) = d->codepage->from[(unsigned char) *p];
				d->infillcount++;
			}	// else skip char (inarray overflow)
			p++;
			continue;
		} else {
			if (++p == d->connect->inbuf + d->connect->intop)
			{
				SET_BIT(d->flags, DDCF_WAIT_CTL_IAC);
				goto finreadbuf;
			}

			goto wait_ctl_iac;
		}
	}

	finreadbuf:

	d->connect->intop = 0;
	return TRUE;
}

/*
 *	check tunneling here may be ?!
 */
bool raw_write_to_buffer_client(DESCRIPTOR_DATA_CLIENT *d, const char *txt, uint length)
{
	if (d->connect == NULL)
		return FALSE;

	if (length <= 0)
		length = strlen(txt);
#ifdef MCCP
	if (d->out_compress)
		return writeCompressed(d, txt, length);
	else
#endif
		return (write_to_buffer_main(d->connect, txt, length) ? FALSE : TRUE);
}

/*
 *	Append onto an output client/user buffer.
 *	Not thread!
 */
bool write_to_buffer_client(DESCRIPTOR_DATA_CLIENT *d, const char *txt, uint length)
{
	char	outbuffer[MAX_CLIENT_BUF_OUT];
	bool	noiac = IS_SET(d->flags, DDCF_NOIAC);
	uint	outtop;
	unsigned char c;

	/*
	 *	Find length in case caller didn't.
	 */
	if (length <= 0)
		length = strlen(txt);

	outtop = 0;
	
#if 0
	/*
	 *	Initial \n\r if needed.
	 */
	if (d->connect->outtop == 0 /* &&  !d->fcommand */
	/*&&  (!d->character || !IS_SET(d->character->comm, COMM_TELNET_GA))*/
	&& !IS_SET(d->flags, DDCF_TELNET_GA)) {
		outtop = 0;
		//outbuffer[0]	= '\n';
		//outbuffer[1]	= '\r';
		//outtop		= 2;
		//strncpy(outbuffer + outtop, go_ahead_str,
		//		sizeof(outbuffer) - outtop);
		//outtop		+= sizeof(go_ahead_str);
	} else
		outtop = 0;
#endif

	/*
	 *	Copy
	 */
	while (length--) {
		c = d->codepage->to[(unsigned char) *txt++];
		outbuffer[outtop] = c;
		if (c == IAC) {
			if (noiac)
				outbuffer[outtop] = IAC_REPL;
			else
				outbuffer[++outtop] = IAC;
		}
		if (++outtop >= sizeof(outbuffer) - 2)
		{
			log_printf("write_to_b_cl: big str: %s",
					strid_dc(d));
			return FALSE;
		}
	}
	outbuffer[outtop] = '\0';

	/*
	 *	Snoop-o-rama.
	 */
	/*
	if (d->snoop_by) {
		if (d->character)
			write_to_buffer(d->snoop_by, d->character->name, 0);
		write_to_buffer(d->snoop_by, "> ", 2);
		write_to_buffer(d->snoop_by, tmp_txt, 0);
	}
	*/

	return (raw_write_to_buffer_client(d, outbuffer, outtop));
}

int get_idsock_connect(DESCRIPTOR_DATA_CLIENT * client)
{
	if (client == NULL || client->connect == NULL)
		return -1;
	return client->connect->socket;
}

const char * get_hostname(DESCRIPTOR_DATA_CLIENT * client)
{
	struct in_addr * ip_addr;

	if (!client->connect)
		return "(disc)";

	return (client->hostname ? client->hostname :
		((ip_addr = get_ip_client(client)) ? inet_ntoa(*ip_addr)
		 : "(hidden)"));
}

struct in_addr * get_ip_client(DESCRIPTOR_DATA_CLIENT * client)
{
	if (client->connect
	&& *((char *) (&(client->connect->addr.sin_addr))))
		return &(client->connect->addr.sin_addr);
	return NULL;
}

bool check_out_ready_client(DESCRIPTOR_DATA_CLIENT * client)
{
	if (client == NULL
	|| client->connect == NULL
	|| client->connect->outtop == 0
	|| !IS_SET(client->flags, DDCF_CAN_OUT))
		return FALSE;
	return TRUE;
}

int hostcmp(DESCRIPTOR_DATA_CLIENT * c1, DESCRIPTOR_DATA_CLIENT * c2)
{
	if (c1->connect == NULL && c2->connect == NULL)
		return 0;
	if (c1->connect == NULL)
		return 1;
	if (c2->connect == NULL)
		return -1;
	return memcmp(	&(c1->connect->addr.sin_addr),
			&(c2->connect->addr.sin_addr),
			sizeof(c1->connect->addr.sin_addr));
}

void echo_off_string(DESCRIPTOR_DATA_CLIENT * d)
{
	raw_write_to_buffer_client(d, echo_off_str, 0);
}

void echo_on_string(DESCRIPTOR_DATA_CLIENT * d)
{
	raw_write_to_buffer_client(d, echo_on_str, 0);
}

/*
 *	may need send after each prompt
 */
void send_ga_string(DESCRIPTOR_DATA_CLIENT * d)
{
	raw_write_to_buffer_client(d, go_ahead_str, 0);
}

