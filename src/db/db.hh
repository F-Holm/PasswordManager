#pragma once

#include <vector>
#include <string>

#include "data_block.hh"

namespace DB {
auto Leer(const std::string& nom_arch) -> std::vector<DataBlock>;
void Escribir(const std::string& nom_arch, std::vector<DataBlock> datos);
} // namespace DB