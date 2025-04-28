#include "dockerhub.h"

static size_t cb(char *data, size_t size, size_t nmemb, void *clientp)
{
    size_t realsize = size * nmemb;
    memory *mem = (memory *)clientp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr)
    {
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

int get_token(char *token)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        char url[256];
        sprintf(url, "https://auth.docker.io/token?service=registry.docker.io&scope=repository:ubuntu:pull");

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION,
                         (long)CURL_HTTP_VERSION_2);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);

        memory chunk = {0};
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_cleanup(curl);

        if (sscanf(
                chunk.response,
                "%*[^:]:\"%5012[^\"]\",%*s",
                token) < 1)
        {
            return -1;
        }

        return 0;
    }

    return -1;
}
