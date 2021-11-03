#include <fstream>
#include <iostream>
#include <exception>
#include <cstdlib>
#include "operation_factory.hpp"
#include "client.hpp"



/*TODO: save the creds in me file! */
Client::Client(Terminal term_interface)
{
    std::string server_ip;
    std::string server_port;
    addrinfo  hints;
    
    
    _client_socket = INVALID_SOCKET;
    terminal = term_interface;
    /* init the winsock2 infrastructure */
    WSAStartup(MAKEWORD(2,2), &wsa_data);

    if(CLIENT__SUCCESS != get_server_info(server_ip, server_port))
    {
        throw std::runtime_error("Failed to initiate client");
    }

    /* setup the server socket */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    if(0 != getaddrinfo(server_ip.c_str(), server_port.c_str(),
                            &hints, &_sock_addrinfo))
    {
        throw std::runtime_error("Failed to getaddrinfo");
    }
    if(is_client_registered())
    {
        _read_client_creds();
        std::cout << "Wellcome " << _name << std::endl;
    }
    else
    {
        // create  ID
        _id = std::vector<uint8_t>(FIELD_CLIENT_ID_SIZE, 1);
        //TOOD generate pubkey aand privkey
        _private_key = std::string(160, '\x41');
    }
}


Client::~Client()
{
    if(INVALID_SOCKET != _client_socket)
    {
        closesocket(_client_socket);
        _client_socket = INVALID_SOCKET;
    }
}


error_code_t Client::get_server_info(std::string & ip, std::string & port)
{
    std::ifstream server_info_file(SERVER_INFO_FILE);
    std::string line;
    std::string temp_port;
    size_t delimiter_index = 0;


    if(!server_info_file.is_open())
    {
        // throw std::runtime_error("Failed to get server info");
        return  CLIENT__FAILED_TO_GET_SERVER_INFO;
    }

    std::getline(server_info_file, line);

    /* validate the input */
    delimiter_index = line.find(':') ;
    if((MIN_IP_ADDR > delimiter_index) || (std::string::npos == delimiter_index))
    {
        //  throw std::runtime_error("Invaled server info file!");
        return CLIENT__INVLAID_SERVER_INFO;
    }

    ip = line.substr(0, delimiter_index);
    port = line.substr(delimiter_index + 1, line.length());

    std::cout << "Server ip " << ip << " port: " << port << std::endl;
    return CLIENT__SUCCESS;
}


void Client::start()
{   
    Operation * op;
    OperationFactory * of = OperationFactory::get_factory();
    uint16_t op_num;

    while(true)
    {
        terminal.init();
        /*receive input from the terminal interface 
        and call the relevant operation via operation factory*/
        _client_socket = socket(_sock_addrinfo->ai_family,
                            _sock_addrinfo->ai_socktype, 
                            _sock_addrinfo->ai_protocol);
                            
        if(INVALID_SOCKET == _client_socket)
        {
            throw std::runtime_error("Failed to create socket");
        }

        /* connect to the server */
        if(0 != connect(_client_socket, _sock_addrinfo->ai_addr,
                        _sock_addrinfo->ai_addrlen))
        {
            throw std::runtime_error("Failed to connect to server");
        }

        op_num = terminal.get_operation_num();
        if(OP_EXIT == op_num)
        {
            closesocket(_client_socket);
            return;
        }

        op = of->create_operation(op_num, this, _client_socket);
        if(nullptr == op)
        {
            closesocket(_client_socket);
            std::cout << "Invalid Operation specified " << op_num << std::endl;
            continue;
        }

        std::cout << "Calling do_operation" << std::endl;
        std::vector<uint8_t>resp = op->do_operation();

        /*for(auto contact : contacts)
        {
            std::cout << contact << std::endl;
        }*/

        closesocket(_client_socket);
        std::cout <<  "Finished request" << std::endl;
    }
}


bool Client::is_known_contact(std::string name)
{
    for(auto contact : contacts)
    {
        if(contact.is_name_equal(name))
            return true;
    }
    return false;
}


bool Client::is_known_contact(std::vector<uint8_t> id)
{
    for(auto contact : contacts)
    {
        if(contact.is_id_equal(id))
            return true;
    }
    return false;
}


std::string Client::get_contact_name_by_id(std::vector<uint8_t> id)
{
    for(auto contact : contacts)
    {
        if(contact.is_id_equal(id))
        {
            return contact.getName();
        }
    }
    return {};
}


std::vector<uint8_t> Client::get_contact_id_by_name(std::string name)
{
    for(auto contact : contacts)
    {
        if(contact.is_name_equal(name))
        {
            return contact.getID();
        }
    }
    return {};
}


std::vector<uint8_t> Client::get_contact_symkey_by_id(std::vector<uint8_t> id)
{
    for(auto contact : contacts)
    {
        if(contact.is_id_equal(id))
        {
            return contact.getSymKey();
        }
    }
    return {};
}


std::string Client::get_contact_pubkey_by_id(std::vector<uint8_t> id)
{
    for(auto contact : contacts)
    {
        if(contact.is_id_equal(id))
        {
            return contact.getPubKey();
        }
    }
    return {};
}



void Client::update_contact_symm_key_by_id
(
    std::vector<uint8_t> id, 
    std::vector<uint8_t> symm_key
)
{
    for(auto contact : contacts)
    {
        if(contact.is_id_equal(id))
        {
            contact.setSymmKey(symm_key);
        }
    }
}

/* TODO: change it to is_me_file_exist  and add private variable _is_client_registered*/
bool Client::is_client_registered()
{
    /* we check  if ME file exist */
    std::ifstream me_file(CLIENT_ME_FILE);

    if(!me_file.is_open())
    {
        return false;
    }

    // check if me file size is acceptable (at least client id size)
    me_file.seekg(0, std::ios::end);
    if(FIELD_CLIENT_ID_SIZE >= me_file.tellg())
    {
        return false;
    }
    me_file.close();
    return true;
}


void Client::save_client_creds()
{
    // overwrite what already existed in me file
    std::ofstream me_file(CLIENT_ME_FILE, std::ofstream::trunc);

    if(me_file.is_open())
    {
        me_file << _name << "\n";

        /* write the client id as hex string */
        for(auto byte : _id)
        {
            if(static_cast<int>(byte) < 0x10)
            {
                me_file << "0";
                me_file << std::hex << static_cast<int>(byte);
            }
            else
            {
                me_file << std::hex << static_cast<int>(byte);
            }
        }
        me_file << "\n";

        /* write the private key */
        for(auto byte : _private_key)
        {   //TODO: change to base64
            me_file << byte;
        }
        me_file << "\n";
        me_file.close();
    }
    else
    {
        std::runtime_error("Failed to save credentials");
    }
}


/* no validation on me.info format */
void Client::_read_client_creds()
{
    std::ifstream me_file;
    std::string temp_client_id;
    char temp_byte[3] = "";
    uint8_t byte = 0;
    size_t i = 0;


    // we do nothing if client not registered
    if(!is_client_registered())
        return;

    me_file.open(CLIENT_ME_FILE);

    if(me_file.is_open())
    {
        //TODO: vulnerable code (this reads without newline)
        std::getline(me_file, _name);

        // hex ascii to bytes
        std::getline(me_file, temp_client_id);
        if(FIELD_CLIENT_ID_SIZE * 2 != temp_client_id.length())
        {
            std::runtime_error("ME file malfourmed!");
        }
        //clear the previously assigned id
        _id.clear();
        for(i = 0; i < temp_client_id.length(); i += 2)
        {
            // parse 2 hex ascii chars to byte (base 16)
            temp_byte[0] = temp_client_id.at(i);
            temp_byte[1] = temp_client_id.at(i + 1);
            byte = (uint8_t)strtoul(temp_byte, NULL, 16);
            _id.push_back(byte);
        }

        //TODO: private key chnage from base 64
        getline(me_file, _private_key);
    }
}