cmake_minimum_required(VERSION 3.8)
project(splc-tests LANGUAGES CXX VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(tests
        catch.hpp
        test_ast.cpp
        test_driver.cpp
        test_type.cpp
        test_utils.cpp
        test_visitor.cpp)

target_link_libraries(tests parser semantic)
