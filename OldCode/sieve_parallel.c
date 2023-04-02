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

    if (argc < 2) {
        printf("Usage: ./primes N\n");
        return 1;
    }

    const int n = atoi(argv[1]);
    unsigned char * primes = (unsigned char *)malloc((n+1) * sizeof(unsigned char));
    //printf("tuki sm ");
    if (!primes) exit(1);
    memset(primes, 1, (n+1));

    //printf("tuki sm ");

    //start time 
    double start = omp_get_wtime();  
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        int block_zero_size = sqrt(n);
        int block_size = (n - block_zero_size) / num_threads;
        int block_start =(block_zero_size) + (thread_id * block_size);
        int block_end = block_start + block_size;
        if (block_end > n) block_end = n;
        // if i am the last thread, i have to take care of the rest of the numbers
        if (thread_id == num_threads - 1) block_end = n;
        //printf("thread_id: %d, num_threads: %d, block_zero_size: %d, block_size: %d, block_start: %d, block_end: %d\n", thread_id, num_threads, block_zero_size, block_size, block_start, block_end); 
        
        // nit 0 obdela prvih sqrt(N) števil 
        if (thread_id == 0){
            // nit nič obdela števila od 2 do sqrt(N)
            for (int p = 2; p * p <= n; p++) {
                //printf("p: %d", p);
                if (primes[p]) {
                    // za vsako število p, ki je praštevilo, nastavi vse njegove večkratnike na 0
                    for (int i = p * p; i*i <= n; i += p) {
                        primes[i] = 0;
                    }
                }
            }

        }
        #pragma omp barrier
        // ko nit nic zakljuci lahko obdelamo se ostale odseke od sqrt(N) do N

        //pogledaš v prvem segmentu keri so primes
        for (int p = 2; p <= (int)block_zero_size; p++) {
            if(primes[p]){
                //če je prime potem pogledaš vse večkratnike v svojem segmentu
                for(int i = fmax(p * p, (block_start + p - 1) / p * p); i <= block_end; i += p){
                    primes[i] = 0;
                }
            }
        }
        
    }


    //na koncu še preštejemo število praštevil
    unsigned int totalPrimes=0;
    #pragma omp parallel for reduction(+:totalPrimes)
    for (int p = 2; p <= n; p++) 
        if (primes[p])
            totalPrimes++;
    
    //end time
    double end = omp_get_wtime();
    if (n == 1000000000 & totalPrimes != 50847534){
        printf("WRONG NUMBER OF PRIMES @%d threds: %d ",omp_get_num_threads(),  totalPrimes);
    }
    //printf("Total primes less or equal to %d: %d\n",n,totalPrimes);
    printf("Elapsed time: %f\n",end-start);
    
    //print_is_prime(primes, n);

    free(primes);
    return 0;
}
