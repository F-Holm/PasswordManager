add_library(db STATIC
    src/db.cc
)

target_include_directories(db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(db PUBLIC crypto types)

ConfigModule(db)
