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
        exit(1);
    }

    const int n = atoi(argv[1]);
    unsigned char * primes = (unsigned char *)malloc((n+1) * sizeof(unsigned char));
    //printf("tuki sm ");
    if (!primes) exit(1);
    memset(primes, 1, (n+1));

    //printf("tuki sm ");
  
    #pragma omp parallel
    {
        int thread_id = omp_get_thread_num();
        int num_threads = omp_get_num_threads();
        int block_zero_size = sqrt(n);
        int block_size = (n - block_zero_size) / num_threads;
        int block_start =(block_zero_size -1) + (thread_id * block_size);
        int block_end = block_start + block_size -1;
        if (block_end > n) block_end = n;   
        
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
        #pragma omp for
        for (int p = 2; p <= (int)block_zero_size; p++) {
            if(primes[p]){
                //če je prime potem pogledaš vse večkratnike v svojem segmentu
                for(int i = fmax(p * p, (block_start + p - 1) / p * p); i <= block_end; i += p){
                    primes[i] = 0;
                }
            }
        }
        
    }

    #pragma omp barrier
    //na koncu še preštejemo število praštevil
    print_is_prime(primes, n);
    int num_primes = 0;
    for (int i = 2; i <= n; i++) {
        if (primes[i]) {
        num_primes++;
        }
    }



    printf("Number of primes <= %d: %d\n", n, num_primes);
    free(primes);
    return 0;
}
