add_library(db
    db.cc
)

target_include_directories(db PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(db PUBLIC
    data_block
)