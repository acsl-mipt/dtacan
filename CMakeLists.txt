project(dtacan)

cmake_minimum_required(VERSION 2.8.11)

add_library(dtacan
    src/dtacan/Encoder.h
    src/dtacan/BaudRate.h
    src/dtacan/Util.h
)

if (NOT MSVC)
    target_compile_options(dtacan PUBLIC -std=c++11)
    target_compile_options(dtacan PRIVATE -Wall -Wextra)
endif()

target_include_directories(dtacan PUBLIC src)

add_subdirectory(thirdparty/gtest)
add_subdirectory(tests)

