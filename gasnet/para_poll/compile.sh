mpicc -O0 -DGASNET_PAR -I$HOME/local/gasnet/include -I$HOME/local/gasnet/include/mpi-conduit -L$HOME/local/gasnet/lib para_poll.c -lgasnet-mpi-par -lammpi
