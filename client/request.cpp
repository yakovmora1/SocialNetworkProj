#include "request.hpp"
#include <exception>




Request::Request(std::vector <uint8_t> client_id, uint16_t code)
{

    // TODO: write it beeter
    if(client_id.empty() )
    {
        std::cout <<"Invalid client id" <<std::endl;
        throw std::invalid_argument("Request: Bad arguments");
    }

    if( !_validate_code(code))
    {
        std::cout <<"Invalid code " << code << std::endl;
    }

    _client_id = client_id;
    _request_code = code;
    _version = SUPPORTED_VERSION;
    _payload_size = 0;

}


Request::Request(std::vector <uint8_t> client_id, uint16_t code,
                std::vector <uint8_t> payload)
{
    if(!_validate_code(code) || (client_id.empty() && code != CODE_REGISTER))
    {
        throw std::invalid_argument("Request: Bad arguments");
    }

    _client_id = client_id;
    _request_code = code;
    _version = SUPPORTED_VERSION;

    _payload_size = payload.size();
    //calss string's copy ctor
    _payload = payload;
}


std::vector<uint8_t> Request::build_request()
{
    size_t raw_data_size = _payload_size + FIELD_VERSION_SIZE + 
                            FIELD_CLIENT_ID_SIZE + FIELD_REQUEST_CODE_SIZE +
                            sizeof(_payload_size);
    _raw_request.insert(_raw_request.end(), _client_id.begin(), _client_id.end());


    _raw_request.push_back(_version);

    _raw_request.push_back((uint8_t) (_request_code >> 8));
    _raw_request.push_back((uint8_t) (_request_code & 0xff));
    
    _raw_request.push_back((uint8_t) (_payload_size >> 24));
    _raw_request.push_back((uint8_t) (_payload_size >> 16));
    _raw_request.push_back((uint8_t) (_payload_size >> 8));
    _raw_request.push_back((uint8_t) _payload_size);

    _raw_request.insert(_raw_request.end(), _payload.begin(), _payload.end());
    return  _raw_request;
}


const std::vector<uint8_t>  Request::get_raw_request(){
    std::cout << "raw_request size: " << _raw_request.size() << std::endl;
    return _raw_request;
}

//TOOD: change to error_code_t
bool Request::_validate_code(uint16_t code)
{
    uint16_t supported_codes[SUPPORTED_CODES_NUM] = SUPPORTED_CODES;
    size_t i = 0;

    for(i = 0; i < sizeof(supported_codes) / sizeof(uint16_t); i++)
    {
        if(code == supported_codes[i])
        {
            return true;
        }
    }

    return false;
}
