add_executable(libressl_test
    libressl_test.cc
)

Link_TLS(libressl_test)

AddTests(libressl_test)

Format(libressl_test .)
