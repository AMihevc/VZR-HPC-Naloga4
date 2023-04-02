#!/bin/sh
#SBATCH --job-name=sieveParallel
#SBATCH --reservation=fri
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=32
#SBATCH --output=RezultatiSieveParallel.txt
#SBATCH --time=00:20:00
#SBATCH --constraint=AMD

#Zazeni z: sbatch --reservation=fri ./runParallel.sh

#prevedi program da bos imel za ziher neki za izvajat
gcc -O2 sieveParalel.c -o sievepar -lm -fopenmp

#Define an array of values for OMP_NUM_THREADS
threads=(1 2 4 8 16 32)

#Loop through each value of OMP_NUM_THREADS
for thread in ${threads[@]}; do
        echo "############# ST NITI $thread #############" 

        # Set the value of OMP_NUM_THREADS
        export OMP_NUM_THREADS=$thread
        
        # Run the program
        srun perf stat -B -e cache-references,cache-misses,cycles,stalled-cycles-backend,instructions,branches,branch-misses ./sievepar 1000000
        
    done


echo "Konec"
