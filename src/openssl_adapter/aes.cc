#include "openssl_.hh"

//#pragma comment(lib, "crypt32")
//#pragma comment(lib, "Ws2_32")
#include <openssl/core_names.h>
#include <openssl/err.h>
#include <openssl/evp.h>

using std::string;

static string IV = OpenSSL_::Hash256_x(
    "Este es un vector de inicializacion super ultra mega secreto", 12);
static string ADD = "Esto es algo totalmente innecesario";
const static size_t TAG_LEN = 16;
const static char *PROTOCOLO = "AES-256-GCM";

/*
 * A library context and property query can be used to select & filter
 * algorithm implementations. If they are NULL then the default library
 * context and properties are used.
 */
static OSSL_LIB_CTX *libctx = nullptr;
static const char *propq = nullptr;

auto OpenSSL_::Encriptar(string str, string key, string &tag) -> string {
  bool error = true;

  key = OpenSSL_::Hash256(key);

  EVP_CIPHER_CTX *ctx;
  EVP_CIPHER *cipher = NULL;
  int outlen, tmplen;
  size_t gcm_ivlen = IV.size();
  auto *outbuf = new unsigned char[str.size()];
  unsigned char outtag[TAG_LEN];
  OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

  /* Create a context for the encrypt operation */
  if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
    goto err;

  /* Fetch the cipher implementation */
  if ((cipher = EVP_CIPHER_fetch(libctx, PROTOCOLO, propq)) == NULL)
    goto err;

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
  if (!EVP_EncryptInit_ex2(ctx, cipher,
                           reinterpret_cast<unsigned char *>(&key[0]),
                           reinterpret_cast<unsigned char *>(&IV[0]), params))
    goto err;

  /* Zero or more calls to specify any AAD */
  if (!EVP_EncryptUpdate(ctx, NULL, &outlen,
                         reinterpret_cast<unsigned char *>(&ADD[0]),
                         ADD.size()))
    goto err;

  /* Encrypt plaintext */
  if (!EVP_EncryptUpdate(ctx, outbuf, &outlen,
                         reinterpret_cast<unsigned char *>(&str[0]),
                         str.size()))
    goto err;

  /* Finalise: note get no output for GCM */
  if (!EVP_EncryptFinal_ex(ctx, outbuf, &tmplen))
    goto err;

  /* Get tag */
  params[0] =
      OSSL_PARAM_construct_octet_string(OSSL_CIPHER_PARAM_AEAD_TAG, outtag, 16);

  if (!EVP_CIPHER_CTX_get_params(ctx, params))
    goto err;

  error = false;

err:
  if (error)
    ERR_print_errors_fp(stderr);
  else {
    str = string(reinterpret_cast<const char *>(outbuf), outlen);
    tag = string(reinterpret_cast<const char *>(outtag), TAG_LEN);
  }

  /* Free memory */
  delete[] outbuf;
  EVP_CIPHER_free(cipher);
  EVP_CIPHER_CTX_free(ctx);

  return error ? "" : str;
}

auto OpenSSL_::Desencriptar(string str, string key, string tag) -> string {
  bool error = true;

  key = OpenSSL_::Hash256(key);

  EVP_CIPHER_CTX *ctx;
  EVP_CIPHER *cipher = NULL;
  int outlen, rv;
  size_t gcm_ivlen = IV.size();
  unsigned char *outbuf = new unsigned char[str.size()];
  OSSL_PARAM params[2] = {OSSL_PARAM_END, OSSL_PARAM_END};

  if ((ctx = EVP_CIPHER_CTX_new()) == NULL)
    goto err;

  /* Fetch the cipher implementation */
  if ((cipher = EVP_CIPHER_fetch(libctx, PROTOCOLO, propq)) == NULL)
    goto err;

  /* Set IV length if default 96 bits is not appropriate */
  params[0] =
      OSSL_PARAM_construct_size_t(OSSL_CIPHER_PARAM_AEAD_IVLEN, &gcm_ivlen);

  /*
   * Initialise an encrypt operation with the cipher/mode, key, IV and
   * IV length parameter.
   */
  if (!EVP_DecryptInit_ex2(ctx, cipher,
                           reinterpret_cast<unsigned char *>(&key[0]),
                           reinterpret_cast<unsigned char *>(&IV[0]), params))
    goto err;

  /* Zero or more calls to specify any AAD */
  if (!EVP_DecryptUpdate(ctx, NULL, &outlen,
                         reinterpret_cast<unsigned char *>(&ADD[0]),
                         ADD.size()))
    goto err;

  /* Decrypt plaintext */
  if (!EVP_DecryptUpdate(ctx, outbuf, &outlen,
                         reinterpret_cast<unsigned char *>(&str[0]),
                         str.size()))
    goto err;

  /* Set expected tag value. */
  params[0] = OSSL_PARAM_construct_octet_string(
      OSSL_CIPHER_PARAM_AEAD_TAG,
      (void *)reinterpret_cast<unsigned char *>(&tag[0]), tag.size());

  if (!EVP_CIPHER_CTX_set_params(ctx, params))
    goto err;

  str = string(reinterpret_cast<const char *>(outbuf), outlen);

  /* Finalise: note get no output for GCM */
  rv = EVP_DecryptFinal_ex(ctx, outbuf, &outlen);

  error = false;

err:
  if (error)
    ERR_print_errors_fp(stderr);

  /* Free memory */
  delete[] outbuf;
  EVP_CIPHER_free(cipher);
  EVP_CIPHER_CTX_free(ctx);

  return (error || rv <= 0) ? "" : str;
}