add_executable(account_test
    account_binary_test.cc
    account_index_test.cc
    account_view_test.cc
)

target_link_libraries(account_test PRIVATE account)

AddTests(account_test)

FormatTest(account_test)
