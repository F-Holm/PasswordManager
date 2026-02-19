add_library(db INTERFACE)

target_include_directories(db INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

ConfigInterface(db)
