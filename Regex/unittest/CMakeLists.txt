set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(.. ../..)

set(REG_TEST_FILES
    test_automaton.cc
    test_reg_exp_syn_tree.cc)

link_directories(..)
add_executable(reg_unittest ${REG_TEST_FILES})

target_link_libraries(reg_unittest xreg)
target_link_libraries(reg_unittest gtest_main gtest pthread)
