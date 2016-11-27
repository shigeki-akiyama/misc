
if [ $# -ne 1 ]; then
    echo "Usage: $0 hostname"
    exit 1
fi
host=$1

mkdir -p data

prog=para_mpi_test
dir=wrk/mpi
scp $host:$dir/$prog/data/log.out ~/$dir/$prog/data/log.$host.out

