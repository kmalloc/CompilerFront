set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.. ../..)

set(INK_TEST_FILES
    testLex.cc
    testParser.cc)

link_directories(..)
add_executable(ink_unittest ${INK_TEST_FILES})

target_link_libraries(ink_unittest ink)
target_link_libraries(ink_unittest gtest_main gtest pthread)
