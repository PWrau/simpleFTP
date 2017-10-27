#pragma once
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#define VERB_STR_MAX_LEN 10
#define VERB_NUM 2
#define MAX_STRING_LENGTH 8192 // 防止忘记在结尾加'\0'

char *verbStr[VERB_NUM] = {
    "USER",
    "PASS"
};

char verbVal[VERB_NUM] = {
    'U',
    'P'
};

typedef struct
{
    // considering storage, use char for value
    char* str;
    char val;
}Verb;

typedef struct
{
    size_t length;
    Verb* list;
}VerbList;

Verb createVerb(size_t order)
{
    // Having passed unit tests.
    Verb b = {"\0", '\0'};
    if (order >= VERB_NUM)
    {
        printf("Verb out of Range.\n");
        return b;
    }
    b.str = (char*)malloc(strlen(verbStr[order]) * sizeof(char));
    strcpy(b.str, verbStr[order]);
    b.val = verbVal[order];
    return b;
}

VerbList createVerbList()
{
    // Having passed unit tests.
    VerbList res;
    memset(&res, 0, sizeof(res));
    res.list = (Verb*)malloc(VERB_NUM * sizeof(Verb));
    res.length = VERB_NUM;
    for (size_t i = 0; i < res.length; i++)
    {
        res.list[i] = createVerb(i);
    }
    return res;
}

typedef struct
{
    Verb verb;
    char* param; // we assume that all params have one char
    char* content;
}Command;

void handleCommand(char* rawCommandStr)
{

}

int indexOf(char* src, char* pattern)
{
    // Having passed unit tests.
    size_t patternLen = strlen(pattern);
    for (size_t i = 0;; i++)
    {
        // avoid endless loop
        if (*(src + i) == '\0' || i > MAX_STRING_LENGTH)
        {
            return -1;
        }
        if (strncmp(src + i, pattern, patternLen) == 0)
        {
            // avoid warning
            return (int)i;
        }
    }
}