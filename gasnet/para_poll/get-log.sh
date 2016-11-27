
if [ $# -ne 1 ]; then
    echo "Usage: $0 hostname"
    exit 1
fi
host=$1

mkdir -p data

dir=wrk/gasnet/para_poll
scp $host:$dir/data/log.out ~/$dir/data/log.$host.out

