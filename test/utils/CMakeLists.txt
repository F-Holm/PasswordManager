add_executable(utils_test
    set_rand_test.cc
)

target_link_libraries(utils_test PRIVATE
    utils
)

AddTests(utils_test)

Format(utils_test .)