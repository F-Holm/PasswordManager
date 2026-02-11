add_library(crypto STATIC
    crypto.cc
)

target_include_directories(crypto PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(crypto PUBLIC rust_crypto)

Format(crypto .)
Format(crypto ./*)
AddCppCheck(crypto)
