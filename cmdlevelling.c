// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdlevelling.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>

Error_t cmdLevelling(Colibri_t * self)
{
    Levelling_t levelling230 = {0};
    Levelling_t levelling260 = {0};
    Levelling_t levelling280 = {0};
    Levelling_t levelling340 = {0};

    Error_t ret = colibriLevelling (self, &levelling230, &levelling260, &levelling280, &levelling340);
    if (ret == ERROR_COLIBRI_OK)
    {
          fprintf(stdout, "%i %i %i %i %i %i %i %i %i %i %i %i %i %i %i %i\n", 
                                                     levelling230.result, levelling230.current, levelling230.amplificationSample, levelling230.amplificationReference, 
                                                     levelling260.result, levelling260.current, levelling260.amplificationSample, levelling260.amplificationReference,
                                                     levelling280.result, levelling280.current, levelling280.amplificationSample, levelling280.amplificationReference,
                                                     levelling340.result, levelling340.current, levelling340.amplificationSample, levelling340.amplificationReference);

        if(levelling230.result == 0 && levelling260.result == 0 && levelling280.result == 0 && levelling340.result == 0)
        {
        }
        else
        {
            ret = ERROR_COLIBRI_LEVELLING_FAILED;
            printError(ret, NULL);
        }
    }
    else
    {
        printError(ret, NULL);
    }
    return ret;
}