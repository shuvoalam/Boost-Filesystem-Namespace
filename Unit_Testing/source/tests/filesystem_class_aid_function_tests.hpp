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
#include <iostream>
#include <string>
#include <boost/filesystem.hpp>

#include "filesystem.hpp"


class test_fixture_class;

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
    std::string newpath(const std::string&, const std::string&, const std::string&);
    
    
#if FILESYSTEM_USE_RUNTIME_ERRORS == true
    #define ethrow(MSG) throw std::runtime_error(("EXCEPTION THROWN: \"" + std::string(__FILE__)\
+ "\"  @ Line " + itoa(__LINE__) + ": " + std::string(MSG)))

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
    
    /* Copies a set of sub-folders whether they exist or not. */
    inline bool copy_directories(const std::string& from, const std::string& to, const std::string& source)
    {
        boost::filesystem::path path_from(from), path_to(to);
        boost::system::error_code err;
        std::string newpath(source), temps, newfrom;
        int levels(0);
        bool temp_b(true);
        
        /* Create the new path in the 'to' folder. */
        {
            if(newpath.size() > from.size())
            {
                short tempi(
                        ((parent_path(from) == boost::filesystem::path("/").make_preferred().string()) ? 0 : 1));
                newpath.erase(newpath.begin(), (newpath.begin() + parent_path(from).size() + tempi));
            }
        }
        newpath = (to + boost::filesystem::path("/").make_preferred().string() + newpath);
        
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
        
        if(source != from)
        {
            while(!fsys::is_folder(newpath).value && temp_b)
            {
                temps = newpath;
                try
                {
                    /* Extract the next folder we can copy: */
                    {
                        boost::filesystem::path p(temps), prev_p(newpath);
                        boost::system::error_code err;
                        levels = 0;
                        if(temps.size() > to.size())
                        {
                            p = parent_path(p.string());
                            levels = 0;
                            try
                            {
                                while((p.string() != to) && !boost::filesystem::is_directory(p, err))
                                {
                                    prev_p = p;
                                    p = parent_path(p.string());
                                    levels++;
                                }
                            }
                            catch(const std::exception& e)
                            {
                                ethrow(e.what());
                            }
                            temps = prev_p.string();
                        }
                        temp_b = (!boost::filesystem::is_directory(prev_p, err) && (p.string() != to));
                    }
                }
                catch(const std::exception& e)
                {
                    ethrow(e.what());
                }
                newfrom = source;
                for(int x = 0; x < levels; x++) newfrom = parent_path(newfrom);
                try
                {
                    if(!fsys::is_folder(temps).value && fsys::is_folder(newfrom).value && 
                            !fsys::is_symlink(newfrom).value)
                    {
                        boost::filesystem::copy_directory(newfrom, temps, err);
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
            temp_b = (
                    !is_error(err) && 
                    boost::filesystem::is_directory(boost::filesystem::path(newpath), err) && 
                    !boost::filesystem::is_symlink(newpath));
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return temp_b;
    }
    
    inline bool is_error(const boost::system::error_code& err)
    {
        return !(!err);
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
    
    /** For use in copy_path, this function constructs the real target
     * path a folder or file should be copied to given the source, destination folder,
     * and the current iteration under the source folder. 
     * 
     * Example of the process:
     * 
     * "/a/folder/that/we/are/copying/file.txt" from "/a/folder" into "/a/destination"
     * first, we take all the subdirectories under the source ("/a/folder"): 
     * "folder/that/we/are/copying/file.txt"
     * and we append them to the end of the destination:
     * "/a/destination/folder/that/we/are/copying/file.txt"*/
    inline std::string newpath(const std::string& from, const std::string& to, const std::string& iteration)
    {
        std::string temps(iteration);
        if((temps.size() > from.size()) && !from.empty())
        {
            temps.erase(temps.begin(), (temps.begin() + (parent_path(from).size() + 1)));
            temps = (to + boost::filesystem::path("/").make_preferred().string() + temps);
        }
        return temps;
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
                                ethrow("Emtpy error message thrown here!");
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
                                                temps = newpath(from, to, it.value());
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
 algorithm can only copy a folder into a folder!");
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
    
    
}


class test_fixture_class
{
public:
    explicit test_fixture_class()
    {
        system("/mnt/ENCRYPTED/C++/Finished_Projects/filesystem_namespace/Unit_Testing/setup_test");
    }
    
    ~test_fixture_class(){}
    
    
private:
    
    
};

TEST_FIXTURE(test_fixture_class, is_error_test)
{
    using boost::filesystem::path;
    using boost::system::error_code;
    
    path p("test_folder");
    error_code ec;
    bool tempb(false);
    
    boost::filesystem::remove(p, ec);
    tempb = is_error(ec);
    CHECK(tempb);
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

//cur_pos recurse_folder_copy test

#endif