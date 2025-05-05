#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>

struct in_addr *get_ip_addr(char *hostname)
{
    struct hostent *host = gethostbyname(hostname);
    struct in_addr *address = NULL;
    char **ip_addr_list = host->h_addr_list;
    for (int i = 0; ip_addr_list[i] != NULL; i++)
    {
        address = (struct in_addr *)ip_addr_list[i];
        break;
    }
    return address;
}

int main(int argc, char **argv)
{
    struct in_addr *address = get_ip_addr(argv[1]);
    fprintf(stderr, "traceroute to %s (%s), %d hops max, %d byte packets\n", argv[1], inet_ntoa(*address), 64, 32);
}