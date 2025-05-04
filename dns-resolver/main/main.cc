#include "dns-resolver.h"

int main(int argc, char **argv)
{
    std::cout << "Hello! Resolving DNS for " << argv[1] << std::endl;
    resolve_dns(argv[1]);
}