#include "single-scan-lib.h"

void *check_port_vanilla(void *params_in)
{
    port_scan_args *params = (port_scan_args *)params_in;
    uint16_t port = params->port;
    struct sockaddr_in server = params->server;
    server.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * 400;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == 0)
    {
        std::cout << "Port: " << port << " is open" << std::endl;
    }
    close(sockfd);
    pthread_exit(0);
    return NULL;
}

void *check_port_in_sweep_vanilla(void *params_in)
{
    port_scan_args *params = (port_scan_args *)params_in;
    uint16_t port = params->port;
    char *hostname = params->hostname;
    struct sockaddr_in server = params->server;
    server.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * 500;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == 0)
    {
        std::cout << "Host " << hostname << " is active on port 5000" << std::endl;
    }

    close(sockfd);
    pthread_exit(0);
    return NULL;
}