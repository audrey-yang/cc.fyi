#include "dockerhub.h"

// Helper function to write the cURL response data to a memory buffer
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

// Function to get the token from DockerHub for pulling the image
int get_token(char *token, char *image_name)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        // Set up the URL for the token request
        char url[256];
        sprintf(url, "https://auth.docker.io/token?service=registry.docker.io&scope=repository:library/%s:pull", image_name);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION,
                         (long)CURL_HTTP_VERSION_2);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb);

        memory chunk = {0};
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_cleanup(curl);

        // Parse the JSON response to extract the token
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

    fprintf(stderr, "Error: cURL setup failed while getting token\n");
    return -1;
}

// Function to pull the manifests list of the image in DockerHub and locate of the digest of the one matching our architecture
int pull_manifests(char *image_name, char *token, char *digest)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        // Set up the URL for the manifests request
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

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_cleanup(curl);

        // Parse the JSON response to extract the matching digest
        json_t *root;
        json_error_t error;

        root = json_loads(chunk.response, 0, &error);

        json_t *manifests;
        manifests = json_object_get(root, "manifests");

        size_t index;
        json_t *value;

        json_array_foreach(manifests, index, value)
        {
            json_t *digest_value, *platform_value;
            platform_value = json_object_get(value, "platform");
            json_t *architecture_value = json_object_get(platform_value, "architecture");
            char architecture[56];
            strcpy(architecture, json_string_value(architecture_value));

            if (strcmp(architecture, "amd64") == 0)
            {
                digest_value = json_object_get(value, "digest");
                if (!json_is_string(digest_value))
                {
                    fprintf(stderr, "Error: %s\n", error.text);
                    json_decref(root);
                    return -1;
                }
                strcpy(digest, json_string_value(digest_value));
                return 0;
            }
        }

        fprintf(stderr, "Error: couldn't find any image matching architecture to current system\n");
        return -1;
    }

    fprintf(stderr, "Error: cURL setup failed while pulling manifests\n");
    return -1;
}

// Helper function to write the cURL response data to a file
static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, (FILE *)stream);
    return written;
}

// Function to pull one layer of an image from DockerHub by digest
int pull_layer(char *image_name, char *token, char *digest)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        // Set up the URL for the layer request
        char url[256];
        sprintf(url, "https://index.docker.io/v2/library/%s/blobs/%s", image_name, digest);

        struct curl_slist *headers = NULL;
        char bearer[5112];
        sprintf(bearer, "Authorization: Bearer %s", token);
        headers = curl_slist_append(headers, bearer);
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_HTTP_VERSION,
                         (long)CURL_HTTP_VERSION_2);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, (long)1);

        // Set up file to write the layer data: /tmp/rootfs/layer.tar
        char file_name[2048];
        strcpy(file_name, "/tmp/rootfs/layer.tar");
        FILE *layer_file;
        layer_file = fopen(file_name, "wb");
        if (!layer_file)
        {
            printf("Failed to open file %s\n", file_name);
            return -1;
        }

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, layer_file);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return -1;
        }

        fclose(layer_file);
        curl_easy_cleanup(curl);

        // Extract the layer and suppress output
        char command[128];
        sprintf(command, "tar -xvf %s -C %s", file_name, "/tmp/rootfs --no-same-owner > /dev/null");
        int ret = system(command);
        if (ret)
        {
            printf("Error: Extracting layer after download failed\n");
            return -1;
        }

        // Clean up the layer tar
        system("rm /tmp/rootfs/layer.tar");
        return 0;
    }

    fprintf(stderr, "Error: cURL setup failed while pulling layer\n");
    return -1;
}

// Function to pull all layers of an image from DockerHub
int pull_layers(char *image_name, char *token)
{
    // Get the matching manifest digest for the image from the manifest list
    char digest[2048];
    if (pull_manifests(image_name, token, digest) < 0)
    {
        return -1;
    }

    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();

    if (curl)
    {
        // Set up the URL for the manifest request
        char url[5012];
        sprintf(url, "https://index.docker.io/v2/library/%s/manifests/%s", image_name, digest);

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

        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n",
                    curl_easy_strerror(res));
            curl_easy_cleanup(curl);
            return -1;
        }

        curl_easy_cleanup(curl);

        // Parse the JSON response to extract the layer digests
        json_t *root;
        json_error_t error;

        root = json_loads(chunk.response, 0, &error);

        json_t *layers;
        layers = json_object_get(root, "layers");

        size_t index;
        json_t *value;

        // Loop through the layers and process each one
        json_array_foreach(layers, index, value)
        {
            json_t *digest_value;
            digest_value = json_object_get(value, "digest");
            if (!json_is_string(digest_value))
            {
                fprintf(stderr, "Error: %s\n", error.text);
                json_decref(root);
                return -1;
            }
            strcpy(digest, json_string_value(digest_value));
            if (pull_layer(image_name, token, digest) < 0)
            {
                fprintf(stderr, "Failed to pull layer\n");
                json_decref(root);
                return -1;
            }
        }

        return 0;
    }

    fprintf(stderr, "Error: cURL setup failed while pulling image layers\n");
    return -1;
}

int pull_image(char *image_name)
{
    char token[5012];
    if (get_token(token, image_name) < 0)
    {
        return -1;
    }

    if (pull_layers(image_name, token) < 0)
    {
        return -1;
    }
    return 0;
}