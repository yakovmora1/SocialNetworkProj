#include <fstream>
#include <iostream>
#include <exception>
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

    _client_socket = socket(_sock_addrinfo->ai_family,
                            _sock_addrinfo->ai_socktype, 
                            _sock_addrinfo->ai_protocol);
                            
    if(INVALID_SOCKET == _client_socket)
    {
        throw std::runtime_error("Failed to create socket");
    }

    // create temp ID
    _id = std::vector<uint8_t>(FIELD_CLIENT_ID_SIZE, 1);
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

    while(true)
    {
        /* connect to the server */
        if(0 != connect(_client_socket, _sock_addrinfo->ai_addr,
                        _sock_addrinfo->ai_addrlen))
        {
            throw std::runtime_error("Failed to connect to server");
        }

        std::cout << "Client connected! " << std::endl;

        /* receive input from the terminal interface 
            and call the relevant operation via operation
                factory*/
        /*
        const uint16_t op_num = OP_REGISTER;
        op = OperationFactory::get_factory()->create_operation(op_num,
                                                            this,
                                                            _client_socket);
        if(nullptr == op)
        {
            std::cout << "Failed to initiate operation" << std::endl;
        }

        std::cout << "Calling get_operation_num" << std::endl;
        std::cout << op->get_operation_num() << std::endl; 
        std::cout << "Calling do_operation" << std::endl;
        std::vector<uint8_t> resp = op->do_operation();

        for(auto byte : resp)
        {
            std::cout << byte;
            std::cout <<" ";
        }
        std::cout << std::endl;
        
        */

        /*
        const uint16_t op_num2 = OP_CLIENT_LIST;
        op = OperationFactory::get_factory()->create_operation(op_num2,
                                                            this,
                                                            _client_socket);
        if(nullptr == op)
        {
            std::cout << "Failed to initiate operation" << std::endl;
        }

        std::cout << "Calling get_operation_num" << std::endl;
        std::cout << op->get_operation_num() << std::endl; 
        std::cout << "Calling do_operation" << std::endl;
        std::vector<uint8_t>resp = op->do_operation();

        for(auto contact : contacts)
        {
            std::cout << contact << std::endl;
        }
        */
       /* 
        const uint16_t op_num = OP_PUBLIC_KEY_REQUEST;
        op = OperationFactory::get_factory()->create_operation(op_num,
                                                            this,
                                                            _client_socket);
        if(nullptr == op)
        {
            std::cout << "Failed to initiate operation" << std::endl;
        }

        std::cout << "Calling get_operation_num" << std::endl;
        std::cout << op->get_operation_num() << std::endl; 
        std::cout << "Calling do_operation" << std::endl;
        std::vector<uint8_t>resp = op->do_operation();

        for(auto contact : contacts)
        {
            std::cout << contact << std::endl;
        }
        break;
        */
       /*
        const uint16_t op_num = OP_GET_SYM_KEY;
        op = OperationFactory::get_factory()->create_operation(op_num,
                                                            this,
                                                            _client_socket);
        if(nullptr == op)
        {
            std::cout << "Failed to initiate operation" << std::endl;
        }

        std::cout << "Calling get_operation_num" << std::endl;
        std::cout << op->get_operation_num() << std::endl; 
        std::cout << "Calling do_operation" << std::endl;
        std::vector<uint8_t>resp = op->do_operation();

        */
        
        const uint16_t op_num = OP_SEND_SYM_KEY;
        op = OperationFactory::get_factory()->create_operation(op_num,
                                                            this,
                                                            _client_socket);
        if(nullptr == op)
        {
            std::cout << "Failed to initiate operation" << std::endl;
        }

        std::cout << "Calling get_operation_num" << std::endl;
        std::cout << op->get_operation_num() << std::endl; 
        std::cout << "Calling do_operation" << std::endl;
        std::vector<uint8_t>resp = op->do_operation();

        for(auto contact : contacts)
        {
            std::cout << contact << std::endl;
        }

        break;

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