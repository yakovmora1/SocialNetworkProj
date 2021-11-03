#include <vector>
#include <string>
#include <iostream>
#include "common.hpp"

#ifndef _RESPONSE_H
#define _RESPONSE_H

#define MIN_RESPONSE_SIZE (7)
typedef enum supported_response_e
{
    CODE_REGISTRATION_SUCCESS = 2000,
    CODE_CLIENTS_LIST,
    CODE_PUBLIC_KEY,
    CODE_MSG_SENT,
    CODE_MSG_RECEIVED,
    CODE_ERROR = 9000,

} supported_response_t;

#define SUPPORTED_RESPONSES {CODE_REGISTRATION_SUCCESS, CODE_CLIENTS_LIST, \
                            CODE_PUBLIC_KEY, CODE_MSG_SENT, CODE_MSG_RECEIVED, \
                            CODE_ERROR}
#define SUPPORTED_RESPONSES_NUM (6)



class Response
{
    private:
        std::vector<uint8_t> _payload;
        size_t _payload_size;
        uint8_t _version;
        uint16_t _response_code;
        bool _is_valid;
        bool _is_response_code_valid(uint16_t response_code = 0);

    public:
        //Response(Response & resp);
        Response(const std::vector<uint8_t>  data);

        void add_payload(std::vector<uint8_t> payload);
        bool is_response_valid()
        {
            return _is_valid;
        }

        bool is_error_response();

        std::vector<uint8_t> get_payload()
        {
            return _payload;
        }

        size_t get_payload_size()
        {
            return _payload_size;
        }

        uint16_t get_response_code()
        {
            return _response_code;
        }

        friend std::ostream & operator<<(std::ostream &output, const Response & resp)
        {
            size_t i = 0;

            output << "###########Response ojbect######### " << std::endl;
            output << "response code: " << resp._response_code << std::endl;
            output << "version: " << neeble_to_hex(resp._version & 0xf0) << 
                                        neeble_to_hex(resp._version & 0xf) 
                                        << std::endl;

            output << "payload_size: " << resp._payload_size << std::endl;
            print_payload(output, resp._payload);
            output << "##################################" << std::endl;
            
            return output;
        }
};

#endif //_RESPONSE_H