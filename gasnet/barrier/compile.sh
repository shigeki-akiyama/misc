MPICC=mpig++px
GASNET_PREFIX=$HOME/local/sparc/gasnet

$MPICC -std=c++0x -g barrier.cc -DGASNET_PAR \
    -I$GASNET_PREFIX/include \
    -I$GASNET_PREFIX/include/mpi-conduit \
    -L$GASNET_PREFIX/lib \
    -lgasnet-mpi-par -lammpi

