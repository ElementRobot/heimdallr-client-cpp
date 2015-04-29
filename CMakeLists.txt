cmake_minimum_required (VERSION 2.6)
project (hc_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SIO_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/lib/sio_client)

include(ExternalProject)
ExternalProject_Add(
    sio_client
    PREFIX ${SIO_CLIENT_DIR}/tmp
    SOURCE_DIR ${SIO_CLIENT_DIR}
)

include_directories(${SIO_CLIENT_DIR}/build/include)
link_directories(${SIO_CLIENT_DIR}/build/lib/Release)

add_library(heimdallr_client ${CMAKE_CURRENT_LIST_DIR}/src/heimdallr_client.cpp)
add_dependencies(heimdallr_client sio_client)
target_link_libraries(heimdallr_client boost_date_time boost_random boost_system sioclient)

add_executable(hc_test ${CMAKE_CURRENT_LIST_DIR}/src/test.cpp)
target_link_libraries(hc_test heimdallr_client)