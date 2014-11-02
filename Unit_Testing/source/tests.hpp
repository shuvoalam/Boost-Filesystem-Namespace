#ifndef TESTS_HPP_INCLUDED
#define TESTS_HPP_INCLUDED
#define BOOST_TEST_MODULE filesystem_namespace_module
#include <boost/test/unit_test.hpp>

#ifdef COPY_PATH_TESTS
#define COPY_PATH_TESTS
#include "filesystem.hpp"
#include <string>
#include <vector>
#endif

#ifdef DELETE_PATH_TESTS
#define DELETE_PATH_TESTS
#include <iostream>
#include <string>
#include <vector>
#include "filesystem.hpp"
#endif

#ifdef MOVE_RENAME_AND_CREATE_TESTS
#define MOVE_RENAME_AND_CREATE_TESTS
#include <iostream>
#include <string>
#include <vector>
#include <boost/filesystem.hpp>

#include "filesystem.hpp"
#endif


namespace
{
#ifdef COPY_PATH_TESTS
    unsigned int percent_complete(const unsigned int&, const unsigned int&);
    
    
    inline unsigned int percent_complete(const unsigned int& prog, const unsigned int& max)
    {
        return (((float)prog / (float)max) * 100);
    }
    
    
#endif
    
    
#ifdef DELETE_PATH_TESTS
    unsigned int percent_complete(const unsigned int&, const unsigned int&);
    
    
    inline unsigned int percent_complete(const unsigned int& prog, const unsigned int& max)
    {
        return (((float)prog / (float)max) * 100);
    }
    
    
#endif
    
#ifdef MOVE_RENAME_AND_CREATE_TESTS
    
    
    
#endif
    
}

BOOST_AUTO_TEST_SUITE(copy_path_test)

#ifdef COPY_PATH_TESTS

/*
BOOST_AUTO_TEST_CASE(copy_test_one)
{
    std::string source("/home/jonathan/Desktop/Link to C++/Current_Projects/filesystem_namespace/Unit Testing/filesystem.cpp"),
            destination("/home/jonathan/Desktop/Link to C++/Current_Projects/filesystem_namespace/Unit Testing/destination");
    fsys::result_data_boolean result;
    result = fsys::fcopy(source, destination);
    if(!result.value)
    {
        std::cout<< "\n\n"<< result.error<< "\n\n";
    }
    result = fsys::fcopy("/home/jonathan/netbeans-8.0", destination);
    if(!result.value)
    {
        std::cout<< "\n\n"<< result.error<< "\n";
    }
    result = fsys::fcopy("/home/jonathan/Desktop/Link to C++/Current_Projects/filesystem_namespace/Unit Testing",
            destination);
    if(!result.value)
    {
        std::cout<< "\n\n"<< result.error<< "\n";
    }
}
*/

BOOST_AUTO_TEST_CASE(copy_iterator_test)
{
    using fsys::copy_iterator_class;
    using namespace std;
    
    unsigned long long count(0), bcount(0);
    
    copy_iterator_class copy_iter("/home/jonathan/Documents/School",
            "/home/jonathan/Desktop/Link to C++/Current_Projects/filesystem_namespace/Unit Testing/destination");
    vector<string> fails;
    
    count = (copy_iter.count_from_beg() + copy_iter.count_from_end());
    
    cout<< "Paths to copy: "<< count<< endl;
    
    while(!copy_iter.at_end())
    {
        bcount = copy_iter.count_from_beg();
        cout<< bcount<< '\n';
        if(!copy_iter.err.value)
        {
            fails.push_back(std::string("\"") + copy_iter.value() + "\"\nREASON: " + copy_iter.err.error);
        }
        copy_iter++;
    }
    if(fails.size() > 0)
    {
        cout<< fails.size()<< " failed operations.\n\n";
        for(std::string s : fails) cout<< s<< '\n';
    }
}

#endif


#ifdef DELETE_PATH_TESTS

BOOST_AUTO_TEST_CASE(delete_path_test_case_one)
{
    using namespace fsys;
    using namespace std;
    unsigned int count(0);
    
    std::vector<string> fails;
    string temps;
    copy_iterator_class copy_it;
    
    try
    {
        copy_it = copy_iterator_class("/home/jonathan/Desktop/Link to C++/Current_Projects/filesystem_namespace/Unit Testing/source",
            "/home/jonathan/Desktop/Link to C++/Current_Projects/filesystem_namespace/Unit Testing/destination");
    }
    catch(...)
    {
        cout<< "error, path doesn't exist."<< endl;
        cin.get();
        std::abort();
    }
    delete_iterator_class del_it;
    
    count = copy_it.count_from_end();
    
    while(!copy_it.at_end())
    {
        copy_it++;
        cout<< "Percent complete: "<< percent_complete(copy_it.count_from_beg(), count)<< '\n';
        if(!copy_it.err.value)
        {
            fails.push_back(copy_it.err.error);
        }
    }
    if(fails.size() > 0)
    {
        cout<< "Failed copies: "<< fails.size()<< '\n';
    }
    cout<< "press enter...\n";
    cin.get();
    count = del_it.count_from_end();
    cout<< "\n\n\n\n";
    try
    {
        del_it = delete_iterator_class("/home/jonathan/Desktop/Link to C++/Current\
_Projects/filesystem_namespace/Unit Testing/destination/source");
    }
    catch(...)
    {
        cout<< "error, path doesn't exist."<< endl;
        cin.get();
        std::abort();
    }
    
    fails.erase(fails.begin(), fails.end());
    
    while(!del_it.at_end())
    {
        temps = del_it.value();
        del_it++;
        cout<< "Paths left: "<< del_it.count_from_end()<< '\n';
        //cout<< "\""<< temps<< "\"\n";
        if(!del_it.err.value)
        {
            fails.push_back(("\"" + temps + "\"\nREASON: " + del_it.err.error));
        }
    }
    if(fails.size() > 0)
    {
        cout<< "\n\nERRORS: ("<< fails.size()<< ")\n\n";
        for(std::string s : fails) cout<< s<< '\n';
    }
}


#endif


#ifdef MOVE_RENAME_AND_CREATE_TESTS

BOOST_AUTO_TEST_CASE(move_rename_and_create_test_case)
{
    using namespace fsys;
    using namespace std;
    
    tree_iterator_class diter;
    copy_iterator_class copy_it;
    delete_iterator_class del_it;
    string source(current_path() + "/source folder"), 
            dest(current_path() + "/destination"),
            data_folder("/home/jonathan/Desktop/Git Repositories"), 
            new_name("Test rename function"), temps;
    
    cout<< "Creating destination folder: \n";
    create_folder(dest);
    create_folder(source);
    cout<< "done, press enter...\n\n";
    cin.get();
    
    diter = tree_iterator_class(data_folder);
    copy_it = copy_iterator_class(data_folder, source);
    cout<< "Filling source folder with "<< copy_it.count_from_end()<< " contents\n";
    while(!copy_it.at_end())
    {
        cout<< "Copying: \""<< copy_it.value()<< "\"\n";
        copy_it++;
    }
    cout<< "done, press enter...\n\n";
    cin.get();
    
    temps = (source + '/' + boost::filesystem::path(data_folder).filename().string());
    cout<< "Renaming: \""<< temps<< "\" to \""<< new_name<< "\"\n";
    frename(temps, (source + '/' + new_name));
    cout<< "done, press enter to continue...\n\n";
    cin.get();
    
    temps = (source + '/' + new_name);
    cout<< "moveing \""<< temps<< "\" into \""<< dest<< "\"\n";
    fmove(temps, dest);
    cout<< "done, press enter...\n\n";
    cin.get();
    
    cout<< "Removing \""<< source<< "\"\n";
    fdelete(source);
    cout<< "done, press enter...\n\n";
    cin.get();
    
    cout<< "Removing \""<< dest<< "\"\n";
    del_it = delete_iterator_class(dest);
    while(!del_it.at_end()) del_it++;
    cout<< "done, press enter...\n\n";
    cin.get();
    
}


#endif


BOOST_AUTO_TEST_SUITE_END()


#endif