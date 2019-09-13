/*
 * $Id: comm_control.h,v 1.31 2003/04/22 07:35:22 xor Exp $
 */

#ifndef _COMM_CONTROL_H_
#define _COMM_CONTROL_H_

typedef struct control_desc CONTROL_DESC;
struct control_desc {
//	int		fd;
	DESCRIPTOR_DATA_CLIENT	* client;
	CONTROL_DESC		* next;
};

//void info_newconn	(int infofd);
CONTROL_DESC * control_newconn(DESCRIPTOR_DATA_CLIENT *client);
void control_process_cmd(CONTROL_DESC *id, const char * argument);
void close_descriptor_control(CONTROL_DESC *id);
void control_desc_fulldestroy(CONTROL_DESC *id);

extern CONTROL_DESC *ctrl_list;

#endif

