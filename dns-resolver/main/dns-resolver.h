#include <cstdint>
#include <string>

struct Header
{
  uint16_t ID = 22;
  bool QR = false;
  uint8_t OPCODE = 0;
  bool AA = false;
  bool TC = false;
  bool RD = true;
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
