#include <exception>
#include "operation.hpp"



RegisterOperation::RegisterOperation(Client * client, SOCKET sock)
{
    if(NULL == client)
    {
        throw std::invalid_argument("Register Operation: Invalid Request");
    }
    _client = client;
    _interface = sock;

    _public_key = _client->getPubKey();
    //TODO: not sure if needed
    for(size_t i = _public_key.length() + 1; i <= PUBLIC_KEY_SIZE; i++)
    {
        _public_key.append(FILLING_BYTE);
    }

    std::cout << "Init RegisterOperation" <<std::endl;
}


std::vector<uint8_t> RegisterOperation::_create_payload()
{
    std::vector<uint8_t> payload;
    size_t padding_size = 0;

    // limit to unsigned char
    padding_size = (NAME_SIZE - _name.size()) && 0xff;
     /* first go the Name */
    for(auto chr : _name)
    {
        payload.push_back(chr);
    }

    payload.insert(payload.end(), PADDING_BYTE, padding_size);

    /* next go the public key */
    for(auto byte : _public_key)
    {
        payload.push_back(byte);
    }

    return payload;
}


std::vector<uint8_t> RegisterOperation::do_operation()
{
    int result = -1;
    char received_data[RegisterOperation::RESPONSE_SIZE];
    std::vector<uint8_t> data_to_send;
    std::vector<uint8_t> resp_data;

    // if client is registered we don't allow registration
    if(_client->is_client_registered())
    {
        std::cout << "Client already registerd!" << std::endl;
        return {};
    }

    _name = _client->terminal.get_client_name();
    _client->setName(_name);
    for(size_t i = _name.length() + 1; i <= NAME_SIZE; i++)
    {
        _name.append(FILLING_BYTE);
    }
    
    /* create the Request for the operation (the client id is dummy)*/
    _request = Request(_client->getID(), CODE_REGISTER, _create_payload());

    std::cout << _request << std::endl;
    /*  build the request and send it*/
    data_to_send = _request.build_request();
    
    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()) ,
                 data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        throw std::runtime_error("Register Operation: Failed to send data");
    }

    std::cout <<"Receiving" <<std::endl;
    result = recv(_interface, received_data, RegisterOperation::RESPONSE_SIZE , 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        throw std::runtime_error("Register Operation: Failed to recv data");
    }

    resp_data = std::vector<uint8_t>(received_data, received_data + (size_t)result);

    Response resp(resp_data);
    std::cout << resp << std::endl;
    if((CODE_ERROR == resp.get_response_code()) || !resp.is_response_valid())
    {
        std::cout << "Server responded with an error" << std::endl;
        return {};
    }

    // set the newly acquired id for our client and write it to the ME file
    _client->setID(resp.get_payload());
    _client->save_client_creds();

    std::cout << "Client Successfully Registered" << std::endl;
    return {};
}


ListUserOperation::ListUserOperation(Client * client, SOCKET sock)
{
    _interface = sock;
    _request = Request(client->getID(), CODE_LIST_CLIENTS);
    _client = client;
}


std::vector<uint8_t> ListUserOperation::do_operation()
{
    int result = -1;
    char received_data[MIN_RESPONSE_SIZE];
    std::vector<uint8_t> data_to_send;
    std::vector<uint8_t> resp_data;
    size_t received_data_size = 0;

    std::cout << _request << std::endl;
    /*  build the request and send it*/
    data_to_send = _request.build_request();
    
    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()) ,
                 data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        throw std::runtime_error("ListUser Operation: Failed to send data");
    }

    /* Receive the header so we will know how much more to receive */
    result = recv(_interface, received_data, MIN_RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        throw std::runtime_error("ListUser Operation: Failed to recv data");
    }
    resp_data = std::vector<uint8_t>(received_data, received_data + (size_t)result);

    // partially build the response to findout the payload size
    Response resp(resp_data);
    
    if(CODE_ERROR ==  resp.get_response_code())
    {
        std::cout << "Server responded with an error" << std::endl;
        return {};
    }

    resp_data.clear();

    if(MAX_PAYLOAD_SIZE < resp.get_payload_size())
    {
        std::cout << "Payload too big, skiping" << std::endl;
        return {};
    }

    while(received_data_size < resp.get_payload_size())
    {
        result = recv(_interface, received_data, MIN_RESPONSE_SIZE, 0);
        if(SOCKET_ERROR == result || 0 == result)
        {
            throw std::runtime_error("ListUser Operation: Failed to recv data");
        }

        resp_data.insert(resp_data.end(), received_data, received_data + (size_t)result);
        received_data_size += result;
    }
    std::cout << "received resp_data size: " << resp_data.size() << std::endl;

    // add the newly received contacts to the contacts list
    _add_contacts(resp_data);
    std::cout << "Received Contacts: " << std::endl;
    for(auto contact : _client->contacts)
    {
        std::cout << contact.getName() << std::endl;
    }

    // nothing to return
    return {};
}


void ListUserOperation::_add_contacts(std::vector<uint8_t> payload)
{
    std::string temp_name;
    std::string stripped_string;
    std::vector<uint8_t> contact_id;
    size_t pair_size = (NAME_SIZE + FIELD_CLIENT_ID_SIZE);
    size_t contacts_num = payload.size() / pair_size;
    size_t string_delimiter = 0;


    for(size_t i = 0; i < contacts_num; i++)
    {
        temp_name = std::string(payload.begin() + (pair_size * i) + FIELD_CLIENT_ID_SIZE,
                                payload.begin() + (pair_size * i) + pair_size);
        contact_id = std::vector<uint8_t>(payload.begin() + (pair_size * i),
                                    payload.begin() + (pair_size * i) + FIELD_CLIENT_ID_SIZE);

        // strip the string from null bytes
        string_delimiter = temp_name.find_first_of('\x00');
        
        std::cout << "string delimiter: " << string_delimiter << std::endl;
        // if the delimiter passed the NAME_SIZE meaning the name is of size NAME_SIZE
        if(string_delimiter > NAME_SIZE)
            string_delimiter = NAME_SIZE;

        stripped_string = std::string(temp_name.begin(), temp_name.begin() + string_delimiter);

        // add only new contacts!
        if(!_client->is_known_contact(contact_id))
            _client->contacts.push_back(Contact(stripped_string, contact_id));
    }
}


GetPublicKeyOperation::GetPublicKeyOperation(Client * client, SOCKET sock)
{
    std::string dest_name;

    _client =  client;
    _interface = sock;

    //TODO: fetch the id of the requested user from terminal
    dest_name = _client->terminal.get_destination_name();
    std::cout << dest_name << std::endl;
    _wanted_client_id = _client->get_contact_id_by_name(dest_name);
}


std::vector<uint8_t> GetPublicKeyOperation::do_operation()
{
    std::vector<uint8_t> data_to_send;
    char * received_data = new char[GetPublicKeyOperation::RESPONSE_SIZE];
    std::vector<uint8_t> resp_data;
    size_t result = -1;

    // if the size is 0 it means we couldn't find the contact in the constructor
    if(FIELD_CLIENT_ID_SIZE != _wanted_client_id.size())
    {
        std::cout << "Unknown contact - you can send only to your contacts" <<std::endl;
        return {};
    }

    _request = Request(_client->getID(), CODE_GET_PUBLIC_KEY, _wanted_client_id);
    std::cout << _request << std::endl;
    data_to_send = _request.build_request();

    std::cout << "data to send size: " << data_to_send.size() << std::endl; 

    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()), data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        throw std::runtime_error("GetPublicKey Operation: Failed to send data");
    }
    
    result = recv(_interface, received_data, GetPublicKeyOperation::RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        throw std::runtime_error("GetPublicKey Operation: Failed to recv data");
    }
    
    resp_data = std::vector<uint8_t>(received_data, received_data + result);
    Response resp = Response(resp_data);
    if((CODE_ERROR ==  resp.get_response_code()) || !resp.is_response_valid())
    {
        std::cout << "Server responded with an error" << std::endl;
        return {};
    }

    if(GetPublicKeyOperation::RESPONSE_SIZE != result)
    {
        throw std::runtime_error("Received invalid response from server!");
    }

    _update_contact(std::vector<uint8_t>(received_data,
                                         received_data + GetPublicKeyOperation::PAYLOAD_SIZE));

    delete received_data;
    return {};
}


void GetPublicKeyOperation::_update_contact(std::vector<uint8_t> raw_data)
{
    std::vector<uint8_t> contact_id = std::vector<uint8_t>(raw_data.begin(),
                                                        raw_data.begin() + FIELD_CLIENT_ID_SIZE);

    std::string contact_pub_key = std::string(raw_data.begin() + FIELD_CLIENT_ID_SIZE,
                                            raw_data.end());
    // we do not check the contact id validity
    for(auto contact : _client->contacts)
    {
        if(contact.is_id_equal(contact_id))
        {
            contact.setPubKey(contact_pub_key);
        }
    }

    std::cout << "Received Public key: \n";
    std::cout << contact_pub_key << std::endl;
}



GetSymmKeyOperation::GetSymmKeyOperation(Client * client, SOCKET sock)
{
    std::string dest_name;
    _client = client;
    _interface = sock;

    //TODO: fetch the id of the requested user from terminal
    dest_name = _client->terminal.get_destination_name();
    _wanted_client_id = _client->get_contact_id_by_name(dest_name);

}


std::vector<uint8_t> GetSymmKeyOperation::do_operation()
{
    std::vector<uint8_t> data_to_send;
    char * received_data = new char[GetSymmKeyOperation::RESPONSE_SIZE];
    std::vector<uint8_t> resp_data;
    size_t result = -1;

    Message msg(_wanted_client_id, MESSAGE_TYPE_REQUEST_SYM_KEY);
    std::vector<uint8_t> payload = msg.build_msg();

    std::cout << msg << std::endl;
    _request = Request(_client->getID(), CODE_SEND_MESSAGE, payload);

    if(FIELD_CLIENT_ID_SIZE !=  _wanted_client_id.size())
    {
        std::cout << "Unknown contact - you can send only to your contacts" << std::endl;
        return {};
    }
    std::cout << _request << std::endl;
    data_to_send = _request.build_request();

    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()), data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        throw std::runtime_error("GetSymmKey Operation: Failed to send data");
    }
    
    result = recv(_interface, received_data, GetSymmKeyOperation::RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        throw std::runtime_error("GetSymmKey Operation: Failed to recv data");
    }

    resp_data = std::vector<uint8_t>(received_data, received_data + result);
    
    Response resp(resp_data);
    if(CODE_ERROR ==  resp.get_response_code())
    {
        std::cout << "Server responded with an error" << std::endl;
        return {};
    }

    if(GetSymmKeyOperation::RESPONSE_SIZE != result)
    {
        throw std::runtime_error("Received invalid response from server!");
    }
    std::cout << "Message sent successfully " << std::endl;

    delete received_data;
    return {};
}


SendSymKeyOperation::SendSymKeyOperation(Client * client, SOCKET sock)
{
    std::string dest_name;
    _client = client;
    _interface = sock;

    dest_name = _client->terminal.get_destination_name();
    _wanted_client_id = _client->get_contact_id_by_name(dest_name);
}


std::vector<uint8_t> SendSymKeyOperation::do_operation()
{
    std::vector<uint8_t> data_to_send;
    char * received_data = new char[SendSymKeyOperation::RESPONSE_SIZE];
    std::vector<uint8_t> resp_data;
    size_t result = -1;

    if(FIELD_CLIENT_ID_SIZE != _wanted_client_id.size())
    {
        std::cout << "Contact is unknown! You can send only to your contacts";
        std::cout << std::endl;
        return {};
    }

    //TODO: we create symmetric key and add it to the msg
    std::vector<uint8_t> sym_key(SYMMETRIC_KEY_SIZE, '\x44');
    Message msg(_wanted_client_id, MESSAGE_TYPE_SEND_SYM_KEY, sym_key);
    std::vector<uint8_t> payload = msg.build_msg();

    std::cout << msg << std::endl;
    _request = Request(_client->getID(), CODE_SEND_MESSAGE, payload);
    
    std::cout << _request << std::endl;
    data_to_send = _request.build_request();

    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()), data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        throw std::runtime_error("SendSymKey Operation: Failed to send data");
    }
    
    result = recv(_interface, received_data, SendSymKeyOperation::RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        throw std::runtime_error("SendSymKey Operation: Failed to recv data");
    }

    resp_data = std::vector<uint8_t>(received_data, received_data + result);
    
    Response resp(resp_data);
    if(CODE_ERROR ==  resp.get_response_code())
    {
        std::cout << "Server responded with an error" << std::endl;
        return {};
    }

    if(SendSymKeyOperation::RESPONSE_SIZE != result)
    {
        throw std::runtime_error("Received invalid response from server!");
    }
    std::cout << "Message sent successfully " << std::endl;

    delete received_data;
    return {};
}


SendTxtOperation::SendTxtOperation(Client * client, SOCKET sock)
{
    std::string dest_name;
    _client = client;
    _interface = sock;

    dest_name = _client->terminal.get_destination_name();
    _wanted_client_id = _client->get_contact_id_by_name(dest_name);
}


std::vector<uint8_t> SendTxtOperation::do_operation()
{
    std::vector<uint8_t> data_to_send;
    char * received_data = new char[SendTxtOperation::RESPONSE_SIZE];
    std::vector<uint8_t> resp_data;
    std::string temp_msg;
    size_t result = -1;

    if(FIELD_CLIENT_ID_SIZE != _wanted_client_id.size())
    {
        std::cout << "Unknown contact - ou can send only to your contacts" <<std::endl;
        return {};
    }
 
    temp_msg = _client->terminal.get_txt_msg();
    std::cout << temp_msg << std::endl;

    std::vector<uint8_t> msg_payload(temp_msg.begin(), temp_msg.end());
    Message msg(_wanted_client_id, MESSAGE_TYPE_SEND_TXT, msg_payload);

    std::vector<uint8_t> payload = msg.build_msg();

    std::cout << msg << std::endl;
    _request = Request(_client->getID(), CODE_SEND_MESSAGE,payload);

    std::cout << _request << std::endl;
    data_to_send = _request.build_request();

    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()), data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        throw std::runtime_error("SendTxt Operation: Failed to send data");
    }
    
    result = recv(_interface, received_data, SendTxtOperation::RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        throw std::runtime_error("SendTxt Operation: Failed to recv data");
    }

    resp_data = std::vector<uint8_t>(received_data, received_data + result);
    
    Response resp(resp_data);
    if((CODE_ERROR ==  resp.get_response_code()) || !resp.is_response_valid())
    {
        std::cout << "Server responded with an error" << std::endl;
        return {};
    }

    if(SendTxtOperation::RESPONSE_SIZE != result)
    {
        throw std::runtime_error("Received invalid response from server!");
    }
    std::cout << "Message sent successfully " << std::endl;

    delete received_data;
    return {};
}



FetchMsgOperation::FetchMsgOperation(Client * client, SOCKET sock)
{
    _interface = sock;
    _client = client;
    _request = Request(client->getID(), CODE_GET_INCOMING_MESSAGES);
}


std::vector<uint8_t> FetchMsgOperation::do_operation()
{
    int result = -1;
    char received_data[MIN_RESPONSE_SIZE];
    std::vector<uint8_t> data_to_send;
    std::vector<uint8_t> resp_data;

    std::cout << _request << std::endl;
    /*  build the request and send it*/
    data_to_send = _request.build_request();
    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()),
                 data_to_send.size(), 0);

    if(SOCKET_ERROR == result)
    {
        throw std::runtime_error("FetchMsgOperation: Failed to send data");
    }

    /* Receive the header so we will know how much more to receive */
    result = recv(_interface, received_data, MIN_RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        throw std::runtime_error("FetchMsgOperation: Failed to recv data");
    }
    resp_data = std::vector<uint8_t>(received_data, received_data + (size_t)result);

    // partially build the response to findout the payload size
    Response resp(resp_data);
    if((CODE_ERROR ==  resp.get_response_code()))
    {
        std::cout << "Server responded with an error" << std::endl;
        return {};
    }

    if(MAX_PAYLOAD_SIZE < resp.get_payload_size())
    {
        std::cout << "Payload too big, skiping" << std::endl;
        return {};
    }

    _fetch_massages(resp.get_payload_size());

    return {};
}


void FetchMsgOperation::_fetch_massages(size_t msgs_data_size)
{
    size_t received_data_size = 0;
    char received_data[MIN_MESSAGE_SIZE];
    char * content_data;
    int result = -1;
    std::vector<uint8_t> client_id;
    uint8_t msg_type = 0;
    size_t content_size = 0;

    std::cout << "msgs_data_size " << msgs_data_size <<  std::endl;

    while(received_data_size < msgs_data_size)
    {
        result = recv(_interface, received_data, FetchMsgOperation::MIN_MSG_RESPONSE_SIZE, 0);
        if(SOCKET_ERROR == result || 0 == result)
        {
            throw std::runtime_error("Failed to recv data");
        }
        received_data_size += result;

        client_id = std::vector<uint8_t>(received_data, received_data + FIELD_CLIENT_ID_SIZE);

        // the msg type is after the client id and msg_id (msg id is irrelevant)
        msg_type = received_data[FIELD_CLIENT_ID_SIZE + MSG_ID_SIZE];
        
        content_size = *((size_t *)(received_data + FIELD_CLIENT_ID_SIZE + MSG_ID_SIZE + 1));
        content_size = _byteswap_ulong(content_size);
        std::cout << "msg content size: " << content_size << std::endl;
        /* validate content size */
        if(MAX_PAYLOAD_SIZE < content_size)
            throw std::runtime_error("MSG content to big!");
        
        // receive the content if content size is bigger than 0
        if(0 < content_size)
        {
            content_data = new char[content_size];
            result = recv(_interface, content_data, content_size, 0);
            if(SOCKET_ERROR == result || 0 == result)
            {
                throw std::runtime_error("Failed to recv data");
            }
            std::cout << "result " << result << std::endl;
            Message temp_msg(client_id, msg_type,
                            std::vector<uint8_t>(content_data, content_data + (size_t)result));

            _handle_msg(temp_msg);
            delete content_data;
        }
        else
        {
            Message temp_msg(client_id, msg_type);
            _handle_msg(temp_msg);
        }
        
        received_data_size += result;
        std::cout << "received_data_size " <<  received_data_size << std::endl;
    }
}


void FetchMsgOperation::_handle_msg(Message msg)
{
    std::vector<uint8_t> * content;
    std::vector<uint8_t> src_id = msg.get_id();
    std::string contact_name;

    switch(msg.get_message_type())
    {
        case MESSAGE_TYPE_REQUEST_SYM_KEY:
            /* print the msg */
            contact_name = _client->get_contact_name_by_id(src_id).c_str();
            if(0 == contact_name.length())
            {
                contact_name = "Unknown Contact";
            }
            printf(MESSAGE_FORMAT, contact_name.c_str(), SYMM_KEY_REQUEST_MSG);
            break;

        case MESSAGE_TYPE_SEND_SYM_KEY:
            /* print the msg and save the symmkey */
            contact_name = _client->get_contact_name_by_id(src_id).c_str();
            if(0 == contact_name.length())
            {
                contact_name = "Unknown Contact";
            }
            printf(MESSAGE_FORMAT, contact_name.c_str(), SYMM_KEY_RCVD_MSG);
            _client->update_contact_symm_key_by_id(src_id, *msg.get_message_content());
            break;

        case MESSAGE_TYPE_SEND_TXT:
            contact_name = _client->get_contact_name_by_id(src_id).c_str();
            if(0 == contact_name.length())
            {
                contact_name = "Unknown Contact";
            }
            content = msg.get_message_content();
            content->push_back(0);
            printf(MESSAGE_FORMAT,  contact_name.c_str(), content->data());
            break;
        default:
            std::cout << "Uknown message received" << std::endl;
            
         /*TODO: decrypt this content with client's symm key */
    }
}