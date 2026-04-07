add_library(core STATIC
    core.cc
)

target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(core PUBLIC account)

ConfigModule(core)
