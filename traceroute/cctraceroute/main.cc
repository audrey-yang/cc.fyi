#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>

int get_ip_addr(struct in_addr &address, char *hostname)
{
    struct addrinfo hints, *result, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_protocol = IPPROTO_UDP;

    int status = getaddrinfo(hostname, NULL, &hints, &result);
    if (status)
    {
        perror("getaddrinfo");
        return -1;
    }

    for (p = result; p != NULL; p = p->ai_next)
    {
        struct sockaddr_in *ai_addr = (struct sockaddr_in *)p->ai_addr;
        address.s_addr = ai_addr->sin_addr.s_addr;

        if (p->ai_next != NULL)
        {
            std::cerr << "traceroute: Warning: " << hostname << " has multiple addresses " << "using " << inet_ntoa(address) << std::endl;
        }
        break;
    }

    freeaddrinfo(result);

    return 0;
}

void print_hostname(struct sockaddr_in &address)
{
    char hostname[256];
    int res = getnameinfo((struct sockaddr *)&address, sizeof(address), hostname, 256, NULL, 0, NI_DGRAM);
    if (res)
    {
        std::cerr << gai_strerror(res) << std::endl;
    }
    std::cerr << "(" << hostname << ")";
}

int send_recv_message(struct in_addr *server_ip, char *packet, int max_iters)
{
    // Open UDP socket for sending
    int sender_sock = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in send_addr;
    send_addr.sin_addr = *server_ip;
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(33434);

    // Open ICMP socket for receiving
    int receiver_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

    for (int iter = 0; iter < max_iters; iter++)
    {
        // Set TTL
        int ttl = iter;
        setsockopt(sender_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 50000;
        setsockopt(receiver_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

        // Send message
        auto start = std::chrono::high_resolution_clock::now();
        int bytes_sent = sendto(sender_sock, packet, 32, 0, (sockaddr *)&send_addr, sizeof(send_addr));
        if (bytes_sent < 0)
        {
            perror("sendto");
            return -1;
        }

        // Receive response
        char response[128];
        struct sockaddr_in recv_addr;
        uint32_t recv_addr_len;
        int bytes_recv = recvfrom(receiver_sock, response, 65535, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
        auto end = std::chrono::high_resolution_clock::now();
        if (bytes_recv < 0)
        {
            std::cerr << iter << "  * * *" << std::endl;
            continue;
        }

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        std::cerr << iter << "  " << inet_ntoa(recv_addr.sin_addr) << " ";
        print_hostname(recv_addr);
        std::cerr << " " << duration.count() << "ms" << std::endl;

        if (recv_addr.sin_addr.s_addr == server_ip->s_addr)
        {
            break;
        }
    }

    close(sender_sock);
    close(receiver_sock);
    return 0;
}

int main(int argc, char **argv)
{
    struct in_addr address;
    if (get_ip_addr(address, argv[1]) < 0)
    {
        return 1;
    }

    fprintf(stderr, "traceroute to %s (%s), %d hops max, %d byte packets\n", argv[1], inet_ntoa(address), 64, 32);

    char packet[32];
    strcpy(packet, "codingchallenges.fyi traceroute");
    send_recv_message(&address, packet, 64);
    return 0;
}