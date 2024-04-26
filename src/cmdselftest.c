// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdselftest.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>

Error_t cmdSelftest(Colibri_t *self)
{
    uint32_t result = 0;
    Error_t ret;

    ret = colibriSelftest(self, &result);

    if (ret == ERROR_COLIBRI_OK)
    {
        if(result == 0)
        {
            fprintf(stdout, "Selftest passed.\n");
        }
        else
        {
            fprintf(stdout, "Selftest failed:\n");
            if(result & SELFTEST_ILED_230)
            {
                fprintf(stdout, "  - ILED_230\n");
            }
            if(result & SELFTEST_ILED_260)
            {
                fprintf(stdout, "  - ILED_260\n");
            }
            if(result & SELFTEST_ILED_280)
            {
                fprintf(stdout, "  - ILED_280\n");
            }
            if(result & SELFTEST_ILED_340)
            {
                fprintf(stdout, "  - ILED_340\n");
            }

            if(result & SELFTEST_SAMPLE_230)
            {
                fprintf(stdout, "  - SAMPLE_230\n");
            }
            if(result & SELFTEST_SAMPLE_260)
            {
                fprintf(stdout, "  - SAMPLE_260\n");
            }
            if(result & SELFTEST_SAMPLE_280)
            {
                fprintf(stdout, "  - SAMPLE_280\n");
            }
            if(result & SELFTEST_SAMPLE_340)
            {
                fprintf(stdout, "  - SAMPLE_340\n");
            }

            if(result & SELFTEST_REFERENCE_230)
            {
                fprintf(stdout, "  - REFERENCE_230\n");
            }
            if(result & SELFTEST_REFERENCE_260)
            {
                fprintf(stdout, "  - REFERENCE_260\n");
            }
            if(result & SELFTEST_REFERENCE_280)
            {
                fprintf(stdout, "  - REFERENCE_280\n");
            }
            if(result & SELFTEST_REFERENCE_340)
            {
                fprintf(stdout, "  - REFERENCE_340\n");
            }

            if(result & SELFTEST_REFERENCE)
            {
                fprintf(stdout, "  - REFERENCE CHANNEL AMPLIFICATION\n");
            }
            if(result & SELFTEST_SAMPLE)
            {
                fprintf(stdout, "  - SAMPLE CHANNEL AMPLIFICATION\n");
            }
        }
    }
    else
    {
        printError(ret, NULL);
    }
    return ret;
}