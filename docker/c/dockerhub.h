#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>
#include <jansson.h>

typedef struct memory_struct
{
    char *response;
    size_t size;
} memory;

int get_token(char *token, char *image_name);
int pull_layers(char *image_name, char *token);
