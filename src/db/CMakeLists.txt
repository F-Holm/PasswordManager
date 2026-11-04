add_library(db STATIC
    src/db.cc
    src/db_status.cc
    src/statement.cc
    src/transaction.cc
)

target_include_directories(db PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries(db PUBLIC crypto types sqlite3)

ConfigModule(db)
