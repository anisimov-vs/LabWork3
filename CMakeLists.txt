cmake_minimum_required(VERSION 3.16)
project(jump_list VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library target
add_library(jump_list INTERFACE)
target_include_directories(jump_list INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Dependencies
find_package(Threads REQUIRED)
find_package(GTest REQUIRED)

# Tests
enable_testing()
add_executable(test_jump_list tests/test.cpp)
target_link_libraries(test_jump_list PRIVATE jump_list GTest::gtest_main Threads::Threads)
add_test(NAME jump_list_tests COMMAND test_jump_list) 