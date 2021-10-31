#include "contact.hpp"


bool Contact::is_id_equal(std::vector<uint8_t> id)
{
    /* as follows from the documentation std::vector uses ==
    for compariosn on each element of the vector (in our case uint8_t)
    which is what we want */
    if(_id == id)
        return true;
    return false;
}


bool Contact::is_name_equal(std::string name)
{
    if(_name == name)
        return true;
    return false;
}