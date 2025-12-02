#include <stdio.h>
#include <time.h>

int main() {
    clock_t start, end;
    volatile long long sum = 0;

    start = clock();

    for (long long i = 0; i < 100000000; i++) {
        sum += i;
    }

    end = clock();

    double time_taken = (double)(end - start) / CLOCKS_PER_SEC;

    printf("Sum: %lld\n", sum);
    printf("Time taken: %f seconds\n", time_taken);

    return 0;
}