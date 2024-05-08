// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "colibri.h"
#include "crc-16-ccitt.h"
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#define VERSION_DLL "1.0.0"

typedef struct
{
	char * value;
	size_t length;
} UserGet;

typedef struct
{
	uint32_t * sample230;
	uint32_t * reference230;
	uint32_t * sample260;
	uint32_t * reference260;
	uint32_t * sample280;
	uint32_t * reference280;
	uint32_t * sample340;
	uint32_t * reference340;
} UserMeasurement;


typedef struct
{
	Levelling_t * levelling230;
	Levelling_t * levelling260;
	Levelling_t * levelling280;
	Levelling_t * levelling340;
} UserLevelling;

typedef struct
{
	uint32_t * voltageSample;
	uint32_t * voltageReference;
	uint32_t * voltage5V;
	uint32_t * current;
} UserADC;

typedef struct
{
	uint32_t * result;
} UserSelftest;

const char * colibriError2String(Error_t e)
{
	switch(e)
	{
        case ERROR_COLIBRI_OK:
		  return "OK";
        case ERROR_COLIBRI_UNKNOWN_COMMAND:
		  return "Unknown command";
        case ERROR_COLIBRI_INVALID_PARAMETER:
		  return "Invalid parameter";
        case ERROR_COLIBRI_TIMEOUT:
		  return "Timeout";
        case ERROR_COLIBRI_SREC_FLASH_WRITE_ERROR:
		  return "SREC Flash write error";
        case ERROR_COLIBRI_SREC_UNSUPPORTED_TYPE:
		  return "SREC Unsupported type";
        case ERROR_COLIBRI_SREC_INVALID_CRC:
		  return "SREC Invalid crc";
        case ERROR_COLIBRI_SREC_INVALID_STRING:
		  return "SREC Invalid string";
        case ERROR_COLIBRI_NOT_FOUND:
		  return "Colibri module not found";
		case ERROR_COLIBRI_FILE_NOT_FOUND:
		  return "Colibri file not found";
		case ERROR_COLIBRI_LEVELLING_FAILED:
		  return "Colibri levelling failed. Cuvette holder blocked?";
		default:
		  return "?";
	}
}

ColibriResponse_t *colibriCreateResponse()
{
	ColibriResponse_t *response = (ColibriResponse_t *)calloc(1, sizeof(ColibriResponse_t));
	return response;
}

void colibriFreeResponse(ColibriResponse_t *response)
{
	free(response);
}

Error_t colibriCommandComm(Colibri_t *self, HANDLE hComm, const char * command, ColibriResponse_t *response)
{
	Error_t ret = ERROR_COLIBRI_OK;

		uint32_t txSize = COLIBRI_MAX_LINE_LENGTH;
		char * tx = (char *)calloc(1, txSize);
		char s[20] = {0};
		if(self->useChecksum)
		{
			s[0] = COLIBRI_START_WITH_CHK;
			strncat_s(tx, txSize, s, 1);
			strncat_s(tx, txSize, command, strlen(command));
			s[0] = COLIBRI_CHECKSUM_SEPARATOR;
			strncat_s(tx, txSize, s, 1);
  		    crc_t crc = crc_init();
            crc = crc_update(crc, command, strlen(command));
            crc = crc_finalize(crc);
			snprintf(s, sizeof(s), "%d", (uint32_t)crc);
			strncat_s(tx, txSize, s, strlen(s));
		}
		else
		{
      	  s[0] = COLIBRI_START_NO_CHK;
		  strncat_s(tx, txSize, s, 1);
		  strncat_s(tx, txSize, command, strlen(command));
		}
		strncat_s(tx, txSize, "\n", 1);

		if (hComm != INVALID_HANDLE_VALUE)
		{
			colibriPortWrite(hComm, tx, self->verbose);
			if (colibriPortRead(hComm, response->response, COLIBRI_MAX_LINE_LENGTH, self->verbose) > 0)
			{
				int lastWasSpace;
				int isSpace;

				for (int i = 0; i < COLIBRI_MAX_ARGS; i++)
				{
					response->argv[i] = 0;
				}
				response->argc = 0;
				lastWasSpace = 1;

				char *d = response->response;

				for (int i = 0; (i < COLIBRI_MAX_LINE_LENGTH) && (d[i] != 0) && (response->argc < COLIBRI_MAX_ARGS); i++)
				{
					isSpace = isspace(d[i]);
					if (lastWasSpace != 0 && isSpace == 0)
					{
						response->argv[response->argc] = d + i;
						response->argc++;
					}
					else if (lastWasSpace == 0 && isSpace != 0)
					{
						d[i] = 0;
					}
					lastWasSpace = isSpace;
				}
				ret = ERROR_COLIBRI_OK;
			}
		}
		else
		{
			ret = ERROR_COLIBRI_NOT_FOUND;
		}

		free(tx);
return ret;
}

Error_t colibriCommand(Colibri_t *self, const char * command, ColibriResponse_t *response)
{
	char portNameBuffer[1024];
	size_t portNameBufferSize = sizeof(portNameBuffer);

	Error_t ret = ERROR_COLIBRI_OK;
	if (self->portName)
	{
		strcpy_s(portNameBuffer, portNameBufferSize, self->portName);
	}
	else
	{
		ret = colibriFindDevice(portNameBuffer, &portNameBufferSize, self->verbose);
	}

	if (ret == ERROR_COLIBRI_OK)
	{
		HANDLE hComm = colibriPortOpen(portNameBuffer);
        ret = colibriCommandComm(self, hComm, command, response);
		colibriPortClose(hComm);
	}
	else
	{
		ret = ERROR_COLIBRI_NOT_FOUND;
	}
	return ret;
}

Error_t colibriExecute(Colibri_t * self, char * cmd, Error_t(execute)(ColibriResponse_t *response, void *user), void *user)
{
	ColibriResponse_t *response = colibriCreateResponse();
	Error_t ret = colibriCommand(self, cmd, response);
	if (ret == ERROR_COLIBRI_OK)
	{
		if (strncmp(response->argv[0], cmd, 1) == 0)
		{
			ret = execute(response, user);
		}
		else
		{
			if(response->argc == 2 && strncmp(response->argv[0], "E", 1) == 0)
			{
			  ret = atoi(response->argv[1]);			  
			}
			else
			{
			  ret = ERROR_COLIBRI_RESPONSE_ERROR;
			}
		}
	}
	colibriFreeResponse(response);
	return ret;
}

Error_t colibriNoReturn_(ColibriResponse_t *response, void *user)
{
	if (response->argc == 1)
	{
		return ERROR_COLIBRI_OK;
	}
	else
	{
		return ERROR_COLIBRI_PROTOCOL_ERROR;
	}
}


Error_t colibriGet_(ColibriResponse_t *response, void *user)
{
	UserGet *u = (UserGet *)user;
	if (response->argc == 2)
	{
		strncpy_s(u->value, u->length, response->argv[1], u->length);
		return ERROR_COLIBRI_OK;
	}
	else
	{
		return ERROR_COLIBRI_PROTOCOL_ERROR;
	}
}

Error_t colibriGet(Colibri_t * self, uint32_t index, char * value, size_t valueSize)
{
	char cmd[30];
	UserGet user = { 0 };
	user.value = value;
	user.length = valueSize;
	sprintf(cmd, "V %i", index);
	return colibriExecute(self, cmd, colibriGet_, &user);
}

Error_t colibriSet(Colibri_t * self, uint32_t index, const char * value)
{
	char cmd[COLIBRI_MAX_LINE_LENGTH];
	sprintf(cmd, "V %i %s", index, value);
	return colibriExecute(self, cmd, colibriNoReturn_, 0);
}

Error_t colibriMeasure_(ColibriResponse_t *response, void *user)
{
	UserMeasurement *u = (UserMeasurement *)user;
	if (response->argc == 9)
	{
		*(u->sample230)    = atoi(response->argv[1]);
		*(u->reference230) = atoi(response->argv[2]);
		*(u->sample260)    = atoi(response->argv[3]);
		*(u->reference260) = atoi(response->argv[4]);
		*(u->sample280)    = atoi(response->argv[5]);
		*(u->reference280) = atoi(response->argv[6]);
		*(u->sample340)    = atoi(response->argv[7]);
		*(u->reference340) = atoi(response->argv[8]);
		return ERROR_COLIBRI_OK;
	}
	else
	{
		return ERROR_COLIBRI_PROTOCOL_ERROR;
	}
}

Error_t colibriMeasure(Colibri_t * self, uint32_t * sample230, uint32_t * reference230, uint32_t * sample260, uint32_t * reference260, uint32_t * sample280, uint32_t * reference280, uint32_t * sample340, uint32_t * reference340)
{
	UserMeasurement user = {sample230 = sample230, reference230 = reference230, sample260 = sample260, reference260 = reference260, sample280 = sample280, reference280 = reference280, sample340 = sample340, reference340 = reference340};
	return colibriExecute(self, "M", colibriMeasure_, &user);
}

Error_t colibriLastMeasurements(Colibri_t * self, uint32_t last, uint32_t * sample230, uint32_t * reference230, uint32_t * sample260, uint32_t * reference260, uint32_t * sample280, uint32_t * reference280, uint32_t * sample340, uint32_t * reference340)
{
	UserMeasurement user = {sample230 = sample230, reference230 = reference230, sample260 = sample260, reference260 = reference260, sample280 = sample280, reference280 = reference280, sample340 = sample340, reference340 = reference340};
	char cmd[COLIBRI_MAX_LINE_LENGTH];
	sprintf(cmd, "M %i", last);
	return colibriExecute(self, cmd, colibriMeasure_, &user);
}

Error_t colibriLevelling_(ColibriResponse_t *response, void *user)
{
	UserLevelling *u = (UserLevelling *)user;
	if (response->argc == 17)
	{
		u->levelling230->result                 = atoi(response->argv[1]);
		u->levelling230->current                = atoi(response->argv[2]);
		u->levelling230->amplificationSample    = atoi(response->argv[3]);
		u->levelling230->amplificationReference = atoi(response->argv[4]);

		u->levelling260->result                 = atoi(response->argv[5]);
		u->levelling260->current                = atoi(response->argv[6]);
		u->levelling260->amplificationSample    = atoi(response->argv[7]);
		u->levelling260->amplificationReference = atoi(response->argv[8]);

		u->levelling280->result                 = atoi(response->argv[9]);
		u->levelling280->current                = atoi(response->argv[10]);
		u->levelling280->amplificationSample    = atoi(response->argv[11]);
		u->levelling280->amplificationReference = atoi(response->argv[12]);

		u->levelling340->result                 = atoi(response->argv[13]);
		u->levelling340->current                = atoi(response->argv[14]);
		u->levelling340->amplificationSample    = atoi(response->argv[15]);
		u->levelling340->amplificationReference = atoi(response->argv[16]);

		return ERROR_COLIBRI_OK;
	}
	else
	{
		return ERROR_COLIBRI_PROTOCOL_ERROR;
	}
}

Error_t colibriLevelling(Colibri_t * self, Levelling_t * levelling230, Levelling_t * levelling260, Levelling_t * levelling280, Levelling_t * levelling340)
{
	UserLevelling user = {levelling230 = levelling230, levelling260 = levelling260, levelling280 = levelling280, levelling340 = levelling340};
	return colibriExecute(self, "C", colibriLevelling_, &user);
}

Error_t colibriLastLevelling(Colibri_t * self, Levelling_t * levelling230, Levelling_t * levelling260, Levelling_t * levelling280, Levelling_t * levelling340)
{
	UserLevelling user = {levelling230 = levelling230, levelling260 = levelling260, levelling280 = levelling280, levelling340 = levelling340};
	return colibriExecute(self, "C 0", colibriLevelling_, &user);
}

Error_t colibriBaseline_(ColibriResponse_t *response, void *user)
{
	UserMeasurement *u = (UserMeasurement *)user;
	if (response->argc == 9)
	{
		*(u->sample230)    = atoi(response->argv[1]);
		*(u->reference230) = atoi(response->argv[2]);
		*(u->sample260)    = atoi(response->argv[3]);
		*(u->reference260) = atoi(response->argv[4]);
		*(u->sample280)    = atoi(response->argv[5]);
		*(u->reference280) = atoi(response->argv[6]);
		*(u->sample340)    = atoi(response->argv[7]);
		*(u->reference340) = atoi(response->argv[8]);
		return ERROR_COLIBRI_OK;
	}
	else
	{
		return ERROR_COLIBRI_PROTOCOL_ERROR;
	}
}

Error_t colibriBaseline(Colibri_t * self, uint32_t * sample230, uint32_t * reference230, uint32_t * sample260, uint32_t * reference260, uint32_t * sample280, uint32_t * reference280, uint32_t * sample340, uint32_t * reference340)
{
	UserMeasurement user = {sample230 = sample230, reference230 = reference230, sample260 = sample260, reference260 = reference260, sample280 = sample280, reference280 = reference280, sample340 = sample340, reference340 = reference340};
	return colibriExecute(self, "G", colibriBaseline_, &user);
}

Error_t colibriSelftest_(ColibriResponse_t *response, void *user)
{
	UserSelftest *u = (UserSelftest *)user;
	if (response->argc == 2)
	{
		*(u->result)    = strtoul(response->argv[1], 0, 10);
		return ERROR_COLIBRI_OK;
	}
	else
	{
		return ERROR_COLIBRI_PROTOCOL_ERROR;
	}
}

Error_t colibriSelftest(Colibri_t * self, uint32_t * result)
{
	UserSelftest user = {result = result};
	return colibriExecute(self, "Y", colibriSelftest_, &user);
}

static int getlineInternal(char **lineptr, size_t *n, FILE *stream) {
    char *bufptr = NULL;
    char *p = bufptr;
    size_t size;
    int c;

    if (lineptr == NULL) {
        return -1;
    }
    if (stream == NULL) {
        return -1;
    }
    if (n == NULL) {
        return -1;
    }
    bufptr = *lineptr;
    size = *n;

    c = fgetc(stream);
    if (c == EOF) {
        return -1;
    }
    if (bufptr == NULL) {
        bufptr = malloc(128);
        if (bufptr == NULL) {
            return -1;
        }
        size = 128;
    }
    p = bufptr;
    while(c != EOF) {
        if ((p - bufptr) > (size - 1)) {
            size = size + 128;
            bufptr = realloc(bufptr, size);
            if (bufptr == NULL) {
                return -1;
            }
        }
        *p++ = c;
        if (c == '\n') {
            break;
        }
        c = fgetc(stream);
    }

    *p++ = '\0';
    *lineptr = bufptr;
    *n = size;

    return p - bufptr - 1;
}

Error_t colibriFwUpdate(Colibri_t * self, const char * file)
{
	Error_t ret = ERROR_COLIBRI_OK;
	FILE * f = fopen(file, "r");
	if(f != NULL)
	{
		size_t n;
		char * line = NULL;
		int length = 0;
		Error_t ret;
		char cmd[255];
	    char portNameBuffer[1024];
	    size_t portNameBufferSize = sizeof(portNameBuffer);
        
		if(self->portName == 0)
		{
          ret = colibriFindDevice(portNameBuffer, &portNameBufferSize, self->verbose);
		  self->portName = portNameBuffer;
		  if(ret != ERROR_COLIBRI_OK)
		  {
			return ret;
		  }
		}

		ColibriResponse_t *response = colibriCreateResponse();
		HANDLE hComm = colibriPortOpen(self->portName);
        ret = colibriCommandComm(self, hComm, "F", response);
		do
		{
		   length = getlineInternal(&line, &n, f);
		   if(length != -1)
		   {
		     snprintf(cmd, sizeof(cmd), "S %s", line);
			 ret = colibriCommandComm(self, hComm, cmd, response);
		   }
		}
		while(length != -1 && ret == ERROR_COLIBRI_OK);

		ret = colibriCommandComm(self, hComm, "R", response);

		Sleep(5000);

		free(line);
		fclose(f);
		colibriFreeResponse(response);
		colibriPortClose(hComm);
	}
	else
	{
		ret = ERROR_COLIBRI_FILE_NOT_FOUND;
	}

	return ret;
}

const char * colibriVersion()
{
	return VERSION_DLL;
}