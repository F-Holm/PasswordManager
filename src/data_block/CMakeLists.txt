add_library(data_block STATIC
    data_block.cc
)

target_include_directories(data_block PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

InstrumentForCoverage(data_block)
CleanCoverage(data_block)
Format(data_block .)
AddCppCheck(data_block)
AddClangTidy(data_block)