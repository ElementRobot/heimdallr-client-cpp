cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(heimdallr_client)

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/build)
set(SIO_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sio_client)

include(${SIO_CLIENT_DIR}/CMakeLists.txt)
include_directories(heimdallr_client PRIVATE ${SIO_CLIENT_DIR}/src)

add_library(heimdallr_client STATIC ${SRC_DIR}/heimdallr_client.cpp)
set_property(TARGET heimdallr_client PROPERTY CXX_STANDARD 11)

target_link_libraries(
    heimdallr_client boost_date_time boost_random boost_system sioclient
)