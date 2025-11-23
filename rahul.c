// mr_experiment_a.c
#include <stdio.h>
#include <stdlib.h>
#include <gmp.h>
#include <time.h>
#include <stdint.h>

// Modular exponentiation: x = a^d mod n
void mod_exp(mpz_t result, const mpz_t a, const mpz_t d, const mpz_t n) {
    mpz_powm(result, a, d, n);
}

// One round of Miller-Rabin
int miller_rabin_round(const mpz_t n, const mpz_t d, unsigned int s) {
    gmp_randstate_t gmp_randstate;  // global RNG state (defined before use)
    mpz_t a, x;
    mpz_inits(a, x, NULL);

    // Pick random base a in [2, n-2]
    mpz_sub_ui(x, n, 3);           // x = n-3
    mpz_urandomm(a, gmp_randstate, x); 
    mpz_add_ui(a, a, 2);           // a = 2..n-2

    mod_exp(x, a, d, n);
    if (mpz_cmp_ui(x, 1) == 0 || mpz_cmp(x, n) == 0 || mpz_cmp_ui(x, mpz_get_ui(n)-1) == 0) {
        mpz_clears(a, x, NULL);
        return 1; // probably prime for this round
    }

    for (unsigned int r = 1; r < s; r++) {
        mpz_powm_ui(x, x, 2, n);
        if (mpz_cmp_ui(x, mpz_get_ui(n)-1) == 0) {
            mpz_clears(a, x, NULL);
            return 1;
        }
    }

    mpz_clears(a, x, NULL);
    return 0; // composite
}

// Miller-Rabin primality test
int miller_rabin(const mpz_t n, int k) {
    if (mpz_cmp_ui(n, 2) < 0) return 0;
    if (mpz_cmp_ui(n, 2) == 0 || mpz_cmp_ui(n, 3) == 0) return 1;
    if (mpz_even_p(n)) return 0;

    mpz_t d, n_minus1;
    mpz_inits(d, n_minus1, NULL);

    mpz_sub_ui(n_minus1, n, 1); // n-1
    mpz_set(d, n_minus1);
    unsigned int s = 0;
    while (mpz_even_p(d)) {
        mpz_divexact_ui(d, d, 2);
        s++;
    }

    for (int i = 0; i < k; i++) {
        if (!miller_rabin_round(n, d, s)) {
            mpz_clears(d, n_minus1, NULL);
            return 0; // definitely composite
        }
    }

    mpz_clears(d, n_minus1, NULL);
    return 1; // probably prime
}

// Generate random prime of given bits
void generate_prime(mpz_t prime, unsigned int bits, int k) {
    gmp_randstate_t gmp_randstate;  // global RNG state (defined before use)
    do {
        mpz_urandomb(prime, gmp_randstate, bits);
        mpz_setbit(prime, bits - 1); // set MSB
        mpz_setbit(prime, 0);        // make odd
    } while (!miller_rabin(prime, k));
}

// Global GMP random state
gmp_randstate_t gmp_randstate;

// Main
int main() {
    gmp_randinit_default(gmp_randstate);
    gmp_randseed_ui(gmp_randstate, time(NULL));

    mpz_t p, q, n;
    mpz_inits(p, q, n, NULL);

    printf("Generating 256-bit primes...\n");
    clock_t start = clock();
    generate_prime(p, 256, 20);
    generate_prime(q, 256, 20);
    clock_t end = clock();

    mpz_mul(n, p, q);
    printf("Prime generation time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);

    gmp_printf("p = %Zd\nq = %Zd\nn = %Zd (512-bit composite)\n", p, q, n);

    int trials = 100000, lies = 0;
    printf("Running %d Miller-Rabin trials...\n", trials);

    mpz_t d, n_minus1;
    mpz_inits(d, n_minus1, NULL);
    mpz_sub_ui(n_minus1, n, 1);
    mpz_set(d, n_minus1);
    unsigned int s = 0;
    while (mpz_even_p(d)) { mpz_divexact_ui(d, d, 2); s++; }

    for (int i = 0; i < trials; i++) {
        if (miller_rabin_round(n, d, s)) lies++;
    }

    printf("False acceptances: %d / %d (%.6f%%)\n", lies, trials, 100.0 * lies / trials);

    mpz_clears(p, q, n, d, n_minus1, NULL);
    gmp_randclear(gmp_randstate);
    return 0;
}