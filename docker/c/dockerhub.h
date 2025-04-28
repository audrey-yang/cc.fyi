#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>

typedef struct memory_struct
{
    char *response;
    size_t size;
} memory;

int get_token(char *token);