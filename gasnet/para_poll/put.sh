
if [ $# -ne 1 ]; then
    echo "Usage: $0 hostname"
    exit 1
fi
host=$1

prog=para_poll
filelist="$prog.c compile.sh run.sh"

ssh $host "mkdir -p wrk/gasnet/$prog"
scp $filelist $host:wrk/gasnet/$prog/

