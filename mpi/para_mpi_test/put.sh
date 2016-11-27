
if [ $# -ne 1 ]; then
    echo "Usage: $0 hostname"
    exit 1
fi
host=$1

dir=wrk/mpi
prog=para_mpi_test
filelist="$prog.c compile.sh run.sh"

ssh $host "mkdir -p $dir/$prog/"
scp $filelist $host:$dir/$prog/

