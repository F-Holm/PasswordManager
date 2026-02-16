add_executable(crypto_test
    crypto_test.cc
)

target_link_libraries(crypto_test PRIVATE crypto)

ConfigTest(crypto_test)
