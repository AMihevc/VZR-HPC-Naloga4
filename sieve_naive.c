// C program to count primes smaller than or equal to N
// compile: gcc -O2 sieve_naive.c --openmp -o sieve
// example run: ./sieve 20

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <omp.h>

void SieveOfEratosthenes(int n)
{
    //Initialize the array of flags for the primes
    //kjer je praštevilo je 1 in kjer ni je 0
    unsigned char * primes = (unsigned char *)malloc((n+1) * sizeof(unsigned char));
    if (!primes) return;
    memset(primes, 1, (n+1));

    //Intialize array for the prime counts
    // koliko je praštevil v posameznem threadu
    unsigned int * prime_cnts = (unsigned int *)calloc(omp_get_max_threads(), sizeof(unsigned int));
    double start = omp_get_wtime();

    //check for primes until sqrt(n) p<=sqrt(n) lahko v zanki ampak je isto sam da se izogne klicu funkcije sqrt
    for (int p = 2; p * p <= n; p++)
    {

        //if flag is set then we encountered a prime number
        if (primes[p])
        {
            //cross out multiples of p grater than the square of p,
            //smaller have already been marked
            // skace za p ker so vsa praštevila ki so manjša od p že označena
            #pragma omp parallel for
            for (int i = p * p; i <= n; i += p)
                primes[i] = 0;
        }
    }
    
    unsigned int totalPrimes=0;
    //find and sum up all primes
    //koliko je praštevil v seznamu 
    #pragma omp parallel
    {
        int id=omp_get_thread_num();
        #pragma omp for
        for (int p = 2; p <= n; p++) 
            if (primes[p])
                prime_cnts[id]++;
                
            // namest tega if napišeš 
            // prime_cnts[id]+= primes[p];

            /*
            če maš kle 
            if(p%2==0)
                prime_cnts[id]++;
            
            in poženeš
            je % napačnih skokov 0.02%
            to je zato ker zna bolje napovedat skoke vsak drug skok je izpolnjen to zna hitro in dobro napovedat
            Rezultat => niso vsi if stavki slabi za napovedovanje skokov 
            */

           /*
           kle lahko nardiš še redukcijo da se znebiš falsesharinga
           #pragma omp for reduction(+:totalPrimes)
            for (int p = 2; p <= n; p++) 
                if (primes[p])
                    prime_cnts[id]++;
           */ 
          //pol zakomentiraš še spodnjo zanko for
    
    }
    //seštej delne rezultate
    for(int i=0; i<omp_get_max_threads(); i++)
    {
        totalPrimes+=prime_cnts[i];
    }

    double stop=omp_get_wtime();
    printf("Total primes less or equal to %d: %d\n",n,totalPrimes);
    printf("Elapsed time: %.3f\n",stop-start);
    free(primes);
    free(prime_cnts);
}
 
// Driver Code
int main(int argc,char* argv[])
{
    if(argc<2){
        printf("Not enough arguments!\n");
        printf("Usage: sieve <N>!\n");
        return 1;
    }
    unsigned int N = atoi(argv[1]);

    SieveOfEratosthenes(N);
    return 0;
}
/*
uporablal bomo linux program perf da nam pove statistiko o programu
predvsem nas zanima število zgrešitev v pomnilniku 

perf stat -B -e cache-references,cache-misses,cycles,stalled-cycles-backend,instructions,branches,branch-misses <executable>

za poganjat uporabi priloženo skripto run.sh

št zgrešitev dobro je 0-10% ta dobi 88% zgrešitev
čas in št zgrešitev se zmanjša ko nardiš segmented sieve

napačni skoki so tisti ki so v if stavku
povzročijo da more procesor razveljavit ukaze v cevovodu ker jih ne bi blo treba delat in jih tut ne smeš
tega se procesorji probajo izognit s tem da napovedujejo skoke (ML, neke tabele) so precej dobri ne pa vedno 
v tem primeru se cca 1% skokov napačno napove

v programu se naberejo pri 
if (primmes[p])
ker procesor ne zna napovedat a bo to praštevilo al ne 

if (primes[p])
    prime_cnts[id]++;
    
// namest tega if napišeš 
// prime_cnts[id]+= primes[p];

Pazi: to poveča število seštevanj ker prištevaš 1 in 0 namesto samo 1
ni nujno da je vedno časovno ugodna operacija



*/
