#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <chrono>
using namespace std;

int main(int argc, char ** argv) {
    srandom(time(0));

    /// Setting up input data
    const long N = strtol(argv[1], nullptr, 10);
    long original[N];
    for (int i = 0; i < N; ++i) {
        original[i] = random();
    }

    int rank, size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    cout << "I am " << rank + 1 << " of " << size << endl;

    MPI_Finalize();
    return 0;
}