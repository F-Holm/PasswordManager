add_executable(db_test
    db_test.cc
    statement_test.cc
    transaction_test.cc
)

target_link_libraries(db_test PRIVATE db)

ConfigTest(db_test)
