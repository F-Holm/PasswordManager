add_executable(types_test
    account_binary_test.cc
    account_index_test.cc
    account_test.cc
)

target_link_libraries(types_test PRIVATE types)

AddTests(types_test)

Format(types_test .)