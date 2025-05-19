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

void *check_port_syn(void *params_in)
{
    port_scan_args *params = (port_scan_args *)params_in;
    uint16_t port = params->port;
    struct sockaddr_in server = params->server;
    server.sin_port = htons(port);

    struct sockaddr_in my_addr;
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    my_addr.sin_port = htons(326); // random unassigned port

    int sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror("socket");
        return NULL;
    }
    if (bind(sockfd, (const sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        perror("bind");
    }
    std::cout << "bound" << std::endl;

    char buffer[44];
    memset(buffer, 0, 44);
    struct iphdr *iph = (struct iphdr *)buffer;
    struct tcphdr *tcph = (struct tcphdr *)(buffer + sizeof(struct iphdr));

    // Create IP header
    iph->iph_ihl = 5;
    iph->iph_ver = 4;
    iph->iph_tos = 16; // Low delay
    iph->iph_len = 44; // IP + TCP
    iph->iph_ident = 0;
    iph->iph_ttl = 64;
    iph->iph_protocol = 6; // TCP
    iph->iph_sourceip = my_addr.sin_addr.s_addr;
    iph->iph_destip = server.sin_addr.s_addr;

    // Create TCP header
    tcph->th_sport = my_addr.sin_port;
    tcph->th_dport = server.sin_port;
    tcph->th_seq = 1;
    tcph->th_flags = TH_SYN;

    if (sendto(sockfd, buffer, iph->iph_len, 0, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("sendto");
    }
    std::cout << "sent" << std::endl;

    char recv_buffer[44];
    struct sockaddr_in recv_addr;
    uint32_t recv_addr_len;
    if (recvfrom(sockfd, recv_buffer, iph->iph_len, 0, (struct sockaddr *)&recv_addr, &recv_addr_len) < 0)
    {
        perror("sendto");
    }
    std::cout << "received" << std::endl;

    std::cout << recv_buffer << std::endl;

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