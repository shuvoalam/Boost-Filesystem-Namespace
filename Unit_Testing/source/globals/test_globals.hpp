#ifndef TEST_GLOBALS_HPP_INCLUDED
#define TEST_GLOBALS_HPP_INCLUDED
#include <string>


//attn make sure that you change these according to your system:

//parent of the test folder.  This is hard coded so that parent_folder test can be properly tested.
#define unittest_PARENT_OF_TEST_FOLDER "/mnt/ENCRYPTED/C++/Finished_Projects/filesystem_namespace/Unit_Testing/build"

//the test folder under the build directory
#define unittest_TEST_FOLDER (unittest_PARENT_OF_TEST_FOLDER + std::string("/test_folder"))

#define unittest_DEST (std::string(unittest_PARENT_OF_TEST_FOLDER) + "/DESTINATION OF COPY TEST")

#endif