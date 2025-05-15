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
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

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
    struct sockaddr_in server;
    uint16_t port;
    char hostname[16];
};

void *check_port(void *params_in)
{
    struct check_port_params *params = (struct check_port_params *)params_in;
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

void vanilla_scan(std::string &hostname, int port)
{
    std::cerr << "Scanning host: " << hostname << " ";
    if (port >= 0)
    {
        std::cerr << "port: " << port;
    }
    std::cerr << std::endl;

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    get_ip_addr(server_addr.sin_addr, hostname);

    if (port < 0)
    {
        pthread_t threads[65535];
        for (uint16_t i = 1; i != 0; i++)
        {
            struct check_port_params params;
            params.server.sin_family = AF_INET;
            params.server.sin_addr.s_addr = server_addr.sin_addr.s_addr;
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
        struct check_port_params params;
        params.server.sin_family = AF_INET;
        params.server.sin_addr.s_addr = server_addr.sin_addr.s_addr;
        params.port = port;
        check_port(&params);
    }
}

void *check_port_in_sweep(void *params_in)
{
    struct check_port_params *params = (struct check_port_params *)params_in;
    uint16_t port = params->port;
    char *hostname = params->hostname;
    struct sockaddr_in server = params->server;
    server.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 1000 * 400;
    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == 0)
    {
        std::cout << "Host " << hostname << " is active" << std::endl;
    }

    close(sockfd);
    pthread_exit(0);
    return NULL;
}

void sweep_scan(std::vector<std::string> &hostnames)
{
    pthread_t threads[65535];
    for (int i = 0; i < hostnames.size(); i++)
    {
        auto &hostname = hostnames[i];
        struct check_port_params params;
        params.server.sin_family = AF_INET;
        get_ip_addr(params.server.sin_addr, hostname);
        strcpy(params.hostname, hostname.c_str());
        params.port = 5000;

        if (pthread_create(&threads[i], NULL, check_port_in_sweep, (void *)&params))
        {
            std::cerr << "Error creating thread " << i << std::endl;
        }
    }

    for (int i = 0; i < hostnames.size(); i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            perror("pthread_join");
            std::cerr << "Error joining thread " << i << std::endl;
        }
    }
}

void sweep_scan_from_wildcard(std::string hostname)
{
    // Process wildcard
    std::vector<std::string> octets;
    size_t start = 0;
    size_t end = hostname.find('.');

    while (end != std::string::npos)
    {
        octets.push_back(hostname.substr(start, end - start));
        start = end + 1;
        end = hostname.find('.', start);
    }
    octets.push_back(hostname.substr(start));

    int octet1_start = octets[0] == "*" ? 0 : std::stoi(octets[0]);
    int octet1_end = octets[0] == "*" ? 256 : octet1_start + 1;
    int octet2_start = octets[1] == "*" ? 0 : std::stoi(octets[1]);
    int octet2_end = octets[1] == "*" ? 256 : octet1_start + 1;
    int octet3_start = octets[2] == "*" ? 0 : std::stoi(octets[2]);
    int octet3_end = octets[2] == "*" ? 256 : octet1_start + 1;
    int octet4_start = octets[3] == "*" ? 0 : std::stoi(octets[3]);
    int octet4_end = octets[3] == "*" ? 5 : octet1_start + 1;

    std::vector<pthread_t> threads;
    int num_threads = 0;
    for (; octet1_start < octet1_end; octet1_start++)
    {
        for (; octet2_start < octet2_end; octet2_start++)
        {
            for (; octet3_start < octet3_end; octet3_start++)
            {
                for (; octet4_start < octet4_end; octet4_start++)
                {
                    std::string ip_addr = std::to_string(octet1_start) + "." + std::to_string(octet2_start) +
                                          "." + std::to_string(octet3_start) + "." + std::to_string(octet4_start);

                    struct check_port_params *params = (struct check_port_params *)malloc(sizeof(struct check_port_params));
                    strcpy(params->hostname, ip_addr.c_str());
                    params->server.sin_family = AF_INET;
                    inet_pton(AF_INET, ip_addr.c_str(), &params->server.sin_addr);
                    params->port = 5000;

                    pthread_t thread;
                    if (pthread_create(&thread, NULL, check_port_in_sweep, (void *)params))
                    {
                        std::cerr << "Error creating thread " << num_threads << std::endl;
                    }
                    threads.push_back(thread);
                    num_threads++;
                }
            }
        }
    }

    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_join(threads[i], NULL))
        {
            std::cerr << "Error joining thread " << i << std::endl;
        }
    }
}

int main(int argc, char **argv)
{
    cxxopts::Options options("ccscan", "Port scanner");
    options.add_options()(
        "host", "host", cxxopts::value<std::vector<std::string>>())(
        "port", "port number", cxxopts::value<int>()->default_value("-1"));

    auto result = options.parse(argc, argv);
    std::vector<std::string> hostnames = result["host"].as<std::vector<std::string>>();
    int port = result["port"].as<int>();

    if (hostnames.size() == 1)
    {
        std::string hostname = hostnames[0];
        if (hostname.find('*') != std::string::npos)
        {
            sweep_scan_from_wildcard(hostname);
        }
        else
        {
            vanilla_scan(hostname, port);
        }
    }
    else
    {
        sweep_scan(hostnames);
    }

    return 0;
}