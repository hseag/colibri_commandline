# Command line tool for Colibri Module

# Usage
```
Usage: colibri [OPTIONS] COMMAND [ARGUMENTS]\
Commands:
  get INDEX           : get a value from the device
  set INDEX VALUE     : set a value in the device
  levelling           : prepares the module for a measurment
  save                : save the last measurement(s)
  calculate           : caluclate od's or concentrations
  measure             : starts a measurement and return the values
  version             : return the CLI and DLL version
  selftest            : executes an internal selftest
  fwupdate FILE       : loads a new firmware
  command COMMAND     : executes a command e.g colibri.exe command \"V 0\" returns the value at index 0
  help COMMAND        : Prints a detailed help

Options:
  --verbose           : prints debug info
  --help -h           : show this help and exit
  --device            : use the given device, if omitted the CLI searchs for a device
  --use-checksum      : use the protocol with a checksum
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
## Command save
```
Usage: colibri save [FILE]
  Saves the levelling data and the last measurements in the given file FILE as a csv file.
  If FILE is -, the data is printed to stdout.
  If FILE is omitted, an automatic file name is choosen: SN-YEAR-MONTH-DAY_HOUR_MIN_SEC.csv where SN is the serial number of the current colibri module.
```
## Command calculate od
```
Usage: colibri calculate od
  Calculates from the last two measurements stored in the colibri module the optical density (od).
Output:
  OD_230 OD_260 OD_280 OD_340
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
## Command version
```
Usage: colibri version
  Prints the version of this tool and the libcolibri to stdout.
```
## Command selftest
```
Usage: colibri selftest
  Executes a selftest and prints the result.
  If the result is not ok, the must common case is that the cuvette guide is blocking the optical path
  or a cuvette is stuck in the cuvette guide.
```
## Command fwupdate
```
Usage: colibri fwupdate SREC_FILE
  Updates the firmware.
```
## Command command
```
Usage: colibri command COMMAND
  Executes any colibri command. Usefull for testing.
```
