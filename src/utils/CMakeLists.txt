add_library(utils STATIC
    set_rand.cc
)

target_include_directories(utils PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

InstrumentForCoverage(utils)
CleanCoverage(utils)
Format(utils .)
AddCppCheck(utils)
AddClangTidy(utils)