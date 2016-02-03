set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

include_directories(.)

set(CALC_PARSER_FILES CalcParser.cpp CalcParser.h)
set(EXPRESSION_FILES expressive_cpp.cc)

add_executable(run_express_tmp ${EXPRESSION_FILES})

set(BOOST_INCLUDE /usr/include/boost)
include_directories(${BOOST_INCLUDE})

add_library(calc_parser STATIC ${CALC_PARSER_FILES})
add_executable(unittest_calc_parser CalcParserTest.cpp)
target_link_libraries(unittest_calc_parser calc_parser gtest_main gtest pthread)