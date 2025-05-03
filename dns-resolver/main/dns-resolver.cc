#include <iostream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include "dns-resolver.h"

class DNSMessage
{
public:
    DNSMessage() = default;
    DNSMessage(std::string name) : question{std::move(name)} {}
    std::vector<uint8_t> build_byte_string_message()
    {
        std::vector<uint8_t> message;

        // Two bytes of the id
        message.push_back((header.ID & 0xFF00) >> 8);
        message.push_back(header.ID & 0x00FF);

        // Two bytes for the flags
        message.push_back(((header.QR << 7) | (header.OPCODE << 3) | (header.AA << 2) | (header.TC << 1) | header.RD));
        message.push_back((header.RCODE));

        // Two bytes each for the number of questions, answer resource records, authority resource records, and additional resource records
        message.push_back((header.QDCOUNT) >> 8);
        message.push_back(header.QDCOUNT);
        message.push_back((header.ANCOUNT) >> 8);
        message.push_back(header.ANCOUNT);
        message.push_back((header.NSCOUNT) >> 8);
        message.push_back(header.NSCOUNT);
        message.push_back((header.ARCOUNT) >> 8);
        message.push_back(header.ARCOUNT);

        // The encoded byte string for the question
        std::string label;
        std::stringstream labelStream(question.QNAME);
        while (std::getline(labelStream, label, '.'))
        {
            message.push_back(static_cast<uint8_t>(label.length()));
            for (char c : label)
            {
                message.push_back(static_cast<uint8_t>(c));
            }
        }
        message.push_back(0); // null terminate QNAME

        // Two bytes for the query type
        message.push_back((question.QTYPE) >> 8);
        message.push_back(question.QTYPE);

        // Two bytes for the query class
        message.push_back((question.QCLASS) >> 8);
        message.push_back(question.QCLASS);

        return message;
    }

private:
    Header header;
    Question question;
};

int sent_to_ns(std::vector<uint8_t> message)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in dns_server;
    dns_server.sin_addr.s_addr = inet_addr("8.8.8.8");
    dns_server.sin_family = AF_INET;
    dns_server.sin_port = htons(53);

    int bytes_sent = sendto(sockfd, &message[0], message.size(), 0, (sockaddr *)&dns_server, sizeof(dns_server));
    std::cout << bytes_sent << std::endl;

    uint8_t response[65535];
    int bytes_recv = recvfrom(sockfd, response, 65535, 0, NULL, NULL);
    std::cout << bytes_recv << std::endl;

    close(sockfd);

    for (int i = 0; i < bytes_recv; i++)
    {
        printf("%02x", response[i]);
    }

    return 0;
}

int main()
{
    DNSMessage message("dns.google.com");
    std::vector<uint8_t> message_bytes = message.build_byte_string_message();
    for (uint8_t c : message_bytes)
    {
        printf("%02x", c);
    }
    std::cout << std::endl;

    sent_to_ns(message_bytes);
}