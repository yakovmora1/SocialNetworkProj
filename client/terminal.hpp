#pragma once
#include <string>
#include "common.hpp"
#include "message.hpp"


#ifndef _TERMINAL_H
#define _TERMINAL_H


#define MAIN_MENU_MESSAGE ("MessageU client at your service.\n", \
                        "10) Register\n" \
                        "20) Request for clients list\n" \
                        "30) Request for public key\n" \
                        "40) Request for waiting messages\n" \
                        "50) Send a text message\n" \
                        "51) Send a request for symmetric key\n" \
                        "52) Send your symmetric key\n" \
                        "0) Exit client\n")


class Terminal
{
    public:
        Terminal(){}
        /*Note: no terminal close - because it's not needed by now */
        void init();
        std::string get_destination_name();
        std::string get_txt_msg();
        std::string get_client_name();
        uint16_t get_operation_num();
};

#endif //_TERMINAL_H