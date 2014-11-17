#include <unittest++/UnitTest++.h>
#include <cstdlib>

//change this to build a test suite of choice
#define TEST_SUITE_INCLUDE 1


#if TEST_SUITE_INCLUDE == 1
#include "filesystem_class_aid_function_tests.hpp"
#elif TEST_SUITE_INCLUDE == 2
#include "iterator_tests.hpp"
#endif

/* attn write a script that copies the test folder (which should be full of subdirectories, 
 * each one treated as it's own root during testing to simulate different 
 * environments) into the build folder.*/

int main(int ac __attribute__((unused)), char **av __attribute__((unused)))
{
    srand(time(NULL));
    return UnitTest::RunAllTests();
}