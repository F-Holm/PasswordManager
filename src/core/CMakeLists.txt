add_library(core STATIC
    src/core.cc
    src/core_interface.cc
)

target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(core PUBLIC crypto db types utils)

ConfigModule(core)
