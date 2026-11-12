#include "switch_net.h"

#include <stdio.h>
#include <stdlib.h>
#include <switch.h>

void ipx_shutdown(void);        /* db_ipx.c: stops the IPX client and its thread */

static int started = 0;

static void stop(void){

    ipx_shutdown();
    if(started){

        socketExit();
        started = 0;
    }
}

int SwitchNet_start(void){

    Result rc;

    if(started) return 1;

    if(R_FAILED(rc = socketInitializeDefault())){

        printf("[dRally.Network] sockets could not be started (0x%x)\n", (unsigned)rc);
        return 0;
    }

    started = 1;
    atexit(stop);
    return 1;
}
