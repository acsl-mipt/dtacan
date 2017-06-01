enable_testing()

project(dtacan)

cmake_minimum_required(VERSION 2.8.11)

add_library(dtacan INTERFACE)

if (NOT MSVC)
    target_compile_options(dtacan INTERFACE -std=c++11)
endif()

target_include_directories(dtacan INTERFACE src)

get_directory_property(HAS_PARENT_SCOPE PARENT_DIRECTORY)
if(NOT HAS_PARENT_SCOPE)
    add_subdirectory(thirdparty/gtest)
    add_subdirectory(tests)
endif()

