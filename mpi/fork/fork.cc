#include <mpi.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    printf("The parent start spawning\n");
    fflush(stdout);

    int pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        // child
        printf("I'm a child\n");

        return 0;
    }

    // parent
    printf("spawned\n");

    int status;
    pid_t r = waitpid(pid, &status, 0);

//    if (WIFEXITED(status)) {
//        int result = WEXITSTATUS(status);
//    } else


    MPI_Finalize();
    return 0;
}

