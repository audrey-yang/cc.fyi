#include <iostream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <string>

#include "dns-resolver.h"

class DNSMessage
{
public:
    DNSMessage() = default;
    DNSMessage(std::string name) : question{std::move(name)} {}
    std::vector<uint8_t> buildMessage()
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

int main()
{
    DNSMessage message("dns.google.com");
    for (uint8_t c : message.buildMessage())
    {
        printf("%02x", c);
    }
    std::cout << std::endl;
}