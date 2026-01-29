add_executable(openssl_test
    openssl_test.cc
)

Link_TLS(openssl_test)

AddTests(openssl_test)

Format(openssl_test .)