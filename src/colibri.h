// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#define DLLEXPORT __declspec(dllexport)
#else
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdarg.h>
#define DLLEXPORT
typedef int HANDLE;
typedef int errno_t;
typedef size_t rsize_t;
#define INVALID_HANDLE_VALUE -1
extern errno_t strncat_s(char *restrict dest, rsize_t destsz, const char *restrict src, rsize_t count);
extern errno_t strncpy_s(char *restrict dest, rsize_t destsz, const char *restrict src, rsize_t count);
extern errno_t strcpy_s(char *restrict dest, rsize_t destsz, const char *restrict src);
extern void Sleep(uint32_t dwMilliseconds);
#endif

#define COLIBRI_MAX_LINE_LENGTH 255
#define COLIBRI_MAX_ARGS 20
#define COLIBRI_START_NO_CHK ':'
#define COLIBRI_START_WITH_CHK ';'
#define COLIBRI_CHECKSUM_SEPARATOR '@'
#define COLIBRI_STOP1 '\n'
#define COLIBRI_STOP2 '\r'

typedef struct
{
    uint32_t argc;
    char *argv[COLIBRI_MAX_ARGS];
    char response[COLIBRI_MAX_LINE_LENGTH];
} ColibriResponse_t;

typedef struct
{
    bool verbose;
    char *portName;
    bool useChecksum;
} Colibri_t;

typedef struct
{
    uint32_t result;
    uint32_t current;
    uint32_t amplificationSample;
    uint32_t amplificationReference;
} Levelling_t;

typedef enum
{
    ERROR_COLIBRI_OK = 0,
    ERROR_COLIBRI_UNKNOWN_COMMAND = 1,
    ERROR_COLIBRI_INVALID_PARAMETER = 2,
    ERROR_COLIBRI_TIMEOUT = 3,
    ERROR_COLIBRI_SREC_FLASH_WRITE_ERROR = 4,
    ERROR_COLIBRI_SREC_UNSUPPORTED_TYPE = 5,
    ERROR_COLIBRI_SREC_INVALID_CRC = 6,
    ERROR_COLIBRI_SREC_INVALID_STRING = 7,
    ERROR_COLIBRI_LEVELLING_FAILED = 8,
    ERROR_COLIBRI_RESPONSE_ERROR = 100,
    ERROR_COLIBRI_PROTOCOL_ERROR = 101,
    ERROR_COLIBRI_UNKOWN_COMMAND_LINE_ARGUMENT = 200,
    ERROR_COLIBRI_UNKOWN_COMMAND_LINE_OPTION = 201,
    ERROR_COLIBRI_NOT_FOUND = 202,
    ERROR_COLIBRI_INVALID_NUMBER = 203,
    ERROR_COLIBRI_FILE_NOT_FOUND = 204,    
    ERROR_COLIBRI_NUMBER_OF_MEASUREMENTS = 207,
} Error_t;

typedef enum
{
    INDEX_VERSION = 0,
    INDEX_SERIALNUMBER = 1,
    INDEX_HARDWARETYPE = 2,

    INDEX_LAST_MEASUREMENT_COUNT = 10,

    INDEX_LED230NM_MAX_CURRENT = 23,
    INDEX_LED260NM_MAX_CURRENT = 33,
    INDEX_LED280NM_MAX_CURRENT = 43,
    INDEX_LED340NM_MAX_CURRENT = 53,

    INDEX_AMPLIFIER_SAMPLEFACTOR___1_1 = 60,
    INDEX_AMPLIFIER_SAMPLEFACTOR__11_0 = 61,
    INDEX_AMPLIFIER_SAMPLEFACTOR_111_0 = 62,
    INDEX_AMPLIFIER_REFERENCEFACTOR___1_1 = 63,
    INDEX_AMPLIFIER_REFERENCEFACTOR__11_0 = 64,
    INDEX_AMPLIFIER_REFERENCEFACTOR_111_0 = 65,

    INDEX_SETUP_TARGET230 = 80,
    INDEX_SETUP_TARGET260 = 81,
    INDEX_SETUP_TARGET280 = 82,
    INDEX_SETUP_TARGET340 = 83,
} Index_t;

typedef enum
{
    SETUPRESULT_OK = 0,
    SETUPRESULT_AMPLIFICATION_NOT_FOUND = 1,
    SETUPRESULT_REFERENCE_CHANNEL_TOOH_IGH = 2,
    SETUPRESULT_SAMPLE_CHANNEL_TOO_HIGH = 3,
} SetupResult_t;

#define SELFTEST_ILED_230      0x00000001
#define SELFTEST_ILED_260      0x00000002
#define SELFTEST_ILED_280      0x00000004
#define SELFTEST_ILED_340      0x00000008
#define SELFTEST_SAMPLE_230    0x00000010
#define SELFTEST_SAMPLE_260    0x00000020
#define SELFTEST_SAMPLE_280    0x00000040
#define SELFTEST_SAMPLE_340    0x00000080
#define SELFTEST_REFERENCE_230 0x00000100
#define SELFTEST_REFERENCE_260 0x00000200
#define SELFTEST_REFERENCE_280 0x00000400
#define SELFTEST_REFERENCE_340 0x00000800
#define SELFTEST_REFERENCE     0x00001000
#define SELFTEST_SAMPLE        0x00002000

    DLLEXPORT Error_t
    colibriFindDevice(char *portName, size_t *portNameSize, bool verbose);
DLLEXPORT ColibriResponse_t *colibriCreateResponse();
DLLEXPORT void colibriFreeResponse(ColibriResponse_t *response);
DLLEXPORT Error_t colibriCommand(Colibri_t *self, const char *command, ColibriResponse_t *response);

DLLEXPORT Error_t colibriGet(Colibri_t *self, uint32_t index, char *value, size_t valueSize);
DLLEXPORT Error_t colibriSet(Colibri_t *self, uint32_t index, const char *value);
DLLEXPORT Error_t colibriMeasure(Colibri_t *self, uint32_t *sample230, uint32_t *reference230, uint32_t *sample260, uint32_t *reference260, uint32_t *sample280, uint32_t *reference280, uint32_t *sample340, uint32_t *reference340);
DLLEXPORT Error_t colibriBaseline(Colibri_t *self, uint32_t *sample230, uint32_t *reference230, uint32_t *sample260, uint32_t *reference260, uint32_t *sample280, uint32_t *reference280, uint32_t *sample340, uint32_t *reference340);
DLLEXPORT Error_t colibriLevelling(Colibri_t *self, Levelling_t *levelling230, Levelling_t *levelling260, Levelling_t *levelling280, Levelling_t *levelling340);
DLLEXPORT Error_t colibriSelftest(Colibri_t *self, uint32_t *result);
DLLEXPORT Error_t colibriFwUpdate(Colibri_t *self, const char *file);
DLLEXPORT Error_t colibriLastMeasurements(Colibri_t *self, uint32_t last, uint32_t *sample230, uint32_t *reference230, uint32_t *sample260, uint32_t *reference260, uint32_t *sample280, uint32_t *reference280, uint32_t *sample340, uint32_t *reference340);
DLLEXPORT Error_t colibriLastLevelling(Colibri_t *self, Levelling_t *levelling230, Levelling_t *levelling260, Levelling_t *levelling280, Levelling_t *levelling340);
DLLEXPORT const char *colibriError2String(Error_t e);
DLLEXPORT const char *colibriVersion();

HANDLE colibriPortOpen(char *portName);
void colibriPortClose(HANDLE hComm);
bool colibriPortWrite(HANDLE hComm, char *buffer, bool verbose);
uint32_t colibriPortRead(HANDLE hComm, char *buffer, size_t size, bool verbose);
