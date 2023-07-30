cmake_minimum_required(VERSION 3.25)
project(fcap-c)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make the FCAP Library
include_directories(include)

add_library(fcap src/fcap.c)

# Make the tests
enable_testing()
find_package(GTest CONFIG REQUIRED)

add_executable(fcap_tests tests/tests.cpp)
target_link_libraries(fcap_tests fcap GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(FcapTest fcap_tests)


