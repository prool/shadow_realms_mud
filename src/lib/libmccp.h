/*
 * libmccp.h		<MGater libs>
 * 	Shadow Realms (2003)
 */

#ifdef MCCP	// see Makefile
#ifndef _LIBMCCP_H_
#define _LIBMCCP_H_

#include <zlib.h>
#include "libmnet.h"
#include "libtypedef.h"

#define TELOPT_COMPRESS 85
#define TELOPT_COMPRESS2 86
#define COMPRESS_BUF_SIZE 16384

/* mccp: compression negotiation strings */
extern const	char	compress_will	[];
extern const	char	compress_do	[];
extern const	char	compress_dont	[];
//extern const	char	compress_start  [];

/* MCCP */
bool check_mccp(DESCRIPTOR_DATA_CLIENT *desc);
bool compressStart(DESCRIPTOR_DATA_CLIENT *desc);
bool compressEnd(DESCRIPTOR_DATA_CLIENT *desc);
bool processCompressed(DESCRIPTOR_DATA_CLIENT *desc);
bool writeCompressed(DESCRIPTOR_DATA_CLIENT *desc, const char *txt, uint length);

//bool write_to_descriptor_nz(int desc, const char *txt, uint length);
//bool write_to_descriptor(DESCRIPTOR_DATA_USER *d, const char *txt, uint length);

#endif		// _MGATER_H_
#endif		// MCCP

