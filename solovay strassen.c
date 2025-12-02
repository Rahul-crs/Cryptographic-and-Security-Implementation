#include <stdio.h>
#include <gmp.h>
#include <time.h>
#include <stdlib.h>

void solovay_strassen(mpz_t n, int iterations, int *is_probably_prime) {
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

    mpz_t a, x, n_minus_1, temp;
    mpz_inits(a, x, n_minus_1, temp, NULL);

    mpz_sub_ui(n_minus_1, n, 1);

    gmp_randstate_t state;
    gmp_randinit_mt(state);
    gmp_randseed_ui(state, time(NULL));

    for (int i = 0; i < iterations; i++) {
        mpz_urandomm(a, state, n_minus_1);
        mpz_add_ui(a, a, 1);

        mpz_powm(x, a, n_minus_1, n);
        if (mpz_cmp_ui(x, 1) != 0) {
            *is_probably_prime = 0;
            mpz_clears(a, x, n_minus_1, temp, NULL);
            gmp_randclear(state);
            return;
        }

        int jacobi = mpz_jacobi(a, n);

        if (jacobi == 0) {
            *is_probably_prime = 0;
            mpz_clears(a, x, n_minus_1, temp, NULL);
            gmp_randclear(state);
            return;
        }

        if (jacobi == -1) {
            mpz_sub(x, n, x);
        }

        mpz_mod(x, x, n);
        if (mpz_cmp_ui(x, 1) != 0) {
            *is_probably_prime = 0;
            mpz_clears(a, x, n_minus_1, temp, NULL);
            gmp_randclear(state);
            return;
        }
    }

    *is_probably_prime = 1;

    mpz_clears(a, x, n_minus_1, temp, NULL);
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
        solovay_strassen(n, iterations, &is_probably_prime);
    }
    clock_t end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    gmp_printf("Number tested: %Zd\n", n);
    printf("Solovay-Strassen iterations: %d\n", iterations);
    printf("Is probably prime: %s\n", is_probably_prime ? "Yes" : "No");
    printf("Total time for 1000 iterations: %f seconds\n", time_taken);

    mpz_clear(n);
    gmp_randclear(state);

    return 0;
}