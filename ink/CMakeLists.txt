set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(..)

set(INK_FILES
        ../Basic/NonCopyable.h
        Ast.h
        AstVisitor.h
        Lexer.cc
        Lexer.h
        Noncopyable.h
        OpCode.cc
        OpCode.h
        Parser.cc
        Parser.h
        Types.h
        vm.cc
        vm.h
        ../Basic/Variant.h)

add_library(ink STATIC ${INK_FILES})