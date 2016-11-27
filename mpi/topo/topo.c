#include "topo.h"
#include <stdio.h>
#include <mpi.h>
#include <mpi-ext.h>


int main(int argc, char **argv) {
    int i, j;
    MPI_Init(&argc, &argv);

    int me, n_procs;
    MPI_Comm_rank(MPI_COMM_WORLD, &me);
    MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

    char filename[1024];
    sprintf(filename, "topo.%05d.out", me);

    FILE *fp = fopen(filename, "w");

    int dim = topo_dim();

    int shape[3] = { 0 };
    topo_shape(shape);

    int rank3d[3] = { 0 };
    topo_rank2xyz(me, rank3d);
#if 0
    int tofu_x, tofu_y, tofu_z, tofu_a, tofu_b, tofu_c;
    FJMPI_Topology_sys_rank2xyzabc(
        me, &tofu_x, &tofu_y, &tofu_z, &tofu_a, &tofu_b, &tofu_c);
#endif

    MPI_Barrier(MPI_COMM_WORLD);

    fprintf(fp, "me        = %5d, ", me);
    fprintf(fp, "dimension = %2d, ", dim);
    fprintf(fp, "shape     = (%3d, %3d, %3d), ", 
            shape[0], shape[1], shape[2]);
    fprintf(fp, "rank3d    = (%3d, %3d, %3d)\n",
            rank3d[0], rank3d[1], rank3d[2]);
#if 0
    printf("tofu_rank = (%d, %d, %d, %d, %d, %d)\n",
            tofu_x, tofu_y, tofu_z, tofu_a, tofu_b, tofu_c);
#endif

    int neighbs[6];
    topo_neighbs(me, neighbs);

    for (j = 0; j < 2 * dim; j++) {
        int neighb = neighbs[j];

        int rank3d[3] = { 0 };
        topo_rank2xyz(neighb, rank3d);

        fprintf(fp, "me = %d, neighbs[%d] = %d (%d, %d, %d)\n",
                me, j, neighb, rank3d[0], rank3d[1], rank3d[2]);
    }

    fclose(fp);

    MPI_Finalize();
    return 0;
}
