add_library(account STATIC
    account_binary.cc
    account_index.cc
    account.cc
)

target_include_directories(account PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(account PRIVATE crypto)

Format(account .)
Format(account ./*)
AddCppCheck(account)
