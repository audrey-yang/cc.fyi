#include <iostream>
#include <cxxopts.hpp>

#include "src/lib/port-scanner-lib.h"

int main(int argc, char **argv)
{
    std::cout << "hello" << std::endl;
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
            full_scan(hostname, port);
        }
    }
    else
    {
        sweep_scan(hostnames);
    }
    std::cout << "Scan complete" << std::endl;

    return 0;
}