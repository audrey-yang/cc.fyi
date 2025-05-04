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

struct Question
{
  std::string QNAME;
  uint16_t QTYPE = 1;
  uint16_t QCLASS = 1;
};

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

in_addr_t resolve_dns(const char *query_name);
