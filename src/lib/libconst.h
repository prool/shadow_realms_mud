/*
 * libconst.h		<MGater libs>
 */
#ifndef _LIBCONST_H_
#define _LIBCONST_H_

/*
 * Data files used by the server.
 *
 * AREA_LIST contains a list of areas to boot.
 * All files are read in completely at bootup.
 * Most output files (bug, idea, typo, shutdown) are append-only.
 *
 * The NULL_FILE is held open so that we have a stream handle in reserve,
 *   so players can go ahead and telnet to all the other descriptors.
 * Then we close it whenever we need to open a file (e.g. a save file).
 */

#if defined (WIN32)
#	include <stdlib.h>
#	define PATH_MAX	_MAX_PATH
#	define PATH_SEPARATOR '\\'
#else
#	define PATH_SEPARATOR '/'
#endif

#if defined (WIN32)
#	define NULL_FILE	"NUL"	/* To reserve one stream */
#else
#	define NULL_FILE	"/dev/null"	/* To reserve one stream */
#endif

#define HID_FILE		"hid.db"

/*
 * String and memory management parameters.
 */
#define MAX_KEY_HASH		1024
#define MAX_STRING_HASH		16384

#define MAX_INPUT_LENGTH	1024
//#define MAX_STRING_LENGTH	8192
#define MAX_STRING_LENGTH	(MAX_INPUT_LENGTH * 8)

#define MAX_INCL_BUF_LENGTH	(MAX_STRING_LENGTH * 4) /* max inbuf size for clients */

#define MAX_TUNNEL_BUF_OUT	(MAX_STRING_LENGTH * 4)
#define MAX_TUNNEL_BUF_IN	(MAX_STRING_LENGTH * 4)

/* RT ASCII conversions */

#define A	((flag64_t) 1 <<  0)
#define B	((flag64_t) 1 <<  1)
#define C	((flag64_t) 1 <<  2)
#define D	((flag64_t) 1 <<  3)
#define E	((flag64_t) 1 <<  4)
#define F	((flag64_t) 1 <<  5)
#define G	((flag64_t) 1 <<  6)
#define H	((flag64_t) 1 <<  7)

#define I	((flag64_t) 1 <<  8)
#define J	((flag64_t) 1 <<  9)
#define K	((flag64_t) 1 << 10)
#define L	((flag64_t) 1 << 11)
#define M	((flag64_t) 1 << 12)
#define N	((flag64_t) 1 << 13)
#define O	((flag64_t) 1 << 14)
#define P	((flag64_t) 1 << 15)

#define Q	((flag64_t) 1 << 16)
#define R	((flag64_t) 1 << 17)
#define S	((flag64_t) 1 << 18)
#define T	((flag64_t) 1 << 19)
#define U	((flag64_t) 1 << 20)
#define V	((flag64_t) 1 << 21)
#define W	((flag64_t) 1 << 22)
#define X	((flag64_t) 1 << 23)

#define Y	((flag64_t) 1 << 24)
#define Z	((flag64_t) 1 << 25)
#define aa	((flag64_t) 1 << 26) /* letters doubled due to conflicts */
#define bb	((flag64_t) 1 << 27)
#define cc	((flag64_t) 1 << 28)
#define dd	((flag64_t) 1 << 29)
#define ee	((flag64_t) 1 << 30)
#define ff	((flag64_t) 1 << 31)

#define gg	((flag64_t) 1 << 32)
#define hh	((flag64_t) 1 << 33)
#define ii	((flag64_t) 1 << 34)
#define jj	((flag64_t) 1 << 35)
#define kk	((flag64_t) 1 << 36)
#define ll	((flag64_t) 1 << 37)
#define mm	((flag64_t) 1 << 38)
#define nn	((flag64_t) 1 << 39)

#define oo	((flag64_t) 1 << 40)
#define pp	((flag64_t) 1 << 41)
#define qq	((flag64_t) 1 << 42)
#define rr	((flag64_t) 1 << 43)
#define ss	((flag64_t) 1 << 44)
#define tt	((flag64_t) 1 << 45)
#define uu	((flag64_t) 1 << 46)
#define vv	((flag64_t) 1 << 47)

#define ww	((flag64_t) 1 << 48)
#define xx	((flag64_t) 1 << 49)
#define yy	((flag64_t) 1 << 50)
#define zz	((flag64_t) 1 << 51)

#endif
