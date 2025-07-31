add_executable(db_test
    db_test.cc
)

target_link_libraries(db_test PRIVATE
    db
    datablock
)

AddTests(db_test)