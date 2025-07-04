add_subdirectory(openssl)

add_library(seguridad INTERFACE)

target_include_directories(seguridad INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(seguridad INTERFACE openssl_)