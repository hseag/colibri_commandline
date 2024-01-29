// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "colibri.h"
#include "crc-16-ccitt.h"
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <dirent.h>

#define MIN(x, y) (((x) < (y)) ? (x) : (y))

Error_t colibriFindDevice(char *portName, size_t *portNameSize, bool verbose)
{
    struct dirent **namelist;
    int n;
    char *id = "usb-HSE_Colibri_";
    char * path = "/dev/serial/by-id/";
    bool found = false;

    memset(portName, 0, *portNameSize);

    n = scandir(path, &namelist, NULL, NULL);
    if (n < 0)
    {
        return ERROR_COLIBRI_NOT_FOUND;
    }
    else
    {
        while (n--)
        {
            if(verbose)
            {
                printf("%s\n", namelist[n]->d_name);
            }
            if (!found)
            {
                if (strncmp(namelist[n]->d_name, id, strlen(id)) == 0)
                {
                    found = true;
                    strncat(portName, path, *portNameSize);
                    strncat(portName, namelist[n]->d_name, *portNameSize);
                    *portNameSize = strlen(portName);
                }
            }
            free(namelist[n]);
        }
        free(namelist);
    }

    return found ? ERROR_COLIBRI_OK : ERROR_COLIBRI_NOT_FOUND;
}

int colibriPortOpen(char *portName)
{
    int hComm;
    {
        hComm = open(portName, O_RDWR | O_NOCTTY);
    }

    if (hComm == -1)
    {
        fprintf(stderr, "Could not open port %s\n", portName);
        return -1;
    }

    if (tcflush(hComm, TCIOFLUSH) == -1)
    {
        fprintf(stderr, "Could not flush buffers\n");
        close(hComm);
        return -1;
    }

    // Configure read and write operations to time out after 100 ms.
    struct termios options;
    tcgetattr(hComm, &options);
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 1;

    if (tcsetattr(hComm, TCSANOW, &options) == -1)
    {
        fprintf(stderr, "Could not set timeouts\n");
        close(hComm);
        return -1;
    }

    // Set the baud rate and other options.
    cfsetispeed(&options, B115200);
    cfsetospeed(&options, B115200);
    options.c_cflag |= (CLOCAL | CREAD);
    options.c_cflag &= ~PARENB;
    options.c_cflag &= ~CSTOPB;
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    if (tcsetattr(hComm, TCSANOW, &options) == -1)
    {
        fprintf(stderr, "Could not set serial settings\n");
        close(hComm);
        return -1;
    }

    return hComm;
}

void colibriPortClose(int hComm)
{
    if (hComm != -1)
    {
        close(hComm);
    }
}

bool colibriPortWrite(int hComm, char *buffer, bool verbose)
{
    ssize_t written;
    size_t size = strlen(buffer);

    if (verbose)
    {
        fprintf(stderr, "TX: %s\n", buffer);
    }

    written = write(hComm, buffer, size);

    if (written == -1)
    {
        fprintf(stderr, "Could not write to port\n");
        return false;
    }

    if ((size_t)written != size)
    {
        fprintf(stderr, "Could not write all bytes to port\n");
        return false;
    }

    return true;
}

uint32_t colibriPortRead(int hComm, char *buffer, size_t size, bool verbose)
{
    ssize_t received;
    size_t count = 0;
    char rx[COLIBRI_MAX_LINE_LENGTH];
    bool waitForStart = true;
    bool done = false;
    bool useChecksum = false;
    int checkSumSeparator = -1;

    do
    {
        memset(rx, 0, COLIBRI_MAX_LINE_LENGTH);
        received = read(hComm, rx, COLIBRI_MAX_LINE_LENGTH);
        if (received == -1)
        {
            fprintf(stderr, "Could not read from port\n");
            return 0;
        }

        if (verbose && received > 0)
        {
            fprintf(stderr, "RX: %s\n", rx);
        }

        for (size_t i = 0; i < (size_t)received && !done; i++)
        {
            if (waitForStart)
            {
                if (rx[i] == COLIBRI_START_NO_CHK || rx[i] == COLIBRI_START_WITH_CHK)
                {
                    waitForStart = false;
                    if (rx[i] == COLIBRI_START_WITH_CHK)
                    {
                        useChecksum = true;
                    }
                }
            }
            else
            {
                if (rx[i] == COLIBRI_STOP1 || rx[i] == COLIBRI_STOP2)
                {
                    done = true;
                    buffer[count] = 0;
                }
                else
                {
                    buffer[count] = rx[i];
                    if (buffer[count] == COLIBRI_CHECKSUM_SEPARATOR)
                    {
                        checkSumSeparator = count;
                    }
                    count++;
                }
            }
        }
    } while (!done);

    if (useChecksum)
    {
        crc_t crcReceived;
        crc_t crc = crc_init();
        crc = crc_update(crc, buffer, checkSumSeparator);
        crc = crc_finalize(crc);
        crcReceived = atoi(buffer + checkSumSeparator + 1);
        if (crc == crcReceived)
        {
            buffer[checkSumSeparator] = 0;
        }
        else
        {
            fprintf(stderr, "CRC differ: received message %s, calculated crc=%i", buffer, (uint32_t)crcReceived);
            return 0;
        }
    }

    return count;
}

errno_t strncat_s(char *restrict dest, rsize_t destsz, const char *restrict src, rsize_t count)
{
    size_t s1_len = 0;
    size_t m = destsz - (s1_len = strnlen(dest, destsz));

    // If s2 < n, we are going to read strlen(s2) + its terminating null byte
    // Otherwise, we are going to read exactly n bytes
    rsize_t s2_size = MIN(strnlen(src, count) + 1, count);

    strncat(dest, src, count);
    // should not be necessary
    dest[destsz - 1] = '\0';

    return 0;
}

errno_t strncpy_s(char *restrict dest, rsize_t destsz, const char *restrict src, rsize_t count)
{
    rsize_t m;
    rsize_t s2_size = MIN(strnlen(src, count) + 1, count);

    m = MIN(count, destsz - 1);
    strncpy(dest, src, m);
    dest[MIN(count, destsz - 1)] = '\0';

    // on success
    return 0;
}

errno_t strcpy_s(char *restrict dest, rsize_t destsz, const char *restrict src)
{
    rsize_t s2_len = 0;
    strncpy(dest, src, destsz - 1);
    dest[destsz - 1] = '\0';

    // on success
    return 0;
}

void Sleep(uint32_t dwMilliseconds)
{
    sleep(dwMilliseconds);
}
