#ifndef FILESYSTEM_HPP_INCLUDED
#define FILESYSTEM_HPP_INCLUDED
#include <string>
#include <boost/filesystem.hpp>

/** 
 * The goal of filesystem namespace is to make commonly performed
 * tasks with the boost filesystem library much easier to implement.
 * This wrapper abstracts the boost filesystem library, and 
 * forwards exceptions through runtime errors that povide the 
 * line number and file that the exception was thrown in.  The 
 * purpose of this wrapper is to eliminate a lot of the time-consuming
 * lower-level aspects of the boost library so that the 
 * programmer can worry less about error handling, and more
 * about the algorithms.  If you find anything that can
 * be improved, you may submit a pull-request to the github unit_testing branch
 * for review and testing.
  */

/** 
 * All path-based arguments take FULL paths.
 */

namespace fsys
{
    typedef struct result_data_boolean result_data_boolean;
    typedef class tree_iterator_class tree_iterator_class;
    typedef class tree_riterator_class tree_riterator_class;
    typedef class copy_iterator_class copy_iterator_class;
    typedef class delete_iterator_class delete_iterator_class;
    
    
    result_data_boolean fdelete(const std::string&);
    result_data_boolean fcopy(const std::string&, const std::string&);
    result_data_boolean create_folder(const std::string&);
    result_data_boolean frename(const std::string&, const std::string&);
    result_data_boolean fmove(const std::string&, const std::string&);
    bool can_delete(const std::string&);
    char pref_slash();
    
    std::string current_path();
    result_data_boolean is_file(const std::string&);
    result_data_boolean is_folder(const std::string&);
    result_data_boolean is_symlink(const std::string&);
    result_data_boolean copy_folders(const std::string&, const std::string&, const std::string&);
    
    
    typedef struct result_data_boolean
    {
        explicit result_data_boolean()
        {
            this->value = true;
        }
        
        result_data_boolean& operator=(const result_data_boolean& res)
        {
            if(this != &res)
            {
                this->value = res.value;
                this->error = res.error;
                this->path = res.path;
            }
            return *this;
        }
        
        bool operator==(const result_data_boolean& res) const
        {
            return (
                    (this->value == res.value) && 
                    (this->error == res.error) && 
                    (this->path == res.path));
        }
        
        bool operator!=(const result_data_boolean& res) const
        {
            return !(this->operator==(res));
        }
        
        bool operator!() const
        {
            return !(this->value);
        }
        
        bool value;
        std::string error, path;
    } result_data_boolean;
    
    /* Iterators start with the first element inside the folder.*/
    typedef class tree_iterator_class
    {
    public:
        explicit tree_iterator_class(const std::string&);
        explicit tree_iterator_class() : p(), it(), end(), is_good_path(false) {}
        
        ~tree_iterator_class();
        
        tree_iterator_class& operator=(const tree_iterator_class&);
        tree_iterator_class operator++();
        
        unsigned int count_from_end() const;
        unsigned int count_from_beg() const;
        std::string value() const;
        bool at_end() const;
        
        
    private:
        boost::filesystem::path p;
        boost::filesystem::directory_iterator it, end;
        bool is_good_path;
    } tree_iterator_class;
    
    /* Iterators start with the first element inside the folder.*/
    typedef class tree_riterator_class
    {
    public:
        explicit tree_riterator_class(const std::string&);
        explicit tree_riterator_class() : p(), it(), end(), is_good_path(false) {}
        
        virtual ~tree_riterator_class();
        
        virtual tree_riterator_class& operator=(const tree_riterator_class&);
        virtual tree_riterator_class operator++();
        
        unsigned int count_from_end();
        unsigned int count_from_beg();
        std::string value() const;
        bool at_end() const;
        
        
    protected:
        boost::filesystem::path p;
        boost::filesystem::recursive_directory_iterator it, end;
        bool is_good_path;
    } tree_riterator_class;
    
    
    
    typedef class copy_iterator_class : public tree_riterator_class
    {
    public:
        explicit copy_iterator_class() : tree_riterator_class(), success(), dest() {}
        explicit copy_iterator_class(const std::string&, const std::string&);
        
        ~copy_iterator_class();
        
        tree_riterator_class& operator=(const copy_iterator_class&);
        tree_riterator_class operator++();
        void skip();
        
        result_data_boolean success;
        
    private:
        std::string dest;
        using tree_riterator_class::p;
        using tree_riterator_class::it;
        using tree_riterator_class::end;
        using tree_riterator_class::is_good_path;
    } copy_iterator_class;
    
    
    typedef class delete_iterator_class : public tree_riterator_class
    {
    public:
        explicit delete_iterator_class() : tree_riterator_class(), success() {}
        explicit delete_iterator_class(const std::string&);
        
        ~delete_iterator_class();
        
        tree_riterator_class& operator=(const delete_iterator_class&);
        tree_riterator_class operator++();
        void skip();
        
        
        result_data_boolean success;
    private:
        using tree_riterator_class::p;
        using tree_riterator_class::it;
        using tree_riterator_class::end;
        using tree_riterator_class::is_good_path;
        
        void delete_path(const std::string&);
        
    } delete_iterator_class;
    
    /* Returns a string that represents a group of characters that should
     never appear in a filename on Windows. */
    inline std::string windows_bad_filename_chars()
    {
        return std::string("<>:\"\\/|?*");
    }
    
    /* Returns a string that represents a group of characters that should
     never appear in a filename on Linux. */
    inline std::string linux_bad_filename_chars()
    {
        return std::string("\0\n/");
    }
    
    
}

#endif