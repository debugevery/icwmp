/*
    cwmp_lib.c

    cwmp service client in C

--------------------------------------------------------------------------------
cwmp service client
Copyright (C) 2011-2012, Inteno, Inc. All Rights Reserved.

Any distribution, dissemination, modification, conversion, integral or partial
reproduction, can not be made without the prior written permission of Inteno.
--------------------------------------------------------------------------------
Author contact information:

--------------------------------------------------------------------------------
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdarg.h>
#include "cwmp_lib.h"

int lib_api_cwmp_value_change_call (int count, ...)
{
    FILE                *fp;
    register int        i, s, len;
    struct sockaddr_un  saun;
    va_list             args;
    char                *str,buf[256];


    if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        printf("[CWMP] Error when creating process from other process: Quit\r\n");
        return 1;
    }

    saun.sun_family = AF_UNIX;
    strcpy(saun.sun_path, AF_UNIX_ADDRESS);

    len = sizeof(saun.sun_family) + strlen(saun.sun_path);

    if (connect(s, (struct sockaddr *) &saun, len) < 0)
    {
        printf("[CWMP] Error when creating socket from other process: Quit\r\n");
        return 1;
    }

    va_start(args, count);
    for (i=0;i<count;i++)
    {
        str = (char *) va_arg(args, char *);
        if (str==NULL)
        {
            str = AF_UNIX_NULL;
        }
        sprintf(buf,"%s\n",str);
        str = buf;
        send(s, str, strlen(str), 0);
    }
    va_end(args);

    sprintf(buf,"%s\n",AF_UNIX_END_DATA);
    send(s, buf, strlen(buf), 0);

    close(s);

    return 0;
}
