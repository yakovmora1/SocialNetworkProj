#include <string>
#include <vector>
#include <iostream>
#include "common.hpp"

#ifndef _REQUEST_H
#define _REQUEST_H

/*TODO Copy constructor ! */



/* SUPPORTED CODES */
typedef enum request_code_e
{
    CODE_REGISTER = 1000,
    CODE_LIST_CLIENTS, 
    CODE_GET_PUBLIC_KEY,
    CODE_SEND_MESSAGE,
    CODE_GET_INCOMING_MESSAGES
} request_code_t;

#define SUPPORTED_CODES {CODE_REGISTER, CODE_LIST_CLIENTS, \
                        CODE_GET_PUBLIC_KEY, CODE_SEND_MESSAGE,\
                        CODE_GET_INCOMING_MESSAGES}

#define SUPPORTED_CODES_NUM  (5)
#define FIELD_VERSION_SIZE (1)
#define FIELD_CLIENT_ID_SIZE (16)
#define FILED_REQUEST_CODE_SIZE (2)

class Request
{
    private:
        std::vector<uint8_t> _client_id;
        uint16_t _request_code;
        uint8_t  _version;
        size_t _payload_size;
        std::vector<uint8_t> _payload;
        std::vector<uint8_t>  _raw_request;

        bool _validate_code(uint16_t code);

    public:
        Request()
        {
            _request_code = 0;
            _version = SUPPORTED_VERSION;
            _payload_size = 0;
        }

        Request(std::vector<uint8_t> client_id, uint16_t code);
        
        Request(std::vector<uint8_t> client_id, uint16_t code,
                const std::vector<uint8_t> payload);

        // copy ctor not really needed because default copy ctors called
        //Request(const Request & req);

        void set_payload(const std::vector<uint8_t> payload);

        void append_to_payload(std::string str)
        {
            for(auto chr : str)
            {
                _payload.push_back(chr);
            }
            /* update payload size accordingly */
            _payload_size += str.length();
        }

        void append_to_payload(std::vector<uint8_t> vec)
        {
            for(auto byte : vec)
            {
                _payload.push_back(byte);
            }

            /* update payload size accordingly */
            _payload_size += vec.size();
        }

        std::vector<uint8_t> build_request();

        const std::vector<uint8_t> get_raw_request();

        const std::vector<uint8_t> get_client_id()
        {
            return _client_id;
        }

        const uint8_t get_version()
        {
            return _version;
        }

        const size_t get_payload_size()
        {
            return _payload_size;
        }

       
        friend std::ostream & operator<<(std::ostream &output, const Request & req)
        {
            size_t i = 0;
            output << "###########Request Object######### " << std::endl;
            output << "client_id: " << std::endl;
            for(auto byte: req._client_id)
            {
                output << "0x";
                output << neeble_to_hex(byte & 0xf0);
                output << neeble_to_hex(byte & 0xf);
                output << " ";
                
            }
            output << std::endl;

            output << "version: " << neeble_to_hex(req._version & 0xf0) << 
                                    neeble_to_hex(req._version & 0xf) <<  std::endl;
            
            output << "request code: " << req._request_code << std::endl;
            
            output << "payload_size: " << req._payload_size << std::endl;
            
            output << "payload: \n" << std::endl;
            for(auto byte : req._payload)
            {
                output << byte << " ";
                
                i++;
                
                if(10 == i)
                {
                    i = 0;
                    output << "\n";
                }
            }
            output << "\n##################################" << std::endl;

            return output;
        }
        
        //~Request();
};




#endif //_REQUEST_H