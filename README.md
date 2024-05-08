# Command line tool for Colibri Module

For more informations about the Colibri Module see [here](https://blog.hseag.com/de-de/oem-komponente-misst-proben-qualitaett).

# Usage
```
Usage: colibri [OPTIONS] COMMAND [ARGUMENTS]\
Commands:
  baseline            : starts a baseline measurement and return the values\
  command COMMAND     : executes a command e.g colibri.exe command \"V 0\" returns the value at index 0
  data                : handels data in a data file
  fwupdate FILE       : loads a new firmware
  get INDEX           : get a value from the device
  help COMMAND        : Prints a detailed help
  levelling           : prepares the module for a measurment
  measure             : starts a measurement and return the values
  save                : save the last measurement(s)
  selftest            : executes an internal selftest
  set INDEX VALUE     : set a value in the device
  version             : return the CLI and DLL version
Options:
  --verbose           : prints debug info
  --help -h           : show this help and exit
  --device            : use the given device, if omitted the CLI searchs for a device
  --use-checksum      : use the protocol with a checksum

The commandline tool returns the following exit codes:
    0: No error.
    1: Unknown command
    2: Invalid parameter
    3: Timeout.
    4: SREC Flash write error
    5: SREC Unsupported type
    6: SREC Invalid crc
    7: SREC Invalid string
    8: Leveeling failed. Cuvette holder blocked?
  100: Response error
  101: Protocol error
  202: Unknown command line argument
  202: Unknown command line option
  203: Colibri Module not found
  204: File not found
  207: Unexpected number of measurements.
  
```
# Typical Sequence
1.	Aspirate the sample, a minimal volume of 11.5 µl is needed.
2.	Pickup a cuvette from the Colibri Module.
3.	Move the tip with the empty cuvette over the cuvette holder.
4.	Start the command [baseline](README.md#command-baseline).
5.	Move the tip with the empty cuvette into the cuvette holder.
6.	Start the command [measure](README.md#command-measure).
7.	Dispense 11.5 µl sample into the cuvette. A good speed is about 5 µl/s.
8.	Start the command [measure](README.md#command-measure).
9.	Aspirate the sample back from the cuvette into the tip.
10.	Move the cuvette out of the cuvette holder.
11.	If needed, eject the cuvette, and process the sample further.
12.	Save the data with command [save](README.md#command-save).

Calculated the values with command [data](README.md#command-data).

# Command Details
## Command baseline
```
Usage: colibri baseline
  If a levelling is needed, the command levelling is executed before a measurement is started. For this measurement, the cuvette holder must be empty.
  The firmware has an internal storage for up to ten measurements. The command baseline clears this storage.
Output: all units in [uV]
  SAMPLE_230 REFERENCE_230 SAMPLE_260 REFERENCE_260 SAMPLE_280 REFERENCE_280 SAMPLE_340 REFERENCE_340
```
## Command command
```
Usage: colibri command COMMAND
  Executes any colibri command. Usefull for testing.
```
## Command data
```
Usage: data print FILE
  Prints the calculated values from file FILE.
Output:
  OD_230 OD_260 OD_280 OD_340 CONCENTRATION in ng/ul

Usage: data calculate [OPTIONS] FILE
  Calculates the optical density and concentration in the given file and adds the values to the file.
  To calculate the values at least the first value must be a blank.
Options:
  --blanks      : number of blanks from the begining. Default is 1
  --pathLength  : path length in [mm]. Default is 1.0
  --a260unit    : for dsDNA use 50, for ssDNA use 33 and for ssRNA use 40. Default is 50.\n
```
## Command fwupdate
```
Usage: colibri fwupdate SREC_FILE
  Updates the firmware.
```
## Command get
```
Usage: colibri get INDEX
  Get a value from the device
INDEX:
   0: Firmware version
   1: Serial number
   2: Hardware type
  10: Number of internal stored last measurements
  23: Max current in [uA] for the 230nm LED
  33: Max current in [uA] for the 260nm LED
  43: Max current in [uA] for the 280nm LED
  53: Max current in [uA] for the 340nm LED
  60: Sample amplification factor nominal 1.1
  61: Sample amplification factor nominal 11.0
  62: Sample amplification factor nominal 111.0
  63: Reference amplification factor nominal 1.1
  64: Reference amplification factor nominal 11.0
  65: Reference amplification factor nominal 111.0
  80: Levelling voltage in [uV] for the 230n LED
  81: Levelling voltage in [uV] for the 260n LED
  82: Levelling voltage in [uV] for the 280n LED
  83: Levelling voltage in [uV] for the 340n LED
```
## Command levelling
```
Usage: colibri levelling
  The levelling commands sets the LED currents and amplifications so that the levelling voltage is reached.
  The values determined are internally stored and printed to stdout.
  The firmware has an internal storage for up to ten measurements. The command levelling clears this storage.
Usage: colibri levelling 0
  The last values are printed to stdout.
Output:
  RESULT_230 CURRENT_230 SAMPLE_AMPLIFICATION_230 REFERENCE_AMPLIFICATION_230 RESULT_260 CURRENT_260 SAMPLE_AMPLIFICATION_260 REFERENCE_AMPLIFICATION_260 RESULT_280 CURRENT_280 SAMPLE_AMPLIFICATION_280 REFERENCE_AMPLIFICATION_280 RESULT_340 CURRENT_340 SAMPLE_AMPLIFICATION_340 REFERENCE_AMPLIFICATION_340
  RESULT_XXX:                  0:ok, 1:No amplification found, 2:Reference channel too high, 3:Sample channel too high
  CURRENT_XXX:                 current in [uA]
  REFERENCE_AMPLIFICATION_XXX: 0:x1.1, 1:x11.0, 2:x111.0
  SAMPLE_AMPLIFICATION_XXX:    0:x1.1, 1:x11.0, 2:x111.0
```
## Command measure
```
Usage: colibri measure
  Measures with all LEDs and print the values to stdout.
Usage: colibri measure LAST
  Retrives the last LAST measurement and print the values to stdout.
  The last measurement is at 0, the second last 1.
Output: all units in [uV]
  SAMPLE_230 REFERENCE_230 SAMPLE_260 REFERENCE_260 SAMPLE_280 REFERENCE_280 SAMPLE_340 REFERENCE_340
```
## Command save
```
Usage: colibri save [FILE] [COMMENT]
  Saves the levelling data and the last measurements in the given file FILE as a JSON file. If the file already exists, the data are appended.
  The optional string COMMENT is added as a comment to the measurement in the JSON file.
```
## Command selftest
```
Usage: colibri selftest
  Executes a selftest and prints the result.
  If the result is not ok, the must common case is that the cuvette guide is blocking the optical path
  or a cuvette is stuck in the cuvette guide.
```
## Command set
```
Usage: colibri set INDEX VALUE
  Set a value in the device
WARNING:
  Changing a value can damage the device or lead to incorrect results!
INDEX:
   1: Serial number
   2: Hardware type
  23: Max current in [uA] for the 230nm LED
  33: Max current in [uA] for the 260nm LED
  43: Max current in [uA] for the 280nm LED
  53: Max current in [uA] for the 340nm LED
  60: Sample amplification factor nominal 1.1
  61: Sample amplification factor nominal 11.0
  62: Sample amplification factor nominal 111.0
  63: Reference amplification factor nominal 1.1
  64: Reference amplification factor nominal 11.0
  65: Reference amplification factor nominal 111.0
  80: Levelling voltage in [uV] for the 230n LED
  81: Levelling voltage in [uV] for the 260n LED
  82: Levelling voltage in [uV] for the 280n LED
  83: Levelling voltage in [uV] for the 340n LED
```
## Command version
```
Usage: colibri version
  Prints the version of this tool and the libcolibri to stdout.
```

