set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")

include_directories(.. ../..)

set(THREAD_TEST_FILES
    testThreadpool.cc)

link_directories(..)
add_executable(test_thread ${THREAD_TEST_FILES})

target_link_libraries(test_thread gtest_main gtest pthread)
