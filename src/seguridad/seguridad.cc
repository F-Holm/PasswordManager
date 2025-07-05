#include "seguridad.hh"

#include "openssl.hh"

auto Seg::hash256(std::string str) -> std::string{
    OpenSSL_::hash256(str);
}

auto Seg::hash256_x(std::string str, const size_t &LEN_RTA) -> std::string{
    OpenSSL_::hash256_x(str, LEN_RTA)
}

auto Seg::encriptar(std::string str, std::string key, std::string &tag) -> std::string{
    OpenSSL_::encriptar(str, key, tag)
}

auto Seg::desencriptar(std::string str, std::string key, std::string tag) -> std::string{
    OpenSSL_::desencriptar(str, key, tag)
}
