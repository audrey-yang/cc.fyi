#include "dns-resolver-lib.h"
#include "parser-lib.h"

std::vector<uint8_t> DNSMessageRequest::build_byte_string_message()
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

DNSMessageResponse DNSMessageRequest::send_to_ns(uint32_t query_ns)
{
    std::vector<uint8_t> message = build_byte_string_message();

    // Open socket, set timeout to 0.5s
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 500000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        perror("setsockopt");
    }

    struct sockaddr_in dns_server;
    dns_server.sin_addr.s_addr = query_ns;
    dns_server.sin_family = AF_INET;
    dns_server.sin_port = htons(53);

    // Send DNS message
    int bytes_sent = sendto(sockfd, &message[0], message.size(), 0, (sockaddr *)&dns_server, sizeof(dns_server));
    if (bytes_sent < 0)
    {
        std::cerr << "Error: Could not send request" << std::endl;
        return DNSMessageResponse();
    }

    // Receive response
    uint8_t response[65535];
    int bytes_recv = recvfrom(sockfd, response, 65535, 0, NULL, NULL);
    close(sockfd);

    if (bytes_recv < 0)
    {
        return DNSMessageResponse();
    }

    std::cerr << "Querying " << inet_ntoa(dns_server.sin_addr) << " for " << question.QNAME << "..." << std::endl;

    std::vector<uint8_t> response_vec(response, response + bytes_recv);
    return DNSMessageResponse(response_vec);
}

void DNSMessageResponse::parse_response(std::vector<uint8_t> response)
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

    // Question
    int ind = 12;
    question.QNAME = parse_dns_name(response, ind);
    question.QTYPE = get_x_bytes(response, 2, ind);
    ind += 2;
    question.QCLASS = get_x_bytes(response, 2, ind);
    ind += 2;

    // Parse answer section
    for (int i = 0; i < header.ANCOUNT; i++)
    {
        Answer answer;
        parse_answer(answer, response, ind);
        answers.push_back(answer);
    }

    // Parse authorities section
    for (int i = 0; i < header.NSCOUNT; i++)
    {
        Answer answer;
        parse_answer(answer, response, ind);
        nss.push_back(answer);
    }

    // Parse additional resources section
    for (int i = 0; i < header.ARCOUNT; i++)
    {
        Answer answer;
        parse_answer(answer, response, ind);
        additionals.push_back(answer);
    }
}

int resolve_dns_recursive(const char *query_name, uint32_t query_ns, in_addr_t *res_ip_addr)
{
    if (strlen(query_name) == 0)
    {
        return -1;
    }

    // Create request and send to nameserver
    DNSMessageRequest req(query_name);
    DNSMessageResponse res = req.send_to_ns(query_ns);
    if (!res.isOk)
    {
        return -1;
    }

    // Return if we find an answer
    if (res.answers.size() > 0)
    {
        *res_ip_addr = htonl(get_x_bytes(res.answers[0].RDATA, 4, 0));
        return 0;
    }

    // Try to resolve nameservers to ask
    for (Answer ns : res.nss)
    {
        // Try to find matching A record in additional resources
        int matching_records = 0;
        for (Answer add : res.additionals)
        {
            if (add.TYPE == 1 && ns.RDATA_STR.compare(add.NAME) == 0)
            {
                uint32_t next_ip_addr = htonl(get_x_bytes(add.RDATA, add.RDLENGTH, 0));
                if (resolve_dns_recursive(query_name, next_ip_addr, res_ip_addr) == 0)
                {
                    return 0;
                }
                matching_records++;
            }
        }

        // Try to discover IP address if not in additionals
        if (matching_records == 0)
        {
            uint32_t next_ip_addr;
            if (resolve_dns_recursive(ns.RDATA_STR.c_str(), inet_addr("198.41.0.4"), &next_ip_addr) == 0)
            {
                if (resolve_dns_recursive(query_name, next_ip_addr, res_ip_addr) == 0)
                {
                    return 0;
                }
            }
        }
    }

    return -1;
}

in_addr_t resolve_dns(const char *query_name)
{
    in_addr_t query_ns = inet_addr("198.41.0.4");
    in_addr_t ip_addr;
    if (resolve_dns_recursive(query_name, query_ns, &ip_addr) == 0)
    {
        return ip_addr;
    }

    std::cerr << "Error: could not resolve " << query_name << std::endl;
    return INADDR_NONE;
}