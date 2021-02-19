#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <chrono>
#include "utils.h"
#include <cmath>
using namespace std;

#define ML MPI_LONG
#define MCW MPI_COMM_WORLD
#define MASTER rank == 0
#define BLOCK MPI_Barrier(MCW);

int main(int argc, char ** argv) {
//    srandom(time(0));
    srandom(1);
    int rank, p;
//    const long N = strtol(argv[1], nullptr, 10);
    const long N = 36;
    long *original = new long[N] {16 ,2 ,17, 24, 33, 28, 30, 1, 0, 27, 9, 25, 34, 23, 19, 18, 11, 7, 21, 13, 8, 35, 12, 29, 6, 3, 4, 14, 22, 15, 32, 10, 26, 31, 20, 5};
    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &p);
    int p_sqrd = pow(p, 2);

    const int slice_size = N / p;
    int w = N / p_sqrd;
    int displs[p];
    int scounts[p];
    for (int i = 0; i < p-1; ++i) {
        displs[i] = slice_size * i;
        scounts[i] = slice_size;
    }
    displs[p-1] = slice_size * (p-1);
    scounts[p-1] = N - displs[p-1];

//    if (MASTER) {
//        show(&displs[0], p);
//        show(&scounts[0], p);
        /// Setting up input data
//        for (int i = 0; i < N; ++i) {
////            original[i] = random();
//            original[i] = i;
//        }
//    }

    long part[scounts[rank]];

    MPI_Scatterv(original, scounts, displs, ML, part, scounts[rank], ML, 0, MCW);

//    for (int i = 0; i < p; ++i) {
//        MPI_Barrier(MCW);
//        if (rank == i) {
//            show(&part[0], scounts[rank]);
//        }
//    }

    sort(part, part + scounts[rank]);

//    for (int i = 0; i < p; ++i) {
//        MPI_Barrier(MCW);
//        if (rank == i) {
//            show(&part[0], scounts[rank]);
//        }
//    }

    long sample[p];

    for (int i = 0; i < p; ++i) {
        sample[i] = part[i * w];
    }

    long all_samples[p_sqrd];

    MPI_Gather(sample, p, ML, all_samples, p, ML, 0, MCW);

    long pivots[p-1];
    if (MASTER) {
        sort(all_samples, all_samples + p_sqrd);
//        show(&all_samples[0], p_sqrd);
        int other_p = (p / 2) - 1;

        for (int i = 1; i < p; ++i) {
            pivots[i - 1] = all_samples[(i * p) + other_p];
        }
    }

//    MPI_Scatter()

    BLOCK;

    delete[] original;
    MPI_Finalize();
    return 0;
}