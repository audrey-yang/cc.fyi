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

int get_token(char *token, char *image_name)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        char url[256];
        sprintf(url, "https://auth.docker.io/token?service=registry.docker.io&scope=repository:library/%s:pull", image_name);

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

        json_t *root;
        json_error_t error;

        root = json_loads(chunk.response, 0, &error);

        json_t *token_value;
        token_value = json_object_get(root, "token");
        if (!json_is_string(token_value))
        {
            fprintf(stderr, "Error: %s\n", error.text);
            json_decref(root);
            return -1;
        }

        strcpy(token, json_string_value(token_value));

        return 0;
    }

    return -1;
}

int pull_manifests(char *image_name, char *token)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        char url[256];
        sprintf(url, "https://index.docker.io/v2/library/%s/manifests/latest", image_name);

        struct curl_slist *headers = NULL;
        char bearer[5112];
        sprintf(bearer, "Authorization: Bearer %s", token);
        headers = curl_slist_append(headers, bearer);
        headers = curl_slist_append(headers, "Accept: application/vnd.docker.distribution.manifest.v2+json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
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

        json_t *root;
        json_error_t error;

        root = json_loads(chunk.response, 0, &error);
        printf("received: %s\n", chunk.response);

        return 0;
    }

    return -1;
}

int pull_layers(char *image_name)
{
    char token[5012];
    if (get_token(token, image_name) < 0)
    {
        printf("Failed to get token\n");
        return -1;
    }
}
