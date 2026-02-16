add_library(account STATIC
    account_binary.cc
    account_index.cc
    account_view.cc
)

target_include_directories(account PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(account PUBLIC crypto)

ConfigModule(account)
