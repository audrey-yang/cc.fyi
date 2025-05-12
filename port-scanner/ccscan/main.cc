#include <iostream>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <cxxopts.hpp>

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
        std::cout << "port: " << result["port"].as<int>() << std::endl;
    }

    std::string hostname = result["host"].as<std::string>();
    if (hostname == "localhost")
    {
        hostname = "127.0.0.1";
    }
    int port = result["port"].as<int>();

    struct sockaddr_in server;
    if (inet_pton(AF_INET, hostname.c_str(), &(server.sin_addr)) == 1)
    {
        std::cout << server.sin_addr.s_addr << std::endl;
    }
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sockfd, (struct sockaddr *)&server, sizeof(server)))
    {
        std::cout << "Port: " << port << " is open" << std::endl;
    }
    else
    {
        std::cout << "Port: " << port << " is not open" << std::endl;
    }

    return 0;
}