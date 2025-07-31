add_executable(cuenta_test
    cuenta_test.cc
)

target_link_libraries(cuenta_test PRIVATE
    cuenta
    data_block
)

AddTests(cuenta_test)