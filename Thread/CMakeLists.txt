set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(..)

set(THREAD_FILES
    ThreadPool.h
    ThreadPool.cpp)

add_library(xthread STATIC ${THREAD_FILES})