#include <iostream>
#include <string>
#include <boost/filesystem.hpp>
#include <exception>
#include <iterator>
#include <vector>

#include "filesystem.hpp"

namespace
{
    bool is_error(const boost::system::error_code&);
    fsys::result_data_boolean boost_funct(bool (*)(const boost::filesystem::path&,
            boost::system::error_code&), const std::string&);
    boost::filesystem::directory_iterator init_directory_iter(const boost::filesystem::path&);
    fsys::result_data_boolean recurs_folder_copy(const std::string&, const std::string&);
    bool copy_directories(const std::string&, const std::string&, const std::string&);
    boost::filesystem::recursive_directory_iterator init_directory_rec_iterator(const boost::filesystem::path&);
    std::string parent_path(const std::string&);
    bool is_child(const std::string&, const std::string&);
    
    
    
    inline std::string parent_path(const std::string& s)
    {
        std::string temps(s);
        bool slash_hit(false);
        if(s.size() > std::string(boost::filesystem::path("/").make_preferred().string()).size())
        {
            while((temps.size() > 1) && !slash_hit)
            {
                if(temps.back() == boost::filesystem::path("/").make_preferred().string()[0]) slash_hit = true;
                temps.erase((temps.begin() + (temps.size() - 1)));
            }
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
                newpath.erase(newpath.begin(), (newpath.begin() + (parent_path(from).size() + 1)));
            }
        }
        newpath = (to + boost::filesystem::path("/").make_preferred().string() + newpath);
        
        /* Extracts the first copy-able folder. Returns true/false based on
           whether the folder can be created. */
        auto get_folder = [&to, &newpath, &levels](std::string& s)->bool{
            boost::filesystem::path p(s), prev_p(newpath);
            boost::system::error_code err;
            levels = 0;
            if(s.size() > to.size())
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
                catch(...)
                {
                    throw;
                }
                s = prev_p.string();
            }
            return (!boost::filesystem::is_directory(prev_p, err) && (p.string() != to));
        };
        
        try
        {
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
                catch(...)
                {
                    throw;
                }
            }
        }
        catch(...)
        {
            throw;
        }
        
        if(source != from)
        {
            while(!fsys::is_folder(newpath).value && temp_b)
            {
                temps = newpath;
                try
                {
                    temp_b = get_folder(temps);
                }
                catch(...)
                {
                    throw;
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
                catch(...)
                {
                    throw;
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
        catch(...)
        {
            throw;
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
        catch(...)
        {
            throw;
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
            throw "ERROR:  Cannot construct iterator with invalid pathname";
        }
        boost::filesystem::directory_iterator it;
        try
        {
            it = boost::filesystem::directory_iterator(p);
        }
        catch(...)
        {
            throw;
        }
        return it;
    }
    
    inline boost::filesystem::recursive_directory_iterator init_directory_rec_iterator(const boost::filesystem::path& p)
    {
        if((!fsys::is_folder(p.string()) && !fsys::is_file(p.string())) || (fsys::is_folder(p.string()).value && 
                fsys::is_symlink(p.string()).value))
        {
            throw "ERROR:  Cannot construct iterator with invalid pathname";
        }
        boost::filesystem::recursive_directory_iterator it;
        try
        {
            it = boost::filesystem::recursive_directory_iterator(p);
        }
        catch(...)
        {
            throw;
        }
        return it;
    }
    
    inline fsys::result_data_boolean recurs_folder_copy(const std::string& from, const std::string& to)
    {
        fsys::result_data_boolean res;
        fsys::tree_riterator_class it;
        std::vector<std::string> failed_paths;
        std::string temps;
        
        auto newpath = [&from, &to](const std::string& s)->std::string{
            std::string temps(s);
            if((temps.size() > from.size()) && (from.size() > 0))
            {
                temps.erase(temps.begin(), (temps.begin() + (parent_path(from).size() + 1)));
                temps = (to + boost::filesystem::path("/").make_preferred().string() + temps);
            }
            return temps;
        };
        
        switch(fsys::is_folder(from).value && !fsys::is_symlink(from).value && 
                fsys::is_folder(to).value && !fsys::is_symlink(to).value && 
                !is_child(to, from) && !is_child(from, to))
        {
            case true:
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
                        if(fsys::is_file(it.value()).value || fsys::is_symlink(it.value()).value)
                        {
                            temps = parent_path(it.value());
                            (void)copy_directories(from, to, temps);
                            switch(fsys::is_folder(temps).value && !fsys::is_symlink(temps).value)
                            {
                                case true:
                                {
                                    try
                                    {
                                        temps = newpath(it.value());
                                        if(!fsys::is_file(temps).value && !fsys::is_symlink(temps).value)
                                        {
                                            boost::filesystem::copy(it.value(), temps);
                                        }
                                    }
                                    catch(...)
                                    {
                                        throw;
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
                        it++;
                    }
                }
                catch(...)
                {
                    res.value = false;
                    throw;
                }
            }
            break;
            
            case false:
            {
                res.value = false;
                res.error = ("Error: args not valid.  Recursive folder copy\
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
}


/** Object members: */
namespace fsys
{
    tree_iterator_class::tree_iterator_class(const std::string& s) : p(s), 
            it(init_directory_iter(p)), end(), 
            is_good_path((is_folder(p.string()).value && !(is_symlink(p.string()).value)))
    {
        if(this->is_good_path)
        {
            if(this->it->path().string() == s) (*this)++;
        }
    }
    
    tree_iterator_class::~tree_iterator_class()
    {
    }
    
    const tree_iterator_class& tree_iterator_class::operator=(const tree_iterator_class& t)
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
                    while((this->it != this->end) && (this->it->path() != t.it->path())) (*this)++;
                }
            }
            catch(...)
            {
                throw;
            }
        }
        return *this;
    }
    
    tree_iterator_class tree_iterator_class::operator++(int i)
    {
        if(this->is_good_path)
        {
            if(this->it != this->end)
            {
                try
                {
                    this->it++;
                }
                catch(...)
                {
                    throw;
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
                    tempit++;
                    count++;
                }
            }
        }
        catch(...)
        {
            throw;
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
                        tempit++;
                        count++;
                    }
                }
            }
        }
        catch(...)
        {
            throw;
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
        if(this->is_good_path)
        {
            if((this->it != this->end) && (this->it->path().string() == s))
            {
                (*this)++;
            }
        }
    }
    
    tree_riterator_class::~tree_riterator_class()
    {
    }
    
    const tree_riterator_class& tree_riterator_class::operator=(const tree_riterator_class& t)
    {
        if(this != &t)
        {
            this->p = t.p.string();
            this->it = init_directory_rec_iterator(this->p);
            this->end = boost::filesystem::recursive_directory_iterator();
            this->is_good_path = t.is_good_path;
            
            if((this->it != this->end) && (this->it->path() == this->p))
            {
                tree_riterator_class::operator++(1);
            }
            
            if(!t.at_end() && (t.it->path() != t.p))
            {
                while((this->it != this->end) && (this->it->path() != t.it->path()))
                {
                    tree_riterator_class::operator++(1);
                }
            }
        }
        return *this;
    }
    
    tree_riterator_class tree_riterator_class::operator++(int i)
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
                    catch(...)
                    {
                        throw;
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
                //while(!tempit.at_end() && (tempit.it->path() != this->it->path())) tempit++;
                while(!tempit.at_end())
                {
                    tempit++;
                    count++;
                }
            }
            catch(...)
            {
                throw;
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
                    tempit++;
                    tempi++;
                }
            }
            catch(...)
            {
                throw;
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
            catch(...)
            {
                throw;
            }
        }
        return temps;
    }
    
    bool tree_riterator_class::at_end() const
    {
        return (this->it == this->end);
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
                throw "Error: copy_iterator_class::copy_iterator_class(const std::string&) -> can not construct with \
invalid path!  Args can only be a folder.";
            }
        }
        catch(...)
        {
            throw;
        }
    }
    
    copy_iterator_class::~copy_iterator_class()
    {
    }
    
    tree_riterator_class copy_iterator_class::operator++(int x)
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
                    catch(...)
                    {
                        throw;
                    }
                    if(!this->err.value)
                    {
                        this->err.error = "Failed copy directories";
                    }
                }
                if(is_symlink(this->value()).value || is_file(this->value()).value)
                {
                    try
                    {
                        switch(parent_path(this->value()) == this->p.string())
                        {
                            case false:
                            {
                                this->err.value = copy_directories(this->p.string(), this->dest, parent_path(this->value()));
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
                        switch(this->err.value)
                        {
                            case true:
                            {
                                std::string temps(this->value());
                                if(temps.size() > this->p.string().size())
                                {
                                    temps.erase(temps.begin(), (temps.begin() + (parent_path(this->p.string()).size() + 1)));
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
                    catch(...)
                    {
                        throw;
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
                tree_riterator_class::operator++(x);
            }
        }
        catch(...)
        {
            throw;
        }
        return *this;
    }
    
    const tree_riterator_class& copy_iterator_class::operator=(const copy_iterator_class& iter)
    {
        if(this != &iter)
        {
            tree_riterator_class::operator=(iter);
            this->err = iter.err;
            this->dest = iter.dest;
        }
        return *this;
    }
                    
    /**=========================================================================
     * =========================================================================
     */
    
    delete_iterator_class::delete_iterator_class(const std::string& s) : 
                tree_riterator_class(s), err()
    {
        try
        {
            if(!fsys::is_folder(s).value || (fsys::is_folder(s).value && fsys::is_symlink(s).value))
            {
                throw "delete_iterator_class::delete_iterator_class(const std::string&) -> \
can not construct object with invalid pathname!  Path must be a folder!";
            }
        }
        catch(...)
        {
            throw;
        }
    }
    
    delete_iterator_class::~delete_iterator_class()
    {
    }
    
    const tree_riterator_class& delete_iterator_class::operator=(const delete_iterator_class& d)
    {
        if(this != &d)
        {
            tree_riterator_class::operator=(d);
            this->err = d.err;
        }
        return *this;
    }
    
    tree_riterator_class delete_iterator_class::operator++(int i)
    {
        this->err.value = false;
        this->err.error.erase();
        std::string temps(this->it->path().string());
        if(this->it != this->end)
        {
            /* Attempt to delete the path */
            try
            {
                if(is_folder(temps).value && !is_symlink(temps).value)
                {
                    boost::system::error_code ec;
                    this->err.value = true;
                    if(boost::filesystem::is_empty(this->it->path(), ec))
                    {
                        std::remove(temps.c_str());
                        if(is_folder(temps).value)
                        {
                            this->err.value = false;
                            this->err.error = ("\"" + temps + "\"\nREASON: Unknown; path\
registered as empty, but removal failed.");
                        }
                    }
                    if(is_error(ec))
                    {
                        this->err.value = false;
                        this->err.error = ec.message();
                    }
                }
                else if(is_file(temps).value || is_symlink(temps).value)
                {
                    
                    this->err.value = (std::remove(temps.c_str()) == 0);
                    if(is_file(temps).value || is_symlink(temps).value)
                    {
                        this->err.value = false;
                        this->err.error = ("\"" + temps + "\"\nREASON: attempted removal, \
but did not succeed.  Reason for failure is unknown; file persists.");
                    }
                }
                else
                {
                    this->err.value = false;
                    this->err.error = ("\"" + temps + "\"\nREASON: Unknown, ident\
ity of path could not be established.  No action could be taken.");
                }
            }
            catch(...)
            {
                throw;
            }
            
            /* Move on to the next path */
            try
            {
                tree_riterator_class::operator++(1);
                if(this->at_end())
                {
                    boost::system::error_code ec;
                    switch(!boost::filesystem::is_empty(this->p, ec))
                    {
                        case true:
                        {
                            this->it = init_directory_rec_iterator(this->p);
                            this->err.value = true;
                        }
                        break;
                        
                        case false:
                        {
                            boost::filesystem::remove_all(this->p, ec);
                            this->err.value = true;
                            if(is_error(ec))
                            {
                                this->err.value = false;
                                this->err.error = ec.message();
                            }
                        }
                        break;
                        
                        default:
                        {
                        }
                        break;
                    }
                }
            }
            catch(...)
            {
                throw;
            }
        }
        return *this;
    }
    
    
}

/** Functions: */
namespace fsys
{
    
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
                    if(fsys::is_file(from).value)
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
                        catch(...)
                        {
                            throw;
                        }
                        if(is_error(err))
                        {
                            res.error = err.message();
                            res.value = false;
                        }
                    }
                    else if(fsys::is_folder(from).value)
                    {
                        try
                        {
                            res = recurs_folder_copy(from, to);
                        }
                        catch(...)
                        {
                            throw;
                        }
                    }
                    else if(fsys::is_symlink(from).value)
                    {
                        try
                        {
                            boost::filesystem::copy_symlink(boost::filesystem::path(from),
                                    boost::filesystem::path(to), err);
                        }
                        catch(...)
                        {
                            throw;
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
        catch(...)
        {
            throw;
        }
        return res;
    }
    
    /* Will create a new folder, and return the result. */
    result_data_boolean create_folder(const std::string& s)
    {
        result_data_boolean res;
        boost::system::error_code ec;
        switch(is_folder(s).value && !is_symlink(s).value)
        {
            case true:
            {
                res.value = false;
                res.error = ("result_data_boolean create_folder(const std::string& s)\
 ::: ERROR> \"" + s + "\" already exists!  Could not create directory.");
            }
            break;
            
            case false:
            {
                res.value = true;
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
                catch(...)
                {
                    res.value = false;
                    throw;
                }
            }
            break;
            
            default:
            {
            }
            break;
        }
        return res;
    }
    
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
                }
                break;
                
                default:
                {
                }
                break;
            }
        }
        catch(...)
        {
            throw;
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
                if(is_folder(s).value)
                {
                    res.value = false;
                    res.error = ("result_data_boolean fdelete(const std::string&\
 s) ::: ERROR> \"" + s + "\"  path could not be deleted.");
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
        catch(...)
        {
            throw;
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
                catch(...)
                {
                    throw;
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
        catch(...)
        {
            throw;
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
        catch(...)
        {
            throw;
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
        catch(...)
        {
            throw;
        }
        return result;
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
        catch(...)
        {
            throw;
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