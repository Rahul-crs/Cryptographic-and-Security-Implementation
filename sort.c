#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void insertion_sort(int arr[], int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j = i - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            j--;
        }
        arr[j + 1] = key;
    }
}

int compare_ints(const void *a, const void *b) {
    int arg1 = *(const int *)a;
    int arg2 = *(const int *)b;
    return (arg1 > arg2) - (arg1 < arg2);
}

int main() {
    int n = 100000;
    int *arr1 = malloc(n * sizeof(int));
    int *arr2 = malloc(n * sizeof(int));

    if (arr1 == NULL || arr2 == NULL) {
        printf("Memory allocation failed\n");
        return 1;
    }

    srand(time(NULL));
    for (int i = 0; i < n; i++) {
        int val = rand();
        arr1[i] = val;
        arr2[i] = val;
    }

    clock_t start_ins = clock();
    insertion_sort(arr1, n);
    clock_t end_ins = clock();

    clock_t start_qs = clock();
    qsort(arr2, n, sizeof(int), compare_ints);
    clock_t end_qs = clock();

    double time_ins = (double)(end_ins - start_ins) / CLOCKS_PER_SEC;
    double time_qs = (double)(end_qs - start_qs) / CLOCKS_PER_SEC;

    printf("Insertion sort time: %f seconds\n", time_ins);
    printf("qsort time: %f seconds\n", time_qs);

    free(arr1);
    free(arr2);

    return 0;
}