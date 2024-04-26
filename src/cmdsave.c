// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdsave.h"
#include "cJSON.h"
#include "colibri.h"
#include "printerror.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

#define MEASUREMENTS "measurements"
#define SERIALNUMBER "serialnumber"
#define FIRMWAREVERSION "firmwareVersion"

#define SM_SAMPLE "sample"
#define SM_REFERENCE "reference"

#define LV_AMPLIFICATION_SAMPLE "amplificationSample"
#define LV_AMPLIFICATION_REFERENCE "amplificationReference"
#define LV_CURRENT "current"
#define LV_RESULT "result"
#define LV_RESULT_TEXT "resultText"

#define M_LEVELLING "levelling"
#define M_BASELINE "baseline"
#define M_AIR "air"
#define M_SAMPLE "sample"
#define M_COMMENT "comment"

#define L230 "230"
#define L260 "260"
#define L280 "280"
#define L340 "340"

static double amplification2number(Colibri_t* self, bool isSample, uint32_t value)
{
    int  index;
    char buffer[100];

    if (isSample)
    {
        switch (value)
        {
            case 0:
                index = INDEX_AMPLIFIER_SAMPLEFACTOR___1_1;
                break;
            case 1:
                index = INDEX_AMPLIFIER_SAMPLEFACTOR__11_0;
                break;
            case 2:
                index = INDEX_AMPLIFIER_SAMPLEFACTOR_111_0;
                break;
            default:
                return 0.0;
        }
    }
    else
    {
        switch (value)
        {
            case 0:
                index = INDEX_AMPLIFIER_REFERENCEFACTOR___1_1;
                break;
            case 1:
                index = INDEX_AMPLIFIER_REFERENCEFACTOR__11_0;
                break;
            case 2:
                index = INDEX_AMPLIFIER_REFERENCEFACTOR_111_0;
                break;
            default:
                return 0.0;
        }
    }

    if (colibriGet(self, index, buffer, sizeof(buffer)) != ERROR_COLIBRI_OK)
    {
        return 0.0;
    }

    return atof(buffer);
}

static char* result2text(uint32_t value)
{
    switch (value)
    {
        case SETUPRESULT_OK:
            return "OK";
        case SETUPRESULT_AMPLIFICATION_NOT_FOUND:
            return "No amplification found";
        case SETUPRESULT_REFERENCE_CHANNEL_TOOH_IGH:
            return "Reference channel too high";
        case SETUPRESULT_SAMPLE_CHANNEL_TOO_HIGH:
            return "Sample channel too high";
        default:
            return "?";
    }
}

static cJSON* channelObject(uint32_t sample, uint32_t reference)
{
    cJSON* obj        = cJSON_CreateObject();
    cJSON* oSample    = cJSON_CreateNumber(sample);
    cJSON* oReference = cJSON_CreateNumber(reference);

    cJSON_AddItemToObject(obj, SM_SAMPLE, oSample);
    cJSON_AddItemToObject(obj, SM_REFERENCE, oReference);

    return obj;
}

static cJSON* measuremtObject(uint32_t sample230, uint32_t reference230, uint32_t sample260, uint32_t reference260, uint32_t sample280, uint32_t reference280, uint32_t sample340, uint32_t reference340)
{
    cJSON* obj = cJSON_CreateObject();

    cJSON_AddItemToObject(obj, "230", channelObject(sample230, reference230));
    cJSON_AddItemToObject(obj, "260", channelObject(sample260, reference260));
    cJSON_AddItemToObject(obj, "280", channelObject(sample280, reference280));
    cJSON_AddItemToObject(obj, "340", channelObject(sample340, reference340));

    return obj;
}

static Error_t addSingleMeasurement(Colibri_t* self, char* key, int index, cJSON* obj)
{
    Error_t  ret = ERROR_COLIBRI_OK;
    uint32_t sample230;
    uint32_t reference230;
    uint32_t sample260;
    uint32_t reference260;
    uint32_t sample280;
    uint32_t reference280;
    uint32_t sample340;
    uint32_t reference340;

    ret = colibriLastMeasurements(self, index, &sample230, &reference230, &sample260, &reference260, &sample280, &reference280, &sample340, &reference340);
    if (ret == ERROR_COLIBRI_OK)
    {
        cJSON_AddItemToObject(obj, key, measuremtObject(sample230, reference230, sample260, reference260, sample280, reference280, sample340, reference340));
        return ret;
    }
    else
    {
        printError(ret, "Could not read measurement");
        return ret;
    }
}

static cJSON* levellingChannel(Colibri_t* self, Levelling_t levelling)
{
    cJSON* obj = cJSON_CreateObject();

    cJSON_AddItemToObject(obj, LV_AMPLIFICATION_SAMPLE, cJSON_CreateNumber(amplification2number(self, true, levelling.amplificationSample)));
    cJSON_AddItemToObject(obj, LV_AMPLIFICATION_REFERENCE, cJSON_CreateNumber(amplification2number(self, false, levelling.amplificationReference)));
    cJSON_AddItemToObject(obj, LV_CURRENT, cJSON_CreateNumber(levelling.current));
    cJSON_AddItemToObject(obj, LV_RESULT, cJSON_CreateNumber(levelling.result));
    cJSON_AddItemToObject(obj, LV_RESULT_TEXT, cJSON_CreateString(result2text(levelling.result)));

    return obj;
}

static Error_t addLevelling(Colibri_t* self, cJSON* obj)
{
    Error_t     ret = ERROR_COLIBRI_OK;
    Levelling_t levelling230;
    Levelling_t levelling260;
    Levelling_t levelling280;
    Levelling_t levelling340;

    ret = colibriLastLevelling(self, &levelling230, &levelling260, &levelling280, &levelling340);
    if (ret != ERROR_COLIBRI_OK)
    {
        printError(ret, "Could not read levelling info");
        return ret;
    }

    cJSON* objLevelling = cJSON_CreateObject();

    cJSON_AddItemToObject(objLevelling, L230, levellingChannel(self, levelling230));
    cJSON_AddItemToObject(objLevelling, L260, levellingChannel(self, levelling260));
    cJSON_AddItemToObject(objLevelling, L280, levellingChannel(self, levelling280));
    cJSON_AddItemToObject(objLevelling, L340, levellingChannel(self, levelling340));

    cJSON_AddItemToObject(obj, M_LEVELLING, objLevelling);

    return ERROR_COLIBRI_OK;
}

static Error_t addMeasurement(Colibri_t* self, int argcCmd, char** argvCmd, cJSON* json)
{
    Error_t ret = ERROR_COLIBRI_OK;
    char    value[20];

    cJSON* oMeasurements = cJSON_GetObjectItem(json, MEASUREMENTS);

    cJSON* obj = cJSON_CreateObject();

    if (argcCmd == 3)
    {
        cJSON_AddItemToObject(obj, M_COMMENT, cJSON_CreateString(argvCmd[2]));
    }

    ret = colibriGet(self, INDEX_LAST_MEASUREMENT_COUNT, value, sizeof(value));
    if (ret != ERROR_COLIBRI_OK)
    {
        printError(ret, "Could not read measurement count");
        return ret;
    }

    ret = addLevelling(self, obj);
    if (ret != ERROR_COLIBRI_OK)
        return ret;

    int lastMeasurementsCount = atoi(value);
    if (lastMeasurementsCount == 2)
    {
        ret = addSingleMeasurement(self, M_BASELINE, 1, obj);
        if (ret != ERROR_COLIBRI_OK)
            return ret;

        ret = addSingleMeasurement(self, M_SAMPLE, 0, obj);
        if (ret != ERROR_COLIBRI_OK)
            return ret;
    }
    else if (lastMeasurementsCount == 3)
    {
        ret = addSingleMeasurement(self, M_BASELINE, 2, obj);
        if (ret != ERROR_COLIBRI_OK)
            return ret;

        ret = addSingleMeasurement(self, M_AIR, 1, obj);
        if (ret != ERROR_COLIBRI_OK)
            return ret;

        ret = addSingleMeasurement(self, M_SAMPLE, 0, obj);
        if (ret != ERROR_COLIBRI_OK)
            return ret;
    }
    else
    {
        fprintf(stderr, "Colibri error : Expected 2 or 3 measurements");
        return ERROR_COLIBRI_NUMBER_OF_MEASUREMENTS;
    }

    cJSON_AddItemToArray(oMeasurements, obj);

    return ERROR_COLIBRI_OK;
}

static cJSON* loadJson(Colibri_t* self, int argcCmd, char** argvCmd)
{
    FILE*  fin    = 0;
    char*  buffer = NULL;
    cJSON* json   = NULL;
    char*  f      = argvCmd[1];

    fin = fopen(f, "rb");

    if (fin)
    {
        struct stat st;
        stat(argvCmd[1], &st);

        buffer = malloc(st.st_size);

        size_t ret = fread(buffer, 1, st.st_size, fin);

        if (ret == st.st_size)
        {
            json = cJSON_Parse(buffer);
        }

        fclose(fin);
        free(buffer);
    }

    // create new JSON file
    if (json == NULL)
    {
        Error_t ret = ERROR_COLIBRI_OK;
        char    value[100];

        json = cJSON_CreateObject();

        ret = colibriGet(self, INDEX_SERIALNUMBER, value, sizeof(value));
        if (ret == ERROR_COLIBRI_OK)
        {
            cJSON_AddItemToObject(json, SERIALNUMBER, cJSON_CreateString(value));
        }

        ret = colibriGet(self, INDEX_VERSION, value, sizeof(value));
        if (ret == ERROR_COLIBRI_OK)
        {
            cJSON_AddItemToObject(json, FIRMWAREVERSION, cJSON_CreateString(value));
        }

        cJSON_AddItemToObject(json, MEASUREMENTS, cJSON_CreateArray());
    }

    return json;
}

static void saveJson(Colibri_t* self, int argcCmd, char** argvCmd, cJSON* json)
{
    FILE* fout   = 0;
    char* buffer = NULL;

    fout = fopen(argvCmd[1], "w+");

    buffer = cJSON_Print(json);

    fwrite(buffer, strlen(buffer), 1, fout);

    free(buffer);

    fclose(fout);
}

Error_t cmdSave(Colibri_t* self, int argcCmd, char** argvCmd)
{
    cJSON*  json = NULL;
    Error_t ret  = ERROR_COLIBRI_OK;

    if (argcCmd == 2 || argcCmd == 3)
    {
        json = loadJson(self, argcCmd, argvCmd);
        ret  = addMeasurement(self, argcCmd, argvCmd, json);
        if (ret == ERROR_COLIBRI_OK)
        {
            saveJson(self, argcCmd, argvCmd, json);
        }
    }
    else
    {
        ret = ERROR_COLIBRI_UNKOWN_COMMAND_LINE_ARGUMENT;
        printError(ret, NULL);
    }

    if (json)
        cJSON_Delete(json);

    return ret;
}
