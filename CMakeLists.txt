cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(heimdallr_client)

set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/build)
set(SIO_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sio_client)

include(${SIO_CLIENT_DIR}/CMakeLists.txt)
include_directories(${SIO_CLIENT_DIR}/src)

add_library(heimdallr_client STATIC ${SRC_DIR}/heimdallr_client.cpp)
set_property(TARGET heimdallr_client PROPERTY CXX_STANDARD 11)

find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS date_time)
target_include_directories(heimdallr_client PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(heimdallr_client sioclient ${Boost_LIBRARIES})