cmake_minimum_required(VERSION 3.22)
project(fcap-c)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Werror)

if ($<CONFIG:MinSizeRel>)
    add_compile_options(-Os -ffunction-sections -fdata-sections)
    add_link_options(-ffunction-sections -fdata-sections)
endif()

# Make the FCAP Library
add_compile_options(-D__unix__)

add_custom_command(
  COMMAND bitproto c proto/fcap.bitproto
  OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/proto/fcap_bp.h ${CMAKE_CURRENT_SOURCE_DIR}/proto/fcap_bp.c
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(proto)
include_directories(include)
include_directories(include/transports)


add_library(fcap src/fcap.c src/fcap_pkt.c proto/fcap_bp.c proto/bitproto.c)

add_library(transports src/transports/transport_udp.c)

# add_custom_command(
#   COMMAND bitproto c proto/encryption.bitproto
#   OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/proto/encryption_bp.h ${CMAKE_CURRENT_SOURCE_DIR}/proto/encryption_bp.c
#   WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# )

# add_library(middleware_encryption src/middleware/middleware_encryption.c proto/encryption_bp.c)


# Make the tests
enable_testing()
find_package(GTest CONFIG REQUIRED)

# Make user sample
add_executable(fcap_sample tests/sample.c)
target_link_libraries(fcap_sample fcap transports)

# Make sample server
# add_executable(fcap_server tests/server.c)
# target_link_libraries(fcap_server fcap transports)

# Make sample client
# add_executable(fcap_client tests/client.c)
# target_link_libraries(fcap_client fcap transports)

# add_executable(fcap_tests tests/protocol_tests.cpp)
# target_link_libraries(fcap_tests fcap GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)
# add_test(FcapTest fcap_tests)

# Automatically build and update the docs when we build the fcap library
# add_custom_command(
#   TARGET fcap POST_BUILD
#   WORKING_DIRECTORY ../
#   COMMAND doxygen ./docs.config
#   VERBATIM)
