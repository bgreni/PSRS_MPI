#include <iostream>
#include <mpi.h>
#include <stdlib.h>
#include <chrono>
#include "utils.h"
#include <cmath>
#include <algorithm>
#include <vector>
using namespace std;

#define ML MPI_LONG
#define MCW MPI_COMM_WORLD
#define MASTER rank == 0
#define BLOCK MPI_Barrier(MCW);


const int ROOT = 0;

int main(int argc, char ** argv) {
//    srandom(time(0));
    const long SEED = strtol(argv[2], nullptr, 10);
    srandom(SEED);
    int rank, p;
    const long N = strtol(argv[1], nullptr, 10);
//    const long N = 36;
    long *original = new long[N]; //{16 ,2 ,17, 24, 33, 28, 30, 1, 0, 27, 9, 25, 34, 23, 19, 18, 11, 7, 21, 13, 8, 35, 12, 29, 6, 3, 4, 14, 22, 15, 32, 10, 26, 31, 20, 5};
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

    if (MASTER) {
//        show(&displs[0], p);
//        show(&scounts[0], p);
        // Setting up input data
        for (int i = 0; i < N; ++i) {
            original[i] = random();
//            original[i] = i;
        }
    }

    long part[scounts[rank]];

    MPI_Scatterv(original, scounts, displs, ML, part, scounts[rank], ML, ROOT, MCW);

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

    MPI_Gather(sample, p, ML, all_samples, p, ML, ROOT, MCW);

    long pivots[p-1];
    if (MASTER) {
        sort(all_samples, all_samples + p_sqrd);
//        show(&all_samples[0], p_sqrd);
        int other_p = (p / 2) - 1;

        for (int i = 1; i < p; ++i) {
            pivots[i - 1] = all_samples[(i * p) + other_p];
        }
    }

    MPI_Bcast(pivots, p-1, ML, ROOT, MCW);
    vector<vector<long>> partitions(p);
    for (int i = 0; i < p; ++i) {
        // rough estimation of how big each partition will be
        partitions[i].reserve(slice_size / 3);
    }

    int start = 0;
    int end = 0;
    int last_reached = -1;
    for (int i = 0; i < p-1; ++i) {
        for(;end < scounts[rank]; ++end) {
            if (part[end] > pivots[i]) {
                for (int j = start; j < end; ++j) {
                    partitions[i].push_back(part[j]);
                }
                last_reached = i;
                start = end;
                break;
            }
        }
    }

    for (int i = start; i < scounts[rank]; ++i) {
        partitions[last_reached + 1].push_back(part[i]);
    }

    MPI_Request reqs[p * 2];
    MPI_Status stats[p * 2];

    long **recp = new long*[p];
    int part_size_est = slice_size / (p-1);

    for (int i = 0; i < p; ++i) {
        recp[i] = new long[part_size_est];
    }


    for (int i = 0; i < p; ++i) {
        MPI_Isend(&partitions[i][0], (int)partitions[i].size(), ML, i, 0, MCW, &reqs[i]);
        MPI_Irecv(recp[i], part_size_est, ML, i, 0, MCW, &reqs[i + p]);
    }
    BLOCK
    MPI_Waitall(p*2, reqs, stats);
    int sizes[p];
    for (int i = p; i < p*2; ++i) {
        MPI_Get_count(&stats[i], ML, &sizes[i - p]);
    }

    int indexes[p] = { 0 };
    int new_size = 0;
    for (int i = 0; i < p; ++i) {
        new_size += sizes[i];
    }

    long new_part[new_size];
    for (int i = 0; i < new_size; ++i) {
        int taken_index = 0;
        long smallest = INT64_MAX;
        long curr;
        for (int j = 0; j < p; ++j) {
            if (indexes[j] < sizes[j]) {
                curr = recp[j][indexes[j]];
                if (curr < smallest) {
                    smallest = curr;
                    taken_index = j;
                }
            }
        }
        indexes[taken_index] += 1;
        new_part[i] = smallest;
    }



//    for (int i = 0; i < p; ++i) {
//        MPI_Barrier(MCW);
//        if (rank == i) {
//            show(&new_part[0], new_size);
//        }
//    }

    verify_sorted(new_part, new_size);


//    if (MASTER) {
//    }
//    for (int i = 0; i < p; ++i) {
//        MPI_Barrier(MCW);
//        if (rank == i) {
//            cout << "RANK: " << rank << endl;
//            for (int j = 0; j < p; ++j) {
//                show(&recp[j][0], sizes[j]);
//            }
//        }
//    }

    for (int i = 0; i < p; ++i) {
        delete[] recp[i];
    }
    delete[] recp;

    delete[] original;
    MPI_Finalize();
    if (MASTER)
        cout << "done :)" << endl;
    return 0;
}