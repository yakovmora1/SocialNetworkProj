#include "terminal.hpp"
#include <cstdlib>


void Terminal::init()
{
    std::cout << MAIN_MENU_MESSAGE << std::endl;
}


std::string Terminal::get_client_name()
{
    std::string input_name;

    while(true)
    {
        std::cout << "Enter Your name: " << std::endl;
        std::getline(std::cin, input_name);

        if(NAME_SIZE < input_name.length())
        {
            std::cout << "Illigal name, enter again" << std::endl;
            continue;
        }
        return input_name;
    }
}

std::string Terminal::get_destination_name()
{
    std::string input_name;

    while(true)
    {
        std::cout << "Enter Destination name: " << std::endl;
        std::getline(std::cin, input_name);

        if(NAME_SIZE < input_name.length())
        {
            std::cout << "Illigal name, enter again" << std::endl;
            continue;
        }
        return input_name;
    }
}

std::string Terminal::get_txt_msg()
{
 std::string txt_msg;

    while(true)
    {
        std::cout << "Enter Text message: " << std::endl;
        std::getline(std::cin, txt_msg);

        if((MAX_PAYLOAD_SIZE - MIN_MESSAGE_SIZE) < txt_msg.length())
        {
            std::cout << "Illigal text, enter again" << std::endl;
            continue;
        }
        return txt_msg;
    }
}


uint16_t Terminal::get_operation_num()
{
    std::string op_number;
    char op_number_to_convert[3] = "";

    while(true)
    {
        std::getline(std::cin, op_number);
        if(2 < op_number.length() || 0 == op_number.length())
        {
            std::cout << "Illigal operation number - try again" << std::endl;
            continue;
        }
        break;
    }

    if(1 == op_number.length()) 
    {

    op_number_to_convert[0] = op_number.at(0);
    op_number_to_convert[1] = '0';
    }
    else
    {
        op_number_to_convert[0] = op_number.at(0);
        op_number_to_convert[1] = op_number.at(1);
    }

    return (uint16_t)strtoul(op_number_to_convert, NULL, 10);
}

