#include <vector>
#include <string>
#include <iostream>
#include "common.hpp"


#ifndef _MESSAGE_H
#define _MESSAGE_H

typedef enum message_types_e
{
    MESSAGE_TYPE_REQUEST_SYM_KEY = 1,
    MESSAGE_TYPE_SEND_SYM_KEY,
    MESSAGE_TYPE_SEND_TXT,
    MESSAGE_TYPE_SEND_FILE,
} messag_type_t;

#define SUPPORTED_MESSAGE_TYPES {MESSAGE_TYPE_REQUEST_SYM_KEY, \
                                MESSAGE_TYPE_SEND_SYM_KEY, \
                                MESSAGE_TYPE_SEND_TXT, \
                                MESSAGE_TYPE_SEND_FILE}


#define SUPPORTED_MESSAGE_TYPES_NUM (4)


class Message
{
    private:
        std::vector<uint8_t> _client_id;
        uint8_t _message_type;
        size_t _content_size;
        std::vector<uint8_t> _message_content;
        std::vector<uint8_t> _raw_message;
        bool _is_msg_valid;

    public:
        Message();
        Message(std::vector<uint8_t> id, uint8_t type, 
                std::vector<uint8_t> content);
        Message(std::vector<uint8_t> id, uint8_t type);

        void set_content(std::vector<uint8_t> content);

        std::vector<uint8_t> build_msg();
              //error_code_t parse_msg(std::vector<uint8_t> data);
        bool is_msg_type_valid(uint8_t msg_type);
        std::vector<uint8_t> get_raw_message()
        {
            return _raw_message;
        }
        bool is_message_valid();
        
        friend std::ostream & operator<<(std::ostream &output, const Message & msg)
        {
            size_t i = 0;

            output << "###########Message Object######### ";
            output << "ID: ";
            for(auto byte: msg._client_id)
            {
                output << "0x";
                output << neeble_to_hex(byte >> 4);
                output << neeble_to_hex(byte & 0xf);
                output << " ";
            }
            output << "\n";

            output << "Type:\n";
            output << msg._message_type << std::endl;

            output << "Content size: \n";
            output << msg._content_size << std::endl;
            output << "Content: \n";
            for(auto byte : msg._message_content)
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
};

#endif // _MESSAGE_H