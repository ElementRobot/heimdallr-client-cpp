 cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
project(heimdallr_client_test)

set(HEIMDALLR_CLIENT_DIR ${CMAKE_CURRENT_LIST_DIR}/..)

include(${HEIMDALLR_CLIENT_DIR}/CMakeLists.txt)

find_package(Boost ${BOOST_VER} REQUIRED COMPONENTS unit_test_framework date_time thread)

add_executable(heimdallr_client_test heimdallr_client_test.cpp)
set_property(TARGET heimdallr_client_test PROPERTY CXX_STANDARD 11)

target_link_libraries(heimdallr_client_test PRIVATE heimdallr_client ${Boost_LIBRARIES})
target_include_directories(heimdallr_client_test PRIVATE ${HEIMDALLR_CLIENT_DIR}/src ${Boost_INCLUDE_DIRS})