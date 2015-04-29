cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(heimdallr_client_test)

set(HEIMDALLR_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

include(ExternalProject)
ExternalProject_Add(
    hmdlr_client
    PREFIX ${HEIMDALLR_CLIENT_DIR}/tmp
    SOURCE_DIR ${HEIMDALLR_CLIENT_DIR}  # SOURCE_DIR is where the CMakeLists.txt is located
)

include_directories(${HEIMDALLR_CLIENT_DIR}/build/include)
link_directories(${HEIMDALLR_CLIENT_DIR}/build/link)
link_directories(${HEIMDALLR_CLIENT_DIR}/lib/sio_client/build/lib/Release)

add_executable(heimdallr_client_test ${CMAKE_CURRENT_LIST_DIR}/heimdallr_client_test.cpp)
set_property(TARGET heimdallr_client_test PROPERTY CXX_STANDARD 11)

add_dependencies(heimdallr_client_test hmdlr_client)
target_link_libraries(heimdallr_client_test heimdallr_client boost_date_time boost_random boost_system sioclient)