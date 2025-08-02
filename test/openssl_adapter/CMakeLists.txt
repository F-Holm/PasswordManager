add_executable(openssl_adapter_test
    openssl_adapter_test.cc
)

target_link_libraries(openssl_adapter_test PRIVATE
    openssl_adapter
)

AddTests(openssl_adapter_test)

Format(openssl_adapter_test .)