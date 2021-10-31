#include  <iostream>
#include <string>
#include <vector>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include "common.hpp"
#include "contact.hpp"
#include "terminal.hpp"

#ifndef _CLIENT_H
#define _CLIENT_H

/* Use Winsock2 library */
#pragma comment(lib, "Ws2_32.lib")


#define SERVER_INFO_FILE ("server.info")
#define MIN_IP_ADDR (7)
#define MAX_PORT (65535)


class Client
{
    private:
        std::string _name;
        std::vector<uint8_t> _id;
        std::string _pub_key;
        std::vector<uint8_t> _symm_key;
        SOCKET _client_socket;
        addrinfo * _sock_addrinfo;

        error_code_t get_server_info(std::string & ip, std::string & port);
        error_code_t init_client_info();
        error_code_t save_client_info(std::string name, std::vector<uint8_t>);

        /* Winsock dta */ 
        WSADATA wsa_data;

    public:
        Client(Terminal term_interface);
        ~Client();
        
        std::vector<Contact> contacts;
        void start();

        Terminal terminal;
        
        const std::string getName()
        {
            //return _name;
            std::string name = "Daniel";
            for(size_t i = name.length() + 1; i <= NAME_SIZE; i++)
            {
                name.append(FILLING_BYTE);
            }
            return name;
        }

        const std::vector<uint8_t> getID()
        {
            return _id;
        }

        void setID(std::vector<uint8_t> id)
        {
            _id = id;
        }

        const std::string getPubKey()
        {
            //return _pub_key;
            std::string pub_key = "1234567890ABCDEF";
            for(size_t i = pub_key.length() + 1; i <= PUBLIC_KEY_SIZE; i++)
            {
                pub_key.append(FILLING_BYTE);
            }
            return pub_key;
        }

        const std::vector<uint8_t> getSymKey()
        {
            return _symm_key;
        }

        bool is_known_contact(std::string name);

        bool is_known_contact(std::vector<uint8_t> id);
};

#endif // _CLIENT_H