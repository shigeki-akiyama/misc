ssh $1 'mkdir -p wrk/gasnet/barrier'
rsync -arv common.h barrier.cc compile.sh $1:wrk/gasnet/barrier/

