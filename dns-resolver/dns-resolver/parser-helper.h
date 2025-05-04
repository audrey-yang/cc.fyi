#pragma once
#include <iostream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <string>
#include <unistd.h>

#include "dns-resolver.h"

/**
 * @brief Gets the next x bytes of a vector
 *
 * Gets the next x bytes of a vector and contiguously populates an integer.
 * Meant to be used with x <= 8.
 *
 * @param vec: Byte vector from which to parse
 * @param x: Number of bytes to parse
 * @param offset: Current location in the vector
 * @return uint64_t: Integer combining x bytes from the vector
 */
uint64_t get_x_bytes(std::vector<uint8_t> &vec, int x, int offset);

/**
 * @brief Parses a DNS name
 *
 * Parses and uncompresses a DNS name from a response, as specified in
 * RFC 1035 Section 4.1.4
 *
 * @param response: Byte vector from which to parse
 * @param ind: Reference to the current location in the vector
 * @return string: Parsed and uncompressed DNS name
 */
std::string parse_dns_name(std::vector<uint8_t> &response, int &ind);

/**
 * @brief Parses an answer from a DNS response
 *
 * Parses an answer from a DNS response by the definition specified in
 * RFC 1035 Section 4.1.3
 *
 * @param answer: Reference to the Answer to populate
 * @param response: Byte vector from which to parse
 * @param ind: Reference to the current location in the vector
 */
void parse_answer(Answer &answer, std::vector<uint8_t> &response, int &ind);
