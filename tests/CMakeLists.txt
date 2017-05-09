include_directories(${GTEST_INCLUDE_DIRS})

if(NOT MSVC)
    add_definitions(-Wall -Wextra)
endif()

set(TESTS_DIR ${CMAKE_BINARY_DIR}/bin/tests)
file(MAKE_DIRECTORY ${TESTS_DIR})

macro(add_unit_test test file)
    add_executable(${test} ${file})
    target_link_libraries(${test}
        ${ARGN}
        dtacan
        gtest
        gtest_main
    )

    set_target_properties(${test}
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${TESTS_DIR}
        FOLDER "tests"
    )
    add_test(${test} ${TESTS_DIR}/${test})
endmacro()

add_unit_test(encoder_tests EncoderTest.cpp)
add_unit_test(parser_tests ParserTest.cpp)

