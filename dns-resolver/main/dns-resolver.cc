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
        std::stringstream label_stream(question.QNAME);
        while (std::getline(label_stream, label, '.'))
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
    void parse_response(std::vector<uint8_t> response)
    {
        header.ID = response[0] << 8 | response[1];
        header.QR = (response[2] >> 7) & 0x01;
        header.OPCODE = (response[2] >> 3) & 0x0F;
        header.AA = (response[2] >> 2) & 0x01;
        header.TC = (response[2] >> 1) & 0x01;
        header.RD = (response[2] >> 1) & 0x01;
        header.RCODE = response[3] & 0x0F;
        header.QDCOUNT = response[4] << 8 | response[5];
        header.ANCOUNT = response[6] << 8 | response[7];
        header.NSCOUNT = response[8] << 8 | response[9];
        header.ARCOUNT = response[10] << 8 | response[11];

        printf("QDCOUNT %d, ANCOUNT %d, NSCOUNT %d, ARCOUNT %d\n", header.QDCOUNT, header.ANCOUNT, header.NSCOUNT, header.ARCOUNT);

        question.QNAME = "";

        int len = response[12];
        int str_ind = 13;
        while (len > 0)
        {
            for (int i = 0; i < len; i++)
            {
                question.QNAME.push_back(response[str_ind]);
                str_ind++;
            }
            question.QNAME.push_back('.');
            len = response[str_ind];
            str_ind++;
        }
        str_ind++;
        question.QTYPE = response[str_ind] << 8 | response[str_ind + 1];
        str_ind += 2;
        question.QCLASS = response[str_ind] << 8 | response[str_ind + 1];
        str_ind += 2;

        printf("QNAME %s, QTYPE %d, QCLASS %d\n", question.QNAME.c_str(), question.QTYPE, question.QCLASS);

        for (int i = 0; i < header.ANCOUNT; i++)
        {
            Answer answer;
            answers.push_back(answer);
            while (response[str_ind] != 0)
            {
                answer.NAME.push_back(char(response[str_ind]));
                str_ind++;
            }
            std::cout << "NAME: " << answer.NAME << std::endl;
            answer.TYPE = response[str_ind] << 8 | response[str_ind + 1];
            str_ind += 2;
            answer.CLASS = response[str_ind] << 8 | response[str_ind + 1];
            str_ind += 2;
            answer.TTL = response[str_ind] << 24 | response[str_ind + 1] << 16 | response[str_ind + 2] << 8 | response[str_ind + 3];
            str_ind += 4;
            answer.RDLENGTH = response[str_ind] << 8 | response[str_ind + 1];
            str_ind += 2;
            for (int j = 0; j < answer.RDLENGTH; j++)
            {
                answer.RDATA.push_back(response[str_ind]);
                str_ind++;
            }
            std::cout << "RDATA: ";
            for (uint8_t c : answer.RDATA)
            {
                printf("%02x", c);
            }
            std::cout << std::endl;
        }
    }

private:
    Header header;
    Question question;
    std::vector<Answer> answers;
};

std::vector<uint8_t> sent_to_ns(std::vector<uint8_t> message)
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
    std::cout << std::endl;

    std::vector<uint8_t> response_vec(response, response + bytes_recv);
    return response_vec;
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

    std::vector<uint8_t> response = sent_to_ns(message_bytes);
    message.parse_response(response);
}