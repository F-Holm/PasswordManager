add_executable(openssl_test
    openssl_test.cc
)

target_link_libraries(openssl_test PRIVATE
    openssl_static
)

AddTests(openssl_test)

Format(openssl_test .)