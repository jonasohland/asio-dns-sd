add_executable(mdns_asio_example "${CMAKE_CURRENT_LIST_DIR}/test.cc")

target_link_libraries(mdns_asio_example async_mdns)