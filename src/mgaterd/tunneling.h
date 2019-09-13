/*
 * tunneling.h		<MGaterD>
 */

#ifndef	_MG_TUNNELING_H_
#define _MG_TUNNELING_H_

#include <libmnet.h>

DESCRIPTOR_DATA_MAIN * connect_to_muddy(struct sockaddr_in * fromhost, \
		struct sockaddr_in *target);
bool process_tunneling_in(void);	// from client to remote muddy
bool process_tunneling_out(void);	// from remote muddy to connected clients

#endif	// _MG_TUNNELING_H_

