/*
    AddObject.c

    cwmp service client in C

--------------------------------------------------------------------------------
cwmp service client
Copyright (C) 20011-2012, Inteno, Inc. All Rights Reserved.

Any distribution, dissemination, modification, conversion, integral or partial
reproduction, can not be made without the prior written permission of Inteno.
--------------------------------------------------------------------------------
Author contact information:

--------------------------------------------------------------------------------
*/

#include "soapH.h"
#include "cwmp.h"
#include "list.h"
#include "dm.h"
#include "dm_rpc.h"

LIST_HEAD(list_dm_add_handler);

struct rpc_cpe *cwmp_add_session_rpc_cpe (struct session *session);
struct rpc_cpe *cwmp_add_session_rpc_cpe_addObject (struct session *session);
int cwmp_rpc_cpe_addObject (struct cwmp *cwmp, struct session *session, struct rpc_cpe *this);
int cwmp_rpc_cpe_addObject_response_data_init(struct cwmp *cwmp, struct session *session, struct rpc_cpe *this);
int cwmp_rpc_cpe_addObject_response(struct cwmp *cwmp, struct session *session, struct rpc_cpe *this);
int cwmp_rpc_cpe_addObject_end(struct cwmp *cwmp, struct session *session, struct rpc_cpe *this);
int cwmp_session_rpc_cpe_destructor (struct cwmp *cwmp, struct session *session, struct rpc_cpe *rpc_cpe);
struct rpc_cpe *cwmp_add_session_rpc_cpe_Fault (struct session *session, int idx);
int cwmp_dm_addObject(struct cwmp *cwmp, struct dm_set_handler *dm_set_handler, char *path, int *InstanceNumber);
int dm_run_queue_cmd_handler_at_end_session (struct cwmp *cwmp, struct dm_set_handler *dm_set_handler);
int cwmp_reboot(struct cwmp *cwmp,void *v);
int dm_cwmp_config_reload (struct cwmp *cwmp, void *v );

struct rpc_cpe *cwmp_add_session_rpc_cpe_addObject (struct session *session)
{
    struct rpc_cpe                  *rpc_cpe;
    struct soap_cwmp1_methods__rpc  *soap_methods;

    rpc_cpe = cwmp_add_session_rpc_cpe (session);
    if (rpc_cpe == NULL)
    {
        return NULL;
    }
    soap_methods                                    = &(rpc_cpe->soap_methods);
    rpc_cpe->method_data                            = (void *) calloc (1,sizeof(struct _cwmp1__AddObject));
    rpc_cpe->method                                 = cwmp_rpc_cpe_addObject;
    rpc_cpe->method_response_data                   = (void *) calloc (1,sizeof(struct _cwmp1__AddObjectResponse));
    rpc_cpe->method_response_data_init              = cwmp_rpc_cpe_addObject_response_data_init;
    rpc_cpe->method_response                        = cwmp_rpc_cpe_addObject_response;
    rpc_cpe->method_end                             = cwmp_rpc_cpe_addObject_end;
    rpc_cpe->destructor                             = cwmp_session_rpc_cpe_destructor;
    soap_methods->envelope                          = "cwmp:AddObject";
    soap_methods->envelope_response                 = "cwmp:AddObjectResponse";
    soap_methods->soap_serialize_cwmp1__send_data   = (void *) soap_serialize__cwmp1__AddObjectResponse;
    soap_methods->soap_put_cwmp1__send_data         = (void *) soap_put__cwmp1__AddObjectResponse;
    soap_methods->soap_get_cwmp1__rpc_received_data = (void *) soap_get__cwmp1__AddObject;
    return rpc_cpe;
}

int cwmp_rpc_cpe_addObject (struct cwmp *cwmp, struct session *session, struct rpc_cpe *this)
{
    return CWMP_OK;
}

int cwmp_rpc_cpe_addObject_response_data_init(struct cwmp *cwmp, struct session *session, struct rpc_cpe *this)
{
    struct _cwmp1__AddObject            *p_soap_cwmp1__AddObject;
    struct _cwmp1__AddObjectResponse    *p_soap_cwmp1__AddObjectResponse;
    struct dm_set_handler               *dm_add_handler;
    char                                buf[128];
    int                                 error;
    int                                 InstanceNumber;

    dm_add_handler = calloc(1,sizeof(struct dm_set_handler));

    if (dm_add_handler == NULL)
    {
        return FAULT_CPE_INTERNAL_ERROR_IDX;
    }

    list_add_tail(&(dm_add_handler->list),&(list_dm_add_handler));

    INIT_LIST_HEAD (&(dm_add_handler->cmd_list));
    INIT_LIST_HEAD (&(dm_add_handler->cancel_list));
    INIT_LIST_HEAD (&(dm_add_handler->service_list));

    p_soap_cwmp1__AddObject         = (struct _cwmp1__AddObject*)this->method_data;
    p_soap_cwmp1__AddObjectResponse = (struct _cwmp1__AddObjectResponse*)this->method_response_data;

    error = cwmp_dm_addObject(cwmp, dm_add_handler, p_soap_cwmp1__AddObject->ObjectName,&InstanceNumber);

    if (error != FAULT_CPE_NO_FAULT_IDX)
    {
        dm_free_dm_set_handler_queues(dm_add_handler);
        if (dm_add_handler!=NULL)
        {
            list_del(&(dm_add_handler->list));
            free(dm_add_handler);
        }
        if (cwmp_add_session_rpc_cpe_Fault(session,error)==NULL)
        {
            return CWMP_GEN_ERR;
        }
        return CWMP_FAULT_CPE;
    }

    CWMP_LOG(INFO,"RUN uci commit"); /* TODO to be removed*/
    sprintf(buf,"%s=%s",UCI_ACS_PARAMETERKEY_PATH,p_soap_cwmp1__AddObject->ParameterKey);
    uci_set_value (buf);
    uci_commit_value();

    dm_run_queue_cmd_handler(dm_add_handler,FALSE);
    p_soap_cwmp1__AddObjectResponse->InstanceNumber = InstanceNumber;
    p_soap_cwmp1__AddObjectResponse->Status         = _cwmp1__AddObjectResponse_Status__0;

    if (dm_add_handler->reboot_required==TRUE)
    {
        CWMP_LOG(INFO,"Add reboot at the end of the session");
        add_session_end_func(session,cwmp_reboot,NULL,TRUE);
        p_soap_cwmp1__AddObjectResponse->Status = _cwmp1__AddObjectResponse_Status__1;
    }

    if (dm_add_handler->cmd_list.next!=&(dm_add_handler->cmd_list))
    {
        add_session_end_func(session,dm_run_queue_cmd_handler_at_end_session,dm_add_handler,FALSE);
        p_soap_cwmp1__AddObjectResponse->Status = _cwmp1__AddObjectResponse_Status__1;
        return CWMP_OK;
    }

    dm_free_dm_set_handler_queues(dm_add_handler);

    if (dm_add_handler!=NULL)
    {
        list_del(&(dm_add_handler->list));
        free(dm_add_handler);
    }

    return CWMP_OK;
}

int cwmp_rpc_cpe_addObject_response(struct cwmp *cwmp, struct session *session, struct rpc_cpe *this)
{
    int error;
    CWMP_LOG (INFO,"Trying to send AddObject response to the ACS");
    error = cwmp_soap_send_rpc_cpe_response (cwmp, session, this);
    return error;
}

int cwmp_rpc_cpe_addObject_end(struct cwmp *cwmp, struct session *session, struct rpc_cpe *this)
{
    return CWMP_OK;
}
