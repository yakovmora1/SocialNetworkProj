#include "common.hpp"

char neeble_to_hex(uint8_t byte)
{
    char hex = 0;
    if(10 <= (byte & 0xf))
    {
        hex = ((byte - 10) + 'a');
    }
    else
    {
        hex = (byte + '0');
    }
    return hex;
}

/* debug function */
void print_payload(std::ostream &output, std::vector<uint8_t> payload)
{
    size_t i = 0;
    output << "Payload Raw: " << std::endl;
    for(i = 0; i < payload.size(); i++)
    {
        output << payload.at(i);
    }

    output << std::endl;
    output << "Payload hex: " << std::endl;
    for(i = 0; i < payload.size(); i++)
    {
        output << std::hex << static_cast<int>(payload.at(i)) << " ";
    }
    output << std::endl;
}

