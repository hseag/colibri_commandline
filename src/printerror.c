// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "printerror.h"
#include <stdlib.h>
#include <stdio.h>

Error_t printError(Error_t error, char * format, ...)
{
    if(format)
    {
      va_list args;
      fprintf(stderr, "Colibri error (%i): ", error);
      va_start(args, format);
      vfprintf(stderr, format, args);
      va_end(args);
    }
    else
    {
      fprintf(stderr, "Colibri error (%i): %s", error, colibriError2String(error));
    }
    return error;
}