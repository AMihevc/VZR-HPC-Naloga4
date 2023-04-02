#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <omp.h>
#include <string.h>

void print_is_prime(char* is_prime, int N) {
  printf("is_prime:\n");
  for (int i = 0; i <= N; i++) {
    printf("%d: %d\n", i, is_prime[i]);
  }
}


int main(int argc, char** argv) {

    //check usage
    if (argc < 2) {
        printf("Usage: ./primes N\n");
        return 1;
    }

    const int n = atoi(argv[1]);

    //allocate memory for array of primes
    unsigned char * primes = (unsigned char *)malloc((n+1) * sizeof(unsigned char));

    //check if memory was allocated and initialize array
    if (!primes) exit(1);
    memset(primes, 1, (n+1));

    int block_zero_size = sqrt(n);
    int block_size = 30000; 

    //start time
    double start = omp_get_wtime();

    // najprej zračunaš primes v prvem segmentu od 2 do sqrt(N)
    for (int p = 2; p * p <= n; p++) {
        //printf("p: %d", p);
        if (primes[p]) {
            // za vsako število p, ki je praštevilo, nastavi vse njegove večkratnike na 0
            for (int i = p * p; i*i <= n; i += p) {
                primes[i] = 0;
            }
        }
    }

    //go through the rest of the blocks
    #pragma omp parallel for schedule(guided)
    for (int j = block_zero_size + 1; j <= n; j += block_size) {
        //calculate primes from p to p + block_size - 
        int block_start = j;
        int block_end = block_start + block_size - 1;
        if (block_end > n) block_end = n;

        //printf("j: %d, block_start: %d, block_end: %d\n", j, block_start, block_end);

        //go through the primes in the first block
        for (int prime = 2; prime * prime <= n; prime++) {
            //if it is a prime, go through all the multiples of p in the current block
            if (primes[prime]) {
                for (int i = fmax(prime * prime, (block_start + prime - 1) / prime * prime); i <= block_end; i += prime) {
                    primes[i] = 0;
                    //printf("p: %d, i: %d\n", p, i);
                }
            }
        }
    }


    #pragma omp barrier
    //na koncu še preštejemo število praštevil
    unsigned int totalPrimes=0;
    #pragma omp parallel for reduction(+:totalPrimes)
    for (int p = 2; p <= n; p++) 
        if (primes[p])
            totalPrimes++;
    
    //end time
    double end = omp_get_wtime();
    
    // for debugging
    //print_is_prime(primes, n);

    
    if (n == 1000000000 & totalPrimes != 50847534){
        printf("WRONG NUMBER OF PRIMES @%d threds: %d ",omp_get_num_threads(),  totalPrimes);
    }
    printf("Total primes less or equal to %d: %d\n",n,totalPrimes);
    printf("Elapsed time: %f\n",end-start);
    
    
    //clean up
    free(primes);
    //free(primes_in_block_zero);

    return 0;
}
