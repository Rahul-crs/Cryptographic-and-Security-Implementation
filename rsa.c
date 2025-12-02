#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>

void generate_rsa_keys(mpz_t n, mpz_t e, mpz_t d, gmp_randstate_t state, unsigned long int bits) {
    mpz_t p, q, phi, gcd;
    mpz_inits(p, q, phi, gcd, NULL);

    mpz_urandomb(p, state, bits / 2);
    mpz_nextprime(p, p);

    mpz_urandomb(q, state, bits / 2);
    mpz_nextprime(q, q);

    mpz_mul(n, p, q);

    mpz_t p_minus_1, q_minus_1;
    mpz_inits(p_minus_1, q_minus_1, NULL);
    mpz_sub_ui(p_minus_1, p, 1);
    mpz_sub_ui(q_minus_1, q, 1);

    mpz_mul(phi, p_minus_1, q_minus_1);

    mpz_set_ui(e, 65537);

    mpz_gcd(gcd, e, phi);
    while (mpz_cmp_ui(gcd, 1) != 0) {
        mpz_add_ui(e, e, 2);
        mpz_gcd(gcd, e, phi);
    }

    mpz_invert(d, e, phi);

    mpz_clears(p, q, phi, gcd, p_minus_1, q_minus_1, NULL);
}

void rsa_encrypt(mpz_t ciphertext, const mpz_t plaintext, const mpz_t e, const mpz_t n) {
    mpz_powm(ciphertext, plaintext, e, n);
}

void rsa_decrypt(mpz_t plaintext, const mpz_t ciphertext, const mpz_t d, const mpz_t n) {
    mpz_powm(plaintext, ciphertext, d, n);
}

int main() {
    mpz_t n, e, d, plaintext, ciphertext, decrypted;
    mpz_inits(n, e, d, plaintext, ciphertext, decrypted, NULL);

    gmp_randstate_t state;
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(NULL));

    unsigned long int bits = 2048;

    generate_rsa_keys(n, e, d, state, bits);

    mpz_set_ui(plaintext, 123456789);

    clock_t start_enc = clock();
    rsa_encrypt(ciphertext, plaintext, e, n);
    clock_t end_enc = clock();

    clock_t start_dec = clock();
    rsa_decrypt(decrypted, ciphertext, d, n);
    clock_t end_dec = clock();

    double enc_time = (double)(end_enc - start_enc) / CLOCKS_PER_SEC;
    double dec_time = (double)(end_dec - start_dec) / CLOCKS_PER_SEC;

    gmp_printf("Plaintext: %Zd\n", plaintext);
    gmp_printf("Ciphertext: %Zd\n", ciphertext);
    gmp_printf("Decrypted: %Zd\n", decrypted);
    printf("Encryption time: %f seconds\n", enc_time);
    printf("Decryption time: %f seconds\n", dec_time);

    mpz_clears(n, e, d, plaintext, ciphertext, decrypted, NULL);
    gmp_randclear(state);

    return 0;
}