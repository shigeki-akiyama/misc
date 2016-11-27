#include <mpi.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    int i;

    MPI_Init(&argc, &argv);

//     char hostname[256];
//     gethostname(hostname, 256 - 1);
//
//     MPI_Info info;
//     MPI_Info_create(&info);
//     MPI_Info_set(info, "host", hostname);

    MPI_Comm parent;
    MPI_Comm_get_parent(&parent);

    if (parent == MPI_COMM_NULL) {
        int me, n_procs;
        MPI_Comm_rank(MPI_COMM_WORLD, &me);
        MPI_Comm_size(MPI_COMM_WORLD, &n_procs);

        printf("The parent (%d) start spawning\n", me);
#if 0
        MPI_Comm comm;
        int max_procs = 2; //4 * n_procs;
        int root = 0;
        int result = MPI_Comm_spawn(argv[0], MPI_ARGV_NULL, max_procs,
                                    MPI_INFO_NULL, root, MPI_COMM_WORLD,
                                    &comm, MPI_ERRCODES_IGNORE);
#else
        int max_procs = n_procs;
        int root = 0;

        char **command_arr = new char *[max_procs];
        int *size_arr = new int[max_procs];
        MPI_Info *info_arr = new MPI_Info[max_procs];

        int hostname_len;
        char hostname[MPI_MAX_PROCESSOR_NAME];
        MPI_Get_processor_name(hostname, &hostname_len);

        char *hostnames = new char[MPI_MAX_PROCESSOR_NAME * max_procs];

        MPI_Allgather(hostname, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                      hostnames, MPI_MAX_PROCESSOR_NAME, MPI_CHAR,
                      MPI_COMM_WORLD);

        char *host = hostnames;
        for (i = 0; i < max_procs; i++) {
            command_arr[i] = argv[0];
            size_arr[i] = 1;

            MPI_Info_create(&info_arr[i]);
            MPI_Info_set(info_arr[i], "host", host);

            host += MPI_MAX_PROCESSOR_NAME;
        }

        MPI_Comm comm;
        int result = MPI_Comm_spawn_multiple(max_procs, command_arr,
                                             MPI_ARGVS_NULL, size_arr,
                                             info_arr, root, MPI_COMM_WORLD,
                                             &comm, MPI_ERRCODES_IGNORE);
#endif

        printf("spawned (%d)\n", me);

//        for (;;) sleep(0);
    } else {
        int me, n_procs;
        MPI_Comm_rank(MPI_COMM_WORLD, &me);
        MPI_Comm_size(MPI_COMM_WORLD, &n_procs);
        printf("I'm child (%d/%d)\n", me, n_procs);

//        for (;;)
//            ;
    }

//     MPI_Info_free(&info);

    MPI_Finalize();
    return 0;
}

