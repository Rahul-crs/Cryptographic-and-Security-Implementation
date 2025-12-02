#include <sodium.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

void chacha_encrypt(const unsigned char *plaintext, unsigned long long plaintext_len,
                    unsigned char *ciphertext, const unsigned char *key, const unsigned char *nonce) {
    crypto_stream_chacha20_xor(ciphertext, plaintext, plaintext_len, nonce, key);
}

int main() {
    if (sodium_init() < 0) {
        return 1;
    }

    unsigned char key[crypto_stream_chacha20_KEYBYTES];
    unsigned char nonce[crypto_stream_chacha20_NONCEBYTES];
    unsigned char plaintext[1024];
    unsigned char ciphertext[1024];

    memset(plaintext, 'A', sizeof(plaintext));
    randombytes_buf(key, sizeof(key));
    randombytes_buf(nonce, sizeof(nonce));

    clock_t start = clock();
    chacha_encrypt(plaintext, sizeof(plaintext), ciphertext, key, nonce);
    clock_t end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;
    printf("ChaCha20 encryption time: %f seconds\n", time_taken);

    return 0;
}