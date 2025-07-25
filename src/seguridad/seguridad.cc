#include "seguridad.hh"

#include "openssl_adapter.hh"

auto Seg::Hash256(std::string str) -> std::string{
    return OpenSSL_::Hash256(str);
}

auto Seg::Hash256_x(std::string str, const size_t &len_rta) -> std::string{
    return OpenSSL_::Hash256_x(str, len_rta);
}

auto Seg::Encriptar(std::string str, std::string key, std::string &tag) -> std::string{
    return OpenSSL_::Encriptar(str, key, tag);
}

auto Seg::Desencriptar(std::string str, std::string key, std::string tag) -> std::string{
    return OpenSSL_::Desencriptar(str, key, tag);
}
