add_library(crypto STATIC
    crypto.cc
)

target_include_directories(crypto PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(crypto PUBLIC rust_crypto)

FormatModule(crypto)
AddCppCheck(crypto)
