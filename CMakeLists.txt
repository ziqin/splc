cmake_minimum_required(VERSION 3.8)
project(splc LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_CXX_FLAGS "--coverage")

include_directories(. ${CMAKE_CURRENT_BINARY_DIR})

# On macOS, search Homebrew for keg-only versions of Bison and Flex. Xcode does
# not provide new enough versions for us to use.
if (CMAKE_HOST_SYSTEM_NAME MATCHES "Darwin")
    execute_process(
            COMMAND brew --prefix bison
            RESULT_VARIABLE BREW_BISON
            OUTPUT_VARIABLE BREW_BISON_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_BISON EQUAL 0 AND EXISTS "${BREW_BISON_PREFIX}")
        message(STATUS "Found Bison keg installed by Homebrew at ${BREW_BISON_PREFIX}")
        set(BISON_EXECUTABLE "${BREW_BISON_PREFIX}/bin/bison")
    endif()

    execute_process(
            COMMAND brew --prefix flex
            RESULT_VARIABLE BREW_FLEX
            OUTPUT_VARIABLE BREW_FLEX_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (BREW_FLEX EQUAL 0 AND EXISTS "${BREW_FLEX_PREFIX}")
        message(STATUS "Found Flex keg installed by Homebrew at ${BREW_FLEX_PREFIX}")
        set(FLEX_EXECUTABLE "${BREW_FLEX_PREFIX}/bin/flex")
    endif()
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)
BISON_TARGET(Syntax syntax.y ${CMAKE_CURRENT_BINARY_DIR}/syntax.cpp)
FLEX_TARGET(Lex lex.l  ${CMAKE_CURRENT_BINARY_DIR}/lex.cpp)
ADD_FLEX_BISON_DEPENDENCY(Lex Syntax)

add_library(parser
        ast.cpp
        ast.hpp
        parser.hpp
        syntax_err.hpp
        utils.hpp
        ${BISON_Syntax_OUTPUTS}
        ${FLEX_Lex_OUTPUTS})

add_library(semantic
        ast.hpp
        semantic.cpp
        semantic.hpp
        semantic_err.hpp
        symbol_table.hpp
        type.cpp
        type.hpp
        utils.hpp)

add_library(gentac
        ast.hpp
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
