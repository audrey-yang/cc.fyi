#include <iostream>
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
        std::cout << "host: " << result["host"].as<std::string>() << std::endl;
    }
    if (result.count("port") > 0)
    {
        std::cout << "port: " << result["port"].as<int>() << std::endl;
    }

    return 0;
}