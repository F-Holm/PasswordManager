add_library(core_interface SHARED
    core_interface.cc
)

target_include_directories(core_interface PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(core_interface PUBLIC account)

ConfigModule(core_interface)
