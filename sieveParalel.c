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


// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | čas            | št niti = 1 | št niti = 2   | št niti = 4   | št niti = 8   | št niti = 16  | št niti = 32 |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | meritev 1      | "11,9"      | "6,17"        | "3,14"        | "1,77"        | "0,825"       | "0,589"      |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | meritev 2      | "12,5"      | "6,31"        | "3,04"        | "1,5"         | "0,92"        | "0,63"       |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | meritev 3      | "10,51"     | "5,5"         | "2,78"        | "1,39"        | "0,78"        | "0,56"       |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | meritev 4      | "11,93"     | "6,22"        | "3,13"        | "1,55"        | "0,9"         | "0,67"       |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | meritev 5      | "11,9"      | "6,18"        | "3,12"        | "1,57"        | "0,8"         | "0,6"        |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | avrage:        | "11,748"    | "6,076"       | "3,042"       | "1,556"       | "0,845"       | "0,6098"     |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | Posp. (serial):| /           | "1,933508887" | "3,861932939" | "7,550128535" | "13,90295858" | "19,2653329" |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | Posp. (prejš): | /           | "1,933508887" | "1,997370151" | "1,955012853" | "1,841420118" | "1,38570023" |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// | Posp. (naive): | "1,4300306" | "2,764976959" | "5,522682446" | "10,79691517" | "19,88165680" | "27,5500164" |
// +----------------+-------------+---------------+---------------+---------------+---------------+--------------+
// Posp. (serial) = pospešitev glede na serijski način (1 nit) deluje v snopih
// Posp. (prejš) = pospešitev glede na prejšnje število niti
// Posp. (naive) = pospešitev glede na naive implementacijo (1 nit)
/*
    The code significantly improves the performance when the number of threads is increased. 
    It also addreses the shortcomings of the naive implementation of the Sieve of Eratosthenes. 
    The cache-misses go down to less than 0.6% of the total memory accesses in all cases.

    Here is an example: 
        ############# ST NITI 32 #############
    Total primes less or equal to 1000000000: 50847534
    Elapsed time: 0.606687

    Performance counter stats for './sievepar 1000000000':

        881,884,160      cache-references:u                                            (71.28%)
            6,940,450      cache-misses:u            #    0.787 % of all cache refs      (71.41%)
        37,863,601,686      cycles:u                                                      (28.74%)
        8,200,681,845      stalled-cycles-backend:u  #   21.66% backend cycles idle      (28.66%)
        27,382,493,426      instructions:u            #    0.72  insn per cycle         
                                                    #    0.30  stalled cycles per insn  (42.92%)
        6,468,499,272      branches:u                                                    (57.11%)
        153,546,744      branch-misses:u           #    2.37% of all branches          (71.32%)

        0.928010958 seconds time elapsed

        16.506594000 seconds user
        0.222073000 seconds sys
*/