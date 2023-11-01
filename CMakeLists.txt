cmake_minimum_required(VERSION 3.25)
project(fcap-c)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# if ($<CONFIG:MinSizeRel>)
    add_compile_options(-Os -ffunction-sections -fdata-sections)
    add_link_options(-ffunction-sections -fdata-sections)
# endif()

# Make the FCAP Library
include_directories(include)

add_library(fcap src/fcap.c src/fcap_pkt.c)

add_library(fcap_udp src/fcap_udp.c)

# Make the tests
enable_testing()
find_package(GTest CONFIG REQUIRED)

# Make user sample
add_executable(fcap_sample tests/sample.c)
target_link_libraries(fcap_sample fcap fcap_udp)

# Make sample server
add_executable(fcap_server tests/server.c)
target_link_libraries(fcap_server fcap fcap_udp)

# Make sample client
add_executable(fcap_client tests/client.c)
target_link_libraries(fcap_client fcap fcap_udp)

add_executable(fcap_tests tests/protocol_tests.cpp)
target_link_libraries(fcap_tests fcap GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
add_test(FcapTest fcap_tests)
