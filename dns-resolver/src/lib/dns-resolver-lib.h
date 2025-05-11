#pragma once
#include <iostream>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

/**
 * @brief DNS Header
 *
 * DNS header structure, as defined in RFC 1035 Section 4.1.1
 */
struct Header
{
  uint16_t ID = 22;
  bool QR = false;
  uint8_t OPCODE = 0;
  bool AA = false;
  bool TC = false;
  bool RD = false;
  bool RA = false;
  uint8_t RCODE = 0;
  uint16_t QDCOUNT = 1;
  uint16_t ANCOUNT = 0;
  uint16_t NSCOUNT = 0;
  uint16_t ARCOUNT = 0;
};

/**
 * @brief DNS Question
 *
 * DNS question structure, as defined in RFC 1035 Section 4.1.2
 */
struct Question
{
  std::string QNAME;
  uint16_t QTYPE = 1;
  uint16_t QCLASS = 1;
};

/**
 * @brief DNS Answer
 *
 * DNS answer structure, as defined in RFC 1035 Section 4.1.3
 */
struct Answer
{
  std::string NAME;
  uint16_t TYPE;
  uint16_t CLASS;
  uint32_t TTL;
  uint16_t RDLENGTH;
  std::vector<uint8_t> RDATA;
  std::string RDATA_STR;
};

/**
 * @brief DNS Message Response
 *
 * Parses and stores a DNS response message
 *
 * Public Methods:
 * - DNSMessageResponse(std::vector<uint8_t> raw_response): Constructor that parses a raw DNS response
 *
 * Public Members:
 * - bool isOk: Indicates if the response was successful
 * - std::vector<Answer> answers: A list of answer records
 * - std::vector<Answer> nss: A list of name server (authority) records
 * - std::vector<Answer> additionals: A list of additional resource records
 *
 * Private Members:
 * - Header header: The DNS header of the response
 * - Question question: The DNS question that was sent
 *
 * Private Methods:
 * - void parse_response(std::vector<uint8_t> response): Parses the raw DNS response and populates
 *    header, answers, nss, and additionals
 */
class DNSMessageResponse
{
public:
  DNSMessageResponse() = default;
  DNSMessageResponse(std::vector<uint8_t> raw_response)
  {
    parse_response(raw_response);
    isOk = true;
  }
  bool isOk = false;
  std::vector<Answer> answers;
  std::vector<Answer> nss;
  std::vector<Answer> additionals;

private:
  Header header;
  Question question;
  void parse_response(std::vector<uint8_t> response);
};

/**
 * @brief DNS Message Request
 *
 * Constructs and sends a DNS query message
 *
 * Public Methods:
 * - DNSMessageRequest(std::string name): Constructor that initializes a DNS query for a given domain name
 * - DNSMessageResponse send_to_ns(uint32_t query_ns): Sends the DNS query to a given nameserver
 *    (IPv4 address in network byte order) and returns the response
 *
 * Private Members:
 * - Header header: The DNS header of the request
 * - Question question: The DNS question to be sent
 *
 * Private Methods:
 * - std::vector<uint8_t> build_byte_string_message(): Constructs the DNS query as a raw byte string
 *    suitable for sending over UDP
 */
class DNSMessageRequest
{
public:
  DNSMessageRequest(std::string name) : question{std::move(name)} {}
  DNSMessageResponse send_to_ns(uint32_t query_ns);

private:
  Header header;
  Question question;
  std::vector<uint8_t> build_byte_string_message();
};

/**
 * @brief Resolves a domain name to an IPv4 address using a custom DNS resolver
 *
 * Sends a DNS query for the given domain name and parses the response to extract an IPv4 address
 *
 * @param query_name: The domain name to resolve
 * @return in_addr_t: The resolved IPv4 address in network byte order. Returns INADDR_NONE on failure.
 */
in_addr_t resolve_dns(const char *query_name);
