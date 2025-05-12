#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
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

void check_port(std::string &hostname, uint16_t port)
{
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    get_ip_addr(server.sin_addr, hostname);
    server.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)) == 0)
    {
        std::cout << "Port: " << port << " is open" << std::endl;
    }
    close(sockfd);
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
        std::cout << "Scanning host: " << result["host"].as<std::string>() << " ";
    }
    if (result.count("port") > 0)
    {
        std::cout << "port: " << result["port"].as<int>();
    }
    std::cout << std::endl;

    std::string hostname = result["host"].as<std::string>();
    int port = result["port"].as<int>();

    if (port < 0)
    {
        // Perform vanilla scan
        for (uint16_t port = 1; port != 0; port++)
        {
            check_port(hostname, port);
        }
    }
    else
    {
        check_port(hostname, port);
    }

    return 0;
}