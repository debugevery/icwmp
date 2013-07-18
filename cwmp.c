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

#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include "cwmp.h"
#include "backupSession.h"
#include "xml.h"
#include "log.h"
#include "external.h"

struct cwmp         	cwmp_main = {0};

struct rpc *cwmp_add_session_rpc_acs (struct session *session, int type)
{
    struct rpc     *rpc_acs;

    rpc_acs = calloc (1,sizeof(struct rpc));
    if (rpc_acs==NULL)
    {
        return NULL;
    }
    rpc_acs->type = type;
    list_add_tail (&(rpc_acs->list), &(session->head_rpc_acs));
    return rpc_acs;
}

struct rpc *cwmp_add_session_rpc_cpe (struct session *session, int type)
{
    struct rpc     *rpc_cpe;

    rpc_cpe = calloc (1,sizeof(struct rpc));
    if (rpc_cpe==NULL)
    {
        return NULL;
    }
    rpc_cpe->type = type;
    list_add_tail (&(rpc_cpe->list), &(session->head_rpc_cpe));
    return rpc_cpe;
}

struct rpc *cwmp_add_session_rpc_acs_head (struct session *session, int type)
{
    struct rpc     *rpc_acs;

    rpc_acs = calloc (1,sizeof(struct rpc));
    if (rpc_acs==NULL)
    {
        return NULL;
    }
    rpc_acs->type = type;
    list_add (&(rpc_acs->list), &(session->head_rpc_acs));
    return rpc_acs;
}

int cwmp_session_rpc_destructor (struct rpc *rpc)
{
    list_del(&(rpc->list));
    free (rpc);
    return CWMP_OK;
}

int cwmp_get_retry_interval (struct cwmp *cwmp)
{
    switch (cwmp->retry_count_session)
    {
        case 0: return MAX_INT32;
        case 1: return 6;
        case 2: return 11;
        case 3: return 21;
        case 4: return 41;
        case 5: return 81;
        case 6: return 161;
        case 7: return 321;
        case 8: return 641;
        case 9: return 1281;
        default: return 2561;
    }
}

static void cwmp_prepare_value_change (struct cwmp *cwmp, struct session *session)
{
	struct event_container *event_container;
	if (external_list_value_change.next == &(external_list_value_change))
		return;
	pthread_mutex_lock(&(cwmp->mutex_session_queue));
	event_container = cwmp_add_event_container (cwmp, EVENT_IDX_4VALUE_CHANGE, "");
	if (!event_container) goto end;
	pthread_mutex_lock(&(external_mutex_value_change));
	list_splice_init(&(external_list_value_change), &(event_container->head_parameter_container));
	pthread_mutex_unlock(&(external_mutex_value_change));
	cwmp_save_event_container (cwmp,event_container);

end:
	pthread_mutex_unlock(&(cwmp->mutex_session_queue));
}

void cwmp_schedule_session (struct cwmp *cwmp)
{
    struct list_head                    *ilist;
    struct session                      *session;
    int                                 t,error = CWMP_OK;
    static struct timespec              time_to_wait = {0, 0};
    bool                                retry = false;

    while (1)
    {
    	pthread_mutex_lock (&(cwmp->mutex_session_send));
    	ilist = (&(cwmp->head_session_queue))->next;
        while ((ilist == &(cwmp->head_session_queue)) || retry)
        {
            t = cwmp_get_retry_interval(cwmp);
            time_to_wait.tv_sec = time(NULL) + t;
            CWMP_LOG(INFO,"Waiting the next session");
            pthread_cond_timedwait(&(cwmp->threshold_session_send), &(cwmp->mutex_session_send), &time_to_wait);
            ilist = (&(cwmp->head_session_queue))->next;
            retry = false;
        }

        session = list_entry(ilist, struct session, list);

        cwmp_prepare_value_change(cwmp, session);

        if (error = cwmp_move_session_to_session_send (cwmp, session))
        {
            CWMP_LOG(EMERG,"FATAL error in the mutex process in the session scheduler!");
            exit(EXIT_FAILURE);
        }
        CWMP_LOG (INFO,"Start session");
        error = cwmp_schedule_rpc (cwmp,session);
        CWMP_LOG (INFO,"End session");
        run_session_end_func(session);
        if (session->error == CWMP_RETRY_SESSION)
        {
            error = cwmp_move_session_to_session_queue (cwmp, session);
            CWMP_LOG(INFO,"Retry session, retry count = %d, retry in %ds",cwmp->retry_count_session,cwmp_get_retry_interval(cwmp));
            retry = true;
            pthread_mutex_unlock (&(cwmp->mutex_session_send));
            continue;
        }
        cwmp_session_destructor (cwmp, session);
        cwmp->session_send          = NULL;
        cwmp->retry_count_session   = 0;
        pthread_mutex_unlock (&(cwmp->mutex_session_send));
    }
}

int cwmp_rpc_cpe_handle_message (struct session *session, struct rpc *rpc_cpe)
{
	if (xml_prepare_msg_out(session))
		return -1;
	if (rpc_cpe_methods[rpc_cpe->type].handler(session, rpc_cpe))
		return -1;
	if (xml_set_cwmp_id(session))
		return -1;
	return 0;
}

int cwmp_schedule_rpc (struct cwmp *cwmp, struct session *session)
{
    struct list_head	*ilist;
    struct rpc			*rpc_acs, *rpc_cpe;
    int					error=0,e;

    if (http_client_init(cwmp)) {
		CWMP_LOG(INFO, "Initializing http client failed");
		goto retry;
	}

    while (1)
    {
        list_for_each(ilist, &(session->head_rpc_acs))
        {
            rpc_acs = list_entry (ilist, struct rpc, list);
            if (!rpc_acs->type)
            	goto retry;

            CWMP_LOG (INFO,"Preparing the %s RPC message to send to the ACS",
            		rpc_acs_methods[rpc_acs->type].name);
            if (rpc_acs_methods[rpc_acs->type].prepare_message(cwmp, session, rpc_acs))
            	goto retry;

            CWMP_LOG (INFO,"Send the %s RPC message to the ACS",
            		rpc_acs_methods[rpc_acs->type].name);
            if (xml_send_message(cwmp, session, rpc_acs))
            	goto retry;

            CWMP_LOG (INFO,"Get the %sResponse message from the ACS",
                        		rpc_acs_methods[rpc_acs->type].name);
            if (rpc_acs_methods[rpc_acs->type].parse_response)
            	if (rpc_acs_methods[rpc_acs->type].parse_response(cwmp, session, rpc_acs))
            		goto retry;

            ilist = ilist->prev;
            if (rpc_acs_methods[rpc_acs->type].extra_clean != NULL)
            	rpc_acs_methods[rpc_acs->type].extra_clean(session,rpc_acs);
            cwmp_session_rpc_destructor(rpc_acs);
            MXML_DELETE(session->tree_in);
            MXML_DELETE(session->tree_out);
            if (session->hold_request)
                 break;
        }
        CWMP_LOG (INFO,"Send empty message to the ACS");
        if (xml_send_message(cwmp, session, NULL))
			goto retry;
		if (!session->tree_in)
			goto next;

		CWMP_LOG (INFO,"Receive request from the ACS");
		if (xml_handle_message(session))
			goto retry;

		while (session->head_rpc_cpe.next != &(session->head_rpc_cpe))
		{
			rpc_cpe = list_entry (session->head_rpc_cpe.next, struct rpc, list);
			if (!rpc_cpe->type)
				goto retry;

			CWMP_LOG (INFO,"Preparing the %s%s message",
					rpc_cpe_methods[rpc_cpe->type].name,
					(rpc_cpe->type != RPC_CPE_FAULT) ? "Response" : "");
			if (cwmp_rpc_cpe_handle_message(session, rpc_cpe))
				goto retry;
			MXML_DELETE(session->tree_in);

			CWMP_LOG (INFO,"Send the %s%s message to the ACS",
					rpc_cpe_methods[rpc_cpe->type].name,
					(rpc_cpe->type != RPC_CPE_FAULT) ? "Response" : "");
			if (xml_send_message(cwmp, session, rpc_cpe))
				goto retry;
			MXML_DELETE(session->tree_out);

			cwmp_session_rpc_destructor(rpc_cpe);
			if (!session->tree_in)
				break;

			CWMP_LOG (INFO,"Receive request from the ACS");
			if (xml_handle_message(session))
				goto retry;
		}

next:
		if (session->head_rpc_acs.next==&(session->head_rpc_acs))
            break;
        MXML_DELETE(session->tree_in);
        MXML_DELETE(session->tree_out);
    }

success:
	session->error = CWMP_OK;
	goto end;

retry:
	CWMP_LOG (INFO,"Failed");
	session->error = CWMP_RETRY_SESSION;

end:
	MXML_DELETE(session->tree_in);
	MXML_DELETE(session->tree_out);
	http_client_exit();
	xml_exit();
    return session->error;
}

int cwmp_move_session_to_session_send (struct cwmp *cwmp, struct session *session)
{
    pthread_mutex_lock (&(cwmp->mutex_session_queue));
    if (cwmp->session_send != NULL)
    {
        pthread_mutex_unlock (&(cwmp->mutex_session_queue));
        return CWMP_MUTEX_ERR;
    }
    list_del (&(session->list));
    cwmp->session_send          = session;
    cwmp->head_event_container  = NULL;
    bkp_session_move_inform_to_inform_send ();
    bkp_session_save();
    pthread_mutex_unlock (&(cwmp->mutex_session_queue));
    return CWMP_OK;
}

int cwmp_move_session_to_session_queue (struct cwmp *cwmp, struct session *session)
{
    struct list_head            *ilist, *jlist;
    struct rpc              	*rpc_acs, *queue_rpc_acs, *rpc_cpe;
    struct event_container      *event_container_old, *event_container_new;
    struct session              *session_queue;
    bool                        dup;

    pthread_mutex_lock (&(cwmp->mutex_session_queue));
    cwmp->retry_count_session ++;
    cwmp->session_send  = NULL;
    if (cwmp->head_session_queue.next == &(cwmp->head_session_queue))
    {
        list_add_tail (&(session->list), &(cwmp->head_session_queue));
        session->hold_request       = 0;
        session->digest_auth        = 0;
        cwmp->head_event_container  = &(session->head_event_container);
        if (session->head_rpc_acs.next != &(session->head_rpc_acs))
        {
            rpc_acs = list_entry(session->head_rpc_acs.next, struct rpc, list);
            if (rpc_acs->type != RPC_ACS_INFORM)
            {
                if ((rpc_acs = cwmp_add_session_rpc_acs_head(session, RPC_ACS_INFORM)) == NULL)
                {
                    pthread_mutex_unlock (&(cwmp->mutex_session_queue));
                    return CWMP_MEM_ERR;
                }
            }
        }
        else
        {
        	if ((rpc_acs = cwmp_add_session_rpc_acs_head(session, RPC_ACS_INFORM)) == NULL)
			{
				pthread_mutex_unlock (&(cwmp->mutex_session_queue));
				return CWMP_MEM_ERR;
			}
        }
        while (session->head_rpc_cpe.next != &(session->head_rpc_cpe))
		{
        	rpc_cpe = list_entry(session->head_rpc_cpe.next, struct rpc, list);
			cwmp_session_rpc_destructor(rpc_cpe);
		}
        bkp_session_move_inform_to_inform_queue ();
        bkp_session_save();
        pthread_mutex_unlock (&(cwmp->mutex_session_queue));
        return CWMP_OK;
    }
    list_for_each(ilist, &(session->head_event_container))
    {
        event_container_old = list_entry (ilist, struct event_container, list);
        event_container_new = cwmp_add_event_container (cwmp, event_container_old->code, event_container_old->command_key);
        if (event_container_new == NULL)
        {
            pthread_mutex_unlock (&(cwmp->mutex_session_queue));
            return CWMP_MEM_ERR;
        }
        list_splice_init(&(event_container_old->head_parameter_container),
        		&(event_container_new->head_parameter_container));
        cwmp_save_event_container (cwmp,event_container_new);
    }
    session_queue = list_entry(cwmp->head_event_container,struct session, head_event_container);
    list_for_each(ilist, &(session->head_rpc_acs))
    {
        rpc_acs = list_entry(ilist, struct rpc, list);
        dup     = false;
        list_for_each(jlist, &(session_queue->head_rpc_acs))
        {
            queue_rpc_acs = list_entry(jlist, struct rpc, list);
            if (queue_rpc_acs->type == rpc_acs->type &&
                (rpc_acs->type == RPC_ACS_INFORM ||
                 rpc_acs->type == RPC_ACS_GET_RPC_METHODS))
            {
                dup = true;
                break;
            }
        }
        if (dup)
        {
            continue;
        }
        ilist = ilist->prev;
        list_del(&(rpc_acs->list));
        list_add_tail (&(rpc_acs->list), &(session_queue->head_rpc_acs));
    }
    cwmp_session_destructor (cwmp, session);
    pthread_mutex_unlock (&(cwmp->mutex_session_queue));
    return CWMP_OK;
}

void cwmp_set_end_session (unsigned int end_session_flag)
{
	if (cwmp_main.session_send)
		cwmp_main.session_send->end_session |= end_session_flag;
}

int cwmp_session_destructor (struct cwmp *cwmp, struct session *session)
{
	struct rpc          *rpc;
    struct session_end_func *session_end_func;

    while (session->head_rpc_acs.next != &(session->head_rpc_acs))
    {
    	rpc = list_entry(session->head_rpc_acs.next, struct rpc, list);
		if (rpc_acs_methods[rpc->type].extra_clean != NULL)
			rpc_acs_methods[rpc->type].extra_clean(session,rpc);
    	cwmp_session_rpc_destructor(rpc);
    }
    while (session->head_rpc_cpe.next != &(session->head_rpc_cpe))
    {
    	rpc = list_entry(session->head_rpc_cpe.next, struct rpc, list);
    	cwmp_session_rpc_destructor(rpc);
    }
    if (session->list.next != NULL && session->list.prev != NULL)
    {
        list_del (&(session->list));
    }
    free (session);

    return CWMP_OK;
}



struct session *cwmp_add_queue_session (struct cwmp *cwmp)
{
    struct session     *session;
    struct rpc		   *rpc_acs;

    session = calloc (1,sizeof(struct session));
    if (session==NULL)
    {
        return NULL;
    }
    list_add_tail (&(session->list), &(cwmp->head_session_queue));
    INIT_LIST_HEAD (&(session->head_event_container));
    INIT_LIST_HEAD (&(session->head_rpc_acs));
    INIT_LIST_HEAD (&(session->head_rpc_cpe));
    if ((rpc_acs = cwmp_add_session_rpc_acs_head(session, RPC_ACS_INFORM)) == NULL)
    {
    	free (session);
        return NULL;
    }

    return session;
}

int run_session_end_func (struct session *session)
{
	if (session->end_session & END_SESSION_EXTERNAL_ACTION)
	{
		CWMP_LOG (INFO,"Executing external commands: end session request");
		external_simple("end_session", NULL);
	}

	if (session->end_session & END_SESSION_FACTORY_RESET)
	{
		CWMP_LOG (INFO,"Executing factory reset: end session request");
		external_simple("factory_reset", NULL);
		exit(EXIT_SUCCESS);
	}

	if (session->end_session & END_SESSION_REBOOT)
	{
		CWMP_LOG (INFO,"Executing Reboot: end session request");
		external_simple("reboot", NULL);
		exit(EXIT_SUCCESS);
	}

	if (session->end_session & END_SESSION_RELOAD)
	{
		CWMP_LOG (INFO,"Config reload: end session request");
		cwmp_apply_acs_changes();
	}

	session->end_session = 0;

	return CWMP_OK;
}

int cwmp_apply_acs_changes ()
{
    int error;
    if (error = cwmp_config_reload(&cwmp_main))
    {
        return error;
    }
    if (error = cwmp_root_cause_events(&cwmp_main))
    {
        return error;
    }
    return CWMP_OK;
}

void *thread_uloop_run (void *v)
{
	ubus_init(&cwmp_main);
	return NULL;
}

void *thread_exit_program (void *v)
{
	CWMP_LOG(INFO,"EXIT CWMP");
	pthread_mutex_lock(&mutex_backup_session);
	exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{

    struct cwmp                     *cwmp = &cwmp_main;
    int                             error;
    pthread_t                       periodic_event_thread;
    pthread_t                       scheduleInform_thread;
    pthread_t                       download_thread;
    pthread_t                       ubus_thread;

    if (error = cwmp_init(argc, argv, cwmp))
    {
        return error;
    }
    CWMP_LOG(INFO,"STARTING CWMP");

    if (error = cwmp_init_backup_session(cwmp, NULL, ALL))
    {
        return error;
    }

    if (error = cwmp_root_cause_events(cwmp))
    {
        return error;
    }

    error = pthread_create(&ubus_thread, NULL, &thread_uloop_run, NULL);
    if (error<0)
	{
		CWMP_LOG(ERROR,"Error when creating the ubus thread!");
	}
    error = pthread_create(&periodic_event_thread, NULL, &thread_event_periodic, (void *)cwmp);
    if (error<0)
    {
        CWMP_LOG(ERROR,"Error error when creating the periodic event thread!");
    }
    error = pthread_create(&scheduleInform_thread, NULL, &thread_cwmp_rpc_cpe_scheduleInform, (void *)cwmp);
    if (error<0)
    {
        CWMP_LOG(ERROR,"Error when creating the scheduled inform thread!");
    }
    error = pthread_create(&download_thread, NULL, &thread_cwmp_rpc_cpe_download, (void *)cwmp);
    if (error<0)
    {
        CWMP_LOG(ERROR,"Error when creating the download thread!");
    }
    cwmp_schedule_session (cwmp);
    pthread_join(ubus_thread, NULL);
    pthread_join(periodic_event_thread, NULL);
    pthread_join(scheduleInform_thread, NULL);
    pthread_join(download_thread, NULL);

    CWMP_LOG(INFO,"EXIT CWMP");
    return CWMP_OK;
}