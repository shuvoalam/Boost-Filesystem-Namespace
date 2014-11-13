#ifndef ITERATOR_TESTS_HPP_INCLUDED
#define ITERATOR_TESTS_HPP_INCLUDED
#include <unittest++/UnitTest++.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <utility>

#include "filesystem.hpp"
#include "test_globals.hpp"

typedef class test_fixture_class
{
public:
    explicit test_fixture_class() : test_folder(unittest_TEST_FOLDER), 
                    dest_folder(unittest_DEST)
    {
        system("../setup_test");
    }
    
    ~test_fixture_class(){}
    
    std::string test_folder, dest_folder;
    
} test_fixture_class;

namespace
{
    bool copy(const std::string&, const std::string&);
    unsigned long content_count(const std::string&);
    std::pair<std::string, std::string> split_subdir(const std::string&, const std::string&);
    bool check_copy(const std::string&, const std::string&);
    std::string parent_path(const std::string&);
    bool del(const std::string&);
    void find_invalid_paths();
    
    
    
    inline unsigned long content_count(const std::string& folder)
    {
        using fsys::tree_riterator_class;
        
        tree_riterator_class it(folder);
        return it.count_from_end();
    }
    
    inline std::string parent_path(const std::string& s)
    {
        std::string temps(s);
        std::size_t pos(temps.rfind(fsys::pref_slash()));
        
        if(pos != std::string::npos)
        {
            temps.erase((temps.begin() + pos), temps.end());
        }
        return temps;
    }
    
    inline bool check_copy(const std::string& from, const std::string& to)
    {
        using fsys::tree_riterator_class;
        using std::cout;
        using std::endl;
        using boost::filesystem::exists;
        using boost::filesystem::path;
        
        if(fsys::is_symlink(from).value)
        {
            return fsys::is_symlink(to + fsys::pref_slash() + path(from).filename().string()).value;
        }
        else if(fsys::is_file(from).value)
        {
            return fsys::is_file(to + fsys::pref_slash() + path(from).filename().string()).value;
        }
        else if(fsys::is_folder(from).value)
        {
            for(tree_riterator_class it(from); !it.at_end(); ++it)
            {
                if(!exists(path((to + split_subdir(parent_path(from), it.value()).second))))
                {
                    using std::cout;
                    using std::endl;
                    
                    cout<< endl<< "check_copy:\n from = \""<< from<< "\""<< endl;
                    cout<< "to = \""<< (to + split_subdir(parent_path(from), it.value()).second)<< "\""<< endl;
                    return false;
                }
            }
            return (content_count(from) == (content_count(to) - 1));
        }
        else return false;
        return true;
    }
    
    /** Splits a directory into a root, and a relative subdirectory.  Ex: 
     * 
     * /a/root/path
     * /a/root/path/with/a/child
     * 
     * returns: std::pair<std::string, std::string>("/a/root/path", "/with/a/child") */
    inline std::pair<std::string, std::string> split_subdir(const std::string& root, const std::string& sub)
    {
        std::pair<std::string, std::string> p;
        if(!sub.empty())
        {
            p.first = root;
            if(sub.size() > root.size())
            {
                p.second = sub;
                p.second.erase(p.second.begin(), (p.second.begin() + p.first.size()));
            }
            else p.second = "";
        }
        return p;
    }
    
    //copy function that will test copy, and check the results. 
    inline bool copy(const std::string& from, const std::string& to)
    {
        using fsys::copy_iterator_class;
        using std::cout;
        using std::endl;
        
        unsigned long total(0), progress(0);
        short percent(0), prev_perc(0);
        
        cout<< "counting, please wait..."<< endl;
        total = content_count(from);
        cout<< "done counting!"<< endl;
        
        cout<< endl;
        cout<< "testing copy..."<< endl<< endl;
        for(copy_iterator_class it(from, to); !it.at_end(); ++it, ++progress)
        {
            if(!it.err.value)
            {
                cout<< "Error: \""<< it.err.error<< "\""<< endl;
                it.skip();
            }
            prev_perc = percent;
            percent = (100 * ((long double)progress / (long double)total));
            if(percent != prev_perc)
            {
                cout<< "Percent complete: %"<< percent<< endl;
            }
        }
        cout<< "copy test done."<< endl;
        return check_copy(from, to);
    }
    
    inline bool del(const std::string& p)
    {
        if(fsys::is_symlink(p).value || fsys::is_file(p).value)
        {
            return fsys::fdelete(p).value;
        }
        else if(fsys::is_folder(p).value)
        {
            for(fsys::delete_iterator_class it(p); !it.at_end();)
            {
                if(!it.err.value)
                {
                    using std::cout;
                    using std::endl;
                    
                    cout<< endl<< "del(const std::string&): ERROR: \""<< 
                                    it.err.error<< "\""<< endl;
                    cout<< "bool del(const std::string&): skipping: \""<< 
                                    it.value()<< "\""<< endl;
                    it.skip();
                    continue;
                }
                ++it;
            }
            return !boost::filesystem::exists(boost::filesystem::path(p));
        }
        return false;
    }
    
    inline void find_invalid_paths()
    {
        using fsys::tree_riterator_class;
        using fsys::is_file;
        using fsys::is_folder;
        using fsys::is_symlink;
        using std::cout;
        using std::endl;
        
        for(unsigned int x = 0; x < 2; ++x) cout<< endl;
        cout<< "Please wait, finding invalid paths in home...."<< endl;
        for(tree_riterator_class it("/home/jonathan"); !it.at_end(); ++it)
        {
            if(!is_folder(it.value()).value && !is_file(it.value()).value && 
                            !is_symlink(it.value()).value)
            {
                using std::cout;
                using std::endl;
                
                cout<< "Invalid: \""<< it.value()<< "\""<< endl;
            }
        }
        for(unsigned int x = 0; x < 2; ++x) cout<< endl;
    }
    
    
}

TEST_FIXTURE(test_fixture_class, copy_iterator_test_case)
{
    bool result(false);
    
    if(fsys::create_folder(dest_folder).value) result = copy(test_folder, dest_folder);
    CHECK(result);
}

TEST_FIXTURE(test_fixture_class, delete_iterator_test_case)
{
    bool result(false);
    
    if(fsys::is_folder(test_folder).value && !fsys::is_symlink(test_folder).value)
    {
        result = del(test_folder);
    }
    CHECK(result);
    find_invalid_paths();
}


#endif