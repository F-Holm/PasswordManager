#pragma once

#include <vector>
#include <string>

#include "data_block.hh"

namespace DB {
auto leer(const std::string& nombreArchivo) -> std::vector<DataBlock>;
void escribir(const std::string& nombreArchivo, std::vector<DataBlock> datos);
} // namespace DB