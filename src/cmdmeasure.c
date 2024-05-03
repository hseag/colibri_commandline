// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdmeasure.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>

Error_t cmdMeasure(Colibri_t * self)
{
    uint32_t sample230 = 0;
    uint32_t reference230 = 0;
    uint32_t sample260 = 0;
    uint32_t reference260 = 0;
    uint32_t sample280 = 0;
    uint32_t reference280 = 0;
    uint32_t sample340 = 0;
    uint32_t reference340 = 0;

    Error_t ret = colibriMeasure(self, &sample230, &reference230, &sample260, &reference260, &sample280, &reference280, &sample340, &reference340);
    if (ret == ERROR_COLIBRI_OK)
    {
        fprintf(stdout, "%i %i %i %i %i %i %i %i\n", sample230, reference230, sample260, reference260, sample280, reference280, sample340, reference340);
    }
    else
    {
        printError(ret, NULL);
    }
    return ret;
}