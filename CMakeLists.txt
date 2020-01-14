cmake_minimum_required(VERSION 3.8)
project(splc LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
BISON_TARGET(Syntax syntax.y ${CMAKE_CURRENT_BINARY_DIR}/syntax.cpp)
FLEX_TARGET(Lex lex.l  ${CMAKE_CURRENT_BINARY_DIR}/lex.cpp)
ADD_FLEX_BISON_DEPENDENCY(Lex Syntax)

add_library(parser
        ast.cpp
        ast.hpp
        ast_visitor.hpp
        parser.hpp
        syntax_err.hpp
        utils.hpp
        ${BISON_Syntax_OUTPUTS}
        ${FLEX_Lex_OUTPUTS})

add_library(semantic
        ast.hpp
        ast_visitor.hpp
        semantic.cpp
        semantic.hpp
        semantic_err.hpp
        symbol_table.hpp
        type.cpp
        type.hpp
        utils.hpp)

add_library(gentac
        ast.hpp
        ast_visitor.hpp
        gen_tac.cpp
        gen_tac.hpp
        tac.hpp)

add_executable(splc
        main.cpp
        ast_dump.hpp
        parser.hpp
        semantic.hpp
        gen_tac.hpp)

target_link_libraries(splc parser semantic gentac)

add_subdirectory(tests)
