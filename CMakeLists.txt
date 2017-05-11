enable_testing()

project(dtacan)

cmake_minimum_required(VERSION 2.8.11)

add_library(dtacan INTERFACE)

if (NOT MSVC)
    target_compile_options(dtacan INTERFACE -std=c++11)
endif()

target_include_directories(dtacan INTERFACE src)

add_subdirectory(thirdparty/gtest)
add_subdirectory(tests)

