/*
 * mgaterd.h		<MGaterD>
 */

#ifndef _MG_MGATERD_H_
#define _MG_MGATERD_H_

#include "const.h"

void mgater_done(int exit_code, const char *str);

/* connected clients to mgater daemon */
typedef struct descriptor_data_cmuddy DESCRIPTOR_DATA_CMUDDY;
struct descriptor_data_cmuddy
{
	DESCRIPTOR_DATA_CLIENT	* client;
	DESCRIPTOR_DATA_CMUDDY	* next;

	flag32_t		flags;
};

/*
 *	flags of DESCRIPTOR_DATA_CMUDDY
 */
#define		DDCF_NONE			0
#define		DDCF_REQ_REMOTE_INIT		(A)


DESCRIPTOR_DATA_CMUDDY * descriptor_list_cmuddy;

/* Error codes */
#define		ERRMG_NONE			0
#define		ERRMG_WRONG_ARGUMENTS		1	/* parametrs string in shell call */
#define		ERRMG_FAIL_CREATE_TUNNEL	2
#define		ERRMG_FAIL_INIT_TUNNEL		3
//#define		ERRMG_FAIL_CONN_MUD		4
#define		ERRMG_FAIL_FORK			5
#define		ERRMG_FAIL_NETWORK		6
#define		ERRMG_FAIL_SELECT_P		7	/* select for pause */
#define		ERRMG_FAIL_READ_CONFIG		8
#define		ERRMG_FIND_PID			9
#define		ERRMG_FAIL_REUID		10

//#define		ERRMG_FAIL_INIT_SOCKET		20
#define		ERRMG_NOMEM			21

#define		ERRMG_EXIT_CLEANUP		254
#define		ERRMG_UNKNOWN			255
/* end of 'Error codes' */

#endif

