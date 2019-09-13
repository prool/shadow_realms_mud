/*
 * $Id: libtypedef.h,v 1.2 2003/04/22 07:35:33 xor Exp $
 */

#ifndef _LIBTYPEDEF_H_
#define _LIBTYPEDEF_H_

#include <sys/types.h>

#if	!defined(FALSE)
#define FALSE	 0
#endif

#if	!defined(TRUE)
#define TRUE	 1
#endif

typedef int bool;

#if defined (WIN32)
typedef unsigned int			uint;
#endif

/* WIN32 Microsoft specific definitions */
#if defined (WIN32)
#	define vsnprintf	_vsnprintf
#	define snprintf		_snprintf
#	define vsnprintf	_vsnprintf
#endif
 
/* 64-bit int value is compiler-specific (not a ANSI standard) */
#if defined (WIN32)
typedef __int64		flag64_t;	/* For MSVC4.2/5.0 - flags */
typedef __int32		flag32_t;	/* short flags (less memory usage) */
typedef unsigned int	u_int;
typedef unsigned char	u_char;
#else
typedef int64_t		flag64_t;	/* For GNU C compilers - flags */
typedef int32_t		flag32_t;	/* short flags (less memory usage) */
#endif
  
#define IS_NULLSTR(str)		(!(str) || *(char*)(str) == '\0')

#define IS_SET(flag, bit)	((flag) & (bit))
#define SET_BIT(var, bit)	((var) |= (bit))
#define TOGGLE_BIT(var, bit)    ((var) ^= (bit))
#define REMOVE_BIT(var, bit)	((var) &= ~(bit))

#define UMIN(a, b)		((a) < (b) ? (a) : (b))
#define UMAX(a, b)		((a) > (b) ? (a) : (b))
#define URANGE(a, b, c) 	((b) < (a) ? (a) : ((b) > (c) ? (c) : (b)))
#define ENTRE(min, num, max)	((min) < (num) && (num) < (max))

#define IS_VALID(data)		((data))
#define VALIDATE(data)
#define INVALIDATE(data)

#define ISLOWER(c)		(islower((unsigned char) (c)))
#define ISUPPER(c)		(isupper((unsigned char) (c)))
#define LOWER(c)		(to_lower_case((unsigned char) (c)))
#define UPPER(c)		(to_upper_case((unsigned char) (c)))

/*
 *
 */
//typedef struct descriptor_data_user DESCRIPTOR_DATA_USER;
//typedef struct descriptor_data_tunnel DESCRIPTOR_DATA_TUNNEL;
//typedef struct link_tunnel_data LINK_TUNNEL_DATA;

#endif

