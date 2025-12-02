#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

void ksa(uint8_t *key, uint8_t *S, size_t keylen) {
    for (int i = 0; i < 256; i++) {
        S[i] = i;
    }

    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + S[i] + key[i % keylen]) % 256;
        uint8_t temp = S[i];
        S[i] = S[j];
        S[j] = temp;
    }
}

void prga(uint8_t *S, uint8_t *data, size_t datalen) {
    int i = 0, j = 0;
    for (size_t k = 0; k < datalen; k++) {
        i = (i + 1) % 256;
        j = (j + S[i]) % 256;
        uint8_t temp = S[i];
        S[i] = S[j];
        S[j] = temp;
        uint8_t K = S[(S[i] + S[j]) % 256];
        data[k] ^= K;
    }
}

int main() {
    uint8_t key[] = "secretkey";
    uint8_t S[256];

    uint8_t plaintext[1024];
    memset(plaintext, 'A', sizeof(plaintext));

    uint8_t ciphertext[1024];
    memcpy(ciphertext, plaintext, sizeof(plaintext));

    ksa(key, S, strlen((char *)key));

    clock_t start = clock();
    prga(S, ciphertext, sizeof(ciphertext));
    clock_t end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("RC4 encryption time: %f seconds\n", time_taken);

    return 0;
}