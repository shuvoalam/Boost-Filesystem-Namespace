#include <string>
#include <cstdlib>

#include "random_test_data.hpp"
#include "global_defines.hpp"

namespace
{
}

namespace rdata
{
    unsigned int random_number(const unsigned int& min, const unsigned int& max)
    {
        unsigned int num(rand() % ((min < max) ? (max - min) : (min - max)));
        return (num + ((min < max) ? min : max));
    }
    
    char random_char()
    {
        std::string characters(LETTERS + std::string(NUMBERS) + SPECIALS);
        return characters.at(random_number(0, (characters.size() - 1)));
    }
    
    std::string random_string(const unsigned int& min, const unsigned int& max)
    {
        unsigned int size(random_number(min, max));
        std::string s;
        
        for(unsigned int x = 0; x < size; x++) s += random_char();
        return s;
    }
    
    
}