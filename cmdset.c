// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdset.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>

Error_t cmdSet(Colibri_t *self, const char *sIndex, const char *sValue)
{
    char *endptr;
    long index = strtol(sIndex, &endptr, 10);
    Error_t ret;

    if (*endptr == '\0')
    {
        ret = colibriSet(self, index, sValue);

        if (ret != ERROR_COLIBRI_OK)
        {
            printError(ret, NULL);
        }
    }
    else
    {
        ret = ERROR_COLIBRI_INVALID_NUMBER;
        printError(ret, "'%s' is not a valid number.", sIndex);
    }
    return ret;
}