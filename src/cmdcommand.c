// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdcommand.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>

Error_t cmdCommand(Colibri_t * self, const char * command)
{   
    ColibriResponse_t * response = colibriCreateResponse();
    Error_t ret = colibriCommand(self, command, response);

    if (ret == ERROR_COLIBRI_OK)
    {
        for(uint32_t i=0; i<response->argc; i++)
        {
            fprintf(stdout, "%s%s", i==0 ? "":" ", response->argv[i]);
        }
        fprintf(stdout, "\n");
        
    }
    else
    {
        printError(ret, NULL);
    }

    colibriFreeResponse(response);
    return ret;
}