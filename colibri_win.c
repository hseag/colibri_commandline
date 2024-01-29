// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "colibri.h"
#include "crc-16-ccitt.h"
#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include <tchar.h>
#include <setupapi.h>
#include <initguid.h>
#include <stdio.h>
#include <stdarg.h>

#pragma comment(lib, "Setupapi.lib")
// This is the GUID for the USB device class
DEFINE_GUID(GUID_DEVINTERFACE_USB_DEVICE, 0xA5DCBF10L, 0x6530, 0x11D2, 0x90, 0x1F, 0x00, 0xC0, 0x4F, 0xB9, 0x51, 0xED);


Error_t colibriFindDevice(char * portName, size_t * portNameSize, bool verbose)
{
	HDEVINFO hDevInfo;
	SP_DEVICE_INTERFACE_DATA devIntfData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA devIntfDetailData;
	SP_DEVINFO_DATA devData;

	DWORD dwSize;
	DWORD dwType;
	uint32_t dwMemberIdx;
	HKEY hKey;
	bool found = false;

	memset(portName, 0, *portNameSize);

	// We will try to get device information set for all USB devices that have a
	// device interface and are currently present on the system (plugged in).
	hDevInfo = SetupDiGetClassDevs(&GUID_DEVINTERFACE_USB_DEVICE, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);

	if (hDevInfo != INVALID_HANDLE_VALUE)
	{
		// Prepare to enumerate all device interfaces for the device information
		// set that we retrieved with SetupDiGetClassDevs(..)
		devIntfData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
		dwMemberIdx = 0;

		// Next, we will keep calling this SetupDiEnumDeviceInterfaces(..) until this
		// function causes GetLastError() to return  ERROR_NO_MORE_ITEMS. With each
		// call the dwMemberIdx value needs to be incremented to retrieve the next
		// device interface information.

		SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_USB_DEVICE, dwMemberIdx, &devIntfData);

		if(verbose)
		{
			fprintf(stderr, "USB Devices:\n");
		}
		while (GetLastError() != ERROR_NO_MORE_ITEMS && !found)
		{
			// As a last step we will need to get some more details for each
			// of device interface information we are able to retrieve. This
			// device interface detail gives us the information we need to identify
			// the device (VID/PID), and decide if it's useful to us. It will also
			// provide a DEVINFO_DATA structure which we can use to know the serial
			// port name for a virtual com port.

			devData.cbSize = sizeof(devData);

			// Get the required buffer size. Call SetupDiGetDeviceInterfaceDetail with
			// a NULL DevIntfDetailData pointer, a DevIntfDetailDataSize
			// of zero, and a valid RequiredSize variable. In response to such a call,
			// this function returns the required buffer size at dwSize.
			SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntfData, NULL, 0, &dwSize, NULL);

			// Allocate memory for the DeviceInterfaceDetail struct. Don't forget to deallocate it later!
			devIntfDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwSize);
			devIntfDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			if (SetupDiGetDeviceInterfaceDetail(hDevInfo, &devIntfData, devIntfDetailData, dwSize, &dwSize, &devData))
			{
				if(verbose)
				{
					fprintf(stderr, "USB Device: %s\n", (TCHAR *)devIntfDetailData->DevicePath);
				}
				// Finally we can start checking if we've found a useable device,
				// by inspecting the DevIntfDetailData->DevicePath variable.
				if (NULL != _tcsstr((TCHAR *)devIntfDetailData->DevicePath, _T("vid_1cbe&pid_0002")))
				{
					hKey = SetupDiOpenDevRegKey(hDevInfo, &devData, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
					dwType = REG_SZ;
					DWORD d = *portNameSize;
					RegQueryValueEx(hKey, _T("PortName"), NULL, &dwType, (LPBYTE)portName, &d);
					*portNameSize = d;
					RegCloseKey(hKey);
					found = true;
				}
			}

			HeapFree(GetProcessHeap(), 0, devIntfDetailData);

			// Continue looping
			SetupDiEnumDeviceInterfaces(hDevInfo, NULL, &GUID_DEVINTERFACE_USB_DEVICE, ++dwMemberIdx, &devIntfData);
		}

		SetupDiDestroyDeviceInfoList(hDevInfo);
	}

	return found ? ERROR_COLIBRI_OK : ERROR_COLIBRI_NOT_FOUND;
}

HANDLE colibriPortOpen(char * portName)
{
	HANDLE hComm;
	{
		LPTSTR dn = "\\\\.\\";
		DWORD deviceSize = strlen(portName) + strlen(dn) + 1;
		LPTSTR device = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, deviceSize);
		strcat_s(device, deviceSize, dn);
		strcat_s(device, deviceSize, portName);

		hComm = CreateFile(device,						 // port name
						   GENERIC_READ | GENERIC_WRITE, // Read/Write
						   0,							 // No Sharing
						   NULL,						 // No Security
						   OPEN_EXISTING,				 // Open existing port only
						   0,							 // Non Overlapped I/O
						   NULL);						 // Null for Comm Devices

		HeapFree(GetProcessHeap(), 0, device);
	}

	if (hComm == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "could not open port %s\n", portName);
		return INVALID_HANDLE_VALUE;
	}

	BOOL success = FlushFileBuffers(hComm);
	if (!success)
	{
		fprintf(stderr, "could not flush buffers\n");
		CloseHandle(hComm);
		return INVALID_HANDLE_VALUE;
	}

	// Configure read and write operations to time out after 100 ms.
	COMMTIMEOUTS timeouts = {0};
	timeouts.ReadIntervalTimeout = 0;
	timeouts.ReadTotalTimeoutConstant = 1;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 1;
	timeouts.WriteTotalTimeoutMultiplier = 0;

	success = SetCommTimeouts(hComm, &timeouts);
	if (!success)
	{
		fprintf(stderr, "could not timeouts\n");
		CloseHandle(hComm);
		return INVALID_HANDLE_VALUE;
	}

	// Set the baud rate and other options.
	DCB state = {0};
	state.DCBlength = sizeof(DCB);
	state.BaudRate = CBR_115200;
	state.ByteSize = 8;
	state.Parity = NOPARITY;
	state.StopBits = ONESTOPBIT;
	success = SetCommState(hComm, &state);
	if (!success)
	{
		fprintf(stderr, "could not set serial settings\n");
		CloseHandle(hComm);
		return INVALID_HANDLE_VALUE;
	}

	return hComm;
}

void colibriPortClose(HANDLE hComm)
{
	if (hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hComm); // Closing the Serial Port
	}
}

bool colibriPortWrite(HANDLE hComm, LPTSTR buffer, bool verbose)
{
	DWORD written;
	DWORD size = strlen(buffer);

   if(verbose)
   {
	fprintf(stderr, "TX: %s\n", buffer);
   }

	BOOL success = WriteFile(hComm, buffer, size, &written, NULL);
	if (!success)
	{
		fprintf(stderr, "could not write to port\n");
		return false;
	}
	if (written != size)
	{
    	fprintf(stderr, "could not all bytes to port\n");
		return false;
	}
	return true;
}

uint32_t colibriPortRead(HANDLE hComm, LPTSTR buffer, size_t size, bool verbose)
{
	DWORD received;
	DWORD count = 0;
	char rx[COLIBRI_MAX_LINE_LENGTH];
	bool waitForStart = true;
	bool done = false;
	bool useChecksum = false;
	int checkSumSeparator = -1;

	do
	{
		memset(rx, 0, COLIBRI_MAX_LINE_LENGTH);
		BOOL success = ReadFile(hComm, rx, COLIBRI_MAX_LINE_LENGTH, &received, NULL);
		if (!success)
		{
			fprintf(stderr, "could not read from port\n");
			return -1;
		}

        if(verbose && received > 0)
        {
	      fprintf(stderr, "RX: %s\n", rx);
        }		

		for (DWORD i = 0; i < received && !done; i++)
		{
			if (waitForStart)
			{
				if (rx[i] == COLIBRI_START_NO_CHK || rx[i] == COLIBRI_START_WITH_CHK)
				{
					waitForStart = false;
					if(rx[i] == COLIBRI_START_WITH_CHK)
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
					if(buffer[count] == COLIBRI_CHECKSUM_SEPARATOR)
					{
						checkSumSeparator = count;
					}
					count++;
				}
			}
		}
	} while (!done);

    if(useChecksum)
	{
		crc_t crcReceived;
		crc_t crc = crc_init();
		crc = crc_update(crc, buffer, checkSumSeparator);
		crc = crc_finalize(crc);
		crcReceived = atoi(buffer+checkSumSeparator+1);
		if(crc == crcReceived)
		{
          buffer[checkSumSeparator] = 0;
		}
		else
		{
			fprintf(stderr, "CRC differ: received message %s, calculated crc=%i", buffer, crcReceived);
			return 0;
		}
	}

	return count;
}