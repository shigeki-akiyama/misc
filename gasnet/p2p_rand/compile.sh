MPICC=mpigccpx
GASNET_PREFIX=$HOME/local/sparc/gasnet

$MPICC -g p2p_rand.c -DGASNET_PAR \
    -I$GASNET_PREFIX/include \
    -I$GASNET_PREFIX/include/mpi-conduit \
    -L$GASNET_PREFIX/lib \
    -lgasnet-mpi-par -lammpi

