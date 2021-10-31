#include <string>
#include <vector>
#include <iostream>
#include "common.hpp"

#ifndef _CONTACT_H
#define _CONTACT_H


class Contact
{
    std::string _name;
    std::vector<uint8_t> _id;
    std::string _pub_key;
    std::vector<uint8_t> _symm_key;

    public:
    Contact(std::string name, std::vector<uint8_t> id, std::string pub_key, std::vector <uint8_t> symm_key)
    {
        _name = name;
        _id = id;
        _pub_key = pub_key;
        _symm_key = symm_key;

    }

    Contact(std::string name, std::vector<uint8_t> id)
    {
        _name = name;
        _id = id;
    }

    bool is_name_equal(std::string name);
    bool is_id_equal(std::vector<uint8_t> id);

    const std::string getName()
    {
        return _name;
    }

    const std::vector<uint8_t> getID()
    {
        return _id;
    }

    const std::string getPubKey()
    {
        return _pub_key;
    }

    void setPubKey(std::string public_key)
    {
        _pub_key = public_key;
    }

    const std::vector<uint8_t> getSymKey()
    {
        return _symm_key;
    }

    void setSymmKey(std::vector<uint8_t> sym_key)
    {
        _symm_key = sym_key;
    }

    friend std::ostream & operator<<(std::ostream &output, const Contact & contact)
    {
        output << "###########Contact Object######### ";
        output << "ID: ";
        for(auto byte: contact._id)
        {
            output << "0x";
            output << neeble_to_hex(byte >> 4);
            output << neeble_to_hex(byte & 0xf);
            output << " ";
        }
        output << "\n";

        output << "Name:\n";
        output << contact._name << std::endl;
        return output;
    }
};

#endif //  _CONTACT_H
