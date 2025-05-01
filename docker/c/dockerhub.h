#include <stdio.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
#include <stdlib.h>
#include <jansson.h>

/**
 * @brief Structure to hold the response data from the cURL request.
 *
 * This structure contains a pointer to the response data and its size.
 *
 * @param response Pointer to the response data.
 * @param size Size of the response data.
 */
typedef struct memory_struct
{
    char *response;
    size_t size;
} memory;

/**
 * @brief Pulls a Docker image from Docker Hub.
 *
 * This function retrieves the image from Docker Hub and stores it in the local system at /tmp/rootfs.
 *
 * @param image_name The name of the Docker image to pull
 * @return 0 on success, -1 on failure
 */
int pull_image(char *image_name);
