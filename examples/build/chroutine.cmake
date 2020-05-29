add_library(chroutine INTERFACE)
target_include_directories(chroutine INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../engin
    ${CMAKE_CURRENT_SOURCE_DIR}/../../util
    ${CMAKE_CURRENT_SOURCE_DIR}/../../vendors
    ${CMAKE_CURRENT_SOURCE_DIR}/../../net/epoll
    ${CMAKE_CURRENT_SOURCE_DIR}/../../net/http_client
    ${CMAKE_CURRENT_SOURCE_DIR}/../../net/rpc
    ${CMAKE_CURRENT_SOURCE_DIR}/../../net/proto_code)
