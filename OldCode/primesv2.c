#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>

void print_is_prime(char* is_prime, int N) {
  printf("is_prime:\n");
  for (int i = 0; i <= N; i++) {
    printf("%d: %d\n", i, is_prime[i]);
  }
}

int main(int argc, char** argv) {

    if (argc < 2) {
        printf("Usage: ./primes N\n");
        exit(1);
    }

    const int N = atoi(argv[1]);
    char* is_prime = (char*) malloc((N+1) * sizeof(char));

    #pragma omp parallel
    {

        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        int block_size = sqrt(N);
        int block_start = thread_id * block_size;
        int block_end = (thread_id + 1) * block_size - 1;
        if (block_end > N) block_end = N;
        
        #pragma omp single
        {
            is_prime[0] = 0;
            is_prime[1] = 0;
            for (int i = 2; i <= N; i++) {
                is_prime[i] = 1;
            }
        }
        print_is_prime(is_prime, N);

        for (int p = 2; p * p <= N; p++) {
            if (is_prime[p]) {
                for (int i = fmax(p * p, (block_start + p - 1) / p * p); i <= block_end; i += p) {
                    is_prime[i] = 0;
                }
            }
        }
    }

    #pragma omp barrier
    int num_primes = 0;
    print_is_prime(is_prime, N);
    for (int i = 2; i <= N; i++) {
        if (is_prime[i]) {
            num_primes++;
            }
    }

    printf("Number of primes <= %d: %d\n", N, num_primes);
    free(is_prime);
    return 0;
}
