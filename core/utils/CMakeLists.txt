add_library(utils STATIC
    set_rand.cc
)

target_include_directories(utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

ConfigModule(utils)
