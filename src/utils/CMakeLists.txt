add_library(utils STATIC
    set_rand.cc
)

target_include_directories(utils PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")