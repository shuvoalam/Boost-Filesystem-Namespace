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


#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <exception>
#include <fstream>
#include <iterator>
#include <vector>
#include <sstream>
#include <iostream>

#include "filesystem.hpp"


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
+ "\" @ Line " + itoa(__LINE__) + ": " + std::string(MSG)))

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
        return (err == boost::system::errc::success);
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


/** Object members: */
namespace fsys
{
    tree_iterator_class::tree_iterator_class(const std::string& s) : p(s), 
            it(init_directory_iter(p)), end(), 
            is_good_path((is_folder(p.string()).value && !(is_symlink(p.string()).value)))
    {
        if(s.find(boost::filesystem::root_path().string().c_str()) != 0)
        {
            ethrow("[PROGRAMMING ERROR]: can not pass non-absolute path to iterator!");
        }
        if(this->is_good_path && !this->at_end())
        {
            if(this->it->path().string() == s) ++(*this);
        }
    }
    
    tree_iterator_class::~tree_iterator_class()
    {
    }
    
    tree_iterator_class& tree_iterator_class::operator=(const tree_iterator_class& t)
    {
        if(this != &t)
        {
            this->end = boost::filesystem::directory_iterator();
            this->p = t.p;
            this->is_good_path = t.is_good_path;
            
            try
            {
                this->it = init_directory_iter(this->p);
                if(!t.at_end())
                {
                    while((this->it != this->end) && (this->it->path() != t.it->path())) ++(*this);
                }
            }
            catch(const std::exception& e)
            {
                ethrow(e.what());
            }
        }
        return *this;
    }
    
    tree_iterator_class tree_iterator_class::operator++()
    {
        if(this->is_good_path)
        {
            if(this->it != this->end)
            {
                try
                {
                    this->it++;
                }
                catch(const std::exception& e)
                {
                    ethrow(e.what());
                }
            }
        }
        return *this;
    }
    
    /* Returns the distance the iterator is from the end */
    unsigned int tree_iterator_class::count_from_end() const
    {
        tree_iterator_class tempit;
        unsigned int count(0);
        try
        {
            if(this->it != this->end)
            {
                tempit = *this;
                while(!tempit.at_end())
                {
                    ++tempit;
                    count++;
                }
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return count;
    }
    
    /* Returns the distance the iterator is from the beginning. */
    unsigned int tree_iterator_class::count_from_beg() const
    {
        unsigned int count(0);
        tree_iterator_class tempit;
        try
        {
            if(this->it != this->end)
            {
                if(this->it->path() != this->p)
                {
                    tempit = tree_iterator_class(this->p.string());
                    while((tempit.it->path() != this->it->path()) && !tempit.at_end())
                    {
                        ++tempit;
                        count++;
                    }
                }
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return count;
    }
    
    /* Returns the path that the iterator currently points to. */
    std::string tree_iterator_class::value() const
    {
        std::string temps;
        if(this->is_good_path)
        {
            if(this->it != this->end)
            {
                temps = this->it->path().string();
            }
        }
        return temps;
    }
    
    /* Determines whether the iterator is currently pointing to the end. */
    bool tree_iterator_class::at_end() const
    {
        return (this->it == this->end);
    }
    
    /**=========================================================================
     * =========================================================================
     */
    
    tree_riterator_class::tree_riterator_class(const std::string& s) : p(s), 
            it(init_directory_rec_iterator(p)), 
            end(), 
            is_good_path((is_folder(p.string()).value && !is_symlink(p.string()).value))
    {
        if(s.find(boost::filesystem::root_path().string().c_str()) != 0)
        {
            ethrow("[PROGRAMMING ERROR]: can not pass non-absolute path to iterator!");
        }
        if(this->is_good_path)
        {
            if((this->it != this->end) && (this->it->path().string() == s))
            {
                tree_riterator_class::operator++();
            }
        }
    }
    
    tree_riterator_class::~tree_riterator_class()
    {
    }
    
    tree_riterator_class& tree_riterator_class::operator=(const tree_riterator_class& t)
    {
        if(this != &t)
        {
            this->p = t.p.string();
            this->it = init_directory_rec_iterator(this->p);
            this->end = boost::filesystem::recursive_directory_iterator();
            this->is_good_path = t.is_good_path;
            
            if((this->it != this->end) && (this->it->path() == this->p))
            {
                tree_riterator_class::operator++();
            }
            
            if(!t.at_end() && (t.it->path() != t.p))
            {
                while((this->it != this->end) && (this->it->path() != t.it->path()))
                {
                    tree_riterator_class::operator++();
                }
            }
        }
        return *this;
    }
    
    tree_riterator_class tree_riterator_class::operator++()
    {
        if(this->is_good_path)
        {
            if(this->it != this->end)
            {
                if(is_folder(this->it->path().string()).value && is_symlink(this->it->path().string()).value)
                {
                    this->it.no_push();
                }
                try
                {
                    this->it++;
                }
                catch(...)
                {
                    this->it.no_push();
                    try
                    {
                        this->it++;
                    }
                    catch(const std::exception& e)
                    {
                        ethrow(e.what());
                    }
                }
            }
        }
        return *this;
    }
    
    unsigned int tree_riterator_class::count_from_end()
    {
        unsigned int count(0);
        if(this->it != this->end)
        {
            try
            {
                tree_riterator_class tempit;
                tempit = *this;
                while(!tempit.at_end())
                {
                    ++tempit;
                    count++;
                }
            }
            catch(const std::exception& e)
            {
                ethrow(e.what());
            }
        }
        return count;
    }
    
    unsigned int tree_riterator_class::count_from_beg()
    {
        unsigned int tempi(0);
        if(this->it != this->end)
        {
            try
            {
                tree_riterator_class tempit(this->p.string());
                while((tempit.it != tempit.end) && (tempit.it->path().string() != this->it->path().string()))
                {
                    ++tempit;
                    tempi++;
                }
            }
            catch(const std::exception& e)
            {
                ethrow(e.what());
            }
        }
        return tempi;
    }
    
    std::string tree_riterator_class::value() const
    {
        std::string temps;
        if(this->is_good_path && (this->it != this->end))
        {
            try
            {
                temps = this->it->path().string();
            }
            catch(const std::exception& e)
            {
                ethrow(e.what());
            }
        }
        return temps;
    }
    
    bool tree_riterator_class::at_end() const
    {
        return (this->it == boost::filesystem::recursive_directory_iterator());
    }
    
    /**
     * =========================================================================
     * =========================================================================
     * =========================================================================
     */
    
    copy_iterator_class::copy_iterator_class(const std::string& from, const std::string& to) : 
                tree_riterator_class::tree_riterator_class(from), 
                err(), dest(to)
    {
        try
        {
            if(!fsys::is_folder(from).value || !fsys::is_folder(to).value || 
                    (fsys::is_folder(from).value && fsys::is_symlink(from).value) || 
                    (fsys::is_folder(to).value && fsys::is_symlink(to).value))
            {
                ethrow("Error: copy_iterator_class::copy_iterator_class(const std::string&) -> can not construct with \
invalid path!  Args can only be a folder.");
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        if(boost::filesystem::is_empty(this->p)) this->err = fsys::fcopy(from, to);
    }
    
    copy_iterator_class::~copy_iterator_class()
    {
    }
    
    /* Copies the path that the iterator currently points to, and then it increments
     the iterator. */
    tree_riterator_class copy_iterator_class::operator++()
    {
        this->err.value = false;
        this->err.error.erase();
        switch(this->it != this->end)
        {
            case true:
            {
                if(is_folder(this->value()).value && 
                        !is_symlink(this->value()).value)
                {
                    try
                    {
                        this->err.value = copy_directories(this->p.string(), this->dest, this->value());
                    }
                    catch(const std::exception& e)
                    {
                        ethrow(e.what());
                    }
                    if(!this->err.value)
                    {
                        this->err.error = "Failed copy directories";
                    }
                }
                else if(is_symlink(this->value()).value || is_file(this->value()).value)
                {
                    try
                    {
                        //copy the parent path of the file if it isn't there:
                        switch(parent_path(this->value()) == this->p.string())
                        {
                            case false:
                            {
                                std::string temps(construct_new_path(this->p.string(), 
                                        this->dest, parent_path(this->value())));
                                
                                //only copy the parent path if it doesn't exist
                                if(!is_folder(temps).value)
                                {
                                    this->err.value = copy_directories(this->p.string(), 
                                                    this->dest, parent_path(this->value()));
                                }
                                else this->err.value = true;
                            }
                            break;
                            
                            case true:
                            {
                                std::string temps(this->dest + 
                                        boost::filesystem::path("/").make_preferred().string() + 
                                        this->p.filename().string());
                                if(!fsys::is_folder(temps).value)
                                {
                                    boost::filesystem::copy_directory(this->p, boost::filesystem::path(temps));
                                }
                                this->err.value = fsys::is_folder(temps).value;
                            }
                            break;
                            
                            default:
                            {
                            }
                            break;
                        }
                        
                        //did we succeed in copying the parent path of the file?
                        switch(this->err.value)
                        {
                            case true:
                            {
                                std::string temps(this->value());
                                if(temps.size() > this->p.string().size())
                                {
                                    if(parent_path(this->p.string()).size() < 2)
                                    {
                                        temps.erase(temps.begin(), (temps.begin() + parent_path(this->p.string()).size()));
                                    }
                                    else temps.erase(temps.begin(), (temps.begin() + (parent_path(this->p.string()).size() + 1)));
                                    temps = (this->dest + boost::filesystem::path("/").make_preferred().string() +
                                            temps);
                                }
                                this->err = fcopy(this->value(), parent_path(temps));
                            }
                            break;

                            case false:
                            {
                                this->err.error = ("Failed to create parent directories at destination!");
                            }
                            break;

                            default:
                            {
                            }
                            break;
                        }
                    }
                    catch(const std::exception& e)
                    {
                        ethrow(e.what());
                    }
                }
            }
            break;
            
            case false:
            {
                this->err.error = "Iterator fault: it = end; can not iterate!";
            }
            break;
            
            default:
            {
            }
            break;
        }
        
        try
        {
            if(!this->at_end())
            {
                tree_riterator_class::operator++();
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return *this;
    }
    
    tree_riterator_class& copy_iterator_class::operator=(const copy_iterator_class& iter)
    {
        if(this != &iter)
        {
            tree_riterator_class::operator=(iter);
            this->err = iter.err;
            this->dest = iter.dest;
        }
        return *this;
    }
    
    /* increments the iterator without copying the path that the iterator 
     * points to. */
    void copy_iterator_class::skip()
    {
        if(this->it != this->end)
        {
            tree_riterator_class::operator++();
        }
    }
                    
    /**=========================================================================
     * =========================================================================
     */
    
    delete_iterator_class::delete_iterator_class(const std::string& s) : 
                tree_riterator_class(s), err()
    {
        boost::system::error_code ec;
        try
        {
            if(!fsys::is_folder(s).value || (fsys::is_folder(s).value && fsys::is_symlink(s).value))
            {
                throw "delete_iterator_class::delete_iterator_class(const std::string&) -> \
can not construct object with invalid pathname!  Path must be a folder!";
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        if(boost::filesystem::is_empty(this->p, ec))
        {
            this->err = fsys::fdelete(this->p.string());
        }
    }
    
    delete_iterator_class::~delete_iterator_class()
    {
    }
    
    tree_riterator_class& delete_iterator_class::operator=(const delete_iterator_class& d)
    {
        if(this != &d)
        {
            tree_riterator_class::operator=(d);
            this->err = d.err;
        }
        return *this;
    }
    
    /* Deletes the path that the iterator currently points to, and then reconstructs
     * the iterator.  Due to the nature of the operation, we can't delete the
     * path the iterator is pointing to and reliably continue iteration
     * with garuntee of the iterator's validity.  Each iteration
     * consists of constructing a new iterator, and iterating to the next path
     * that can be deleted as determined by ::can_delete(const std::string&), 
     * and deleting that path.  After that is done, the iterator is 
     * reconstructed to maintain validity. */
    tree_riterator_class delete_iterator_class::operator++()
    {
        boost::system::error_code ec;
        
        this->err.value = false;
        this->err.error.erase();
        
        //check if we can delete the top directory:
        if(can_delete(this->p.string()))
        {
            this->err.value = (std::remove(this->p.string().c_str()) == 0);
            if(!this->err.value)
            {
                this->err.error = ("tree_riterator_class delete_iterator_class::\
operator++() line " + std::string(itoa((__LINE__))) + ": could not delete \"" + 
                                this->p.string() + "\"!");
            }
            return *this;
        }
        
        try
        {
            if(is_folder(this->p.string()).value && !is_symlink(this->p.string()).value)
            {
                try
                {
                    if(is_empty(this->p.string()).value)
                    {
                        this->err.value = (std::remove(this->p.string().c_str()) == 0);
                        if(!this->err.value)
                        {
                            this->err.error = "tree_riterator_class delete_iterator_class::operator++(): \
error: couldn't delete the folder!";
                        }
                    }
                    else
                    {
                        this->it = init_directory_rec_iterator(this->p);
                        while(this->it != this->end)
                        {
                            if(can_delete(this->it->path().string())) break;
                            tree_riterator_class::operator++();
                        }
                        if(this->it != this->end)
                        {
                            if(can_delete(this->it->path().string()))
                            {
                                this->err.value = (std::remove(this->it->path().string().c_str()) == 0);
                                if(!this->err.value)
                                {
                                    this->err.error = ("tree_riterator_class delete_iterator_class::operator++() \
line " + std::string(itoa(__LINE__)) + ": std::remove() failed!");
                                }
                            }
                            else
                            {
                                this->err.value = false;
                                this->err.error = ("tree_riterator_class delete_iterator_class::operator++() \
unknown error: can_delete returned false for un-completed delete_iterator!");
                            }
                        }
                    }
                }
                catch(const std::exception& e)
                {
                    ethrow(e.what());
                }
            }
            else
            {
                this->err.value = true;
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        
        try
        {
            /* The iterator should still be valid after this operation, but
             * the only way this can be garunteed is if we completely reconstruct it: */
            this->it = init_directory_rec_iterator(this->p);
            if(this->it == this->end)
            {
                if(can_delete(this->p.string()))
                {
                    this->err.value = (std::remove(this->p.string().c_str()) == 0);
                    if(!this->err.value)
                    {
                        this->err.error = ("tree_riterator_class delete_iterator\
_class::operator++()  line " + std::string(itoa((__LINE__))) + ": could not delete \"" + 
                                    this->p.string() + "\"!");
                    }
                }
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        
        return *this;
    }
    
    /* Increments the iterator without deleting the path that the iterator
     currently points to. */
    void delete_iterator_class::skip()
    {
        if(this->it != this->end)
        {
            tree_riterator_class::operator++();
        }
    }
    
    
}

/** Functions: */
namespace fsys
{
    char pref_slash()
    {
        return boost::filesystem::path("/").make_preferred().string().at(0);
    }
    
    /* Copies path "from", into "to".  To must be a folder.  Will attempt
     any kind of path. */
    result_data_boolean fcopy(const std::string& from, const std::string& to)
    {
        result_data_boolean res;
        boost::system::error_code err;
        try
        {
            switch(fsys::is_folder(to).value && !fsys::is_symlink(to).value)
            {
                case true:
                {
                    if(fsys::is_file(from).value && !fsys::is_symlink(from).value)
                    {
                        try
                        {
                            boost::filesystem::copy(boost::filesystem::path(from), 
                                    boost::filesystem::path(to + 
                                            boost::filesystem::path("/").make_preferred().string() + 
                                            boost::filesystem::path(from).filename().string()), 
                                    err);
                            res.value = true;
                        }
                        catch(const std::exception& e)
                        {
                            ethrow(e.what());
                        }
                        if(is_error(err))
                        {
                            res.error = err.message();
                            res.value = false;
                        }
                    }
                    else if(fsys::is_folder(from).value && !is_symlink(from).value)
                    {
                        try
                        {
                            res = recurs_folder_copy(from, to);
                        }
                        catch(const std::exception& e)
                        {
                            ethrow(e.what());
                        }
                    }
                    else if(fsys::is_symlink(from).value)
                    {
                        try
                        {
                            boost::filesystem::copy_symlink(boost::filesystem::path(from),
                                    boost::filesystem::path(to + 
                                    boost::filesystem::path("/").make_preferred().string() + 
                                    boost::filesystem::path(from).filename().string()),
                                    err);
                            res.value = true;
                        }
                        catch(const std::exception& e)
                        {
                            ethrow(e.what());
                        }
                        if(is_error(err))
                        {
                            res.value = false;
                            res.error = err.message();
                        }
                    }
                    else
                    {
                        res.error = ("Error: \"" + from + "\" is not a valid path!");
                        res.value = false;
                    }
                }
                break;

                case false:
                {
                    res.value = false;
                    res.error = ("\"" + to + "\" is NOT a folder!!");
                }
                break;

                default:
                {
                }
                break;
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return res;
    }
    
    /* Will create a new folder, and return the result. */
    result_data_boolean create_folder(const std::string& s)
    {
        result_data_boolean res;
        boost::system::error_code ec;
        if(!is_folder(s).value)
        {
            try
            {
                boost::filesystem::create_directories(boost::filesystem::path(s),
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
                ethrow(e.what());
            }
        }
        if(is_folder(s).value && !is_symlink(s).value)
        {
            res.value = true;
        }
        else
        {
            res.value = false;
            if(res.error.empty())
            {
                res.error = "Folder wasn't created.  No erorr specified!";
            }
        }
        return res;
    }
    
    /* rename uses full paths */
    result_data_boolean frename(const std::string& from, const std::string& to)
    {
        result_data_boolean res;
        boost::system::error_code ec;
        try
        {
            switch(is_folder(to).value || is_symlink(to).value || is_file(to).value)
            {
                case true:
                {
                    res.value = false;
                    res.error = ("result_data_boolean frename(const std::string\
& from, const std::string& to) :::  ERROR> \"" + to + "\"  REASON: path exists!");
                }
                break;
                
                case false:
                {
                    res.value = true;
                    boost::filesystem::rename(boost::filesystem::path(from), 
                            boost::filesystem::path(to), ec);
                    if(is_error(ec))
                    {
                        res.value = false;
                        res.error = ec.message();
                    }
                    else
                    {
                        res.value = (is_folder(to).value || is_symlink(to).value || is_file(to).value);
                    }
                }
                break;
                
                default:
                {
                }
                break;
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return res;
    }
    
    /* Will delete path "s".  Can only delete empty folders, but it can
       delete any file. */
    result_data_boolean fdelete(const std::string& s)
    {
        result_data_boolean res;
        boost::system::error_code ec;
        try
        {
            if(is_folder(s).value && !is_symlink(s).value)
            {
                if(boost::filesystem::is_empty(s, ec))
                {
                    res.value = (std::remove(s.c_str()) == 0);
                    if(!res.value)
                    {
                        res.error = "Unknown error...  remove returned fail";
                    }
                }
            }
            if(is_file(s).value || is_symlink(s).value)
            {
                res.value = (std::remove(s.c_str()) == 0);
                if(is_file(s).value || is_symlink(s).value)
                {
                    res.value = false;
                    res.error = ("result_data_boolean fdelete(const std::string\
& s) ::: ERROR> \"" + s + "\"  file could not be deleted.");
                }
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return res;
    }
    
    /* Moves path "from" into path "to" */
    result_data_boolean fmove(const std::string& from, const std::string& to)
    {
        result_data_boolean res;
        switch(is_folder(to).value && !is_symlink(to).value)
        {
            case true:
            {
                try
                {
                    res = frename(from, (to + boost::filesystem::path("/").make_preferred().string() + 
                            boost::filesystem::path(from).filename().string()));
                }
                catch(const std::exception& e)
                {
                    ethrow(e.what());
                }
            }
            break;
            
            case false:
            {
                res.value = false;
                res.error = ("result_data_boolean fmove(const std::string& from\
, const std::string& to) :::  ERROR> \"" + to + "\" is not a folder.  Move ope\
ation failed!");
            }
            break;
            
            default:
            {
            }
            break;
        }
        return res;
    }
    
    result_data_boolean is_file(const std::string& s)
    {
        result_data_boolean result;
        bool (*isfile)(const boost::filesystem::path&, boost::system::error_code&) = boost::filesystem::is_regular_file;
        try
        {
            result = boost_bool_funct(isfile, s);
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return result;
    }
    
    result_data_boolean is_folder(const std::string& s)
    {
        result_data_boolean result;
        bool (*isfolder)(const boost::filesystem::path&, boost::system::error_code&) = boost::filesystem::is_directory;
        try
        {
            result = boost_bool_funct(isfolder, s);
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return result;
    }
    
    result_data_boolean is_symlink(const std::string& s)
    {
        result_data_boolean result;
        bool (*issym)(const boost::filesystem::path&, boost::system::error_code&) = boost::filesystem::is_symlink;
        try
        {
            result = boost_bool_funct(issym, s);
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return result;
    }
    
    /* Returns true if the path is an empty folder or a file.  Any path that
     * this returns true for should return success for std::remove(). */
    bool can_delete(const std::string& s)
    {
        using fsys::is_folder;
        using fsys::is_file;
        using fsys::is_symlink;
        
        bool tempb(false);
        
        try
        {
            tempb = (is_symlink(s).value || is_file(s).value);
            if(!tempb)
            {
                if(is_folder(s).value && !is_symlink(s).value)
                {
                    tempb = is_empty(s).value;
                }
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return tempb;
    }
    
    /* This operates exactly like boost::filesystem::copy_directory(), except
     * that it copies a subdirectory of the source, and any of it's parent paths
     * if they don't yet exist.
     * 
     * example:
     * result_data_boolean copy_folders(from, to, source_subfolder_inside_the_from_directory) */
    result_data_boolean copy_folders(const std::string& from, std::string& to, const std::string& source)
    {
        result_data_boolean res;
        try
        {
            res.value = copy_directories(from, to, source);
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        return res;
    }
    
    /* Returns the full path that the program initially operates under. */
    std::string current_path()
    {
        static std::string cpath;
        if((cpath.size() == 0) || (!is_file(cpath).value && !is_folder(cpath).value))
        {
            cpath = boost::filesystem::current_path().string();
        }
        return cpath;
    }
    
    
}