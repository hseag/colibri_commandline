// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdfwupdate.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>

Error_t cmdFwUpdate(Colibri_t *self, const char * file)
{
    Error_t ret;
    ret = colibriFwUpdate(self, file);

    if (ret != ERROR_COLIBRI_OK)
    {
        printError(ret, NULL);
    }
    return ret;
}