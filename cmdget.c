// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdget.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>

Error_t cmdGet(Colibri_t *self, const char *sIndex)
{
    char value[COLIBRI_MAX_LINE_LENGTH];
    uint32_t valueSize = COLIBRI_MAX_LINE_LENGTH;
    char *endptr;
    long index = strtol(sIndex, &endptr, 10);

    Error_t ret;

    if (*endptr == '\0')
    {
        ret = colibriGet(self, index, value, valueSize);

        if (ret == ERROR_COLIBRI_OK)
        {
            fprintf(stdout, "%s\n", value);
        }
        else
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