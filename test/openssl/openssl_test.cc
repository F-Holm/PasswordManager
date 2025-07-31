#include <openssl/crypto.h>
#include <gtest/gtest.h>

#include <iostream>

TEST(OpenSSLLinkageTest, OpenSSLVersionIsAvailable) {
    const char* version = OpenSSL_version(OPENSSL_VERSION);
    ASSERT_NE(version, nullptr);
    EXPECT_STRNE(version, "");  // Verifica que no sea string vacío
    std::cout << version << std::endl;
}
