add_library(db
    db.cc
)

target_include_directories(db PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})