#include <iostream>
#include "client.hpp"


void main()
{
    std::cout << "Client Started" << std::endl;
    Terminal term;
    Client c(term);

    try
    {
        c.start();
    }

    catch( const std::exception & e)
    {
        std::cout << "Exception" << std::endl;
        std::cout << e.what() << std::endl;
    }
}