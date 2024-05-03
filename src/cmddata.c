// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmddata.h"
#include "printerror.h"
#include "colibri.h"
#include "colibriJson.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

typedef struct Parameters
{
    double pathLength;
    double a260Unit;
    uint32_t blanks;

} Parameters_t;

Parameters_t parametersCreate()
{
    Parameters_t ret;

    ret.pathLength = 1.0;
    ret.a260Unit = 50.0;
    ret.blanks = 1;

    return ret;
}

typedef struct
{
    double value230nm;
    double value260nm;
    double value280nm;
    double value340nm;
} Quadruple_t;

Quadruple_t quadrupleCreate(double value)
{
    Quadruple_t ret = {0};

    ret.value230nm = value;
    ret.value260nm = value;
    ret.value280nm = value;
    ret.value340nm = value;

    return ret;
}

Quadruple_t quadrupleAdd(Quadruple_t a, Quadruple_t b)
{
    Quadruple_t ret;

    ret.value230nm = a.value230nm + b.value230nm;
    ret.value260nm = a.value260nm + b.value260nm;
    ret.value280nm = a.value280nm + b.value280nm;
    ret.value340nm = a.value340nm + b.value340nm;

    return ret;
}

Quadruple_t quadrupleSub(Quadruple_t a, Quadruple_t b)
{
    Quadruple_t ret;

    ret.value230nm = a.value230nm - b.value230nm;
    ret.value260nm = a.value260nm - b.value260nm;
    ret.value280nm = a.value280nm - b.value280nm;
    ret.value340nm = a.value340nm - b.value340nm;

    return ret;
}

Quadruple_t quadrupleMul(Quadruple_t a, Quadruple_t b)
{
    Quadruple_t ret;

    ret.value230nm = a.value230nm * b.value230nm;
    ret.value260nm = a.value260nm * b.value260nm;
    ret.value280nm = a.value280nm * b.value280nm;
    ret.value340nm = a.value340nm * b.value340nm;

    return ret;
}

Quadruple_t quadrupleDiv(Quadruple_t a, Quadruple_t b)
{
    Quadruple_t ret;

    ret.value230nm = a.value230nm / b.value230nm;
    ret.value260nm = a.value260nm / b.value260nm;
    ret.value280nm = a.value280nm / b.value280nm;
    ret.value340nm = a.value340nm / b.value340nm;

    return ret;
}

typedef struct
{
    double sample;
    double reference;
} SingleMeasurement_t;

SingleMeasurement_t singleMeasurement(cJSON *obj)
{
    SingleMeasurement_t ret = {0};

    cJSON *oSample = cJSON_GetObjectItem(obj, DICT_SAMPLE);
    cJSON *oReference = cJSON_GetObjectItem(obj, DICT_REFERENCE);

    ret.sample = cJSON_GetNumberValue(oSample);
    ret.reference = cJSON_GetNumberValue(oReference);

    return ret;
}

static double calculateOneOD(cJSON *baseline, cJSON *measurement, char *key)
{
    double ret = 0.0;

    SingleMeasurement_t b = singleMeasurement(cJSON_GetObjectItem(baseline, key));
    SingleMeasurement_t m = singleMeasurement(cJSON_GetObjectItem(measurement, key));

    ret = log10(b.sample / b.reference * m.reference / m.sample);

    return ret;
}

static Quadruple_t calculateOD(cJSON *baseline, cJSON *measurement)
{
    Quadruple_t ods = quadrupleCreate(0.0);

    ods.value230nm = calculateOneOD(baseline, measurement, DICT_230);
    ods.value260nm = calculateOneOD(baseline, measurement, DICT_260);
    ods.value280nm = calculateOneOD(baseline, measurement, DICT_280);
    ods.value340nm = calculateOneOD(baseline, measurement, DICT_340);

    return ods;
}

static cJSON *calculate(cJSON *measurement, Quadruple_t factors, Parameters_t parameters)
{
    cJSON *obj = cJSON_CreateObject();

    cJSON *baseline = cJSON_GetObjectItem(measurement, DICT_BASELINE);
    cJSON *air = cJSON_GetObjectItem(measurement, DICT_AIR);
    cJSON *sample = cJSON_GetObjectItem(measurement, DICT_SAMPLE);

    Quadruple_t ods;

    if (air)
    {
        Quadruple_t odsSample = calculateOD(baseline, sample);
        Quadruple_t odsAir = calculateOD(baseline, air);
        ods = quadrupleSub(odsSample, quadrupleMul(odsAir, factors));
        cJSON_AddItemToObject(obj, DICT_CONCENTRATION, cJSON_CreateNumber(ods.value260nm * 10.0 / parameters.pathLength * parameters.a260Unit));
    }
    else
    {
        ods = calculateOD(baseline, sample);
    }

    cJSON *oOD = cJSON_CreateObject();
    cJSON_AddItemToObject(oOD, DICT_230, cJSON_CreateNumber(ods.value230nm));
    cJSON_AddItemToObject(oOD, DICT_260, cJSON_CreateNumber(ods.value260nm));
    cJSON_AddItemToObject(oOD, DICT_280, cJSON_CreateNumber(ods.value280nm));
    cJSON_AddItemToObject(oOD, DICT_340, cJSON_CreateNumber(ods.value340nm));
    cJSON_AddItemToObject(obj, DICT_OD, oOD);

    return obj;
}

static Quadruple_t calculateFactors(cJSON *oMeasurments, uint32_t nrOfBlanks)
{
    Quadruple_t factors = quadrupleCreate(0.0);

    cJSON *iterator = NULL;
    uint32_t index = 0;
    uint32_t count = 0;
    cJSON_ArrayForEach(iterator, oMeasurments)
    {
        if (index < nrOfBlanks)
        {
            cJSON *oBaseline = cJSON_GetObjectItem(iterator, DICT_BASELINE);
            cJSON *oAir = cJSON_GetObjectItem(iterator, DICT_AIR);
            cJSON *oSample = cJSON_GetObjectItem(iterator, DICT_SAMPLE);
            if (oAir)
            {
                Quadruple_t odsAir = calculateOD(oBaseline, oAir);
                Quadruple_t odsSample = calculateOD(oBaseline, oSample);
                factors = quadrupleAdd(factors, quadrupleDiv(odsSample, odsAir));
                count++;
            }
        }
        index++;
    }
    if (count == 0)
    {
        return quadrupleCreate(1.0);
    }
    else
    {
        Quadruple_t c = quadrupleCreate(count);
        return quadrupleDiv(factors, c);
    }
    return factors;
}

static Error_t cmdCalculate(Colibri_t *self, int argcCmd, char **argvCmd)
{
    Error_t ret = ERROR_COLIBRI_OK;
    Parameters_t parameters = parametersCreate();
    bool options = true;
    int i = 0;

    while (i < argcCmd && options)
    {
        if (strncmp(argvCmd[i], "--", 2) == 0 || strncmp(argvCmd[i], "-", 1) == 0)
        {
            if ((strcmp(argvCmd[i], "--pathLength") == 0) && (i + 1 < argcCmd))
            {
                i++;
                parameters.pathLength = atof(argvCmd[i]);
            }
            else if ((strcmp(argvCmd[i], "--a260unit") == 0) && (i + 1 < argcCmd))
            {
                i++;
                parameters.a260Unit = atof(argvCmd[i]);
            }
            else if ((strcmp(argvCmd[i], "--blanks") == 0) && (i + 1 < argcCmd))
            {
                i++;
                parameters.blanks = atoi(argvCmd[i]);
            }
            else
            {
                return printError(ERROR_COLIBRI_UNKOWN_COMMAND_LINE_OPTION, "Unknown option: %s\n", argvCmd[i]);
            }
            i++;
        }
        else
        {
            options = false;
        }
    }

    if (i < argcCmd)
    {
        char *file = argvCmd[i];

        cJSON *json = colibriJsonLoad(file);

        if (json != NULL)
        {
            cJSON *oMeasurments = cJSON_GetObjectItem(json, DICT_MEASUREMENTS);

            if (oMeasurments)
            {
                Quadruple_t factors = calculateFactors(oMeasurments, parameters.blanks);
                cJSON *iterator = NULL;
                cJSON_ArrayForEach(iterator, oMeasurments)
                {
                    cJSON_DeleteItemFromObject(iterator, DICT_CALCULATED);
                    cJSON_AddItemToObject(iterator, DICT_CALCULATED, calculate(iterator, factors, parameters));
                }
            }

            colibriJsonSave(file, json);

            cJSON_Delete(json);
        }
        else
        {
            ret = ERROR_COLIBRI_FILE_NOT_FOUND;
            printError(ret, "File %s not found.", file);
        }
    }
    return ret;
}

static Error_t cmdDataPrint(Colibri_t *self, char *file)
{
    cJSON *json = colibriJsonLoad(file);

    if (json != NULL)
    {
        cJSON *oMeasurments = cJSON_GetObjectItem(json, DICT_MEASUREMENTS);
        if (oMeasurments)
        {
            cJSON *iterator = NULL;
            cJSON_ArrayForEach(iterator, oMeasurments)
            {
                cJSON *oCalculated = cJSON_GetObjectItem(iterator, DICT_CALCULATED);
                cJSON *oComment = cJSON_GetObjectItem(iterator, DICT_COMMENT);
                if (oCalculated)
                {
                    cJSON *oOd = cJSON_GetObjectItem(oCalculated, DICT_OD);
                    if (oOd)
                    {
                        cJSON *o230 = cJSON_GetObjectItem(oOd, DICT_230);
                        cJSON *o260 = cJSON_GetObjectItem(oOd, DICT_260);
                        cJSON *o280 = cJSON_GetObjectItem(oOd, DICT_280);
                        cJSON *o340 = cJSON_GetObjectItem(oOd, DICT_340);

                        fprintf(stdout, "%f %f %f %f ", o230 ? cJSON_GetNumberValue(o230) : 0.0,
                                o260 ? cJSON_GetNumberValue(o260) : 0.0,
                                o280 ? cJSON_GetNumberValue(o280) : 0.0,
                                o340 ? cJSON_GetNumberValue(o340) : 0.0);
                    }

                    cJSON *oConcentration = cJSON_GetObjectItem(oCalculated, DICT_CONCENTRATION);
                    if (oConcentration)
                    {
                        fprintf(stdout, "%f ", cJSON_GetNumberValue(oConcentration));
                    }

                    if (oComment)
                    {
                        fprintf(stdout, "%s ", cJSON_GetStringValue(oComment));
                    }
                    fprintf(stdout, "\n");
                }
            }
        }
        cJSON_Delete(json);
        return ERROR_COLIBRI_OK;
    }
    else
    {
        printError(ERROR_COLIBRI_FILE_NOT_FOUND, "File %s not found.", file);
        return ERROR_COLIBRI_FILE_NOT_FOUND;
    }
}

Error_t cmdData(Colibri_t *self, int argcCmd, char **argvCmd)
{
    Error_t ret = ERROR_COLIBRI_OK;

    if ((argcCmd >= 3) && (strcmp(argvCmd[1], "calculate") == 0))
    {
        ret = cmdCalculate(self, argcCmd - 2, argvCmd + 2);
    }
    else if ((argcCmd == 3) && (strcmp(argvCmd[1], "print") == 0))
    {
        ret = cmdDataPrint(self, argvCmd[2]);
    }
    else
    {
        ret = ERROR_COLIBRI_INVALID_PARAMETER;
    }

    if (ret != ERROR_COLIBRI_OK)
    {
        printError(ret, NULL);
    }

    return ret;
}