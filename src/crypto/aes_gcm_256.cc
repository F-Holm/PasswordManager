#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#include <cstddef>
#include <memory>

#include "openssl_adapter.hh"

const static size_t TAG_LEN = 16;
constexpr static uint8_t iv_len = 96 / 8;
const static char* PROTOCOLO = "AES-256-GCM";
static std::string IV = OpenSslAdapter::Hash256_x(
    "Este es un vector de inicializacion super ultra mega secreto", iv_len);
static std::string AAD = "Esto es algo totalmente innecesario";

/*
 * A library context and property query can be used to select & filter
 * algorithm implementations. If they are NULL then the default library
 * context and properties are used.
 */
static OSSL_LIB_CTX* libctx = nullptr;
static const char* propq = nullptr;

static void ReleaseMemory(unsigned char*& outbuf, EVP_CIPHER*& cipher,
                          EVP_CIPHER_CTX*& ctx) {
  delete[] outbuf;
  EVP_CIPHER_free(cipher);
  EVP_CIPHER_CTX_free(ctx);
}

auto OpenSslAdapter::Encriptar(const std::string& str, std::string key,
                               std::string& tag) -> std::string {
  key = OpenSslAdapter::Hash256(key);

  EVP_CIPHER_CTX* ctx;
  EVP_CIPHER* cipher = nullptr;
  int outlen, tmplen;
  size_t gcm_ivlen = iv_len;
  auto* outbuf = new unsigned char[str.size()];
  unsigned char outtag[TAG_LEN];
  OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

  /* Create a context for the encrypt operation */
  ctx = EVP_CIPHER_CTX_new();
  if (ctx == nullptr) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Fetch the cipher implementation */
  cipher = EVP_CIPHER_fetch(libctx, PROTOCOLO, propq);
  if (cipher == nullptr) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Set IV length if default 96 bits is not appropriate */
  params[0] =
      OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

  /*
   * Initialise an encrypt operation with the cipher/mode, key, IV and
   * IV length parameter.
   * For demonstration purposes the IV is being set here. In a compliant
   * application the IV would be generated internally so the iv passed in
   * would be NULL.
   */
  if (!EVP_EncryptInit_ex2(
          ctx, cipher, reinterpret_cast<const unsigned char*>(key.c_str()),
          reinterpret_cast<const unsigned char*>(IV.c_str()), params)) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Zero or more calls to specify any AAD */
  if (!EVP_EncryptUpdate(ctx, NULL, &outlen,
                         reinterpret_cast<const unsigned char*>(AAD.c_str()),
                         AAD.size())) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Encrypt plaintext */
  if (!EVP_EncryptUpdate(ctx, outbuf, &outlen,
                         reinterpret_cast<const unsigned char*>(str.c_str()),
                         str.size())) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Finalise: note get no output for GCM */
  if (!EVP_EncryptFinal_ex(ctx, outbuf, &tmplen)) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Get tag */
  params[0] =
      OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, outtag, 16);

  if (!EVP_CIPHER_CTX_get_params(ctx, params)) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  std::string rta(reinterpret_cast<const char*>(outbuf), outlen);
  tag = std::string(reinterpret_cast<const char*>(outtag), TAG_LEN);

  /* Free memory */
  ReleaseMemory(outbuf, cipher, ctx);

  return rta;
}

auto OpenSslAdapter::Desencriptar(const std::string& str, std::string key,
                                  std::string tag) -> std::string {
  key = OpenSslAdapter::Hash256(key);

  EVP_CIPHER_CTX* ctx;
  EVP_CIPHER* cipher = nullptr;
  int outlen, rv;
  size_t gcm_ivlen = IV.size();
  auto* outbuf = new unsigned char[str.size()];
  OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

  if ((ctx = EVP_CIPHER_CTX_new()) == nullptr) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Fetch the cipher implementation */
  if ((cipher = EVP_CIPHER_fetch(libctx, PROTOCOLO, propq)) == NULL) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Set IV length if default 96 bits is not appropriate */
  params[0] =
      OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

  /*
   * Initialise an encrypt operation with the cipher/mode, key, IV and
   * IV length parameter.
   */
  if (!EVP_DecryptInit_ex2(
          ctx, cipher, reinterpret_cast<const unsigned char*>(key.c_str()),
          reinterpret_cast<const unsigned char*>(IV.c_str()), params)) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Zero or more calls to specify any AAD */
  if (!EVP_DecryptUpdate(ctx, nullptr, &outlen,
                         reinterpret_cast<const unsigned char*>(AAD.c_str()),
                         AAD.size())) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Decrypt plaintext */
  if (!EVP_DecryptUpdate(ctx, outbuf, &outlen,
                         reinterpret_cast<const unsigned char*>(str.c_str()),
                         str.size())) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  /* Set expected tag value. */
  params[0] = OSSL_PARAM_construct_octet_string(
      OSSL_CIPHER_PARAM_AEAD_TAG, reinterpret_cast<void*>(&tag[0]), tag.size());

  if (!EVP_CIPHER_CTX_set_params(ctx, params)) {
    ERR_print_errors_fp(stderr);
    ReleaseMemory(outbuf, cipher, ctx);
    return "";
  }

  std::string rta(reinterpret_cast<const char*>(outbuf), outlen);

  /* Finalise: note get no output for GCM */
  rv = EVP_DecryptFinal_ex(ctx, outbuf, &outlen);

  /* Free memory */
  ReleaseMemory(outbuf, cipher, ctx);

  return (rv <= 0) ? "" : rta;
}