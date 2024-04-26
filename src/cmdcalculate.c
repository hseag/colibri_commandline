// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "cmdcalculate.h"
#include "printerror.h"
#include "colibri.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
 #include <math.h>

struct Meeasurment
{
    uint32_t sample230;
    uint32_t reference230;
    uint32_t sample260;
    uint32_t reference260;
    uint32_t sample280;
    uint32_t reference280;
    uint32_t sample340;
    uint32_t reference340;
};

static double calculateOD(int32_t actBlank, int32_t refBlank, int32_t actSample, int32_t refSample, double f, bool * valid)
{
  double ret = 0.0;

  double actBlank_d  = actBlank;
  double refBlank_d  = refBlank;
  double actSample_d = actSample;
  double refSample_d = refSample;

  if(!(refBlank == 0 || actSample_d == 0))
  {
    ret = log10(actBlank_d / refBlank_d * refSample_d / actSample_d * f);
  }
  else
  {
    *valid = false;
  }

  return ret;
}

static Error_t cmdCalculateOD(Colibri_t *self)
{
    struct Meeasurment meeasurmentBlank;
    struct Meeasurment meeasurmentSample;
    Error_t ret = colibriLastMeasurements(self,1, &meeasurmentBlank.sample230, &meeasurmentBlank.reference230, &meeasurmentBlank.sample260, &meeasurmentBlank.reference260, &meeasurmentBlank.sample280, &meeasurmentBlank.reference280, &meeasurmentBlank.sample340, &meeasurmentBlank.reference340);

    if(ret == ERROR_COLIBRI_OK)
    {
        ret = colibriLastMeasurements(self,0, &meeasurmentSample.sample230, &meeasurmentSample.reference230, &meeasurmentSample.sample260, &meeasurmentSample.reference260, &meeasurmentSample.sample280, &meeasurmentSample.reference280, &meeasurmentSample.sample340, &meeasurmentSample.reference340);

        if(ret == ERROR_COLIBRI_OK)
        {
            bool valid = true;
            double od230 = calculateOD(meeasurmentBlank.sample230, meeasurmentBlank.reference230, meeasurmentSample.sample230, meeasurmentSample.reference230, 1.0, &valid);
            double od260 = calculateOD(meeasurmentBlank.sample260, meeasurmentBlank.reference260, meeasurmentSample.sample260, meeasurmentSample.reference260, 1.0, &valid);
            double od280 = calculateOD(meeasurmentBlank.sample280, meeasurmentBlank.reference280, meeasurmentSample.sample280, meeasurmentSample.reference280, 1.0, &valid);
            double od340 = calculateOD(meeasurmentBlank.sample340, meeasurmentBlank.reference340, meeasurmentSample.sample340, meeasurmentSample.reference340, 1.0, &valid);

            fprintf(stdout, "%f %f %f %f\n", od230, od260, od280, od340);
        }
    }

   return ret;
}

static Error_t cmdCalculateNo340(Colibri_t *self, char * f)
{
    //#warning TBD
    struct Meeasurment meeasurmentBlank;
    struct Meeasurment meeasurmentSample;
    Error_t ret = colibriLastMeasurements(self,1, &meeasurmentBlank.sample230, &meeasurmentBlank.reference230, &meeasurmentBlank.sample260, &meeasurmentBlank.reference260, &meeasurmentBlank.sample280, &meeasurmentBlank.reference280, &meeasurmentBlank.sample340, &meeasurmentBlank.reference340);

    if(ret == ERROR_COLIBRI_OK)
    {
        ret = colibriLastMeasurements(self,0, &meeasurmentSample.sample230, &meeasurmentSample.reference230, &meeasurmentSample.sample260, &meeasurmentSample.reference260, &meeasurmentSample.sample280, &meeasurmentSample.reference280, &meeasurmentSample.sample340, &meeasurmentSample.reference340);

        if(ret == ERROR_COLIBRI_OK)
        {
            bool valid = true;
            double od230 = calculateOD(meeasurmentBlank.sample230, meeasurmentBlank.reference230, meeasurmentSample.sample230, meeasurmentSample.reference230, 1.0, &valid);
            double od260 = calculateOD(meeasurmentBlank.sample260, meeasurmentBlank.reference260, meeasurmentSample.sample260, meeasurmentSample.reference260, 1.0, &valid);
            double od280 = calculateOD(meeasurmentBlank.sample280, meeasurmentBlank.reference280, meeasurmentSample.sample280, meeasurmentSample.reference280, 1.0, &valid);
            double od340 = calculateOD(meeasurmentBlank.sample340, meeasurmentBlank.reference340, meeasurmentSample.sample340, meeasurmentSample.reference340, 1.0, &valid);

            fprintf(stdout, "%f %f %f %f\n", od230, od260, od280, od340);
        }
    }

   return ret;
}

Error_t cmdCalculate(Colibri_t *self, int argcCmd, char **argvCmd)
{
    uint32_t result = 0;
    Error_t ret = ERROR_COLIBRI_OK;

    if((argcCmd == 2) && (strcmp(argvCmd[1], "od") == 0))
    {
        ret = cmdCalculateOD(self);
    }
    else if((argcCmd == 3) && (strcmp(argvCmd[1], "no340") == 0))
    {
        ret = cmdCalculateNo340(self, argvCmd[2]);
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