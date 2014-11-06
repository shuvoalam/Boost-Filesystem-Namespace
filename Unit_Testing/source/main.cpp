#include <unittest++/UnitTest++.h>
#include <cstdlib>

#include "filesystem_class_aid_function_tests.hpp"


int main(int ac __attribute__((unused)), char **av __attribute__((unused)))
{
    srand(time(NULL));
    return UnitTest::RunAllTests();
}