#ifndef FILESYSTEM_AIDE_FUNCTION_TESTS_HPP_INCLUDED
#define FILESYSTEM_AIDE_FUNCTION_TESTS_HPP_INCLUDED

//error reporting:
#ifndef ethrow

//turn throwing exceptions on/off.  turning this off could cause undefined behavior should an error occur.
#define FILESYSTEM_USE_RUNTIME_ERRORS true


#if FILESYSTEM_USE_RUNTIME_ERRORS == true

#include <stdexcept>
#include <exception>
#include <string>

#endif
#endif


#include <unittest++/UnitTest++.h>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <utility>

#include "filesystem.hpp"
#include "random_test_data.hpp"


typedef class test_fixture_class test_fixture_class;


typedef class test_fixture_class
{
public:
    explicit test_fixture_class()
    {
        system("/mnt/ENCRYPTED/C++_Dev/Current_Projects/filesystem_namespace/Unit_Testing/setup_test");
    }
    
    ~test_fixture_class(){}
    
private:
    
    
} test_fixture_class;


namespace
{
    template<class type>
    std::string itoa(const type& t)
    {
        std::ostringstream ss;
        ss<< t;
        return ss.str();
    }
    
    
    template std::string itoa<int>(const int&);
    bool is_error(const boost::system::error_code&);
    fsys::result_data_boolean boost_bool_funct(bool (*)(const boost::filesystem::path&,
            boost::system::error_code&), const std::string&);
    boost::filesystem::directory_iterator init_directory_iter(const boost::filesystem::path&);
    fsys::result_data_boolean recurs_folder_copy(const std::string&, const std::string&);
    bool copy_directories(const std::string&, const std::string&, const std::string&);
    boost::filesystem::recursive_directory_iterator init_directory_rec_iterator(const boost::filesystem::path&);
    std::string parent_path(const std::string&);
    bool is_child(const std::string&, const std::string&);
    std::string construct_new_path(const std::string&, const std::string&, const std::string&);
    fsys::result_data_boolean is_empty(const std::string&);
    std::pair<std::string, std::string> split_subdir(const std::string&, const std::string&);
    std::string dive(const unsigned int&, const std::string&);
    
    
#if FILESYSTEM_USE_RUNTIME_ERRORS == true
    #define ethrow(MSG) throw std::runtime_error(("EXCEPTION THROWN: \n\"" + std::string(__FILE__)\
+ "\"\n @ Line " + itoa(__LINE__) + ": \n" + std::string(MSG)))

#else
    #define ethrow(MSG)
#endif
    
    
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
    
    /** Returns a string that is "levels" levels into "path".  
     * A "level" is one level of the directory tree.  example: /mnt/run/etc is 3 "levels"
     * deep.*/
    inline std::string dive(const unsigned int& levels, const std::string& path)
    {
        size_t first_slash(path.find(fsys::pref_slash()));
        unsigned int pos(path.size()), lev(0);
        
        if(first_slash != std::string::npos)
        {
            for(unsigned int x = (first_slash + 1); ((x < path.size()) && (lev < levels)); ++x)
            {
                if(path[x] == fsys::pref_slash())
                {
                    ++lev;
                    pos = x;
                }
            }
            if(lev != levels) pos = path.size();
        }
        return path.substr(0, pos);
    }
    
    /* Copies a set of sub-folders into a target folder whether they exist or not. */
    inline bool copy_directories(const std::string& from, const std::string& to, const std::string& source)
    {
        if(!is_child(source, from) && (from != source))
        {
            ethrow("copy_directories: source is not a child of from.");
        }
        boost::system::error_code err;
        
        try
        {
            //create the root if it doesn't yet exist
            if(!boost::filesystem::is_directory(boost::filesystem::path(
                    to + boost::filesystem::path("/").make_preferred().string() +
                    boost::filesystem::path(from).filename().string())))
            {
                try
                {
                    boost::filesystem::copy_directory(boost::filesystem::path(from), 
                            boost::filesystem::path(to + boost::filesystem::path("/").make_preferred().string() + 
                                    boost::filesystem::path(from).filename().string()));
                }
                catch(const std::exception& e)
                {
                    ethrow(e.what());
                }
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        
        if((source != from) && fsys::is_folder(source).value && !fsys::is_symlink(source).value)
        {
            std::string temps(split_subdir(from, source).second);
            
            std::string newdest, newfrom;
            int tempi(0);
            
            for(unsigned int x = 0; x < temps.size(); x++) if(temps[x] == fsys::pref_slash()) ++tempi;
            for(int x = 1; x <= tempi; ++x)
            {
                newdest = (to + fsys::pref_slash() + 
                                boost::filesystem::path(from).filename().string() + 
                                dive(x, temps));
                newfrom = (from + dive(x, temps));
                try
                {
                    if(!fsys::is_folder(newdest).value && fsys::is_folder(newfrom).value && 
                            !fsys::is_symlink(newfrom).value)
                    {
                        boost::filesystem::copy_directory(newfrom, newdest, err);
                    }
                }
                catch(const std::exception& e)
                {
                    ethrow(e.what());
                }
            }
        }
        try
        {
            std::string new_path(to + split_subdir(parent_path(from), source).second);
            return (
                    !is_error(err) && 
                    boost::filesystem::is_directory(boost::filesystem::path(new_path), err) && 
                    !boost::filesystem::is_symlink(new_path));
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return false;
    }
    
    inline bool is_error(const boost::system::error_code& err)
    {
        return (err != boost::system::errc::success);
    }
    
    inline fsys::result_data_boolean boost_bool_funct(bool (*f)(const boost::filesystem::path&, 
            boost::system::error_code&), const std::string& s)
    {
        fsys::result_data_boolean result;
        boost::filesystem::path p(s);
        boost::system::error_code err;
        
        try
        {
            result.value = f(p, err);
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        if(is_error(err))
        {
            result.error = err.message();
            result.value = false;
        }
        return result;
    }
    
    inline boost::filesystem::directory_iterator init_directory_iter(const boost::filesystem::path& p)
    {
        if((!fsys::is_folder(p.string()) && !fsys::is_file(p.string())) || (fsys::is_folder(p.string()).value && 
                fsys::is_symlink(p.string()).value))
        {
            ethrow("[PROGRAMMING ERROR]  Cannot construct iterator with invalid pathname");
        }
        boost::filesystem::directory_iterator it;
        try
        {
            it = boost::filesystem::directory_iterator(p);
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return it;
    }
    
    inline boost::filesystem::recursive_directory_iterator init_directory_rec_iterator(const boost::filesystem::path& p)
    {
        if((!fsys::is_folder(p.string()) && !fsys::is_file(p.string())) || (fsys::is_folder(p.string()).value && 
                fsys::is_symlink(p.string()).value))
        {
            ethrow("[PROGRAMMING ERROR]  Cannot construct iterator with invalid pathname");
        }
        boost::filesystem::recursive_directory_iterator it;
        try
        {
            it = boost::filesystem::recursive_directory_iterator(p);
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return it;
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
    
    inline fsys::result_data_boolean recurs_folder_copy(const std::string& from, const std::string& to)
    {
        fsys::result_data_boolean res;
        fsys::tree_riterator_class it;
        std::vector<std::string> failed_paths;
        std::string temps;
        bool (*f)(const boost::filesystem::path&, boost::system::error_code&);
        
        switch(fsys::is_folder(from).value && !fsys::is_symlink(from).value && 
                fsys::is_folder(to).value && !fsys::is_symlink(to).value && 
                !is_child(to, from) && !is_child(from, to))
        {
            case true:
            {
                f = boost::filesystem::is_empty;
                switch(boost_bool_funct(f, from).value)
                {
                    case true:
                    {
                        boost::system::error_code ec;
                        res.value = true;
                        try
                        {
                            boost::filesystem::copy_directory(boost::filesystem::path(from),
                                    boost::filesystem::path(
                                    (to + boost::filesystem::path("/").make_preferred().string() + 
                                    boost::filesystem::path(from).filename().string())),
                                    ec);
                            if(is_error(ec))
                            {
                                ethrow(ec.message());
                                res.value = false;
                                res.error = ec.message();
                            }
                        }
                        catch(const std::exception& e)
                        {
                            res.value = false;
                            res.error = e.what();
                            if((sizeof e.what()) == 0)
                            {
                                ethrow("Empty error message thrown here!");
                            }
                        }
                    }
                    break;
                    
                    case false:
                    {
                        res.value = true;
                        try
                        {
                            it = fsys::tree_riterator_class(from);
                            while(!it.at_end())
                            {
                                if(fsys::is_folder(it.value()).value && !fsys::is_symlink(it.value()).value)
                                {
                                    if(!copy_directories(from, to, it.value()))
                                    {
                                        failed_paths.push_back(it.value());
                                    }
                                }
                                else if(fsys::is_file(it.value()).value || fsys::is_symlink(it.value()).value)
                                {
                                    temps = parent_path(it.value());
                                    (void)copy_directories(from, to, temps);
                                    switch(fsys::is_folder(temps).value && !fsys::is_symlink(temps).value)
                                    {
                                        case true:
                                        {
                                            try
                                            {
                                                temps = (to + split_subdir(parent_path(from), it.value()).second);
                                                if(!fsys::is_file(temps).value && !fsys::is_symlink(temps).value)
                                                {
                                                    boost::filesystem::copy(it.value(), temps);
                                                }
                                            }
                                            catch(const std::exception& e)
                                            {
                                                ethrow(e.what());
                                            }
                                        }
                                        break;

                                        case false:
                                        {
                                            failed_paths.push_back(it.value());
                                        }
                                        break;

                                        default:
                                        {
                                            failed_paths.push_back(it.value());
                                        }
                                        break;
                                    }
                                }
                                ++it;
                            }
                        }
                        catch(const std::exception& e)
                        {
                            res.value = false;
                            ethrow(e.what());
                        }
                    }
                    break;
                    
                    default:
                    {
                    }
                    break;
                }
            }
            break;
            
            case false:
            {
                res.value = false;
                res.error = ("Error: args not valid.  Recursive folder copy \
 algorithm can only copy a folder into a folder!\n\nFrom = \"" + from + "\"\n\n\
 To = \"" + to + "\"");
            }
            break;
            
            default:
            {
            }
            break;
        }
        if(failed_paths.size() > 0)
        {
            using namespace std;
            cout<< "\n\nFAILED ATTEMPTS TO COPY THE FOLLOWING PATHS:\n\n";
            for(std::vector<std::string>::const_iterator it = failed_paths.begin(); it != failed_paths.end(); ++it)
            {
                cout<< *it<< '\n';
            }
            cout<< '\n';
            res.value = false;
            res.error = "Error: Some contents of the folder could not be copied!";
        }
        return res;
    }
    
    /** returns true if one path is a subdirectory of another.  Useful for preventing
     * infinite recursive copies due to asanign use...*/
    inline bool is_child(const std::string& child, const std::string& parent)
    {
        bool ischild(child == parent);
        std::string temps(child);
        if(child.size() > parent.size())
        {
            temps.resize(parent.size());
            ischild = (temps == parent);
        }
        return ischild;
    }
    
    /* Creates a new path under a destination folder, given a root folder, a destination folder, 
     and a path that is assumed to be under the root folder.
     
     Example:
     cur: "/home/username/documents/essays/an essay.txt"  
     root: "/home/username/documents"
     Destination: "/home/username"
     
     Result: /home/username/essays/an essay.txt" */
    inline std::string construct_new_path(const std::string& root, const std::string& destination, 
            const std::string& cur)
    {
        std::string new_path(cur);
        if(cur.size() > root.size())
        {
            new_path.erase(new_path.begin(), (new_path.begin() + (cur.size() - (root.size() + 1))));
            if(*new_path.begin() == boost::filesystem::path("/").make_preferred().string()[0])
            {
                new_path.erase(new_path.begin());
            }
            new_path = (destination + boost::filesystem::path("/").make_preferred().string() +
                    new_path);
        }
        else new_path = root;
        
        return new_path;
    }
    
    inline fsys::result_data_boolean is_empty(const std::string& s)
    {
        using fsys::is_folder;
        using fsys::is_symlink;
        
        fsys::result_data_boolean res;
        boost::system::error_code ec;
        
        if(!is_folder(s).value)
        {
            res.value = false;
            res.error = ("fsys::result_data_boolean is_empty(const std::string&) at line: " + 
                            std::string(itoa((__LINE__))) + ": \"" + s + "\" is not a folder!");
        }
        else if(is_folder(s).value && !is_symlink(s).value)
        {
            res.value = boost::filesystem::is_empty(boost::filesystem::path(s), ec);
            if(is_error(ec))
            {
                res.value = false;
                res.error = ec.message();
            }
        }
        else
        {
            res.value = false;
            res.error = "fsys::result_data_boolean is_empty(const std::string&): \
unknown error occured!";
        }
        return res;
    }
    
    inline bool file_is_type(const std::string& s, const boost::filesystem::file_type& t)
    {
        using boost::filesystem::status;
        using boost::filesystem::path;
        using boost::filesystem::exists;
        
        boost::system::error_code ec;
        bool b(false);
        
        if(exists(s, ec))
        {
            try
            {
                b = (status(path(s), ec).type() == t);
                if(is_error(ec))
                {
                    ethrow(ec.message());
                }
            }
            catch(const std::exception& e)
            {
                ethrow(e.what());
            }
        }
        return b;
    }
    
    
}


/* anonymous test functions: */
namespace
{
    typedef struct copy_test_data copy_test_data;
    
    bool test_copy_directories(const std::string&);
    std::string parent_of_test_folder();
    std::string random_child(const std::string&);
    unsigned long count_contents_of_folder(const std::string&);
    copy_test_data const_copy_test_data();
    std::string random_child_folder(const std::string&);
    
    
    //copy test data.
    typedef struct copy_test_data
    {
        explicit copy_test_data();
        explicit copy_test_data(const std::string&, const std::string&);
        
        copy_test_data& operator=(const copy_test_data&);
        bool operator==(const copy_test_data&) const;
        bool operator!=(const copy_test_data&) const;
        
        std::string source, dest;
    } copy_test_data;
    
    copy_test_data::copy_test_data(){}
    
    copy_test_data::copy_test_data(const std::string& s1,
                    const std::string& s2)
    {
        this->source = s1;
        this->dest = s2;
    }
    
    copy_test_data& copy_test_data::operator=(const copy_test_data& d)
    {
        if(this != &d)
        {
            this->source = d.source;
            this->dest = d.dest;
        }
        return *this;
    }
    
    bool copy_test_data::operator==(const copy_test_data& d) const
    {
        return ((this->source == d.source) && 
                    (this->dest == d.dest));
    }
    
    bool copy_test_data::operator!=(const copy_test_data& d) const
    {
        return ((this->source != d.source) && 
                    (this->dest != d.dest));
    }
    
    
    inline copy_test_data const_copy_test_data()
    {
        return copy_test_data(
            "/mnt/ENCRYPTED/C++_Dev/Current_Projects/filesystem_namespace/Unit_Testing/build/test_folder",
            "/mnt/ENCRYPTED/C++_Dev/Current_Projects/filesystem_namespace/Unit_Testing/build/test_folder/DESTINATION OF COPY TEST");
    }
    
    inline std::string parent_of_test_folder()
    {
        return "/mnt/ENCRYPTED/C++_Dev/Current_Projects/filesystem_namespace/Unit_Testing/build";
    }
    
    inline bool test_copy_directories(const std::string& subfolder)
    {
        test_fixture_class reset;
        copy_test_data data(const_copy_test_data());
        std::string new_path(data.dest + fsys::pref_slash() + 
                        boost::filesystem::path(data.source).filename().string() + 
                        split_subdir(data.source, subfolder).second);
        if(fsys::create_folder(data.dest).value)
        {
            return (copy_directories(data.source, data.dest, subfolder) && 
                            fsys::is_folder(new_path).value && 
                            !fsys::is_symlink(new_path).value);
        }
        ethrow("test: could not create the test folder: \"" + data.dest + "\"");
    }
    
    inline unsigned long count_contents_of_folder(const std::string& folder)
    {
        unsigned long count(0);
        for(fsys::tree_riterator_class it(folder); !it.at_end(); ++it) count++;
        return count;
    }
    
    inline std::string random_child(const std::string& folder)
    {
        unsigned long child(rand() % count_contents_of_folder(folder)), iteration(0);
        for(fsys::tree_riterator_class it(folder); !it.at_end(); ++it, ++iteration)
        {
            if(iteration == child) return it.value();
        }
        return "";
    }
    
    inline std::string random_child_folder(const std::string& folder)
    {
        unsigned int element, count(0);
        
        for(fsys::tree_riterator_class it(folder); !it.at_end(); ++it)
        {
            if(fsys::is_folder(it.value()).value && !fsys::is_symlink(it.value()).value) ++count;
        }
        element = rdata::random_number(0, count);
        count = 0;
        for(fsys::tree_riterator_class it(folder); !it.at_end(); ++it)
        {
            if(fsys::is_folder(it.value()).value && !fsys::is_symlink(it.value()).value)
            {
                ++count;
                if(count == element) return it.value();
            }
        }
        return folder;
    }
    
    
}




TEST_FIXTURE(test_fixture_class, is_error_test)
{
    using boost::filesystem::path;
    using boost::system::error_code;
    
    path p("test_folder");
    error_code ec;
    
    boost::filesystem::remove(p, ec);
    CHECK(is_error(ec));
    CHECK(ec != boost::system::errc::success);
}

TEST_FIXTURE(test_fixture_class, init_directory_iter_test_case)
{
    boost::filesystem::directory_iterator it(init_directory_iter("test_folder"));
    CHECK(boost::filesystem::directory_iterator("test_folder")->path() == it->path());
}

TEST_FIXTURE(test_fixture_class, init_recursive_directory_iterator_test_case)
{
    boost::filesystem::recursive_directory_iterator it(init_directory_rec_iterator("test_folder"));
    CHECK(boost::filesystem::recursive_directory_iterator("test_folder")->path() == it->path());
}

TEST_FIXTURE(test_fixture_class, recurse_folder_copy_test_case)
{
    std::string test_folder(boost::filesystem::current_path().string() + "/test_folder"), 
                    dest_folder(test_folder + "/DESTINATION OF COPY TEST");
    std::vector<std::string> subfolders;
    subfolders.push_back("empty folder");
    subfolders.push_back("folder of files");
    subfolders.push_back("folder of folders");
    subfolders.push_back("folder of symlinks");
    subfolders.push_back("mixed folder");
    
    fsys::create_folder(dest_folder);
    CHECK(boost::filesystem::exists(boost::filesystem::path(dest_folder)));
    if(!boost::filesystem::exists(boost::filesystem::path(dest_folder))) ethrow("Could not create the destination folder for the copy test!");
    
    for(std::vector<std::string>::const_iterator it = subfolders.begin(); it != subfolders.end(); ++it)
    {
        {
            fsys::result_data_boolean tempres(recurs_folder_copy((test_folder + "/" + *it), dest_folder));
            if(!tempres.value)
            {
                ethrow(("Error returned: " + tempres.error));
            }
        }
        CHECK(boost::filesystem::exists(boost::filesystem::path((dest_folder + "/" + *it))));
    }
}

TEST(dive_test_case)
{
    {
        using namespace std;
        
        cout<< endl<< endl<< string(70, '-')<< endl<< "Testing Dive: "<< endl<< endl;
        for(unsigned int x = 0; x < 20; x++) cout<< "level "<< x<< ":  "<< dive(x, const_copy_test_data().source)<< endl;
        cout<< endl<< endl<< "Test completed!"<< endl<< string(70, '-')<< endl;
    }
}

TEST_FIXTURE(test_fixture_class, split_subdir_test)
{
    copy_test_data data(const_copy_test_data());
    std::string test_child(random_child(data.source));
    std::pair<std::string, std::string> tempp(split_subdir(data.source, test_child));
    {
        using namespace std;
        
        cout<< endl<< endl<< std::string(70, '-')<< endl;
        cout<< "split_subdir test case: "<< endl<< endl;
        cout<< "root =       \""<< data.source<< endl;
        cout<< "test_child = \""<< test_child<< endl;
        cout<< endl;
        cout<< "result.first:  \""<< tempp.first<< "\""<< endl;
        cout<< "result.second: \""<< tempp.second<< "\""<< endl;
        cout<< endl<< string(70, '-')<< endl;
    }
    CHECK(tempp.first == data.source);
    CHECK((tempp.first + tempp.second) == test_child);
    //ethrow("split_subdir test:\nroot: \"" + tempp.first + "\"\nsub: \"" + tempp.second + "\"");
}

TEST_FIXTURE(test_fixture_class, copy_directories_test_case)
{
    copy_test_data data(const_copy_test_data());
    bool tempb(false);
    std::string temps;
    
    tempb = test_copy_directories(data.source + "/nothing/whatever");
    CHECK(!tempb);
    tempb = true;
    for(unsigned int x = 0; ((x < 500) && tempb); x++)
    {
        {
            test_fixture_class fixture;
        }
        tempb = test_copy_directories(random_child_folder(data.source));
        if(!tempb) std::cout<< "temps = \""<< temps<< "\""<< std::endl;
        CHECK(tempb);
    }
    tempb = test_copy_directories(data.source);
    CHECK(tempb);
    //the next test is supposed to fail with an exception for the programmer:
    /*tempb = test_copy_directories(parent_path(data.source));
    CHECK(!tempb);*/
}

TEST_FIXTURE(test_fixture_class, parent_path_test_case)
{
    /*this test may have to be modified for windows users.  In that case, the 
     * expected result of operating on the "root" (aka: "C:\") would be an empty string. */
    copy_test_data data(const_copy_test_data());
    std::string parent(parent_path(data.source));
    CHECK(parent == parent_of_test_folder());
    parent = parent_path("/");
    CHECK(parent.empty());
    parent = data.source;
    for(unsigned int x = 0; x < 50; x++) parent = parent_path(parent);
    CHECK(parent.empty());
}

TEST_FIXTURE(test_fixture_class, is_child_test_case)
{
    copy_test_data data(const_copy_test_data());
    for(unsigned int x = 0; x < 500; x++)
    {
        CHECK(is_child(random_child(data.source), data.source));
        CHECK(!is_child(data.source, random_child(data.source)));
    }
}

//test construct_new_path

#endif