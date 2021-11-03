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

#define MIN_IP_ADDR (7)
#define MAX_PORT (65535)


class Client
{
    private:
        std::string _name;
        std::vector<uint8_t> _id;
        std::string _pub_key;
        std::string _private_key;
        std::vector<uint8_t> _symm_key;
        SOCKET _client_socket;
        addrinfo * _sock_addrinfo;

        error_code_t get_server_info(std::string & ip, std::string & port);
        void _read_client_creds();
        /* Winsock data */ 
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
            std::string name = "Dor";
            return name;
        }

        void setName(std::string name)
        {
            _name = name;
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
            std::string pub_key = std::string(PUBLIC_KEY_SIZE, '9');
            return pub_key;
        }

        const std::vector<uint8_t> getSymKey()
        {
            return _symm_key;
        }

        bool is_known_contact(std::string name);

        bool is_known_contact(std::vector<uint8_t> id);

        void save_client_creds();

        bool is_client_registered();


        std::string get_contact_name_by_id(std::vector<uint8_t> id);

        std::vector<uint8_t> get_contact_id_by_name(std::string name);
        
        std::vector<uint8_t> get_contact_symkey_by_id(std::vector<uint8_t> id);
        
        std::string get_contact_pubkey_by_id(std::vector<uint8_t> id);
        
        void update_contact_symm_key_by_id(std::vector<uint8_t> id,
                                            std::vector<uint8_t> symm_key);
};

#endif // _CLIENT_H