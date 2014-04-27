#Filesystem Namespace:  
The goal of this project was to create a simple way to use the Boost Library.  While the boost/filesystem library itself is a very good tool, commonly emplemented functionalities are very complex.  One such example is recursively copying a folder.  While the boost library provides for this, it doesn't allow the user to interrupt the process, forcing you to re-write it if you want such functionality for your project.

##So, what's different?  Why should I use this?  

I have always found it tedious to write code to recursively copy a folder, or delete a folder, just because I want to add somthing like a "percent complete" display.  This is way too time consuming for what I needed.  I decided to write this header and .cpp combo so that I can write my programs with much more ease.  The entire thing forwards any exceptions from boost, and any errors that occur are reported.

Provided in this header/.cpp combo are:

- **complete error reporting**: boost exceptions are all forwarded, and any other errors are checked for.  Any function that returns a result_data_boolean will double-check the results to make sure the operation was a success before returning anything.
- **tree_iterator_class**: iterates through a folder's most shallow level.
- **tree_riterator_class**: recursively iterates through a folder's contents.
- **copy_iterator_class : public tree_ritetator_class**: A recursive iterator that operates the same way in every respect to it's base class, except that it also copies one of the contents of a source folder to a destination folder upon each iteration.
- **delete_iterator_class : public tree_riterator_class**: A recursive iterator that operates the same way in every respect to it's base class, except that it deletes one of the contents of a folder upon each iteration.
- **is_folder**: A convenience.  Uses boost::system::error_code.
- **is_file**: A convenience.  Uses boost::system::error_code.
- **is_symlink**: A convenience.  Uses boost::system::error_code.
- **fcopy**: Will attempt to copy any path.  The equivilant of boost::filesystem::copy.  It performs checks, and takes advantage of boost::system::error_code.
- **fdelete**: deletes a path if it's either a file, or and empty folder.
- **fmove**: moves any path into a folder.  Added for convenience.  The equivilant of rename.
- **frename**: the equivilant of rename.  Added for convenience.  Reports any errors.

---

##Specs:  
This was made to work with Windows and Linux.  Windows has yet to be tested.  Linux has been tested.

- Uses the boost library.

---
This is still not a finished product.  I would like to perform more testing on windows and inux with it.  Right now, it is functional on Linux.