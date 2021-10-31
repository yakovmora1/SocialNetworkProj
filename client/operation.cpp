#include <exception>
#include "operation.hpp"



RegisterOperation::RegisterOperation(Client * client, SOCKET sock)
{
    if(NULL == client)
    {
        throw std::invalid_argument("Register Operation: Invalid Request");
    }

    _interface = sock;
    _name = client->getName();
    _public_key = client->getPubKey();

    /* create the Request for the operation */
    _request = Request(client->getID(), CODE_REGISTER, _create_payload());
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


bool RegisterOperation::_is_request_valid(Request * request)
{
    /* validate client id   shuold be empty*/
    if(FIELD_CLIENT_ID_SIZE != request->get_client_id().size())
    {
        return false;
    }

    if(NAME_SIZE + PUBLIC_KEY_SIZE != request->get_payload_size())
    {
        return false;
    }

    if(SUPPORTED_VERSION != request->get_version())
    {
        return false;
    }

    /* no need to check for request code */
    return true;
}


std::vector<uint8_t> RegisterOperation::do_operation()
{
    int result = -1;
    char received_data[RegisterOperation::RESPONSE_SIZE];
    std::vector<uint8_t> data_to_send;
    std::vector<uint8_t> resp_data;

    std::cout << _request << std::endl;
    /*  build the request and send it*/
    data_to_send = _request.build_request();
    std::cout << "Register Operation sending data" << std::endl;
    
    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()) ,
                 data_to_send.size(), 0);

    if(SOCKET_ERROR == result)
    {
        std::cout << "Register Operation: Failed to send data" << std::endl;
        throw std::runtime_error("Failed to send data");
    }

    std::cout <<"Receiving" <<std::endl;
    result = recv(_interface, received_data, RegisterOperation::RESPONSE_SIZE , 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        std::cout << " Register Operation: Failed to recv data" << std::endl;
        throw std::runtime_error("Failed to recv data");
    }

    resp_data = std::vector<uint8_t>(received_data, received_data + (size_t)result);

    Response resp(resp_data);
    std::cout << resp << std::endl; 

    _client->setID(resp.get_payload());

    return resp.get_payload();
}




ListUserOperation::ListUserOperation(Client * client, SOCKET sock)
{
    _interface = sock;
    _request = Request(client->getID(), CODE_LIST_CLIENTS);
    _client = client;
    std::cout << "ListUserOperation Init" << std::endl;
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
    std::cout << "ListUser Operation sending data" << std::endl;
    
    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()) ,
                 data_to_send.size(), 0);

    if(SOCKET_ERROR == result)
    {
        std::cout << "ListUser Operation: Failed to send data" << std::endl;
        throw std::runtime_error("Failed to send data");
    }

    /* Receive the header so we will know how much more to receive */
    result = recv(_interface, received_data, MIN_RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        std::cout << "ListUser Operation: Failed to recv data" << std::endl;
        throw std::runtime_error("Failed to recv data");
    }
    resp_data = std::vector<uint8_t>(received_data, received_data + (size_t)result);

    // partially build the response to findout the payload size
    Response resp(resp_data);
    
    if(CODE_ERROR ==  resp.get_response_code())
    {
        std::cout << "General Error occured in server - Failed" << std::endl;
        return {};
    }

    resp_data.clear();

    if(MIN_PAYLOAD_SIZE < resp.get_payload_size())
    {
        std::cout << "Payload too big, skiping" << std::endl;
        return {};
    }

    while(received_data_size < resp.get_payload_size())
    {
        result = recv(_interface, received_data, MIN_RESPONSE_SIZE, 0);
        if(SOCKET_ERROR == result || 0 == result)
        {
            std::cout << "ListUser Operation: Failed to recv data" << std::endl;
            throw std::runtime_error("Failed to recv data");
        }

        resp_data.insert(resp_data.end(), received_data, received_data + (size_t)result);
        received_data_size += result;
    }
    std::cout << "received resp_data size: " << resp_data.size() << std::endl;

    // add the newly received contacts to the contacts list
    _add_contacts(resp_data);
    //TODO: print the contacts' names

    // nothing to return
    return {};

}


void ListUserOperation::_add_contacts(std::vector<uint8_t> payload)
{
    std::string temp_name;
    std::string stripped_string;
    std::vector<uint8_t> temp_id;
    size_t pair_size = (NAME_SIZE + FIELD_CLIENT_ID_SIZE);
    size_t contacts_num = payload.size() / pair_size;
    size_t string_delimiter = 0;


    for(size_t i = 0; i < contacts_num; i++)
    {
        temp_name = std::string(payload.begin() + (pair_size * i) + FIELD_CLIENT_ID_SIZE,
                                payload.begin() + (pair_size * i) + pair_size);
        temp_id = std::vector<uint8_t>(payload.begin() + (pair_size * i),
                                    payload.begin() + (pair_size * i) + FIELD_CLIENT_ID_SIZE);

        // strip the string from null bytes
        string_delimiter = temp_name.find_first_of('\x00');
        
        std::cout << "string delimiter: " << string_delimiter << std::endl;
        if(string_delimiter > NAME_SIZE)
            string_delimiter = NAME_SIZE;

        stripped_string = std::string(temp_name.begin(), temp_name.begin() + string_delimiter);

        _client->contacts.push_back(Contact(stripped_string, temp_id));
    }
}


GetPublicKeyOperation::GetPublicKeyOperation(Client * client, SOCKET sock)
{
    _client =  client;
    _interface = sock;

    //TODO: fetch the id of the requested user from terminal
    std::string cont_id = "\xe8\xf6\x75\xd7\xa3\xd1\xef\x41\x85\xf0\x3b\x67\x5e\x03\xe0\x51";
    _wanted_client_id = std::vector<uint8_t>(cont_id.begin(), cont_id.end());
}


std::vector<uint8_t> GetPublicKeyOperation::do_operation()
{
    std::vector<uint8_t> data_to_send;
    char * received_data = new char[GetPublicKeyOperation::RESPONSE_SIZE];
    std::vector<uint8_t> resp_data;
    size_t result = -1;

/*TODO
    if(!_client->is_known_contact(_wanted_client_id))
    {
        std::cout << "Contact is unknown! You can send only to your contacts";
        std::cout << std::endl;
        return {};
    }
*/
    _request = Request(_client->getID(), CODE_GET_PUBLIC_KEY, _wanted_client_id);
    std::cout << _request << std::endl;
    data_to_send = _request.build_request();

    std::cout << "datyatosend size: " << data_to_send.size() << std::endl; 

    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()), data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        std::cout << "GetPublicKey Operation: Failed to send data" << std::endl;
        throw std::runtime_error("Failed to send data");
    }
    
    result = recv(_interface, received_data, GetPublicKeyOperation::RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        std::cout << "GetPublicKey Operation: Failed to recv data" << std::endl;
        throw std::runtime_error("Failed to recv data");
    }

    
    resp_data = std::vector<uint8_t>(received_data, received_data + result);
    Response resp = Response(resp_data);
    if(CODE_ERROR ==  resp.get_response_code())
    {
        std::cout << "General Error occured in server - Failed" << std::endl;
        return {};
    }

    if(GetPublicKeyOperation::RESPONSE_SIZE != result)
    {
        throw std::runtime_error("Received invalid response from server!");
    }

    _update_contact(std::vector<uint8_t>(received_data,
                                         received_data + GetPublicKeyOperation::RESPONSE_SIZE));

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
    _client = client;
    _interface = sock;

    //TODO: fetch the id of the requested user from terminal
    std::string cont_id = "\xe8\xf6\x75\xd7\xa3\xd1\xef\x41\x85\xf0\x3b\x67\x5e\x03\xe0\x51";
    _wanted_client_id = std::vector<uint8_t>(cont_id.begin(), cont_id.end());

}


std::vector<uint8_t> GetSymmKeyOperation::do_operation()
{
    std::vector<uint8_t> data_to_send;
    char * received_data = new char[GetPublicKeyOperation::RESPONSE_SIZE];
    std::vector<uint8_t> resp_data;
    size_t result = -1;

    Message msg(_wanted_client_id, MESSAGE_TYPE_SEND_SYM_KEY);
    std::vector<uint8_t> payload = msg.build_msg();

    std::cout << msg << std::endl;
    _request = Request(_client->getID(), CODE_SEND_MESSAGE,payload);

/*TODO
    if(!_client->is_known_contact(_wanted_client_id))
    {
        std::cout << "Contact is unknown! You can send only to your contacts";
        std::cout << std::endl;
        return {};
    }
*/
    std::cout << _request << std::endl;
    data_to_send = _request.build_request();

    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()), data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        std::cout << "GetSymmKey Operation: Failed to send data" << std::endl;
        throw std::runtime_error("Failed to send data");
    }
    
    result = recv(_interface, received_data, GetSymmKeyOperation::RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        std::cout << "GetSymmKey Operation: Failed to recv data" << std::endl;
        throw std::runtime_error("Failed to recv data");
    }

    resp_data = std::vector<uint8_t>(received_data, received_data + result);
    
    Response resp(resp_data);
    if(CODE_ERROR ==  resp.get_response_code())
    {
        std::cout << "General Error occured in server - Failed" << std::endl;
        return {};
    }

    if(GetSymmKeyOperation::RESPONSE_SIZE != result)
    {
        throw std::runtime_error("Received invalid response from server!");
    }
    std::cout << "Message sent successfully " << std::endl;
    //_update_contact(resp.get_payload());

    delete received_data;
    return {};
}


SendSymKeyOperation::SendSymKeyOperation(Client * client, SOCKET sock)
{
    _client = client;
    _interface = sock;

    //TODO: fetch the id of the requested user from terminal
    std::string cont_id = "\xe8\xf6\x75\xd7\xa3\xd1\xef\x41\x85\xf0\x3b\x67\x5e\x03\xe0\x51";
    _wanted_client_id = std::vector<uint8_t>(cont_id.begin(), cont_id.end());

}


std::vector<uint8_t> SendSymKeyOperation::do_operation()
{
    std::vector<uint8_t> data_to_send;
    char * received_data = new char[GetPublicKeyOperation::RESPONSE_SIZE];
    std::vector<uint8_t> resp_data;
    size_t result = -1;

    //TODO: we create symmetric key and add it to the msg
    std::vector<uint8_t> sym_key(SYMMETRIC_KEY_SIZE, '\x44');
    Message msg(_wanted_client_id, MESSAGE_TYPE_SEND_SYM_KEY, sym_key);
    std::vector<uint8_t> payload = msg.build_msg();

    std::cout << msg << std::endl;
    _request = Request(_client->getID(), CODE_SEND_MESSAGE,payload);

/*TODO
    if(!_client->is_known_contact(_wanted_client_id))
    {
        std::cout << "Contact is unknown! You can send only to your contacts";
        std::cout << std::endl;
        return {};
    }
*/
    std::cout << _request << std::endl;
    data_to_send = _request.build_request();

    result = send(_interface, reinterpret_cast<char *>(data_to_send.data()), data_to_send.size(), 0);
    if(SOCKET_ERROR == result)
    {
        std::cout << "SendSymKey Operation: Failed to send data" << std::endl;
        throw std::runtime_error("Failed to send data");
    }
    
    result = recv(_interface, received_data, SendSymKeyOperation::RESPONSE_SIZE, 0);
    if(SOCKET_ERROR == result || 0 == result)
    {
        std::cout << "SendSymKey Operation: Failed to recv data" << std::endl;
        throw std::runtime_error("Failed to recv data");
    }

    resp_data = std::vector<uint8_t>(received_data, received_data + result);
    
    Response resp(resp_data);
    if(CODE_ERROR ==  resp.get_response_code())
    {
        std::cout << "General Error occured in server - Failed" << std::endl;
        return {};
    }

    if(SendSymKeyOperation::RESPONSE_SIZE != result)
    {
        throw std::runtime_error("Received invalid response from server!");
    }
    std::cout << "Message sent successfully " << std::endl;
    //_update_contact(resp.get_payload());

    delete received_data;
    return {};
}


/*
void GetSymmKeyOperation::_update_contact(std::vector<uint8_t> raw_data)
{
    std::vector<uint8_t> contact_id = std::vector<uint8_t>(raw_data.begin(),
                                                        raw_data.begin() + FIELD_CLIENT_ID_SIZE);

    std::vector<uint8_t> contact_symm_key(raw_data.begin() + FIELD_CLIENT_ID_SIZE,
                                            raw_data.end());
    // we do not check the contact id validity
    for(auto contact : _client->contacts)
    {
        if(contact.is_id_equal(contact_id))
        {
            contact.setSymmKey(contact_symm_key);
        }
    }

    std::cout << "Received Symmetric key: \n";
    for(auto byte : contact_symm_key)
    {
        std::cout << byte << " ";
    }
    std::cout << std::endl;
}
*/