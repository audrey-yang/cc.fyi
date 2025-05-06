#include <iostream>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

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

int send_recv_message(struct in_addr *server_ip, char *message)
{
    // Open UDP socket for sending
    int sender_sock = socket(AF_INET, SOCK_DGRAM, 0);

    // Set TTL
    int ttl = 1;
    setsockopt(sender_sock, IPPROTO_IP, IP_TTL, &ttl, sizeof(ttl));

    struct sockaddr_in send_addr;
    send_addr.sin_addr = *server_ip;
    send_addr.sin_family = AF_INET;
    send_addr.sin_port = htons(33530);

    // Send message
    int bytes_sent = sendto(sender_sock, message, 32, 0, (sockaddr *)&send_addr, sizeof(send_addr));
    if (bytes_sent < 0)
    {
        std::cerr << "Error: Could not send request" << std::endl;
        return -1;
    }
    close(sender_sock);

    // Open ICMP socket for receiving
    int receiver_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_ICMP);

    // Receive response
    char response[128];
    struct sockaddr_in recv_addr;
    uint32_t recv_addr_len;
    int bytes_recv = recvfrom(receiver_sock, response, 65535, 0, (struct sockaddr *)&recv_addr, &recv_addr_len);
    if (bytes_recv < 0)
    {
        return -1;
    }

    std::cerr << 1 << "\t" << inet_ntoa(recv_addr.sin_addr) << std::endl;
    close(receiver_sock);
    return 0;
}

int main(int argc, char **argv)
{
    struct in_addr *address = get_ip_addr(argv[1]);
    fprintf(stderr, "traceroute to %s (%s), %d hops max, %d byte packets\n", argv[1], inet_ntoa(*address), 64, 32);

    char message[32];
    strcpy(message, "codingchallenges.fyi traceroute");
    send_recv_message(address, message);
    return 0;
}