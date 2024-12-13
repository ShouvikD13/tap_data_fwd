#include <iostream>
#include <fstream>
#include "aes256.h"

using namespace std;

unsigned char key[32];
unsigned char iv[12];

string test_key;
string test_iv;

// void readConfig()
// {
//     ifstream configFile("SECRET.ini");
//     if (!configFile.is_open())
//     {
//         // return false;
//     }

//     string line;
//     while (getline(configFile, line))
//     {
//         if (line.find("[AES 256 KEY]") != string::npos)
//         {
//             while (getline(configFile, line))
//             {
//                 if (line.find("KEY") != string::npos)
//                 {
//                     test_key = line.substr(line.find("=") + 1);
//                     //test_key.erase(remove(test_key.begin(), test_key.end(), ' '), test_key.end());
//                 }
//                 else if (line.find("IV") != string::npos)
//                 {
//                     test_iv = line.substr(line.find("=") + 1);
//                     //test_iv.erase(remove(test_iv.begin(), test_iv.end(), ' '), test_iv.end());
//                     break;
//                 }
//             }
//             break;
//         }
//     }

//     configFile.close();
//     // return true;
// }

void encrypt_EVP_aes_256_gcm_init(EVP_CIPHER_CTX **ctx, unsigned char *key, unsigned char *iv)
{
    if (!(*ctx = EVP_CIPHER_CTX_new()))
        std::cout << "Unable to create a new context for encryption!\n";
    if (EVP_EncryptInit_ex(*ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1)
        std::cout << "Unable to initialize the encryption using AES 256 GCM cipher\n";
}

void encrypt(EVP_CIPHER_CTX *ctx, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len)
{
    int len;
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1)
        std::cout << "Unable to encrypt the message\n";
    else
    {
        *ciphertext_len = len;
    }
}

void decrypt_EVP_aes_256_gcm_init(EVP_CIPHER_CTX **ctx, unsigned char *key, unsigned char *iv)
{
    if (!(*ctx = EVP_CIPHER_CTX_new()))
        std::cout << "Unable to create a new context for decryption!\n";
    if (EVP_DecryptInit_ex(*ctx, EVP_aes_256_gcm(), NULL, key, iv) != 1)
        std::cout << "Unable to initialize the decryption using AES 256 GCM cipher\n";
}

void decrypt(EVP_CIPHER_CTX *ctx, unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len)
{
    int len;
    if (EVP_DecryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1)
        std::cout << "Unable to decrypt the message\n";
    else
        *ciphertext_len = len;
}

// void encryptText(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len)
// {
//     readConfig();
//     for (int i = 0; i < 32; i++)
//         key[i] = static_cast<unsigned char>(test_key[i]);
//     for (int i = 0; i < 12; i++)
//         iv[i] = static_cast<unsigned char>(test_iv[i]);
//     EVP_CIPHER_CTX *ctx;
//     encrypt_EVP_aes_256_gcm_init(&ctx, key, iv);
//     encrypt(ctx, plaintext, plaintext_len, ciphertext, ciphertext_len);
//     EVP_CIPHER_CTX_free(ctx);
// }

// void decryptText(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, int *plaintext_len)
// {
//     readConfig();
//     for (int i = 0; i < 32; i++)
//         key[i] = static_cast<unsigned char>(test_key[i]);
//     for (int i = 0; i < 12; i++)
//         iv[i] = static_cast<unsigned char>(test_iv[i]);
//     EVP_CIPHER_CTX *ctx;
//     decrypt_EVP_aes_256_gcm_init(&ctx, key, iv);
//     decrypt(ctx, ciphertext, ciphertext_len, plaintext, plaintext_len);
//     EVP_CIPHER_CTX_free(ctx);
// }
