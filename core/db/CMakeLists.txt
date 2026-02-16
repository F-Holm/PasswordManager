add_library(db STATIC
    db.cc
)

target_include_directories(db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

ConfigModule(db)
