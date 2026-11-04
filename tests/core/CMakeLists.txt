add_executable(core_test
    core_test.cc
    core_interface_test.cc
)

target_link_libraries(core_test PRIVATE core)

ConfigTest(core_test)
