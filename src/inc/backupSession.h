/*
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 2 of the License, or
 *	(at your option) any later version.
 *	Powered by Inteno Broadband Technology AB
 *
 *	Copyright (C) 2013 Mohamed Kallel <mohamed.kallel@pivasoftware.com>
 *	Copyright (C) 2013 Ahmed Zribi <ahmed.zribi@pivasoftware.com>
 *
 */

#ifndef _BACKUPSESSION_H__
#define _BACKUPSESSION_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <microxml.h>
#include <unistd.h>
#include "xml.h"

#define RPC_NO_STATUS   -1
#define RPC_QUEUE       0
#define RPC_SEND        1

# define CWMP_BACKUP_SESSION "<cwmp></cwmp>"

typedef enum backup_loading {
    ALL,
    ACS
} backup_loading;

struct search_keywords {
	char		*name;
	char		*value;
};

void bkp_session_save();
int cwmp_load_saved_session(struct cwmp *cwmp, char **acsurl, enum backup_loading load);
mxml_node_t *bkp_session_insert_event(int index, char *command_key, int id, char *status);
void bkp_session_delete_event(int id, char *status);
void bkp_session_insert_parameter(mxml_node_t *b, char *name);
void bkp_session_insert_acs(char *value);
void bkp_session_move_inform_to_inform_send ();
void bkp_session_move_inform_to_inform_queue ();
void bkp_session_insert_schedule_inform(time_t schedule_time,char *command_key);
void bkp_session_delete_schedule_inform(time_t schedule_time,char *command_key);
void bkp_session_insert_download(struct download *pdownload);
void bkp_session_delete_download(struct download *pdownload);
void bkp_session_insert_transfer_complete(struct transfer_complete *ptransfer_complete);
void bkp_session_delete_transfer_complete(struct transfer_complete *ptransfer_complete);

#endif /* _BACKUPSESSION_H__ */
