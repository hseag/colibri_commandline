
#include "colibriJson.h"
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

cJSON* colibriJsonLoad(char * file)
{
    FILE*  fin    = 0;
    char*  buffer = NULL;
    cJSON* json   = NULL;
    
    fin = fopen(file, "rb");

    if (fin)
    {
        struct stat st;
        stat(file, &st);

        buffer = malloc(st.st_size);

        size_t ret = fread(buffer, 1, st.st_size, fin);

        if (ret == st.st_size)
        {
            json = cJSON_Parse(buffer);
        }

        fclose(fin);
        free(buffer);
    }
    return json;
}

void colibriJsonSave(char* file, cJSON* json)
{
    FILE* fout   = 0;
    char* buffer = NULL;

    fout = fopen(file, "w+");

    buffer = cJSON_Print(json);

    fwrite(buffer, strlen(buffer), 1, fout);

    free(buffer);

    fclose(fout);
}
