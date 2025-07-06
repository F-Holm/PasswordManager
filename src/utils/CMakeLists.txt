add_library(utils
    utils/set_rand.cc
)

target_include_directories(utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/utils")