#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <gmp.h>
#include <x86intrin.h>   // for __rdtsc
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sched.h>

#define ITER_PRIME_GEN 1000UL   /* set lower for development; change to 1000000 if you will run long */
#define MESSAGE_BITS 1023

/* read 64-bit seed from /dev/urandom */
static unsigned long long get_entropy_u64() {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        perror("open /dev/urandom");
        exit(1);
    }
    unsigned long long v;
    ssize_t r = read(fd, &v, sizeof(v));
    if (r != sizeof(v)) { perror("read"); close(fd); exit(1); }
    close(fd);
    return v;
}

/* pin process to CPU 0 to reduce core migration noise */
static void pin_to_cpu0() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    if (sched_setaffinity(0, sizeof(cpuset), &cpuset) != 0) {
        perror("sched_setaffinity");
        /* not fatal */
    }
}

/* helper: set top bit and low bit to ensure exact bit length and oddness */
void force_bitlength_and_odd(mpz_t x, unsigned int bits) {
    mpz_setbit(x, bits-1); // ensure MSB = 1
    mpz_setbit(x, 0);      // ensure odd
}

uint64_t rdtsc_now() {
    /* Use serializing rdtsc? Use __rdtsc() which is fine for this task.
       Optionally could use RDTSCP for serialization. */
    return __rdtsc();
}

void generate_random_prime(mpz_t out, gmp_randstate_t state, unsigned int bits) {
    mpz_t candidate;
    mpz_init(candidate);
    while (1) {
        mpz_urandomb(candidate, state, bits);
        force_bitlength_and_odd(candidate, bits);
        /* Option A: use mpz_probab_prime_p (repeat 25 checks for high confidence) */
        int reps = 25;
        int isprob = mpz_probab_prime_p(candidate, reps);
        if (isprob > 0) { // 1 = probably prime, 2 = definitely prime (rare)
            mpz_set(out, candidate);
            break;
        }
        /* else try again */
    }
    mpz_clear(candidate);
}

int main(int argc, char **argv) {
    pin_to_cpu0();

    unsigned long iterations = ITER_PRIME_GEN;
    if (argc >= 2) iterations = strtoul(argv[1], NULL, 10);

    unsigned int prime_bits_list[3] = {512, 768, 1024};
    size_t sets = 3;

    /* GMP random state */
    gmp_randstate_t rstate;
    gmp_randinit_default(rstate);
    unsigned long long seed = get_entropy_u64();
    gmp_randseed_ui(rstate, (unsigned long)(seed & 0xffffffffUL));

    printf("# Iterations per size: %lu\n", iterations);
    printf("# Fields: size,batch,step,iteration,cycles\n");

    for (size_t s = 0; s < sets; ++s) {
        unsigned int bits = prime_bits_list[s];

        /* arrays to keep min/max/total for prime generation (for p and q combined) */
        uint64_t min_cycles_p = UINT64_MAX, max_cycles_p = 0, sum_cycles_p = 0;
        uint64_t min_cycles_q = UINT64_MAX, max_cycles_q = 0, sum_cycles_q = 0;

        /* per-iteration loop */
        for (unsigned long i = 0; i < iterations; ++i) {
            mpz_t p, q;
            mpz_init(p);
            mpz_init(q);

            /* time p generation */
            uint64_t t0 = rdtsc_now();
            generate_random_prime(p, rstate, bits);
            uint64_t t1 = rdtsc_now();
            uint64_t cyc_p = t1 - t0;
            if (cyc_p < min_cycles_p) min_cycles_p = cyc_p;
            if (cyc_p > max_cycles_p) max_cycles_p = cyc_p;
            sum_cycles_p += cyc_p;

            /* time q generation */
            t0 = rdtsc_now();
            generate_random_prime(q, rstate, bits);
            t1 = rdtsc_now();
            uint64_t cyc_q = t1 - t0;
            if (cyc_q < min_cycles_q) min_cycles_q = cyc_q;
            if (cyc_q > max_cycles_q) max_cycles_q = cyc_q;
            sum_cycles_q += cyc_q;

            /* output raw iteration data (optional) */
            printf("%u,prime_gen,p,%lu,%" PRIu64 "\n", bits, i, cyc_p);
            printf("%u,prime_gen,q,%lu,%" PRIu64 "\n", bits, i, cyc_q);

            /* Step 2: compute N and phi, time it */
            mpz_t N, phi, tmp1, tmp2;
            mpz_init(N); mpz_init(phi); mpz_init(tmp1); mpz_init(tmp2);

            t0 = rdtsc_now();
            mpz_mul(N, p, q);                  // N = p * q
            mpz_sub_ui(tmp1, p, 1);            // tmp1 = p-1
            mpz_sub_ui(tmp2, q, 1);            // tmp2 = q-1
            mpz_mul(phi, tmp1, tmp2);          // phi = (p-1)*(q-1)
            t1 = rdtsc_now();
            uint64_t cyc_N_phi = t1 - t0;
            printf("%u,compute,N_phi,0,%" PRIu64 "\n", bits, cyc_N_phi);

            /* Step 3: compute d = invmod(e, phi) */
            mpz_t e, d;
            mpz_init_set_ui(e, 65537UL);
            mpz_init(d);

            t0 = rdtsc_now();
            if (mpz_invert(d, e, phi) == 0) {
                fprintf(stderr, "Error: e not invertible mod phi (rare). Re-generate primes.\n");
            }
            t1 = rdtsc_now();
            uint64_t cyc_d = t1 - t0;
            printf("%u,compute,d,0,%" PRIu64 "\n", bits, cyc_d);

            /* Step 4: message encryption/decryption (single trial per iteration) */
            mpz_t m, c, m2;
            mpz_init(m); mpz_init(c); mpz_init(m2);

            /* generate message less than N, 1023-bit as requested */
            mpz_urandomb(m, rstate, MESSAGE_BITS);
            /* ensure m < N */
            if (mpz_cmp(m, N) >= 0) {
                mpz_mod(m, m, N);
            }

            /* encrypt: c = m^e mod N */
            t0 = rdtsc_now();
            mpz_powm(c, m, e, N);
            t1 = rdtsc_now();
            uint64_t cyc_enc = t1 - t0;
            printf("%u,encrypt,enc,0,%" PRIu64 "\n", bits, cyc_enc);

            /* decrypt: m2 = c^d mod N */
            t0 = rdtsc_now();
            mpz_powm(m2, c, d, N);
            t1 = rdtsc_now();
            uint64_t cyc_dec = t1 - t0;
            printf("%u,encrypt,dec,0,%" PRIu64 "\n", bits, cyc_dec);

            if (mpz_cmp(m, m2) != 0) {
                fprintf(stderr, "Decryption mismatch on iteration %lu size %u!\n", i, bits);
            }

            /* clear for next iter */
            mpz_clear(p); mpz_clear(q);
            mpz_clear(N); mpz_clear(phi); mpz_clear(tmp1); mpz_clear(tmp2);
            mpz_clear(e); mpz_clear(d);
            mpz_clear(m); mpz_clear(c); mpz_clear(m2);
        }

        /* print summary statistics for p and q */
        double avg_p = (double)sum_cycles_p / (double)iterations;
        double avg_q = (double)sum_cycles_q / (double)iterations;
        printf("%u,summary,p,min,%" PRIu64 "\n", bits, min_cycles_p);
        printf("%u,summary,p,max,%" PRIu64 "\n", bits, max_cycles_p);
        printf("%u,summary,p,avg,%.2f\n", bits, avg_p);

        printf("%u,summary,q,min,%" PRIu64 "\n", bits, min_cycles_q);
        printf("%u,summary,q,max,%" PRIu64 "\n", bits, max_cycles_q);
        printf("%u,summary,q,avg,%.2f\n", bits, avg_q);
    }

    gmp_randclear(rstate);
    return 0;
}