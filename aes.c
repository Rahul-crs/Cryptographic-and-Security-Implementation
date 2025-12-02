#include <stdio.h>
#include <string.h>
#include <openssl/aes.h>
#include <time.h>
#include <stdlib.h>

void aes_ecb_encrypt(const unsigned char *plaintext, unsigned char *ciphertext, const unsigned char *key, int size) {
    AES_KEY enc_key;
    AES_set_encrypt_key(key, 128, &enc_key);

    for (int i = 0; i < size; i += AES_BLOCK_SIZE) {
        AES_ecb_encrypt(plaintext + i, ciphertext + i, &enc_key, AES_ENCRYPT);
    }
}

void aes_ecb_decrypt(const unsigned char *ciphertext, unsigned char *plaintext, const unsigned char *key, int size) {
    AES_KEY dec_key;
    AES_set_decrypt_key(key, 128, &dec_key);

    for (int i = 0; i < size; i += AES_BLOCK_SIZE) {
        AES_ecb_encrypt(ciphertext + i, plaintext + i, &dec_key, AES_DECRYPT);
    }
}

int main() {
    unsigned char key[16] = "thisisakey123456";

    int size = 1024;
    unsigned char *plaintext = malloc(size);
    unsigned char *ciphertext = malloc(size);
    unsigned char *decryptedtext = malloc(size);

    if (!plaintext || !ciphertext || !decryptedtext) {
        printf("Memory allocation failed\n");
        return 1;
    }

    memset(plaintext, 'A', size);

    clock_t start_enc = clock();
    aes_ecb_encrypt(plaintext, ciphertext, key, size);
    clock_t end_enc = clock();

    clock_t start_dec = clock();
    aes_ecb_decrypt(ciphertext, decryptedtext, key, size);
    clock_t end_dec = clock();

    double enc_time = (double)(end_enc - start_enc) / CLOCKS_PER_SEC;
    double dec_time = (double)(end_dec - start_dec) / CLOCKS_PER_SEC;

    printf("AES ECB encryption time: %f seconds\n", enc_time);
    printf("AES ECB decryption time: %f seconds\n", dec_time);

    if (memcmp(plaintext, decryptedtext, size) == 0) {
        printf("Decryption successful, plaintext matches.\n");
    } else {
        printf("Decryption failed, plaintext does not match.\n");
    }

    free(plaintext);
    free(ciphertext);
    free(decryptedtext);

    return 0;
}