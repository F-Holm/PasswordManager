macro(LinkOpenSSL target)
    find_package(OpenSSL REQUIRED)
    target_link_libraries("${target}" PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endmacro()
