add_library(data_block STATIC
    data_block.cc
)

target_include_directories(data_block PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})