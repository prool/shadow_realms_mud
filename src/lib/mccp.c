/*
 * mccp.c	<MGater libs>
 * (c) Shadow Realms     2003
 */

/*
 *	Requiare zlib (http://www.cdrom.com/pub/infozip/zlib/)
 *		(zlib-devel*.rpm for Linux Red Hat)
 *	Xor: "I use Rot14-mccp.patch for create SR MCCP support"
 *		<URL:http://homepages.ihug.co.nz/~icecube/compress/>
 *		http://www.randomly.org/projects/MCCP/
 *		Copyright (c) 1999, Oliver Jowett <icecube@ihug.co.nz>
 */

#ifdef MCCP	// see Makefile

#ifndef LINUX
#define     ENOSR           63      /* Out of streams resources */
#endif

#include <ctype.h>
//#include <limits.h>
//#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/telnet.h>
#include <zlib.h>
#include <netinet/in.h>
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netinet/in.h>
//#include <arpa/inet.h>
//#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

//#include "descriptors.h"
#include "log.h"
#include "libmccp.h"


/**********************************************************************
 *
 * 			Common MCCP
 *
 **********************************************************************/

char	compress_start	[] = {	IAC, SB, TELOPT_COMPRESS, WILL, SE, '\0' };
char	compress_start2	[] = {	IAC, SB, TELOPT_COMPRESS2, IAC, SE, '\0' };

/* mccp: compression negotiation strings */
/*
 *	IAC WILL COMPRESS indicates the sender supports version 1 of the
 *	protocol, and is willing to compress data it sends. 
 *	
 * 	IAC WILL COMPRESS2 indicates the sender supports version 2, and is willing
 * 	to compress data it sends. 
 */
const   char    compress_will   [] = { IAC, WILL, TELOPT_COMPRESS, '\0' };
/*
 *	IAC DO COMPRESS indicates the sender supports version 1 of the protocol,
 *	and is willing to decompress data received. 
 *
 * 	IAC DO COMPRESS2 indicates the sender supports version 2 or above,
 * 	and is willing to decompress data received. 
 */
const   char    compress_do     [] = { IAC, DO, TELOPT_COMPRESS, '\0' };
/*
 *	IAC DONT COMPRESS indicates the sender refuses to support version 1.
 *	If compression was previously negotiated and is currently being used,
 *	the server should terminate compression. 
 *
 *	IAC DONT COMPRESS2 indicates the sender refuses to support version 2.
 *	If compression was previously negotiated and is currently being used,
 *	the server should terminate compression. 
 */
const   char    compress_dont   [] = { IAC, DONT, TELOPT_COMPRESS, '\0' };

/*
 *	IAC WONT COMPRESS indicates the sender refuses to compress data using version 1. 
 *	IAC WONT COMPRESS2 indicates the sender refuses to compress data using version 2. 
 */

//DESCRIPTOR_DATA_MUD *tunnel_list = NULL;
//varr listen_mgater_list = { sizeof(LISTEN_MGATER), 1 };
//varr mgater_sockets = { sizeof(int), 2 };

bool	processCompressed(DESCRIPTOR_DATA_CLIENT *desc);

/*
 *	Memory management - zlib uses these hooks to allocate and free memory
 *	it needs
 */

bool check_mccp(DESCRIPTOR_DATA_CLIENT *desc)
{
	return desc && desc->out_compress ? TRUE : FALSE;
}

void *zlib_alloc(void *opaque, unsigned int items, unsigned int size)
{
	return calloc(items, size);
}

void zlib_free(void *opaque, void *address)
{
	free(address);
}

/*
 *	Begin compressing data on `desc'
 */
bool compressStart(DESCRIPTOR_DATA_CLIENT *desc)
{
	z_stream *s;

	if (desc->out_compress)	/* already compressing */
		return TRUE;

	s = (z_stream *) calloc(1, sizeof(*s));
	desc->out_compress_buf = (unsigned char *) calloc(1, COMPRESS_BUF_SIZE);

	s->next_in = NULL;
	s->avail_in = 0; 	/* not requaire: calloc do zero mem swap */

	s->next_out = desc->out_compress_buf;
	s->avail_out = COMPRESS_BUF_SIZE;

	s->data_type	= Z_BINARY;

	s->zalloc = zlib_alloc;
	s->zfree  = zlib_free;
	s->opaque = NULL;

	if (deflateInit(s, Z_DEFAULT_COMPRESSION) != Z_OK) {
		/* change Z_DEFAULT_COMPRESSION to 9 for max compression */
		/* problems with zlib, try to clean up */
		//free_mem(desc->out_compress_buf, COMPRESS_BUF_SIZE);
		//free_mem(s, sizeof(z_stream));
		free(desc->out_compress_buf);
		desc->out_compress_buf = NULL;
		free(s);
		return FALSE;
	}

	raw_write_to_buffer_client(desc, compress_start, strlen(compress_start) );
	
	/* now we're compressing */
	desc->out_compress = s;
	return TRUE;
}

/* Cleanly shut down compression on `desc' */
bool compressEnd(DESCRIPTOR_DATA_CLIENT *desc)
{
	unsigned char dummy[1];

	if (!desc->out_compress)
		return TRUE;
	
	desc->out_compress->avail_in = 0;
	desc->out_compress->next_in = dummy;

	/* No terminating signature is needed - receiver will get Z_STREAM_END */
	
	if (deflate(desc->out_compress, Z_FINISH) != Z_STREAM_END)
		return FALSE;

	if (!processCompressed(desc)) /* try to send any residual data */
		return FALSE;

	deflateEnd(desc->out_compress);
	//free_mem(desc->out_compress_buf, COMPRESS_BUF_SIZE);
	//free_mem(desc->out_compress, sizeof(z_stream));
	free(desc->out_compress_buf);
	free(desc->out_compress);
	desc->out_compress = NULL;
	desc->out_compress_buf = NULL;

	return TRUE;
}

/* Try to send any pending compressed-but-not-sent data in `desc' */
bool processCompressed(DESCRIPTOR_DATA_CLIENT *desc)
{
	int /*iStart, nBlock, nWrite,*/ len;
	int iSave;

	if (!desc->out_compress)
		return TRUE;

	/* Try to write out some data.. */
	len = desc->out_compress->next_out - desc->out_compress_buf;
	if (len > 0)
	{
		//log_printf("mccp_d: %s", desc->out_compress->next_out);
		iSave = write_to_buffer_main(desc->connect, (char*)desc->out_compress_buf, len);
		if (iSave && iSave < len)
		{
			memmove(desc->out_compress_buf, desc->out_compress_buf + len - iSave,
					iSave);
		}
		desc->out_compress->next_out = desc->out_compress_buf + iSave;
	}
#if 0
	if (len > 0) {
		/* we have some data to write */
		for (iStart = 0; iStart < len; iStart += nWrite)
		{
			nBlock = UMIN (len - iStart, 4096);

#if !defined( WIN32 )
			if ((nWrite = write (desc->connect->socket,
				desc->out_compress_buf + iStart, nBlock)) < 0)
#else
			if ((nWrite = send(desc->connect->socket,
				desc->out_compress_buf + iStart, nBlock, 0)) < 0)
#endif
			{
				if (errno == EAGAIN || errno == ENOSR)
					break;

				log_printf("processCompressed: %s", strerror(errno));
				return FALSE; /* write error */
			}

			if (nWrite <= 0)
				break;
		}
		if (iStart) {
			/* We wrote "iStart" bytes */
			if (iStart < len)
				memmove(desc->out_compress_buf,
					desc->out_compress_buf+iStart,
					len - iStart);
			desc->out_compress->next_out = desc->out_compress_buf
				+ len - iStart;
		}
	}
#endif		// 0
	return TRUE;
}

/* write_to_descriptor, the compressed case */
bool writeCompressed(DESCRIPTOR_DATA_CLIENT *desc, const char *txt, uint length)
{
	z_stream *s = desc->out_compress;

	s->next_in = (unsigned char *)txt;
	s->avail_in = length;
	/*	raw log
	{
		char  ttttstr[MAX_STRING_LENGTH];

		snprintf(ttttstr, sizeof(ttttstr), "%s", txt);
		ttttstr[length] = '\0';
		log_printf("mccp_d2: %s", ttttstr);
	}
	*/

	while (s->avail_in) {
		s->avail_out = COMPRESS_BUF_SIZE
			- (s->next_out - desc->out_compress_buf);
		if (s->avail_out) {
			int status = deflate(s, Z_SYNC_FLUSH);

			if (status != Z_OK) {
				/* Boom */
				return FALSE;
			}
		}
		/* Try to write out some data.. */
		if (!processCompressed(desc))
			return FALSE;
		/* loop */
	}
	/* Done. */
	return TRUE;
}

#if 0
/* mccp: write_to_descriptor wrapper */
bool	write_to_descriptor(DESCRIPTOR_DATA_USER *d, const char *txt, uint length)
{
	if (d->out_compress)
		return writeCompressed(d, txt, length);
	else
		return write_to_descriptor_nz(d->socket, txt, length);
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

#endif	//#ifdef 0

#endif	//#ifdef MCCP 

