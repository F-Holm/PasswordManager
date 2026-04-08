add_executable(db_test
    storage_test.cc
)

target_link_libraries(db_test PRIVATE db storage)

ConfigTest(db_test)
