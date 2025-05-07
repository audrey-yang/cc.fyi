#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>

/**
 * @brief Resolves a hostname to an IPv4 address
 *
 * Uses getaddrinfo to resolve the specified hostname into an IPv4 address.
 * If multiple addresses are returned, it selects the first and prints a warning.
 *
 * @param address: Reference to an in_addr struct that will be filled with the resolved IP address
 * @param hostname: The hostname to resolve
 * @return int: 0 on success, or -1 if resolution fails
 */
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
            std::cerr << "traceroute: Warning: " << hostname << " has multiple addresses "
                      << "using " << inet_ntoa(address) << std::endl;
        }
        break;
    }

    freeaddrinfo(result);

    return 0;
}

/**
 * @brief Prints the hostname for a given IPv4 address
 *
 * Uses getnameinfo to perform a reverse DNS lookup on the provided address.
 * If successful, prints the hostname to std::cerr. Otherwise, prints an error message.
 *
 * @param address: sockaddr_in structure containing the IPv4 address to look up
 */
void print_hostname(struct sockaddr_in &address)
{
    char hostname[256];
    int res = getnameinfo((struct sockaddr *)&address, sizeof(address), hostname, 256, NULL, 0, NI_DGRAM);
    if (res)
    {
        std::cerr << gai_strerror(res) << std::endl;
        return;
    }
    std::cerr << "(" << hostname << ")";
}

/**
 * @brief Traces the network path (hops) to a specified IPv4 address
 *
 * Sends UDP packets with increasing TTL values toward the destination IP address.
 * Receives ICMP responses from intermediate routers to determine each hop along the route.
 * Measures and prints the round-trip time for each hop.
 *
 * @param server_ip: Pointer to the target IPv4 address
 * @param packet: Raw message to send in the UDP packet
 * @param packet_len: Length of the packet in bytes
 * @param max_iters: Maximum number of hops to trace before giving up
 * @return int: 0 on success (target reached), or -1 on failure
 */
int trace_ip_route(struct in_addr *server_ip, char *packet, int packet_len, int max_iters)
{
    // Open UDP socket for sending
    int sender_sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in send_addr;

    // Open ICMP socket for receiving with 5s timeout
    int receiver_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

    struct timeval tv;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    setsockopt(receiver_sock, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    int iter;
    for (iter = 1; iter <= max_iters; iter++)
    {
        // Set TTL
        int ttl = iter;
        setsockopt(sender_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

        // Send message
        send_addr.sin_addr = *server_ip;
        send_addr.sin_family = AF_INET;
        send_addr.sin_port = htons(33434);

        auto start = std::chrono::high_resolution_clock::now();
        int bytes_sent = sendto(sender_sock, packet, packet_len, 0, (sockaddr *)&send_addr, sizeof(sockaddr_in));
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

        auto duration = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(end - start);

        std::cerr << iter << "  " << inet_ntoa(recv_addr.sin_addr) << " ";
        print_hostname(recv_addr);
        std::cerr << " " << duration.count() << "ms" << std::endl;

        if (recv_addr.sin_addr.s_addr == server_ip->s_addr)
        {
            break;
        }
    }

    if (close(sender_sock) < 0)
    {
        perror("close");
        return -1;
    }

    if (close(receiver_sock) < 0)
    {
        perror("close");
        return -1;
    }

    if (iter > max_iters)
    {
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    struct in_addr address;
    if (get_ip_addr(address, argv[1]) < 0)
    {
        std::cerr << "Error: could not resolve hostname" << std::endl;
        return 1;
    }

    int max_iters = 64;
    char packet[32];
    strcpy(packet, "codingchallenges.fyi traceroute");

    std::cerr << "traceroute to " << argv[1] << " (" << inet_ntoa(address) << ") "
              << max_iters << " hops max, 32 byte packets" << std::endl;

    if (trace_ip_route(&address, packet, 32, max_iters) < 0)
    {
        std::cerr << "Error: could not reach " << argv[1] << " within "
                  << max_iters << " hops" << std::endl;
        return 1;
    }

    return 0;
}