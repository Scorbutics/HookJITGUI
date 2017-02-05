#include <string.h>
#include <stdio.h>

int FileExists(const char* pathFileName)
{
    FILE * file = fopen(pathFileName, "r");
    int result = file != NULL;
    fclose(file);

    return result;
}

int FindArgIndex(const char** argStr, unsigned int argc, const char* arg)
{
    unsigned int i;
    for(i = 1; i < argc; i++)
    {
        if(strstr(argStr[i], arg) == argStr[i])
        {
            return i;
        }
    }

    return -1;
}


const char* FindArg(const char** argStr, unsigned int argc, const char* arg)
{
    int index = FindArgIndex(argStr, argc, arg);
    if(index != -1)
    {
        return argStr[index];
    }
    return NULL;
}


const char* FindNextArg(const char** argStr, unsigned int argc, const char* arg)
{
    int argIndex = FindArgIndex(argStr, argc, arg);
    if(argIndex != -1 && ((unsigned int)argIndex + 1) < argc)
    {
        return argStr[argIndex + 1];
    }
    return NULL;
}

const char* FindExtensionPathName(const char** argStr, unsigned int argc, const char* ext)
{
    unsigned int i;
    for(i = 1; i < argc; i++)
    {
        if(strstr(argStr[i], ext) == (argStr[i] + strlen(argStr[i]) - strlen(ext)))
        {
            return argStr[i];
        }
    }
    return NULL;
}

