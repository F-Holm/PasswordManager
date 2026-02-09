add_executable(libressl_adapter_test
    libressl_adapter_test.cc
)

target_link_libraries(libressl_adapter_test PRIVATE
    libressl_adapter
)

AddTests(libressl_adapter_test)

Format(libressl_adapter_test .)
