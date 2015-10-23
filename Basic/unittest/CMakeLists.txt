set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.. ../..)

set(BASIC_FUN_TEST_FILES
        testVariant.cc)

link_directories(..)
add_executable(basic_func_unittest ${BASIC_FUN_TEST_FILES})
target_link_libraries(basic_func_unittest gtest_main gtest pthread)
