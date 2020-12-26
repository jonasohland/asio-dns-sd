add_executable(dnssd_browse_sync_example "${CMAKE_CURRENT_LIST_DIR}/browse_sync.cc")
add_executable(dnssd_browse_async_example "${CMAKE_CURRENT_LIST_DIR}/browse_async.cc")

add_executable(dnssd_linkage_test "${CMAKE_CURRENT_LIST_DIR}/link_test_a.cc" 
                                    "${CMAKE_CURRENT_LIST_DIR}/link_test_b.cc")

target_link_libraries(dnssd_browse_sync_example async_dnssd)
target_link_libraries(dnssd_browse_async_example async_dnssd)
target_link_libraries(dnssd_linkage_test async_dnssd)