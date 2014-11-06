#ifndef RANDOM_TEST_DATA_HPP_INCLUDED
#define RANDOM_TEST_DATA_HPP_INCLUDED

namespace rdata
{
    unsigned int random_number(const unsigned int&, const unsigned int&);
    char random_char();
    std::string random_string(const unsigned int&, const unsigned int&);
    
}

#endif