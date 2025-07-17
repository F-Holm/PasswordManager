add_library(db STATIC
    db.cc
)

target_include_directories(db PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(db PRIVATE
    data_block
)