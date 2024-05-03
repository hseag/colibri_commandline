#include "cJSON.h"

#define DICT_MEASUREMENTS "measurements"
#define DICT_SERIALNUMBER "serialnumber"
#define DICT_FIRMWAREVERSION "firmwareVersion"

#define DICT_SAMPLE "sample"
#define DICT_REFERENCE "reference"

#define DICT_AMPLIFICATION_SAMPLE "amplificationSample"
#define DICT_AMPLIFICATION_REFERENCE "amplificationReference"
#define DICT_CURRENT "current"
#define DICT_RESULT "result"
#define DICT_RESULT_TEXT "resultText"

#define DICT_LEVELLING "levelling"
#define DICT_BASELINE "baseline"
#define DICT_AIR "air"
#define DICT_COMMENT "comment"

#define DICT_230 "230"
#define DICT_260 "260"
#define DICT_280 "280"
#define DICT_340 "340"

#define DICT_CALCULATED "calculated"
#define DICT_OD "od"
#define DICT_CONCENTRATION "concentration"

cJSON *colibriJsonLoad(char *file);
void colibriJsonSave(char* file, cJSON* json);