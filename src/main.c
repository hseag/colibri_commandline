// SPDX-License-Identifier: MIT
// SPDX-FileCopyrightText: © 2024 HSE AG, <opensource@hseag.com>

#include "colibri.h"
#include "cmdget.h"
#include "cmdset.h"
#include "cmdmeasure.h"
#include "cmdbaseline.h"
#include "cmdselftest.h"
#include "cmdcommand.h"
#include "cmdfwupdate.h"
#include "cmdlevelling.h"
#include "cmdsave.h"
#include "cmddata.h"
#include "printerror.h"
#include <stdio.h>
#include <string.h>

#define VERSION_TOOL "0.0.1"

void help(int argcCmd, char **argvCmd)
{
	if(argvCmd == NULL)
	{
			fprintf(stdout, "Usage: colibri [OPTIONS] COMMAND [ARGUMENTS]\n");
			fprintf(stdout, "Commands:\n");
			fprintf(stdout, "  get INDEX           : get a value from the device\n");
			fprintf(stdout, "  set INDEX VALUE     : set a value in the device\n");
			fprintf(stdout, "  levelling           : prepares the module for a measurment\n");
			fprintf(stdout, "  save                : save the last measurement(s)\n");
			fprintf(stdout, "  calculate           : caluclate od's or concentrations\n");
			fprintf(stdout, "  baseline            : starts a baseline measurement and return the values\n");
			fprintf(stdout, "  measure             : starts a measurement and return the values\n");
			fprintf(stdout, "  version             : return the CLI and DLL version\n");
			fprintf(stdout, "  selftest            : executes an internal selftest\n");
			fprintf(stdout, "  fwupdate FILE       : loads a new firmware\n");
			fprintf(stdout, "  command COMMAND     : executes a command e.g colibri.exe command \"V 0\" returns the value at index 0\n");
			fprintf(stdout, "  help COMMAND        : Prints a detailed help\n");
			fprintf(stdout, "\n");
			fprintf(stdout, "Options:\n");
			fprintf(stdout, "  --verbose           : prints debug info\n");
			fprintf(stdout, "  --help -h           : show this help and exit\n");
			fprintf(stdout, "  --device            : use the given device, if omitted the CLI searchs for a device\n");
			fprintf(stdout, "  --use-checksum      : use the protocol with a checksum\n");
	}
	else
	{
		if(argcCmd == 2)
		{
			if(strcmp(argvCmd[1], "get") == 0)
			{
				fprintf(stdout, "Usage: colibri get INDEX\n");
				fprintf(stdout, "  Get a value from the device\n");
				fprintf(stdout, "INDEX:\n");
				fprintf(stdout, "   0: Firmware version\n");
				fprintf(stdout, "   1: Serial number\n");
				fprintf(stdout, "   2: Hardware type\n");
				fprintf(stdout, "  10: Number of internal stored last measurements\n");
				fprintf(stdout, "  23: Max current in [uA] for the 230nm LED\n");
				fprintf(stdout, "  33: Max current in [uA] for the 260nm LED\n");
				fprintf(stdout, "  43: Max current in [uA] for the 280nm LED\n");
				fprintf(stdout, "  53: Max current in [uA] for the 340nm LED\n");
				fprintf(stdout, "  60: Sample amplification factor nominal 1.1\n");
				fprintf(stdout, "  61: Sample amplification factor nominal 11.0\n");
				fprintf(stdout, "  62: Sample amplification factor nominal 111.0\n");
				fprintf(stdout, "  63: Reference amplification factor nominal 1.1\n");
				fprintf(stdout, "  64: Reference amplification factor nominal 11.0\n");
				fprintf(stdout, "  65: Reference amplification factor nominal 111.0\n");				
				fprintf(stdout, "  80: Levelling voltage in [uV] for the 230n LED\n");
				fprintf(stdout, "  81: Levelling voltage in [uV] for the 260n LED\n");
				fprintf(stdout, "  82: Levelling voltage in [uV] for the 280n LED\n");
				fprintf(stdout, "  83: Levelling voltage in [uV] for the 340n LED\n");
			}
			else if(strcmp(argvCmd[1], "set") == 0)
			{
				fprintf(stdout, "Usage: colibri set INDEX VALUE\n");
				fprintf(stdout, "  Set a value in the device\n");
				fprintf(stdout, "WARNING:\n");
				fprintf(stdout, "  Changing a value can damage the device or lead to incorrect results!\n");
				fprintf(stdout, "INDEX:\n");
				fprintf(stdout, "   1: Serial number\n");
				fprintf(stdout, "   2: Hardware type\n");
				fprintf(stdout, "  23: Max current in [uA] for the 230nm LED\n");
				fprintf(stdout, "  33: Max current in [uA] for the 260nm LED\n");
				fprintf(stdout, "  43: Max current in [uA] for the 280nm LED\n");
				fprintf(stdout, "  53: Max current in [uA] for the 340nm LED\n");
				fprintf(stdout, "  60: Sample amplification factor nominal 1.1\n");
				fprintf(stdout, "  61: Sample amplification factor nominal 11.0\n");
				fprintf(stdout, "  62: Sample amplification factor nominal 111.0\n");
				fprintf(stdout, "  63: Reference amplification factor nominal 1.1\n");
				fprintf(stdout, "  64: Reference amplification factor nominal 11.0\n");
				fprintf(stdout, "  65: Reference amplification factor nominal 111.0\n");				
				fprintf(stdout, "  80: Levelling voltage in [uV] for the 230n LED\n");
				fprintf(stdout, "  81: Levelling voltage in [uV] for the 260n LED\n");
				fprintf(stdout, "  82: Levelling voltage in [uV] for the 280n LED\n");
				fprintf(stdout, "  83: Levelling voltage in [uV] for the 340n LED\n");
			}
			else if(strcmp(argvCmd[1], "levelling") == 0)
			{
				fprintf(stdout, "Usage: colibri levelling\n");
				fprintf(stdout, "  The levelling commands sets the LED currents and amplifications so that the levelling voltage is reached.\n");
				fprintf(stdout, "  The values determined are internally stored and printed to stdout.\n");
				fprintf(stdout, "  The firmware has an internal storage for up to ten measurements. The command levelling clears this storage.\n");
				fprintf(stdout, "Usage: colibri levelling 0\n");
				fprintf(stdout, "  The last values are printed to stdout.\n");
				fprintf(stdout, "Output:\n");
				fprintf(stdout, "  RESULT_230 CURRENT_230 SAMPLE_AMPLIFICATION_230 REFERENCE_AMPLIFICATION_230 RESULT_260 CURRENT_260 SAMPLE_AMPLIFICATION_260 REFERENCE_AMPLIFICATION_260 RESULT_280 CURRENT_280 SAMPLE_AMPLIFICATION_280 REFERENCE_AMPLIFICATION_280 RESULT_340 CURRENT_340 SAMPLE_AMPLIFICATION_340 REFERENCE_AMPLIFICATION_340\n");
				fprintf(stdout, "  RESULT_XXX:                  0:ok, 1:No amplification found, 2:Reference channel too high, 3:Sample channel too high\n");
				fprintf(stdout, "  CURRENT_XXX:                 current in [uA]\n");
				fprintf(stdout, "  REFERENCE_AMPLIFICATION_XXX: 0:x1.1, 1:x11.0, 2:x111.0\n");
				fprintf(stdout, "  SAMPLE_AMPLIFICATION_XXX:    0:x1.1, 1:x11.0, 2:x111.0\n");
			}
			else if(strcmp(argvCmd[1], "save") == 0)
			{
				fprintf(stdout, "Usage: colibri save [FILE]\n");
				fprintf(stdout, "  Saves the levelling data and the last measurements in the given file FILE as a csv file.\n");
				fprintf(stdout, "  If FILE is -, the data is printed to stdout.\n");
				fprintf(stdout, "  If FILE is omitted, an automatic file name is choosen: SN-YEAR-MONTH-DAY_HOUR_MIN_SEC.csv where SN is the serial number of the current colibri module.\n");
			}
			else if(strcmp(argvCmd[1], "calculate") == 0)
			{
				fprintf(stdout, "Usage: colibri calculate od\n");
				fprintf(stdout, "  Calculates from the last two measurements stored in the colibri module the optical density (od).\n");
				fprintf(stdout, "Output:\n");
				fprintf(stdout, "  OD_230 OD_260 OD_280 OD_340\n");
			}
			else if(strcmp(argvCmd[1], "measure") == 0)
			{
				fprintf(stdout, "Usage: colibri measure\n");
				fprintf(stdout, "  Measures with all LEDs and print the values to stdout.\n");
				fprintf(stdout, "Usage: colibri measure LAST\n");
				fprintf(stdout, "  Retrives the last LAST measurement and print the values to stdout.\n");
				fprintf(stdout, "  The last measurement is at 0, the second last 1.\n");
				fprintf(stdout, "Output: all units in [uV]\n");
				fprintf(stdout, "  SAMPLE_230 REFERENCE_230 SAMPLE_260 REFERENCE_260 SAMPLE_280 REFERENCE_280 SAMPLE_340 REFERENCE_340\n");
			}
			else if(strcmp(argvCmd[1], "baseline") == 0)
			{
				fprintf(stdout, "Usage: colibri baseline\n");
				fprintf(stdout, "  If a levelling is needed, the command levelling is executed before a measurement is started. For this measurement, the cuvette holder must be empty.\n");
				fprintf(stdout, "  The firmware has an internal storage for up to ten measurements. The command baseline clears this storage.\n");
				fprintf(stdout, "Output: all units in [uV]\n");
				fprintf(stdout, "  SAMPLE_230 REFERENCE_230 SAMPLE_260 REFERENCE_260 SAMPLE_280 REFERENCE_280 SAMPLE_340 REFERENCE_340\n");
			}
			else if(strcmp(argvCmd[1], "version") == 0)
			{
				fprintf(stdout, "Usage: colibri version\n");
				fprintf(stdout, "  Prints the version of this tool and the libcolibri to stdout.\n");
			}
			else if(strcmp(argvCmd[1], "selftest") == 0)
			{
 				fprintf(stdout, "Usage: colibri selftest\n");
				fprintf(stdout, "  Executes a selftest and prints the result.\n");
				fprintf(stdout, "  If the result is not ok, the must common case is that the cuvette guide is blocking the optical path\n");
				fprintf(stdout, "  or a cuvette is stuck in the cuvette guide.\n");
			}
			else if(strcmp(argvCmd[1], "fwupdate") == 0)
			{
				fprintf(stdout, "Usage: colibri fwupdate SREC_FILE\n");
				fprintf(stdout, "  Updates the firmware.\n");
			}
			else if(strcmp(argvCmd[1], "command") == 0)
			{
				fprintf(stdout, "Usage: colibri command COMMAND\n");
				fprintf(stdout, "  Executes any colibri command. Usefull for testing.\n");
			}
			else
			{
				fprintf(stdout, "No help for command '%s'\n", argvCmd[1]);
			}
		}
		else
		{
			help(0, NULL);
		}
	}
}


int main(int argc, char *argv[])
{
	Error_t ret = ERROR_COLIBRI_OK;
	int argcCmd = argc;
	char **argvCmd = argv;
	bool options = true;
	int i = 1;
	Colibri_t colibri = {0};

	while (i < argc && options)
	{
		if (strncmp(argv[i], "--", 2) == 0 || strncmp(argv[i], "-", 1) == 0)
		{
			if (strcmp(argv[i], "--verbose") == 0)
			{
				colibri.verbose = true;
			}
			else if (strcmp(argv[i], "--use-checksum") == 0)
			{
				colibri.useChecksum = true;
			}
			else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0)
			{
				help(0, NULL);
				return ERROR_COLIBRI_OK;
			}
			else if ((strcmp(argv[i], "--device") == 0) && (i + 1 < argc))
			{
				i++;
				colibri.portName = argv[i];
			}
			else
			{
				return printError(ERROR_COLIBRI_UNKOWN_COMMAND_LINE_OPTION, "Unknown option: %s\n", argv[i]);
			}
			i++;
		}
		else
		{
			options = false;
		}
	}

	argcCmd = argc - i;
	argvCmd = argv + i;

	if (argcCmd > 0)
	{
		if (strcmp(argvCmd[0], "get") == 0 && argcCmd == 2)
		{
			return cmdGet(&colibri, argvCmd[1]);
		}
		else if (strcmp(argvCmd[0], "set") == 0 && argcCmd == 3)
		{
			return cmdSet(&colibri, argvCmd[1], argvCmd[2]);
		}
		else if (strcmp(argvCmd[0], "measure") == 0)
		{
			return cmdMeasure(&colibri);
		}
		else if (strcmp(argvCmd[0], "baseline") == 0)
		{
			return cmdBaseline(&colibri);
		}
		else if (strcmp(argvCmd[0], "levelling") == 0)
		{
			return cmdLevelling(&colibri);
		}
		else if (strcmp(argvCmd[0], "version") == 0)
		{
			fprintf(stdout, "command-line interface:%s library:%s\n", VERSION_TOOL, colibriVersion());
		}
		else if (strcmp(argvCmd[0], "selftest") == 0)
		{
			return cmdSelftest(&colibri);
		}
		else if (strcmp(argvCmd[0], "fwupdate") == 0 && argcCmd == 2)
		{
			return cmdFwUpdate(&colibri, argvCmd[1]);
		}
		else if (strcmp(argvCmd[0], "command") == 0 && argcCmd == 2)
		{
			return cmdCommand(&colibri, argvCmd[1]);
		}
		else if (strcmp(argvCmd[0], "data") == 0)
		{
			return cmdData(&colibri, argcCmd, argvCmd);
		}
		else if (strcmp(argvCmd[0], "save") == 0)
		{
			return cmdSave(&colibri, argcCmd, argvCmd);
		}
		else if (strcmp(argvCmd[0], "help") == 0)
		{
			help(argcCmd, argvCmd);
			return ERROR_COLIBRI_OK;
		}
		else
		{
			return printError(ERROR_COLIBRI_UNKOWN_COMMAND_LINE_ARGUMENT, "'%s' is not a colibri command. See 'colibri --help'.", argvCmd[0]);
		}
	}
	else
	{
		help(0, NULL);
	}

	return ret;
}