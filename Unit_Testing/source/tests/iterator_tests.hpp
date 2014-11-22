#define FILESYSTEM_USE_RUNTIME_ERRORS true

#ifndef ITERATOR_TESTS_HPP_INCLUDED
#define ITERATOR_TESTS_HPP_INCLUDED
#include <unittest++/UnitTest++.h>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>
#include <utility>

#include "filesystem.hpp"
#include "test_globals.hpp"

typedef class test_fixture_class
{
public:
    explicit test_fixture_class() : test_folder(unittest_TEST_FOLDER), 
                    dest_folder(unittest_DEST),
                    build_dir(unittest_PARENT_OF_TEST_FOLDER)
    {
        system("../setup_test");
    }
    
    ~test_fixture_class(){}
    
    std::string test_folder, dest_folder, build_dir;
    
} test_fixture_class;

namespace
{
    template<typename type>
    std::string to_string(const type& t)
    {
        std::stringstream ss;
        ss<< t;
        return ss.str();
    }
    
    bool copy(const std::string&, const std::string&);
    unsigned long content_count(const std::string&);
    std::pair<std::string, std::string> split_subdir(const std::string&, const std::string&);
    bool check_copy(const std::string&, const std::string&);
    std::string parent_path(const std::string&);
    bool del(const std::string&);
    void find_invalid_paths();
    bool test_fdelete(const std::string&);
    std::string random_sub_path(const std::string&, fsys::result_data_boolean (*)(const std::string&));
    std::string new_pathname(const std::string&);
    std::string filename(const std::string&);
    std::string extension(const std::string&);
    std::string remove_extension(const std::string&);
    fsys::result_data_boolean match_any_path(const std::string&);
    fsys::result_data_boolean can_delete_match_function(const std::string&);
    bool has_extension(const std::string&);
    
    
#if FILESYSTEM_USE_RUNTIME_ERRORS == true
    #define ethrow(MSG) throw std::runtime_error(("EXCEPTION THROWN: \n\"" + std::string(__FILE__)\
+ "\"\n @ Line " + to_string(__LINE__) + ": \n" + std::string(MSG)))

#else
    #define ethrow(MSG)
#endif
    
    
    
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
            if(!it.success.value)
            {
                cout<< "Error: \""<< it.success.error<< "\""<< endl;
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
                if(!it.success.value)
                {
                    using std::cout;
                    using std::endl;
                    
                    cout<< endl<< "del(const std::string&): ERROR: \""<< 
                                    it.success.error<< "\""<< endl;
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
    
    inline std::string random_sub_path(const std::string& root, fsys::result_data_boolean (*match)(const std::string&))
    {
        using fsys::tree_riterator_class;
        using fsys::is_folder;
        
        unsigned long count(0), selection(0);
        
        if(!is_folder(root).value) return "";
        for(tree_riterator_class it(root); !it.at_end(); ++it)
        {
            if(match(it.value()).value) ++count;
        }
        selection = (rand() % count);
        count = 0;
        for(tree_riterator_class it(root); !it.at_end(); ++it)
        {
            if(match(it.value()).value)
            {
                if(count == selection) return it.value();
                ++count;
            }
        }
        return "";
    }
    
    inline fsys::result_data_boolean can_delete_match_function(const std::string& s)
    {
        fsys::result_data_boolean res;
        res.path = s;
        res.value = fsys::can_delete(s);
        return res;
    }
    
    inline fsys::result_data_boolean match_any_path(const std::string& s)
    {
        fsys::result_data_boolean res;
        res.value = (fsys::is_folder(s).value || fsys::is_file(s).value || fsys::is_symlink(s).value);
        res.path = s;
        return res;
    }
    
    inline bool test_fdelete(const std::string& root)
    {
        using fsys::can_delete;
        using fsys::is_folder;
        using fsys::is_symlink;
        using fsys::fdelete;
        
        /*we'll need to explicitly construct and destruct to eliminate possibility 
         * that it will be optimized out: */
        test_fixture_class *fixture(NULL);
        std::string temps;
        
        if(!is_folder(root).value) return false;
        for(unsigned int x = 0; x < 500; ++x)
        {
            fixture = new test_fixture_class;
            temps = random_sub_path(root, can_delete_match_function);
            if(!fdelete(temps).value && !temps.empty())
            {
                if(!is_folder(temps).value)
                {
                    std::cout<< "fdelete test: failed to delete \""<< temps<< "\""<< std::endl;
                    return false;
                }
            }
            delete fixture;
        }
        return true;
    }
    
    inline std::string filename(const std::string& s)
    {
        if(s.empty()) return s;
        std::string temps(s);
        std::size_t pos(temps.rfind(fsys::pref_slash()));
        if(pos != std::string::npos) temps.erase(temps.begin(), (temps.begin() + pos + 1));
        return temps;
    }
    
    inline std::string extension(const std::string& s)
    {
        if(s.empty()) return s;
        std::string temps(filename(s));
        std::size_t pos(temps.rfind('.'));
        if(pos != std::string::npos) temps.erase(temps.begin(), (temps.begin() + pos));
        return temps;
    }
    
    inline std::string remove_extension(const std::string& s)
    {
        if(s.empty()) return s;
        std::string temps(filename(s));
        std::size_t pos(temps.rfind('.'));
        if(pos != std::string::npos) temps.erase((temps.begin() + pos), temps.end());
        return (parent_path(s) + fsys::pref_slash() + temps);
    }
    
    inline bool has_extension(const std::string& s)
    {
        return (filename(s).rfind('.') != std::string::npos);
    }
    
    inline std::string new_pathname(const std::string& s)
    {
        using fsys::is_file;
        using fsys::is_folder;
        using fsys::is_symlink;
        using boost::filesystem::path;
        
        unsigned long id(0);
        std::string temps(s);
        fsys::result_data_boolean (*match)(const std::string&);
        
        if(fsys::is_symlink(s).value) match = fsys::is_symlink;
        else if(fsys::is_file(s).value) match = fsys::is_file;
        else if(fsys::is_folder(s).value) match = fsys::is_folder;
        else ethrow("could not recognize path: \"" + s + "\"");
        while(match(temps).value)
        {
            if(has_extension(s)) temps = (remove_extension(s) + to_string(++id) + extension(s));
            else temps = (s + to_string(id));
        }
        return temps;
    }
    
    
}

TEST_FIXTURE(test_fixture_class, recursive_directory_iterator_test_case)
{
    using fsys::tree_riterator_class;
    
    tree_riterator_class it(test_folder);
    CHECK(it.value() != test_folder);
    std::cout<< "recursive_directory_iterator_test_case DONE"<< std::endl;
}

TEST_FIXTURE(test_fixture_class, directory_iterator_test_case)
{
    using fsys::tree_iterator_class;
    
    tree_iterator_class it(test_folder);
    CHECK(it.value() != test_folder);
    std::cout<< "directory_iterator_test_case DONE"<< std::endl;
}

TEST_FIXTURE(test_fixture_class, copy_iterator_test_case)
{
    bool result(false);
    
    if(fsys::create_folder(dest_folder).value) result = copy(test_folder, dest_folder);
    CHECK(result);
    std::cout<< "copy_iterator_test_case DONE"<< std::endl;
}

TEST_FIXTURE(test_fixture_class, delete_iterator_test_case)
{
    bool result(false);
    
    if(fsys::is_folder(test_folder).value && !fsys::is_symlink(test_folder).value)
    {
        result = del(test_folder);
    }
    CHECK(result);
    std::cout<< "delete_iterator_test_case DONE"<< std::endl;
}

TEST_FIXTURE(test_fixture_class, can_delete_test_case)
{
    using fsys::can_delete;
    using fsys::is_folder;
    using fsys::is_file;
    using std::cout;
    using std::endl;
    
    std::string temps(random_sub_path(test_folder, fsys::is_file));
    
    CHECK(!can_delete(test_folder));
    CHECK(can_delete((test_folder + fsys::pref_slash() + "empty folder")));
    
    if(!temps.empty())
    {
        CHECK(can_delete(random_sub_path(test_folder, fsys::is_file)));
    }
    else
    {
        cout<< "Error: couldn't locate a random file."<< endl;
        CHECK(false);
    }
    std::cout<< "can_delete_test_case DONE"<< std::endl;
}

TEST_FIXTURE(test_fixture_class, fdelete_test_case)
{
    CHECK(test_fdelete(test_folder));
    std::cout<< "fdelete_test_case DONE"<< std::endl;
}

TEST(create_folder_test_case)
{
    using fsys::create_folder;
    using fsys::is_folder;
    using fsys::is_symlink;
    using fsys::result_data_boolean;
    
    std::string temps(std::string(unittest_PARENT_OF_TEST_FOLDER) + "/created folder");
    result_data_boolean res(create_folder(temps));
    CHECK(is_folder(temps).value && !is_symlink(temps).value);
    std::cout<< "create_folder_test_case DONE"<< std::endl;
}

TEST_FIXTURE(test_fixture_class, rename_paths_test_case)
{
    using fsys::is_file;
    using fsys::is_folder;
    using fsys::is_symlink;
    using fsys::frename;
    
    std::string temps, new_path;
    
    for(unsigned int x = 0; x < 500; ++x)
    {
        temps = random_sub_path(test_folder, match_any_path);
        if(!temps.empty())
        {
            new_path = new_pathname(temps);
            CHECK(frename(temps, new_path).value && 
                    (is_folder(new_path).value || is_file(new_path).value || is_symlink(new_path).value));
        }
        else
        {
            std::cout<< "couldn't find a path to test...?"<< std::endl;
        }
    }
    std::cout<< "rename_paths_test_case DONE"<< std::endl;
}

TEST_FIXTURE(test_fixture_class, move_path_test_case)
{
    using fsys::is_folder;
    using std::cout;
    using std::endl;
    using fsys::fmove;
    using fsys::is_file;
    using fsys::is_symlink;
    
    std::string temps, new_path;
    
    for(unsigned int x = 0; x < 100; ++x)
    {
        temps = random_sub_path(test_folder, match_any_path);
        if(!temps.empty())
        {
            new_path = (dest_folder + fsys::pref_slash() + filename(temps));
            if(!is_folder(new_path).value && !is_file(new_path).value && !is_symlink(new_path).value)
            {
                CHECK(fmove(temps, dest_folder).value && (is_folder(new_path).value || 
                        is_file(new_path).value || is_symlink(new_path).value));
            }
        }
    }
    cout<< "move_path_test_case DONE"<< endl;
}


#endif