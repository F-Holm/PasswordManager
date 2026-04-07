add_executable(storage_test
    storage_test.cc
)

target_link_libraries(storage_test PRIVATE storage)

ConfigTest(storage_test)
