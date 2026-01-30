#pragma once

#include <string>
#include <vector>

#include "data_block.hh"

class DB{
};

namespace DB {
auto Leer(const std::string &nom_arch) -> std::vector<DataBlock>;
void Escribir(const std::string &nom_arch, const std::vector<DataBlock> &datos);
}  // namespace DB