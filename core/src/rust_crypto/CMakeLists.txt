add_library(rust_crypto STATIC)

target_include_directories(rust_crypto PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rust_crypto PRIVATE rust_crypto_lib)

Format(rust_crypto ./*)
AddCppCheck(rust_crypto)
