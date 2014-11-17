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


/** Object members: */
namespace fsys
{
    tree_iterator_class::tree_iterator_class(const std::string& s) : p(s), 
            it(init_directory_iter(p)), end(), 
            is_good_path((is_folder(p.string()).value && !(is_symlink(p.string()).value)))
    {
        //todo add check for root_path here
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
        //todo add check for root path here
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
                success(), dest(to)
    {
        try
        {
            if(!fsys::is_folder(from).value || !fsys::is_folder(to).value || 
                    (fsys::is_folder(from).value && fsys::is_symlink(from).value) || 
                    (fsys::is_folder(to).value && fsys::is_symlink(to).value) ||
                    is_child(to, from))
            {
                ethrow("Error: copy_iterator_class::copy_iterator_class(const std::string&) -> can not construct with \
invalid path!  Args can only be a folder.");
            }
        }
        catch(const std::exception& e)
        {
            ethrow(e.what());
        }
        if(boost::filesystem::is_empty(this->p)) this->success = fsys::fcopy(from, to);
    }
    
    copy_iterator_class::~copy_iterator_class()
    {
    }
    
    /* Copies the path that the iterator currently points to, and then it increments
     the iterator. */
    tree_riterator_class copy_iterator_class::operator++()
    {
        this->success.value = false;
        this->success.error.erase();
        this->success.path.erase();
        switch(this->it != this->end)
        {
            case true:
            {
                this->success.path = this->value();
                if(is_folder(this->value()).value && 
                        !is_symlink(this->value()).value)
                {
                    try
                    {
                      this->success.value = copy_directories(this->p.string(), this->dest, this->value());
                    }
                    catch(const std::exception& e)
                    {
                        ethrow(e.what());
                    }
                    if(!this->success.value)
                    {
                        this->success.error= "Failed copy directories";
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
                                std::string temps(this->dest + split_subdir(parent_path(this->p.string()), this->value()).second);
                                this->success.value = true;
                                
                                //only copy the parent path if it doesn't exist
                                if(!is_folder(temps).value)
                                {
                                  this->success.value = copy_directories(this->p.string(), 
                                                    this->dest, parent_path(this->value()));
                                }
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
                                this->success.value = fsys::is_folder(temps).value;
                            }
                            break;
                            
                            default:
                            {
                            }
                            break;
                        }
                        
                        //did we succeed in copying the parent path of the file?
                        switch(this->success.value)
                        {
                            case true:
                            {
                                std::string temps;
                                if(is_child(this->value(), this->p.string()))
                                {
                                    temps = (this->dest + split_subdir(parent_path(this->p.string()), this->value()).second);
                                    this->success = fcopy(this->value(), parent_path(temps));
                                }
                                else
                                {
                                    this->success.value = false;
                                    this->success.error = ("Error, could not copy path!  \"" + this->value() + 
                                                    "\" is not a child path of \"" + 
                                                    this->p.string() + "\"");
                                }
                            }
                            break;

                            case false:
                            {
                                this->success.error = ("Failed to create parent directories at destination!");
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
              this->success.error = "Iterator fault: it = end; can not iterate!";
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
            this->success = iter.success;
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
                tree_riterator_class(s), success()
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
            this->success = fsys::fdelete(this->p.string());
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
            this->success = d.success;
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
        
        this->success.value = false;
        this->success.error.erase();
        
        if(!this->at_end())
        {
            this->success.path = this->value();
        }
        
        //check if we can delete the top directory:
        if(can_delete(this->p.string()))
        {
            this->success.value = (std::remove(this->p.string().c_str()) == 0);
            if(!this->success.value)
            {
                this->success.error = ("tree_riterator_class delete_iterator_class::\
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
                        this->success.value = (std::remove(this->p.string().c_str()) == 0);
                        if(!this->success.value)
                        {
                            this->success.error = "tree_riterator_class delete_iterator_class::operator++(): \
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
                                this->success.value = (std::remove(this->it->path().string().c_str()) == 0);
                                if(!this->success.value)
                                {
                                    this->success.error = ("tree_riterator_class delete_iterator_class::operator++() \
line " + std::string(itoa(__LINE__)) + ": std::remove() failed!");
                                }
                            }
                            else
                            {
                                this->success.value = false;
                                this->success.error = ("tree_riterator_class delete_iterator_class::operator++() \
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
                this->success.value = true;
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
                    this->success.value = (std::remove(this->p.string().c_str()) == 0);
                    if(!this->success.value)
                    {
                        this->success.error = ("tree_riterator_class delete_iterator\
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
        
        res.path = from;
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
        
        res.path = from;
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
        
        res.path = s;
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
        
        res.path = from;
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
        
        res.path = from;
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