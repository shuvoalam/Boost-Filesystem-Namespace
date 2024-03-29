# This cmake script is COPYRIGHT Jonathan Craig Whitlock
#
# I release this under the MIT License:
#
# ------------------------------------------------------------------------------
# The MIT License (MIT)
#
# Copyright (c) 2014 Jonathan Whitlock
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# ------------------------------------------------------------------------------
#
#
# General purpose CMakeLists.txt can generate a makefile
# easily by specifying the folder containing sources,
# and running CMake to generate the makefile.

set(PROGRAM_NAME filesystem_namespace)
set(SOURCE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/source)

#compiler flag settings
set(BUILD_RELEASE false) #default debug
set(ENABLE_MULTITHREADING false)
set(CPLUSPLUS_11 false)
set(C_11 false)
set(STRICT_COMPILE true)
set(STATIC_COMPILE true)
set(OPTIMIZE_BUILD_FLAGS true) #will only be used for release.  :)
set(64_BIT_BUILD true)

#boost settings
set(USING_BOOST_LIBRARY true)
set(BOOST_LIB_COMPONENTS system filesystem) #change these library filenames if necessary
set(BOOST_LIB_COMPONENTS_LITERAL libboost_system.a libboost_filesystem.a)

#Additional Libraries
set(USING_ADDITIONAL_LIBS false)
set(ADDITIONAL_LIB_COMPONENTS_LITERAL libUnitTest++.a) #add additional misc libs here

#ncurses settings
set(USING_NCURSES_LIBRARY false)
set(NCURSES_LIB_COMPONENTS ncurses) #change these library filenames if necessary
set(NCURSES_LIB_COMPONENTS_LITERAL libncurses.a)

#-------------------------------------------------------------------------------------------------
#cmake settings:

#CMAKE_CXX_FLAGS_DEBUG = debug
#CMAKE_CXX_FLAGS_RELEASE = release
#CMAKE_CXX_FLAGS = no flags
if(BUILD_RELEASE) #this makes it easier to switch between release and debug
    set(CMAKE_BUILD_TYPE Release)
else()
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(BUILD_SHARED_LIBS OFF)


# =================================================================================================
# =================================================================================================
# =================================================================================================
# =================================================================================================


#sets c++11
macro(use_cplusplus11)
    add_definitions(-std=c++11)
endmacro(use_cplusplus11)

#sets c99
macro(use_c99)
    add_definitions(-std=c99)
endmacro(use_c99)

#sets c11
macro(use_c11)
    add_definitions(-std=c11)
endmacro(use_c11)

#sets strict
macro(set_strict)
    add_definitions(-Wall -pedantic -Wextra)
endmacro(set_strict)

#sets static
macro(set_static)
    add_definitions(-static)
endmacro(set_static)

macro(add_mthread)
    if(CPLUSPLUS_11)
        if(ENABLE_MULTITHREADING)
            add_definitions(-pthread -Wl,--no-as-needed)
        endif()
    endif()
endmacro(add_mthread)

macro(link_mthread)
    if(CPLUSPLUS_11)
        if(ENABLE_MULTITHREADING)
            target_link_libraries(${PROGRAM_NAME} pthread)
        endif()
    endif()
endmacro(link_mthread)

#adds optimization flags
macro(add_build_optimizations)
    if(BUILD_RELEASE)
        add_definitions(-O -O3)
    endif()
endmacro(add_build_optimizations)

#adds boost library
macro(add_boost)
    set(Boost_USE_STATIC_LIBS ON)
    set(Boost_USE_MULTITHREADED ON)
    set(Boost_USE_STATIC_RUNTIME ON)
    find_package(Boost REQUIRED COMPONENTS ${BOOST_LIB_COMPONENTS})
    if(Boost_FOUND)
        include_directories(${Boost_INCLUDE_DIRS})
        link_directories(${Boost_LIBRARY_DIRS})
        target_link_libraries(${PROGRAM_NAME} ${BOOST_LIB_COMPONENTS_LITERAL})
    else()
        message("FATAL ERROR: boost library not found!")
    endif()
endmacro(add_boost)

#adds additional libraries
macro(add_additional_libs)
    if(USING_ADDITIONAL_LIBS)
        foreach(tempf ${ADDITIONAL_LIB_COMPONENTS_LITERAL})
            message("Searching for library " ${tempf})
            find_library(LIBUNIT_TEST_PP_LOCATION add_unittestpp NAMES ${tempf})
            if(NOT LIBUNIT_TEST_PP_LOCATION)
                message("FATAL ERROR: Could not find " ${tempf} "!")
            else()
                message("Found library " ${tempf})
                target_link_libraries(${PROGRAM_NAME} ${LIBUNIT_TEST_PP_LOCATION})
            endif()
        endforeach()
    endif()
endmacro(add_additional_libs)

#adds ncurses library
macro(add_ncurses)
    set(CURSES_NEED_NCURSES TRUE)
    find_package(Curses 5.9 REQUIRED COMPONENTS ${NCURSES_LIB_COMPONENTS})
    if(CURSES_FOUND)
        include_directories(${CURSES_INCLUDE_DIR})
        target_link_libraries(${PROGRAM_NAME} ${NCURSES_LIB_COMPONENTS})
    else()
        message("FATAL ERROR: curses library not found!")
    endif()
endmacro(add_ncurses)

macro(find_dependent_includes)

    file(GLOB_RECURSE project_HEADERS "${SOURCE_FOLDER}/*.hpp")

    set(project_INCLUDE_DIRS "")
    foreach(_headerFile ${project_HEADERS})
        get_filename_component(_dir ${_headerFile} PATH)
        list(APPEND project_INCLUDE_DIRS ${_dir})
    endforeach(_headerFile)
    list(REMOVE_DUPLICATES project_INCLUDE_DIRS)
    
    foreach(_include_directory_path ${project_INCLUDE_DIRS})
        include_directories(${_include_directory_path})
    endforeach(_include_directory_path)

endmacro(find_dependent_includes)

#adds executable and source files
macro(add_files)
    find_dependent_includes()
    file(GLOB_RECURSE CPP_SOURCE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${SOURCE_FOLDER}/*.cpp" "${SOURCE_FOLDER}/*.hpp")
    add_executable(${PROGRAM_NAME} ${CPP_SOURCE})
    file(GLOB_RECURSE CPP "${SOURCE_FOLDER}/*.cpp" "${SOURCE_FOLDER}/*.hpp")
    link_directories(${CPP})
endmacro(add_files)

cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake_modules" ${CMAKE_MODULE_PATH})
project(${PROGRAM_NAME})

if(STRICT_COMPILE)
    set_strict()
endif()

if(NOT BUILD_RELEASE)
    add_definitions(-g)
endif()

if(OPTIMIZE_BUILD_FLAGS)
    add_build_optimizations()
endif()

if(STATIC_COMPILE)
    set_static()
endif()

if(CPLUSPLUS_11)
    use_cplusplus11()
endif()

if(64_BIT_BUILD)
    add_definitions(-m64)
else()
    add_definitions(-m32)
endif()

if(USING_BOOST_LIBRARY)
    foreach(temp ${BOOST_LIB_COMPONENTS_LITERAL})
        add_definitions(-l:${temp})
    endforeach(temp)
endif()

if(USING_NCURSES_LIBRARY)
    foreach(temp2 ${NCURSES_LIB_COMPONENTS_LITERAL})
        add_definitions(-l:${temp2})
    endforeach(temp2)
endif()

if(USING_ADDITIONAL_LIBS)
    foreach(temp1 ${ADDITIONAL_LIB_COMPONENTS_LITERAL})
        add_definitions(-l:${temp1})
    endforeach(temp1)
endif()

add_mthread()

add_files()

link_mthread()

if(USING_ADDITIONAL_LIBS)
    add_additional_libs()
endif()

if(USING_NCURSES_LIBRARY)
    add_ncurses()
endif()

if(USING_BOOST_LIBRARY)
    add_boost()
endif()
