cmake_minimum_required(VERSION 3.12)

project(async_mdns CXX)

set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Boost 1.73 REQUIRED COMPONENTS)
find_package(DNSSD REQUIRED)

file(GLOB_RECURSE async_mdns_sources "${CMAKE_CURRENT_LIST_DIR}/include/*.?pp")

add_library(async_mdns INTERFACE ${async_mdns_includes})

add_executable(async_mdns_sources ${async_mdns_sources})

target_compile_features(async_mdns INTERFACE cxx_std_11)

target_include_directories(async_mdns INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(async_mdns INTERFACE ${DNSSD_INCLUDE_DIRS})

target_link_libraries(async_mdns INTERFACE Boost::headers)
target_link_libraries(async_mdns INTERFACE ${DNSSD_LIBRARIES})

target_link_libraries(async_mdns_sources PRIVATE async_mdns)

target_compile_options(async_mdns_sources PRIVATE -Wall -Wpedantic -Wextra)
