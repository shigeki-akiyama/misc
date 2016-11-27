
mkdir -p data
rm -f dat/log.out

for i in 1 2 4 8 16 32 64 128; do 
    mpirun -n $i ./a.out seq $((10 * 1000 * 1000)) | tee -a data/log.out
done

