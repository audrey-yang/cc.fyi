#include "parser-helper.h"

uint64_t get_x_bytes(std::vector<uint8_t> &vec, int x, int offset)
{
    uint64_t res = 0;
    for (int i = 0; i < x; i++)
    {
        res |= vec[offset + i] << (8 * (x - 1 - i));
    }
    return res;
}

std::string parse_dns_name(std::vector<uint8_t> &response, int &ind)
{
    std::string name;
    int len = response[ind++];
    while (1)
    {
        if (len >> 6 == 3) // Check if we have a pointer (first 2 bits are 1)
        {
            int new_ind = (len & 0x3F) << 8 | response[ind++];
            name.append(parse_dns_name(response, new_ind));
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

        // Repopulate . from compression
        name.push_back('.');
    }
    return name;
}

void parse_answer(Answer &answer, std::vector<uint8_t> &response, int &ind)
{
    answer.NAME = parse_dns_name(response, ind);
    answer.TYPE = get_x_bytes(response, 2, ind);
    ind += 2;
    answer.CLASS = get_x_bytes(response, 2, ind);
    ind += 2;
    answer.TTL = get_x_bytes(response, 4, ind);
    ind += 4;
    answer.RDLENGTH = get_x_bytes(response, 2, ind);
    ind += 2;

    if (answer.TYPE == 2) // NS RDATA sections are DNS names
    {
        answer.RDATA_STR = parse_dns_name(response, ind);
    }
    else
    {
        for (int j = 0; j < answer.RDLENGTH; j++)
        {
            answer.RDATA.push_back(response[ind]);
            ind++;
        }
    }
}