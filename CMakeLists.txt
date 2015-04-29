cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(heimdallr_client)

set(SIO_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sio_client)
set(SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/build)

include(ExternalProject)
ExternalProject_Add(
    sio_client
    PREFIX ${SIO_CLIENT_DIR}/tmp
    SOURCE_DIR ${SIO_CLIENT_DIR}
)

include_directories(${SIO_CLIENT_DIR}/build/include)
link_directories(${SIO_CLIENT_DIR}/build/lib/Release)

add_library(heimdallr_client STATIC ${SRC_DIR}/heimdallr_client.cpp)
set_property(TARGET heimdallr_client PROPERTY CXX_STANDARD 11)

add_dependencies(heimdallr_client sio_client)
target_link_libraries(
    heimdallr_client boost_date_time boost_random boost_system sioclient
)

file(GLOB ALL_HEADERS ${SIO_CLIENT_DIR}/src/*.h ${SRC_DIR}/*.h)
install(FILES ${ALL_HEADERS} DESTINATION ${BUILD_DIR}/include)
install(TARGETS heimdallr_client DESTINATION ${BUILD_DIR}/link)