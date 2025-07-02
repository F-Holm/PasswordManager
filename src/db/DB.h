#pragma once

#include <vector>

#include "DataBlock.h"

namespace DB {
std::vector<DataBlock> leer(const std::string nombreArchivo);
void escribir(const std::string nombreArchivo, std::vector<DataBlock> datos);
} // namespace DB