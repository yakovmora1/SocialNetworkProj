#include "message.hpp"
#include <exception>


Message::Message()
{
    _is_msg_valid = false;
    _message_type = MESSAGE_TYPE_SEND_TXT;
}

Message::Message(std::vector<uint8_t> id, uint8_t type, 
        std::vector<uint8_t> content)
{
    _client_id = id;
    if(!is_msg_type_valid(type))
        throw std::invalid_argument("Message(): invalid message type");

    _message_type = type;

    set_content(content);
    _is_msg_valid = true;
}

Message::Message(std::vector<uint8_t> id, uint8_t type)
{
    _client_id = id;
    _message_type = type;
    
    if(!is_msg_type_valid(type))
        throw std::invalid_argument("Message(): invalid message type");

    // empty message size
    _content_size = 0;
    _is_msg_valid = true;
}

void Message::set_content(std::vector<uint8_t> content)
{
    if(_message_type == MESSAGE_TYPE_REQUEST_SYM_KEY)
    {
        throw std::logic_error("Message: can't send content for pub key msg");
    }

    _message_content = content;
    _content_size = _message_content.size();
}


bool Message::is_msg_type_valid(uint8_t msg_type)
{
    uint8_t msg_types[SUPPORTED_MESSAGE_TYPES_NUM] = SUPPORTED_MESSAGE_TYPES;
    for(size_t i = 0; i < sizeof(msg_types); i++)
    {
        if(msg_type == msg_types[i])
            return true;
    }
    return false;
}


bool Message::is_message_valid()
{
    return _is_msg_valid;
}


std::vector<uint8_t> Message::build_msg()
{
    _raw_message.insert(_raw_message.end(), _client_id.begin(), _client_id.end());
    _raw_message.push_back(_message_type);

    _raw_message.push_back((uint8_t)(_content_size >> 24));
    _raw_message.push_back((uint8_t)(_content_size >> 16));
    _raw_message.push_back((uint8_t)(_content_size >> 8));
    _raw_message.push_back((uint8_t)(_content_size));

    _raw_message.insert(_raw_message.end(), _message_content.begin(), _message_content.end());
        
    return _raw_message;
}