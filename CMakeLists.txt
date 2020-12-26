cmake_minimum_required(VERSION 3.12)

project(async_dnssd CXX)

set(CMAKE_CXX_CLANG_TIDY "clang-tidy;-checks=*")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

find_package(Boost 1.73 REQUIRED COMPONENTS)
find_package(DNSSD REQUIRED)

file(GLOB_RECURSE async_dnssd_sources "${CMAKE_CURRENT_LIST_DIR}/include/*.?pp")

add_library(async_dnssd INTERFACE ${async_dnssd_includes})

add_executable(async_dnssd_sources ${async_dnssd_sources})

target_compile_features(async_dnssd INTERFACE cxx_std_11)

target_include_directories(async_dnssd INTERFACE "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(async_dnssd INTERFACE ${DNSSD_INCLUDE_DIRS})

target_link_libraries(async_dnssd INTERFACE Boost::headers)
target_link_libraries(async_dnssd INTERFACE ${DNSSD_LIBRARIES})

target_link_libraries(async_dnssd_sources PRIVATE async_dnssd)
target_compile_options(async_dnssd_sources PRIVATE -Wall -Wpedantic -Wextra)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/example)