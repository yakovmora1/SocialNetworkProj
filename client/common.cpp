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