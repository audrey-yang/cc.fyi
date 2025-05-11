#include <gtest/gtest.h>
#include <netdb.h>
#include "src/lib/dns-resolver-lib.h"

void test_resolve_dns(const char *hostname, in_addr_t our_dns)
{
    in_addr_t their_dns = INADDR_NONE;
    struct addrinfo hints, *result, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status == 0)
    {
        for (p = result; p != NULL; p = p->ai_next)
        {
            struct sockaddr_in *ai_addr = (struct sockaddr_in *)p->ai_addr;
            if (ai_addr->sin_addr.s_addr == our_dns)
            {
                their_dns = ai_addr->sin_addr.s_addr;
                break;
            }
        }
    }

    freeaddrinfo(result);
    EXPECT_EQ(our_dns, their_dns);
}

TEST(DNSResolverTest, ResolveGoogle)
{
    test_resolve_dns("dns.google.com", resolve_dns("dns.google.com"));
}

TEST(DNSResolverTest, ResolveUnknownHost)
{
    test_resolve_dns("idontknow", resolve_dns("idontknow"));
}