#include "common.h"

#include <stdio.h>
#include <limits.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/time.h>

#include <tuple>
using namespace std;

static size_t bin(size_t depth)
{
    size_t i;

    if (depth == 0) {
        return 1;
    } else {
        size_t result = 1;
        result += bin(depth - 1);
        result += bin(depth - 1);
        return result;
    }
}

void real_main(int argc, char **argv)
{
    const size_t n_args = 5;
    if (argc != n_args + 1) {
        fprintf(stderr,
                "usage: %s depth leaf_loops interm_loops interm_iters "
                "pre_exec\n",
                argv[0]);
        return;
    }

    size_t arg_idx = 1;
    size_t depth = (size_t)atol(argv[arg_idx++]);
    size_t leaf_loops = (size_t)atol(argv[arg_idx++]);
    size_t interm_loops = (size_t)atol(argv[arg_idx++]);
    size_t interm_iters = (size_t)atol(argv[arg_idx++]);
    size_t pre_exec = (size_t)atol(argv[arg_idx++]);
    assert(arg_idx == n_args + 1);

    printf("program = bin,\n");
    printf("depth = %zu, leaf_loops = %zu, "
           "interm_loops = %zu, interm_iters = %zu, pre_exec = %zu\n",
           depth, leaf_loops,
           interm_loops, interm_iters, pre_exec);

    // pre-exec
    size_t i;
    for (i = 0; i < pre_exec; i++) {
        bin(depth);
    }

    // do main execution
    double t0 = now();

    size_t nodes = bin(depth);

    double t1 = now();
    
    double time = t1 - t0;
    double throughput = (double)nodes / time;

    //        madm_conf_dump_to_file(stdout);
    printf("np = %zd, time = %.6f,\n"
           "throughput = %.6f, throughput/np = %.6f, "
           "task overhead = %.0f\n",
           1, time,
           1e-6 * throughput,
           1e-6 * throughput,
           1e+9 / throughput);
}

int main(int argc, char **argv)
{
    real_main(argc, argv);

    return 0;
}

