#include <exception>
#include "response.hpp"



Response::Response(std::vector<uint8_t> data)
{
    _is_valid = false;
    std::cout << "Response size: " << data.size() << std::endl;

    if(data.size() < MIN_RESPONSE_SIZE)
    {
        std::cout << "Invalid response size received" << std::endl;
        _is_valid = false;
        return;
    }

    _version = data.at(0);
    if(SUPPORTED_VERSION !=  _version)
    {
        std::cout << "Received bad version: " << _version << std::endl;
        _is_valid = false;
        return;
    }

    _response_code = data.at(1) << 8;
    _response_code |= data.at(2);
    if(!_is_response_code_valid(_response_code))
    {
        _is_valid = false;
        std::cout << "Received bad response code: " 
                            << _response_code << std::endl;
        return;
    }
   
    _payload_size = data.at(3) << 24;
    _payload_size |= data.at(4) << 16;
    _payload_size |= data.at(5) << 8;
    _payload_size |= data.at(6);
    if(_payload_size != (data.size() - MIN_RESPONSE_SIZE))
    {
        _is_valid = false;
        std::cout << "payload size and response data size not match "
                    << _payload_size << std::endl;
        return;
    }

    _payload = std::vector<uint8_t>(data.begin() + MIN_RESPONSE_SIZE,
                                    data.end());
    _is_valid = true;
}


void Response::add_payload(std::vector<uint8_t> payload)
{
    // the default assignment operator handles the copy
    _payload = payload;
}


bool Response::_is_response_code_valid(uint16_t response_code)
{
    uint16_t response_codes[SUPPORTED_RESPONSES_NUM] = SUPPORTED_RESPONSES;
    size_t i = 0;

    for(i = 0; i <= sizeof(response_codes) / sizeof(uint16_t); i++)
    {
        if(response_code == response_codes[i])
            return true;
    }

    return false;
}


bool Response::is_error_response()
{
    return _response_code == CODE_ERROR;
}


