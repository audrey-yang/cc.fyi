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

uint64_t get_x_bytes(std::vector<uint8_t> &vec, int x, int offset)
{
    uint64_t res = 0;
    for (int i = 0; i < x; i++)
    {
        res |= vec[offset + i] << (8 * (x - 1 - i));
    }
    return res;
}

std::string get_dns_name(std::vector<uint8_t> &response, int &ind)
{
    std::string name;
    int len = response[ind++];
    while (1)
    {
        if (len >> 6 == 3) // check if we have a pointer
        {
            int new_ind = (len & 0x3F) << 8 | response[ind++];
            printf("got a pointer to %d... ", new_ind);
            name.append(get_dns_name(response, new_ind));
            break;
        }

        for (int i = 0; i < len; i++)
        {
            name.push_back(response[ind++]);
        }

        len = response[ind++];
        if (len == 0)
        {
            break;
        }
        name.push_back('.');
    }
    return name;
}

void read_answer(Answer &answer, std::vector<uint8_t> &response, int &str_ind)
{
    answer.NAME = get_dns_name(response, str_ind);
    std::cout << "NAME: " << answer.NAME << std::endl;
    answer.TYPE = get_x_bytes(response, 2, str_ind);
    str_ind += 2;
    answer.CLASS = get_x_bytes(response, 2, str_ind);
    str_ind += 2;
    answer.TTL = get_x_bytes(response, 4, str_ind);
    str_ind += 4;
    answer.RDLENGTH = get_x_bytes(response, 2, str_ind);
    str_ind += 2;

    if (answer.TYPE == 2)
    {
        std::string dns_name = get_dns_name(response, str_ind);
        std::vector<uint8_t> dns_name_vec(dns_name.begin(), dns_name.end());
        answer.RDATA = dns_name_vec;
    }
    else
    {
        for (int j = 0; j < answer.RDLENGTH; j++)
        {
            answer.RDATA.push_back(response[str_ind]);
            str_ind++;
        }
    }

    std::cout << "RDATA " << answer.TYPE << ", " << answer.CLASS << ": ";
    for (uint8_t c : answer.RDATA)
    {
        if (answer.TYPE == 2)
        {
            printf("%c", char(c));
        }
        else
        {
            printf("%d", c);
        }
    }
    std::cout << std::endl;
}

class DNSMessage
{
public:
    DNSMessage() = default;
    DNSMessage(std::string name) : question{std::move(name)} {}
    uint32_t query_nameserver = 0;

    std::vector<uint8_t> build_byte_string_message()
    {
        std::vector<uint8_t> message;

        // Two bytes of the id
        message.push_back((header.ID & 0xFF00) >> 8);
        message.push_back(header.ID & 0x00FF);

        // Two bytes for the flags
        message.push_back(((header.QR << 7) | (header.OPCODE << 3) | (header.AA << 2) | (header.TC << 1) | header.RD));
        message.push_back(header.RCODE & 0x00FF);

        // Two bytes each for the number of questions, answer resource records, authority resource records, and additional resource records
        message.push_back((header.QDCOUNT & 0xFF00) >> 8);
        message.push_back(header.QDCOUNT);
        message.push_back((header.ANCOUNT & 0xFF00) >> 8);
        message.push_back(header.ANCOUNT);
        message.push_back((header.NSCOUNT & 0xFF00) >> 8);
        message.push_back(header.NSCOUNT);
        message.push_back((header.ARCOUNT & 0xFF00) >> 8);
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
        // Two bytes of the id
        header.ID = get_x_bytes(response, 2, 0);
        header.QR = (response[2] >> 7) & 0x01;

        // Two bytes for the flags
        header.OPCODE = (response[2] >> 3) & 0x0F;
        header.AA = (response[2] >> 2) & 0x01;
        header.TC = (response[2] >> 1) & 0x01;
        header.RD = response[2] & 0x01;
        header.RCODE = response[3] & 0x0F;

        // Two bytes each for the number of questions, answer resource records, authority resource records, and additional resource records
        header.QDCOUNT = get_x_bytes(response, 2, 4);
        header.ANCOUNT = get_x_bytes(response, 2, 6);
        header.NSCOUNT = get_x_bytes(response, 2, 8);
        header.ARCOUNT = get_x_bytes(response, 2, 10);

        printf("QDCOUNT %d, ANCOUNT %d, NSCOUNT %d, ARCOUNT %d\n", header.QDCOUNT, header.ANCOUNT, header.NSCOUNT, header.ARCOUNT);

        // The question
        int str_ind = 12;
        question.QNAME = get_dns_name(response, str_ind);
        question.QTYPE = get_x_bytes(response, 2, str_ind);
        str_ind += 2;
        question.QCLASS = get_x_bytes(response, 2, str_ind);
        str_ind += 2;

        printf("QNAME %s, QTYPE %d, QCLASS %d\n", question.QNAME.c_str(), question.QTYPE, question.QCLASS);

        std::cout << std::endl
                  << "ANSWER" << std::endl;
        for (int i = 0; i < header.ANCOUNT; i++)
        {
            Answer answer;
            answers.push_back(answer);
            read_answer(answer, response, str_ind);
        }

        std::cout << std::endl
                  << "NS" << std::endl;
        for (int i = 0; i < header.NSCOUNT; i++)
        {
            Answer answer;
            nss.push_back(answer);
            read_answer(answer, response, str_ind);
        }

        std::cout << std::endl
                  << "ADDITIONAL" << std::endl;
        for (int i = 0; i < header.ARCOUNT; i++)
        {
            Answer answer;
            additionals.push_back(answer);
            read_answer(answer, response, str_ind);

            for (Answer ns : nss)
            {
                if (ns.NAME.compare(answer.NAME) && answer.TYPE == 1)
                {
                    uint64_t converted_rdata = 0;
                    for (int k = 0; k < answer.RDLENGTH; k++)
                    {
                        converted_rdata |= answer.RDATA[k] << (answer.RDLENGTH - k);
                    }
                    query_nameserver = converted_rdata;
                }
            }
        }
    }

private:
    Header header;
    Question question;
    std::vector<Answer> answers;
    std::vector<Answer> nss;
    std::vector<Answer> additionals;
};

std::vector<uint8_t> sent_to_ns(std::vector<uint8_t> message, uint32_t query_ns)
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in dns_server;
    dns_server.sin_addr.s_addr = query_ns;
    dns_server.sin_family = AF_INET;
    dns_server.sin_port = htons(53);

    int bytes_sent = sendto(sockfd, &message[0], message.size(), 0, (sockaddr *)&dns_server, sizeof(dns_server));
    std::cout << "Sent " << bytes_sent << " bytes to " << inet_ntoa(dns_server.sin_addr) << std::endl;

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
    uint32_t query_ns = inet_addr("198.41.0.4");

    while (1)
    {
        DNSMessage message("dns.google.com");
        std::vector<uint8_t> message_bytes = message.build_byte_string_message();
        for (int i = 0; i < message_bytes.size(); i++)
        {
            printf("%02x", message_bytes[i]);
        }
        std::cout << std::endl;
        std::vector<uint8_t> response = sent_to_ns(message_bytes, query_ns);
        message.parse_response(response);
        if (message.query_nameserver > 0)
        {
            query_ns = message.query_nameserver;
        }
    }
}