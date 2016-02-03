project(CompilerFront)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(Regex bin/reg)
add_subdirectory(Regex/unittest bin/reg/test)
add_subdirectory(ink bin/ink)
add_subdirectory(ink/unittest bin/ink/test)
add_subdirectory(Basic bin/basic)
add_subdirectory(Practise bin/practise)