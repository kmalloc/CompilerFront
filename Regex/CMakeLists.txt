set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(..)

set(REG_FILES
    ../Basic/NonCopyable.h
    ../Parsing/LexException.h
    ../Parsing/LexTokenizerBase.h
    ../Parsing/SyntaxTreeBase.h
    ../Parsing/SyntaxTreeNodeBase.h
    AutomatonBase.cc
    AutomatonBase.h
    MachineComponent.h
    RegExpAutomata.cc
    RegExpAutomata.h
    RegExpSyntaxTree.cc
    RegExpSyntaxTree.h
    RegExpSynTreeNode.cc
    RegExpSynTreeNode.h
    RegExpTokenizer.cc
    RegExpTokenizer.h)

add_library(xreg STATIC ${REG_FILES})