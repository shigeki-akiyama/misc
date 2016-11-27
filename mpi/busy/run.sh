
mkdir -p data
rm -f data/log.*.out

for i in 1 2 4 8 16 32 64 128; do 
    mpirun -n $i ./a.out seq $((200 * 1000 * 1000)) | tee -a data/log.seq.out
done

for i in 1 2 4 8 16 32 64 128; do
    mpirun -n $i ./a.out par $((200 * 1000 * 1000)) | tee -a data/log.par.out
done

