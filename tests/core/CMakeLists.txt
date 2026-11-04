add_executable(core_test
    core_test.cc
    core_interface_test.cc
    password_generator_test.cc
    c_header_test.c
)

# Compile the C smoke test as strict C99 so any C++-only construct in
# core/core_interface.h breaks the build.
if(NOT MSVC)
    set_source_files_properties(c_header_test.c PROPERTIES
        COMPILE_OPTIONS "-std=c99;-pedantic-errors;-Wall;-Werror")
endif()

target_link_libraries(core_test PRIVATE core)

ConfigTest(core_test)
