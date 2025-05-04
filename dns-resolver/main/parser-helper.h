#pragma once
#include <iostream>
#include <cstdio>
#include <vector>
#include <sstream>
#include <string>
#include <unistd.h>

#include "dns-resolver.h"

uint64_t get_x_bytes(std::vector<uint8_t> &vec, int x, int offset);
std::string parse_dns_name(std::vector<uint8_t> &response, int &ind);
void parse_answer(Answer &answer, std::vector<uint8_t> &response, int &str_ind);
