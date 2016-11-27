ssh $1 'mkdir -p wrk/mpi/barrier'
rsync -rav common.h barrier.cc $1:wrk/mpi/barrier/
