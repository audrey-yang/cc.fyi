#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>
#include <cxxopts.hpp>

int get_ip_addr(struct in_addr &address, std::string &hostname)
{
    struct addrinfo hints, *result, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int status = getaddrinfo(hostname.c_str(), NULL, &hints, &result);
    if (status)
    {
        perror("getaddrinfo");
        return -1;
    }

    for (p = result; p != NULL; p = p->ai_next)
    {
        struct sockaddr_in *ai_addr = (struct sockaddr_in *)p->ai_addr;
        address.s_addr = ai_addr->sin_addr.s_addr;
        break;
    }

    freeaddrinfo(result);

    return 0;
}

struct check_port_params
{
    struct sockaddr_in *server;
    uint16_t port;
};

void *check_port(void *params_in)
{
    struct check_port_params *params = (struct check_port_params *)params_in;
    uint16_t port = params->port;
    struct sockaddr_in *server = params->server;
    server->sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * 350;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    if (connect(sockfd, (struct sockaddr *)server, sizeof(*server)) == 0)
    {
        std::cout << "Port: " << port << " is open" << std::endl;
    }
    close(sockfd);
    pthread_exit(0);
    return NULL;
}

int main(int argc, char **argv)
{
    cxxopts::Options options("ccscan", "Port scanner");
    options.add_options()(
        "host", "host", cxxopts::value<std::string>())(
        "port", "port number", cxxopts::value<int>()->default_value("-1"));

    auto result = options.parse(argc, argv);

    if (result.count("host") > 0)
    {
        std::cerr << "Scanning host: " << result["host"].as<std::string>() << " ";
    }
    if (result.count("port") > 0)
    {
        std::cerr << "port: " << result["port"].as<int>();
    }
    std::cerr << std::endl;

    std::string hostname = result["host"].as<std::string>();
    int port = result["port"].as<int>();

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    get_ip_addr(server_addr.sin_addr, hostname);

    struct check_port_params params;
    params.server = &server_addr;

    if (port < 0)
    {
        // Perform vanilla scan
        pthread_t threads[65535];
        for (uint16_t i = 1; i != 0; i++)
        {
            params.port = i;
            if (pthread_create(&threads[i - 1], NULL, check_port, (void *)&params))
            {
                std::cerr << "Error creating thread " << i - 1 << std::endl;
            }
        }
        for (uint16_t i = 1; i != 0; i++)
        {
            if (pthread_join(threads[i - 1], NULL))
            {
                std::cerr << "Error joining thread " << i - 1 << std::endl;
            }
        }
    }
    else
    {
        params.port = port;
        check_port(&params);
    }

    return 0;
}