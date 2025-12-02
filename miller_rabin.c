#include <stdio.h>
#include <gmp.h>
#include <time.h>
#include <stdlib.h>

void miller_rabin(mpz_t n, int iterations, int *is_probably_prime) {
    if (mpz_cmp_ui(n, 2) < 0) {
        *is_probably_prime = 0;
        return;
    }

    if (mpz_cmp_ui(n, 2) == 0) {
        *is_probably_prime = 1;
        return;
    }

    if (mpz_even_p(n)) {
        *is_probably_prime = 0;
        return;
    }

    mpz_t d, a, x, n_minus_1, temp;
    mpz_inits(d, a, x, n_minus_1, temp, NULL);

    mpz_sub_ui(n_minus_1, n, 1);

    unsigned long r = 0;
    mpz_set(d, n_minus_1);
    while (mpz_even_p(d)) {
        mpz_div_ui(d, d, 2);
        r++;
    }

    gmp_randstate_t state;
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(NULL));

    for (int i = 0; i < iterations; i++) {
        mpz_urandomm(a, state, n_minus_1);
        mpz_add_ui(a, a, 2);

        mpz_powm(x, a, d, n);

        if (mpz_cmp_ui(x, 1) == 0 || mpz_cmp(x, n_minus_1) == 0) {
            continue;
        }

        int continue_loop = 0;
        for (unsigned long j = 0; j < r - 1; j++) {
            mpz_powm_ui(x, x, 2, n);

            if (mpz_cmp(x, n_minus_1) == 0) {
                continue_loop = 1;
                break;
            }
        }

        if (continue_loop) {
            continue;
        }

        *is_probably_prime = 0;
        mpz_clears(d, a, x, n_minus_1, temp, NULL);
        gmp_randclear(state);
        return;
    }

    *is_probably_prime = 1;

    mpz_clears(d, a, x, n_minus_1, temp, NULL);
    gmp_randclear(state);
}

int main() {
    mpz_t n;
    mpz_init(n);

    gmp_randstate_t state;
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(NULL));

    unsigned long bits = 1024;
    int iterations = 10;
    int is_probably_prime;

    mpz_urandomb(n, state, bits);
    mpz_setbit(n, bits - 1);
    mpz_nextprime(n, n);

    clock_t start = clock();
    for (int i = 0; i < 1000; i++) {
        miller_rabin(n, iterations, &is_probably_prime);
    }
    clock_t end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    gmp_printf("Number tested: %Zd\n", n);
    printf("Miller-Rabin iterations: %d\n", iterations);
    printf("Is probably prime: %s\n", is_probably_prime ? "Yes" : "No");
    printf("Total time for 1000 iterations: %f seconds\n", time_taken);

    mpz_clear(n);
    gmp_randclear(state);

    return 0;
}