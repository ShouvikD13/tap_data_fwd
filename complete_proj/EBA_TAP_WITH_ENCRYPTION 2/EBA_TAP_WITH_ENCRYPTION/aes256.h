#include <openssl/evp.h>

void encrypt_EVP_aes_256_gcm_init(EVP_CIPHER_CTX **ctx, unsigned char *key, unsigned char *iv);
void encrypt(EVP_CIPHER_CTX *ctx, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len);
void decrypt_EVP_aes_256_gcm_init(EVP_CIPHER_CTX **ctx, unsigned char *key, unsigned char *iv);
void decrypt(EVP_CIPHER_CTX *ctx, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len);
// void encryptText(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len);
// void decryptText(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, int *plaintext_len);
