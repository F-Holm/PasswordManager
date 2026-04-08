add_library(storage INTERFACE)

target_include_directories(storage INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(storage INTERFACE crypto)

ConfigModule(storage)

add_library(db PUBLIC
    src/db.cc
)

target_include_directories(db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(db PUBLIC crypto storage types)

ConfigModule(db)
