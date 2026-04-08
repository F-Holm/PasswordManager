add_library(types STATIC
    src/account_binary.cc
    src/account_index.cc
    src/account_view.cc
)

target_include_directories(types PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(types PUBLIC crypto)

ConfigModule(types)
