// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdsave.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>

static char * amlification2text(uint32_t value)
{
    switch(value)
    {
        case 0: return "1.1";
        case 1: return "11.0";
        case 2: return "111.0";
        default: return "?";
    }
}

static char * result2text(uint32_t value)
{
    switch(value)
    {
        case SETUPRESULT_OK: return "OK";
        case SETUPRESULT_AMPLIFICATION_NOT_FOUND: return "No amplification found";
        case SETUPRESULT_REFERENCE_CHANNEL_TOOH_IGH: return "Reference channel too high";
        case SETUPRESULT_SAMPLE_CHANNEL_TOO_HIGH: return "Sample channel too high";
        default: return "?";
    }
}

Error_t cmdSave(Colibri_t *self, int argcCmd, char **argvCmd)
{
    uint32_t result = 0;
    Error_t ret = ERROR_COLIBRI_OK;
    uint32_t lastMeasurementsCount = 0;
    char value[100];
    FILE *f = stdout;

    if (argcCmd == 1)
    {
        char file[200];
        ret = colibriGet(self, INDEX_SERIALNUMBER, value, sizeof(value));
        if (ret == ERROR_COLIBRI_OK)
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            snprintf(file, sizeof(file), "%s-%d-%02d-%02d_%02d-%02d-%02d.csv", value, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            f = fopen(file, "w+");
            if (f == NULL)
            {
                return ERROR_COLIBRI_FILE_ERROR;
            }
        }
        else
        {
            return ret;
        }
    }
    else if (argcCmd == 2 && strcmp(argvCmd[1], "-") == 0)
    {
        // Stdout, do nothing
    }
    else if (argcCmd == 2)
    {
        f = fopen(argvCmd[1], "w+");
        if (f == NULL)
        {
            return ERROR_COLIBRI_FILE_ERROR;
        }
    }
    else
    {
        printError(ERROR_COLIBRI_UNKOWN_COMMAND_LINE_ARGUMENT, NULL);
        return ERROR_COLIBRI_UNKOWN_COMMAND_LINE_ARGUMENT;
    }

    uint32_t sample230;
    uint32_t reference230;
    uint32_t sample260;
    uint32_t reference260;
    uint32_t sample280;
    uint32_t reference280;
    uint32_t sample340;
    uint32_t reference340;

    Levelling_t levelling230;
    Levelling_t levelling260;
    Levelling_t levelling280;
    Levelling_t levelling340;

    ret = colibriGet(self, INDEX_LAST_MEASUREMENT_COUNT, value, sizeof(value));
    if(ret != ERROR_COLIBRI_OK)
    {
        goto exit;
    }

    ret = colibriLastLevelling(self, &levelling230, &levelling260, &levelling280, &levelling340);
    if(ret != ERROR_COLIBRI_OK)
    {
        goto exit;
    }

    fprintf(f, "%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s;%s\n", "Result 230nm", "Current 230nm [uA]", "Sample amplification 230nm", "Reference amplification 230nm"
                                                                  , "Result 260nm", "Current 260nm [uA]", "Sample amplification 260nm", "Reference amplification 260nm"
                                                                  , "Result 280nm", "Current 280nm [uA]", "Sample amplification 280nm", "Reference amplification 280nm"
                                                                  , "Result 340nm", "Current 340nm [uA]", "Sample amplification 340nm", "Reference amplification 340nm");
    fprintf(f, "%s;%i;%s;%s;%s;%i;%s;%s;%s;%i;%s;%s;%s;%i;%s;%s\n",
            result2text(levelling230.result), levelling230.current, amlification2text(levelling230.amplificationSample), amlification2text(levelling230.amplificationReference),
            result2text(levelling260.result), levelling260.current, amlification2text(levelling260.amplificationSample), amlification2text(levelling260.amplificationReference),
            result2text(levelling280.result), levelling280.current, amlification2text(levelling280.amplificationSample), amlification2text(levelling280.amplificationReference),
            result2text(levelling340.result), levelling340.current, amlification2text(levelling340.amplificationSample), amlification2text(levelling340.amplificationReference));

    lastMeasurementsCount = atoi(value);
    fprintf(f, "%s;%s;%s;%s;%s;%s;%s;%s\n", "Sample channel 230nm [uV]", "Reference channel 230nm [uV]", "Sample channel 260nm [uV]", "Reference channel 260nm [uV]", "Sample channel 280nm [uV]", "Reference channel 280nm [uV]", "Sample channel 340nm [uV]", "Reference channel 340nm [uV]");
    for (int i = lastMeasurementsCount - 1; i >= 0; i--)
    {
        ret = colibriLastMeasurements(self, i, &sample230, &reference230, &sample260, &reference260, &sample280, &reference280, &sample340, &reference340);
       if(ret == ERROR_COLIBRI_OK)
       {
         fprintf(f, "%d;%d;%d;%d;%d;%d;%d;%d\n", sample230, reference230, sample260, reference260, sample280, reference280, sample340, reference340);
       }
    }

exit:
    if (f != stdout)
    {
        fclose(f);
    }

    return ret;
}