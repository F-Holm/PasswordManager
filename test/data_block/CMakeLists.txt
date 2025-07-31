add_executable(data_block_test
    data_block_test.cc
)

target_link_libraries(data_block_test PRIVATE data_block)

AddTests(data_block_test)